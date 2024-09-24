#pragma once
#include "nx/nxuri.h"

#ifdef __cplusplus
extern "C" {
#endif
	enum
	{
		EXTENSION_FOR_PLAYBACK,
		EXTENSION_FOR_METADATA,
		EXTENSION_FOR_AUDIO_DECODE,
	};
	bool IsMyExtension(nx_uri_t filename, int search_style);
	int EnumerateExtensions(unsigned int index, nx_string_t *extension, int search_style);

#ifdef __cplusplus
}
#endif