/***
*smalheap.c - small, simple heap manager
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*
*
*******************************************************************************/
#include <stdlib.h>
#include <malloc.h>
#include <windows.h>

#define BYTES_PER_PARA      16
#define DWORDS_PER_PARA     4

#define PARAS_PER_PAGE      256     //  tunable value
#define PAGES_PER_GROUP     8       //  tunable value
#define GROUPS_PER_REGION   32      //  tunable value (max 32)

#define BYTES_PER_PAGE      (BYTES_PER_PARA * PARAS_PER_PAGE)
#define BYTES_PER_GROUP     (BYTES_PER_PAGE * PAGES_PER_GROUP)
#define BYTES_PER_REGION    (BYTES_PER_GROUP * GROUPS_PER_REGION)
#ifdef __cplusplus
extern "C" {
#endif
HANDLE _crtheap;

/*
 * Primary heap routines (Initialization, termination, malloc and free).
 */

void __cdecl free (
        void * pblock
        )
{
        if ( pblock == NULL )
            return;

        HeapFree(_crtheap, 0, pblock);
}


int __cdecl _heap_init (
        int mtflag
        )
{
        if ( (_crtheap = HeapCreate( mtflag ? 0 : HEAP_NO_SERIALIZE,
                                     BYTES_PER_PAGE, 0 )) == NULL )
            return 0;

        return 1;
}


void __cdecl _heap_term (
        void
        )
{
        HeapDestroy( _crtheap );
}


void * __cdecl _nh_malloc (
        size_t size,
        int nhFlag
        )
{
        void * retp;

          retp = HeapAlloc( _crtheap, 0, size );

            /*
             * if successful allocation, return pointer to memory
             * if new handling turned off altogether, return NULL
             */

              return retp;


}


void * __cdecl malloc (
        size_t size
        )
{
        return _nh_malloc( size, 0 );
}

/*
 * Secondary heap routines.
 */

void * __cdecl calloc (
        size_t num,
        size_t size
        )
{
        void * retp;

        size *= num;


            retp = HeapAlloc( _crtheap, HEAP_ZERO_MEMORY, size );

                return retp;

            /* new handler was successful -- try to allocate again */

}


void * __cdecl _expand (
        void * pblock,
        size_t newsize
        )
{
        return HeapReAlloc( _crtheap,
                            HEAP_REALLOC_IN_PLACE_ONLY,
                            pblock,
                            newsize );
}


int __cdecl _heapchk(void)
{
        int retcode = _HEAPOK;

        if ( !HeapValidate( _crtheap, 0, NULL ) &&
             (GetLastError() != ERROR_CALL_NOT_IMPLEMENTED) )
                retcode = _HEAPBADNODE;

        return retcode;
}


int __cdecl _heapmin(void)
{
        if ( (HeapCompact( _crtheap, 0 ) == 0) &&
             (GetLastError() != ERROR_CALL_NOT_IMPLEMENTED) )
            return -1;

        return 0;
}


size_t __cdecl _msize (
        void * pblock
        )
{
        return (size_t)HeapSize( _crtheap, 0, pblock );
}


void * __cdecl realloc (
        void * pblock,
        size_t newsize
        )
{
        void * retp;

        /* if pblock is NULL, call malloc */
        if ( pblock == (void *) NULL )
            return malloc( newsize );

        /* if pblock is !NULL and size is 0, call free and return NULL */
        if ( newsize == 0 ) {
            free( pblock );
            return NULL;
        }


            retp = HeapReAlloc( _crtheap, 0, pblock, newsize );

                return retp;

}

#ifdef __cplusplus
}
#endif