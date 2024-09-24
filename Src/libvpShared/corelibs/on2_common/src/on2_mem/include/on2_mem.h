#ifndef __ON2_MEM_H__
#define __ON2_MEM_H__

/* on2_mem version info */
#define on2_mem_version "2.0.1.1"

#define ON2_MEM_VERSION_CHIEF 2
#define ON2_MEM_VERSION_MAJOR 0
#define ON2_MEM_VERSION_MINOR 1
#define ON2_MEM_VERSION_PATCH 1
/* end - on2_mem version info */

#define ON2_TRACK_MEM_USAGE       0  //enable memory tracking/integrity checks
#define ON2_CHECK_MEM_FUNCTIONS   0  //enable basic safety checks in _memcpy,
                                     //_memset, and _memmove
#define REPLACE_BUILTIN_FUNCTIONS 0  //replace builtin functions with their
                                     //on2_ equivalents

#include <stddef.h>
#include "on2_mem_tracker.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
    on2_mem_get_version()
    provided for runtime version checking. Returns an unsigned int of the form
    CHIEF | MAJOR | MINOR | PATCH, where the chief version number is the high
    order byte.
*/
unsigned int on2_mem_get_version();

/*
    on2_mem_set_heap_size(size_t size)
      size - size in bytes for the memory manager to allocate for its heap
    Sets the memory manager's initial heap size
    Return:
      0: on success
      -1: if memory manager calls have not been included in the on2_mem lib
      -2: if the memory manager has been compiled to use static memory
      -3: if the memory manager has already allocated its heap
*/
int on2_mem_set_heap_size(size_t size);

void* on2_memalign(size_t align, size_t size);
void* on2_malloc(size_t size);
void* on2_calloc(size_t num, size_t size);
void* on2_realloc(void* memblk, size_t size);
void on2_free(void* memblk);

void* on2_memcpy(void* dest, const void* src, size_t length);
void* on2_memset(void* dest, int val, size_t length);
void* on2_memmove(void* dest, const void* src, size_t count);

/* some defines for backward compatibility */
#define DMEM_GENERAL 0

#define duck_memalign(X,Y,Z) on2_memalign(X,Y)
#define duck_malloc(X,Y) on2_malloc(X)
#define duck_calloc(X,Y,Z) on2_calloc(X,Y)
#define duck_realloc  on2_realloc
#define duck_free     on2_free
#define duck_memcpy   on2_memcpy
#define duck_memmove  on2_memmove
#define duck_memset   on2_memset

#if REPLACE_BUILTIN_FUNCTIONS
#define memalign on2_memalign
#define malloc   on2_malloc
#define calloc   on2_calloc
#define realloc  on2_realloc
#define free     on2_free
#define memcpy   on2_memcpy
#define memmove  on2_memmove
#define memset   on2_memset
#endif

#if ON2_TRACK_MEM_USAGE
# ifndef __ON2_MEM_C__
#  define on2_memalign(align, size) xon2_memalign((align), (size), __FILE__, __LINE__)
#  define on2_malloc(size)          xon2_malloc((size), __FILE__, __LINE__)
#  define on2_calloc(num, size)     xon2_calloc(num, size, __FILE__, __LINE__)
#  define on2_realloc(addr, size)   xon2_realloc(addr, size, __FILE__, __LINE__)
#  define on2_free(addr)            xon2_free(addr, __FILE__, __LINE__)
# endif

 void* xon2_memalign(size_t align, size_t size, char* file, int line);
 void* xon2_malloc(size_t size, char* file, int line);
 void* xon2_calloc(size_t num, size_t size, char* file, int line);
 void* xon2_realloc(void* memblk, size_t size, char* file, int line);
 void xon2_free(void* memblk, char* file, int line);
#endif

#if !ON2_CHECK_MEM_FUNCTIONS
# ifndef __ON2_MEM_C__
#  include <string.h>
#  define on2_memcpy  memcpy
#  define on2_memset  memset
#  define on2_memmove memmove
# endif
#endif

#if defined(__cplusplus)
}
#endif

#endif /* __ON2_MEM_H__ */
