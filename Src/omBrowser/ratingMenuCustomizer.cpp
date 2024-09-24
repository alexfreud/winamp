#include "main.h"
#include "./ratingMenuCustomizer.h"
#include "./ifc_skinhelper.h"
#include "./ifc_skinnedrating.h"

#include "../Plugins/General/gen_ml/ml_ipc_0313.h"

#define RATING_MINSPACECX		16

RatingMenuCustomizer::RatingMenuCustomizer(HMENU hMenu, ifc_skinnedrating *skinnedRating) 
	: ref(1), menu(hMenu), skin(skinnedRating)
{
	if (NULL != skin) 
		skin->AddRef();
}

RatingMenuCustomizer::~RatingMenuCustomizer()
{
	if (NULL != skin)
		skin->Release();
}

HRESULT RatingMenuCustomizer::CreateInstance(HMENU hMenu, ifc_skinnedrating *skinnedRating, RatingMenuCustomizer **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;
	
	if (NULL == hMenu || NULL == skinnedRating) return E_INVALIDARG;
	
	*instance = new RatingMenuCustomizer(hMenu, skinnedRating);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t RatingMenuCustomizer::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t RatingMenuCustomizer::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int RatingMenuCustomizer::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_MenuCustomizer))
		*object = static_cast<ifc_menucustomizer*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}


INT RatingMenuCustomizer::CustomDraw(HMENU menuInstance, INT action, HDC hdc, LPARAM param)
{
	if (menuInstance != menu) 
		return FALSE;

	switch(action)
	{
		case MLMENU_ACTION_MEASUREITEM:	return MeasureRating(hdc, (MEASUREITEMSTRUCT*)param);
		case MLMENU_ACTION_DRAWITEM:		return MLMENU_WANT_DRAWPART;
		case MLMENU_ACTION_DRAWBACK:		break;
		case MLMENU_ACTION_DRAWICON:		break;
		case MLMENU_ACTION_DRAWTEXT:		return DrawRating(hdc, (DRAWITEMSTRUCT*)param);
	}

	return FALSE;
}

HRESULT RatingMenuCustomizer::GetValue(INT itemId, INT *valueOut)
{
	if (NULL == menu) return E_UNEXPECTED;

	WCHAR szBuffer[32] = {0};
	INT cchBuffer = GetMenuStringW(menu, itemId, szBuffer, ARRAYSIZE(szBuffer), MF_BYCOMMAND);
	if (cchBuffer < 1 || cchBuffer > 5) 
        return E_INVALIDARG;
	
	for (INT i = 1; i < cchBuffer; i++)
	{
		if (szBuffer[i -1] != szBuffer[i])
			return E_INVALIDARG;
	}

	if (NULL != valueOut)
		*valueOut = cchBuffer;

	return S_OK;
}

INT RatingMenuCustomizer::MeasureRating(HDC hdc, MEASUREITEMSTRUCT *pmis)
{
	RECT rect;
	if (NULL == skin || NULL == hdc || 
		FAILED(GetValue(pmis->itemID, NULL)) || 
		FAILED(skin->CalcMinRect(5, &rect)))
	{
		return FALSE;
	}
	
	pmis->itemHeight = rect.bottom - rect.top + 6;
	
	TEXTMETRIC tm;
	if (GetTextMetrics(hdc, &tm) && 
		(UINT)(tm.tmHeight + 2) > pmis->itemHeight)
	{
		pmis->itemHeight = tm.tmHeight + 2;
	}
				
	INT spaceCX = (pmis->itemHeight > RATING_MINSPACECX) ? pmis->itemHeight : RATING_MINSPACECX;
	pmis->itemWidth = rect.right - rect.left + (2 * spaceCX) - (GetSystemMetrics(SM_CXMENUCHECK) - 1);

	return TRUE;
}

INT RatingMenuCustomizer::DrawRating(HDC hdc, DRAWITEMSTRUCT *pdis)
{
	INT value;
	if (NULL == skin || NULL == hdc || FAILED(GetValue(pdis->itemID, &value)))
		return FALSE;

	INT spaceCX = ((pdis->rcItem.bottom - pdis->rcItem.top) > RATING_MINSPACECX) ? 
					(pdis->rcItem.bottom - pdis->rcItem.top) : 
					RATING_MINSPACECX;

	RECT rect;
	CopyRect(&rect, &pdis->rcItem);
	rect.left += spaceCX;
	
	UINT menuState = GetMenuState(menu, pdis->itemID, MF_BYCOMMAND);
	UINT trackingValue = (0 == ((MF_DISABLED | MF_GRAYED) & menuState)) ? value : 0;
	
	return SUCCEEDED(skin->Draw(hdc, 5, value, trackingValue, &rect, RDS_LEFT | RDS_VCENTER | RDS_HOT));
}

#define CBCLASS RatingMenuCustomizer
START_DISPATCH;
  CB(API_CUSTOMDRAW, CustomDraw);
END_DISPATCH;
#undef CBCLASS

