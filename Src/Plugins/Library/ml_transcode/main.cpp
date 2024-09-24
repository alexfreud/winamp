#define PLUGIN_VERSION L"2.79"

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <shlwapi.h>
#include <shlobj.h>
#include "..\..\General\gen_ml/ml.h"
#include "../winamp/wa_ipc.h"
#include "../nu/AutoWide.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoChar.h"
#include "..\..\General\gen_ml/itemlist.h"
#include "../playlist/ifc_playlistloadercallback.h"
#include "LinkedQueue.h"
#include "resource.h"
#include <vector>
#include <api/service/waServiceFactory.h>

#include "api__ml_transcode.h"

#include <strsafe.h>
#define WM_TRANSCODE_START    WM_APP+1
#define WM_TRANSCODE_ADD      WM_APP+10
#define WM_TRANSCODE_UPDATEUI WM_APP+11
#define WM_TRANSCODE_ABORT    WM_APP+12

static int init();
static void quit();
static HWND GetDialogBoxParent();
static INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

extern "C" winampMediaLibraryPlugin plugin = 
{
	MLHDR_VER,
	"nullsoft(ml_transcode.dll)",
	init,
	quit,
	PluginMessageProc,
	0,
	0,
	0,
};
typedef std::vector<const wchar_t*> PtrListWCharPtr;
LinkedQueue transcodeQueue;

int transcodeConfig(HWND parent); // returns fourcc

void transcode(const itemRecordListW *ice, HWND parent);
void transcode(PtrListWCharPtr &filenames, HWND parent);

void addTrackToTranscodeQueue(itemRecordW *track, unsigned int format, const wchar_t* dest, const wchar_t* folder);
void addTrackToTranscodeQueue(const wchar_t *track, unsigned int format, const wchar_t* dest, const wchar_t* folder);

void filenameToItemRecord(const wchar_t *file, itemRecordW *ice);
void copyTags(const itemRecordW *in, const wchar_t *out);

extern void RecursiveCreateDirectory(wchar_t *buf1);
extern wchar_t *FixReplacementVars(wchar_t *str, size_t str_size, itemRecordW *song);
void FixFileLength(wchar_t *str);

HWND transcoderWnd = NULL;

wchar_t inifile[MAX_PATH] = {0};
char inifileA[MAX_PATH] = {0};

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_metadata *AGAVE_API_METADATA=0;
api_application *WASABI_API_APP=0;
api_playlistmanager *AGAVE_API_PLAYLISTMANAGER = 0;
api_albumart *AGAVE_API_ALBUMART = 0;
api_stats *AGAVE_API_STATS = 0;

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (plugin.service)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (plugin.service && api_t)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

int init()
{
	ServiceBuild(WASABI_API_APP,            applicationApiServiceGuid);
	ServiceBuild(WASABI_API_LNG,            languageApiGUID);
	ServiceBuild(AGAVE_API_METADATA,        api_metadataGUID);
	ServiceBuild(AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID);
	ServiceBuild(AGAVE_API_ALBUMART,        albumArtGUID);
	ServiceBuild(AGAVE_API_STATS,           AnonymousStatsGUID);

	const wchar_t *iniDirectory = WASABI_API_APP->path_getUserSettingsPath();
	PathCombine(inifile, iniDirectory, L"Plugins\\ml\\ml_transcode.ini");
	lstrcpynA(inifileA, AutoChar(inifile), MAX_PATH);

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,MlTranscodeLangGUID);

	static wchar_t szDescription[256];
	StringCchPrintfW( szDescription, ARRAYSIZE( szDescription ), WASABI_API_LNGSTRINGW( IDS_NULLSOFT_FORMAT_CONVERTER ), PLUGIN_VERSION );

	plugin.description = (char*)szDescription;

	return ML_INIT_SUCCESS;
}

void quit()
{
	if(IsWindow(transcoderWnd))
		SendMessage(transcoderWnd,WM_TRANSCODE_ABORT,0,0);

	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	ServiceRelease(AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID);
	ServiceRelease(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceRelease(AGAVE_API_STATS, AnonymousStatsGUID);
}

class TranscodePlaylistLoader : public ifc_playlistloadercallback
{
public:
	TranscodePlaylistLoader(PtrListWCharPtr*_fileList);
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info);

protected:
	PtrListWCharPtr*fileList;
	RECVS_DISPATCH;
};

TranscodePlaylistLoader::TranscodePlaylistLoader(PtrListWCharPtr*_fileList)
{
	fileList = _fileList;
}

void TranscodePlaylistLoader::OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
{
	fileList->push_back(_wcsdup(filename));
}

#define CBCLASS TranscodePlaylistLoader
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
END_DISPATCH;
#undef CBCLASS


INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	if (message_type == ML_MSG_ONSENDTOBUILD)
	{
		if (param1 == ML_TYPE_ITEMRECORDLIST || param1 == ML_TYPE_FILENAMES ||
			param1 == ML_TYPE_ITEMRECORDLISTW || param1 == ML_TYPE_FILENAMESW
			|| (AGAVE_API_PLAYLISTMANAGER && (param1 == ML_TYPE_PLAYLIST || param1 == ML_TYPE_PLAYLISTS)))
		{
			mlAddToSendToStructW s;
			s.context=param2;
			s.desc=WASABI_API_LNGSTRINGW(IDS_FORMAT_CONVERTER);
			s.user32=(intptr_t)PluginMessageProc;
			SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&s,ML_IPC_ADDTOSENDTOW);
		}
	}
	else if (message_type == ML_MSG_ONSENDTOSELECT)
	{
		if (param2 && param3 == (INT_PTR)PluginMessageProc)
		{
			if(param1 == ML_TYPE_FILENAMES)
			{
				PtrListWCharPtr fileList;
				TranscodePlaylistLoader loader(&fileList);
				const char * filenames = (const char *)param2;
				while(filenames && *filenames)
				{
					// try to load as playlist first
					if (AGAVE_API_PLAYLISTMANAGER->Load(AutoWide(filenames), &loader) != PLAYLISTMANAGER_SUCCESS)
					{
						// not a playlist.. just add it directly
						fileList.push_back(AutoWideDup(filenames));
					}
					filenames+=strlen(filenames)+1;
				}
				transcode(fileList,GetDialogBoxParent());
				
				//fileList.freeAll();
				for (auto file : fileList)
				{
					free((void*)file);
				}
				fileList.clear();

				return 1;
			}
			else if(param1 == ML_TYPE_FILENAMESW)
			{
				PtrListWCharPtr fileList;
				TranscodePlaylistLoader loader(&fileList);
				const wchar_t * filenames = (const wchar_t *)param2;
				while(filenames && *filenames)
				{
					// try to load as playlist first
					if (AGAVE_API_PLAYLISTMANAGER->Load(filenames, &loader) != PLAYLISTMANAGER_SUCCESS)
					{
						// not a playlist.. just add it directly
						fileList.push_back(filenames);
					}
					filenames+=wcslen(filenames)+1;
				}
				transcode(fileList,GetDialogBoxParent());
				return 1;
			}
			else if(param1 == ML_TYPE_ITEMRECORDLIST)
			{
				const itemRecordList *ico=(const itemRecordList*)param2;
				itemRecordListW list = {0,};
				convertRecordList(&list, ico);
				transcode(&list, GetDialogBoxParent());
				freeRecordList(&list);
				return 1;
			}
			else if(param1 == ML_TYPE_ITEMRECORDLISTW)
			{
				const itemRecordListW *list =(const itemRecordListW*)param2;
				transcode(list,GetDialogBoxParent());
				return 1;
			}
			else if (param1 == ML_TYPE_PLAYLIST)
			{
				mlPlaylist *playlist = (mlPlaylist *)param2;
				PtrListWCharPtr fileList;
				fileList.reserve(playlist->numItems);
				TranscodePlaylistLoader loader(&fileList);
				AGAVE_API_PLAYLISTMANAGER->Load(playlist->filename, &loader);
				transcode(fileList,GetDialogBoxParent());

				//fileList.freeAll();
				for (auto file : fileList)
				{
					free((void*)file);
				}
				fileList.clear();

				return 1;
			}
			else if (param1 == ML_TYPE_PLAYLISTS)
			{
				mlPlaylist **playlists = (mlPlaylist **)param2;
				PtrListWCharPtr  fileList;
				while (playlists && *playlists)
				{
					mlPlaylist *playlist = *playlists;
					fileList.reserve(fileList.size() + playlist->numItems);
					TranscodePlaylistLoader loader(&fileList);
					AGAVE_API_PLAYLISTMANAGER->Load(playlist->filename, &loader);
					playlists++;
				}
				transcode(fileList,GetDialogBoxParent());

				//fileList.freeAll();
				for (auto file : fileList)
				{
					free((void*)file);
				}
				fileList.clear();

				return 1;
			}
		}
	}
	else if (message_type == ML_MSG_CONFIG)
	{
		transcodeConfig((HWND)param1);
		return 1;
	}
	return 0;
}

class TranscodeItem
{
	public:
	itemRecordW ice;
	unsigned int fourcc;
	wchar_t *outfile;
	TranscodeItem( unsigned int fourcc, const wchar_t *folder, const wchar_t *outfile, const itemRecordW *i ) : fourcc( fourcc )
	{
		ZeroMemory( &ice, sizeof( ice ) );
		copyRecord( &ice, const_cast<itemRecordW *>( i ) ); // TODO: remove for 5.53. this just works around some 5.52 build weirdness
		makefn( folder, outfile );
	}
	TranscodeItem( unsigned int fourcc, const wchar_t *folder, const wchar_t *outfile, const wchar_t *i ) : fourcc( fourcc )
	{
		ZeroMemory( &ice, sizeof( ice ) );
		filenameToItemRecord( i, &ice );
		makefn( folder, outfile );
	}
	void makefn( const wchar_t *folder, const wchar_t *outfile )
	{
		if ( GetPrivateProfileInt( L"transcoder", L"usefilename", 0, inifile ) )
		{
			size_t len = wcslen( ice.filename ) + 10;
			this->outfile = (wchar_t *)calloc( len, sizeof( this->outfile[ 0 ] ) );
			StringCchCopyW( this->outfile, len, ice.filename );
			const wchar_t *extn = wcsrchr( outfile, L'.' );
			wchar_t *exto = wcsrchr( this->outfile, L'.' );
			if ( extn && exto && wcslen( extn ) < 10 )
				StringCchCopy( exto, len, extn );
		}
		else
		{
			wchar_t filename[ 2048 ] = { 0 };
			StringCchCopy( filename, 2048, outfile );
			FixReplacementVars( filename, 2048, &ice );
			FixFileLength( filename );
			PathCombine( filename, folder, filename );
			this->outfile = _wcsdup( filename );
		}
	}
	~TranscodeItem()
	{
		free( outfile ); freeRecord( &ice );
	}
};

void getViewport( RECT *r, HWND wnd, int full, RECT *sr )
{
	POINT *p = NULL;
	if ( p || sr || wnd )
	{
		HMONITOR hm = NULL;

		if ( sr )
			hm = MonitorFromRect( sr, MONITOR_DEFAULTTONEAREST );
		else if ( wnd )
			hm = MonitorFromWindow( wnd, MONITOR_DEFAULTTONEAREST );
		else if ( p )
			hm = MonitorFromPoint( *p, MONITOR_DEFAULTTONEAREST );

		if ( hm )
		{
			MONITORINFOEX mi;
			memset( &mi, 0, sizeof( mi ) );
			mi.cbSize = sizeof( mi );

			if ( GetMonitorInfoW( hm, &mi ) )
			{
				if ( !full )
					*r = mi.rcWork;
				else
					*r = mi.rcMonitor;

				return;
			}
		}
	}

	if ( full )
	{ // this might be borked =)
		r->top    = r->left = 0;
		r->right  = GetSystemMetrics( SM_CXSCREEN );
		r->bottom = GetSystemMetrics( SM_CYSCREEN );
	}
	else
	{
		SystemParametersInfoW( SPI_GETWORKAREA, 0, r, 0 );
	}
}

BOOL windowOffScreen( HWND hwnd, POINT pt )
{
	RECT r = { 0 }, wnd = { 0 }, sr = { 0 };

	GetWindowRect( hwnd, &wnd );
	sr.left   = pt.x;
	sr.top    = pt.y;
	sr.right  = sr.left + ( wnd.right - wnd.left );
	sr.bottom = sr.top + ( wnd.bottom - wnd.top );

	getViewport( &r, hwnd, 0, &sr );

	return !PtInRect( &r, pt );
}

bool transcoding = false;
bool transcoderIdle = false;
int totalItems = 0;
int itemsDone = 0;
int itemsFailed = 0;

static BOOL CALLBACK transcode_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static convertFileStructW * cfs;
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			transcoderWnd = hwndDlg;
			cfs = (convertFileStructW *)calloc(1, sizeof(convertFileStructW));
			SendDlgItemMessage(hwndDlg,IDC_TRACKPROGRESS,PBM_SETRANGE32,0,100);
			SendDlgItemMessage(hwndDlg,IDC_TOTALPROGRESS,PBM_SETRANGE32,0,totalItems*100);
			SendDlgItemMessage(hwndDlg,IDC_TRACKPROGRESS,PBM_SETPOS,0,0);
			SendDlgItemMessage(hwndDlg,IDC_TOTALPROGRESS,PBM_SETPOS,0,0);
			PostMessage(hwndDlg,WM_TRANSCODE_START,0,0);

			// show edit info window and restore last position as applicable
			POINT pt = {GetPrivateProfileInt(L"transcoder", L"convert_x", -1, inifile),
						GetPrivateProfileInt(L"transcoder", L"convert_y", -1, inifile)};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
			else
				ShowWindow(hwndDlg, SW_SHOWNA);
		}
			break;
		case WM_TRANSCODE_START:
			for(;;)
			{
				TranscodeItem * t = (TranscodeItem *)transcodeQueue.Peek();
				if(!t)
				{
					SetDlgItemText(hwndDlg,IDC_ABORT,WASABI_API_LNGSTRINGW(IDS_DONE));
					SendMessage(transcoderWnd,WM_TRANSCODE_UPDATEUI,0,0);
					transcoderIdle=true;
					return 0;
				}
				transcoderIdle=false;
				RecursiveCreateDirectory(t->outfile);
				ZeroMemory(cfs,sizeof(*cfs));
				cfs->callbackhwnd = hwndDlg;
				cfs->sourcefile = t->ice.filename;
				cfs->destfile = t->outfile;
				cfs->destformat[0] = t->fourcc;
				//cfs->destformat[1] = 44100;
				//cfs->destformat[2] = 16;
				//cfs->destformat[3] = 2;
				cfs->destformat[6] = mmioFOURCC('I','N','I',' ');
				cfs->destformat[7] = (intptr_t)inifileA;
				cfs->error = L"";

				SendDlgItemMessage(hwndDlg,IDC_TRACKPROGRESS,PBM_SETPOS,0,0);
				wchar_t buf[1024] = {0};
				StringCchPrintf(buf, 1024, L"%s - %s", t->ice.artist?t->ice.artist:L"", t->ice.title?t->ice.title:L"");
				SetDlgItemText(hwndDlg,IDC_CURRENTTRACK,buf);

				if(SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)cfs,IPC_CONVERTFILEW)) break;
				else
				{
					if (cfs->error && *cfs->error)
					{
						MessageBox(hwndDlg, cfs->error, WASABI_API_LNGSTRINGW(IDS_TRANSCODING_FAILED), MB_ICONWARNING | MB_OK);
					}
					delete (TranscodeItem*)transcodeQueue.Poll();
					itemsDone++; itemsFailed++;
				}
			}
		case WM_TRANSCODE_ADD: // update totals, stuff added to queue
			SetWindowPos(hwndDlg,HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
			if(transcoderIdle)
			{
				SetDlgItemText(hwndDlg,IDC_ABORT,WASABI_API_LNGSTRINGW(IDS_ABORT));
				SendMessage(hwndDlg,WM_TRANSCODE_START,0,0);
				return 0;
			} // else update UI
		case WM_TRANSCODE_UPDATEUI:
		{
			SendDlgItemMessage(hwndDlg,IDC_TOTALPROGRESS,PBM_SETRANGE32,0,totalItems*100);
			SendDlgItemMessage(hwndDlg,IDC_TOTALPROGRESS,PBM_SETPOS,itemsDone*100,0);
			wchar_t buf[100] = {0};
			StringCchPrintfW(buf, 100, WASABI_API_LNGSTRINGW(IDS_TRACKS_DONE_REMAINING_FAILED),
							 itemsDone-itemsFailed, totalItems-itemsDone, itemsFailed);
			SetDlgItemText(hwndDlg,IDC_TOTALCAPTION,buf);
		}
			break;
		case WM_WA_IPC:
			switch(lParam)
			{
				case IPC_CB_CONVERT_STATUS:
					if(wParam >= 0 && wParam <= 100)
					{
						SendDlgItemMessage(hwndDlg,IDC_TOTALPROGRESS,PBM_SETPOS,(int)wParam + itemsDone*100,0);
						SendDlgItemMessage(hwndDlg,IDC_TRACKPROGRESS,PBM_SETPOS,(int)wParam,0);
					}
					break;
				case IPC_CB_CONVERT_DONE:
				{
					SendDlgItemMessage(hwndDlg,IDC_TRACKPROGRESS,PBM_SETPOS,100,0);
					TranscodeItem * t = (TranscodeItem*)transcodeQueue.Poll();
					itemsDone++;
					cfs->callbackhwnd=NULL;
					SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)cfs,IPC_CONVERTFILEW_END);
					copyTags(&t->ice,t->outfile);
					if (AGAVE_API_STATS)
					{
						AGAVE_API_STATS->IncrementStat(api_stats::TRANSCODE_COUNT);
						AGAVE_API_STATS->SetStat(api_stats::TRANSCODE_FORMAT, t->fourcc);
					}
					delete t;
					PostMessage(hwndDlg,WM_TRANSCODE_START,0,0);
				}
					break;
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_ABORT:
					transcode_dlgproc(hwndDlg,WM_TRANSCODE_ABORT,0,0);
					break;
			}
			break;
		case WM_CLOSE:
		case WM_TRANSCODE_ABORT:
		{
			transcoding = false;
			cfs->callbackhwnd = NULL;
			if(!transcoderIdle) SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)cfs,IPC_CONVERTFILEW_END);
			TranscodeItem * t;
			while((t = (TranscodeItem*)transcodeQueue.Poll()) != NULL)
			{
				// prompt to delete incomplete conversions
				if(PathFileExists(t->outfile))
				{
					wchar_t title[64] = {0}, prompt[512] = {0}, file[MAX_PATH] = {0};
					lstrcpyn(file, t->outfile, MAX_PATH);
					PathStripPath(file);
					StringCchPrintf(prompt, 512, WASABI_API_LNGSTRINGW(IDS_REMOVE_PARTIAL_FILE), file);
					if(MessageBox(hwndDlg, prompt,
								   WASABI_API_LNGSTRINGW_BUF(IDS_TRANSCODING_ABORTED, title, 64), MB_YESNO) == IDYES)
					{
						DeleteFile(t->outfile);
					}
				}
				delete t;
			}

			RECT rect = {0};
			GetWindowRect(hwndDlg, &rect);
			wchar_t buf[16] = {0};
			StringCchPrintf(buf, 16, L"%d", rect.left);
			WritePrivateProfileString(L"transcoder", L"convert_x", buf, inifile);
			StringCchPrintf(buf, 16, L"%d", rect.top);
			WritePrivateProfileString(L"transcoder", L"convert_y", buf, inifile);

			EndDialog(hwndDlg,0);
			itemsDone = 0;
			totalItems = 0;
			itemsFailed = 0;
			transcoderWnd = NULL;
			transcoding = false;
			transcoderIdle = false;
			free(cfs);
		}
			break;
	}
	return 0;
}

void startTranscoding()
{
	if(transcoding)
	{
		totalItems++;
		SendMessage(transcoderWnd,WM_TRANSCODE_ADD,0,0);
	}
	else
	{
		transcoding = true;
		totalItems=1;
		WASABI_API_CREATEDIALOGW(IDD_TRANSCODE, NULL, transcode_dlgproc);
	}
}

void addTrackToTranscodeQueue(const wchar_t * track, unsigned int format, const wchar_t* filepart, const wchar_t* folder)
{
	transcodeQueue.Offer(new TranscodeItem(format, folder, filepart, track));
	startTranscoding();
}

void addTrackToTranscodeQueue(itemRecordW * track, unsigned int format, const wchar_t* filepart, const wchar_t* folder)
{
	transcodeQueue.Offer(new TranscodeItem(format, folder, filepart, track));
	startTranscoding();
}

static void fourccToString(unsigned int f, wchar_t * str, int str_len)
{
	char s[4] = {(char)(f&0xFF),(char)((f>>8)&0xFF),(char)((f>>16)&0xFF),0};
	StringCchCopy(str, str_len, AutoWide(s));
	CharLower(str);
}

wchar_t* GetDefaultSaveToFolder(wchar_t* path_to_store)
{
	if(FAILED(SHGetFolderPath(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, path_to_store)))
	{
		if(FAILED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path_to_store)))
		{
			// and if that all fails then do a reasonable default
			lstrcpyn(path_to_store, L"C:\\My Music", MAX_PATH);
		}
		// if there's no valid My Music folder (typically win2k) then default to %my_documents%\my music
		else
		{
			PathCombine(path_to_store, path_to_store, L"My Music");
		}
	}
	return path_to_store;
}

DWORD GetPrivateProfileStringUTF8(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPTSTR lpReturnedString,  DWORD nSize,  LPCSTR lpFileName)
{
	char utf8_text[2048] = {0};
	GetPrivateProfileStringA(lpAppName,lpKeyName,lpDefault,utf8_text, 2048, lpFileName);

	return MultiByteToWideCharSZ(CP_UTF8, 0, utf8_text, -1, lpReturnedString, nSize);
}

BOOL WritePrivateProfileStringUTF8(LPCSTR lpAppName,  LPCSTR lpKeyName,  LPCTSTR lpString,  LPCSTR lpFileName)
{
	return WritePrivateProfileStringA(lpAppName, lpKeyName, AutoChar(lpString, CP_UTF8), lpFileName);
}

class EncodableFormat
{
public:
	unsigned int fourcc;
	wchar_t *desc;
	EncodableFormat(unsigned int fourcc,wchar_t *desc) :
    fourcc(fourcc) {this->desc = _wcsdup(desc);}
	~EncodableFormat() {free(desc);}
};

static void enumProc(intptr_t user_data, const char *desc, int fourcc) {
	((C_ItemList*)user_data)->Add(new EncodableFormat((unsigned int)fourcc,AutoWide(desc)));
}

static void BuildEncodableFormatsList(C_ItemList * list, HWND winampWindow,wchar_t * inifile) {
	converterEnumFmtStruct e = {enumProc,(intptr_t)list};
	SendMessage(winampWindow,WM_WA_IPC,(WPARAM)&e,IPC_CONVERT_CONFIG_ENUMFMTS);
}

unsigned int transcodeGatherSettings(wchar_t *format, wchar_t *folder, int format_len, HWND parent)
{
	if(GetPrivateProfileInt(L"transcoder",L"showconf",1,inifile)) 
	if(!transcodeConfig(parent)) 
		return 0;
	
	wchar_t tmp[MAX_PATH] = {0};
	GetPrivateProfileStringUTF8("transcoder", "fileformat","<Artist> - <Album>\\## - <Title>",format,1024,inifileA);
	GetPrivateProfileStringUTF8("transcoder", "fileroot", AutoChar(GetDefaultSaveToFolder(tmp), CP_UTF8), folder, MAX_PATH, inifileA);
 
	unsigned int fourcc = GetPrivateProfileInt(L"transcoder",L"format",mmioFOURCC('A','A','C','f'),inifile);

	char extA[8]=".";
	convertConfigItem c = {fourcc,"extension",&extA[1],7,inifileA};
	if(!SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&c,IPC_CONVERT_CONFIG_GET_ITEM))
	{
		// if there was an error, see if it's from an invalid fourcc and try to fallback
		C_ItemList * formats = new C_ItemList;
		BuildEncodableFormatsList(formats,plugin.hwndWinampParent,inifile);
		bool doFail = false;

		for(int i=0; i < formats->GetSize(); i++) {
			EncodableFormat * f = (EncodableFormat *)formats->Get(i);
			// if it exists then abort and fail as prior behaviour
			if(f->fourcc == fourcc)
			{
				doFail = true;
				break;
			}
		}
		if(!doFail)
		{
			fourcc = mmioFOURCC('A','A','C','f');
			c.format = fourcc;
			SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&c,IPC_CONVERT_CONFIG_GET_ITEM);
		}
	}
	if(extA[1]) StringCchCat(format, format_len, AutoWide(extA));
	else
	{
		wchar_t ext[8]=L".";
		fourccToString(fourcc,&ext[1], 8);
		StringCchCat(format, format_len, ext);
	}
	return fourcc;
}

void transcode( PtrListWCharPtr &filenames, HWND parent )
{
	wchar_t format[ 2048 ] = { 0 }, folder[ MAX_PATH ] = { 0 };
	unsigned int fourcc = transcodeGatherSettings( format, folder, 2048, parent );

	if ( !fourcc )
		return;

	for ( const wchar_t *l_filename : filenames )
		addTrackToTranscodeQueue( l_filename, fourcc, format, folder );
}

void transcode(const itemRecordListW *ice, HWND parent)
{
	wchar_t format[2048] = {0}, folder[MAX_PATH] = {0};
	unsigned int fourcc = transcodeGatherSettings(format, folder, 2048, parent);
	if(!fourcc) return;

	for(int i=0; i < ice->Size; i++) {
		addTrackToTranscodeQueue(&ice->Items[i],fourcc,format,folder);
	}
}

static void FreeEncodableFormatsList(C_ItemList * list) {
	int l = list->GetSize();
	while(--l >= 0) {
		delete ((EncodableFormat*)list->Get(l));
		list->Del(l);
	}
}

static void doConfigResizeChild(HWND parent, HWND child) {
	if (child) {
		RECT r;
		GetWindowRect(GetDlgItem(parent, IDC_ENC_CONFIG), &r);
		ScreenToClient(parent, (LPPOINT)&r);
		SetWindowPos(child, 0, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		ShowWindow(child, SW_SHOW);
	}
}

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam)
{
	wchar_t cl[32] = {0};
	GetClassNameW(hwnd, cl, ARRAYSIZE(cl));
	if (!lstrcmpiW(cl, WC_TREEVIEW))
	{
		PostMessage(hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection(hwnd));
		return FALSE;
	}

	return TRUE;
}

int CALLBACK WINAPI BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED)
	{
		wchar_t buf[4096]=L"";
		GetDlgItemText((HWND)lpData,IDC_ROOTDIR,buf,sizeof(buf)/sizeof(wchar_t));
		SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)buf);
		SetWindowText(hwnd,WASABI_API_LNGSTRINGW(IDS_SELECT_WHERE_TO_SAVE));

		// this is not nice but it fixes the selection not working correctly on all OSes
		EnumChildWindows(hwnd, browseEnumProc, 0);
	}
	return 0;
}

static BOOL CALLBACK config_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static C_ItemList * formats;
	static convertConfigStruct * ccs;
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			bool usefn = !!GetPrivateProfileInt(L"transcoder", L"usefilename",0,inifile);
			if(GetPrivateProfileInt(L"transcoder", L"showconf", 1, inifile)) CheckDlgButton(hwndDlg,IDC_SHOWEVERY,BST_CHECKED);
			if(usefn) CheckDlgButton(hwndDlg,IDC_USE_FILENAME,BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ROOTDIR),!usefn);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE),!usefn);
			EnableWindow(GetDlgItem(hwndDlg, IDC_NAMING),!usefn);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FORMATHELP),!usefn);

			wchar_t buf[4096]=L"", tmp[MAX_PATH]=L"";
			GetPrivateProfileStringUTF8("transcoder", "fileformat","<Artist> - <Album>\\## - <Title>",buf,4096,inifileA);
			SetDlgItemText(hwndDlg,IDC_NAMING,buf);

			GetPrivateProfileStringUTF8("transcoder", "fileroot", AutoChar(GetDefaultSaveToFolder(tmp), CP_UTF8), buf, 4096, inifileA);
			SetDlgItemText(hwndDlg,IDC_ROOTDIR,buf);
			formats = new C_ItemList;
			BuildEncodableFormatsList(formats,plugin.hwndWinampParent,inifile);
			    
			ccs = (convertConfigStruct *)calloc(sizeof(convertConfigStruct),1);
			ccs->extra_data[6] = mmioFOURCC('I','N','I',' ');
			ccs->extra_data[7] = (int)inifileA;
			ccs->hwndParent = hwndDlg;
			ccs->format = GetPrivateProfileInt(L"transcoder",L"format",mmioFOURCC('A','A','C','f'),inifile);

			for(int i=0; i < formats->GetSize(); i++) {
				EncodableFormat * f = (EncodableFormat *)formats->Get(i);
				int a = SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_ADDSTRING, 0, (LPARAM)f->desc);
				SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_SETITEMDATA, (WPARAM)a, (LPARAM)f);

				if(f->fourcc == ccs->format)
					SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_SETCURSEL, (WPARAM)a, 0);
			}

			// if there is no selection then force things to the correct default
			if(SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETCURSEL, (WPARAM)0, 0) == CB_ERR) {
				for(int i=0; i < SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETCOUNT, 0, 0); i++) {
					EncodableFormat * f = (EncodableFormat *)SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETITEMDATA, (WPARAM)i, 0);
					if(f->fourcc  == mmioFOURCC('A','A','C','f')) {
						ccs->format = f->fourcc;
						SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_SETCURSEL, (WPARAM)i, 0);
						break;
					}
				}
			}

			HWND h = (HWND)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)ccs, IPC_CONVERT_CONFIG);
			doConfigResizeChild(hwndDlg, h);

			// show config window and restore last position as applicable
			POINT pt = {GetPrivateProfileInt(L"transcoder", L"showconf_x", -1, inifile),
						GetPrivateProfileInt(L"transcoder", L"showconf_y", -1, inifile)};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
		}
			break;

		case WM_DESTROY:
		{
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)ccs, IPC_CONVERT_CONFIG_END);
			free(ccs); ccs=0;
			FreeEncodableFormatsList(formats);
			delete formats; formats=0;
		}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_USE_FILENAME:
				{
					bool usefn = IsDlgButtonChecked(hwndDlg,IDC_USE_FILENAME)!=0;
					EnableWindow(GetDlgItem(hwndDlg, IDC_ROOTDIR),!usefn);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE),!usefn);
					EnableWindow(GetDlgItem(hwndDlg, IDC_NAMING),!usefn);
					EnableWindow(GetDlgItem(hwndDlg, IDC_FORMATHELP),!usefn);
				}
					break;

				case IDC_ENCFORMAT:
					if (HIWORD(wParam) != CBN_SELCHANGE) return 0;
					{
						int sel = SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETCURSEL, 0, 0);
						if (sel != CB_ERR)
						{
							SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)ccs, IPC_CONVERT_CONFIG_END);
							EncodableFormat * f = (EncodableFormat *)SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETITEMDATA, sel, 0);
							ccs->format = f->fourcc;
							HWND h = (HWND)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)ccs, IPC_CONVERT_CONFIG);
							doConfigResizeChild(hwndDlg, h);
						}
					}
					break;

				case IDC_FORMATHELP:
				{
					wchar_t titleStr[64] = {0};
					MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_FILENAME_FORMAT_HELP),
							   WASABI_API_LNGSTRINGW_BUF(IDS_FILENAME_FORMAT_HELP_TITLE,titleStr,64), MB_OK);
					break;
				}

				case IDC_BROWSE:
				{
					BROWSEINFO bi = {0};
					LPMALLOC lpm = 0;
					wchar_t bffFileName[MAX_PATH] = {0};
					bi.hwndOwner = hwndDlg;
					bi.pszDisplayName = bffFileName;
					bi.lpszTitle = WASABI_API_LNGSTRINGW(IDS_CHOOSE_FOLDER);
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
					bi.lpfn = BrowseCallbackProc;
					bi.lParam = (LPARAM)hwndDlg;
					LPITEMIDLIST iil = SHBrowseForFolder(&bi);
					if(iil)
					{
						SHGetPathFromIDList(iil,bffFileName);
						SHGetMalloc(&lpm);
						lpm->Free(iil);
						SetDlgItemText(hwndDlg, IDC_ROOTDIR, bffFileName);
					}
				}
					break;

				case IDOK:
				case IDCANCEL:
				{
					DWORD fourcc=0;
					if (LOWORD(wParam) == IDOK)
					{
						wchar_t buf[4096]=L"";
						GetDlgItemText(hwndDlg,IDC_NAMING,buf,sizeof(buf)/sizeof(wchar_t));
						WritePrivateProfileStringUTF8("transcoder","fileformat",buf,inifileA);
						GetDlgItemText(hwndDlg,IDC_ROOTDIR,buf,sizeof(buf)/sizeof(wchar_t));
						WritePrivateProfileStringUTF8("transcoder","fileroot",buf,inifileA);
						WritePrivateProfileString(L"transcoder",L"showconf",IsDlgButtonChecked(hwndDlg,IDC_SHOWEVERY)?L"1":L"0",inifile);
						WritePrivateProfileString(L"transcoder",L"usefilename",IsDlgButtonChecked(hwndDlg,IDC_USE_FILENAME)?L"1":L"0",inifile);

						LRESULT esel = SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETCURSEL, 0, 0);
						if (esel!=CB_ERR)
						{
							LRESULT data = SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETITEMDATA, esel, 0);
							if (data != CB_ERR)
							{
								EncodableFormat * f = (EncodableFormat *)data;
								StringCchPrintf(buf, 4096, L"%d", f->fourcc);
								WritePrivateProfileString(L"transcoder",L"format",buf,inifile);
								fourcc=f->fourcc;
							}
						}
					}

					EndDialog(hwndDlg, (LOWORD(wParam) ? fourcc : 0));

					RECT rect = {0};
					GetWindowRect(hwndDlg, &rect);
					wchar_t buf[16] = {0};
					StringCchPrintf(buf, 16, L"%d", rect.left);
					WritePrivateProfileString(L"transcoder", L"showconf_x", buf, inifile);
					StringCchPrintf(buf, 16, L"%d", rect.top);
					WritePrivateProfileString(L"transcoder", L"showconf_y", buf, inifile);
				}
					break;
			}
			break;
	}
	return 0;
}

int transcodeConfig(HWND parent) { // returns fourcc
	return WASABI_API_DIALOGBOXW(IDD_TRANSCODE_CONFIG, parent, config_dlgproc);
}
// metadata shit

extern wchar_t *guessTitles(const wchar_t *filename, int *tracknum,wchar_t **artist, wchar_t **album,wchar_t **title);

#define atoi_NULLOK(s) ((s)?_wtoi(s):0)

void filenameToItemRecord(const wchar_t *file, itemRecordW * ice)
{
	int gtrack=0;
	wchar_t *gartist=NULL,*galbum=NULL,*gtitle=NULL;
	wchar_t *guessbuf = guessTitles(file,&gtrack,&gartist,&galbum,&gtitle);
	if(!gartist) gartist=L"";
	if(!galbum) galbum=L"";
	if(!gtitle) gtitle=L"";

	wchar_t buf[512] = {0};
	AGAVE_API_METADATA->GetExtendedFileInfo(file, L"title", buf, 512);
	if(buf[0]) { ice->title=_wcsdup(buf); gartist=L""; galbum=L""; gtrack=-1;}
	else ice->title=_wcsdup(gtitle);

	buf[0]=0;
	AGAVE_API_METADATA->GetExtendedFileInfo(file, L"album", buf, 512);
	if(buf[0]) ice->album=_wcsdup(buf);
	else ice->album=_wcsdup(galbum);

	buf[0]=0;
	AGAVE_API_METADATA->GetExtendedFileInfo(file, L"artist", buf, 512);
	if(buf[0]) ice->artist=_wcsdup(buf);
	else ice->artist=_wcsdup(gartist);
		
	buf[0]=0;
	AGAVE_API_METADATA->GetExtendedFileInfo(file, L"albumartist", buf, 512);
	if(buf[0]) ice->albumartist=_wcsdup(buf);
	else ice->albumartist=_wcsdup(ice->artist);
	  
	buf[0]=0;
	AGAVE_API_METADATA->GetExtendedFileInfo(file, L"track", buf, 512);
	if(buf[0]) ice->track=atoi_NULLOK(buf);
	else ice->track=gtrack;
	  
	buf[0]=0;
	AGAVE_API_METADATA->GetExtendedFileInfo(file, L"genre", buf, 512);
	ice->genre=_wcsdup(buf);
	  
	buf[0]=0;
	AGAVE_API_METADATA->GetExtendedFileInfo(file, L"comment", buf, 512);
	ice->comment=_wcsdup(buf);

	buf[0]=0;
	AGAVE_API_METADATA->GetExtendedFileInfo(file, L"year", buf, 512);
	ice->year=atoi_NULLOK(buf);
	  
	basicFileInfoStructW b={0};
	b.filename=const_cast<wchar_t *>(file); //benski> changed extendedFileInfoStruct but not basicFileInfoStruct, i'll have to do that later so we can get rid of this cast
	b.quickCheck=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&b,IPC_GET_BASIC_FILE_INFOW);
	ice->length=b.length;
	  
	ice->filename = _wcsdup(file);
	free(guessbuf);
}

void copyTags(const itemRecordW *in, const wchar_t *out) 
{
	// check if the old file still exists - if it does, we will let Winamp copy metadata for us
	if (wcscmp(in->filename, out) && PathFileExists(in->filename))
	{
		copyFileInfoStructW copy;
		copy.dest = out;
		copy.source = in->filename;
		if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&copy, IPC_COPY_EXTENDED_FILE_INFOW) == 0) // 0 indicates success here
		{
			AGAVE_API_ALBUMART->CopyAlbumArt(in->filename, out);
			return;
		}
	}

	wchar_t buf[32] = {0}, file[MAX_PATH] = {0};
	StringCchCopy(file, MAX_PATH, out);
	extendedFileInfoStructW e = {0};
	e.filename=file;

	e.metadata=L"album";
	e.ret=in->album;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&e,IPC_SET_EXTENDED_FILE_INFOW);

	e.metadata=L"artist";
	e.ret=in->artist;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&e,IPC_SET_EXTENDED_FILE_INFOW);

	e.metadata=L"albumartist";
	e.ret=in->albumartist;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&e,IPC_SET_EXTENDED_FILE_INFOW);

	e.metadata=L"title";
	e.ret=in->title;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&e,IPC_SET_EXTENDED_FILE_INFOW);

	e.metadata=L"track";
	StringCchPrintf(buf, 32, L"%d", in->track);
	e.ret=buf;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&e,IPC_SET_EXTENDED_FILE_INFOW);

	e.metadata=L"genre";
	e.ret=in->genre;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&e,IPC_SET_EXTENDED_FILE_INFOW);

	e.metadata=L"comment";
	e.ret=in->comment;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&e,IPC_SET_EXTENDED_FILE_INFOW);

	if(in->year > 0)
	{
		e.metadata=L"year";
		StringCchPrintf(buf, 32, L"%d", in->year);
		e.ret=buf;
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&e,IPC_SET_EXTENDED_FILE_INFOW);
	}

	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&e,IPC_WRITE_EXTENDED_FILE_INFO);
}

extern "C" {
	__declspec( dllexport ) winampMediaLibraryPlugin * winampGetMediaLibraryPlugin() {
		return &plugin;
	}

	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		// prompt to remove our settings with default as no (just incase)
		wchar_t title[256] = {0};
		StringCchPrintf(title, 256, WASABI_API_LNGSTRINGW(IDS_NULLSOFT_FORMAT_CONVERTER), PLUGIN_VERSION);
		if(MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_DO_YOU_ALSO_WANT_TO_REMOVE_SETTINGS),
					  title,MB_YESNO|MB_DEFBUTTON2) == IDYES)
		{
			DeleteFile(inifile);
		}
		// if not transcoding then can remove on the fly (5.37+)
		if(!IsWindow(transcoderWnd)){
			return ML_PLUGIN_UNINSTALL_NOW;
		}
		// otherwise allow for any prompting/full restart removal (default)
		return ML_PLUGIN_UNINSTALL_REBOOT;
	}
};

static HWND GetDialogBoxParent()
{
	HWND parent = (HWND)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETDIALOGBOXPARENT);
	if (!parent || parent == (HWND)1)
		return plugin.hwndWinampParent;
	return parent;
}