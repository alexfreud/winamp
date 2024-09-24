#include <precomp.h>

#include "skinfilter.h"

#include <studio/services/svc_skinfilter.h>

void ApplySkinFilters::apply(const char *elementid, const char *forced_gammagroup, ARGB32 *bits, int w, int h, int bpp) {
	if ((elementid == NULL && forced_gammagroup == NULL) || bits == NULL || w <= 0 || h <= 0) return;
	SkinFilterEnum sfe;

	while (1) {
		svc_skinFilter *obj = sfe.getNext(FALSE);
		if (!obj) break;
		obj->filterBitmap((unsigned char *)bits, w, h, bpp, elementid, forced_gammagroup);
		sfe.getLastFactory()->releaseInterface(obj);
	}
}
