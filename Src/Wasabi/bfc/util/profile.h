//
// profile.h
// Profile support.
// Programed by XuYiFeng 1995.4.25, All rights reserved.
//

#ifndef __PROFILE_H
#define __PROFILE_H

#ifdef __cplusplus
extern "C" {
#endif

long GetPrivateProfileInt( const char *section, const char *entry, long defaultInt,
                   const char *fileName );
int GetPrivateProfileString( const char *section, const char *entry,
                      const char *defaultString, char *buffer,
                      int   bufLen, const char *fileName );
int WritePrivateProfileString( const char *section, const char *entry,
                        const char *string, const char *fileName );

#ifdef __cplusplus
}
#endif

#endif
