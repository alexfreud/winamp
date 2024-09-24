#define __ON2_MEM_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "on2_mem.h"

#define INCLUDE_MEMORY_MANAGER  0 //include heap manager functionality
#define INCLUDE_MEM_TRACKER     0 //include xon2_* calls in the lib
#define INCLUDE_MEM_CHECKS      1 //include some basic safety checks in
                                  //on2_memcpy, _memset, and _memmove
#if INCLUDE_MEM_TRACKER
# include "on2_mem_tracker.h"
# if ON2_MEM_TRACKER_VERSION_CHIEF != 2
#  error "on2_mem requires memory tracker version 2 to track memory usage"
# endif
#endif

#define ADDRESS_STORAGE_SIZE      sizeof(size_t)

#if defined(VXWORKS)
# define DEFAULT_ALIGNMENT        32        //default addr alignment to use in
                                            //calls to on2_* functions other
                                            //than on2_memalign
#else
# define DEFAULT_ALIGNMENT        1
#endif

#if INCLUDE_MEM_TRACKER
# define TRY_BOUNDS_CHECK         1         //when set to 1 pads each allocation,
                                            //integrity can be checked using 
                                            //on2_MemoryTrackerCheckIntegrity
                                            //or on free by defining
                                            //TRY_BOUNDS_CHECK_ON_FREE
#else
# define TRY_BOUNDS_CHECK         0
#endif

#if TRY_BOUNDS_CHECK
# define TRY_BOUNDS_CHECK_ON_FREE 0          //checks mem integrity on every
                                             //free, very expensive
# define BOUNDS_CHECK_VALUE       0xdeadbeef //value stored before/after ea.
                                             //mem addr for bounds checking
# define BOUNDS_CHECK_PAD_SIZE    32         //size of the padding before and
                                             //after ea allocation to be filled
                                             //with BOUNDS_CHECK_VALUE.
                                             //this should be a multiple of 4
#else
# define BOUNDS_CHECK_VALUE       0
# define BOUNDS_CHECK_PAD_SIZE    0
#endif

unsigned long g_AllocCount = 0;

#if INCLUDE_MEMORY_MANAGER
# include "heapmm.h"
# include "hmm_intrnl.h"

# define SHIFT_HMM_ADDR_ALIGN_UNIT 5
# define TOTAL_MEMORY_TO_ALLOCATE  20971520 // 20 * 1024 * 1024
//# define TOTAL_MEMORY_TO_ALLOCATE 10485100 // 10 * 1024 * 1024
//# define TOTAL_MEMORY_TO_ALLOCATE 16777216 // 16 * 1024 * 1024

# define MM_DYNAMIC_MEMORY 1
# if MM_DYNAMIC_MEMORY
   unsigned char* g_p_mng_memory_raw = NULL;
   unsigned char* g_p_mng_memory     = NULL;
# else
   unsigned char g_p_mng_memory[TOTAL_MEMORY_TO_ALLOCATE];
# endif

 size_t g_mm_memory_size = TOTAL_MEMORY_TO_ALLOCATE;

 hmm_descriptor hmm_d;
 int g_mngMemoryAllocated = 0;

 static int On2_MM_CreateHeapMemory();
 static void* On2_MM_realloc(void* memblk, size_t size);
#endif //INCLUDE_MEMORY_MANAGER

unsigned int on2_mem_get_version()
{
    unsigned int ver = ((unsigned int)(unsigned char)ON2_MEM_VERSION_CHIEF << 24 |
                        (unsigned int)(unsigned char)ON2_MEM_VERSION_MAJOR << 16 |
                        (unsigned int)(unsigned char)ON2_MEM_VERSION_MINOR << 8  |
                        (unsigned int)(unsigned char)ON2_MEM_VERSION_PATCH);
    return ver;
}

int on2_mem_set_heap_size(size_t size)
{
    int ret = -1;

    #if INCLUDE_MEMORY_MANAGER
    #if MM_DYNAMIC_MEMORY
    if(!g_mngMemoryAllocated && size) {
        g_mm_memory_size = size;
        ret = 0;
    } else
        ret = -3;
    #else
    ret = -2;
    #endif
    #else
    (void)size;
    #endif

    return ret;
}

void* on2_memalign(size_t align, size_t size)
{
    void* addr,
	    * x = NULL;

    #if INCLUDE_MEMORY_MANAGER
	int number_aau;

	if (On2_MM_CreateHeapMemory() < 0)
	{
		printf("[on2][mm] ERROR xon2_memalign() Couldn't create memory for Heap.\n");
	}

	number_aau = ((size + align + ADDRESS_STORAGE_SIZE) >>
                   SHIFT_HMM_ADDR_ALIGN_UNIT) + 1;
	
	addr = hmm_alloc(&hmm_d, number_aau);
    #else
    addr = malloc(size + align + ADDRESS_STORAGE_SIZE);
    #endif //INCLUDE_MEMORY_MANAGER
    
    if(addr) {
        ptrdiff_t align_ = align;

        x = (void*)(((size_t)
                ((unsigned char*)addr + ADDRESS_STORAGE_SIZE) + (align_ - 1)) & (size_t)-align_);
        /* save the actual malloc address */
        ((size_t*)x)[-1] = (size_t)addr;
    }

    return x;
}

void* on2_malloc(size_t size)
{   
    return on2_memalign(DEFAULT_ALIGNMENT, size);
}

void* on2_calloc(size_t num, size_t size)
{   
	void *x; 

	x = on2_memalign(DEFAULT_ALIGNMENT, num*size);

	if(x)
        memset(x, 0, num*size);

	return x;
}

void* on2_realloc(void* memblk, size_t size)
{
    void* addr,
        * new_addr = NULL;
    int align = DEFAULT_ALIGNMENT;
	/*
	The realloc() function changes the size of the object pointed to by 
	ptr to the size specified by size, and returns a pointer to the 
	possibly moved block. The contents are unchanged up to the lesser 
	of the new and old sizes. If ptr is null, realloc() behaves like 
	malloc() for the specified size. If size is zero (0) and ptr is 
	not a null pointer, the object pointed to is freed. 
	*/
    if(!memblk)
        new_addr = on2_malloc(size);
    else if (!size)
        on2_free(memblk);
    else
    {
        addr   = (void*)(((size_t*)memblk)[-1]);
        memblk = NULL;

        #if INCLUDE_MEMORY_MANAGER
        new_addr = On2_MM_realloc(addr, size + align + ADDRESS_STORAGE_SIZE);
        #else
        new_addr = realloc(addr, size + align + ADDRESS_STORAGE_SIZE); 
        #endif
        if(new_addr) {
            addr = new_addr;
            new_addr = (void*)(((size_t)
                ((unsigned char*)new_addr + ADDRESS_STORAGE_SIZE) + (align - 1)) &
                (size_t)-align);
            /* save the actual malloc address */
            ((size_t*)new_addr)[-1] = (size_t)addr;
        }
    }

    return new_addr;
}

void on2_free(void* memblk)
{
	if(memblk) {
        void* addr = (void*)(((size_t*)memblk)[-1]);
        #if INCLUDE_MEMORY_MANAGER
	    hmm_free(&hmm_d, addr);
        #else
        free(addr);
        #endif
    }
}

#if INCLUDE_MEM_TRACKER

void* xon2_memalign(size_t align, size_t size, char* file, int line)
{
    #if TRY_BOUNDS_CHECK
	unsigned char *xBounds; 
    #endif

	void *x;

	if (g_AllocCount == 0)
	{
		int iRv = on2_MemoryTrackerInit(BOUNDS_CHECK_PAD_SIZE, BOUNDS_CHECK_VALUE);
		if (iRv < 0)
		{
			printf("ERROR xon2_malloc MEM_TRACK_USAGE error on2_MemoryTrackerInit().\n");
		}
	}

    #if TRY_BOUNDS_CHECK
	{
        int i;
		unsigned int tempme = BOUNDS_CHECK_VALUE;

        xBounds = on2_memalign(align, size + (BOUNDS_CHECK_PAD_SIZE * 2));

        for (i=0;i<BOUNDS_CHECK_PAD_SIZE;i+=sizeof(unsigned int))
        {
		    memcpy(xBounds+i, &tempme, sizeof(unsigned int));
		    memcpy(xBounds + size + BOUNDS_CHECK_PAD_SIZE + i, &tempme, sizeof(unsigned int));
        }
		x = (void*)(xBounds + BOUNDS_CHECK_PAD_SIZE);
	}
    #else
    x = on2_memalign(align, size);
    #endif //TRY_BOUNDS_CHECK

	g_AllocCount++;

    on2_MemoryTrackerAdd((size_t)x, size, file, line);

    return x;
}

void* xon2_malloc(size_t size, char *file, int line)
{
    return xon2_memalign(DEFAULT_ALIGNMENT, size, file, line);
}

void* xon2_calloc(size_t num, size_t size, char *file, int line)
{   
    void* x = xon2_memalign(DEFAULT_ALIGNMENT, num*size, file, line);

	if(x)
        memset(x, 0, num*size);

    return x;
}

void* xon2_realloc(void* memblk, size_t size, char *file, int line)
{
    struct MemBlock* p = NULL;
    int orig_size = 0,
        orig_line = 0;
    char* orig_file = NULL;

    #if TRY_BOUNDS_CHECK
	unsigned char *xBounds = memblk ?
                             (unsigned char*)memblk - BOUNDS_CHECK_PAD_SIZE :
                             NULL;
    #endif

	void *x;

	if (g_AllocCount == 0)
	{
		if (!on2_MemoryTrackerInit(BOUNDS_CHECK_PAD_SIZE, BOUNDS_CHECK_VALUE))
		{
			printf("ERROR xon2_malloc MEM_TRACK_USAGE error on2_MemoryTrackerInit().\n");
		}
	}

    if (p = on2_MemoryTrackerFind((size_t)memblk))
    {
        orig_size = p->size;
        orig_file = p->file;
        orig_line = p->line;
    }

    #if TRY_BOUNDS_CHECK_ON_FREE
    on2_MemoryTrackerCheckIntegrity(file, line);
    #endif

    //have to do this regardless of success, because
    //the memory that does get realloc'd may change
    //the bounds values of this block
    on2_MemoryTrackerRemove((size_t)memblk);

    #if TRY_BOUNDS_CHECK
	{
		xBounds = on2_realloc(xBounds, size + (BOUNDS_CHECK_PAD_SIZE * 2));

        if (xBounds)
        {
            int i;
		    unsigned int tempme = BOUNDS_CHECK_VALUE;

            for (i=0;i<BOUNDS_CHECK_PAD_SIZE;i+=sizeof(unsigned int))
            {
		        memcpy(xBounds+i, &tempme, 4);
		        memcpy(xBounds + size + BOUNDS_CHECK_PAD_SIZE + i, &tempme, 4);
            }

		    x = (void*)(xBounds + BOUNDS_CHECK_PAD_SIZE);
        }
        else
            x = NULL;
	}
    #else
    x = on2_realloc(memblk, size);
    #endif //TRY_BOUNDS_CHECK

    if (x)
        on2_MemoryTrackerAdd((size_t)x, size, file, line);
    else
        on2_MemoryTrackerAdd((size_t)memblk, orig_size, orig_file, orig_line);

    return x;
}

void xon2_free(void *pAddress, char *file, int line)
{
    #if TRY_BOUNDS_CHECK
	unsigned char *pBoundsAddress = (unsigned char*)pAddress;
	pBoundsAddress -= BOUNDS_CHECK_PAD_SIZE;
    #endif

    #if !TRY_BOUNDS_CHECK_ON_FREE
    (void)file; (void)line;
    #endif

	if(pAddress)
	{
	    g_AllocCount--;

        #if TRY_BOUNDS_CHECK_ON_FREE
        on2_MemoryTrackerCheckIntegrity(file, line);
        #endif

        //if the addr isn't found in the list, assume it was allocated via
        //on2_ calls not xon2_, therefore it does not contain any padding
        if (on2_MemoryTrackerRemove((size_t)pAddress) == -2)
            pBoundsAddress = pAddress;

        #if TRY_BOUNDS_CHECK
	    on2_free(pBoundsAddress);
        #else
	    on2_free(pAddress);
        #endif
    }
}

#endif /*INCLUDE_MEM_TRACKER*/

#if INCLUDE_MEM_CHECKS
#if defined(VXWORKS)
/* This function is only used to get a stack trace of the player 
object so we can se where we are having a problem. */
int getMyTT(int task)
{
	tt(task);

	return 0;
}
#endif
#endif

void * on2_memcpy(void *dest, const void *source, size_t length)
{
    #if INCLUDE_MEM_CHECKS
	if (((intptr_t)dest < 0x4000) || ((intptr_t)source < 0x4000))
	{
		printf("WARNING: on2_memcpy dest:0x%p source:0x%p len:%d\n", dest, source, length);

        #if defined(VXWORKS)
		sp(getMyTT, taskIdSelf(), 0, 0, 0, 0, 0, 0, 0, 0);

		on2Timer_Sleep(10000);
        #endif
	}
    #endif

	return memcpy(dest, source, length);
}


void * on2_memset(void *dest, int val, size_t length)
{
    #if INCLUDE_MEM_CHECKS
	if ((intptr_t)dest < 0x4000)
	{
		printf("WARNING: on2_memset dest:0x%p val:%d len:%d\n", dest, val, length);

        #if defined(VXWORKS)
		sp(getMyTT, taskIdSelf(), 0, 0, 0, 0, 0, 0, 0, 0);

		on2Timer_Sleep(10000);
        #endif
	}
    #endif

	return memset(dest, val, length);
}


void * on2_memmove(void *dest, const void *src, size_t count)
{
    #if INCLUDE_MEM_CHECKS
	if (((intptr_t)dest < 0x4000) || ((intptr_t)src < 0x4000))
	{
		printf("WARNING: on2_memmove dest:0x%p src:0x%p count:%d\n", dest, src, count);

        #if defined(VXWORKS)
		sp(getMyTT, taskIdSelf(), 0, 0, 0, 0, 0, 0, 0, 0);

		on2Timer_Sleep(10000);
        #endif
	}
    #endif

    return memmove(dest, src, count);
}

#if INCLUDE_MEMORY_MANAGER

static int On2_MM_CreateHeapMemory()
{
	int iRv = 0;

	if (!g_mngMemoryAllocated)
	{
        #if MM_DYNAMIC_MEMORY
		g_p_mng_memory_raw =
            (unsigned char*)malloc(g_mm_memory_size + HMM_ADDR_ALIGN_UNIT);

		if (g_p_mng_memory_raw)
		{
			g_p_mng_memory = (unsigned char*)((((unsigned int)g_p_mng_memory_raw) +
                                               HMM_ADDR_ALIGN_UNIT-1) &
                                              -(int)HMM_ADDR_ALIGN_UNIT);

			printf("[on2][mm] total memory size:%d g_p_mng_memory_raw:0x%x g_p_mng_memory:0x%x\n"
				, g_mm_memory_size + HMM_ADDR_ALIGN_UNIT
				, (unsigned int)g_p_mng_memory_raw
				, (unsigned int)g_p_mng_memory);
		}
		else
		{
			printf("[on2][mm] Couldn't allocate memory:%d for on2 memory manager.\n"
				, g_mm_memory_size);

			iRv = -1;
		}

		if (g_p_mng_memory)
        #endif
		{
			int chunkSize = 0;

			g_mngMemoryAllocated = 1;

			hmm_init(&hmm_d);

			chunkSize = g_mm_memory_size >> SHIFT_HMM_ADDR_ALIGN_UNIT;

			chunkSize -= DUMMY_END_BLOCK_BAUS;

			printf("[on2][mm] memory size:%d for on2 memory manager. g_p_mng_memory:0x%x  chunkSize:%d\n"
				, g_mm_memory_size
				, (unsigned int)g_p_mng_memory
				, chunkSize);

			hmm_new_chunk(&hmm_d, (void*)g_p_mng_memory, chunkSize);
		}
        #if MM_DYNAMIC_MEMORY
		else
		{
			printf("[on2][mm] Couldn't allocate memory:%d for on2 memory manager.\n"
				, g_mm_memory_size);

			iRv = -1;
		}
        #endif
	}

	return iRv;
}

static void* On2_MM_realloc(void* memblk, size_t size)
{
	void* pRet = NULL;

	if (On2_MM_CreateHeapMemory() < 0)
	{
		printf("[on2][mm] ERROR On2_MM_realloc() Couldn't create memory for Heap.\n");
	}
	else
	{
		int iRv = 0;
		int old_num_aaus;
		int new_num_aaus;
		
		old_num_aaus = hmm_true_size(memblk);
		new_num_aaus = (size >> SHIFT_HMM_ADDR_ALIGN_UNIT) + 1;
		
		if (old_num_aaus == new_num_aaus)
		{
			pRet = memblk;
		}
		else
		{
			iRv = hmm_resize(&hmm_d, memblk, new_num_aaus);
			if (iRv == 0)
			{
                pRet = memblk;
			}
			else
			{
				/* Error. Try to malloc and then copy data. */
				void* pFromMalloc;
				
	            new_num_aaus = (size >> SHIFT_HMM_ADDR_ALIGN_UNIT) + 1;
	            pFromMalloc  = hmm_alloc(&hmm_d, new_num_aaus);
				
				if (pFromMalloc)
				{
					on2_memcpy(pFromMalloc, memblk, size);
                    hmm_free(&hmm_d, memblk);
					
					pRet = pFromMalloc;
				}
			}
		}
	}

	return pRet;
}

#endif //INCLUDE_MEMORY_MANAGER
