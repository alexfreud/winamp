#include "precomp.h"

#include "solid.h"
#include <api/xml/xmlparams.h>
#include <api/memmgr/api_memmgr.h>

#ifndef _WASABIRUNTIME

BEGIN_SERVICES(SolidGen_Svc);
DECLARE_SERVICETSINGLE(svc_imageGenerator, SolidImage);
END_SERVICES(SolidGen_Svc, _SolidGen_Svc);

#ifdef _X86_
extern "C" { int _link_SolidGen_Svc; }
#else
extern "C" { int __link_SolidGen_Svc; }
#endif

#endif

int SolidImage::testDesc(const wchar_t *desc)
{
	return !WCSICMP(desc, L"$solid");
}

void premultiply(ARGB32 *m_pBits, int nwords)
{
	for (; nwords > 0; nwords--, m_pBits++)
	{
		unsigned __int8 *pixel = (unsigned __int8 *)m_pBits;
		unsigned int alpha = pixel[3];
		if (alpha == 255) continue;
		pixel[0] = (pixel[0] * alpha) >> 8;	// blue
		pixel[1] = (pixel[1] * alpha) >> 8;	// green
		pixel[2] = (pixel[2] * alpha) >> 8;	// red
	}
}

ARGB32 *SolidImage::genImage(const wchar_t *desc, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params)
{
	int _w = params->getItemValueInt(L"w", 1);
	if (_w == 0) _w = 1;
	int _h = params->getItemValueInt(L"h", 1);
	if (_h == 0) _h = 1;
	if (_w <= 0 || _h <= 0) return NULL;
	ARGB32 color = _byteswap_ulong(WASABI_API_SKIN->parse(params->getItemValue(L"color"), L"color") << 8);

	unsigned int alpha = params->getItemValueInt(L"alpha", 255); 
	color |= ((alpha & 0xff) << 24);

	premultiply(&color, 1);

#ifdef WASABI_COMPILE_MEMMGR
	ARGB32 *ret = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(_w * _h * sizeof(ARGB32));
#else
	ARGB32 *ret = (ARGB32*)MALLOC(_w * _h * sizeof(ARGB32));
#endif

	MEMFILL<ARGB32>(ret, color, _w * _h);

	*w = _w;
	*h = _h;

	*has_alpha = (alpha == 255) ? 0 : 1;

	return ret;
}
