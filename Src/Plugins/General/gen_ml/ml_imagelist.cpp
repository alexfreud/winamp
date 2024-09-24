#include "./ml_imagelist.h"
#include "../nu/trace.h"

typedef struct _MLILSTATE
{
	INT			ilIndex;
	COLORREF	rgbBk;
	COLORREF	rgbFg;

	UINT		created;
	UINT		counter;
} MLILSTATE;

typedef struct  _MLILREC
{
	MLIMAGESOURCE_I	imgSource;
	GUID			filterUID;
	UINT			filterFlags;
	INT_PTR			tag;

	MLILSTATE		*states;
	INT				usedCount;
	INT				lastIndex;
	UINT			counter;
} MLILREC;

typedef struct _MLIL
{
	HIMAGELIST		ilImages;
    INT				maxCache;
	INT				imageWidth;
	INT				imageHeight;
	UINT			flags;

	MLILREC			*records;
	INT				recCount;
	INT				recAllocated;
	INT				recGrow;
	HMLIMGFLTRMNGR	hmlifMngr;
} MLIL;

static BOOL MLImageListI_PatchHILIndices(HMLIMGLST hmlil, INT hilIndex) // scan for all records - states  and subtract 1 from all indices that higher than hilIndex.
{
	INT index, stateIdx;
	MLIL *pmlil;
	if (!hmlil || hilIndex < 0 ) return FALSE;
	pmlil = (MLIL*)hmlil;

	for (index = 0; index < pmlil->recCount; index++)
	{
		if (pmlil->records[index].states)
		{
			for (stateIdx = 0; stateIdx < pmlil->records[index].usedCount; stateIdx++)
			{
				if (pmlil->records[index].states[stateIdx].ilIndex > hilIndex)	pmlil->records[index].states[stateIdx].ilIndex--;
			}
		}
	}
	return TRUE;
}

HMLIMGLST MLImageListI_Create(INT cx, INT cy, UINT flags, INT cInitial, INT cGrow, INT cCacheSize, HMLIMGFLTRMNGR hmlifManager)
{
	if (!cx || !cy || !cCacheSize) return NULL;

	MLIL *pmlil = (MLIL*)calloc(1, sizeof(MLIL));
	if (!pmlil) return NULL;

	BOOL fSuccess = TRUE;
	if (!(0x38 & pmlil->flags)) pmlil->flags |= MLILC_COLOR24_I;

	pmlil->imageWidth	= cx;
	pmlil->imageHeight	= cy;
	pmlil->flags		= flags;
	pmlil->recAllocated	= (cGrow < 0 ) ? 0 : ((cInitial > MAX_ALLOWED_LIST_SIZE) ? MAX_ALLOWED_LIST_SIZE : cInitial);
	pmlil->recGrow		= (cGrow < 1 ) ? 1 : ((cGrow > MAX_ALLOWED_LIST_SIZE) ? MAX_ALLOWED_LIST_SIZE : cGrow);
	pmlil->maxCache		= (cCacheSize > MAX_ALLOWED_CACHE_SIZE) ? MAX_ALLOWED_CACHE_SIZE : cCacheSize;
	pmlil->hmlifMngr	= hmlifManager;

	if (pmlil->recAllocated)
	{
		pmlil->records = (MLILREC*)calloc(pmlil->recAllocated, sizeof(MLILREC));
		if (!pmlil->records) fSuccess = FALSE;
		else
		{
			pmlil->ilImages = ImageList_Create(pmlil->imageWidth, pmlil->imageHeight, pmlil->flags, pmlil->recAllocated * pmlil->maxCache, pmlil->recGrow * pmlil->maxCache);
			if (!pmlil->ilImages) fSuccess = FALSE;
		}
	}

	if (!fSuccess && pmlil)
	{
		if (pmlil->records) free(pmlil->records);
		if (pmlil->ilImages) ImageList_Destroy(pmlil->ilImages);
		free(pmlil);
		pmlil = NULL;
	}
	return (HMLIMGLST)pmlil;
}

BOOL MLImageListI_Destroy(HMLIMGLST hmlil)
{
	if (!hmlil) return FALSE;

	if (((MLIL*)hmlil)->records)
	{
		for (INT index = 0; index < ((MLIL*)hmlil)->recCount; index++) 
		{
			if (((MLIL*)hmlil)->records[index].states) free(((MLIL*)hmlil)->records[index].states);
			MLImageLoaderI_FreeData(&((MLIL*)hmlil)->records[index].imgSource);
		}
		free(((MLIL*)hmlil)->records);
	}
	if (((MLIL*)hmlil)->ilImages) ImageList_Destroy(((MLIL*)hmlil)->ilImages);
	free(hmlil);
	return TRUE;
}

INT MLImageListI_Add(HMLIMGLST hmlil, MLIMAGESOURCE_I *pImageSource, REFGUID filterUID, INT_PTR nTag)
{
	MLIL *pmlil;
	MLIMAGEFILTERINFO_I fi;

	if (!hmlil || !pImageSource) return -1;

	pmlil = (MLIL*)hmlil;

	if (pmlil->recCount == pmlil->recAllocated)
	{
		LPVOID data;
		data  = realloc(pmlil->records, sizeof(MLILREC)* (pmlil->recAllocated + pmlil->recGrow));
		if (!data) return -1;
		pmlil->records = (MLILREC*)data;
		pmlil->recAllocated += pmlil->recGrow; 

		if (!pmlil->ilImages)
		{
			pmlil->ilImages = ImageList_Create(pmlil->imageWidth, pmlil->imageHeight, pmlil->flags,
												pmlil->recAllocated * pmlil->maxCache, pmlil->recGrow * pmlil->maxCache);
		}
	}

	if (!pmlil->ilImages) return -1;

	ZeroMemory(&pmlil->records[pmlil->recCount], sizeof(MLILREC));
	if (!MLImageLoaderI_CopyData(&pmlil->records[pmlil->recCount].imgSource, pImageSource)) return -1;

	pmlil->records[pmlil->recCount].filterUID	= filterUID;
	pmlil->records[pmlil->recCount].tag			= nTag;
	pmlil->records[pmlil->recCount].lastIndex	= -1;

	fi.uid = filterUID;
	fi.mask = MLIFF_FLAGS_I;
	pmlil->records[pmlil->recCount].filterFlags = (MLImageFilterI_GetInfo(pmlil->hmlifMngr, &fi)) ? fi.fFlags : 0;

	return pmlil->recCount++;
}

BOOL MLImageListI_Replace(HMLIMGLST hmlil, INT index, MLIMAGESOURCE_I *pImageSource, REFGUID filterUID, INT_PTR nTag)
{
	MLIL *pmlil;
	MLIMAGEFILTERINFO_I fi;

	if (!hmlil || !pImageSource) return FALSE;

	pmlil = (MLIL*)hmlil;
	if (index < 0 || index >= pmlil->recCount) return FALSE;

	MLImageLoaderI_FreeData(&pmlil->records[index].imgSource);
	if (!MLImageLoaderI_CopyData(&pmlil->records[index].imgSource, pImageSource)) return -1;

	if (pmlil->records[index].states) 
	{
		free(pmlil->records[index].states);
		pmlil->records[index].states = NULL;
	}

	pmlil->records[index].filterUID = filterUID;
	pmlil->records[index].tag		= nTag;
	pmlil->records[index].lastIndex	= -1;
	pmlil->records[index].usedCount = 0;
	pmlil->records[index].counter	= 0;

	fi.uid = filterUID;
	fi.mask = MLIFF_FLAGS_I;
	pmlil->records[index].filterFlags = (MLImageFilterI_GetInfo(pmlil->hmlifMngr, &fi)) ? fi.fFlags : 0;

	return TRUE;
}

BOOL MLImageListI_Remove(HMLIMGLST hmlil, INT index)
{
	MLIL *pmlil;
	if (!hmlil) return FALSE;

	pmlil = (MLIL*)hmlil;
	if (index < 0 || index >= pmlil->recCount) return FALSE;

	MLImageLoaderI_FreeData(&pmlil->records[index].imgSource);

	if (pmlil->records[index].states) 
	{
		INT stateIdx;
		for (stateIdx = 0; stateIdx < pmlil->records[index].usedCount; stateIdx++)
		{
			INT hilIdx = pmlil->records[index].states[stateIdx].ilIndex;
			if (-1 != hilIdx && ImageList_Remove(pmlil->ilImages, hilIdx)) MLImageListI_PatchHILIndices(hmlil, hilIdx);
		}
		free(pmlil->records[index].states);
	}

	if (index != (pmlil->recCount - 1)) MoveMemory(&pmlil->records[index], &pmlil->records[index + 1], sizeof(MLILREC)*(pmlil->recCount - index));

	pmlil->recCount--;
	return TRUE;
}

HIMAGELIST MLImageListI_GetRealList(HMLIMGLST hmlil)
{
	return (hmlil) ? ((MLIL*)hmlil)->ilImages : NULL;
}

INT MLImageListI_GetRealIndex(HMLIMGLST hmlil, INT index, COLORREF rgbBk, COLORREF rgbFg)
{
	INT		realIndex;
	MLIL		*pmlil;
	MLILREC	*prec;
	HBITMAP	hbmp;

	pmlil = (MLIL*)hmlil;
	if (!pmlil || index < 0 || index >= pmlil->recCount) 
		return -1;

	prec = &pmlil->records[index];

	if (MLIFF_IGNORE_FGCOLOR_I & prec->filterFlags) 
		rgbFg = 0xFFFF00FF;

	if (MLIFF_IGNORE_BKCOLOR_I & prec->filterFlags) 
		rgbBk = 0xFFFF00FF;

	if (pmlil->ilImages && prec->states && prec->usedCount > 0)
	{
		if (prec->lastIndex >= 0 && prec->lastIndex < prec->usedCount && 
			prec->states[prec->lastIndex].rgbBk == rgbBk && prec->states[prec->lastIndex].rgbFg == rgbFg) realIndex = prec->lastIndex;
		else
		{
			for (realIndex = 0; realIndex < prec->usedCount; realIndex++)
			{
				if (prec->states[realIndex].rgbBk == rgbBk && prec->states[realIndex].rgbFg == rgbFg)
				{
					prec->lastIndex = realIndex;
					break;
				}
			}
		}

		if (prec->lastIndex == realIndex && -1 != prec->states[realIndex].ilIndex)
		{
			prec->counter++;
			prec->states[realIndex].counter++;
			return prec->states[realIndex].ilIndex;
		}
	}

	hbmp = MLImageLoaderI_LoadDib(&prec->imgSource);
	if (!hbmp) 
		return -1;

	if (!MLImageFilterI_Apply(pmlil->hmlifMngr, &prec->filterUID, hbmp, rgbBk, rgbFg, prec->tag))
	{
		rgbFg = 0xFFFF00FF;
		rgbBk = 0xFFFF00FF;
	}

	if (!prec->states)
	{
		prec->states = (MLILSTATE*)calloc(pmlil->maxCache, sizeof(MLILSTATE));
		if (!prec->states)
		{
			DeleteObject(hbmp);
			return -1;
		}
		prec->counter = 0;
		prec->lastIndex = -1;
		prec->usedCount = 0;
	}
	if (prec->usedCount < pmlil->maxCache)
	{
		realIndex = prec->usedCount++;
		prec->states[realIndex].ilIndex = -1;
	}
	else
	{ /// lets find less used record
		INT minVal, minIndex;
		minIndex = 0;
		minVal = (prec->counter != prec->states[minIndex].created) ? 
					(prec->counter - prec->states[minIndex].created) : 
					1;

		minVal = MulDiv(100000, prec->states[minIndex].counter, minVal);
		
		for (realIndex = 0; realIndex < prec->usedCount; realIndex++)
		{
			INT testVal = (prec->counter != prec->states[realIndex].created) ? 
						(prec->counter - prec->states[realIndex].created) :
						1;

			testVal = MulDiv(100000, prec->states[realIndex].counter, testVal);

			if (testVal < minVal) 
			{ 
				minVal = testVal; 
				minIndex = realIndex; 
			}
		}
		realIndex = minIndex;
	}

	prec->states[realIndex].rgbBk = rgbBk;
	prec->states[realIndex].rgbFg = rgbFg;
	prec->states[realIndex].counter = 1;
	prec->states[realIndex].created = prec->counter;

	if(NULL != pmlil->ilImages)
	{
		if (-1 == prec->states[realIndex].ilIndex) 
			prec->states[realIndex].ilIndex = ImageList_Add(pmlil->ilImages, hbmp, NULL);
		else 
			ImageList_Replace(pmlil->ilImages, prec->states[realIndex].ilIndex, hbmp, NULL);
	}

	DeleteObject(hbmp);
	if (-1 != prec->states[realIndex].ilIndex) 
	{
		prec->counter++;
		prec->lastIndex = realIndex;
	}

	return prec->states[realIndex].ilIndex;
}

BOOL MLImageListI_GetImageSize(HMLIMGLST hmlil, INT *cx, INT *cy)
{
	if (!hmlil)
	{
		if (cx) *cx = 0;
		if (cy) *cy = 0;
		return FALSE;
	}
	if (cx) *cx = ((MLIL*)hmlil)->imageWidth;
	if (cy) *cy = ((MLIL*)hmlil)->imageHeight;
	return TRUE;
}

INT MLImageListI_GetImageCount(HMLIMGLST hmlil)
{
	return  (hmlil) ? ((MLIL*)hmlil)->recCount : 0;
}

INT MLImageListI_GetIndexFromTag(HMLIMGLST hmlil, INT_PTR nTag)
{
	INT index;
	if (!hmlil || !((MLIL*)hmlil)->records) return -1;
	for (index = 0; index <((MLIL*)hmlil)->recCount; index++)
	{
		if (((MLIL*)hmlil)->records[index].tag == nTag) return index;
	}
	return -1;
}

BOOL MLImageListI_GetTagFromIndex(HMLIMGLST hmlil, INT index, INT_PTR *nTag)
{
	if (!hmlil || !((MLIL*)hmlil)->records || index < 0 || index >= ((MLIL*)hmlil)->recCount)  return FALSE;
	if (nTag) *nTag = ((MLIL*)hmlil)->records[index].tag;
	return TRUE;
}

BOOL MLImageListI_CheckItemExist(HMLIMGLST hmlil, INT index)
{
	MLIL	 *pmlil = (MLIL*)hmlil;
	if (NULL == pmlil || index < 0 || index >= pmlil->recCount) return FALSE;
	return MLImageLoaderI_CheckExist(&pmlil->records[index].imgSource);
}