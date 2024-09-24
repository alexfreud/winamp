#include "./loginGui.h"
#include "./common.h"
#include "./imageLoader.h"

#include "../resource.h"
#include "../api.h"

#include <strsafe.h>

static size_t threadStorage = TLS_OUT_OF_INDEXES;

LoginGuiObject::LoginGuiObject()
: ref(1), bitmapIcons(NULL), fontTitle(NULL), fontEditor(NULL), fontText(NULL)
{
}

LoginGuiObject::~LoginGuiObject()
{
	Reset();
}

HRESULT LoginGuiObject::InitializeThread()
{
	
	if (TLS_OUT_OF_INDEXES == threadStorage)
	{
		if (NULL == WASABI_API_APP)
			return E_UNEXPECTED;
		
		threadStorage = WASABI_API_APP->AllocateThreadStorage();
		if (TLS_OUT_OF_INDEXES == threadStorage)
			return E_UNEXPECTED;
	}
	
	LoginGuiObject *cache = (LoginGuiObject*)WASABI_API_APP->GetThreadStorage(threadStorage);
	if (NULL != cache) 
	{
		cache->AddRef();
		return S_FALSE;
	}
	
	if (NULL == cache)
	{
		cache = new LoginGuiObject();
		if (NULL == cache) return E_OUTOFMEMORY;
		WASABI_API_APP->SetThreadStorage(threadStorage, cache);
	}

	return S_OK;
}

HRESULT LoginGuiObject::UninitializeThread()
{
	if (TLS_OUT_OF_INDEXES == threadStorage)
		return E_FAIL;
	
	if (NULL == WASABI_API_APP)
		return E_UNEXPECTED;
	
	LoginGuiObject *cache = (LoginGuiObject*)WASABI_API_APP->GetThreadStorage(threadStorage);
	if (NULL != cache && 0 == cache->Release())
		WASABI_API_APP->SetThreadStorage(threadStorage, NULL);
	
	return S_OK;
}

HRESULT LoginGuiObject::QueryInstance(LoginGuiObject **instance)
{
	if (NULL == instance)
		return E_POINTER;
	
	if (TLS_OUT_OF_INDEXES == threadStorage)
		return E_UNEXPECTED;
	
	*instance = (LoginGuiObject*)WASABI_API_APP->GetThreadStorage(threadStorage);
	if (NULL == *instance) return E_FAIL;
	(*instance)->AddRef();	

	return S_OK;
}

ULONG LoginGuiObject::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginGuiObject::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginGuiObject::Reset()
{
	if (NULL != bitmapIcons)
	{
		DeleteObject(bitmapIcons);
		bitmapIcons = NULL;
	}

	if (NULL != fontTitle)
	{
		DeleteObject(fontTitle);
		fontTitle = NULL;
	}

	if (NULL != fontEditor)
	{
		DeleteObject(fontEditor);
		fontEditor = NULL;
	}

	if (NULL != fontText)
	{
		DeleteObject(fontText);
		fontText = NULL;
	}

	return S_OK;
}

HRESULT LoginGuiObject::GetIconDimensions(INT *pWidth, INT *pHeight)
{
	if (NULL == bitmapIcons)
	{
		INT width, height;
		bitmapIcons = ImageLoader_LoadBitmap(WASABI_API_ORIG_HINST, MAKEINTRESOURCE(IDR_NOTIFIERICONS_IMAGE), 
							TRUE, &width, &height);

		if (NULL == bitmapIcons)
			return E_FAIL;

		if (height < 0) height = -height;
		
		if (NULL != pWidth) *pWidth = width;
		if (NULL != pHeight) *pHeight = width;

		return S_OK;
	}

	BITMAP bm;
	if (sizeof(bm) != GetObject(bitmapIcons, sizeof(bm), &bm))
		return E_FAIL;
	
	if (NULL != pWidth) *pWidth = bm.bmWidth;
	if (NULL != pHeight) *pHeight = bm.bmWidth;

	return S_OK;
}

HBITMAP LoginGuiObject::GetIcon(INT iconId, RECT *prcIcon)
{
	if (NULL == prcIcon || iconId < 0) 
		return NULL;
	
	INT width, height;

	if (NULL != bitmapIcons)
	{
		BITMAP bm;
		if (sizeof(bm) != GetObject(bitmapIcons, sizeof(bm), &bm))
			bitmapIcons = NULL;
		else
		{
			width = bm.bmWidth;
			height = bm.bmHeight;
		}
	}

	if (NULL == bitmapIcons)
	{		
		bitmapIcons = ImageLoader_LoadBitmap(WASABI_API_ORIG_HINST, MAKEINTRESOURCE(IDR_NOTIFIERICONS_IMAGE), 
							TRUE, &width, &height);

		if (NULL == bitmapIcons)
			return NULL;
	}
	
	if (height < 0) height = -height;

	if (width * (iconId + 1) > height)
		return NULL;

	prcIcon->left = 0;
	prcIcon->right = width;
	prcIcon->top = width * iconId;
	prcIcon->bottom = prcIcon->top + width;
	
	return bitmapIcons;
}


static HFONT LoginGuiObject_DuplicateFont(HFONT fontBase, INT heightDeltaPt)
{
	if (NULL == fontBase) return NULL;
	
	LOGFONT lf;
	if (sizeof(lf) != GetObject(fontBase, sizeof(lf), &lf))
		return NULL;

	if (0 != heightDeltaPt)
	{
		HDC hdc = GetDCEx(NULL, NULL, DCX_WINDOW | DCX_CACHE | DCX_NORESETATTRS);
		HDC hdcTmp = NULL;

		if (NULL != hdc)
		{
			hdcTmp = CreateCompatibleDC(hdc);
			ReleaseDC(NULL, hdc);
		}
		
		if (NULL == hdcTmp)
			return NULL;
		
		LONG pixelsY = GetDeviceCaps (hdcTmp, LOGPIXELSY);
		HFONT fontOrig = (HFONT)SelectObject(hdcTmp, fontBase);
		
		TEXTMETRIC tm;
		if (FALSE != GetTextMetrics(hdcTmp, &tm))
		{
			INT basePt = MulDiv(tm.tmHeight - tm.tmInternalLeading, 72, pixelsY);
			lf.lfHeight = -MulDiv((basePt + heightDeltaPt), pixelsY, 72);

		}

		SelectObject(hdcTmp, fontOrig);
		DeleteDC(hdcTmp);
	}

	return CreateFontIndirect(&lf);
}

HFONT LoginGuiObject::GetTitleFont()
{
	if (NULL == fontTitle)
	{		
		HFONT fontBase = GetTextFont();
		if (NULL != fontBase)
		{
			fontTitle = LoginGuiObject_DuplicateFont(fontBase, 3);
		}
	}
	return fontTitle;
}

HFONT LoginGuiObject::GetEditorFont()
{
	if (NULL == fontEditor)
	{		
		HFONT fontBase = GetTextFont();
		if (NULL != fontBase)
		{
			fontEditor = LoginGuiObject_DuplicateFont(fontBase, 0);
		}
	}
	return fontEditor;
}

HFONT LoginGuiObject::GetTextFont()
{
	if (NULL == fontText)
	{
		LOGFONT lf;
		if (FALSE != SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0))
		{
			lf.lfQuality = LoginBox_GetSysFontQuality();
	   		fontText = CreateFontIndirect(&lf);
		}
	}
	return fontText;
}
