#include "main.h"
#include "IVideoD3DOSD.h"
#include "resource.h"

extern wchar_t FileTitle[];
static HMODULE d3dx_lib = 0;

// For non-debug builds, comment out DXTraceW debug statements to 
// remove them completely
//#ifndef _DEBUG
#define DXTraceW //
//#endif

typedef HRESULT (WINAPI *D3DXCREATESPRITE)(LPDIRECT3DDEVICE9, LPD3DXSPRITE *);
typedef HRESULT (WINAPI *D3DXCREATEFONTW)(LPDIRECT3DDEVICE9, INT, UINT, UINT, UINT, BOOL, DWORD, DWORD, DWORD, DWORD, LPCWSTR, LPD3DXFONT *);
typedef HRESULT (WINAPI *D3DXCREATETEXTUREFROMRESOURCEEXW)(LPDIRECT3DDEVICE9, HMODULE, LPCWSTR, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, DWORD, DWORD, D3DCOLOR, D3DXIMAGE_INFO *, PALETTEENTRY *, LPDIRECT3DTEXTURE9 *);
D3DXCREATESPRITE pCreateSprite = NULL;
D3DXCREATEFONTW pCreateFontW = NULL;
D3DXCREATETEXTUREFROMRESOURCEEXW pCreateTextureFromResourceExW = NULL;

HMODULE FindD3DX9()
{
	if (d3dx_lib)
		return d3dx_lib;

	HMODULE d3dx9 = NULL;
	HANDLE hFind;
	WIN32_FIND_DATAW pfiledata;
	wchar_t systemDir[MAX_PATH] = {0};
	wchar_t libPath[MAX_PATH] = {0};
	GetSystemDirectoryW(systemDir, MAX_PATH);
	StringCchCatW(systemDir, MAX_PATH,L"\\d3dx9_");
	StringCchCopyW(libPath, MAX_PATH, systemDir);
	StringCchCatW(systemDir, MAX_PATH,L"*.dll");

	hFind = FindFirstFileW(systemDir,&pfiledata);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		BOOL more = true;
		int iHighVersion = 0;
		while (more)
		{
			wchar_t *start = wcsrchr(pfiledata.cFileName,L'_') + 1;
			int version = _wtoi(start);
			if (version <= 42 && version > iHighVersion)
				iHighVersion = version;
			more = FindNextFileW(hFind,&pfiledata);
		}

		FindClose(hFind);
		if (iHighVersion >= 24)
		{
			wchar_t finalD3DX9LibPath[MAX_PATH] = {0};
			StringCchPrintfW(finalD3DX9LibPath,MAX_PATH,L"%s%d%s",libPath,iHighVersion,L".dll");

			d3dx9 = LoadLibraryW(finalD3DX9LibPath);
		}
	}

	return d3dx9;
}

IVideoD3DOSD::IVideoD3DOSD(void)
{
	osdSprite = NULL;
	osdAtlasTexture = NULL;
	osdTimeFont = NULL;
	osdTitleFont = NULL;
	streaming = 0;
	titleFits = false;

	// Texture Src Coordinates for sprite images
	// Right and Bottom (last two) excluded from image
	SetRect(&osdBkgrndTextSrcCoords, 38, 534, 647, 635);

	SetRect(&osdPrevButtonNormalSrcCoords, 41, 17, 63, 31);
	SetRect(&osdPlayButtonNormalSrcCoords, 145, 14, 161, 35);
	SetRect(&osdPauseButtonNormalSrcCoords, 95, 16, 110, 33);
	SetRect(&osdStopButtonNormalSrcCoords, 195, 16, 210, 33);
	SetRect(&osdNextButtonNormalSrcCoords, 242, 17, 264, 31);
	SetRect(&osdProgressFrameNormalSrcCoords, 41, 226, 606, 235);		  
	SetRect(&osdVolumeFrameNormalSrcCoords, 41, 294, 111, 302);
	SetRect(&osdEndFSButtonNormalSrcCoords, 41, 140, 59, 158);
	SetRect(&osdMuteButtonNormalSrcCoords, 41, 416, 51, 428);
	SetRect(&osdProgressSliderNormalSrcCoords, 41, 343, 57, 361);
	SetRect(&osdVolumeSliderNormalSrcCoords, 41, 343, 57, 361);
	SetRect(&osdProgressProgressSrcCoords, 41, 274, 606, 282);  //hilited progress indicator
	SetRect(&osdVolumeProgressSrcCoords, 41, 314, 111, 322);    //hilited volume indicator

	SetRect(&osdPrevButtonClickSrcCoords, 41, 76, 63, 90);
	SetRect(&osdPlayButtonClickSrcCoords, 145, 73, 161, 94);
	SetRect(&osdPauseButtonClickSrcCoords, 95, 75, 110, 92);
	SetRect(&osdStopButtonClickSrcCoords, 195, 75, 210, 92);
	SetRect(&osdNextButtonClickSrcCoords, 242, 76, 264, 90);
	SetRect(&osdEndFSButtonClickSrcCoords, 41, 192, 59, 210);
	SetRect(&osdProgressSliderClickSrcCoords, 41, 385, 57, 403);
	SetRect(&osdVolumeSliderClickSrcCoords, 41, 385, 57, 403);

	SetRect(&osdPrevButtonDisabledSrcCoords, 41, 106, 63, 120);
	SetRect(&osdNextButtonDisabledSrcCoords, 242, 106, 264, 120);

	SetRect(&osdPrevButtonHiliteSrcCoords, 41, 46, 63, 60);
	SetRect(&osdPlayButtonHiliteSrcCoords, 145, 43, 161, 64);
	SetRect(&osdPauseButtonHiliteSrcCoords, 95, 45, 110, 62);
	SetRect(&osdStopButtonHiliteSrcCoords, 195, 45, 210, 62);
	SetRect(&osdNextButtonHiliteSrcCoords, 242, 46, 264, 60);
	SetRect(&osdEndFSButtonHiliteSrcCoords, 41, 166, 59, 184);
	SetRect(&osdProgressSliderHiliteSrcCoords, 41, 363, 57, 381);
	SetRect(&osdVolumeSliderHiliteSrcCoords, 41, 363, 57, 381);

	xScalingFactor = 1.0f; 
	yScalingFactor = 1.0f;

	for (int i = 0; i < 12; i++)
	{
		bState[i] = NORMAL;
	}

	mouseOver = NO_BUTTON;
	mouseLastOver = NO_BUTTON;
	mousePressed = NO_BUTTON;
	mouseLastPressed = NO_BUTTON;
	mouseDragging = false;

	displayTitle = NULL;
	marqueeTitleSrc = NULL;
	titleRestart = 0;
	dtFormat = 0;

	isInited = false;
	isReadyToDraw = false;
}

RECT IVideoD3DOSD::BuildHitRect(D3DXVECTOR3 position, RECT size)
{
	RECT hitRect;
	// casting float to long since I know the position vector will not be too big.
	hitRect.left = (long)position.x;
	hitRect.top = (long)position.y;
	hitRect.right = (long)position.x + size.right - size.left;
	hitRect.bottom = (long)position.y + size.bottom - size.top;
	return hitRect;
}

IVideoD3DOSD::~IVideoD3DOSD(void)
{
	if (osdSprite)
	{
		osdSprite->Release();
		osdSprite = NULL;
	}
	if (osdAtlasTexture)
	{
		osdAtlasTexture->Release();
		osdAtlasTexture = NULL;
	}
	if (marqueeTitleSrc)
	{
		delete [] marqueeTitleSrc;
		marqueeTitleSrc = NULL;
	}

	if (displayTitle)
	{
		delete [] displayTitle;
		displayTitle = NULL;
	}

	//if (d3dx_lib) 
	//{
	//	FreeLibrary(d3dx_lib);
	//	d3dx_lib = NULL;
	//}
}

void IVideoD3DOSD::SetScalingFactor(float fx, float fy)
{
	xScalingFactor = fx;
	yScalingFactor = fy;
}

void IVideoD3DOSD::CreateOSD(IDirect3DDevice9 * device)
{
	HRESULT hr;

	d3dx_lib = FindD3DX9();
	if (!d3dx_lib)
		return;

	pCreateFontW = (D3DXCREATEFONTW) GetProcAddress(d3dx_lib,"D3DXCreateFontW");
	pCreateSprite = (D3DXCREATESPRITE) GetProcAddress(d3dx_lib,"D3DXCreateSprite");
	pCreateTextureFromResourceExW = (D3DXCREATETEXTUREFROMRESOURCEEXW) GetProcAddress(d3dx_lib,"D3DXCreateTextureFromResourceExW");

	if (!pCreateFontW || !pCreateSprite || !pCreateTextureFromResourceExW)
		return;

	hr = pCreateSprite(device,&osdSprite);
	if (FAILED(hr))
	{
		DXTraceW(__FILE__, __LINE__, hr, L"CreateSprite Error", TRUE);
		return;
	}

	int font_size = -12 ;
	hr = pCreateFontW(
		device, 
		font_size, 
		0,		
		FW_NORMAL,
		1,                       
		0, 
		DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, 
		ANTIALIASED_QUALITY,	//DEFAULT_QUALITY, 
		DEFAULT_PITCH,
		L"Arial", 
		&osdTimeFont);
	if (FAILED(hr))
	{
		DXTraceW(__FILE__, __LINE__, hr, L"CreateFont (Time) Error", TRUE);
		return;
	}

	font_size = -16 ;
	hr = pCreateFontW(
		device, 
		font_size, 
		0,		
		FW_NORMAL,
		1,                       
		0, 
		DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, 
		ANTIALIASED_QUALITY,//DEFAULT_QUALITY, 
		DEFAULT_PITCH,
		L"Trebuchet MS", 
		&osdTitleFont);
	if (FAILED(hr))
	{
		DXTraceW(__FILE__, __LINE__, hr, L"CreateFont (Title) Error", TRUE);
		return;
	}

	ResetOSD(device);

	isInited = true;
}

void IVideoD3DOSD::UpdateOSD(HWND hWnd, VideoOutput *adjuster)
{
	// Position of sprites in screen coordinates
	// Center of sprite (where the position is mapped) is left to default to upper left corner
	// Note the Bkgrnd is positioned and then all other sprites are relative to that
	RECT clientRect;
	GetClientRect(hWnd,&clientRect);
	// Need to adjust the client rect to match the video aspect ration to fit the osd on the video.
	// adjuster->adjustAspect(clientRect);
	// width of the client area - width of the bkgrnd / 2 gives the space on each side
	// add that space to the offset of the left side of the client area.
	float xPosBkg = clientRect.left + 
		(((clientRect.right - clientRect.left) - (osdBkgrndTextSrcCoords.right - osdBkgrndTextSrcCoords.left))/2.0f);
	// width of the client area * .95 give the location of the bottom of the osd (i.e. 5% from bottom)
	// that minus the height of the bkgrnd gives the location of the upper left of background
	// add that space to the offset of the top of the client area.
	float yPosBkg = clientRect.top +
		(((clientRect.bottom - clientRect.top) * 1.0f) - (osdBkgrndTextSrcCoords.bottom - osdBkgrndTextSrcCoords.top));
	osdBkgrndPosition = D3DXVECTOR3(floor(xPosBkg), floor(yPosBkg), 0.0f);

	osdPrevButtonPosition = osdBkgrndPosition + D3DXVECTOR3(191.0f, 75.0f, 0.0f);
	osdPlayButtonPosition = osdBkgrndPosition + D3DXVECTOR3(246.0f, 72.0f, 0.0f);
	osdPauseButtonPosition = osdBkgrndPosition + D3DXVECTOR3(296.0f, 74.0f, 0.0f);
	osdStopButtonPosition = osdBkgrndPosition + D3DXVECTOR3(345.5f, 74.0f, 0.0f);
	osdNextButtonPosition = osdBkgrndPosition + D3DXVECTOR3(392.5f, 75.0f, 0.0f);
	osdProgressFramePosition = osdBkgrndPosition + D3DXVECTOR3(22.0f, 49.0f, 0.0f);
	osdVolumeFramePosition = osdBkgrndPosition + D3DXVECTOR3(518.0f, 76.0f, 0.0f);
	osdEndFSButtonPosition = osdBkgrndPosition + D3DXVECTOR3(583.0f, 19.0f, 0.0f);
	osdMuteButtonPosition = osdVolumeFramePosition + D3DXVECTOR3(-15.0f, -1.0f, 0.0f);
	osdProgressSliderPosition = osdProgressFramePosition + D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	osdVolumeSliderPosition = osdVolumeFramePosition + D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	SetRect(&osdTimeRect,
		(long)osdBkgrndPosition.x + 26,
		(long)osdBkgrndPosition.y + 76,
		(long)osdBkgrndPosition.x + 98, 
		(long)osdBkgrndPosition.y + 85);

	SetRect(&osdTitleRect,
		(long)osdBkgrndPosition.x + 26,
		(long)osdBkgrndPosition.y + 17,
		(long)osdBkgrndPosition.x + 503, 
		(long)osdBkgrndPosition.y + 37);

	// Create Hit Test Rects for ui elements that don't move
	osdPrevButtonHit = BuildHitRect(osdPrevButtonPosition, osdPrevButtonNormalSrcCoords );
	osdPlayButtonHit = BuildHitRect(osdPlayButtonPosition, osdPlayButtonNormalSrcCoords );
	osdPauseButtonHit = BuildHitRect(osdPauseButtonPosition, osdPauseButtonNormalSrcCoords );
	osdStopButtonHit = BuildHitRect(osdStopButtonPosition, osdStopButtonNormalSrcCoords );
	osdNextButtonHit = BuildHitRect(osdNextButtonPosition, osdNextButtonNormalSrcCoords );
	osdEndFSButtonHit = BuildHitRect(osdEndFSButtonPosition, osdEndFSButtonNormalSrcCoords);
	osdProgressFrameHit = BuildHitRect(osdProgressFramePosition, osdProgressFrameNormalSrcCoords);
	osdVolumeFrameHit = BuildHitRect(osdVolumeFramePosition, osdVolumeFrameNormalSrcCoords);

	streaming = (in_getlength() < 0) || !in_mod || !in_mod->is_seekable;
	if (streaming)
	{
		bState[PREV_BUTTON] = DISABLED;
		bState[NEXT_BUTTON] = DISABLED;
		bState[PROGRESS_FRAME] = DISABLED;
		bState[PROGRESS_SLIDER] = DISABLED;
	}

	// Find out if the title will fit in the UI space for it
	RECT tempTitleRect = osdTitleRect;
	osdTitleFont->DrawTextW(NULL, FileTitle, -1, &tempTitleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_CALCRECT, D3DCOLOR_XRGB(255,255,255));
	if (tempTitleRect.right <= osdTitleRect.right)
	{
		// The title fits, just use it
		titleFits = true;
		displayTitle = FileTitle;
		dtFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP;
	} else 
	{
		// title will not fit, we need to set up a marquee.
		//
		// a string with two copies of the title makes it easier to process
		// sizeNeeded, in chars, includes one space, five dots, 
		//       one space and 1 extra for null.
		size_t sizeNeeded = (lstrlenW(FileTitle)*2) + 8;
		marqueeTitleSrc = new wchar_t[sizeNeeded];
		displayTitle = new wchar_t[sizeNeeded];
		titleRestart = lstrlenW(FileTitle);
		StringCchPrintfW(marqueeTitleSrc, sizeNeeded, L"%s ..... %s", FileTitle, FileTitle);
		titleFits = false;
		dtFormat = DT_RIGHT | DT_TOP | DT_SINGLELINE;
	}

	isReadyToDraw = true;
}
#ifdef _DEBUG
#define DRAW_OSD_SET_ERROR(x) draw_osd_error=x
#else
#define DRAW_OSD_SET_ERROR(x)
#endif

void IVideoD3DOSD::DrawOSD(IDirect3DDevice9 * device)
{
	HRESULT hr;
	D3DXVECTOR3 sliderCenter(8.0f, 8.0f, 0.0f);

	const wchar_t *draw_osd_error;
	hr = osdSprite->Begin(D3DXSPRITE_ALPHABLEND);
	if (FAILED(hr))
	{

		DXTraceW(__FILE__, __LINE__, hr, L"Sprite Begin Error", TRUE);
		return ;
	}

	// Doing Scaling of sprites here
	// If we do translations and/or rotations we'll have to do a more 
	// robust hit test (picking) since the current one assumes a rectangular 
	// shape based on screen coordinates.  Only scaling is currently handled
	// in the hit test.
	//D3DXMATRIX scalingMatrix;
	//osdSprite->SetTransform(D3DXMatrixScaling(&scalingMatrix, 1.0f /*xScalingFactor*/, 1.0f /*yScalingFactor*/, 0.0f));

	hr = osdSprite->Draw(osdAtlasTexture, &osdBkgrndTextSrcCoords, NULL, &osdBkgrndPosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Background Sprite Draw Error");
		goto DrawOSD_Error;
	}
	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(PREV_BUTTON), NULL, &osdPrevButtonPosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Prev Button Sprite Draw Error");
		goto DrawOSD_Error;
	}
	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(PLAY_BUTTON), NULL, &osdPlayButtonPosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr)) 
	{
		DRAW_OSD_SET_ERROR(L"Play Button Sprite Draw Error");
		goto DrawOSD_Error;
	}
	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(PAUSE_BUTTON), NULL, &osdPauseButtonPosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Pause Button Sprite Draw Error");
		goto DrawOSD_Error;
	}
	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(STOP_BUTTON), NULL, &osdStopButtonPosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Stop Button Sprite Draw Error");
		goto DrawOSD_Error;
	}
	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(NEXT_BUTTON), NULL, &osdNextButtonPosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Next Button Sprite Draw Error");
		goto DrawOSD_Error;
	}
	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(PROGRESS_FRAME), NULL, &osdProgressFramePosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Progress Frame Sprite Draw Error");
		goto DrawOSD_Error;
	}
	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(VOLUME_FRAME), NULL, &osdVolumeFramePosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Volume Frame Sprite Draw Error");
		goto DrawOSD_Error;
	}
	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(ENDFS_BUTTON), NULL, &osdEndFSButtonPosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"EndFS Button Sprite Draw Error");
		goto DrawOSD_Error;
	}
	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(MUTE_BUTTON), NULL, &osdMuteButtonPosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Mute Button Sprite Draw Error");
		goto DrawOSD_Error;
	}

	if (playing && !streaming && !mouseDragging) // if mouseDragging we may be repositioning the slider, don't set it back till Lmouseup
	{
		// calculate the relative position of the slider
		float ppercent = (in_getouttime() / 1000.0f) / in_getlength();
		float sizeOfProgFrame = (float)osdProgressFrameHit.right - osdProgressFrameHit.left - 0;
		// position the progress slider
		osdProgressSliderPosition.x = osdProgressFramePosition.x + (sizeOfProgFrame * ppercent);
		// Now build the hit rect based on the new position.
		osdProgressSliderHit = BuildHitRect(osdProgressSliderPosition + D3DXVECTOR3(0.0f,0.0f,0.0f), osdProgressSliderNormalSrcCoords);
	}


	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(PROGRESS_SLIDER), &sliderCenter, &osdProgressSliderPosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Progress Slider Sprite Draw Error");
		goto DrawOSD_Error;
	}

	// Build the progress hilite line by drawing only a certain amount (width) of the texture.
	RECT seekProgress;
	// The progress hilite line goes on top of progress frame
	seekProgress = osdProgressProgressSrcCoords;
	// The width of the progress hilite line is determined by the location of the slider
	seekProgress.right = seekProgress.left + (osdProgressSliderHit.left - osdProgressFrameHit.left + 0);

	hr = osdSprite->Draw(osdAtlasTexture, 
		&seekProgress,
		NULL, 
		&osdProgressFramePosition, 
		D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Seek Progress Sprite Draw Error");
		goto DrawOSD_Error;
	}

	if (!mouseDragging) // if mouseDragging we may be repositioning the slider, don't set it back till Lmouseup
	{
		// calculate the relative position of the slider
		float vpercent = config_volume / 255.0f;
		float sizeOfVolFrame = (float)osdVolumeFrameHit.right - osdVolumeFrameHit.left - 0;
		// position the volume slider
		osdVolumeSliderPosition.x = (osdVolumeFramePosition.x) + (sizeOfVolFrame * vpercent);
		// Now build the hit rect based on the new position.
		osdVolumeSliderHit = BuildHitRect(osdVolumeSliderPosition + D3DXVECTOR3(0.0f,0.0f,0.0f), osdVolumeSliderNormalSrcCoords);
	}
	hr = osdSprite->Draw(osdAtlasTexture, GetTextCoords(VOLUME_SLIDER), &sliderCenter, &osdVolumeSliderPosition, D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Volume SLider Sprite Draw Error");
		goto DrawOSD_Error;
	}

	// Build the volume hilite line by drawing only a certain amount (width) of the texture.
	RECT volProgress;
	// The volume hilite line goes on top of volume frame
	volProgress = osdVolumeProgressSrcCoords;
	// The width of the volume hilite line is determined by the location of the slider
	volProgress.right = volProgress.left + (osdVolumeSliderHit.left - osdVolumeFrameHit.left + 0);

	hr = osdSprite->Draw(osdAtlasTexture, 
		&volProgress,
		NULL, 
		&osdVolumeFramePosition, 
		D3DCOLOR_XRGB(255,255,255));
	if (FAILED(hr))
	{
		DRAW_OSD_SET_ERROR(L"Volume Progress Sprite Draw Error");
		goto DrawOSD_Error;
	}


	if (osdTimeFont)
	{
		int seconds_in = in_getouttime() / 1000;
		int time_to_go = in_getlength();

		wchar_t timerText[256] = {0};
		if (streaming)
			StringCbPrintfW(timerText,sizeof(timerText),L"%.2u:%.2u",seconds_in /60,seconds_in % 60);
		else
			StringCbPrintfW(timerText,sizeof(timerText),L"%.2u:%.2u / %.2u:%.2u",seconds_in /60,seconds_in % 60,time_to_go / 60, time_to_go % 60);
		osdTimeFont->DrawTextW(osdSprite, timerText, -1, &osdTimeRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP, D3DCOLOR_XRGB(163,164,167)); // #A3A4A7
	}

	if (osdTitleFont)
	{
		// found possibility that user can pause in full screen and remove items from playlist
		// This prevents a crash by not trying to display a title.
		if (lstrlenW(FileTitle) > 0)
		{
			if (!titleFits)
			{
				// title does not fit, build marquee
				DWORD now = GetTickCount();
				static DWORD then;
				if (now - then > 250)  // slow it down so people can read it.
				{
					static int charCount = 2; // start with the first char + 1 for null
					lstrcpynW(displayTitle,marqueeTitleSrc,charCount);
					charCount++;
					if (charCount > lstrlenW(marqueeTitleSrc))
						charCount = lstrlenW(FileTitle);
					then = now;
				}
			}

			osdTitleFont->DrawTextW(osdSprite, displayTitle, -1, &osdTitleRect, dtFormat, D3DCOLOR_XRGB(204,204,204)); // #cccccc
		}
	}

	hr = osdSprite->End();
	if (FAILED(hr))
	{
		DXTraceW(__FILE__, __LINE__, hr, L"Sprite End Error", TRUE);
		return;
	}

	return;
DrawOSD_Error:
	DXTraceW(__FILE__, __LINE__, hr, draw_osd_error, TRUE);
	osdSprite->End();
}

RECT *IVideoD3DOSD::GetTextCoords(UI_ELEM item)
{ 
	switch (item) 
	{
	case PREV_BUTTON :
		switch (bState[item]) 
		{
		case NORMAL :
			return &osdPrevButtonNormalSrcCoords;
			break;
		case CLICKED :
			return &osdPrevButtonClickSrcCoords;
			break;
		case HILITE :
			return &osdPrevButtonHiliteSrcCoords;
			break;
		case DISABLED :
			return &osdPrevButtonDisabledSrcCoords;
			break;
		}

		break;
	case PLAY_BUTTON :
		switch (bState[item]) 
		{
		case NORMAL :
		case DISABLED :
			return &osdPlayButtonNormalSrcCoords;
			break;
		case CLICKED :
			return &osdPlayButtonClickSrcCoords;
			break;
		case HILITE :
			return &osdPlayButtonHiliteSrcCoords;
			break;
		}

		break;
	case PAUSE_BUTTON :
		switch (bState[item]) 
		{
		case NORMAL :
		case DISABLED :
			return &osdPauseButtonNormalSrcCoords;
			break;
		case CLICKED :
			return &osdPauseButtonClickSrcCoords;
			break;
		case HILITE :
			return &osdPauseButtonHiliteSrcCoords;
			break;
		}

		break;
	case STOP_BUTTON :
		switch (bState[item]) 
		{
		case NORMAL :
		case DISABLED :
			return &osdStopButtonNormalSrcCoords;
			break;
		case CLICKED :
			return &osdStopButtonClickSrcCoords;
			break;
		case HILITE :
			return &osdStopButtonHiliteSrcCoords;
			break;
		}

		break;
	case NEXT_BUTTON :
		switch (bState[item]) 
		{
		case NORMAL :
			return &osdNextButtonNormalSrcCoords;
			break;
		case CLICKED :
			return &osdNextButtonClickSrcCoords;
			break;
		case HILITE :
			return &osdNextButtonHiliteSrcCoords;
			break;
		case DISABLED :
			return &osdNextButtonDisabledSrcCoords;
			break;
		}

		break;
	case ENDFS_BUTTON :
		switch (bState[item]) 
		{
		case NORMAL :
		case DISABLED :
			return &osdEndFSButtonNormalSrcCoords;
			break;
		case CLICKED :
			return &osdEndFSButtonClickSrcCoords;
			break;
		case HILITE :
			return &osdEndFSButtonHiliteSrcCoords;
			break;
		}

		break;
	case MUTE_BUTTON :
		return &osdMuteButtonNormalSrcCoords;
		//switch (bState[item]) 
		//{
		//case NORMAL :
		//case DISABLED :
		//	return &osdMuteButtonNormalSrcCoords;
		//	break;
		//case CLICKED :
		//	return &osdMuteButtonClickSrcCoords;
		//	break;
		//case HILITE :
		//	return &osdMuteButtonHiliteSrcCoords;
		//	break;
		//}

		break;
	case PROGRESS_FRAME :
		return &osdProgressFrameNormalSrcCoords;

		break;
	case VOLUME_FRAME :
		return &osdVolumeFrameNormalSrcCoords;

		break;
	case PROGRESS_SLIDER :
		switch (bState[item]) 
		{
		case NORMAL :
		case DISABLED :
			return &osdProgressSliderNormalSrcCoords;
			break;
		case CLICKED :
			return &osdProgressSliderClickSrcCoords;
			break;
		case HILITE :
			return &osdProgressSliderHiliteSrcCoords;
			break;
		}

		break;
	case VOLUME_SLIDER :
		switch (bState[item]) 
		{
		case NORMAL :
		case DISABLED :
			return &osdVolumeSliderNormalSrcCoords;
			break;
		case CLICKED :
			return &osdVolumeSliderClickSrcCoords;
			break;
		case HILITE :
			return &osdVolumeSliderHiliteSrcCoords;
			break;
		}

		break;
	}

	return NULL;
}

void IVideoD3DOSD::LostOSD()
{
	if (osdSprite)
		osdSprite->OnLostDevice();
	if (osdTimeFont)
		osdTimeFont->OnLostDevice();
	if (osdTitleFont)
		osdTitleFont->OnLostDevice();
	if (osdAtlasTexture)
	{
		osdAtlasTexture->Release();
		osdAtlasTexture = 0;
	}
}

void IVideoD3DOSD::ResetOSD(IDirect3DDevice9 * device)
{
	if (osdSprite)
		osdSprite->OnResetDevice();
	if (osdTimeFont)
		osdTimeFont->OnResetDevice();
	if (osdTitleFont)
		osdTitleFont->OnResetDevice();

	if (device)
	{
		HRESULT hr;
		hr = pCreateTextureFromResourceExW(
			device,
			NULL,	// HMODULE
			MAKEINTRESOURCEW(IDB_OSD),			// Our texture image atlas
			D3DX_DEFAULT,				// width
			D3DX_DEFAULT,				// height
			1,							// MIP levels
			0,							// usage
			D3DFMT_UNKNOWN,				// get format from file
			D3DPOOL_DEFAULT,			// mem pool
			D3DX_DEFAULT,				// filter
			D3DX_DEFAULT,				// MIP filter
			0,							// transparent color key
			NULL,						// image info struct
			NULL,						// palette
			&osdAtlasTexture);				// the returned texture, if success
		if (FAILED(hr))
		{
			DXTraceW(__FILE__, __LINE__, hr, L"CreateTextureFromFileEx Error", TRUE);
			return;
		}
	}

}

bool IVideoD3DOSD::MouseDown(int xpt, int ypt, WPARAM wParam)
{
	Show();	
	// mouseLastPressed is used during mouse up to verify that the up is on the 
	// same UI element as the mouse down.
	mouseLastPressed = HitTest((float)xpt, (float)ypt);
	bState[mouseLastPressed] = CLICKED;
	return false;
}
bool IVideoD3DOSD::MouseMove(int ixpt, int iypt, WPARAM wParam)
{
	static int saved_ixpt;
	static int saved_iypt;

	// Need to check whether the mouse cursor is still in the same place.
	// Evidently, WM_MOUSEMOVE can get triggered for other reasons than
	// actually moving the mouse, per Microsoft blogs
	// This code was triggering with IM and EMAIL notifications without
	// moving the mouse.
	if (ixpt == saved_ixpt && iypt == saved_iypt)
		return false;

	saved_ixpt = ixpt;
	saved_iypt = iypt;

	Show();	

	// Change input ints to floats so later calculations are more precise.
	float xpt = (float)ixpt;
	float ypt = (float)iypt;

	mouseOver = HitTest((float)xpt, (float)ypt);
	if (wParam & MK_LBUTTON) //dragging
	{
		mouseDragging = true;
		if (mouseLastPressed == VOLUME_SLIDER)
		{
			if (xpt < (osdVolumeFrameHit.left)) xpt = (float) osdVolumeFrameHit.left;
			else if (xpt > (osdVolumeFrameHit.right) - 0) xpt = (float) osdVolumeFrameHit.right - 0;

			//move the volume slider
			osdVolumeSliderPosition.x = xpt; 
			// slider uses center as center
			osdVolumeSliderHit = BuildHitRect(osdVolumeSliderPosition + D3DXVECTOR3(0.0f,0.0f,0.0f),osdVolumeSliderNormalSrcCoords);
		}
		else if (mouseLastPressed == PROGRESS_SLIDER)
		{			
			if (xpt < osdProgressFrameHit.left) xpt = (float)osdProgressFrameHit.left;
			else if (xpt > (osdProgressFrameHit.right)) xpt = (float)osdProgressFrameHit.right;

			//move the progress slider
			osdProgressSliderPosition.x = xpt; 
			osdProgressSliderHit = BuildHitRect(osdProgressSliderPosition + D3DXVECTOR3(0.0f,0.0f,0.0f),osdProgressSliderNormalSrcCoords);
		}
	} else // no click, just mousemove 
	{
		mouseDragging = false;
		if (mouseLastOver != mouseOver)
		{
			if (bState[mouseLastOver] == HILITE) 
				bState[mouseLastOver] = NORMAL;
			if (bState[mouseOver] == NORMAL) 
				bState[mouseOver] = HILITE;
			mouseLastOver = mouseOver;
		}
	}

	return false;
}

bool IVideoD3DOSD::MouseUp(int xpt, int ypt, WPARAM wParam)
{
	mousePressed = HitTest((float)xpt, (float)ypt);

	bState[mouseLastPressed] = NORMAL;

	if (bState[mousePressed] == HILITE) 
		bState[mousePressed] = NORMAL;

	mouseDragging = false;

	switch (mousePressed)
	{
	case ENDFS_BUTTON :
		if (mouseLastPressed == ENDFS_BUTTON)
			return true;  // end full screen
		break;
	case PREV_BUTTON : 
		if (mouseLastPressed == PREV_BUTTON)
			PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON1, 0);
		break;
	case PLAY_BUTTON :
		if (mouseLastPressed == PLAY_BUTTON)
			PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON2, 0);
		break;
	case PAUSE_BUTTON :
		if (mouseLastPressed == PAUSE_BUTTON)
			PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON3, 0);
		break;
	case STOP_BUTTON :
		if (mouseLastPressed == STOP_BUTTON)
			PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON4, 0);
		break;
	case NEXT_BUTTON :
		if (mouseLastPressed == NEXT_BUTTON)
			PostMessageW(hMainWindow, WM_COMMAND, WINAMP_BUTTON5, 0);
		break;
	default :
		{
			// If not a button, check the sliders
			// The successful use of the sliders should not depend on 
			// releasing the mouse inside the frame, which may be very small.
			switch (mouseLastPressed)
			{
			case PROGRESS_SLIDER :
			case PROGRESS_FRAME :
				{
					float xIntoFrame = (float)xpt - osdProgressFrameHit.left;
					// -8 is half the width of the slider
					float rightMaxOfFrame = (float)osdProgressFrameHit.right - 0;
					float leftMinOfFrame = (float)osdProgressFrameHit.left;
					float sizeOfFrame = rightMaxOfFrame - leftMinOfFrame;
					float t = xIntoFrame / sizeOfFrame;
					if (t < 0) 
						t = 0;
					if (t > 1) 
						t = 1;

					int len = in_getlength();			
					in_seek((int)(t*len*1000));
				}
				break;
			case VOLUME_SLIDER :
			case VOLUME_FRAME :
				{
					float xIntoFrame      = (float)xpt - (osdVolumeFrameHit.left);
					// -8 is half the width of the slider
					float rightMaxOfFrame = (float)osdVolumeFrameHit.right - 0;
					float leftMinOfFrame  = (float)osdVolumeFrameHit.left;
					float sizeOfFrame     = rightMaxOfFrame - leftMinOfFrame;
					float t               = xIntoFrame / sizeOfFrame;

					if (t < 0) 
						t = 0;
					if (t > 1) 
						t = 1;

					unsigned char v = (unsigned char)(t * 255);

					config_volume = v;

					in_setvol(v);
				}
				break;
			}
		}
		break;
	}
	mouseLastPressed = NO_BUTTON;
	return false;
}

IVideoD3DOSD::UI_ELEM IVideoD3DOSD::HitTest(float xpt, float ypt)
{
	if (PointInRect(xpt, ypt, osdPrevButtonHit))
		return PREV_BUTTON;
	else if (PointInRect(xpt, ypt, osdPlayButtonHit))
		return PLAY_BUTTON;
	else if (PointInRect(xpt, ypt, osdPauseButtonHit))
		return PAUSE_BUTTON;
	else if (PointInRect(xpt, ypt, osdStopButtonHit))
		return STOP_BUTTON;
	else if (PointInRect(xpt, ypt, osdNextButtonHit))
		return NEXT_BUTTON;
	else if (PointInRect(xpt, ypt, osdEndFSButtonHit))
		return ENDFS_BUTTON;
	else if (PointInRect(xpt, ypt, osdVolumeSliderHit))
		return VOLUME_SLIDER;
	else if (PointInRect(xpt, ypt, osdProgressSliderHit))
		return PROGRESS_SLIDER;
	else if (PointInRect(xpt, ypt, osdProgressFrameHit))
		return PROGRESS_FRAME;
	else if (PointInRect(xpt, ypt, osdVolumeFrameHit))
		return VOLUME_FRAME;
	else
		return NO_BUTTON;
}

bool IVideoD3DOSD::PointInRect(float x, float y, RECT testRect)
{
	if ((x >= testRect.left) && (x <= testRect.right) &&
		(y >= testRect.top) && (y <= testRect.bottom))
		return true;
	else
		return false;
}
