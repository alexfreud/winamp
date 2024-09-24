#include "main.h"
#include "api.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoWideFn.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoCharFn.h"
#include "../nu/AutoLock.h"
#include "../nu/ns_wc.h"
#include <malloc.h>

using namespace Nullsoft::Utility;
/* Get Extended File Info */

typedef int (__cdecl *GetANSIInfo)(const char *fn, const char *data, char *dest, size_t destlen);
typedef int (__cdecl *GetUnicodeInfo)(const wchar_t *fn, const char *data, wchar_t *dest, size_t destlen);

int ConvertGetExtendedFileInfo(GetUnicodeInfo getter, const char *fn, const char *data, char *dest, size_t destlen)
{
	wchar_t *destW = 0;

	if (dest && destlen)
	{
		destW = (wchar_t *)_malloca(destlen * sizeof(wchar_t));
		memset(destW, 0, destlen * sizeof(wchar_t));
	}

	int retVal = getter(AutoWideFn(fn), data, destW, destlen);
	if (retVal && dest && destlen)
		WideCharToMultiByteSZ(CP_ACP, 0, destW, -1, dest, (int)destlen, 0, 0);

	if (destW) _freea(destW);
	return retVal;
}

int ConvertGetExtendedFileInfo(GetANSIInfo getter, const char *fn, const char *data, wchar_t *dest, size_t destlen)
{
	char *destA = 0;

	if (dest && destlen)
	{
		destA = (char *)_malloca(destlen * sizeof(char));
		memset(destA, 0, destlen * sizeof(char));
	}

	int retVal = getter(fn, data, destA, destlen);
	if (retVal && dest && destlen)
		MultiByteToWideCharSZ(CP_ACP, 0, destA, -1, dest, (int)destlen);

	if (destA) _freea(destA);
	return retVal;
}

static GetANSIInfo gefi;
static GetUnicodeInfo gefiW;
static char cachedExt[16];
LockGuard getMetadataGuard;

int in_get_extended_fileinfo(const char *fn, const char *metadata, char *dest, size_t destlen)
{
	AutoLock lock(getMetadataGuard);

	In_Module *i=0;
	char ext[16] = {0};
	int a = 0;

	// do some extra checks on the params as quite a few clients crash when accessing here
	try {
		if (destlen > 65536 || destlen == 0 || !fn || (unsigned int)(ULONG_PTR)fn < 65536 ||
			fn && !*fn || !metadata || (unsigned int)(ULONG_PTR)metadata < 65536 ||
			metadata && !*metadata || !dest ||  (unsigned int)(ULONG_PTR)dest < 65536)
			return 0;
	} catch (...) {
		return 0;
	}

	if (dest)
		memset(dest, 0, destlen);

	extension_ex(fn, ext, sizeof(ext));

	if (!_stricmp(cachedExt, ext) && *ext)
	{
		if (gefi)
			return gefi(fn, metadata, dest, destlen);
		else if (gefiW) // should always be true if we got this far
			return ConvertGetExtendedFileInfo(gefiW, fn, metadata, dest, destlen);
		else
			return 0;
	}

	while (a >= 0)
	{
		i = in_setmod_noplay(AutoWideFn(fn), &a);
		if (!i)
			break;

		if (a >= 0) a++;

		lstrcpynA(cachedExt, ext, sizeof(cachedExt)/sizeof(*cachedExt));

		gefi = (GetANSIInfo)GetProcAddress(i->hDllInstance, "winampGetExtendedFileInfo");
		gefiW = (GetUnicodeInfo)GetProcAddress(i->hDllInstance, "winampGetExtendedFileInfoW");
		if (gefi || gefiW)
			break;
	}

	// if no one claimed it, then check if it's the currently playing track
	// benski> TODO: there is a race condition here.  In theory, in_mod could change between the lstrcmpiW and the assignment
	if (!i && !lstrcmpiW(FileName, AutoWide(fn))) 
	{
		i=in_mod;
		if (i)
		{
			cachedExt[0]=0;
			gefi = (GetANSIInfo)GetProcAddress(i->hDllInstance, "winampGetExtendedFileInfo");
			gefiW = (GetUnicodeInfo)GetProcAddress(i->hDllInstance, "winampGetExtendedFileInfoW");
		}
	}

	if (!i)
		return 0;

	if (gefi)
		return gefi(fn, metadata, dest, destlen);
	else if (gefiW)
		return ConvertGetExtendedFileInfo(gefiW, fn, metadata, dest, destlen);
	else
		return 0;
}

int in_get_extended_fileinfoW(const wchar_t *fn, const wchar_t *metadata, wchar_t *dest, size_t destlen)
{
	AutoLock lock(getMetadataGuard);

	In_Module *i=0;
	char ext[16] = {0};
	int a = 0;

	// do some extra checks on the params as quite a few clients crash when accessing here
	try {
		if (destlen > 65536 || destlen == 0 || !fn || (unsigned int)(ULONG_PTR)fn < 65536 ||
			fn && !*fn || !metadata || (unsigned int)(ULONG_PTR)metadata < 65536 ||
			metadata && !*metadata || !dest ||  (unsigned int)(ULONG_PTR)dest < 65536)
			return 0;
	} catch (...) {
		return 0;
	}

	if (dest)
		memset(dest, 0, destlen);

	AutoCharFn charFn(fn);
	extension_ex(charFn, ext, sizeof(ext));

	if (!_stricmp(cachedExt, ext) && *ext)
	{
		if (gefiW) // should always be true if we got this far
			return gefiW(fn, AutoChar(metadata), dest, destlen);
		if (gefi)
			return ConvertGetExtendedFileInfo(gefi, charFn, AutoChar(metadata), dest, destlen);
		else
			return 0;
	}

	while (a >= 0)
	{
		i = in_setmod_noplay(fn, &a);
		if (!i)
			break;

		if (a >= 0) a++;

		lstrcpynA(cachedExt, ext, sizeof(cachedExt)/sizeof(*cachedExt));

		gefi = (GetANSIInfo)GetProcAddress(i->hDllInstance, "winampGetExtendedFileInfo");
		gefiW = (GetUnicodeInfo)GetProcAddress(i->hDllInstance, "winampGetExtendedFileInfoW");
		if (gefi || gefiW)
			break;
	}

	// if no one claimed it, then check if it's the currently playing track
	// benski> TODO: there is a race condition here.  In theory, in_mod could change between the lstrcmpiW and the assignment
	if (!i && !lstrcmpiW(FileName, fn)) // currently playing file?
	{
		i=in_mod;
		if (i)
		{
			cachedExt[0]=0;
			gefi = (GetANSIInfo)GetProcAddress(i->hDllInstance, "winampGetExtendedFileInfo");
			gefiW = (GetUnicodeInfo)GetProcAddress(i->hDllInstance, "winampGetExtendedFileInfoW");
		}
	}

	if (!i)
		return 0;

	if (gefiW)
		return gefiW(fn, AutoChar(metadata), dest, destlen);
	else if (gefi)
		return ConvertGetExtendedFileInfo(gefi, charFn, AutoChar(metadata), dest, destlen);
	else
		return 0;
}

/* Set Extended File Info */

typedef int (__cdecl *SetANSIInfo)(const char *fn, const char *metadata, const char *data);
typedef int (__cdecl *SetUnicodeInfo)(const wchar_t *fn, const char *metadata, const wchar_t *data);

static SetANSIInfo sefi;
static SetUnicodeInfo sefiW;
static int (*wefi)();
static char cachedExtSet[16];

static int ConvertSetExtendedFileInfo(SetUnicodeInfo setter, const char *fn, const char *metadata, const char *data)
{
	return setter(AutoWide(fn), metadata, AutoWide(data));
}

static int ConvertSetExtendedFileInfo(SetANSIInfo setter, const char *fn, const char *metadata, const wchar_t *data)
{
	return setter(fn, metadata, AutoChar(data));
}

int in_set_extended_fileinfo(const char *fn, const char *metadata, char *data)
{
	AutoLock lock(getMetadataGuard);
	char ext[16] = {0};
	int a = 0;

	extension_ex(fn, ext, sizeof(ext));
	if (!_stricmp(cachedExtSet, ext) && *ext)
	{
		if (sefi)
			return sefi(fn, metadata, data);
		else if (sefiW)
			return ConvertSetExtendedFileInfo(sefiW, fn, metadata, data);
		else
			return 0;
	}

	while (a >= 0)
	{
		wefi = NULL;
		sefi = NULL; // if we fail finding an input plugin, dont let in_write_extended_fileinfo work :)
		sefiW = NULL;
		In_Module *i = in_setmod_noplay(AutoWideFn(fn), &a);
		if (a >= 0) a++;
		cachedExtSet[0] = 0;
		if (!i) return 0;
		lstrcpynA(cachedExtSet, ext, 16);

		sefi = (SetANSIInfo)GetProcAddress(i->hDllInstance, "winampSetExtendedFileInfo");
		sefiW = (SetUnicodeInfo)GetProcAddress(i->hDllInstance, "winampSetExtendedFileInfoW");
		wefi = (int (__cdecl *)())GetProcAddress(i->hDllInstance, "winampWriteExtendedFileInfo");
		if (sefi || sefiW) break;
	}

	if (sefi)
		return sefi(fn, metadata, data);
	else if (sefiW)
		return ConvertSetExtendedFileInfo(sefiW, fn, metadata, data);
	else
		return 0;
}

int in_set_extended_fileinfoW(const wchar_t *fn, const wchar_t *metadata, wchar_t *data)
{
	AutoLock lock(getMetadataGuard);

	char ext[16] = {0};
	int a = 0;

	AutoCharFn charFn(fn);
	extension_ex(charFn, ext, sizeof(ext));
	if (!_stricmp(cachedExtSet, ext) && *ext)
	{
		if (sefiW)
			return sefiW(fn, AutoChar(metadata), data);
		else if (sefi)
			return ConvertSetExtendedFileInfo(sefi, charFn, AutoChar(metadata), data);
		else
			return 0;
	}

	while (a >= 0)
	{
		wefi = NULL;
		sefi = NULL; // if we fail finding an input plugin, dont let in_write_extended_fileinfo work :)
		sefiW = NULL;
		In_Module *i = in_setmod_noplay(fn, &a);
		if (a >= 0) a++;
		cachedExtSet[0] = 0;
		if (!i) return 0;
		lstrcpynA(cachedExtSet, ext, 16);

		sefi = (SetANSIInfo)GetProcAddress(i->hDllInstance, "winampSetExtendedFileInfo");
		sefiW = (SetUnicodeInfo)GetProcAddress(i->hDllInstance, "winampSetExtendedFileInfoW");
		wefi = (int (__cdecl *)())GetProcAddress(i->hDllInstance, "winampWriteExtendedFileInfo");
		if (sefi || sefiW) break;
	}
	if (sefiW)
		return sefiW(fn, AutoChar(metadata), data);
	else if (sefi)
		return ConvertSetExtendedFileInfo(sefi, charFn, AutoChar(metadata), data);
	else
		return 0;
}

int in_write_extended_fileinfo()
{
	AutoLock lock(getMetadataGuard);
	if (!wefi)
		return 0;
	return
	  wefi();
}

inline void COPY_METADATA(const wchar_t *src, const wchar_t *dest, const wchar_t *item, wchar_t *buf, size_t buflen)
{
	if (NULL != src && NULL != item && NULL != buf)
	{
		buf[0]=0;
		extendedFileInfoStructW efis=
		{
			src,
			item,
			buf,
			buflen,
		};

		if (SendMessageW(hMainWindow,WM_WA_IPC,(WPARAM)&efis,IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE))
			in_set_extended_fileinfoW(dest, item, buf);
	}
	
}


//#define COPY_METADATA(src, dest, item, buf, buflen) { buf[0]=0; if (in_get_extended_fileinfoW(src, item, buf, buflen)) in_set_extended_fileinfoW(dest, item, buf); }
void CopyExtendedFileInfo(const wchar_t *source, const wchar_t *destination)
{
	wchar_t bigData[32768] = {0}; // hopefully big enough for all reasonable metadata

	COPY_METADATA(source, destination, L"title", bigData, 32768);
	COPY_METADATA(source, destination, L"artist", bigData, 32768);
	COPY_METADATA(source, destination, L"albumartist", bigData, 32768);
	COPY_METADATA(source, destination, L"album", bigData, 32768);
	COPY_METADATA(source, destination, L"genre", bigData, 32768);
	COPY_METADATA(source, destination, L"year", bigData, 32768);
	COPY_METADATA(source, destination, L"disc", bigData, 32768);
	COPY_METADATA(source, destination, L"publisher", bigData, 32768);
	COPY_METADATA(source, destination, L"comment", bigData, 32768);
	COPY_METADATA(source, destination, L"track", bigData, 32768);
	COPY_METADATA(source, destination, L"tool", bigData, 32768);
	COPY_METADATA(source, destination, L"composer", bigData, 32768);
	COPY_METADATA(source, destination, L"conductor", bigData, 32768);
	COPY_METADATA(source, destination, L"bpm", bigData, 32768);
	COPY_METADATA(source, destination, L"GracenoteFileID", bigData, 32768);
	COPY_METADATA(source, destination, L"GracenoteExtData", bigData, 32768);
	in_write_extended_fileinfo();
	
	//albumArt->CopyAlbumArt(source, destination);
}