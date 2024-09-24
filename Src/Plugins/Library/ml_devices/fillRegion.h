#ifndef _NULLSOFT_WINAMP_ML_DEVICES_FILL_REGION_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_FILL_REGION_HEADER


typedef struct FillRegion
{
	HRGN fill;
	HRGN tmp;
} FillRegion;

BOOL 
FillRegion_Init(FillRegion *region, 
				const RECT *rect);

void 
FillRegion_Uninit(FillRegion *region);

BOOL 
FillRegion_ExcludeRect(FillRegion *region, 
					   const RECT *rect);

BOOL 
FillRegion_ExcludeRgn(FillRegion *region, 
					  HRGN rgn);

BOOL 
FillRegion_AppendRect(FillRegion *region, 
					  const RECT *rect);

BOOL 
FillRegion_AppendRgn(FillRegion *region, 
					 HRGN rgn);

BOOL 
FillRegion_BrushFill(FillRegion *region, 
					 HDC hdc, 
					 HBRUSH brush);

BOOL 
FillRegion_Offset(FillRegion *region, 
				  long x, 
				  long y);

BOOL
FillRegion_SetRect(FillRegion *region, 
				   const RECT *rect);

BOOL
FillRegion_SetEmpty(FillRegion *region);

#endif //_NULLSOFT_WINAMP_ML_DEVICES_FILL_REGION_HEADER