#ifndef NULLSOFT_TAGZH
#define NULLSOFT_TAGZH

#include "string.h"
#include "varlist.h"
#include "ifc_tagprovider.h"
#include "ifc_tagparams.h"

class FMT
{
public:
	FMT() : vars(0), org_spec(0), spec(0), tagProvider(0), parameters(0), found(0) { }
	FMT(const wchar_t *p_spec, ifc_tagprovider *tagProvider, ifc_tagparams *_parameters, VarList * _vars);
	operator LPTSTR ();
	~FMT();
	void Open(const wchar_t *p_spec, ifc_tagprovider *tagProvider, ifc_tagparams *_parameters, VarList * _vars);

private:
	void run();
	void Error(LPTSTR e = 0);
	FMT(FMT *base, LPTSTR _spec);
	LPTSTR _FMT(LPTSTR s, size_t *f = 0);

private:
	tagz_::string str;
	VarList *vars;
	LPTSTR org_spec, spec;
	ifc_tagprovider *tagProvider;
	ifc_tagparams *parameters;
	int found;
};

#endif