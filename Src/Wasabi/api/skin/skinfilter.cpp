#include <precomp.h>
#include "skinfilter.h"
#include <api/service/svcs/svc_skinfilter.h>

void ApplySkinFilters::apply(const wchar_t *elementid, const wchar_t *forced_gammagroup, ARGB32 *bits, int w, int h, int bpp) 
{
  if ((elementid == NULL && forced_gammagroup == NULL) || bits == NULL || w <= 0 || h <= 0)
    return;
  SkinFilterEnum sfe;

  while (1) 
  {
    svc_skinFilter *obj = sfe.getNext(FALSE);
    if (!obj) break;
    obj->filterBitmap((uint8_t *)bits, w, h, bpp, elementid, forced_gammagroup);
    sfe.getLastFactory()->releaseInterface(obj);
  }
}
