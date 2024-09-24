#include "impl_tagz.h"
#include "tagz.h"
#include "api__tagz.h"
#include "resource.h"

int Tagz::Format(const wchar_t *spec, wchar_t *out, size_t outCch, ifc_tagprovider *tagProvider, ifc_tagparams *parameters)
{
	VarList vars;
	FMT formatter; 
	formatter.Open(spec, tagProvider, parameters, &vars);
	wchar_t *zz = formatter;
	if (zz)
	{
		lstrcpyn(out, zz, (int)outCch);
		free(zz);
	}
	else
	{
		memset(out, 0, (int)outCch);
	}
	return TAGZ_SUCCESS;
}

char *Tagz::Manual()
{
	return (char*)WASABI_API_LOADRESFROMFILEW(L"TEXT", MAKEINTRESOURCE(IDR_TAGZ_TEXT), 0);
}

void Tagz::SetVariable(const wchar_t *name, const wchar_t *value)
{
  /*vars.Put(name, value);*/
}

int Tagz::GetVariable(const wchar_t *name, wchar_t *value, size_t valueLen)
{
	return TAGZ_FAILURE;
	/*
	wchar_t *val = vars.Get(name);
	if (!val)
		return TAGZ_UNDEFINED_VARIABLE;
	lstrcpynW(value, val, (int)valueLen);
	return TAGZ_SUCCESS;
	*/
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS Tagz
START_DISPATCH;
CB(API_TAGZ_FORMAT, Format)
CB(API_TAGZ_HELP_STRING, Manual)
VCB(API_TAGZ_SET_VARIABLE, SetVariable)
END_DISPATCH;