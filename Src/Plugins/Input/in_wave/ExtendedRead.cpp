#include "main.h"
#include <stddef.h>

struct ExtendedRead
{
	SF_INFO info;
	SNDFILE *soundFile;
	int bits;
	int frameSize;
};

extern "C"
{
	//returns handle!=0 if successful, 0 if error
	//size will return the final nb of bytes written to the output, -1 if unknown
	__declspec( dllexport ) intptr_t winampGetExtendedRead_openW( const wchar_t *fn, int *size, int *bps, int *nch, int *srate )
	{
		ExtendedRead *extRead = (ExtendedRead *)calloc( 1, sizeof( ExtendedRead ) );

		extRead->info.format = 0;
		extRead->soundFile = sf_wchar_open( fn, SFM_READ, &extRead->info );
		if ( !extRead->soundFile )
		{
			free( extRead );
			return 0;
		}

		switch ( extRead->info.format & SF_FORMAT_SUBMASK )
		{
			case SF_FORMAT_FLOAT:
			case SF_FORMAT_DOUBLE:
				sf_command( extRead->soundFile, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE );
				break;
		}

		extRead->bits = 16; // TODO: calculate bits per sample (what we want to use, not necessarily what the file has)

		*bps   = extRead->bits;
		*nch   = extRead->info.channels;
		*srate = extRead->info.samplerate;

		extRead->frameSize = ( extRead->bits / 8 ) * extRead->info.channels;
		
		*size = (int)extRead->info.frames * extRead->frameSize; // TODO: is this correct?

		return (intptr_t)extRead;
	}

	//returns nb of bytes read. -1 if read error (like CD ejected). if (ret<len), EOF is assumed
	__declspec( dllexport ) intptr_t winampGetExtendedRead_getData( intptr_t handle, char *dest, int len, int *killswitch )
	{
		ExtendedRead *extRead = (ExtendedRead *)handle;
		
		sf_count_t framesRead = sf_readf_short( extRead->soundFile, (short *)dest, len / extRead->frameSize );
		
		return (int)framesRead * extRead->frameSize;
	}

	// return nonzero on success, zero on failure.
	__declspec( dllexport ) int winampGetExtendedRead_setTime( intptr_t handle, int time_in_ms )
	{
		ExtendedRead *extRead = (ExtendedRead *)handle;
		if ( !extRead->info.seekable )
			return 0;

		int frames = MulDiv( time_in_ms, extRead->info.samplerate, 1000 ); // TODO: verify calculation
		
		sf_seek( extRead->soundFile, frames, SEEK_SET );
		
		return 1;
	}

	__declspec( dllexport ) void winampGetExtendedRead_close( intptr_t handle )
	{
		ExtendedRead *extRead = (ExtendedRead *)handle;
		sf_close( extRead->soundFile );

		free( extRead );
	}
}
