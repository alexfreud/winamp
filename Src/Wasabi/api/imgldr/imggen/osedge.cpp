#include "precomp.h"

#include "osedge.h"
#include <api/xml/xmlparams.h>
#include <bfc/parse/pathparse.h>
#include <api/memmgr/api_memmgr.h>
#ifndef _WASABIRUNTIME

BEGIN_SERVICES(OsEdgeGen_Svc);
DECLARE_SERVICETSINGLE(svc_imageGenerator, OsEdgeImage);
END_SERVICES(OsEdgeGen_Svc, _OsEdgeGen_Svc);

#ifdef _X86_
extern "C" { int _link_OsEdgeGen_Svc; }
#else
extern "C" { int __link_OsEdgeGen_Svc; }
#endif

#endif


int OsEdgeImage::testDesc(const wchar_t *desc) {
  return !_wcsicmp(desc, L"$osedge");
}

ARGB32 *OsEdgeImage::genImage(const wchar_t *desc, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params) 
{
  int _w = params->getItemValueInt(L"w", 1);
  if (_w == 0) _w = 1;
  int _h = params->getItemValueInt(L"h", 1);
  if (_h == 0) _h = 1;
  if (_w <= 0 || _h <= 0) return NULL;

#ifdef WASABI_COMPILE_MEMMGR
  ARGB32 *ret = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(_w * _h * sizeof(ARGB32));
#else
  ARGB32 *ret = (ARGB32*)MALLOC(_w * _h * sizeof(ARGB32));
#endif

  RECT r = Wasabi::Std::makeRect(0, 0, _w, _h);

  BITMAPINFO bmi;
  ZERO(bmi);
  bmi.bmiHeader.biSize = sizeof(bmi);
  bmi.bmiHeader.biWidth = _w;
  bmi.bmiHeader.biHeight = -_h;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  // the rest are 0
  ARGB32 *bits;
  HBITMAP hbmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
  HDC hdc = CreateCompatibleDC(NULL);
  HBITMAP prevbmp = (HBITMAP)SelectObject(hdc, hbmp);

  unsigned long edgev = 0;
  if (!_wcsicmp(params->getItemValue(L"edge"), L"bump")) edgev = EDGE_BUMP;
  else if (!_wcsicmp(params->getItemValue(L"edge"), L"etched")) edgev = EDGE_ETCHED;
  else if (!_wcsicmp(params->getItemValue(L"edge"), L"raised")) edgev = EDGE_RAISED;
  else if (!_wcsicmp(params->getItemValue(L"edge"), L"sunken")) edgev = EDGE_SUNKEN;
  if (edgev == 0) edgev = EDGE_RAISED;

  unsigned long sides = 0;
  PathParserW pp(params->getItemValue(L"sides"), L",");
  for (int i = 0; i < pp.getNumStrings(); i++) {
    const wchar_t *p = pp.enumString(i);
    if (!_wcsicmp(p, L"left")) sides |= BF_LEFT;
    if (!_wcsicmp(p, L"top")) sides |= BF_TOP;
    if (!_wcsicmp(p, L"right")) sides |= BF_RIGHT;
    if (!_wcsicmp(p, L"bottom")) sides |= BF_BOTTOM;
    if (!_wcsicmp(p, L"all")) sides |= BF_RECT;
    if (!_wcsicmp(p, L"middle")) sides |= BF_MIDDLE;
    if (!_wcsicmp(p, L"flat")) sides |= BF_FLAT;
    if (!_wcsicmp(p, L"soft")) sides |= BF_SOFT;
    if (!_wcsicmp(p, L"mono")) sides |= BF_MONO;
  }

// DO EET
  DrawEdge(hdc, &r, edgev, sides);

  MEMCPY(ret, bits, sizeof(ARGB32) * _w * _h);
  for (int i = 0; i < _w * _h; i++) {	// force alpha
    ret[i] |= 0xff000000;
  }

  SelectObject(hdc, prevbmp);
  DeleteDC(hdc);
  DeleteObject(hbmp);

  *w = _w;
  *h = _h;
  *has_alpha = 1;	// will be optimized anyway

  return ret;
}
