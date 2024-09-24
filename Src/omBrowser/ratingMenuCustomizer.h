#ifndef NULLSOFT_WINAMP_RATING_MENU_CUSTOMIZER_HEADER
#define NULLSOFT_WINAMP_RATING_MENU_CUSTOMIZER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_menucustomizer.h"

class ifc_skinnedrating;

class RatingMenuCustomizer : public ifc_menucustomizer
{

protected:
	RatingMenuCustomizer(HMENU hMenu, ifc_skinnedrating *skinnedRating);
	~RatingMenuCustomizer();

public:
	static HRESULT CreateInstance(HMENU hMenu, ifc_skinnedrating *skinnedRating, RatingMenuCustomizer **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_menucustomizer (partial) */
	INT CustomDraw(HMENU menuInstance, INT action, HDC hdc, LPARAM param);

protected:
	HRESULT GetValue(INT itemId, INT *valueOut);
	INT MeasureRating(HDC hdc, MEASUREITEMSTRUCT *pmis);
	INT DrawRating(HDC hdc, DRAWITEMSTRUCT *pdis);

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
	HMENU menu;
	ifc_skinnedrating *skin;
};


#endif //NULLSOFT_WINAMP_RATING_MENU_CUSTOMIZER_HEADER