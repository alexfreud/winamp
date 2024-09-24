#ifndef NULLSOFT_API_H
#define NULLSOFT_API_H

#include <windows.h>
int LoadWasabi();
void UnloadWasabi();

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManager;
#define AGAVE_API_PLAYLISTMANAGER playlistManager

#include "../Agave/Config/api_config.h"
extern api_config *config;
#define AGAVE_API_CONFIG config

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgr;
#define WASABI_API_MEMMGR memmgr

#include "../Agave/Language/api_language.h"

// these are custom defines for the out_wave and out_ds embedded implementations
extern HINSTANCE WASABI_API_LNG_HINST_WAV, WASABI_API_LNG_HINST_WAV_ORIG;
extern HINSTANCE WASABI_API_LNG_HINST_DS, WASABI_API_LNG_HINST_DS_ORIG;

#define WASABI_API_LNGSTRING_WAV(uID) WASABI_API_LNGSTR(WASABI_API_LNG_HINST_WAV,WASABI_API_LNG_HINST_WAV_ORIG,uID)
#define WASABI_API_LNGSTRING_BUF_WAV(uID,buf,len) WASABI_API_LNGSTR(WASABI_API_LNG_HINST_WAV,WASABI_API_LNG_HINST_WAV_ORIG,uID,buf,len)

#define WASABI_API_LNGSTRINGW_WAV(uID) WASABI_API_LNGSTRW(WASABI_API_LNG_HINST_WAV,WASABI_API_LNG_HINST_WAV_ORIG,uID)
#define WASABI_API_LNGSTRINGW_BUF_WAV(uID,buf,len) WASABI_API_LNGSTRW(WASABI_API_LNG_HINST_WAV,WASABI_API_LNG_HINST_WAV_ORIG,uID,buf,len)

#define WASABI_API_LNGSTRING_DS(uID) WASABI_API_LNGSTR(WASABI_API_LNG_HINST_DS,WASABI_API_LNG_HINST_DS_ORIG,uID)
#define WASABI_API_LNGSTRING_BUF_DS(uID,buf,len) WASABI_API_LNGSTR(WASABI_API_LNG_HINST_DS,WASABI_API_LNG_HINST_DS_ORIG,uID,buf,len)

#define WASABI_API_LNGSTRINGW_DS(uID) WASABI_API_LNGSTRW(WASABI_API_LNG_HINST_DS,WASABI_API_LNG_HINST_DS_ORIG,uID)
#define WASABI_API_LNGSTRINGW_BUF_DS(uID,buf,len) WASABI_API_LNGSTRW(WASABI_API_LNG_HINST_DS,WASABI_API_LNG_HINST_DS_ORIG,uID,buf,len)

#endif