#ifndef NULLSOFT_IMPL_TAGZ_H
#define NULLSOFT_IMPL_TAGZ_H

#include "api_tagz.h"
#include "tagz.h"

class Tagz : public api_tagz
{
public:
	int Format(const wchar_t *spec, wchar_t *out, size_t outCch, ifc_tagprovider *tagProvider, ifc_tagparams *parameters);
	char *Manual();
	void SetVariable(const wchar_t *name, const wchar_t *value);
	int GetVariable(const wchar_t *name, wchar_t *value, size_t valueLen);

protected:
	RECVS_DISPATCH;
};

#endif