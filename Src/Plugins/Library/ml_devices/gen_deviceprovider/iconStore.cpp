#include "main.h"
#include "./iconStore.h"

#include <vector>


typedef struct IconStoreRecord
{
	unsigned int width;
	unsigned int height;
	wchar_t *path;
} IconStoreRecord;

typedef std::vector<IconStoreRecord> RecodList;

struct IconStore
{
	RecodList list;
	wchar_t *basePath;
};

IconStore *
IconStore_Create()
{
	IconStore *self;
	
	self = new IconStore();
	if (NULL == self)
		return NULL;

	self->basePath = NULL;
	return self;
}

void
IconStore_Destroy(IconStore *self)
{
	size_t index;
	IconStoreRecord *record;

	if (NULL == self)
		return;

	index = self->list.size();
	while(index--)
	{
		record = &self->list[index];
		String_Free(record->path);
	}

	String_Free(self->basePath);
}

BOOL
IconStore_Add(IconStore *self, const wchar_t *path, unsigned int width, unsigned int height)
{
	IconStoreRecord record, *prec;
	size_t index;

	if (NULL == self)
		return FALSE;
	
	if(width < 1)
		width = 1;

	if(height < 1)
		height = 1;
	
	index = self->list.size();
	while(index--)
	{
		prec = &self->list[index];
		if (width == prec->width &&
			height == prec->height)
		{
			String_Free(prec->path);
			prec->path = String_Duplicate(path);
			return TRUE;
		}
	}

	record.path = String_Duplicate(path);
	record.width = width;
	record.height = height;

	self->list.push_back(record);

	return TRUE;
}

BOOL
IconStore_RemovePath(IconStore *self, const wchar_t *path)
{
	size_t index;
	IconStoreRecord *record;

	if (NULL == self)
		return FALSE;

	index = self->list.size();
	while(index--)
	{
		record = &self->list[index];
		if(CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, record->path, -1, path, -1))
		{
			self->list.eraseAt(index);
		}
	}

	return TRUE;
}

BOOL
IconStore_Remove(IconStore *self, unsigned int width, unsigned int height)
{
	size_t index;
	IconStoreRecord *record;

	if (NULL == self)
		return FALSE;

	index = self->list.size();
	while(index--)
	{
		record = &self->list[index];
		if(record->width == width &&
			record->height == height)
		{
			self->list.eraseAt(index);
		}
	}

	return TRUE;
}

static BOOL
IconStore_GetFullPath(IconStore *self, const wchar_t *path, wchar_t *buffer, size_t bufferMax)
{
	if (NULL == buffer)
		return FALSE;

	if (NULL == self)
		return FALSE;

	if (FALSE != IS_STRING_EMPTY(path))
	{
		*buffer = L'\0';
		return TRUE;
	}

	if (FALSE == PathIsRelative(path) || 
		IS_STRING_EMPTY(self->basePath))
	{
		if (0 == String_CopyTo(buffer, path, bufferMax) && 
			FALSE == IS_STRING_EMPTY(path))
			return FALSE;
		else
			return TRUE;
	}

	if (NULL == PathCombine(buffer, self->basePath, path))
		return FALSE;

	return TRUE;
}

BOOL
IconStore_Get(IconStore *self, wchar_t *buffer, size_t bufferMax, unsigned int width, unsigned int height)
{
	const wchar_t *icon;
	IconStoreRecord *record;
	size_t index;
	double widthDbl, heightDbl;
	double scaleMin, scaleHorz, scaleVert;

	if (NULL == self)
		return FALSE;

	if (NULL == buffer)
		return FALSE;

	icon = NULL;
	
	widthDbl = width;
	heightDbl = height;

	index = self->list.size();
	if (index  > 0)
	{
		record = &self->list[--index];
		scaleHorz = widthDbl/record->width;
		scaleVert = heightDbl/record->height;
		scaleMin = (scaleHorz < scaleVert) ? scaleHorz : scaleVert;
		icon = record->path;
		if (1.0 != scaleMin)
		{
			scaleMin = fabs(1.0 - scaleMin);
			while(index--)
			{
				record = &self->list[index];
				scaleHorz = widthDbl/record->width;
				scaleVert = heightDbl/record->height;
				if (scaleHorz > scaleVert) 
					scaleHorz = scaleVert;
				
				if (1.0 == scaleHorz)
				{
					icon = record->path;
					break;
				}
				
				scaleHorz = fabs(1.0 - scaleHorz);
				if (scaleHorz < scaleMin)
				{
					scaleMin = scaleHorz;
					icon = record->path;
				}
			}
		}
	}

	return IconStore_GetFullPath(self, icon, buffer, bufferMax);
}

BOOL
IconStore_SetBasePath(IconStore *self, const wchar_t *path)
{
	if (NULL == self)
		return FALSE;

	String_Free(self->basePath);
	self->basePath = String_Duplicate(path);

	return TRUE;
}

IconStore *
IconStore_Clone(IconStore *self)
{
	size_t index;
	IconStore *clone;
	IconStoreRecord targetRec;
	const IconStoreRecord *sourceRec;
	
	if (NULL == self)
		return NULL;

	clone = IconStore_Create();
	if (NULL == clone)
		return NULL;

	clone->basePath = String_Duplicate(self->basePath);
	for(index = 0; index < self->list.size(); index++)
	{
		sourceRec = &self->list[index];
		targetRec.path = String_Duplicate(sourceRec->path);
		targetRec.width = sourceRec->width;
		targetRec.height = sourceRec->height;
		clone->list.push_back(targetRec);
	}

	return clone;
}


BOOL
IconStore_Enumerate(IconStore *self, IconEnumerator callback, void *user)
{
	size_t index;
	wchar_t buffer[MAX_PATH*2];
	IconStoreRecord *record;

	if (NULL == self || NULL == callback)
		return FALSE;

	index = self->list.size();
	while(index--)
	{
		record = &self->list[index];
		if (FALSE != IconStore_GetFullPath(self, record->path, buffer, ARRAYSIZE(buffer)))
		{
			if (FALSE == callback(buffer, record->width, record->height, user))
				return TRUE;
		}
	}

	return TRUE;

}
