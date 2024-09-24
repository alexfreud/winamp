/*
    Windows 95 Memory allocation abstraction functions
 */

#include <stdio.h>
#include <windows.h>

#include "dkpltfrm.h"
#include "duck_mem.h"

#define CHECK_FOR_MEMORY_LEAK 0

void *malloc_32b(size_t size)
{
	return LocalAlloc( LMEM_FIXED, size );
	
}
void *calloc_32b(size_t count,size_t size)
{
	/* maybe we should rewrite this to use dwords 
	    (who knows if they do it automatica like) */
	return LocalAlloc( LMEM_ZEROINIT, size*count );
	
}

void free_32b(void * hMem)
{
	LocalFree(hMem);
}

static void *(*ptr_malloc)(size_t size) = malloc_32b;
static void *(*ptr_calloc)(size_t, size_t size) = calloc_32b;
static void (*ptr_free)(void *) = free_32b;
 
void *duck_malloc(size_t size, enum tmemtype fred)
{   
	void *temp;
	
	temp = (*ptr_malloc)(size);
	
#if CHECK_FOR_MEMORY_LEAK
{
	FILE * out;
	
	if ((out = fopen("c:\\sjl.log","a")) != NULL) {
        fprintf(out,"DXV duck_malloc:%x %d\n", temp, size);
        fclose(out);
    }
}
#endif

	return temp;
}

void *duck_calloc(size_t n,size_t size, enum tmemtype  fred)
{   
	void *temp = (*ptr_calloc) (n, size);

#if CHECK_FOR_MEMORY_LEAK
{
	FILE * out;
	
	if ((out = fopen("c:\\sjl.log","a")) != NULL) {
        fprintf(out,"DXV duck_calloc:%x %d %d \n", temp, n, size);
        fclose(out);
    }
}
#endif

	return temp;
}

void duck_free(void *old_blk) 
{  

#if CHECK_FOR_MEMORY_LEAK
{
	FILE * out;
	
	if ((out = fopen("c:\\sjl.log","a")) != NULL) {
        fprintf(out,"DXV duck_free:%x\n", old_blk);
        fclose(out);
    }
}
#endif

	(*ptr_free) (old_blk);
}

void DXV_Setmalloc(void *(*ptr)(size_t))
{
	ptr_malloc = ptr;
}
 
void DXV_Setcalloc(void *(*ptr)(size_t, size_t))
{
	ptr_calloc = ptr;
}

void DXV_Setfree(void (*ptr)(void *))
{
	ptr_free = ptr;
}
