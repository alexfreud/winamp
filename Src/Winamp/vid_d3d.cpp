#include "vid_d3d.h"
#include "vid_subs.h"
#include "../nu/AutoWide.h"
#include "videooutput.h"
#include "../nsutil/image.h"
#include <d3d9.h>
#include "WinampAttributes.h"
#include "IVideoD3DOSD.h"
#include <stdint.h>

struct YV12_PLANES;

typedef HRESULT (WINAPI *DIRECT3DCREATE9EX)(UINT SDKVersion, IDirect3D9Ex **);
typedef IDirect3D9 *(WINAPI *DIRECT3DCREATE9)(UINT SDKVersion);
static DIRECT3DCREATE9EX creatorex=0;
static DIRECT3DCREATE9 creator=0;
static HMODULE d3d_lib = 0;
static bool tried=false;
static DWORD winver;

extern IVideoOSD *posd;

static bool CreateDirect3D(IDirect3D9 **d3d, IDirect3D9Ex **d3dEx)
{
	if (!d3d_lib && !creator)
	{
		if (tried)
			return false;

		d3d_lib = LoadLibraryW(L"d3d9.dll");
		if (!d3d_lib)
		{
			tried=true;
			return false;
		}

		creatorex = (DIRECT3DCREATE9EX)GetProcAddress(d3d_lib, "Direct3DCreate9Ex");
		creator = (DIRECT3DCREATE9)GetProcAddress(d3d_lib, "Direct3DCreate9");
		if (!creatorex && !creator)
		{
			FreeLibrary(d3d_lib);
			tried=true;
			return false;
		}
	}
	if (creatorex)
	{
		if (SUCCEEDED(creatorex(D3D_SDK_VERSION, d3dEx)) && *d3dEx)
		{
			(*d3dEx)->QueryInterface(__uuidof(IDirect3D9), (void **)d3d);
			return true;
		}
	}

	if (creator)
	{
		*d3d = creator(D3D_SDK_VERSION);
		*d3dEx=0;
		return !!d3d;
	}
	return false;
}

static void BuildPresentationParameters(D3DPRESENT_PARAMETERS &presentation_parameters,  HWND hwnd, D3DSWAPEFFECT swap_effect)
{
	memset(&presentation_parameters, 0, sizeof(presentation_parameters));
	presentation_parameters.BackBufferWidth = 0;
	presentation_parameters.BackBufferHeight = 0;
	presentation_parameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentation_parameters.BackBufferCount = 1;
	presentation_parameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	presentation_parameters.MultiSampleQuality = 0;
	presentation_parameters.SwapEffect = swap_effect;
	
	presentation_parameters.hDeviceWindow = hwnd;
	presentation_parameters.Windowed = TRUE;
	presentation_parameters.EnableAutoDepthStencil = FALSE;
	presentation_parameters.Flags = /*D3DPRESENTFLAG_LOCKABLE_BACKBUFFER |*/ D3DPRESENTFLAG_VIDEO;
	presentation_parameters.FullScreen_RefreshRateInHz = 0;
	presentation_parameters.PresentationInterval = (!config_video_vsync2 ? D3DPRESENT_INTERVAL_IMMEDIATE : D3DPRESENT_INTERVAL_ONE);
}

static UINT FindMyMonitor(IDirect3D9 *d3d, HWND hwnd)
{
	HMONITOR monitor=MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	if (monitor)
	{
		UINT num_adapters = d3d->GetAdapterCount();
		for (UINT i=0;i!=num_adapters;i++)
		{
			if (d3d->GetAdapterMonitor(i) == monitor)
				return i;
		}
	}
	return D3DADAPTER_DEFAULT;
}

D3DDEVTYPE Direct3DVideoOutput::GetDeviceType(IDirect3D9 *d3d, UINT display_adapter)
{
	D3DCAPS9 caps;
	D3DDEVTYPE device_type = D3DDEVTYPE_HAL;
	for(;;)
	{
		HRESULT hr = d3d->GetDeviceCaps(display_adapter, device_type, &caps);
		if (hr == S_OK)
		{
			if ((D3DPTFILTERCAPS_MAGFLINEAR & caps.StretchRectFilterCaps) && (D3DPTFILTERCAPS_MINFLINEAR & caps.StretchRectFilterCaps))
				stretch_filter = D3DTEXF_LINEAR;
			else if ((D3DPTFILTERCAPS_MAGFPOINT & caps.StretchRectFilterCaps) && (D3DPTFILTERCAPS_MINFPOINT & caps.StretchRectFilterCaps))
				stretch_filter = D3DTEXF_POINT;
			else
				stretch_filter = D3DTEXF_NONE;
			return device_type;
		}
		if (device_type == D3DDEVTYPE_HAL)
			device_type = D3DDEVTYPE_REF;
		else
			break;
	}

	return (D3DDEVTYPE)0;
}

static D3DSWAPEFFECT GetSwapEffect(IDirect3D9 *d3d, UINT adapter, D3DDEVTYPE device_type, bool ex)
{
	if (ex)
	{
		D3DCAPS9 caps;
		d3d->GetDeviceCaps(adapter, device_type, &caps);
		if (caps.Caps & D3DCAPS_OVERLAY)
		{
			return D3DSWAPEFFECT_OVERLAY;
		}
		else 
		{
			return D3DSWAPEFFECT_FLIPEX;// (D3DSWAPEFFECT)5;
		}
	}
	else
	{
		return D3DSWAPEFFECT_FLIP;
	}
}

Direct3DVideoOutput::Direct3DVideoOutput(HWND parent, VideoAspectAdjuster *_adjuster)
{
	CreateDirect3D(&d3d, &d3dEx);
	device = 0;
	deviceEx = 0;
	surface = 0;
	width=0;
	height=0;
	GetWindowRect(parent, &last_rect);
	surface_type = D3DFMT_UNKNOWN;
	display_adapter = 0;
	subtitle_font = 0;
	current_subtitle = 0;
	need_change = 0;
	adjuster = _adjuster;
	hwnd = parent;
	opened=false;
	valid_surface=false;
	logo_surface = 0;
	input_type = 0;
	flip = 0;
	m_palette = 0;

	if (d3dEx)
	{
		display_adapter = FindMyMonitor(d3d, parent);
		D3DDEVTYPE device_type = GetDeviceType(d3d, display_adapter);
		swap_effect = GetSwapEffect(d3d, display_adapter, device_type, true);
		D3DPRESENT_PARAMETERS presentation_parameters;
		BuildPresentationParameters(presentation_parameters, parent, swap_effect);
		HRESULT hr = d3dEx->CreateDeviceEx(display_adapter, D3DDEVTYPE_HAL, parent,
			D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED|D3DCREATE_NOWINDOWCHANGES,
			&presentation_parameters, 0, 
			&deviceEx);

		if (FAILED(hr))
		{ // try again with mixed processing
			hr = d3dEx->CreateDeviceEx(display_adapter, D3DDEVTYPE_HAL, parent,
				D3DCREATE_MIXED_VERTEXPROCESSING|D3DCREATE_MULTITHREADED|D3DCREATE_NOWINDOWCHANGES,
				&presentation_parameters, 0, 
				&deviceEx);

			if (FAILED(hr))
			{ // and finally software
				hr = d3dEx->CreateDeviceEx(display_adapter, D3DDEVTYPE_HAL, parent,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED|D3DCREATE_NOWINDOWCHANGES,
					&presentation_parameters, 0, 
					&deviceEx);
			}
		}

		if (SUCCEEDED(hr))
			deviceEx->QueryInterface(__uuidof(IDirect3DDevice9), (void **)&device);
	}

	if (!deviceEx)
	{
		display_adapter = FindMyMonitor(d3d, parent);
		D3DDEVTYPE device_type = GetDeviceType(d3d, display_adapter);
		swap_effect = GetSwapEffect(d3d, display_adapter, device_type, false);
		if (device_type)
		{
			D3DPRESENT_PARAMETERS presentation_parameters;
			BuildPresentationParameters(presentation_parameters, parent, swap_effect);
			HRESULT hr = d3d->CreateDevice(display_adapter, device_type, parent,
				D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED|D3DCREATE_NOWINDOWCHANGES,
				&presentation_parameters,
				&device);

			if (FAILED(hr))
			{ // try again with mixed processing
				hr = d3d->CreateDevice(display_adapter, device_type, parent,
					D3DCREATE_MIXED_VERTEXPROCESSING|D3DCREATE_MULTITHREADED|D3DCREATE_NOWINDOWCHANGES,
					&presentation_parameters,
					&device);
				if (FAILED(hr))
				{ // and finally software
					/*hr = */d3d->CreateDevice(display_adapter, device_type, parent,
						D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED|D3DCREATE_NOWINDOWCHANGES,
						&presentation_parameters,
						&device);
				}
			}
		}
	}

	if (device)
	{
		// TODO: retrieve dimensions from VideoOutput
		device->CreateOffscreenPlainSurface(128, 128, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &logo_surface, 0);
		((IVideoD3DOSD *)posd)->CreateOSD(device);
	}	
}

static D3DFORMAT GetColorFormat(unsigned int type)
{
	switch(type)
	{
	case MAKEFOURCC('Y', 'V', '1', '2'):
		return (D3DFORMAT)type;
	case MAKEFOURCC('Y', 'U', 'Y', '2'):
		return D3DFMT_YUY2;
	case MAKEFOURCC('U', 'Y', 'V', 'Y'):
		return D3DFMT_UYVY;
	case MAKEFOURCC('R', 'G', 'B', '8'):
		return D3DFMT_P8;
	case MAKEFOURCC('R', 'G', '3', '2'):
		return D3DFMT_X8R8G8B8;
	case MAKEFOURCC('R', 'G', '2', '4'):
		return D3DFMT_R8G8B8;
	case MAKEFOURCC('R', '5', '5', '5'):
		return D3DFMT_X1R5G5B5;
	case MAKEFOURCC('R', '5', '6', '5'):
		return D3DFMT_R5G6B5;
	default:
		return (D3DFORMAT)D3DFMT_UNKNOWN;
	}
}

static bool SubstituteSurfaceType(D3DFORMAT surface_format, D3DFORMAT &substitute_format, int n)
{
	if (surface_format == D3DFMT_R8G8B8)
	{
		switch(n)
		{
		case 0:
			substitute_format = D3DFMT_X8R8G8B8;
			return true;
		}
	}
	else if (surface_format == 	(D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'))
	{
		switch(n)
		{
		case 0:
			substitute_format = D3DFMT_YUY2;
			return true;
		case 1:
			substitute_format = D3DFMT_UYVY;
			return true;
		case 2:
			substitute_format =  D3DFMT_X8R8G8B8;
			return true;
		}
	}
	return false;
}

static void PreferredSurfaceType(D3DFORMAT &surface_format)
{
	if (surface_format == D3DFMT_P8)
	{
		surface_format = D3DFMT_X8R8G8B8;
	}
}

void Direct3DVideoOutput::setVFlip(int on)
{
	flip = on;
	if (config_video_fliprgb)
		flip = !flip;
}

int Direct3DVideoOutput::OpenVideo(int w, int h, unsigned int type, int flipit, double aspectratio)
{
	if (!device)
		return 0;
	D3DFORMAT new_surface_type = GetColorFormat(type);
	if (new_surface_type == D3DFMT_UNKNOWN) // not supported
		return 0;

	PreferredSurfaceType(new_surface_type);

	input_type = type;

	// see what was set for the flip flag
	// as nsv streams are generally flipped
	// so we'll need to invert the handling
	flip = flipit;
	if (config_video_fliprgb)
		flip = !flip;

	// see if we can re-use our old surface
	if (!surface || surface_type != new_surface_type || w != width || h != height)
	{ 
		if (surface)
		{
			surface->Release();
			surface=0;
		}

		HRESULT hr;

		D3DFORMAT try_surface_type = new_surface_type;
		int n=0;
		do 
		{
			hr = device->CreateOffscreenPlainSurface(w, h, try_surface_type, D3DPOOL_DEFAULT, &surface, 0);

		} while (hr != S_OK && SubstituteSurfaceType(new_surface_type, try_surface_type, n++));

		surface_type = try_surface_type;

		if (hr != S_OK)
			return 0;

		width = w;
		height =h;
	}

	valid_surface=false;
	opened=true;

	return 1;
}

void Direct3DVideoOutput::OnWindowSize()
{
	if (device)
	{
		RECT wnd_rect;
		GetWindowRect(hwnd, &wnd_rect);
		if (need_change || !EqualRect(&wnd_rect, &last_rect))
		{
			if ( need_change || (last_rect.right - last_rect.left) != (wnd_rect.right - wnd_rect.left)
				|| (last_rect.bottom - last_rect.top) != (wnd_rect.bottom - wnd_rect.top))
			{
				// TODO: check adapter
				D3DPRESENT_PARAMETERS presentation_parameters;
				BuildPresentationParameters(presentation_parameters, hwnd, swap_effect);
				//if (surface)
				//				surface->Release();
				//surface=0;
				if (subtitle_font)
					subtitle_font->Release();  // I hate to do this but we might need a new font size, and it works around a bug
				subtitle_font = 0;

				if (((IVideoD3DOSD *)posd)->isOSDInited())
				{ 
					((IVideoD3DOSD *)posd)->LostOSD();
				}
				HRESULT hr = device->Reset(&presentation_parameters);
				if (FAILED(hr))
				{
					if (surface)
						surface->Release();
					surface=0;
					/*hr = */device->Reset(&presentation_parameters);
					/*hr = */device->CreateOffscreenPlainSurface(width, height, surface_type, D3DPOOL_DEFAULT, &surface, 0);
				}
				if (((IVideoD3DOSD *)posd)->isOSDInited())
				{ 
					((IVideoD3DOSD *)posd)->ResetOSD(device);
				}
				drawSubtitle(current_subtitle);
				last_rect = wnd_rect;
				hr = device->BeginScene();
				if (SUCCEEDED(hr))
					DoRender();
			}
			else
			{
				// TODO: check adapter
			}

			last_rect = wnd_rect;
		}
	}
}

HRESULT Direct3DVideoOutput::DoRender()
{
	RECT r;
	r = last_rect;
	r.right -= r.left;
	r.left = 0;
	r.bottom -= r.top;
	r.top = 0;

	//GetClientRect(hwnd, &r);
	HRESULT hr ;

	IDirect3DSurface9 *back_buffer = 0;
	hr = device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
	if (FAILED(hr))
	{
		need_change=1;
		device->EndScene();
		return hr;
	}
	RECT full_rect = r;
	adjuster->adjustAspect(r);

	if (opened  && valid_surface)
	{
		RECT sides[4]={
			{0, 0, r.left, full_rect.bottom}, // left side
			{r.right, 0, full_rect.right, full_rect.bottom}, // right side
			{0, 0, full_rect.right, r.top}, // top
			{0, r.bottom, full_rect.right, full_rect.bottom} // bottom
		};
		for (int i=0;i<4;i++)
			hr=device->ColorFill(back_buffer, &sides[i], D3DCOLOR_XRGB(0, 0, 0));

		hr = device->StretchRect(surface, NULL, back_buffer, &r, stretch_filter);
	}
	else
	{
		hr=device->ColorFill(back_buffer, 0, D3DCOLOR_XRGB(0, 0, 0));
		HDC logo_dc=0;
		HRESULT hr = logo_surface->GetDC(&logo_dc);
		if (hr == S_OK)
		{ // draw logo
			RECT r={0,0,128,128};
			adjuster->DrawLogo(logo_dc, &r);
			logo_surface->ReleaseDC(logo_dc);
			POINT logo_pt = {full_rect.right/2 - 64,full_rect.bottom/2 - 64};
			device->UpdateSurface(logo_surface, 0, back_buffer, &logo_pt);
		}
	}
	back_buffer->Release();

	if (current_subtitle && subtitle_font)
	{
		DWORD oldValue;
		device->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldValue);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		RECT subRect, origRect;
		GetClientRect(hwnd, &subRect);
		origRect = subRect;
		D3DCOLOR text_color = D3DCOLOR_XRGB(current_subtitle->colorRed, current_subtitle->colorGreen, current_subtitle->colorBlue);
		AutoWide wide_text(current_subtitle->text);

		// calculate where to draw
		// TODO: move to drawSubtitle
		subtitle_font->DrawTextW(NULL, wide_text, -1, &subRect, DT_TOP | DT_WORDBREAK | DT_NOCLIP | DT_CENTER | DT_CALCRECT, text_color);
		int height_delta = (origRect.bottom - origRect.top) - (subRect.bottom - subRect.top);
		subRect.top += height_delta;
		subRect.bottom += height_delta;
		// draw 
		hr = subtitle_font->DrawTextW(NULL, wide_text, -1, &subRect, DT_TOP | DT_WORDBREAK | DT_NOCLIP | DT_CENTER, text_color);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, oldValue);
	}

	if (posd && ((IVideoD3DOSD *)posd)->Showing() && ((IVideoD3DOSD *)posd)->isOSDReadyToDraw())
	{
		((IVideoD3DOSD *)posd)->DrawOSD(device);
	}

	hr = device->EndScene();

	if (FAILED(hr))
	{
		need_change = 1;
		return hr;
	}

	if (deviceEx)
		hr = deviceEx->PresentEx(NULL, NULL, NULL, NULL, D3DPRESENT_DONOTWAIT);
	else
		hr = device->Present(NULL, NULL, NULL, NULL);

	if (hr == D3DERR_DEVICELOST)
		need_change = 1;

	return hr;
}

void YV12_to_UYVY(unsigned char *output, const YV12_PLANES *planes, int pitch, int width, int height, int flip);
void YV12_to_YV12(unsigned char *output, const YV12_PLANES *planes, int pitch, int width, int height, int flip);
void YV12_to_YUY2(unsigned char *output, const YV12_PLANES *planes, int pitch, int width, int height, int flip);
void YUY2_to_YUY2(unsigned char *output, const char *buf, int pitch, int width, int height, int flip);

void Direct3DVideoOutput::displayFrame(const char *buf, int size, int time)
{
	if (need_change || !surface)
		return;

	RECT r;
	GetClientRect(hwnd, &r);

	HRESULT hr = device->BeginScene();
	if (FAILED(hr))
	{
		need_change=1;
		return;
	}

	D3DLOCKED_RECT locked_surface;
	hr = surface->LockRect(&locked_surface, NULL, D3DLOCK_DISCARD);
	if (SUCCEEDED(hr))
	{
		if (input_type == MAKEFOURCC('Y','V','1','2'))
		{
			const YV12_PLANES *planes = (const YV12_PLANES *)buf;
			switch((DWORD)surface_type)
			{
			case MAKEFOURCC('Y','V','1','2'):
				YV12_to_YV12((unsigned char *)locked_surface.pBits, planes, locked_surface.Pitch, width, height, flip);
				break;
			case MAKEFOURCC('Y','U','Y','2'):
				YV12_to_YUY2((unsigned char *)locked_surface.pBits, planes, locked_surface.Pitch, width, height, flip);
				break;
			case MAKEFOURCC('U','Y','V','Y'):
				YV12_to_UYVY((unsigned char *)locked_surface.pBits, planes, locked_surface.Pitch, width, height, flip);
				break;
			case D3DFMT_X8R8G8B8:
				{
					const uint8_t *plane_bufs[3] = { planes->y.baseAddr, planes->v.baseAddr, planes->u.baseAddr};
					const size_t plane_strides[3] = { (size_t)planes->y.rowBytes, (size_t)planes->v.rowBytes, (size_t)planes->u.rowBytes };
					if (flip)
						nsutil_image_Convert_YUV420_RGB32((RGB32 *)((int8_t *)locked_surface.pBits + locked_surface.Pitch*(height-1)), -locked_surface.Pitch, width, height, plane_bufs, plane_strides);
					else
						nsutil_image_Convert_YUV420_RGB32((RGB32 *)locked_surface.pBits, locked_surface.Pitch, width, height, plane_bufs, plane_strides);
				}
				break;
			}
		}
		else if (surface_type == D3DFMT_YUY2) // YUY2
		{
			YUY2_to_YUY2((unsigned char *)locked_surface.pBits, buf, locked_surface.Pitch, width, height, flip);
		}
		else if (surface_type == D3DFMT_UYVY) // YUY2
		{
			YUY2_to_YUY2((unsigned char *)locked_surface.pBits, buf, locked_surface.Pitch, width, height, flip);
		}
		else if (surface_type == D3DFMT_P8) // 8bit with palette
		{
			if (flip)
				nsutil_image_CopyFlipped_U8((uint8_t *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width, width, height);
			else
				nsutil_image_Copy_U8((uint8_t *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width, width, height);
		}
		else if (surface_type == D3DFMT_X8R8G8B8) // RGB32
		{
			if (input_type == MAKEFOURCC('R','G','3','2'))
			{
				if (flip)
					nsutil_image_CopyFlipped_U8((uint8_t *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width*4, width*4, height);
				else
					nsutil_image_Copy_U8((uint8_t *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width*4, width*4, height);
			}
			else if (input_type == MAKEFOURCC('R','G','2','4'))
			{
				if (flip)
					nsutil_image_ConvertFlipped_RGB24_RGB32((RGB32 *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width*3, width, height);
				else
					nsutil_image_Convert_RGB24_RGB32((RGB32 *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width*3, width, height);
			}
			else if (input_type == MAKEFOURCC('R','G','B','8') && m_palette)
			{
				if (flip)
					nsutil_image_PaletteFlipped_RGB32((RGB32 *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width, width, height, (RGB32 *)m_palette);
				else
					nsutil_image_Palette_RGB32((RGB32 *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width, width, height, (RGB32 *)m_palette);
			}
		}
		else if (surface_type == D3DFMT_R8G8B8) // RGB24
		{
			if (flip || locked_surface.Pitch != width*3)
			{
				char *start = (char *)locked_surface.pBits;
				if (flip)
					start += locked_surface.Pitch * (height-1);
				ptrdiff_t pitch = flip?-locked_surface.Pitch:locked_surface.Pitch;

				for (int i=0;i<height;i++)
				{
					char *line = start + pitch * i;
					memcpy(line, buf + width*i*3, width*3);
				}
			}
			else
			{
				memcpy(locked_surface.pBits, buf, width*height*3);
			}
		}
		else if (surface_type == D3DFMT_X1R5G5B5)
		{
			if (flip)
				nsutil_image_CopyFlipped_U8((uint8_t *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width*2, width*2, height);
			else
				nsutil_image_Copy_U8((uint8_t *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width*2, width*2, height);
		}
		else if (surface_type == D3DFMT_R5G6B5)
		{
			if (flip)
				nsutil_image_CopyFlipped_U8((uint8_t *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width*2, width*2, height);
			else
				nsutil_image_Copy_U8((uint8_t *)locked_surface.pBits, locked_surface.Pitch, (const uint8_t *)buf, width*2, width*2, height);
		}

		valid_surface = true;
		/*hr = */surface->UnlockRect();
	}
	DoRender();
}

void Direct3DVideoOutput::close()
{
	opened=false;
}

int Direct3DVideoOutput::onPaint(HWND hwnd)
{
	PAINTSTRUCT p;
	BeginPaint(hwnd, &p);
	if (swap_effect == D3DSWAPEFFECT_OVERLAY)
				{
					deviceEx->PresentEx(0, 0, 0, 0, D3DPRESENT_UPDATEOVERLAYONLY);
				}
				else
				{
	RECT r;
	GetClientRect(hwnd, &r);

	if (surface && !need_change)
	{
		HRESULT hr = device->BeginScene();
		if (SUCCEEDED(hr))
			DoRender();
		else
			need_change=1;
	}
	}
	EndPaint(hwnd, &p);
	return 1;
}

void Direct3DVideoOutput::Refresh()
{
	/* TODO: sanity check but do this
	HRESULT hr = device->BeginScene();
	IDirect3DSurface9 *back_buffer = 0;
	hr = device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
	adjuster->adjustAspect(r);
	hr = device->StretchRect(surface, NULL, back_buffer, &r, stretch_filter);
	back_buffer->Release();
	hr = device->EndScene();
	hr = device->Present(NULL, NULL, NULL, NULL);
	*/
	InvalidateRect(hwnd, NULL, TRUE);
}

void Direct3DVideoOutput::timerCallback()
{
}

void Direct3DVideoOutput::drawSubtitle(SubsItem *item)
{
	current_subtitle = item; 
	if (current_subtitle) 
	{
		if (!subtitle_font)
		{
			int font_size = 14 + item->fontSize + MulDiv(18, (last_rect.bottom - last_rect.top), 768);
			pCreateFontW(device, font_size, 0,		400,

				1,                       
				0, 
				DEFAULT_CHARSET, 
				OUT_DEFAULT_PRECIS, 
				ANTIALIASED_QUALITY,//DEFAULT_QUALITY, 
				DEFAULT_PITCH,
				L"Arial", 
				&subtitle_font);
		}

		if (subtitle_font)
		{
			// TODO: make an AutoWideDup and use during rendering also, saves some mallocs
			AutoWide wide_text(item->text, CP_UTF8);
			if (wide_text)
				subtitle_font->PreloadTextW(wide_text, lstrlenW(wide_text));
			// TODO: DT_CALCRECT
		}
	}
}

void Direct3DVideoOutput::resetSubtitle()
{
	current_subtitle = 0;
}

void Direct3DVideoOutput::setPalette(RGBQUAD *pal)
{
	/* benski> can't get D3DFMT_P8 surfaces to use this during StretchRect, so I'll just forget about it
	for (int i=0;i<256;i++)
	{
	pal[i].rgbReserved = 0xFF;
	}
	HRESULT hr = device->SetPaletteEntries(0, (CONST PALETTEENTRY *)pal);
	hr = device->SetCurrentTexturePalette(0);
	hr = device->SetPaletteEntries(0, (CONST PALETTEENTRY *)pal);*/
	m_palette = pal;
}