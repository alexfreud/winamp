#pragma once
#include "../../foundation/types.h"
#include "../../nx/nxapi.h"
#include "../../nx/nxstring.h"

#ifdef __cplusplus
extern "C" {
#endif


    /* this thing is meant to be identical to nx_string_t, but we want to try to protect it so they don't get easily cast to nx_string_t
     since they aren't the same on all platforms */
    typedef struct nx_uri_struct_t
    {
			size_t ref_count;
			size_t len;
			wchar_t string[1]; // utf-16
    } nx_uri_struct_t, *nx_uri_t;

    NX_API nx_uri_t NXURIRetain(nx_uri_t string);
    NX_API void NXURIRelease(nx_uri_t string);

	NX_API nx_uri_t NXURIMalloc(size_t characters);
	NX_API int NXURICreateWithNXString(nx_uri_t *uri, nx_string_t string);
	NX_API int NXURICreateFromPath(nx_uri_t *uri, const wchar_t *filename, const nx_uri_t path); // windows only
	NX_API int NXURICreateWithPath(nx_uri_t *uri, const nx_uri_t filename, const nx_uri_t path);
	NX_API int NXURICreateWithNXString(nx_uri_t *new_value, nx_string_t string);
	NX_API int NXURICreateTempForFilepath(nx_uri_t *out_temp, nx_uri_t filename);
	NX_API int NXURICreateTempWithExtension(nx_uri_t *out_temp, const char *extension);
	NX_API int NXURICreateWithUTF8(nx_uri_t *value, const char *utf8);
	NX_API int NXURICreateTemp(nx_uri_t *out_temp);

	// replaces only the filename portion of the path with the desired filename
	// e.g., filepath = /path/to/1.mp3, *out_uri = /path/to/
	NX_API int NXURICreateRemovingFilename(nx_uri_t *out_uri, nx_uri_t filename);


	NX_API int NXURIGetNXString(nx_string_t *string, nx_uri_t uri);

	NX_API size_t NXURIGetLength(nx_uri_t string);

#ifdef __cplusplus
}
#endif