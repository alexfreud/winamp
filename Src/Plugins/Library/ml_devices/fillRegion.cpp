#include "main.h"
#include "./fillRegion.h"

static BOOL
FillRegion_TempRegionFromRect(FillRegion *region, const RECT *rect)
{
	if (NULL == region || NULL == rect)
		return FALSE;

	if (NULL == region->tmp)
	{
		region->tmp = CreateRectRgn(rect->left, rect->top, rect->right, rect->bottom);
		if (NULL == region->tmp)
			return FALSE;
	}
	else
	{
		if (FALSE == SetRectRgn(region->tmp, rect->left, rect->top, rect->right, rect->bottom))
			return FALSE;
	}

	return TRUE;
}

BOOL 
FillRegion_Init(FillRegion *region, const RECT *rect)
{
	if (NULL == region)
		return FALSE;

	region->fill = (NULL != rect) ? 
					CreateRectRgn(rect->left, rect->top, rect->right, rect->bottom) :
					NULL;

	region->tmp = NULL;

	if (NULL == region->fill)
		return FALSE;

	return TRUE;
}

void 
FillRegion_Uninit(FillRegion *region)
{
	if (NULL != region)
	{
		if (NULL != region->fill)
		{
			DeleteObject(region->fill);
			region->fill = NULL;
		}

		if (NULL != region->tmp)
		{
			DeleteObject(region->tmp);
			region->tmp = NULL;
		}
	}
}

BOOL 
FillRegion_ExcludeRect(FillRegion *region, const RECT *rect)
{
	if (NULL == region || NULL == rect)
		return FALSE;

	return (FALSE != FillRegion_TempRegionFromRect(region, rect) &&
		ERROR != CombineRgn(region->fill, region->fill, region->tmp, RGN_DIFF));
}

BOOL 
FillRegion_ExcludeRgn(FillRegion *region, HRGN rgn)
{
	if (NULL == region || NULL == rgn)
		return FALSE;

	return (ERROR != CombineRgn(region->fill, region->fill, rgn, RGN_DIFF));
}

BOOL 
FillRegion_AppendRect(FillRegion *region, const RECT *rect)
{
	if (NULL == region || NULL == rect)
		return FALSE;

	return (FALSE != FillRegion_TempRegionFromRect(region, rect) &&
			ERROR != CombineRgn(region->fill, region->fill, region->tmp, RGN_OR));
}

BOOL 
FillRegion_AppendRgn(FillRegion *region, HRGN rgn)
{
	if (NULL == region || NULL == rgn)
		return FALSE;

	return (ERROR != CombineRgn(region->fill, region->fill, rgn, RGN_OR));
}

BOOL 
FillRegion_BrushFill(FillRegion *region, HDC hdc, HBRUSH brush)
{
	if (NULL == region)
		return FALSE;

	return FillRgn(hdc, region->fill, brush);
}

BOOL 
FillRegion_Offset(FillRegion *region, long x, long y)
{
	if (NULL == region)
		return FALSE;

	return (ERROR != OffsetRgn(region->fill, x, y));
}

BOOL
FillRegion_SetRect(FillRegion *region, const RECT *rect)
{
	if (NULL == region || NULL == rect)
		return FALSE;

	return SetRectRgn(region->fill, rect->left, rect->top, rect->right, rect->bottom);
}

BOOL
FillRegion_SetEmpty(FillRegion *region)
{
	if (NULL == region)
		return FALSE;

	return SetRectRgn(region->fill, 0, 0, 0, 0);
}