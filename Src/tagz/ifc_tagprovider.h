#ifndef NULLSOFT_TAG_PROVIDER_H
#define NULLSOFT_TAG_PROVIDER_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include "ifc_tagparams.h"
#include <api/service/services.h>

class ifc_tagprovider : public Dispatchable
{
protected:
	ifc_tagprovider() {}
	~ifc_tagprovider() {}
public:
	DISPATCH_CODES
	{
		IFC_TAGPROVIDER_GET_TAG = 10,
		IFC_TAGPROVIDER_FREE_TAG = 20,
	};

	wchar_t *GetTag(const wchar_t *name, ifc_tagparams *parameters); //return 0 if not found
	void FreeTag(wchar_t *tag);
};

inline wchar_t *ifc_tagprovider::GetTag(const wchar_t *name, ifc_tagparams *parameters)
{
	return _call(IFC_TAGPROVIDER_GET_TAG, (wchar_t *)0, name, parameters);
}

inline void ifc_tagprovider::FreeTag(wchar_t *tag)
{
	_voidcall(IFC_TAGPROVIDER_FREE_TAG, tag);
}
#endif