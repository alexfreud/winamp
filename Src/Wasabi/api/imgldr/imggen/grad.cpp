#include "precomp.h"

#include "grad.h"

#include <api/xml/xmlparams.h>
#include <api/memmgr/api_memmgr.h>
#ifndef _WASABIRUNTIME

BEGIN_SERVICES(GradientGen_Svc);
DECLARE_SERVICETSINGLE(svc_imageGenerator, GradientImage);
END_SERVICES(GradientGen_Svc, _GradientGen_Svc);

#ifdef _X86_
extern "C" { int _link_GradientGen_Svc; }
#else
extern "C" { int __link_GradientGen_Svc; }
#endif

#endif


int GradientImage::testDesc(const wchar_t *desc) {
  return !_wcsicmp(desc, L"$gradient");
}

ARGB32 *GradientImage::genImage(const wchar_t *desc, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params) 
{
  int _w = params->getItemValueInt(L"w",1);
  if (_w == 0) _w = 1;
  int _h = params->getItemValueInt(L"h",1);
  if (_h == 0) _h = 1;
  if (_w <= 0 || _h <= 0) return NULL;

#ifdef WASABI_COMPILE_MEMMGR
  ARGB32 *ret = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(_w * _h * sizeof(ARGB32));
#else
  ARGB32 *ret = (ARGB32*)MALLOC(_w * _h * sizeof(ARGB32));
#endif

  setX1((float)WTOF(params->getItemValue(L"gradient_x1")));
  setY1((float)WTOF(params->getItemValue(L"gradient_y1")));
  setX2((float)WTOF(params->getItemValue(L"gradient_x2")));
  setY2((float)WTOF(params->getItemValue(L"gradient_y2")));

  setPoints(params->getItemValue(L"points"));

  setMode(params->getItemValue(L"mode"));

  setReverseColors(TRUE);	// cuz we're imggen

  setAntialias(params->getItemValueInt(L"antialias"));

  renderGradient(ret, _w, _h);

  *w = _w;
  *h = _h;
  *has_alpha = 1;	// will be optimized anyway

  return ret;
}
