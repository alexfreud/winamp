#include "precomp_wasabi_bfc.h"

#include "freelist.h"

// define this to turn off freelist behavior
//#define FREELIST_FUCT

FreelistPriv::FreelistPriv() {
  total_allocated = 0;
}

FreelistPriv::~FreelistPriv() {
#ifdef ASSERTS_ENABLED
//  ASSERTPR(total_allocated == 0, "didn't free entire freelist!(1)");
//  ASSERTPR(blocks.getNumItems() == 0, "didn't free entire freelist!(2)");
  if (total_allocated != 0) DebugStringW(L"didn't free entire freelist!(1)\n");
  if (blocks.getNumItems() != 0) DebugStringW(L"didn't free entire freelist!(2)\n");
#endif
}

void *FreelistPriv::getRecord(int typesize, int blocksize, int initialblocksize) {
#ifdef FREELIST_FUCT
  return MALLOC(typesize);
#else
  ASSERT(typesize >= sizeof(void *));
  FLMemBlock *mem = NULL;
  for (int i = 0; i < blocks.getNumItems(); i++) {
    mem = blocks[i];
    if (mem->freelist != NULL) break;
    mem = NULL;
  }
  if (mem == NULL) {
    // figure record count for this new block
    int siz = (blocks.getNumItems() ? blocksize : initialblocksize);
    // allocate another block of memory
    mem = new FLMemBlock(siz*typesize);
    // prelink it into a freelist
    char *record = static_cast<char *>(mem->freelist);
    void **ptr;
    for (int i = 0; i < siz-1; i++) {
      ptr = reinterpret_cast<void **>(record);
      record += typesize;
      *ptr = static_cast<void *>(record);
    }
    // terminate newly made freelist
    ptr = reinterpret_cast<void **>(record);
    *ptr = NULL;
    blocks.addItem(mem);
  }
  // get first free record
  void *ret = mem->freelist;
  // advance freelist *
  mem->freelist = *(static_cast<void **>(mem->freelist));
  mem->nallocated++;
  total_allocated++;
  return ret;
#endif
}

void FreelistPriv::freeRecord(void *record) {
#ifdef FREELIST_FUCT
  FREE(record);
#else
  FLMemBlock *mem=NULL;
  for (int i = 0; i < blocks.getNumItems(); i++) {
    mem = blocks[i];
    if (mem->isMine(reinterpret_cast<MBT*>(record))) break;
    mem = NULL;
  }
  ASSERTPR(mem != NULL, "attempted to free record with no block");
  // stash it back on the block's freelist
  *reinterpret_cast<void **>(record) = mem->freelist;
  mem->freelist = record;
  ASSERT(mem->nallocated > 0);
  mem->nallocated--;
  if (mem->nallocated == 0) {
    blocks.delItem(mem);
  }
  total_allocated--;
#endif
}
