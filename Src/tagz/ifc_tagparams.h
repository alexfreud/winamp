#ifndef NULLSOFT_TAGZ_IFC_TAGPARAMS_H
#define NULLSOFT_TAGZ_IFC_TAGPARAMS_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

// {7CF804EA-8882-4d45-A4AC-52983815D82C}
static const GUID filenameParameterID = 
{ 0x7cf804ea, 0x8882, 0x4d45, { 0xa4, 0xac, 0x52, 0x98, 0x38, 0x15, 0xd8, 0x2c } };


class ifc_tagparams : public Dispatchable
{
protected:
	ifc_tagparams() {}
	~ifc_tagparams() {}
public:
	void *GetParameter(const GUID *parameterID);
protected:
	DISPATCH_CODES
	{
		IFC_TAGPARAMS_GETPARAMETER = 10,
	};
};

inline void *ifc_tagparams::GetParameter(const GUID *parameterID)
{
	return _call(IFC_TAGPARAMS_GETPARAMETER, (void *)0, parameterID);
}

#endif