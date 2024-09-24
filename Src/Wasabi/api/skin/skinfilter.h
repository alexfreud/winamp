#ifndef _SKINFILTER_H
#define _SKINFILTER_H

#include <bfc/wasabi_std.h>

class ApplySkinFilters 
{
public:
  static void apply(const wchar_t *element_id, const wchar_t *forced_gammagroup, ARGB32 *bits, int w, int h, int bpp=32);
};

#endif
