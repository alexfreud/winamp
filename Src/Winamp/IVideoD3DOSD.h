#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>
#include "videoosd.h"
#include "videooutput.h"
#include "resource.h"

extern HWND hMainWindow;


class IVideoD3DOSD :
	public IVideoOSD
{
public:
	IVideoD3DOSD(void);
	~IVideoD3DOSD(void);

	enum UI_ELEM
	{
		NO_BUTTON,
		PREV_BUTTON,
		PLAY_BUTTON,
		PAUSE_BUTTON,
		STOP_BUTTON,
		NEXT_BUTTON,
		ENDFS_BUTTON,
		MUTE_BUTTON,
		PROGRESS_FRAME,
		VOLUME_FRAME,
		PROGRESS_SLIDER,
		VOLUME_SLIDER
	};

	enum BUTTON_STATE 
	{
	NORMAL,
	CLICKED,
	HILITE,
	DISABLED
	};

	void CreateOSD(IDirect3DDevice9 * device);
	void UpdateOSD(HWND hWnd, VideoOutput *adjuster);
	void DrawOSD(IDirect3DDevice9 * device);
	void LostOSD();
	void ResetOSD(IDirect3DDevice9 *device);
	UI_ELEM HitTest(float x, float y);
	bool MouseDown(int xpt, int ypt, WPARAM wParam);
	bool MouseMove(int xpt, int ypt, WPARAM wParam);
	bool MouseUp(int xpt, int ypt, WPARAM wParam);
	void SetScalingFactor(float x, float y);
	bool isOSDInited(){return isInited;}
	bool isOSDReadyToDraw(){return isReadyToDraw;};

protected:
	ID3DXSprite *osdSprite;
	IDirect3DTexture9 *osdAtlasTexture;
	ID3DXFont *osdTimeFont;
	ID3DXFont *osdTitleFont;

	// Texture Src Coordinates for sprite images
	// Right and Bottom (last two) excluded from image
	RECT osdPrevButtonNormalSrcCoords;
	RECT osdPlayButtonNormalSrcCoords;
	RECT osdPauseButtonNormalSrcCoords;
	RECT osdStopButtonNormalSrcCoords;
	RECT osdNextButtonNormalSrcCoords;
	RECT osdProgressFrameNormalSrcCoords;
	RECT osdVolumeFrameNormalSrcCoords;
	RECT osdEndFSButtonNormalSrcCoords;
	RECT osdMuteButtonNormalSrcCoords;
	RECT osdProgressSliderNormalSrcCoords;
	RECT osdVolumeSliderNormalSrcCoords;
	RECT osdProgressProgressSrcCoords;
	RECT osdVolumeProgressSrcCoords;

	RECT osdPrevButtonClickSrcCoords;
	RECT osdPlayButtonClickSrcCoords;
	RECT osdPauseButtonClickSrcCoords;
	RECT osdStopButtonClickSrcCoords;
	RECT osdNextButtonClickSrcCoords;
	RECT osdEndFSButtonClickSrcCoords;
	RECT osdProgressSliderClickSrcCoords;
	RECT osdVolumeSliderClickSrcCoords;

	RECT osdPrevButtonDisabledSrcCoords;
	RECT osdNextButtonDisabledSrcCoords;
//	RECT osdProgressFrameDisabledSrcCoords;
//	RECT osdProgressSliderDisabledSrcCoords;

	RECT osdPrevButtonHiliteSrcCoords;
	RECT osdPlayButtonHiliteSrcCoords;
	RECT osdPauseButtonHiliteSrcCoords;
	RECT osdStopButtonHiliteSrcCoords;
	RECT osdNextButtonHiliteSrcCoords;
//	RECT osdProgressFrameHiliteSrcCoords;
//	RECT osdVolumeFrameHiliteSrcCoords;
	RECT osdEndFSButtonHiliteSrcCoords;
	RECT osdProgressSliderHiliteSrcCoords;
	RECT osdVolumeSliderHiliteSrcCoords;

	RECT osdBkgrndTextSrcCoords;
	RECT osdTimeRect;
	RECT osdTitleRect;

	// Position of sprites in screen coordinates
	// Center of sprite (where the position is mapped) is left to default to upper left corner
    // except for progress and volume sliders, which are mapped to their center
	// Note the Bkgrnd is positioned and then all other sprites are relative to that
	D3DXVECTOR3 osdBkgrndPosition;
	D3DXVECTOR3 osdPrevButtonPosition;
	D3DXVECTOR3 osdPlayButtonPosition;
	D3DXVECTOR3 osdPauseButtonPosition;
	D3DXVECTOR3 osdStopButtonPosition;
	D3DXVECTOR3 osdNextButtonPosition;
	D3DXVECTOR3 osdProgressFramePosition;
	D3DXVECTOR3 osdVolumeFramePosition;
	D3DXVECTOR3 osdEndFSButtonPosition;
	D3DXVECTOR3 osdMuteButtonPosition;
	D3DXVECTOR3 osdProgressSliderPosition;
	D3DXVECTOR3 osdVolumeSliderPosition;

	// Hit test rects for buttons
	RECT osdPrevButtonHit;
	RECT osdPlayButtonHit;
	RECT osdPauseButtonHit;
	RECT osdStopButtonHit;
	RECT osdNextButtonHit;
	RECT osdEndFSButtonHit;
	RECT osdProgressFrameHit;
	RECT osdVolumeFrameHit;
	RECT osdProgressSliderHit;
	RECT osdVolumeSliderHit;

	float xScalingFactor; 
	float yScalingFactor;
	BUTTON_STATE bState[12];	// bState[0] is for NO_BUTTON
	bool streaming;
	wchar_t *displayTitle;		// title displayed in osd UI
	wchar_t *marqueeTitleSrc;   // temp string used to create displayTitle if title does not fit.
	size_t titleRestart;		// location in title to loop back to for marquee effect 
	bool titleFits;				// indicates whether the title will fit in the UI title field
	DWORD dtFormat;				// format of title text rect based on title size, i.e., center or left justified

	UI_ELEM mouseOver;
	UI_ELEM mouseLastOver;
	UI_ELEM mousePressed;
	UI_ELEM mouseLastPressed;	// used to verify that the LMouse up event matches the LMouse down
	bool mouseDragging;			// whether dragging is in progress to decide to update from winamp info

	bool isInited;				// has run CreateOSD to create all the d3d objects
	bool isReadyToDraw;			// has run UpdateOSD to init OSD for drawing, i.e., positioning the UI elements

	bool PointInRect(float x, float y, RECT testRect);
	RECT BuildHitRect(D3DXVECTOR3 position, RECT size);
	RECT * GetTextCoords(UI_ELEM item);
};
