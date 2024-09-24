#define __ON2_MEM_TRACKER_C__
/*
  on2_mem_tracker.c

  jwz 2003-09-30:
   Stores a list of addreses, their size, and file and line they came from.
   All exposed lib functions are prefaced by on2_ and allow the global list
   to be thread safe.
   Current supported platforms are:
    Linux, Win32, WinCE and VxWorks
   Further support can be added by defining the platform specific mutex
   in the MemoryTracker struct as well as calls to create/destroy/lock/unlock
   the mutex in on2_MemoryTrackerInit/Destroy and MemoryTrackerLockMutex/UnlockMutex
*/

#if defined(LINUX)
#include <pthread.h>
#elif defined(WIN32) || defined(_WIN32_WCE)
#include <windows.h>
#include <winbase.h>
#elif defined(VXWORKS)
#include <semLib.h>
#endif

#include "on2_mem_tracker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //VXWORKS doesn't have a malloc/memory.h file,
                    //this should pull in malloc,free,etc.
#include <stdarg.h>

#undef on2_malloc   //undefine any on2_mem macros that may affect calls to
#undef on2_free     //memory functions in this file
#undef on2_memcpy
#undef on2_memset

struct MemoryTracker
{
    struct MemBlock * head,
                    * tail;
    int len,
        totalsize;
    unsigned int current_allocated,
                 max_allocated;

    #if defined(LINUX)
    pthread_mutex_t mutex;
    #elif defined(WIN32) || defined(_WIN32_WCE)
    HANDLE mutex;
    #elif defined(VXWORKS)
    SEM_ID mutex;
    #else
    #error "No mutex type defined for this platform!"
    #endif

    int padding_size,
        pad_value;

};


/* prototypes for internal library functions */
static void memtrack_log(const char* fmt, ...);
static void MemoryTrackerDump();
static void MemoryTrackerCheckIntegrity(char* file, unsigned int line);
static void MemoryTrackerAdd(size_t addr, unsigned int size,
                             char* file, unsigned int line);
static int MemoryTrackerRemove(size_t addr);
static struct MemBlock* MemoryTrackerFind(size_t addr);

static int MemoryTrackerLockMutex();
static int MemoryTrackerUnlockMutex();

static struct MemoryTracker memtrack;   //our global memory allocation list
static int g_bMemTrackerInited = 0;    //indicates whether the global list has
                                        //been initialized (1:yes/0:no)
static FILE* g_logfile = NULL;
static int g_logtype   = 0;

/*
 *
 * Exposed library functions
 *
*/

/*
    on2_MemoryTrackerInit(int padding_size, int pad_value)
      padding_size - the size of the padding before and after each mem addr.
                     Values > 0 indicate that integrity checks can be performed
                     by inspecting these areas.
      pad_value - the initial value within the padding area before and after
                  each mem addr.
      
    Initializes global memory tracker structure
    Allocates the head of the list
*/
int on2_MemoryTrackerInit(int padding_size, int pad_value)
{
    if (!g_bMemTrackerInited)
    {
        if (memtrack.head = (struct MemBlock*)malloc(sizeof(struct MemBlock)))
        {
            int ret;

            memset(memtrack.head, 0, sizeof(struct MemBlock));

            memtrack.tail = memtrack.head;

            memtrack.current_allocated = 0;
            memtrack.max_allocated     = 0;

            memtrack.padding_size = padding_size;
            memtrack.pad_value    = pad_value;
            
            #if defined(LINUX)
            ret = pthread_mutex_init(&memtrack.mutex,
                                     NULL);            /*mutex attributes (NULL=default)*/
            #elif defined(WIN32) || defined(_WIN32_WCE)
            memtrack.mutex = CreateMutex(NULL,   /*security attributes*/
                                         FALSE,  /*we don't want initial ownership*/
                                         NULL);  /*mutex name*/
            ret = !memtrack.mutex;
            #elif defined(VXWORKS)
            memtrack.mutex = semBCreate(SEM_Q_FIFO, /*SEM_Q_FIFO non-priority based mutex*/
                                        SEM_FULL);  /*SEM_FULL initial state is unlocked*/
            ret = !memtrack.mutex;
            #endif

            if (ret)
            {
                memtrack_log("on2_MemoryTrackerInit: Error creating mutex!\n");

                free(memtrack.head);
                memtrack.head = NULL;
            }
            else
            {
                memtrack_log("Memory Tracker init'd, v."on2_mem_tracker_version"\n");
                g_bMemTrackerInited = 1;
            }
        }
    }
    
    return g_bMemTrackerInited;
}

/*
    on2_MemoryTrackerDestroy()
    If our global struct was initialized zeros out all its members,
    frees memory and destroys it's mutex
*/
void on2_MemoryTrackerDestroy()
{
    if (!MemoryTrackerLockMutex())
    {
        struct MemBlock* p  = memtrack.head,
                       * p2 = memtrack.head;

        MemoryTrackerDump();

        while(p)
        {
            p2 = p;
            p  = p->next;

            free(p2);
        }

        memtrack.head              = NULL;
        memtrack.tail              = NULL;
        memtrack.len               = 0;
        memtrack.current_allocated = 0;
        memtrack.max_allocated     = 0;

        if(!g_logtype && g_logfile && g_logfile != stderr) {
            fclose(g_logfile);
            g_logfile = NULL;
        }

        MemoryTrackerUnlockMutex();

	    g_bMemTrackerInited = 0;
    }
}

/*
    on2_MemoryTrackerAdd(size_t addr, unsigned int size,
                         char * file, unsigned int line)
      addr - memory address to be added to list
      size - size of addr
      file - the file addr was referenced from
      line - the line in file addr was referenced from
    Adds memory address addr, it's size, file and line it came from
    to the global list via the thread safe internal library function
*/
void on2_MemoryTrackerAdd(size_t addr, unsigned int size,
                          char * file, unsigned int line)
{
    MemoryTrackerAdd(addr, size, file, line);
}

/*
    on2_MemoryTrackerRemove(size_t addr)
      addr - memory address to be removed from list
    Removes addr from the global list via the thread safe
    internal remove function
    Return:
      Same as described for MemoryTrackerRemove
*/
int on2_MemoryTrackerRemove(size_t addr)
{
    return MemoryTrackerRemove(addr);
}

/*
    on2_MemoryTrackerFind(size_t addr)
      addr - address to be found in list
    Return:
        If found, pointer to the memory block that matches addr
        NULL otherwise
*/
struct MemBlock* on2_MemoryTrackerFind(size_t addr)
{
    struct MemBlock* p = NULL;

    if (!MemoryTrackerLockMutex())
    {
        p = MemoryTrackerFind(addr);
        MemoryTrackerUnlockMutex();
    }

    return p;
}

/*
    on2_MemoryTrackerDump()
    Locks the memory tracker's mutex and calls the internal
    library function to dump the current contents of the
    global memory allocation list
*/
void on2_MemoryTrackerDump()
{
    if (!MemoryTrackerLockMutex())
    {
        MemoryTrackerDump();
        MemoryTrackerUnlockMutex();
    }
}

/*
    on2_MemoryTrackerCheckIntegrity(char* file, unsigned int line)
      file - The file name where the check was placed
      line - The line in file where the check was placed
    Locks the memory tracker's mutex and calls the internal
    integrity check function to inspect every address in the global
    memory allocation list
*/
void on2_MemoryTrackerCheckIntegrity(char* file, unsigned int line)
{
    if (!MemoryTrackerLockMutex())
    {
        MemoryTrackerCheckIntegrity(file, line);
        MemoryTrackerUnlockMutex();
    }
}

/*
    on2_MemoryTrackerSetLogType
    Sets the logging type for the memory tracker. Based on the value it will
    direct its output to the appropriate place.
    Return:
      0: on success
      -1: if the logging type could not be set, because the value was invalid
          or because a file could not be opened
*/
int on2_MemoryTrackerSetLogType(int type, char* option)
{
    int ret = -1;

    switch(type) {
    case 0:
        g_logtype = 0;
        if(!option) {
            g_logfile = stderr;
            ret = 0;
        } else {
            if (g_logfile = fopen(option, "w"))
                ret = 0;
        }
        break;
#if defined(WIN32) && !defined(_WIN32_WCE)
    case 1:
        g_logtype = type;
        ret = 0;
        break;
#endif
    default:
        break;
    }

    //output the version to the new logging destination
    if(!ret)
        memtrack_log("Memory Tracker init'd, v."on2_mem_tracker_version"\n");

    return ret;
}

/*
 *
 * END - Exposed library functions
 *
*/


/*
 *
 * Internal library functions
 *
*/

static void memtrack_log(const char* fmt, ...)
{
    va_list list;

    va_start(list, fmt);
    switch(g_logtype) {
    case 0:
        if (g_logfile) {
            vfprintf(g_logfile, fmt, list);
            fflush(g_logfile);
        }
        break;
#if defined(WIN32) && !defined(_WIN32_WCE)
    case 1:
        {
            char temp[1024];
            _vsnprintf(temp, sizeof(temp)/sizeof(char)-1, fmt, list);
            OutputDebugString(temp);
        }
        break;
#endif
    default:
        break;
    }
    va_end(list);
}

/*
    MemoryTrackerDump()
    Dumps the current contents of the global memory allocation list
*/
static void MemoryTrackerDump()
{
	int i = 0;
    struct MemBlock* p = (memtrack.head ? memtrack.head->next : NULL);

    memtrack_log("Currently Allocated= %d; Max allocated= %d\n",
        memtrack.current_allocated, memtrack.max_allocated);

    while(p)
    {
        memtrack_log("memblocks[%d].addr= 0x%.8x, memblocks[%d].size= %d, file: %s, line: %d\n", i, 
	        p->addr, i, p->size, 
	        p->file, p->line);

        p = p->next;
        ++i;
    }
}

/*
    MemoryTrackerCheckIntegrity(char* file, unsigned int file)
      file - the file name where the check was placed
      line - the line in file where the check was placed
    If a padding_size was supplied to on2_MemoryTrackerInit()
    this function will ea. addr in the list verifying that
    addr-padding_size and addr+padding_size is filled with pad_value
*/
static void MemoryTrackerCheckIntegrity(char* file, unsigned int line)
{
    if (memtrack.padding_size)
    {
	    int i,
            index = 0;
        unsigned int * pShowMe,
                     * pShowMe2;
	    unsigned int tempme = memtrack.pad_value,
                     dead1,
                     dead2;
        unsigned char *xBounds;
        struct MemBlock* p = memtrack.head->next;

        while (p)
	    {
            xBounds = (unsigned char*)p->addr; 

		    //back up ON2_BYTE_ALIGNMENT
		    xBounds -= memtrack.padding_size;

            for (i=0;i<memtrack.padding_size;i+=sizeof(unsigned int))
            {
		        pShowMe = (unsigned int*)(xBounds+i);
		        pShowMe2 = (unsigned int*)(xBounds + p->size + memtrack.padding_size + i);

		        memcpy(&dead1, pShowMe, sizeof(unsigned int));
		        memcpy(&dead2, pShowMe2, sizeof(unsigned int));

		        if ((dead1 != tempme) || (dead2 != tempme))
		        {
			        memtrack_log("\n[on2_mem integrity check failed]:\n"
                                 "    index[%d] {%s:%d} addr=0x%x, size= %d,"
                                 " file: %s, line: %d c0:0x%x c1:0x%x\n",
				            index, file, line, p->addr, p->size, p->file,
                            p->line, dead1, dead2);
		        }
            }

            ++index;
            p = p->next;
	    }
    }
}

/*
    MemoryTrackerAdd(size_t addr, unsigned int size,
                     char * file, unsigned int line)
    Adds an address (addr), it's size, file and line number to our list.
    Adjusts the total bytes allocated and max bytes allocated if necessary.
    If memory cannot be allocated the list will be destroyed.
*/
void MemoryTrackerAdd(size_t addr, unsigned int size,
                      char * file, unsigned int line)
{
    if (!MemoryTrackerLockMutex())
    {
        struct MemBlock* p;

        p = malloc(sizeof(struct MemBlock));

        if (p)
        {
            p->prev       = memtrack.tail;
            p->prev->next = p;
            p->addr       = addr;
            p->size       = size;
            p->line       = line;
            p->file       = file;
            p->next       = NULL;

            memtrack.tail = p;

            memtrack.current_allocated += size;

            if (memtrack.current_allocated > memtrack.max_allocated)
                memtrack.max_allocated = memtrack.current_allocated;

            MemoryTrackerUnlockMutex();
        }
        else
        {
            memtrack_log("MemoryTrackerAdd: error allocating memory!\n");
            MemoryTrackerUnlockMutex();
            on2_MemoryTrackerDestroy();
        }
    }
}

/*
    MemoryTrackerRemove(size_t addr)
    Removes an address and its corresponding size (if they exist)
    from the memory tracker list and adjusts the current number
    of bytes allocated.
    Return:
      0: on success
      -1: if the mutex could not be locked
      -2: if the addr was not found in the list
*/
int MemoryTrackerRemove(size_t addr)
{
    int ret = -1;

    if (!MemoryTrackerLockMutex())
    {
        struct MemBlock* p;

        if (p = MemoryTrackerFind(addr))
        {
            memtrack.current_allocated -= p->size;

            p->prev->next = p->next;
            if (p->next)
                p->next->prev = p->prev;
            else
                memtrack.tail = p->prev;

            ret = 0;
            free(p);
        }
        else
        {
            memtrack_log("MemoryTrackerRemove(): addr not found in list, 0x%.8x\n", addr);
            ret = -2;
        }

        MemoryTrackerUnlockMutex();
    }

    return ret;
}

/*
    MemoryTrackerFind(size_t addr)
    Finds an address in our addrs list
    NOTE: the mutex MUST be locked in the other internal
          functions before calling this one. This avoids
          the need for repeated locking and unlocking as in Remove
    Returns: pointer to the mem block if found, NULL otherwise
*/
static struct MemBlock* MemoryTrackerFind(size_t addr)
{
    struct MemBlock* p = NULL;

    if (memtrack.head)
    {
        p = memtrack.head->next;

        while(p && (p->addr != addr))
            p = p->next;
    }

    return p;
}

/*
    MemoryTrackerLockMutex()
    Locks the memory tracker mutex with a platform specific call
    Returns:
        0: Success
       <0: Failure, either the mutex was not initialized
           or the call to lock the mutex failed
*/
static int MemoryTrackerLockMutex()
{
    int ret = -1;

    if (g_bMemTrackerInited)
    {

        #if defined(LINUX)
        ret = pthread_mutex_lock(&memtrack.mutex);
        #elif defined(WIN32) || defined(_WIN32_WCE)
        ret = WaitForSingleObject(memtrack.mutex, INFINITE);
        #elif defined(VXWORKS)
        ret = semTake(memtrack.mutex, WAIT_FOREVER);
        #endif

        if (ret)
        {
            memtrack_log("MemoryTrackerLockMutex: mutex lock failed\n");
        }
    }

	return ret;
}

/*
    MemoryTrackerUnlockMutex()
    Unlocks the memory tracker mutex with a platform specific call
    Returns:
        0: Success
       <0: Failure, either the mutex was not initialized
           or the call to unlock the mutex failed
*/
static int MemoryTrackerUnlockMutex()
{
    int ret = -1;

    if (g_bMemTrackerInited)
    {
        
        #if defined(LINUX)
        ret = pthread_mutex_unlock(&memtrack.mutex);
        #elif defined(WIN32) || defined(_WIN32_WCE)
        ret = !ReleaseMutex(memtrack.mutex);
        #elif defined(VXWORKS)
        ret = semGive(memtrack.mutex);
        #endif

        if (ret)
        {
	        memtrack_log("MemoryTrackerUnlockMutex: mutex unlock failed\n");
        }
    }

	return ret;
}
