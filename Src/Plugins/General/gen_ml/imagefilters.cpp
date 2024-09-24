#include "main.h"
#include "./imagefilters.h"

#define FILTER_REMOVE_ALPHA		0
#define FILTER_PRESERVE_ALPHA	1


#define MLIF_FILTER1_TITLE				L"Default filter #1 (removes alpha)"
#define MLIF_FILTER2_TITLE				L"Default filter #2"
#define MLIF_FILTER3_TITLE				L"Grayscale + filter#1"
#define MLIF_GRAYSCALE_TITLE			L"Grayscale filtes"
#define MLIF_BLENDONBK_TITLE			L"AlphaBlend filter"
#define MLIF_FILTER1_PRESERVE_ALPHA_TITLE	L"Default filter #1 (preserves alpha)"

static BOOL CALLBACK MLIF_FILTER1_PROC(LPBYTE pData, LONG cx, LONG cy, INT bpp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag, LPARAM lParam)
{
	LONG pitch, x, y;
	INT step;
	BYTE rBk, gBk, bBk, rFg, gFg, bFg;
	LPBYTE cursor, line;

	if (bpp < 24 || cx < 0) 
		return FALSE;

	if (cy < 0)
		cy = -cy;

	step = (bpp>>3);
	pitch = cx*step;
	while (pitch%4) pitch++;
	
	rFg = GetRValue(rgbFg); gFg = GetGValue(rgbFg); bFg = GetBValue(rgbFg);
	rBk = GetRValue(rgbBk); gBk = GetGValue(rgbBk); bBk = GetBValue(rgbBk);

	if (24 == bpp)
	{
		for (y = 0, line = pData; y < cy; y++, line += pitch)
		{	
			for (x = 0, cursor = line; x < cx; x++, cursor += 3) 
			{
				cursor[0] = bFg - ((bFg - bBk)*(255 - cursor[0])>>8);
				cursor[1] = gFg - ((gFg - gBk)*(255 - cursor[1])>>8);
				cursor[2] = rFg - ((rFg - rBk)*(255 - cursor[2])>>8);
			}
		}
	}
	else if (32 == bpp)
	{
		for (y = 0, line = pData; y < cy; y++, line += pitch )
		{	
			for (x = 0, cursor = line; x < cx; x++, cursor += 4) 
			{
				if (0x00 == cursor[3]) 
				{
					cursor[0] = bBk;
					cursor[1] = gBk;
					cursor[2] = rBk;
				}
				else if (0xFF == cursor[3])
				{
					cursor[0] = bFg - ((bFg - bBk)*(255 - cursor[0])>>8);
					cursor[1] = gFg - ((gFg - gBk)*(255 - cursor[1])>>8);
					cursor[2] = rFg - ((rFg - rBk)*(255 - cursor[2])>>8);
				}
				else
				{
					cursor[0] = ((bFg - ((bFg - bBk)*(255 - cursor[0])>>8))*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*bBk + 127)/255;
					cursor[1] = ((gFg - ((gFg - gBk)*(255 - cursor[1])>>8))*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*gBk + 127)/255;
					cursor[2] = ((rFg - ((rFg - rBk)*(255 - cursor[2])>>8))*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*rBk + 127)/255;
				}
			}
		}

		if (FILTER_REMOVE_ALPHA == lParam)
		{
			for (y = 0, line = pData; y < cy; y++, line += pitch)
			{	
				for (x = 0, cursor = line; x < cx; x++, cursor += 4) 
				{
					cursor[3] = 0xFF;
				}
			}
		}
	}
	return TRUE;
}

static BOOL CALLBACK MLIF_FILTER2_PROC(LPBYTE pData, LONG cx, LONG cy, INT bpp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag, LPARAM lParam)
{
	LONG pitch, x;
	INT step;
	LPBYTE cursor, line;
	BYTE chrom;
	BYTE rBk, gBk, bBk, rFg, gFg, bFg;

	if (bpp < 24) return FALSE;

	step = (bpp>>3);
	pitch = cx*step;
	while (pitch%4) pitch++;

	rFg = GetRValue(rgbFg); gFg = GetGValue(rgbFg); bFg = GetBValue(rgbFg);
	rBk = GetRValue(rgbBk); gBk = GetGValue(rgbBk); bBk = GetBValue(rgbBk);
	
	if (24 == bpp)
	{
		for (line = pData; cy-- != 0; line += pitch )
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += step) 
			{
				chrom = cursor[0];
				cursor[0] = (bBk * (255 - chrom) + bFg * chrom)>>8;
				cursor[1] = (gBk * (255 - chrom) + gFg * chrom)>>8;
				cursor[2] = (rBk * (255 - chrom) + rFg * chrom)>>8;
			}
		}
	}
	else if (32 == bpp)
	{
		for (line = pData; cy-- != 0; line += pitch )
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += 4) 
			{
				chrom = cursor[0];
				if (0x00 == cursor[3]) 
				{
					cursor[0] = bBk;
					cursor[1] = gBk;
					cursor[2] = rBk;
					cursor[3] = 0xFF;
				}
				else if (0xFF == cursor[3])
				{
					cursor[0] = (bBk * (255 - chrom) + bFg * chrom)>>8;
					cursor[1] = (gBk * (255 - chrom) + gFg * chrom)>>8;
					cursor[2] = (rBk * (255 - chrom) + rFg * chrom)>>8;
				}
				else
				{
					cursor[0] = (((bBk * (255 - chrom) + bFg * chrom)>>8)*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*bBk + 127)/255;
					cursor[1] = (((gBk * (255 - chrom) + gFg * chrom)>>8)*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*gBk + 127)/255;
					cursor[2] = (((rBk * (255 - chrom) + rFg * chrom)>>8)*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*rBk + 127)/255;
					cursor[3] = 0xFF;
				}
			}
		}
	}
	return TRUE;
}


static BOOL CALLBACK MLIF_FILTER3_PROC(LPBYTE pData, LONG cx, LONG cy, INT bpp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag, LPARAM lParam)
{
	LONG pitch, x;
	INT step, r, g, b;
	BYTE rBk, gBk, bBk, rFg, gFg, bFg, px;

	LPBYTE cursor, line;

	if (bpp < 24) return FALSE;

	step = (bpp>>3);
	pitch = cx*step;
	while (pitch%4) pitch++;
	
	rFg = GetRValue(rgbFg); gFg = GetGValue(rgbFg); bFg = GetBValue(rgbFg);
	rBk = GetRValue(rgbBk); gBk = GetGValue(rgbBk); bBk = GetBValue(rgbBk);

	if (24 == bpp)
	{
		for (line = pData; cy-- != 0; line += pitch )
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += 3) 
			{
				r = cursor[2];
				g = cursor[1];
				b = cursor[0];
				px = (r*299 + g*587 + b*114)/1000;
				
				cursor[0] = bFg - ((bFg - bBk)*(255 - px)>>8);
				cursor[1] = gFg - ((gFg - gBk)*(255 - px)>>8);
				cursor[2] = rFg - ((rFg - rBk)*(255 - px)>>8);
			}
		}
	}
	else if (32 == bpp)
	{
		for (line = pData; cy-- != 0; line += pitch )
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += 4) 
			{
				if (0x00 == cursor[3]) 
				{
					cursor[0] = bBk;
					cursor[1] = gBk;
					cursor[2] = rBk;
					cursor[3] = 0xFF;
				}
				else
				{
					r = cursor[2];
					g = cursor[1];
					b = cursor[0];
					px = (r*299 + g*587 + b*114)/1000;

					if (0xFF == cursor[3])
					{
						cursor[0] = bFg - ((bFg - bBk)*(255 - px)>>8);
						cursor[1] = gFg - ((gFg - gBk)*(255 - px)>>8);
						cursor[2] = rFg - ((rFg - rBk)*(255 - px)>>8);
					}
					else
					{
						cursor[0] = ((bFg - ((bFg - bBk)*(255 - px)>>8))*px + (((255 - px)*255 + 127)/255)*bBk + 127)/255;
						cursor[1] = ((gFg - ((gFg - gBk)*(255 - px)>>8))*px + (((255 - px)*255 + 127)/255)*gBk + 127)/255;
						cursor[2] = ((rFg - ((rFg - rBk)*(255 - px)>>8))*px + (((255 - px)*255 + 127)/255)*rBk + 127)/255;
						cursor[3] = 0xFF;
					}
				}
			}
		}
	}
	return TRUE;
}

static BOOL CALLBACK MLIF_GRAYSCALE_PROC(LPBYTE pData, LONG cx, LONG cy, INT bpp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag, LPARAM lParam)
{
	LONG pitch, x;
	INT step;
	LPBYTE cursor, line;
	BYTE luma;

	if (bpp < 24) return FALSE;

	step = (bpp>>3);
	pitch = cx*step;
	while (pitch%4) pitch++;
	
	for (line = pData; cy-- != 0; line += pitch )
	{	
		for (x = cx, cursor = line; x-- != 0; cursor += step) 
		{
			luma = (BYTE)((cursor[2]*30 + cursor[1]*59 + cursor[0]*11)/100); 
			cursor[0] = luma;
			cursor[1] = luma;
			cursor[2] = luma; 
		}
	}
	return TRUE;
}


static BOOL CALLBACK MLIF_BLENDONBK_PROC(LPBYTE pData, LONG cx, LONG cy, INT bpp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag, LPARAM lParam)
{
	LONG pitch, x;
	LPBYTE cursor, line;

	if (32 != bpp) return FALSE;

	pitch = cx*4;
	
	for (line = pData; cy-- != 0; line += pitch )
	{	
		for (x = cx, cursor = line; x-- != 0; cursor += 4) 
		{
			if (0x00 == cursor[3]) 
			{
				cursor[0] = GetBValue(rgbBk);
				cursor[1] = GetGValue(rgbBk);
				cursor[2] = GetRValue(rgbBk);
				cursor[3] = 0xFF;
			}
			else if (cursor[3] != 0xFF)
			{
				cursor[0] = (cursor[0]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*GetBValue(rgbBk) + 127)/255;
				cursor[1] = (cursor[1]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*GetGValue(rgbBk) + 127)/255;
				cursor[2] = (cursor[2]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*GetRValue(rgbBk) + 127)/255;
				cursor[3] = 0xFF;
			}
		}
	}
	return TRUE;
}

static BOOL CALLBACK MLIF_NULL_PROC(LPBYTE pData, LONG cx, LONG cy, INT bpp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag, LPARAM lParam)
{
	return TRUE;
}

BOOL RegisterImageFilters(HMLIMGFLTRMNGR hmlifMngr)
{
	MLIMAGEFILTERINFO_I mlif;
	BOOL fResult;
	ZeroMemory(&mlif, sizeof(MLIMAGEFILTERINFO_I));
	
	fResult = TRUE;
	mlif.mask = MLIFF_TITLE_I | MLIFF_FLAGS_I | MLIFF_PROC_I | MLIFF_PARAM_I;
	
	mlif.uid		= GUID_NULL;	// so nobdy can take it 
	mlif.fnProc		= MLIF_NULL_PROC;
	mlif.pszTitle	= NULL;
	mlif.fFlags		= MLIFF_IGNORE_BKCOLOR_I | MLIFF_IGNORE_FGCOLOR_I;
	mlif.lParam		= FILTER_REMOVE_ALPHA;
	if (!MLImageFilterI_Register(hmlifMngr, &mlif)) fResult = FALSE;

	mlif.uid		= MLIF_FILTER1_UID;
	mlif.fnProc		= MLIF_FILTER1_PROC;
	mlif.pszTitle	= MLIF_FILTER1_TITLE;
	mlif.fFlags		= 0;
	mlif.lParam		= 0;
	if (!MLImageFilterI_Register(hmlifMngr, &mlif)) fResult = FALSE;

	mlif.uid		= MLIF_FILTER2_UID;
	mlif.fnProc		= MLIF_FILTER2_PROC;
	mlif.pszTitle	= MLIF_FILTER2_TITLE;
	mlif.fFlags		= 0;
	mlif.lParam		= 0;
	if (!MLImageFilterI_Register(hmlifMngr, &mlif)) fResult = FALSE;

	mlif.uid		= MLIF_FILTER3_UID;
	mlif.fnProc		= MLIF_FILTER3_PROC;
	mlif.pszTitle	= MLIF_FILTER3_TITLE;
	mlif.fFlags		= 0;
	mlif.lParam		= 0;
	if (!MLImageFilterI_Register(hmlifMngr, &mlif)) fResult = FALSE;

	mlif.uid		= MLIF_GRAYSCALE_UID;
	mlif.fnProc		= MLIF_GRAYSCALE_PROC;
	mlif.pszTitle	= MLIF_GRAYSCALE_TITLE;
	mlif.fFlags		= MLIFF_IGNORE_BKCOLOR_I | MLIFF_IGNORE_FGCOLOR_I;
	mlif.lParam		= 0;
	if (!MLImageFilterI_Register(hmlifMngr, &mlif)) fResult = FALSE;

	mlif.uid		= MLIF_BLENDONBK_UID;
	mlif.fnProc		= MLIF_BLENDONBK_PROC;
	mlif.pszTitle	= MLIF_BLENDONBK_TITLE;
	mlif.fFlags		= MLIFF_IGNORE_FGCOLOR_I;
	mlif.lParam		= 0;
	if (!MLImageFilterI_Register(hmlifMngr, &mlif)) fResult = FALSE;

	mlif.uid		= MLIF_FILTER1_PRESERVE_ALPHA_UID;
	mlif.fnProc		= MLIF_FILTER1_PROC;
	mlif.pszTitle	= MLIF_FILTER1_PRESERVE_ALPHA_TITLE;
	mlif.fFlags		= 0;
	mlif.lParam		= FILTER_PRESERVE_ALPHA;
	if (!MLImageFilterI_Register(hmlifMngr, &mlif)) fResult = FALSE;

	return fResult;
}