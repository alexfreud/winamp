#include "main.h"
#include "./stockobjects.h"
#include "./colors.h"


typedef struct MlStockObjects
{
	CRITICAL_SECTION lock;
	HDC cachedDc;
	HFONT skinFont;
	HFONT systemFont;
	HBRUSH brushes[BRUSH_MAX - BRUSH_MIN + 1];
	HPEN pens[PEN_MAX - PEN_MIN + 1];
} MlStockObjects;

static MlStockObjects stockObjects;

static void 
MlStockObjects_ResetEx(MlStockObjects *self)
{
	INT index;
	if (NULL != self->cachedDc) 
	{
		DeleteDC(self->cachedDc); 
		self->cachedDc = NULL;
	}

	if (NULL != self->skinFont) 
	{
		DeleteObject(self->skinFont); 
		self->skinFont = NULL;
	}

	if (NULL != self->systemFont)
	{
		DeleteObject(self->systemFont);
		self->systemFont = NULL;
	}
	
	for (index = 0; index < ARRAYSIZE(self->brushes); index++) 
	{ 
		if (NULL != self->brushes[index]) 
			DeleteObject(self->brushes[index]); 
	}
	ZeroMemory(self->brushes, sizeof(self->brushes));

	for (index = 0; index < ARRAYSIZE(self->pens); index++) 
	{ 
		if (NULL != self->pens[index]) 
			DeleteObject(self->pens[index]); 
	}
	ZeroMemory(self->pens, sizeof(self->pens));

}

static BYTE 
MlStockObjects_GetSysFontQuality()
{
	BOOL smoothingEnabled;
	if (FALSE == SystemParametersInfoW(SPI_GETFONTSMOOTHING, 0, &smoothingEnabled, 0) ||
		FALSE == smoothingEnabled)
	{
		return DEFAULT_QUALITY;
	}
    
	UINT smootingType;
    if (FALSE == SystemParametersInfoW(SPI_GETFONTSMOOTHINGTYPE, 0, &smootingType, 0))
		return DEFAULT_QUALITY;
	    
	if (FE_FONTSMOOTHINGCLEARTYPE == smootingType)
			return CLEARTYPE_QUALITY;

	return ANTIALIASED_QUALITY;
}

static HDC 
MlStockObjects_GetCachedDc(MlStockObjects *self)
{
	if (NULL == self->cachedDc) 
	{
		HDC baseDC = GetDCEx(g_hwnd, NULL, DCX_WINDOW | DCX_CACHE | DCX_NORESETATTRS);
		self->cachedDc = CreateCompatibleDC(baseDC);
		ReleaseDC(g_hwnd, baseDC);
	}
	return self->cachedDc;
}

static HFONT 
MlStockObjects_GetSkinFont(MlStockObjects *self)
{		
	if (NULL == self->skinFont &&
		NULL != g_config && 
		0 != g_config->ReadInt(L"plfont_everywhere", 1))
	{
		INT height = (INT)SendMessageW(plugin.hwndParent, WM_WA_IPC, 3, IPC_GET_GENSKINBITMAP);
		DWORD charset = (DWORD)SendMessageW(plugin.hwndParent, WM_WA_IPC, 2, IPC_GET_GENSKINBITMAP);
		char *fontname = (char*)SendMessageW(plugin.hwndParent, WM_WA_IPC, 1, IPC_GET_GENSKINBITMAP);
		self->skinFont = CreateFontA(-height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontname);
	}

	return self->skinFont;
}


static HFONT
MlStockObjects_GetSystemFont(MlStockObjects *self)
{		
	if (NULL == self->systemFont)
	{
		LOGFONTW lf;
	
		if (FALSE == SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0))
			return NULL;
	
		lf.lfQuality = MlStockObjects_GetSysFontQuality();
		self->systemFont = CreateFontIndirectW(&lf);
	}

	return  self->systemFont;
}

static HBRUSH 
MlStockObjects_GetBrush(MlStockObjects *self, UINT type)
{	
	if (NULL == self->brushes[type - BRUSH_MIN]) 
	{
		INT color;
		switch(type)
		{
			case WNDBCK_BRUSH:		color = WADLG_WNDBG; break;
			case ITEMBCK_BRUSH:		color = WADLG_ITEMBG; break;
			case HILITE_BRUSH:		color = WADLG_HILITE; break;
			case ITEMTEXT_BRUSH:	color = WADLG_ITEMFG; break;
			default:				return NULL;
		}
		self->brushes[type - BRUSH_MIN] = CreateSolidBrush(WADlg_getColor(color));
	}
	return self->brushes[type - BRUSH_MIN];
}

static HPEN 
MlStockObjects_GetPen(MlStockObjects *self, UINT type)
{	
	if (NULL == self->pens[type - PEN_MIN]) 
	{
		COLORREF rgb;
		switch(type)
		{
			case HILITE_PEN:		rgb = WADlg_getColor(WADLG_HILITE); break;
			case HEADERTOP_PEN:		rgb = WADlg_getColor(WADLG_LISTHEADER_FRAME_TOPCOLOR); break;
			case HEADERMIDDLE_PEN:	rgb = WADlg_getColor(WADLG_LISTHEADER_FRAME_MIDDLECOLOR); break;
			case HEADERBOTTOM_PEN:	rgb = WADlg_getColor(WADLG_LISTHEADER_FRAME_BOTTOMCOLOR); break;
			case WNDBCK_PEN:		rgb = WADlg_getColor(WADLG_WNDBG); break;
			case MENUBORDER_PEN:	MLGetSkinColor(MLSO_MENU, MP_FRAME, 0, &rgb); break;
			case TOOLTIPBORDER_PEN:	MLGetSkinColor(MLSO_TOOLTIP, TTP_FRAME, 0, &rgb); break;
			case ITEMBCK_PEN:		rgb = WADlg_getColor(WADLG_ITEMBG); break;
			case ITEMTEXT_PEN:		rgb = WADlg_getColor(WADLG_ITEMFG); break;
			default:				return NULL;
		}
		self->pens[type - PEN_MIN] = CreatePen(PS_SOLID, 0, rgb);
	}

	return self->pens[type - PEN_MIN];
}

static HANDLE 
MlStockObjects_GetEx(MlStockObjects *self, UINT type)
{
	HANDLE handle;
	EnterCriticalSection(&self->lock);

	if (CACHED_DC == type)
		handle = MlStockObjects_GetCachedDc(self);
	else if (SKIN_FONT == type) 
		handle = MlStockObjects_GetSkinFont(self);
	else if (DEFAULT_FONT == type) 
		handle = MlStockObjects_GetSystemFont(self);
	else if (BRUSH_MIN <= type && BRUSH_MAX >= type) 
		handle = MlStockObjects_GetBrush(self, type);
	else if (PEN_MIN <= type && PEN_MAX >= type) 
		handle = MlStockObjects_GetPen(self, type);
	else
		handle = NULL;

	LeaveCriticalSection(&self->lock);

	return handle;
}


void
MlStockObjects_Init()
{
	ZeroMemory(&stockObjects, sizeof(MlStockObjects));
	InitializeCriticalSection(&stockObjects.lock);
}

void
MlStockObjects_Free()
{
	MlStockObjects_ResetEx(&stockObjects);
	DeleteCriticalSection(&stockObjects.lock);
}

HANDLE 
MlStockObjects_Get(UINT type)
{
	return MlStockObjects_GetEx(&stockObjects, type);
}

void
MlStockObjects_Reset()
{
	MlStockObjects_ResetEx(&stockObjects);
}
