#ifndef _SKINFILTER_H
#define _SKINFILTER_H

#include <bfc/std.h>

class ApplySkinFilters {
public:
  static void apply(const char *element_id, const char *forced_gammagroup, ARGB32 *bits, int w, int h, int bpp=32);
};

#endif
