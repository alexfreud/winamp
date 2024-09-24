#ifndef NULLSOFT_API_TAGZH
#define NULLSOFT_API_TAGZH

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include "ifc_tagprovider.h"
#include "ifc_tagparams.h"

enum
{
	TAGZ_SUCCESS=0,
	TAGZ_FAILURE=1,
	TAGZ_UNDEFINED_VARIABLE=2,
};

class api_tagz : public Dispatchable
{
protected:
	api_tagz() {}
	~api_tagz() {}
public:
	DISPATCH_CODES
	{
		API_TAGZ_FORMAT = 10,
		API_TAGZ_HELP_STRING=20,
		API_TAGZ_ADD_FUNC=30,
		API_TAGZ_SET_VARIABLE = 40,
	};

	int format(const wchar_t *spec, wchar_t *out, size_t outCch, ifc_tagprovider *tagProvider, ifc_tagparams *parameters);
	char *manual();
	void SetVariable(const wchar_t *name, const wchar_t *value);
	//TODO: int AddFunction();
};

inline int api_tagz::format(const wchar_t *spec, wchar_t *out, size_t outCch, ifc_tagprovider *tagProvider, ifc_tagparams *parameters)
{
	return _call(API_TAGZ_FORMAT, (int)0, spec, out, outCch, tagProvider, parameters);
}

inline char *api_tagz::manual()
{
	return _call(API_TAGZ_HELP_STRING, (char *)0);
}

inline void api_tagz::SetVariable(const wchar_t *name, const wchar_t *value)
{
	_voidcall(API_TAGZ_SET_VARIABLE, name, value);
}

// {063CD9FD-57C1-4da2-9A8F-15FFFA9ABF50}
static const GUID tagzGUID = 
{ 0x63cd9fd, 0x57c1, 0x4da2, { 0x9a, 0x8f, 0x15, 0xff, 0xfa, 0x9a, 0xbf, 0x50 } };


#endif