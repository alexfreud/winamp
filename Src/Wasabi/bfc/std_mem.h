#ifndef _STD_MEM_H
#define _STD_MEM_H

#include <bfc/platform/platform.h>
#include <string.h>

wchar_t *WMALLOC(size_t size);
void *MALLOC(size_t size);
void *CALLOC(size_t records, size_t recordsize);
void *REALLOC(void *ptr, size_t size);
void FREE(void *ptr);

void *MEMDUP(const void *src, size_t n);
void MEMCPY(void *dest, const void *src, size_t n);
void MEMCPY_(void *dest, const void *src, size_t n);
void MEMCPY32(void *dest, const void *src, size_t words);

#ifdef __cplusplus
static __inline int MEMCMP(const void *buf1, const void *buf2, size_t count) {
  return memcmp(buf1, buf2, count);
}
static __inline void MEMSET(void *dest, int c, size_t n) {
  memset(dest, c, n);
}
static __inline void MEMZERO(void *dest, size_t nbytes) {
  memset(dest, 0, nbytes);
}
#else
#define MEMCMP memcmp
#define MEMSET memset
#define MEMZERO(dest, nbytes) memset(dest, 0, nbytes)
#endif

#ifdef __cplusplus

// these are for structs and basic classes only
static __inline void ZERO(int &obj) { obj = 0; }
template<class T>
inline void ZERO(T &obj) { MEMZERO(&obj, sizeof(T)); }

// generic version that should work for all types
template<class T>
inline void MEMFILL(T *ptr, T val, unsigned int n) {
  for (int i = 0; i < n; i++) ptr[i] = val;
}

// asm 32-bits version
void  MEMFILL32(void *ptr, unsigned long val, unsigned int n);

// helpers that call the asm version
template<>
inline void MEMFILL<unsigned long>(unsigned long *ptr, unsigned long val, unsigned int n) { MEMFILL32(ptr, val, n); }

template<>
void  MEMFILL<unsigned short>(unsigned short *ptr, unsigned short val, unsigned int n);

// int
template<>
inline void MEMFILL<int>(int *ptr, int val, unsigned int n) {
  MEMFILL32(ptr, *reinterpret_cast<unsigned long *>(&val), n);
}

// float
template<>
inline void MEMFILL<float>(float *ptr, float val, unsigned int n) {
  MEMFILL32(ptr, *reinterpret_cast<unsigned long *>(&val), n);
}

#endif	// __cplusplus defined

#endif
