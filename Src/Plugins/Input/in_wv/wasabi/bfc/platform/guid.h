#ifndef _GUID_H
#define _GUID_H

#include "types.h"
#include "platform.h"


#ifdef __cplusplus
#ifndef GUID_EQUALS_DEFINED
#define GUID_EQUALS_DEFINED
#include <memory.h>
static __inline int operator ==(const GUID &a, const GUID &b) {
  return !memcmp(&a, &b, sizeof(GUID));
}
static __inline int operator !=(const GUID &a, const GUID &b) {
  return !!memcmp(&a, &b, sizeof(GUID));
}
#endif	//GUID_EQUALS_DEFINED
#endif	//__cplusplus

static const GUID INVALID_GUID = { 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0} };
static const GUID GENERIC_GUID = { 0xFFFFFFFF, 0xFFFF, 0xFFFF, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} };

#endif
