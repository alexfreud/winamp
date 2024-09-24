#ifndef _SVC_IMGGEN_H
#define _SVC_IMGGEN_H

#include <bfc/dispatch.h>
#include <api/service/services.h>


class NOVTABLE svc_imageGenerator : public Dispatchable
{
public:
	static FOURCC getServiceType() { return WaSvc::IMAGEGENERATOR; }
	int testDesc(const wchar_t *desc);
	ARGB32 *genImage(const wchar_t *desc, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params = NULL);
	int outputCacheable();

	enum {
	    TESTDESC = 10,
	    GENIMAGE = 30,
	    OUTPUTCACHEABLE = 40,
	};
};

inline int svc_imageGenerator::testDesc(const wchar_t *desc)
{
	return _call(TESTDESC, 0, desc);
}

inline ARGB32 *svc_imageGenerator::genImage(const wchar_t *desc, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params)
{
	return _call(GENIMAGE, (ARGB32 *)0, desc, has_alpha, w, h, params);
}

inline int svc_imageGenerator::outputCacheable()
{
	return _call(OUTPUTCACHEABLE, 0);
}

// derive from this one
class NOVTABLE svc_imageGeneratorI : public svc_imageGenerator
{
public:
	virtual int testDesc(const wchar_t *desc) = 0;
	virtual ARGB32 *genImage(const wchar_t *desc, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params = NULL) = 0;
	virtual int outputCacheable() { return 0; }

protected:
	RECVS_DISPATCH;
};

#include <api/service/svc_enum.h>
#include <bfc/string/StringW.h>

class ImgGeneratorEnum : public SvcEnumT<svc_imageGenerator>
{
public:
	ImgGeneratorEnum(const wchar_t *_desc) : desc(_desc) { }

protected:
	virtual int testService(svc_imageGenerator *svc)
	{
		return svc->testDesc(desc);
	}

private:
	StringW desc;
};

#endif
