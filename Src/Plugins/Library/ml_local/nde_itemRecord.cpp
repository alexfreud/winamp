/*
** Copyright © 2003-2014 Winamp SA
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
*/

#include <windows.h>
#include "../../General/gen_ml/ml.h"
#include "../nde/nde.h"
#include "../nu/sort.h"

void freeRecord(itemRecord *p)
{
	if (p)
	{
		free(p->title);
		free( p->artist );
		free( p->ext );
		free(p->comment);
		free(p->album);
		free(p->genre);
		free(p->filename);
		if (p->extended_info)
		{
			for (size_t x = 0; p->extended_info[x]; x ++)
				free(p->extended_info[x]);
			free(p->extended_info);
			p->extended_info = 0;
		}
	}
}

void freeRecordList(itemRecordList *obj)
{
	if (obj)
	{
		emptyRecordList(obj);
		_aligned_free(obj->Items);
		obj->Items=0;
		obj->Alloc=obj->Size=0;
	}
}

void emptyRecordList(itemRecordList *obj)
{
	if (obj)
	{
		itemRecord *p=obj->Items;
		while (obj->Size-->0)
		{
			freeRecord(p);
			p++;
		}
		obj->Size=0;
	}
}

void allocRecordList(itemRecordList *obj, int newsize, int granularity)
{
	if (newsize < obj->Alloc || newsize < obj->Size) return;

	int old_Alloc = obj->Alloc;
	obj->Alloc=newsize+granularity;

	itemRecord *data = (itemRecord*)_aligned_realloc(obj->Items, sizeof(itemRecord) * obj->Alloc, 16);
	if (!data)
	{
		data = (itemRecord*)_aligned_malloc(sizeof(itemRecord) * obj->Alloc, 16);
		if (data)
		{
			memcpy(data, obj->Items, sizeof(itemRecord) * old_Alloc);
			_aligned_free(obj->Items);
			obj->Items = data;
		}
		else
			obj->Alloc = old_Alloc;
	}
	else
		obj->Items = data;

	if (!obj->Items) obj->Alloc=0;
}

void compactRecordList(itemRecordListW *obj)
{
	if (obj->Size && obj->Size < obj->Alloc - 1024)
	{
		size_t old_Alloc = obj->Size;
		obj->Alloc = obj->Size;

		itemRecordW *data = (itemRecordW *)_aligned_realloc(obj->Items, sizeof(itemRecordW) * obj->Alloc, 16);
		if (!data)
		{
			data = (itemRecordW *)_aligned_malloc(sizeof(itemRecordW) * obj->Alloc, 16);
			if (data)
			{
				memcpy(data, obj->Items, sizeof(itemRecordW) * old_Alloc);
				_aligned_free(obj->Items);
				obj->Items = data;
			}
			else
				obj->Alloc = old_Alloc;
		}
		else
			obj->Items = data;
	}
}

void copyRecord(itemRecord *out, const itemRecord *in)
{
#define COPYSTR(FOO) out->FOO = in->FOO ? _strdup(in->FOO) : 0;
	COPYSTR(filename)
	COPYSTR(title)
	COPYSTR(ext)
	COPYSTR(album)
	COPYSTR(artist)
	COPYSTR(comment)
	COPYSTR(genre)
	out->year=in->year;
	out->track=in->track;
	out->length=in->length;
#undef COPYSTR
	out->extended_info=0;

	if (in->extended_info)
	{
		for (int y = 0; in->extended_info[y]; y ++)
		{
			char *p=in->extended_info[y];
			if (*p) setRecordExtendedItem(out,p,p+strlen(p)+1);
		}
	}
}

void copyRecordList(itemRecordList *out, const itemRecordList *in)
{
	allocRecordList(out,out->Size+in->Size,0);
	if (!out->Items) return;
	for (int x = 0; x < in->Size; x ++)
	{
		copyRecord(&out->Items[out->Size++],&in->Items[x]);
	}
}

char *getRecordExtendedItem(const itemRecord *item, const char *name)
{
	if (item->extended_info)
	{
		for (size_t x = 0; item->extended_info[x]; x ++)
		{
			if (!_stricmp(item->extended_info[x],name))
				return item->extended_info[x]+strlen(name)+1;
		}
	}
	return NULL;
}

void setRecordExtendedItem(itemRecord *item, const char *name, char *value)
{
	if (!item || !name) return;

	size_t x = 0;
	if (item->extended_info)
	{
		for (x = 0; item->extended_info[x]; x ++)
		{
			if (!_stricmp(item->extended_info[x],name))
			{
				size_t name_len = strlen(name), value_len = strlen(value);
				if (value_len>strlen(item->extended_info[x]+name_len+1))
				{
					free(item->extended_info[x]);
					item->extended_info[x]=(char*)malloc(name_len+value_len+2);
				}
				strncpy(item->extended_info[x], name, name_len);
				strncpy(item->extended_info[x]+strlen(name)+1, value, value_len);
				return;
			}
		}
	}

	// x=number of valid items.
	char **data = (char**)realloc(item->extended_info,sizeof(char*) * (x+2));
	if (data)
	{
		// if we could allocate then add, otherwise we'll have to skip
		item->extended_info = data;

		size_t name_len = strlen(name), value_len = strlen(value);
		item->extended_info[x]=(char*)malloc(name_len+value_len+2);
		strncpy(item->extended_info[x], name, name_len);
		strncpy(item->extended_info[x]+name_len+1, value, value_len);

		item->extended_info[x+1]=0;
	}
	else
	{
		data = (char**)malloc(sizeof(char*) * (x+2));
		if (data)
		{
			memcpy(data, item->extended_info, sizeof(char*) * x);
			free(item->extended_info);

			// if we could allocate then add, otherwise we'll have to skip
			item->extended_info = data;

			size_t name_len = strlen(name), value_len = strlen(value);
			item->extended_info[x]=(char*)malloc(name_len+value_len+2);
			strncpy(item->extended_info[x], name, name_len);
			strncpy(item->extended_info[x]+name_len+1, value, value_len);

			item->extended_info[x+1]=0;
		}
	}
}

/*
---------------------------------- 
wide version starts here
---------------------------------- 
*/
void freeRecord(itemRecordW *p)
{
	if (p)
	{
		ndestring_release(p->title);
		ndestring_release(p->artist);
		ndestring_release(p->comment);
		ndestring_release(p->album);
		ndestring_release(p->genre);
		ndestring_release(p->filename);
		ndestring_release(p->albumartist); 
		ndestring_release(p->replaygain_album_gain); 
		ndestring_release(p->replaygain_track_gain); 
		ndestring_release(p->publisher);
		ndestring_release(p->composer);
		if (p->extended_info)
		{
			for (size_t x = 0; p->extended_info[x].key; x ++)
			{
				// TODO release 'key' ?
//				ndestring_release(p->extended_info[x].key);
				ndestring_release(p->extended_info[x].value);
			}
			free(p->extended_info);
			p->extended_info = 0;
		}
	}
}

void freeRecordList(itemRecordListW *obj)
{
	if (obj)
	{
		emptyRecordList(obj);
		_aligned_free(obj->Items);
		obj->Items=0;
		obj->Alloc=obj->Size=0;
	}
}

void emptyRecordList(itemRecordListW *obj)
{
	if (obj)
	{
		itemRecordW *p=obj->Items;
		while (obj->Size-->0)
		{
			freeRecord(p);
			p++;
		}
		obj->Size=0;
	}
}

void allocRecordList(itemRecordListW *obj, int newsize, int granularity)
{
	if (newsize < obj->Alloc || newsize < obj->Size) return;

	int old_Alloc = obj->Alloc;
	obj->Alloc=newsize+granularity;
	itemRecordW *data = (itemRecordW*)_aligned_realloc(obj->Items,sizeof(itemRecordW)*obj->Alloc, 16);
	if (!data)
	{
		data = (itemRecordW*)_aligned_malloc(sizeof(itemRecordW) * obj->Alloc, 16);
		if (data)
		{
			memcpy(data, obj->Items, sizeof(itemRecordW) * obj->Alloc);
			_aligned_free(obj->Items);
			obj->Items = data;
		}
		else
			obj->Alloc = old_Alloc;
	}
	else
		obj->Items = data;

	if (!obj->Items) obj->Alloc=0;
}

#if 0 // unused, don't re-enable until you verify the TODO below
void copyRecord(itemRecordW *out, const itemRecordW *in)
{
#define COPYSTR(FOO) out->FOO = in->FOO ? _wcsdup(in->FOO) : 0;
#define COPY(FOO) out->FOO = in->FOO;
	/* this is only valid if 'in' is one of our item records! */
	out->filename = in->filename;
	NDEString *ndeString = WCHAR_TO_NDESTRING(out->filename);
	if (ndeString) ndeString->Retain();
	COPYSTR(title)
	COPYSTR(album)
	COPYSTR(artist)
	COPYSTR(comment)
	COPYSTR(genre)
	COPYSTR(albumartist); 
	COPYSTR(replaygain_album_gain); 
	COPYSTR(replaygain_track_gain); 
	COPYSTR(publisher);
	COPYSTR(composer);
	COPY(year);
	COPY(track);
	COPY(tracks);
	COPY(length);
	COPY(rating);
	COPY(playcount); 
	COPY(lastplay); 
	COPY(lastupd); 
	COPY(filetime);
	COPY(filesize);
	COPY(bitrate); 
	COPY(type); 
	COPY(disc); 
	COPY(discs);
	COPY(bpm);
	COPYSTR(category);
#undef COPYSTR
	out->extended_info=0;

	if (in->extended_info)
	{
		for (int y = 0; in->extended_info[y].key; y ++)
		{
			setRecordExtendedItem(out,in->extended_info[y].key,in->extended_info[y].value);
		}
	}
}

void copyRecordList(itemRecordListW *out, const itemRecordListW *in)
{
	allocRecordList(out,out->Size+in->Size,0);
	if (!out->Items) return;
	for (size_t x = 0; x < in->Size; x ++)
	{
		copyRecord(&out->Items[out->Size++],&in->Items[x]);
	}
}
#endif

wchar_t *getRecordExtendedItem(const itemRecordW *item, const wchar_t *name)
{
	if (item && item->extended_info && name)
	{
		for (size_t x = 0; item->extended_info[x].key; x ++)
		{
			if (!_wcsicmp(item->extended_info[x].key,name))
				return item->extended_info[x].value;
		}
	}
	return NULL;
}

wchar_t *getRecordExtendedItem_fast(const itemRecordW *item, const wchar_t *name)
{
	if (item && item->extended_info && name)
	{
		for (size_t x = 0; item->extended_info[x].key; x ++)
		{
			if (item->extended_info[x].key == name)
				return item->extended_info[x].value;
		}
	}
	return NULL;
}

void setRecordExtendedItem(itemRecordW *item, const wchar_t *name, const wchar_t *value)
{
	if (!item || !name) return;

	size_t x=0;
	if (item->extended_info) for (x = 0; item->extended_info[x].key; x ++)
	{
		if (item->extended_info[x].key == name)
		{
			ndestring_retain(const_cast<wchar_t *>(value));
			ndestring_release(item->extended_info[x].value);
			item->extended_info[x].value = const_cast<wchar_t *>(value);
			return;
		}
	}

	// x=number of valid items.
	extendedInfoW *data=(extendedInfoW *)realloc(item->extended_info,sizeof(extendedInfoW) * (x+2));
	if (data)
	{
		item->extended_info=data;

		item->extended_info[x].key = const_cast<wchar_t *>(name);
		ndestring_retain(const_cast<wchar_t *>(value));
		item->extended_info[x].value = const_cast<wchar_t *>(value);

		item->extended_info[x+1].key=0;
		item->extended_info[x+1].value=0;
	}
	else
	{
		data=(extendedInfoW *)malloc(sizeof(extendedInfoW) * (x+2));
		if (data)
		{
			item->extended_info=data;
	
			item->extended_info[x].key = const_cast<wchar_t *>(name);
			ndestring_retain(const_cast<wchar_t *>(value));
			item->extended_info[x].value = const_cast<wchar_t *>(value);
	
			item->extended_info[x+1].key=0;
			item->extended_info[x+1].value=0;
		}
	}
}

// this version assumes that the 'name' won't already be in the itemRecord
void setRecordExtendedItem_fast(itemRecordW *item, const wchar_t *name, const wchar_t *value)
{
	if (!item || !name) return;

	size_t x = 0;
	if (item->extended_info)
	{
		for (x = 0; item->extended_info[x].key; x++)
		{
		}
	}

	// x=number of valid items.
	extendedInfoW *data=(extendedInfoW *)realloc(item->extended_info,sizeof(extendedInfoW) * (x+2));
	if (data)
	{
		item->extended_info = data;
		item->extended_info[x].key = const_cast<wchar_t *>(name);
		ndestring_retain(const_cast<wchar_t *>(value));
		item->extended_info[x].value = const_cast<wchar_t *>(value);

		item->extended_info[x+1].key=0;
		item->extended_info[x+1].value=0;
	}
	else
	{
		data=(extendedInfoW *)malloc(sizeof(extendedInfoW) * (x+2));
		if (data)
		{
			item->extended_info=data;
	
			item->extended_info[x].key = const_cast<wchar_t *>(name);
			ndestring_retain(const_cast<wchar_t *>(value));
			item->extended_info[x].value = const_cast<wchar_t *>(value);
	
			item->extended_info[x+1].key=0;
			item->extended_info[x+1].value=0;
		}
	}
}

// TODO: redo this without AutoChar
#include "../replicant/nu/AutoChar.h"
#include "../nu/AutoCharFn.h"
#define COPY_EXTENDED_STR(field) if (input-> ## field && input-> ## field ## [0]) setRecordExtendedItem(output, #field, AutoChar(input-> ## field));
#define COPY_EXTENDED_INT(field) if (input->##field > 0) { char temp[64] = {0}; _itoa(input->##field, temp, 10); setRecordExtendedItem(output, #field, temp); }
#define COPY_EXTENDED_INT64(field) if (input->##field > 0) { char temp[64] = {0}; _i64toa(input->##field, temp, 10); setRecordExtendedItem(output, #field, temp); }
#define COPY_EXTENDED_INT0(field) if (input->##field >= 0) { char temp[64] = {0}; _itoa(input->##field, temp, 10); setRecordExtendedItem(output, #field, temp); }
void convertRecord(itemRecord *output, const itemRecordW *input)
{
	output->filename=_strdup(AutoCharFn(input->filename));
	output->title=AutoCharDup(input->title);
	output->ext=AutoCharDup(input->ext);
	output->album=AutoCharDup(input->album);
	output->artist=AutoCharDup(input->artist);
	output->comment=AutoCharDup(input->comment);
	output->genre=AutoCharDup(input->genre);
	output->year=input->year;
	output->track=input->track;
	output->length=input->length;
	output->extended_info=0;	
	COPY_EXTENDED_STR(albumartist);
	COPY_EXTENDED_STR(replaygain_album_gain);
	COPY_EXTENDED_STR(replaygain_track_gain);
	COPY_EXTENDED_STR(publisher);
	COPY_EXTENDED_STR(composer);
	COPY_EXTENDED_INT(tracks);
	COPY_EXTENDED_INT(rating);
	COPY_EXTENDED_INT(playcount);
	COPY_EXTENDED_INT64(lastplay);
	COPY_EXTENDED_INT64(lastupd);
	COPY_EXTENDED_INT64(filetime);
	COPY_EXTENDED_INT(filesize);
	COPY_EXTENDED_INT(bitrate);
	COPY_EXTENDED_INT0(type);
	COPY_EXTENDED_INT(disc);
	COPY_EXTENDED_INT(discs);
	COPY_EXTENDED_INT(bpm);
	COPY_EXTENDED_STR(category);

	if (input->extended_info)
	{
		for (int y = 0; input->extended_info[y].key; y ++)
		{
			setRecordExtendedItem(output, AutoChar(input->extended_info[y].key), AutoChar(input->extended_info[y].value));
		}
	}
}
#undef COPY_EXTENDED_STR
#undef COPY_EXTENDED_INT
#undef COPY_EXTENDED_INT0

#include "../replicant/nu/AutoWide.h"
#define COPY_EXTENDED_STR(field) output->##field = ndestring_wcsdup(AutoWideDup(getRecordExtendedItem(input, #field)));
#define COPY_EXTENDED_INT(field) { char *x = getRecordExtendedItem(input, #field); output->##field=x?atoi(x):-1; }

void convertRecord(itemRecordW *output, const itemRecord *input)
{
	output->filename=ndestring_wcsdup(AutoWideDup(input->filename));
	output->title=ndestring_wcsdup(AutoWideDup(input->title));
	output->ext=ndestring_wcsdup(AutoWideDup(input->ext));
	output->album=ndestring_wcsdup(AutoWideDup(input->album));
	output->artist=ndestring_wcsdup(AutoWideDup(input->artist));
	output->comment=ndestring_wcsdup(AutoWideDup(input->comment));
	output->genre=ndestring_wcsdup(AutoWideDup(input->genre));
	output->year=input->year;
	output->track=input->track;
	output->length=input->length;
	output->extended_info=0;
	COPY_EXTENDED_STR(albumartist);
	COPY_EXTENDED_STR(replaygain_album_gain);
	COPY_EXTENDED_STR(replaygain_track_gain);
	COPY_EXTENDED_STR(publisher);
	COPY_EXTENDED_STR(composer);
	COPY_EXTENDED_INT(tracks);
	COPY_EXTENDED_INT(rating);
	COPY_EXTENDED_INT(playcount);
	COPY_EXTENDED_INT(lastplay);
	COPY_EXTENDED_INT(lastupd);
	COPY_EXTENDED_INT(filetime);
	COPY_EXTENDED_INT(filesize);
	COPY_EXTENDED_INT(type);
	COPY_EXTENDED_INT(disc);
	COPY_EXTENDED_INT(discs);
	COPY_EXTENDED_INT(bpm);
	COPY_EXTENDED_INT(bitrate);
	COPY_EXTENDED_STR(composer);
	COPY_EXTENDED_STR(category);
	// TODO: copy input's extended fields
}
#undef COPY_EXTENDED_STR
#undef COPY_EXTENDED_INT

void convertRecordList(itemRecordList *output, const itemRecordListW *input)
{
	output->Alloc = output->Size = input->Size;
	output->Items = (itemRecord*)_aligned_malloc(sizeof(itemRecord)*input->Size, 16);
	if (output->Items)
	{
		memset(output->Items, 0, sizeof(itemRecord)*input->Size);
		for(int i=0; i < input->Size; i++)
		{
			convertRecord(&output->Items[i],&input->Items[i]);
		}
	}
	else
		output->Alloc = output->Size = 0;
}

void convertRecordList(itemRecordListW *output, const itemRecordList *input)
{
	output->Alloc = output->Size = input->Size;
	output->Items = (itemRecordW*)malloc(sizeof(itemRecordW) * input->Size);
	if (output->Items)
	{
		memset(output->Items, 0, sizeof(itemRecordW) * input->Size);
		for(int i=0; i < input->Size; i++)
		{
			convertRecord(&output->Items[i],&input->Items[i]);
		}
	}
}

extern int sse_flag;

// swaps two 16 byte aligned 128-byte values
#ifdef _M_X64
#include <emmintrin.h>
#endif
static void __fastcall swap128(uint8_t *_a, uint8_t *_b)
{
	// ECX = a
	// EDX = b
#ifdef _M_IX86
	_asm
	{
		// load first 64 bytes of each
		movaps	xmm0, XMMWORD PTR [ecx+0]
		movaps	xmm1, XMMWORD PTR [ecx+16]
		movaps	xmm2, XMMWORD PTR [ecx+32]
		movaps	xmm3, XMMWORD PTR [ecx+48]
		movaps	xmm4, XMMWORD PTR [edx+0]
		movaps	xmm5, XMMWORD PTR [edx+16]
		movaps	xmm6, XMMWORD PTR [edx+32]
		movaps	xmm7, XMMWORD PTR [edx+48]

		// store
		movaps XMMWORD PTR [edx+0],  xmm0
		movaps XMMWORD PTR [edx+16], xmm1
		movaps XMMWORD PTR [edx+32], xmm2
		movaps XMMWORD PTR [edx+48], xmm3
		movaps XMMWORD PTR [ecx+0],  xmm4
		movaps XMMWORD PTR [ecx+16], xmm5
		movaps XMMWORD PTR [ecx+32], xmm6
		movaps XMMWORD PTR [ecx+48], xmm7

		// load second 64 bytes of each
		movaps	xmm0, XMMWORD PTR [ecx+64]
		movaps	xmm1, XMMWORD PTR [ecx+80]
		movaps	xmm2, XMMWORD PTR [ecx+96]
		movaps	xmm3, XMMWORD PTR [ecx+112]
		movaps	xmm4, XMMWORD PTR [edx+64]
		movaps	xmm5, XMMWORD PTR [edx+80]
		movaps	xmm6, XMMWORD PTR [edx+96]
		movaps	xmm7, XMMWORD PTR [edx+112]

		// store
		movaps XMMWORD PTR [edx+64],  xmm0
		movaps XMMWORD PTR [edx+80], xmm1
		movaps XMMWORD PTR [edx+96], xmm2
		movaps XMMWORD PTR [edx+112], xmm3
		movaps XMMWORD PTR [ecx+64],  xmm4
		movaps XMMWORD PTR [ecx+80], xmm5
		movaps XMMWORD PTR [ecx+96], xmm6
		movaps XMMWORD PTR [ecx+112], xmm7
	}
#else
	//sizeof(itemRecordW) is 176 on AMD64. that's 11 SSE registers
		__m128i b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10; 
		__m128i a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
		__m128i *a = (__m128i *)_a;
		__m128i *b = (__m128i *)_b;

		a0 = _mm_load_si128(&a[0]); 
		a1 = _mm_load_si128(&a[1]); 
		a2 = _mm_load_si128(&a[2]); 
		a3 = _mm_load_si128(&a[3]); 
		a4 = _mm_load_si128(&a[4]); 
		a5 = _mm_load_si128(&a[5]); 
		a6 = _mm_load_si128(&a[6]); 
		a7 = _mm_load_si128(&a[7]); 
		a8 = _mm_load_si128(&a[0]); 
		a9 = _mm_load_si128(&a[1]); 
		a10 = _mm_load_si128(&a[2]); 

		b0 = _mm_load_si128(&b[0]); 
		b1 = _mm_load_si128(&b[1]); 
		b2 = _mm_load_si128(&b[2]); 
		b3 = _mm_load_si128(&b[3]); 
		b4 = _mm_load_si128(&b[4]); 
		b5 = _mm_load_si128(&b[5]); 
		b6 = _mm_load_si128(&b[6]); 
		b7 = _mm_load_si128(&b[7]); 
		b8 = _mm_load_si128(&b[8]); 
		b9 = _mm_load_si128(&b[9]); 
		b10 = _mm_load_si128(&b[10]); 

		_mm_store_si128(&a[0], b0);
		_mm_store_si128(&a[1], b1);
		_mm_store_si128(&a[2], b2);
		_mm_store_si128(&a[3], b3);
		_mm_store_si128(&a[4], b4);
		_mm_store_si128(&a[5], b5);
		_mm_store_si128(&a[6], b6);
		_mm_store_si128(&a[7], b7);
		_mm_store_si128(&b[0], a0);
		_mm_store_si128(&b[1], a1);
		_mm_store_si128(&b[2], a2);
		_mm_store_si128(&b[3], a3);
		_mm_store_si128(&b[4], a4);
		_mm_store_si128(&b[5], a5);
		_mm_store_si128(&b[6], a6);
		_mm_store_si128(&b[7], a7);

		
#endif
}

/***
*shortsort(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void shortsort_sse(uint8_t *lo, uint8_t *hi,  const void *context,
    int (__fastcall *comp)(const void *, const void *, const void *))
{
	const size_t width=sizeof(itemRecordW);
    uint8_t *p;

    /* Note: in assertions below, i and j are alway inside original bound of
       array to sort. */

    while (hi > lo) {
        /* A[i] <= A[j] for i <= j, j > hi */
        uint8_t *max = lo;
        for (p = lo+width; p <= hi; p += width) {
            /* A[i] <= A[max] for lo <= i < p */
            if (comp(p, max, context) > 0) {
                max = p;
            }
            /* A[i] <= A[max] for lo <= i <= p */
        }

        /* A[i] <= A[max] for lo <= i <= hi */

        swap128(max, hi);

        /* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */

        hi -= width;

        /* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
    }
    /* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
       so array is sorted */
}

#define CUTOFF 8 
#define STKSIZ (8*sizeof(void*) - 2)
extern "C" void qsort_itemRecord(void *base, size_t num, const void *context,
    int (__fastcall *comp)(const void *, const void *, const void *))
{
    /* Note: the number of stack entries required is no more than
       1 + log2(num), so 30 is sufficient for any array */
    uint8_t *lo, *hi;			/* ends of sub-array currently sorting */
    uint8_t *mid;				/* points to middle of subarray */
    uint8_t *loguy, *higuy;		/* traveling pointers for partition step */
    size_t size;				/* size of the sub-array */
    uint8_t *lostk[STKSIZ], *histk[STKSIZ];
    int stkptr;					/* stack for saving sub-array to be processed */
	const size_t width=sizeof(itemRecordW);

#ifdef _M_IX86
		assert(sizeof(itemRecordW) == 128);
#elif defined (_M_X64)
		assert(sizeof(itemRecordW) == 176);
#else
#error not implemented on this processor!
#endif
	if (!sse_flag)
	{
		nu::qsort(base, num, sizeof(itemRecordW), context, comp);
		return;
	}

    if (num < 2)
        return;                 /* nothing to do */

    stkptr = 0;                 /* initialize stack */

    lo = static_cast<uint8_t *>(base);
    hi = (uint8_t *)base + width * (num-1);        /* initialize limits */

    /* this entry point is for pseudo-recursion calling: setting
       lo and hi and jumping to here is like recursion, but stkptr is
       preserved, locals aren't, so we preserve stuff on the stack */
recurse:

    size = (hi - lo) / width + 1;        /* number of el's to sort */

    /* below a certain size, it is faster to use a O(n^2) sorting method */
    if (size <= CUTOFF) {
        shortsort_sse(lo, hi, context, comp);
    }
    else {
        /* First we pick a partitioning element.  The efficiency of the
           algorithm demands that we find one that is approximately the median
           of the values, but also that we select one fast.  We choose the
           median of the first, middle, and last elements, to avoid bad
           performance in the face of already sorted data, or data that is made
           up of multiple sorted runs appended together.  Testing shows that a
           median-of-three algorithm provides better performance than simply
           picking the middle element for the latter case. */

        mid = lo + (size / 2) * width;      /* find middle element */

        /* Sort the first, middle, last elements into order */
        if (comp(lo, mid, context) > 0) {
            swap128(lo, mid);
        }
        if (comp(lo, hi, context) > 0) {
            swap128(lo, hi);
        }
        if (comp(mid, hi, context) > 0) {
            swap128(mid, hi);
        }

        /* We now wish to partition the array into three pieces, one consisting
           of elements <= partition element, one of elements equal to the
           partition element, and one of elements > than it.  This is done
           below; comments indicate conditions established at every step. */

        loguy = lo;
        higuy = hi;

        /* Note that higuy decreases and loguy increases on every iteration,
           so loop must terminate. */
        for (;;) {
            /* lo <= loguy < hi, lo < higuy <= hi,
               A[i] <= A[mid] for lo <= i <= loguy,
               A[i] > A[mid] for higuy <= i < hi,
               A[hi] >= A[mid] */

            /* The doubled loop is to avoid calling comp(mid,mid), since some
               existing comparison funcs don't work when passed the same
               value for both pointers. */

            if (mid > loguy) {
                do  {
                    loguy += width;
                } while (loguy < mid && comp(loguy, mid, context) <= 0);
            }
            if (mid <= loguy) {
                do  {
                    loguy += width;
                } while (loguy <= hi && comp(loguy, mid, context) <= 0);
            }

            /* lo < loguy <= hi+1, A[i] <= A[mid] for lo <= i < loguy,
               either loguy > hi or A[loguy] > A[mid] */

            do  {
                higuy -= width;
            } while (higuy > mid && comp(higuy, mid, context) > 0);

            /* lo <= higuy < hi, A[i] > A[mid] for higuy < i < hi,
               either higuy == lo or A[higuy] <= A[mid] */

            if (higuy < loguy)
                break;

            /* if loguy > hi or higuy == lo, then we would have exited, so
               A[loguy] > A[mid], A[higuy] <= A[mid],
               loguy <= hi, higuy > lo */

            swap128(loguy, higuy);

            /* If the partition element was moved, follow it.  Only need
               to check for mid == higuy, since before the swap,
               A[loguy] > A[mid] implies loguy != mid. */

            if (mid == higuy)
                mid = loguy;

            /* A[loguy] <= A[mid], A[higuy] > A[mid]; so condition at top
               of loop is re-established */
        }

        /*     A[i] <= A[mid] for lo <= i < loguy,
               A[i] > A[mid] for higuy < i < hi,
               A[hi] >= A[mid]
               higuy < loguy
           implying:
               higuy == loguy-1
               or higuy == hi - 1, loguy == hi + 1, A[hi] == A[mid] */

        /* Find adjacent elements equal to the partition element.  The
           doubled loop is to avoid calling comp(mid,mid), since some
           existing comparison funcs don't work when passed the same value
           for both pointers. */

        higuy += width;
        if (mid < higuy) {
            do  {
                higuy -= width;
            } while (higuy > mid && comp(higuy, mid, context) == 0);
        }
        if (mid >= higuy) {
            do  {
                higuy -= width;
            } while (higuy > lo && comp(higuy, mid, context) == 0);
        }

        /* OK, now we have the following:
              higuy < loguy
              lo <= higuy <= hi
              A[i]  <= A[mid] for lo <= i <= higuy
              A[i]  == A[mid] for higuy < i < loguy
              A[i]  >  A[mid] for loguy <= i < hi
              A[hi] >= A[mid] */

        /* We've finished the partition, now we want to sort the subarrays
           [lo, higuy] and [loguy, hi].
           We do the smaller one first to minimize stack usage.
           We only sort arrays of length 2 or more.*/

        if ( higuy - lo >= hi - loguy ) {
            if (lo < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy;
                ++stkptr;
            }                           /* save big recursion for later */

            if (loguy < hi) {
                lo = loguy;
                goto recurse;           /* do small recursion */
            }
        }
        else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;               /* save big recursion for later */
            }

            if (lo < higuy) {
                hi = higuy;
                goto recurse;           /* do small recursion */
            }
        }
    }

    /* We have sorted the array, except for any pending sorts on the stack.
       Check if there are any, and do them. */

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;           /* pop subarray from stack */
    }
    else
        return;                 /* all subarrays done */
}