#include "main.h"
#include "./toolbarProgress.h"
#include "./toolbar.h"
#include "./graphics.h"
#include "./resource.h"
#include "../Plugins/General/gen_ml/ml_ipc_0313.h"
#include "./ifc_imageloader.h"
#include <strsafe.h>

#define PROGRESS_FRAMECOUNT		9

static ATOM TOOLBARPROGRESS_PROP = 0;
#define TIMER_PROGRESSSTEP_ID			111
#define TIMER_PROGRESSSTEP_INTERVAL		120


ToolbarProgress::ToolbarProgress(LPCSTR pszName, UINT nStyle, LPCWSTR pszText, LPCWSTR pszDescription) 
	: ToolbarItem(pszName, nStyle, ICON_NONE, pszText, pszDescription), 
		bitmap(NULL), frame(0), hTimer(NULL)

{
}

ToolbarProgress::~ToolbarProgress()
{	
	if (NULL != hTimer)
		Animate(hTimer, FALSE);
	
	if (NULL != bitmap)
		DeleteObject(bitmap);
}

static void CALLBACK ToolbarProgress_TimerElapsed(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD dwTime)
{
	ToolbarProgress *item = (ToolbarProgress*)GetProp(hwnd, MAKEINTATOM(TOOLBARPROGRESS_PROP));
	if (NULL == item || 0 != ((ToolbarItem::stateDisabled | ToolbarItem::stateHidden) & item->GetStyle()))
	{
		if (NULL != item)
			item->Animate(hwnd, FALSE);
		else
			KillTimer(hwnd, TIMER_PROGRESSSTEP_ID);
		return;
	}
	item->Animate(hwnd, TRUE);
}


ToolbarItem* CALLBACK ToolbarProgress::CreateInstance(ToolbarItem::Template *item)
{
	if (NULL == item) 
		return NULL;

	return new ToolbarProgress( (NULL != item->name) ? item->name : TOOLCLS_PROGRESS,
							item->style | styleNoChevron,
							item->text, 
							item->description);
}

void ToolbarProgress::SetStyle(HWND hToolbar, UINT newStyle, UINT styleMask)
{
	__super::SetStyle(hToolbar, newStyle, styleMask);

	if (0 == ((stateDisabled | stateHidden) & style))
	{
		Animate(hToolbar, TRUE);
	}
}

BOOL ToolbarProgress::Animate(HWND hToolbar, BOOL fAnimate)
{
	if (FALSE == fAnimate)
	{
		if (NULL != hTimer)
		{
			RemoveProp(hTimer, MAKEINTATOM(TOOLBARPROGRESS_PROP));
			KillTimer(hTimer, TIMER_PROGRESSSTEP_ID);
			hTimer = NULL;
		}

		if (0 != frame)
		{
			frame = 0;
			InvalidateRect(hToolbar, &rect, FALSE);
		}

		
	}
	else
	{
		if (NULL == hTimer)
		{
			if (0 == TOOLBARPROGRESS_PROP)
			{
				TOOLBARPROGRESS_PROP = GlobalAddAtom(L"omToolbarProgressProp");
				if (0 == TOOLBARPROGRESS_PROP) return FALSE;
			}

			if (!SetProp(hToolbar, MAKEINTATOM(TOOLBARPROGRESS_PROP), (HANDLE)this))
				return FALSE;
	
			if (!SetTimer(hToolbar, TIMER_PROGRESSSTEP_ID, TIMER_PROGRESSSTEP_INTERVAL,
					ToolbarProgress_TimerElapsed))
			{
				RemoveProp(hToolbar, MAKEINTATOM(TOOLBARPROGRESS_PROP));
				return FALSE;

			}
			hTimer = hToolbar;
		}

		if (++frame >= PROGRESS_FRAMECOUNT)
		{
			frame = 1;
		}
		InvalidateRect(hToolbar, &rect, FALSE);
		
	}

	return TRUE;
}

void ToolbarProgress::UpdateSkin(HWND hToolbar)
{
	
	if (NULL != bitmap)
	{
		DeleteObject(bitmap);
		bitmap = NULL;
	}
				
	ifc_omimageloader *loader;
	
	if (SUCCEEDED(Plugin_QueryImageLoader(Plugin_GetInstance(), MAKEINTRESOURCE(IDR_TOOLBARPROGRESS_IMAGE), TRUE, &loader)))
	{
		BITMAPINFOHEADER headerInfo;
		BYTE *pixelData;
		if (SUCCEEDED(loader->LoadBitmapEx(&bitmap, &headerInfo, (void**)&pixelData)))
		{
			if (headerInfo.biHeight < 0) headerInfo.biHeight = -headerInfo.biHeight;
			
			Image_Colorize(pixelData, headerInfo.biWidth, headerInfo.biHeight, headerInfo.biBitCount, 
				Toolbar_GetBkColor(hToolbar), Toolbar_GetFgColor(hToolbar), TRUE);
		}
		loader->Release();
	}

}

BOOL ToolbarProgress::AdjustRect(HWND hToolbar, RECT *proposedRect)
{
	BITMAP bm;
	if (NULL == bitmap || sizeof(BITMAP) != GetObject(bitmap, sizeof(BITMAP), &bm))
		return FALSE;

	if (bm.bmHeight < 0) bm.bmHeight = -bm.bmHeight;
	bm.bmHeight /= PROGRESS_FRAMECOUNT;
	proposedRect->right = proposedRect->left + bm.bmWidth;
	proposedRect->top += (((proposedRect->bottom - proposedRect->top) - bm.bmHeight)/2);
	proposedRect->bottom = proposedRect->top + bm.bmHeight;

	return TRUE;
}

BOOL ToolbarProgress::Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state)
{	
	BITMAP bm;
	if (NULL == bitmap || sizeof(BITMAP) != GetObject(bitmap, sizeof(BITMAP), &bm))
		return FALSE;
	
	if (bm.bmHeight < 0) bm.bmHeight = -bm.bmHeight;
	bm.bmHeight /= PROGRESS_FRAMECOUNT;
	
	RECT blitRect;
	if (!IntersectRect(&blitRect, paintRect))
		return TRUE;
	
	BOOL success = FALSE;
	HDC hdcSrc = CreateCompatibleDC(hdc);

	if (NULL != hdcSrc)
	{
		HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcSrc, bitmap);

		success = BitBlt(hdc, blitRect.left, blitRect.top, 
					blitRect.right - blitRect.left, blitRect.bottom - blitRect.top, 
					hdcSrc, 
					blitRect.left - rect.left, 
					bm.bmHeight * frame + (blitRect.top - rect.top), 
					SRCCOPY);

		SelectObject(hdcSrc, hbmpOld);
		DeleteDC(hdcSrc);
	}
	
	return success;

}