#pragma once
#include "VideoOutputChild.h"
#include <d3d9.h>
#include <d3dx9.h>

typedef HRESULT (WINAPI *D3DXCREATEFONTW)(LPDIRECT3DDEVICE9, INT, UINT, UINT, UINT, BOOL, DWORD, DWORD, DWORD, DWORD, LPCWSTR, LPD3DXFONT *);
extern D3DXCREATEFONTW pCreateFontW;

class Direct3DVideoOutput 
{
public:
	Direct3DVideoOutput(HWND hwnd, VideoAspectAdjuster *_adjuster);
	int OpenVideo(int w, int h, unsigned int type, int flipit, double aspectratio);
	void OnWindowSize();

	int onPaint(HWND hwnd);
	void displayFrame(const char *buf, int size, int time);
	void close(); // hides any output of the video
	void timerCallback();
	void setPalette(RGBQUAD *pal);
	void drawSubtitle(SubsItem *item);
	void resetSubtitle();
	void setVFlip(int on);
	void Refresh();

private:
	HRESULT DoRender();
	bool FindSuitableConversion(UINT adapter, D3DFORMAT output_format);
	D3DDEVTYPE GetDeviceType(IDirect3D9 *d3d, UINT display_adapter);
	IDirect3D9 *d3d;
	IDirect3D9Ex *d3dEx;
	IDirect3DDevice9 *device;
	IDirect3DDevice9Ex *deviceEx;
	IDirect3DSurface9 *surface, *logo_surface;
	ID3DXFont *subtitle_font;
	D3DFORMAT surface_type;
	D3DSWAPEFFECT swap_effect;
	int input_type;
	int width, height;
	VideoAspectAdjuster *adjuster;
	HWND hwnd;
	RECT last_rect;
	int flip;
	UINT display_adapter;
	SubsItem *current_subtitle;
	bool opened, valid_surface;
	int need_change;
	D3DTEXTUREFILTERTYPE stretch_filter;
	
	RGBQUAD *m_palette;
};