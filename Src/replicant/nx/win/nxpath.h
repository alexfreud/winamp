#pragma once
#include "foundation/types.h"
#include "nx/nxapi.h"
#include "nx/nxstring.h"
#include "nx/nxuri.h"

#ifdef __cplusplus
extern "C" {
#endif

	// returns index into the extension list of a match extension
	// returns >= num_extensions on failure
NX_API size_t NXPathMatchExtensionList(nx_uri_t filename, nx_string_t *extension_list, size_t num_extensions);

// return NErr_True / NErr_False
NX_API int NXPathMatchExtension(nx_uri_t filename, nx_string_t extension);

// return NErr_True / NErr_False
NX_API int NXPathProtocol(nx_uri_t filename, const char *protocol);

// return NErr_True / NErr_False
NX_API int NXPathIsURL(nx_uri_t filename);

#ifdef __cplusplus
}
#endif