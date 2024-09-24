#ifndef __ON2_MEM_TRACKER_H__
#define __ON2_MEM_TRACKER_H__

/* on2_mem_tracker version info */
#define on2_mem_tracker_version "2.3.1.2"

#define ON2_MEM_TRACKER_VERSION_CHIEF 2
#define ON2_MEM_TRACKER_VERSION_MAJOR 3
#define ON2_MEM_TRACKER_VERSION_MINOR 1
#define ON2_MEM_TRACKER_VERSION_PATCH 2
/* END - on2_mem_tracker version info */

struct MemBlock
{
    size_t addr;
    unsigned int size,
                 line;
    char* file;
    struct MemBlock* prev,
                   * next;
};

#if defined(__cplusplus)
extern "C" {
#endif

/*
    on2_MemoryTrackerInit(int padding_size, int pad_value)
      padding_size - the size of the padding before and after each mem addr.
                     Values > 0 indicate that integrity checks can be performed
                     by inspecting these areas.
      pad_value - the initial value within the padding area before and after
                  each mem addr.
      
    Initializes the memory tracker interface. Should be called before any
    other calls to the memory tracker.
*/
int on2_MemoryTrackerInit(int padding_size, int pad_value);

/*
    on2_MemoryTrackerDestroy()
    Deinitializes the memory tracker interface
*/
void on2_MemoryTrackerDestroy();

/*
    on2_MemoryTrackerAdd(size_t addr, unsigned int size,
                         char * file, unsigned int line)
      addr - memory address to be added to list
      size - size of addr
      file - the file addr was referenced from
      line - the line in file addr was referenced from
    Adds memory address addr, it's size, file and line it came from
    to the memory tracker allocation table
*/
void on2_MemoryTrackerAdd(size_t addr, unsigned int size,
                          char * file, unsigned int line);

/*
    on2_MemoryTrackerAdd(size_t addr, unsigned int size, char * file, unsigned int line)
      addr - memory address to be added to be removed
    Removes the specified address from the memory tracker's allocation
    table
    Return:
      0: on success
      -1: if memory allocation table's mutex could not be locked
      -2: if the addr was not found in the list
*/
int on2_MemoryTrackerRemove(size_t addr);

/*
    on2_MemoryTrackerFind(unsigned int addr)
      addr - address to be found in the memory tracker's
             allocation table
    Return:
        If found, pointer to the memory block that matches addr
        NULL otherwise
*/
struct MemBlock* on2_MemoryTrackerFind(size_t addr);

/*
    on2_MemoryTrackerDump()
    Dumps the current contents of the memory
    tracker allocation table
*/
void on2_MemoryTrackerDump();

/*
    on2_MemoryTrackerCheckIntegrity()
    If a padding_size was provided to on2_MemoryTrackerInit()
    This function will verify that the region before and after each
    memory address contains the specified pad_value. Should the check
    fail, the filename and line of the check will be printed out.
*/
void on2_MemoryTrackerCheckIntegrity(char* file, unsigned int line);

/*
    on2_MemoryTrackerSetLogType
      type - value representing the logging type to use
      option - type specific option. This will be interpreted differently
               based on the type.
    Sets the logging type for the memory tracker.
    Values currently supported:
      0: if option is NULL, log to stderr, otherwise interpret option as a
         filename and attempt to open it.
      -1: Use OutputDebugString (WIN32 only), option ignored
    Return:
      0: on success
      -1: if the logging type could not be set, because the value was invalid
          or because a file could not be opened
*/
int on2_MemoryTrackerSetLogType(int type, char* option);

#if !defined(__ON2_MEM_TRACKER_C__) && !defined(__ON2_MEM_C__)
#if ON2_TRACK_MEM_USAGE
#define on2_MemoryTrackerCheckIntegrity() on2_MemoryTrackerCheckIntegrity(__FILE__, __LINE__)
#else
#define on2_MemoryTrackerCheckIntegrity()
#endif
#endif

#if defined(__cplusplus)
}
#endif

#endif //__ON2_MEM_TRACKER_H__
