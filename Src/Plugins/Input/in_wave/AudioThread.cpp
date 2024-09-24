#include <windows.h>
#include "main.h"
#include "../Winamp/wa_ipc.h"
#include "config.h"
#include "api__in_wave.h"
#include <shlwapi.h>
#include "VirtualIO.h"

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
    { 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

HANDLE audioThread = INVALID_HANDLE_VALUE;
DWORD WINAPI ThreadProcedure( void *data );

#define kill    events[0]
#define running events[1]

HANDLE stopped = 0;
HANDLE events[ 2 ] = { 0 };

static size_t   bufferSize  = 0;
static char    *audioBuffer = 0;
static int      frameSize   = 0; // in bytes
static int      endOfFile   = 0;
static int      bits        = 0;
static SF_INFO  info;
static void    *reader      = 0;


int CalcBits()
{
	switch (info.format & SF_FORMAT_SUBMASK)
	{
	case SF_FORMAT_DOUBLE:
		return 64;

	case SF_FORMAT_PCM_32:
	case SF_FORMAT_FLOAT:
		return 32;

	case SF_FORMAT_DWVW_24:
	case SF_FORMAT_PCM_24:
		return 24;

	case SF_FORMAT_DPCM_16:
	case SF_FORMAT_DWVW_16:
	case SF_FORMAT_PCM_16:
		return 16;

	//case SF_FORMAT_PCM_S8: // cut, because 8bits is assumed unsigned
	case SF_FORMAT_PCM_U8:
	case SF_FORMAT_DPCM_8:
		return 8;

	default: return 16;
	}
}

int CalcBitRate( const SF_INFO *info )
{
	switch ( info->format & SF_FORMAT_SUBMASK )
	{
		case SF_FORMAT_PCM_S8:
		case SF_FORMAT_PCM_U8:
		case SF_FORMAT_DPCM_8:
			return MulDiv( 8 * info->channels, info->samplerate, 1000 );
		case SF_FORMAT_DWVW_12:
			return MulDiv( 12 * info->channels, info->samplerate, 1000 );
		case SF_FORMAT_DPCM_16:
		case SF_FORMAT_DWVW_16:
		case SF_FORMAT_PCM_16:
			return MulDiv( 16 * info->channels, info->samplerate, 1000 );
		case SF_FORMAT_DWVW_24:
		case SF_FORMAT_PCM_24:
			return MulDiv( 24 * info->channels, info->samplerate, 1000 );
		case SF_FORMAT_PCM_32:
		case SF_FORMAT_FLOAT:
			return MulDiv( 32 * info->channels, info->samplerate, 1000 );
		case SF_FORMAT_DOUBLE:
			return MulDiv( 64 * info->channels, info->samplerate, 1000 );

		case SF_FORMAT_G721_32:
			return 32;

		case SF_FORMAT_G723_24:
			return 24;

		case SF_FORMAT_G723_40:
			return 40;
		case SF_FORMAT_MS_ADPCM:
		case SF_FORMAT_VOX_ADPCM:
		case SF_FORMAT_IMA_ADPCM:
			return MulDiv( 4 * info->channels, info->samplerate, 1000 );
		default:
			return MulDiv( 16 * info->channels, info->samplerate, 1000 );
	}
}


void CALLBACK APCSeek( ULONG_PTR p_data )
{
	endOfFile = 0;

	int time_in_ms = (int)p_data;
	int frames     = MulDiv( time_in_ms, info.samplerate, 1000 ); // TODO: verify calculation

	sf_seek( sndFile, frames, SEEK_SET );

	plugin.outMod->Flush( time_in_ms );
}

void CALLBACK APCPause( ULONG_PTR p_data )
{
	int pause = (int)p_data;
	if ( pause )
		ResetEvent( running );
	else
		SetEvent( running );

	plugin.outMod->Pause( !!pause );
}

void CALLBACK APCStart( ULONG_PTR p_data )
{
	endOfFile = 0;
	const wchar_t *file = (const wchar_t *)p_data;

	info.format = 0;
	if ( PathIsURLW( file ) )
	{
		reader = CreateReader( file );
		if ( reader )
			sndFile = sf_open_virtual( &httpIO, SFM_READ, &info, reader );
	}
	else  // It's a local file
	{
		sndFile = sf_wchar_open( file, SFM_READ, &info );
	}

	if ( !sndFile )
	{
		if ( WaitForSingleObject( kill, 200 ) == WAIT_TIMEOUT )
			PostMessage( plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0 );

		return;
	}

	currentSongLength = MulDiv( (int)info.frames, 1000, info.samplerate ); // TODO: is this correct?
	switch ( info.format & SF_FORMAT_SUBMASK )
	{
		case SF_FORMAT_FLOAT:
		case SF_FORMAT_DOUBLE:
			sf_command( sndFile, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE );
			break;
	}

	bits = CalcBits();

	size_t config_bits = AGAVE_API_CONFIG->GetUnsigned( playbackConfigGroupGUID, L"bits", 16 );
	if ( config_bits == 16 && config_bits < (size_t)bits )
		bits = (int)config_bits;

	if ( bits < 16 && config_upsample8bit )
		bits = 16;

	int latency = plugin.outMod->Open( info.samplerate, info.channels, bits, -1, -1 );
	if ( latency < 0 )
	{
		sf_close( sndFile );
		if ( reader )
			DestroyReader( reader );

		reader  = 0;
		sndFile = NULL;

		if ( WaitForSingleObject( kill, 200 ) == WAIT_TIMEOUT )
			PostMessage( plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0 );

		return;
	}

	frameSize = ( bits / 8 ) * info.channels;

	plugin.SAVSAInit( latency, info.samplerate );
	plugin.VSASetInfo( info.samplerate, info.channels );

	int bitrate = CalcBitRate( &info );

	plugin.SetInfo( bitrate, info.samplerate / 1000, info.channels, 1 );
	plugin.is_seekable = info.seekable;

	plugin.outMod->SetVolume( volume );
	plugin.outMod->SetPan( pan );

	size_t requiredBufferSize = 576 * frameSize * 2; // * 2 for dsp bullshit
	if ( requiredBufferSize > bufferSize )
	{
		free( audioBuffer );

		audioBuffer = (char *)calloc( requiredBufferSize, sizeof( char ) );
		bufferSize  = requiredBufferSize;
	}

	SetEvent( running );
}

void CALLBACK APCStop( ULONG_PTR p_data )
{
	if ( sndFile )
	{
		sf_close( sndFile );
		if ( reader )
			DestroyReader( reader );

		reader  = 0;
		sndFile = NULL;

	}

	ResetEvent( running );
	SetEvent( stopped );
}


void Kill()
{
	SetEvent( kill );
}

void AudioThreadInit()
{
	DWORD id;

	kill        = CreateEvent( NULL, TRUE, FALSE, NULL );
	running     = CreateEvent( NULL, TRUE, FALSE, NULL );
	stopped     = CreateEvent( NULL, FALSE, FALSE, NULL );
	audioThread = CreateThread( NULL, 0, ThreadProcedure, 0, 0, &id );

	if ( audioThread )
		SetThreadPriority( audioThread, (int)AGAVE_API_CONFIG->GetInt( playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST ) );
}

void AudioThreadQuit()
{
	free( audioBuffer );
	audioBuffer = 0;
	
	bufferSize = 0;
	
	CloseHandle( running );
	running = 0;

	CloseHandle( kill );
	kill = 0;

	CloseHandle( stopped );
	stopped = 0;

	CloseHandle( audioThread );
	audioThread = 0;
}


DWORD WINAPI ThreadProcedure( void *data )
{
	DWORD result;
	sf_count_t framesRead;
	while ( true )
	{
		result = WaitForMultipleObjectsEx( 2, events, FALSE, INFINITE, TRUE );
		if ( result == WAIT_OBJECT_0 ) // kill thread
			return 0;

		if ( result == ( WAIT_OBJECT_0 + 1 ) )
		{
			if ( endOfFile ) // if we hit the end of file previously ...
			{
				if ( plugin.outMod->IsPlaying() ) // see if we're still going
					SleepEx( 10, TRUE ); // sleep for a bit
				else // yay done playing
				{
					PostMessage( plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0 ); // tell winamp we're stopped
					// don't shut down completely yet (mpegeof will trigger a call to stop)
					ResetEvent( running ); // but we can at least sit in waitformultipleobjects ...
				}
			}
			else if ( plugin.outMod->CanWrite() > ( 576 * frameSize ) )
			{
				switch ( bits )
				{
					case 16:
						framesRead = sf_readf_short( sndFile, (short *)audioBuffer, 576 );
						break;
					case 32:
						framesRead = sf_readf_int( sndFile, (int *)audioBuffer, 576 );
						break;
					default:
						framesRead = sf_read_raw( sndFile, (int *)audioBuffer, 576 * frameSize ) / frameSize;
						break;
				}


				if ( framesRead == 0 )
				{
					endOfFile = 1;

					plugin.outMod->Write( NULL, 0 );
				}
				else
				{
					framesRead = plugin.dsp_dosamples( (short *)audioBuffer, (int)framesRead, bits, info.channels, info.samplerate );
					if ( framesRead >= 576 )
					{
						int timestamp = plugin.outMod->GetWrittenTime();
						
						plugin.SAAddPCMData( (char *)audioBuffer, info.channels, bits, timestamp );
						plugin.VSAAddPCMData( (char *)audioBuffer, info.channels, bits, timestamp );
					}

					plugin.outMod->Write( audioBuffer, (int)framesRead * frameSize );
				}
			}
			else
			{
				SleepEx( 10, TRUE );
			}
		}
	}
}
