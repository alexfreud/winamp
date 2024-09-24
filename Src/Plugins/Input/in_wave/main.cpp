//#define PLUGIN_NAME "Nullsoft Waveform Decoder"
#define PLUGIN_VERSION L"3.27"

#include "../Winamp/in2.h"
#include "../Winamp/wa_ipc.h"
#include "main.h"
#include "AudioThread.h"
#include "resource.h"
#include "config.h"
#include "api__in_wave.h"
#include <shlwapi.h>
#include "../Agave/Language/api_language.h"
#include <api/service/waservicefactory.h>
#include "../nu/ns_wc.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoCharFn.h"
#include "VirtualIO.h"
#include <strsafe.h>
#include "../nu/Singleton.h"
#include "RawReader.h"

api_config *AGAVE_API_CONFIG = NULL;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = NULL;

HINSTANCE WASABI_API_LNG_HINST  = 0;
HINSTANCE WASABI_API_ORIG_HINST = 0;

static RawMediaReaderService raw_media_reader_service;
static SingletonServiceFactory<svc_raw_media_reader, RawMediaReaderService> raw_factory;

template <class api_T>
void ServiceBuild( api_T *&api_t, GUID factoryGUID_t )
{
	if ( WASABI_API_SVC )
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid( factoryGUID_t );
		if ( factory )
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
void ServiceRelease( api_T *api_t, GUID factoryGUID_t )
{
	if ( WASABI_API_SVC && api_t )
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid( factoryGUID_t );
		if ( factory )
			factory->releaseInterface( api_t );
	}

	api_t = NULL;
}

volatile int currentSongLength = 0;
SNDFILE *sndFile = NULL;

wchar_t curFile[MAX_PATH*4] = L"";

char *INI_FILE;

class SoundFile
{
public:
	SoundFile( const wchar_t *filename, int mode, SF_INFO *info )
	{
		info->format = 0;
		//reader = CreateUnicodeReader(filename);
		//if (reader)
			//sndFile = sf_open_virtual(&unicode_io, SFM_READ, info, reader);
		sndFile = sf_wchar_open( filename, SFM_READ, info );
	}
	~SoundFile()
	{
		if ( sndFile )
			sf_close( sndFile );
		//if (reader)
			//DestroyUnicodeReader(reader);
		sndFile = NULL;
	}

	operator SNDFILE *()                                              { return sndFile; }
	SNDFILE *operator ->()                                            { return sndFile; }
	operator bool()                                                   { return !!sndFile; }
	bool operator !()                                                 { return !sndFile; }

	SNDFILE *sndFile = NULL;
	//void *reader;
};

void Config( HWND hwnd )
{
	WASABI_API_DIALOGBOXW( IDD_CONFIG, hwnd, PreferencesDialogProc );
}

int DoAboutMessageBox( HWND parent, wchar_t *title, wchar_t *message )
{
	MSGBOXPARAMSW msgbx = { sizeof( MSGBOXPARAMSW ),0 };
	msgbx.lpszText    = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon    = MAKEINTRESOURCEW( 102 );
	msgbx.hInstance   = GetModuleHandle( 0 );
	msgbx.dwStyle     = MB_USERICON;
	msgbx.hwndOwner   = parent;

	return MessageBoxIndirectW( &msgbx );
}

void About( HWND hwndParent )
{
	wchar_t message[ 1024 ] = { 0 }, text[ 1024 ] = { 0 };
	char ver[ 128 ] = { 0 };

	sf_command( 0, SFC_GET_LIB_VERSION, ver, 128 );

	WASABI_API_LNGSTRINGW_BUF( IDS_NULLSOFT_WAVEFORM_DECODER_OLD, text, 1024 );

	StringCchPrintfW( message, 1024, WASABI_API_LNGSTRINGW( IDS_ABOUT_TEXT ), plugin.description, __DATE__, ver );

	DoAboutMessageBox( hwndParent, text, message );
}

int Init()
{
	if ( !IsWindow( plugin.hMainWindow ) )
		return IN_INIT_FAILURE;

	ServiceBuild( AGAVE_API_CONFIG, AgaveConfigGUID );

	// loader so that we can get the localisation service api for use
	ServiceBuild( WASABI_API_LNG, languageApiGUID );
	raw_factory.Register( WASABI_API_SVC, &raw_media_reader_service );

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG( plugin.hDllInstance, InWavLangGUID );

	static wchar_t szDescription[ 256 ];
	StringCchPrintfW( szDescription, 256, WASABI_API_LNGSTRINGW( IDS_NULLSOFT_WAVEFORM_DECODER ), PLUGIN_VERSION );
	plugin.description = (char *)szDescription;

	INI_FILE = (char *)SendMessage( plugin.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE );
	BuildDefaultExtensions();

	GetPrivateProfileStringA( "in_wave", "extensions", defaultExtensions, config_extensions, 1024, INI_FILE );
	SetFileExtensions( config_extensions );

	return IN_INIT_SUCCESS;
}

void Quit()
{
	if ( lstrcmpiA( config_extensions, defaultExtensions ) )
		WritePrivateProfileStringA( "in_wave", "extensions", config_extensions, INI_FILE );
	else
		WritePrivateProfileStringA( "in_wave", "extensions", 0, INI_FILE );

	ServiceRelease( AGAVE_API_CONFIG, AgaveConfigGUID );
	WASABI_API_SVC->service_deregister( &raw_factory );
}

void GetFileInfo( const wchar_t *file, wchar_t *title, int *length_in_ms )
{
	SNDFILE *tempSndFile = 0;
	SF_INFO info;
	info.format = 0;
	const wchar_t *fn = ( file && file[ 0 ] ) ? file : curFile;

	tempSndFile = sf_wchar_open( fn, SFM_READ, &info );
	if ( tempSndFile )
	{
		if ( length_in_ms )
		{
			*length_in_ms = MulDiv( (int)info.frames, 1000, info.samplerate ); // TODO: is this correct?
			if ( !file || !file[ 0 ] )
				currentSongLength = *length_in_ms;
		}

		if ( title )
		{
			const char *meta = sf_get_string( tempSndFile, SF_STR_TITLE );
			if ( meta && meta[ 0 ] )
				MultiByteToWideCharSZ( CP_UTF8, 0, meta, -1, title, GETFILEINFO_TITLE_LENGTH );
			else
			{
				lstrcpynW( title, fn, GETFILEINFO_TITLE_LENGTH );
				PathStripPathW( title );
			}
		}

		sf_close( tempSndFile );
	}
	else
	{
		*length_in_ms = -1;
		if ( title )
		{
			lstrcpynW( title, fn, GETFILEINFO_TITLE_LENGTH );
			PathStripPathW( title );
		}
	}
}

int InfoBox( const wchar_t *file, HWND hwndParent )
{
	SNDFILE *metaFile = 0;
	SF_INFO info;
	info.format = 0;
	metaFile = sf_wchar_open( file, SFM_READ, &info );
	if ( metaFile )
	{
		SF_FORMAT_INFO formatInfo;
		formatInfo.format = info.format & SF_FORMAT_SUBMASK;
		sf_command( 0, SFC_GET_FORMAT_INFO, &formatInfo, sizeof( formatInfo ) );

		char temp[ 1024 ] = { 0 };
		StringCchPrintfA( temp, 1024, WASABI_API_LNGSTRING( IDS_INFO_STR_FMT ), formatInfo.name, info.channels, info.samplerate );
		MessageBoxA( NULL, temp, WASABI_API_LNGSTRING( IDS_FILE_INFORMATION ), MB_OK );
		sf_close( metaFile );
	}

	return INFOBOX_UNCHANGED;
}

int IsOurFile( const wchar_t *file )
{
	return 0;
}

int Play( const wchar_t *file )
{
	AudioThreadInit();
	lstrcpynW( curFile, file, MAX_PATH * 4 );
	QueueUserAPC( APCStart, audioThread, (ULONG_PTR)curFile );

	return 0;
}

static int paused = 0;

void Pause()
{
	paused = 1;
	QueueUserAPC( APCPause, audioThread, (ULONG_PTR)1 );
}

void UnPause()
{
	paused = 0;
	QueueUserAPC( APCPause, audioThread, (ULONG_PTR)0 );
}

int IsPaused()
{
	return paused;
}

void Stop()
{
	QueueUserAPC( APCStop, audioThread, (ULONG_PTR)0 );
	
	WaitForSingleObject( stopped, INFINITE );

	plugin.outMod->Close();
	plugin.SAVSADeInit();

	Kill();
	
	WaitForSingleObject( audioThread, INFINITE );
	
	AudioThreadQuit();
}

int GetLength()
{
	return currentSongLength;
}

int GetOutputTime()
{
	if ( plugin.outMod )
		return plugin.outMod->GetOutputTime();
	else
		return 0;
}

void SetOutputTime( int time_in_ms )
{
	QueueUserAPC( APCSeek, audioThread, (ULONG_PTR)time_in_ms );
}

int pan    = 0;
int volume = -666;

void SetVolume( int _volume )
{
	volume = _volume;
	if ( plugin.outMod )
		plugin.outMod->SetVolume( volume );
}

void SetPan( int _pan )
{
	pan = _pan;
	if ( plugin.outMod )
		plugin.outMod->SetPan( pan );
}

void EQSet( int on, char data[ 10 ], int preamp )
{}

In_Module plugin = {
	IN_VER_RET,
    "nullsoft(in_wave.dll)",
    0,
    0,
    0,
    1,
    1,
    Config,
    About,
    Init,
    Quit,
    GetFileInfo,
    InfoBox,
    IsOurFile,
    Play,
    Pause,
    UnPause,
    IsPaused,
    Stop,
    GetLength,
    GetOutputTime,
    SetOutputTime,
    SetVolume,
    SetPan,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    EQSet,
    0,
    0
};

extern "C"	__declspec( dllexport ) In_Module * winampGetInModule2()
{
	return &plugin;
}

inline bool KeywordMatch(const char *mainString, const char *keyword)
{
	return !lstrcmpiA(mainString, keyword);
}

extern "C"	__declspec( dllexport ) int winampGetExtendedFileInfoW( const wchar_t *fn, const char *data, wchar_t *dest, int destlen )
{
	if ( KeywordMatch( data, "type" ) )
	{
		StringCchCopyW( dest, destlen, L"0" );

		return 1;
	}

	if ( KeywordMatch( data, "family" ) )
	{
		LPCWSTR ext = PathFindExtensionW( fn );
		if ( L'.' != *ext )
			return 0;

		return GetExtensionName( ++ext, dest, destlen );
	}

	if ( KeywordMatch( data, "mime" ) )
	{
		LPCWSTR ext = PathFindExtensionW( fn );
		if ( ext && !_wcsicmp( ext, L".wav" ) )
		{
			StringCchCopyW( dest, destlen, L"audio/wav" );

			return 1;
		}

		return 0;
	}

	if ( !fn || ( fn && !fn[ 0 ] ) )
		return 0;

	SF_INFO info;
	SoundFile metaFile( fn, SFM_READ, &info );
	if ( !metaFile )
		return 0;

	dest[ 0 ] = 0;
	if ( KeywordMatch( data, "artist" ) )
	{
		const char *meta = sf_get_string( metaFile, SF_STR_ARTIST );
		if ( meta )
			lstrcpynW( dest, AutoWide( meta ), destlen );
	}
	else if ( KeywordMatch( data, "title" ) )
	{
		const char *meta = sf_get_string( metaFile, SF_STR_TITLE );
		if ( meta )
			lstrcpynW( dest, AutoWide( meta ), destlen );
	}
	else if ( KeywordMatch( data, "comment" ) )
	{
		const char *meta = sf_get_string( metaFile, SF_STR_COMMENT );
		if ( meta )
			lstrcpynW( dest, AutoWide( meta ), destlen );
	}
	else if ( KeywordMatch( data, "bitrate" ) )
	{
		int br = CalcBitRate( &info );
		if ( br )
			StringCchPrintfW( dest, destlen, L"%d", br );
	}
	else if ( KeywordMatch( data, "length" ) )
	{
		uint64_t length = info.frames * 1000 / info.samplerate;
		StringCchPrintfW( dest, destlen, L"%I64u", length );
	}
	else
		return 0;

	return 1;
}
