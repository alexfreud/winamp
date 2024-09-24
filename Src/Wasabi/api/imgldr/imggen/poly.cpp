#include "precomp.h"

#include "poly.h"

#include <api/xml/xmlparams.h>
#include <api/memmgr/api_memmgr.h>
#ifndef _WASABIRUNTIME

BEGIN_SERVICES(PolyGen_Svc);
DECLARE_SERVICETSINGLE(svc_imageGenerator, PolyImage);
END_SERVICES(PolyGen_Svc, _PolyGen_Svc);

#ifdef _X86_
extern "C" { int _link_PolyGen_Svc; }
#else
extern "C" { int __link_PolyGen_Svc; }
#endif

#endif

int PolyImage::testDesc(const wchar_t *desc) {
  return !_wcsicmp(desc, L"$polygon");
}

void premultiply(ARGB32 *m_pBits, int nwords);

#include <bfc/draw/drawpoly.h>

ARGB32 *PolyImage::genImage(const wchar_t *desc, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params) 
{
  int _w = (params->getItemValueInt(L"w", 1));
  if (_w == 0) _w = 1;
  int _h = (params->getItemValueInt(L"h", 1));
  if (_h == 0) _h = 1;
  if (_w <= 0 || _h <= 0) return NULL;

  const wchar_t *bgcolorstr = params->getItemValue(L"bgcolor");
  ARGB32 bgcolor = (bgcolorstr == NULL || *bgcolorstr=='\0') ? 0 : _byteswap_ulong(WASABI_API_SKIN->parse(params->getItemValue(L"bgcolor"), L"color")<<8);

  unsigned int bgalpha = params->getItemValueInt(L"bgalpha", 0);
  bgcolor |= ((bgalpha & 0xff) << 24);

  premultiply(&bgcolor, 1);

#ifdef WASABI_COMPILE_MEMMGR
  ARGB32 *ret = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(_w * _h * sizeof(ARGB32));
#else
  ARGB32 *ret = (ARGB32*)MALLOC(_w * _h * sizeof(ARGB32));
#endif

  MEMFILL<ARGB32>(ret, bgcolor, _w * _h);

  Draw::drawPointList(ret, _w, _h, params->getItemValue(L"points"));

  *w = _w;
  *h = _h;
  *has_alpha = 1;
  return ret;
}
