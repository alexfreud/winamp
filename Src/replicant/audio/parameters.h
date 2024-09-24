#pragma once
#include "foundation/types.h"
#include "foundation/guid.h"


namespace nsaudio
{
	enum
	{
		/* when this is set, the const void * passed to AudioOutput_Output is assumed to be an array of channels,
		e.g. (format_type == nsaudio_type_float && (format_flags & FORMAT_FLAG_NONINTERLEAVED) && number_of_channels == 2)
		means that you pass a float *[2] to AudioOutput_Output 
		*/
		FORMAT_FLAG_INTERLEAVED=0x1,
		FORMAT_FLAG_NONINTERLEAVED=0x2,
		FORMAT_FLAG_NATIVE_ENDIAN=0x4,
		FORMAT_FLAG_LITTLE_ENDIAN=0x8, /* audio is SPECIFICALLY little endian (as opposed to native endian on little-endian machines) */
		FORMAT_FLAG_BIG_ENDIAN=0x10, /* audio is SPECIFICALLY big endian (as opposed to native endian on big-endian machines) */
		FORMAT_FLAG_SIGNED=0x20, /* e.g. 8 bit PCM is typically unsigned (0-255) */
		FORMAT_FLAG_UNSIGNED=0x40, /* e.g. 8 bit PCM is typically unsigned (0-255) */

		 /* so that you can check if a flag was set that you don't understand */
		FORMAT_FLAG_VALID_INTERLEAVE = FORMAT_FLAG_INTERLEAVED|FORMAT_FLAG_NONINTERLEAVED,
		FORMAT_FLAG_VALID_ENDIAN = FORMAT_FLAG_NATIVE_ENDIAN|FORMAT_FLAG_LITTLE_ENDIAN|FORMAT_FLAG_BIG_ENDIAN,
		FORMAT_FLAG_VALID_SIGNED=FORMAT_FLAG_SIGNED|FORMAT_FLAG_UNSIGNED,
		FORMAT_FLAG_VALID_MASK=FORMAT_FLAG_VALID_INTERLEAVE|FORMAT_FLAG_VALID_ENDIAN|FORMAT_FLAG_VALID_SIGNED,
	};

	// {4B80932C-E55F-4969-91EA-772584ABEDC2}
	static const GUID format_type_pcm = 
	{ 0x4b80932c, 0xe55f, 0x4969, { 0x91, 0xea, 0x77, 0x25, 0x84, 0xab, 0xed, 0xc2 } };

	// {6D47717F-A383-4CF8-BB1E-72254BE3F9DC}
	static const GUID format_type_float = 
	{ 0x6d47717f, 0xa383, 0x4cf8, { 0xbb, 0x1e, 0x72, 0x25, 0x4b, 0xe3, 0xf9, 0xdc } };

	struct Parameters
	{
		double sample_rate;
		GUID format_type; // PCM, floating point, SPDIF pass-thru, etc.
		unsigned int format_flags; // endian, interleaved, signed
		unsigned int bytes_per_sample; // e.g. 4 for 20bit in a 32bit container
		unsigned int bits_per_sample; // number of valid bits within the sample
		unsigned int number_of_channels; 
		unsigned int channel_layout;
	};
};
