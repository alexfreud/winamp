#ifndef _SOLID_H
#define _SOLID_H

#include <api/service/svcs/svc_imggen.h>

class SolidImage : public svc_imageGeneratorI
{
public:
	static const char *getServiceName() { return "Solid Color image generator"; }
	virtual int testDesc(const wchar_t *desc);
	virtual ARGB32 *genImage(const wchar_t *desc, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params = NULL);
};

#endif
