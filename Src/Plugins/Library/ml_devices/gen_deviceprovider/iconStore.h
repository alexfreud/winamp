#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_ICON_STORE_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_ICON_STORE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef struct IconStore IconStore;
typedef BOOL (*IconEnumerator)(const wchar_t *path, unsigned int width, unsigned int height, void *user);

IconStore *
IconStore_Create();

void
IconStore_Destroy(IconStore *self);

BOOL
IconStore_Add(IconStore *self, 
			  const wchar_t *path, 
			  unsigned int width, 
			  unsigned int height);

BOOL
IconStore_RemovePath(IconStore *self, 
				 const wchar_t *path);

BOOL
IconStore_Remove(IconStore *self, 
				 unsigned int width, 
				 unsigned int height);

BOOL
IconStore_Get(IconStore *self, 
			  wchar_t *buffer, 
			  size_t bufferMax,
			  unsigned int width, 
			  unsigned int height);

BOOL
IconStore_SetBasePath(IconStore *self, 
					  const wchar_t *path);

IconStore *
IconStore_Clone(IconStore *self);

BOOL
IconStore_Enumerate(IconStore *self,
					IconEnumerator callback, 
					void *user);

#endif //_NULLSOFT_WINAMP_GEN_DEVICE_ICON_STORE_HEADER