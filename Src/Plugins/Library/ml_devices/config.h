#ifndef _NULLSOFT_WINAMP_ML_DEVICES_CONFIG_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_CONFIG_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


unsigned long 
Config_ReadString(const char *section,
				  const char *key, 
				  const char *defaultValue,
				  char *returnedStrign, 
				  unsigned long size);

unsigned int
Config_ReadInt(const char *section,
			   const char *key, 
			   int defaultValue);

BOOL
Config_ReadBool(const char *section,
				const char *key,
				BOOL defaultValue);

BOOL
Config_WriteString(const char *section, 
				   const char *key, 
				   const char *value);


BOOL
Config_WriteInt(const char *section, 
				const char *key, 
				int value);

BOOL
Config_WriteBool(const char *section,
				 const char *key,
				 BOOL value);

#endif // _NULLSOFT_WINAMP_ML_DEVICES_CONFIG_HEADER
