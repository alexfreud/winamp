#include "memblock.h"
#include <bfc/bfc_assert.h>

#ifdef _DEBUG
int memblocks_totalsize=0;
#endif

VoidMemBlock::VoidMemBlock(int _size, const void *data) {
  mem = NULL;
  size = 0;
  setSize(_size);
  if (data != NULL && size > 0) MEMCPY(mem, data, size);
}

VoidMemBlock::~VoidMemBlock() {
#ifdef _DEBUG
  memblocks_totalsize -= size;
#endif
  FREE(mem);
}

void *VoidMemBlock::setSize(int newsize) {
#ifdef _DEBUG
  memblocks_totalsize -= size;
#endif
  ASSERT(newsize >= 0);
  if (newsize < 0) newsize = 0;
  if (newsize == 0) {
    FREE(mem);
    mem = NULL;
  } else if (size != newsize) {
    mem = REALLOC(mem, newsize);
  }
  size = newsize;
#ifdef _DEBUG
  memblocks_totalsize += size;
#endif
  return getMemory();
}

void *VoidMemBlock::setMinimumSize(int newminsize, int increment) {
  if (newminsize > size) setSize(newminsize+increment);
  return getMemory();
}

void VoidMemBlock::setMemory(const void *data, int datalen, int offsetby) {
  if (datalen <= 0) return;
  ASSERT(mem != NULL);
  ASSERT(offsetby >= 0);
  char *ptr = reinterpret_cast<char *>(mem);
  ASSERT(ptr + offsetby + datalen <= ptr + size);
  MEMCPY(ptr + offsetby, data, datalen);
}

int VoidMemBlock::getSize() const {
  return size;
}

int VoidMemBlock::isMine(void *ptr) {
  return (ptr >= mem && ptr < (char*)mem + size);
}

void VoidMemBlock::zeroMemory() {
  if (mem == NULL || size < 1) return;
  MEMZERO(mem, size);
}
