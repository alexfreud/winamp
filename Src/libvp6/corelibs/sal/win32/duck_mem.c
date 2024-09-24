/***********************************************\
??? duck_mem.c
\***********************************************/

#pragma warning(disable:4786)


#include <stdio.h> 
#include <string.h>
#include <dos.h> 
#include <time.h>
#include <malloc.h>
#include "duck_mem.h"
#include "duck_io.h"
//#include "duck_hfb.h"
#include "duck_dxl.h"

//#define CHECK_MEM

#ifdef CHECK_MEM
#include <map>
#include "debug.h"
#endif

#ifdef CHECK_MEM
struct comp
{
	bool operator()(void* v1, void* v2) const
	{
		return v1 < v2;
	}
};
#endif

#ifdef CHECK_MEM
std::map<void*, size_t, struct comp> pointers;
int g_allocs = 0;
int g_frees = 0;
#endif

void *duck_memmove( void *dst, const void *src, size_t length )
{
    return memmove(dst, src, length);
}

void *duck_malloc(size_t size, dmemType foo)
{
	void *temp = malloc(size);

#ifdef CHECK_MEM
	g_allocs++;
    TRACE("a=%d\t%d\n", g_allocs, (int)temp);
	pointers[temp] = size;
#endif
	return temp;
}

void *duck_memset( void *dest, int c, size_t count )
{ 
    return((void *) memset(dest, c, count));
}                           

void *duck_calloc(size_t n,size_t size, dmemType foo)
{   
	void *temp = calloc(n, size);

#ifdef CHECK_MEM
	g_allocs++;
    TRACE("a=%d\t%d\n", g_allocs, (int)temp);
	pointers[temp] = size;
#endif
	return temp;
}

void duck_free(void *old_blk)
{  
#ifdef CHECK_MEM
	g_frees++;
    TRACE("f=%d\t%d\n", g_frees, (int)old_blk);
	if(!pointers.erase(old_blk))
		assert(0);
#endif
	free(old_blk);
}

void* duck_realloc(void *src, size_t newSize, size_t oldSize)
{
    void *temp;
    if(newSize <= oldSize)
        return src;

#ifdef CHECK_MEM
	temp = duck_malloc(newSize, DMEM_GENERAL);
	duck_memcpy(temp, src, oldSize);
	duck_free(src);
#else
    temp = realloc(src, newSize);
#endif
	return temp;
}

void *duck_memcpy(void *dest, const void *source, size_t length)
{
		return memcpy(dest,source,length);
}

void *duck_memcpy64(void *dest, const void *source, int64_t length)
{
    /* Not fully 64 bit compliant */
    return memcpy(dest,source,(size_t)length);
}


int duck_strcmp(const char *one, const char *two)
{
	return strcmp(one, two);
}

void set_memfuncs()
{
#if defined(DXV_DLL)
	DXV_Setmalloc(malloc);
	DXV_Setcalloc(calloc);
	DXV_Setfree(free);
#endif

#if defined(HFB_DLL)
	HFB_Setmalloc(malloc);
	HFB_Setcalloc(calloc);
	HFB_Setfree(free);
#endif
}
