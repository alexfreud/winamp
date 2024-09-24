#include "./ml_imagefilter.h"

#include <strsafe.h>

typedef struct _IMAGEFILTER
{
	GUID				uid;
	MLIMAGEFILTERPROC	proc;
	LPARAM				param;
	UINT				flags;			
	LPWSTR				pszName;
} IMAGEFILTER;

typedef struct _IMAGEFILTERMGNR
{
	IMAGEFILTER		*filters;
	INT				count;
	INT				allocated;
	INT				allocstep;
} IMAGEFILTERMGNR;

HMLIMGFLTRMNGR MLImageFilterI_CreateManager(INT cInitial, INT cGrow)
{
	IMAGEFILTERMGNR *pMngr = (IMAGEFILTERMGNR*)calloc(1, sizeof(IMAGEFILTERMGNR));
	if (!pMngr) return NULL;

	pMngr->allocstep = (cGrow < 1) ? 1 : cGrow;

	if (cInitial > 0)
	{
		if (cInitial > 40) cInitial = 40;
		pMngr->filters = (IMAGEFILTER*)calloc(cInitial, sizeof(IMAGEFILTER));
		if (!pMngr->filters)
		{
			free(pMngr);
			return NULL;
		}
		pMngr->allocated = cInitial;
	}
	return (HMLIMGFLTRMNGR)pMngr;
}

BOOL MLImageFilterI_DestroyManager(HMLIMGFLTRMNGR hmlifMngr)
{
	if (!hmlifMngr) return FALSE;

	if (((IMAGEFILTERMGNR*)hmlifMngr)->filters) 
	{
		INT index;
		for (index = ((IMAGEFILTERMGNR*)hmlifMngr)->count - 1; index >=0; index--)
		{
			if (((IMAGEFILTERMGNR*)hmlifMngr)->filters[index].pszName) free(((IMAGEFILTERMGNR*)hmlifMngr)->filters[index].pszName);
		}	
		free(((IMAGEFILTERMGNR*)hmlifMngr)->filters);
	}
	free(hmlifMngr);
	return TRUE;
}

static IMAGEFILTER *FindFilter(HMLIMGFLTRMNGR hMngr, const GUID *filterUID)
{
	INT index;
	if (!hMngr || !filterUID) return NULL;
	for (index = ((IMAGEFILTERMGNR*)hMngr)->count - 1; index >=0; index--)
	{
		if (IsEqualGUID(((IMAGEFILTERMGNR*)hMngr)->filters[index].uid, *filterUID)) return &((IMAGEFILTERMGNR*)hMngr)->filters[index];
	}
	return NULL;
}

BOOL MLImageFilterI_Register(HMLIMGFLTRMNGR hmlifMngr, MLIMAGEFILTERINFO_I *pmlif)
{
	IMAGEFILTERMGNR *pMngr;
	IMAGEFILTER *pF;

	if (!hmlifMngr || !pmlif || FindFilter(hmlifMngr, &pmlif->uid) || !pmlif->fnProc) return FALSE;
	pMngr = (IMAGEFILTERMGNR*)hmlifMngr;

	if (pMngr->count == pMngr->allocated)
	{
		LPVOID data;
		data = (IMAGEFILTER*)realloc(pMngr->filters, sizeof(IMAGEFILTER)*(pMngr->allocated + pMngr->allocstep));
		if (!data) return FALSE;
		pMngr->filters = (IMAGEFILTER*)data;
		pMngr->allocated +=  pMngr->allocstep;
	}

	pF = &pMngr->filters[pMngr->count];
	ZeroMemory(pF, sizeof(IMAGEFILTER));
	pF->uid = pmlif->uid;
	pF->proc = pmlif->fnProc;
	if (MLIFF_TITLE_I & pmlif->mask && pmlif->pszTitle)  pF->pszName = _wcsdup(pmlif->pszTitle);
	if (MLIFF_FLAGS_I & pmlif->mask)  pF->flags = pmlif->fFlags;
	if (MLIFF_PARAM_I & pmlif->mask)  pF->param = pmlif->lParam;

	pMngr->count++;
	return TRUE;
}

BOOL MLImageFilterI_Unregister(HMLIMGFLTRMNGR hmlifMngr, REFGUID filterUID)
{
	IMAGEFILTER *pF;
	INT index;
	
	if (!hmlifMngr) return FALSE;

	for (index = ((IMAGEFILTERMGNR*)hmlifMngr)->count - 1; index >=0; index--)
	{
		if (IsEqualGUID(((IMAGEFILTERMGNR*)hmlifMngr)->filters[index].uid, filterUID)) break;
	}
	if (-1 == index) return FALSE;

	pF = &((IMAGEFILTERMGNR*)hmlifMngr)->filters[index];
	if (pF->pszName) free(pF->pszName);

	if (index < ((IMAGEFILTERMGNR*)hmlifMngr)->count - 1)
	{
		MoveMemory(pF, pF + 1, sizeof(IMAGEFILTER)*(((IMAGEFILTERMGNR*)hmlifMngr)->count - index));
	}
	((IMAGEFILTERMGNR*)hmlifMngr)->count--;
	return TRUE;
}

BOOL MLImageFilterI_GetInfo(HMLIMGFLTRMNGR hmlifMngr, MLIMAGEFILTERINFO_I *pmlif)
{
	IMAGEFILTER *pF;

	if (!hmlifMngr || !pmlif) return FALSE;

	pF = FindFilter(hmlifMngr, &pmlif->uid);
	if (!pF) return FALSE;

	if (MLIFF_FLAGS_I & pmlif->mask) pmlif->fFlags = pF->flags;
	if (MLIFF_PARAM_I & pmlif->mask) pmlif->lParam = pF->param;
	if (MLIFF_PROC_I & pmlif->mask) pmlif->fnProc = pF->proc;
	if (MLIFF_TITLE_I & pmlif->mask) 
	{
		if ((!pmlif->pszTitle || pmlif->cchTitleMax < 1)  && pF->pszName) return FALSE;

		if (!pF->pszName)  pmlif->pszTitle[0] = 0x00;
		else if (S_OK != StringCchCopyW(pmlif->pszTitle, pmlif->cchTitleMax, pF->pszName)) return FALSE;
	}

	return TRUE;
}

BOOL MLImageFilterI_ApplyEx(HMLIMGFLTRMNGR hmlifMngr, const GUID *filterUID, LPBYTE pData, LONG cx, LONG cy, INT bpp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag)
{
	IMAGEFILTER *pF;
	pF = FindFilter(hmlifMngr, filterUID);

	if (!pF || !pF->proc || !pData) return FALSE;
	return pF->proc(pData, cx, cy, bpp, rgbBk, rgbFg, imageTag, pF->param);
}

BOOL MLImageFilterI_Apply(HMLIMGFLTRMNGR hmlifMngr, const GUID *filterUID, HBITMAP hbmp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag)
{
	DIBSECTION dibsec;
	
	if (!hbmp || 
		sizeof(DIBSECTION) != GetObjectW(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression ||
		1 != dibsec.dsBmih.biPlanes) return FALSE;

	return MLImageFilterI_ApplyEx(hmlifMngr, filterUID, (LPBYTE)dibsec.dsBm.bmBits,
									dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight, dibsec.dsBm.bmBitsPixel,
									rgbBk, rgbFg, imageTag);
	
}
