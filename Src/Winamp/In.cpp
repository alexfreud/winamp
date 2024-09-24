#include "Main.h"

#include "resource.h"
#include <math.h>
#include "api.h"
//#define PROFILE_PLUGINS_LOAD_TIME
#include "timing.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include "WinampAttributes.h"
#include "eq10dsp.h"
#include "../nsutil/pcm.h"


int filter_srate = 0, filter_enabled = 1, filter_top = 0, filter_top2 = 10;
static In_Module* dsp_init_mod = 0;
static int dsp_in_init = 0;
std::vector<In_Module*> in_modules;
In_Module* in_mod = 0;
float preamp_val = 1.0f;

eq10_t* eq = 0;

extern "C" volatile int sa_override;

static void setinfo(int bitrate, int srate, int stereo, int synched);
static void vissa_init(int maxlatency_in_ms, int srate);
static void vissa_deinit();
static int sa_getmode();
static int eq_dosamples_4front(short* samples, int numsamples, int bps, int nch, int srate);
static int eq_dosamples(short* samples, int numsamples, int bps, int nch, int srate);
int benskiQ_eq_dosamples(short* samples, int numsamples, int bps, int nch, int srate);
static int eq_isactive();

static int myisourfile(const char* filename) // mjf bullshit
{
	return 0;
}

typedef struct _PLUGINORDER
{
	LPCWSTR	name;
	bool	found;
} PLUGINORDER;

static PLUGINORDER preload[] =
{
	// the one plug-in to rule all
	{ L"in_mp3.dll",    false },

	// no extension configuration
	{ L"in_avi.dll",    false },
	{ L"in_cdda.dll",   false },
	{ L"in_linein.dll", false },
	{ L"in_mkv.dll",    false },
	{ L"in_nsv.dll",    false },
	{ L"in_swf.dll",    false },
	{ L"in_vorbis.dll", false },

	// extension configuration
	{ L"in_mp4.dll",    false },
	{ L"in_dshow.dll",  false },	// load after in_avi and in_mkv as often those extensions are associated with this one as well
	{ L"in_flac.dll",   false },
	{ L"in_flv.dll",    false },
	{ L"in_wm.dll",     false },

	// tend to have a lot of extensions so do later
	{ L"in_wave.dll",   false },
	{ L"in_midi.dll",   false },
	{ L"in_mod.dll",    false }
};

static void LoadInputPlugin(const wchar_t* filename)
{
	wchar_t file[MAX_PATH] = { 0 };
	PathCombineW(file, PLUGINDIR, filename);

	HINSTANCE hLib = LoadLibraryW(file);
	if ( hLib == NULL )
		return;


	In_Module* (*pr)();
	pr = (In_Module * (__cdecl*)(void)) GetProcAddress(hLib, "winampGetInModule2");
	if ( pr == NULL )
		return;


	In_Module* mod = pr();
	if ( mod == NULL )
	{
		FreeModule(hLib);
		return;
	}


	int ver = ((mod->version & ~IN_UNICODE) & ~IN_INIT_RET);
	if ( !(ver == IN_VER || ver == IN_VER_OLD) )
	{
		FreeModule(hLib);
		return;
	}


	if ( g_safeMode )
	{
		char desc[128] = { 0 };
		lstrcpynA(desc, mod->description, sizeof(desc));
		if ( desc[0] && !memcmp(desc, "nullsoft(", 9) )
		{
			char* p = strrchr(desc, ')');
			if ( p )
			{
				*p = 0;
				if ( _wcsicmp(filename, AutoWide(desc + 9)) )
				{
					FreeModule(hLib);
					return;
				}
			}
		}
		else
		{
			FreeModule(hLib);
			return;
		}
	}

	mod->hDllInstance = hLib;
	mod->dsp_dosamples = eq_dosamples;
	mod->dsp_isactive = eq_isactive;
	mod->SAGetMode = sa_getmode;
	mod->SAAdd = (int(__cdecl*)(void*, int, int))sa_add;
	mod->SAVSAInit = vissa_init;
	mod->SAVSADeInit = vissa_deinit;
	mod->VSASetInfo = vis_setinfo;
	mod->VSAAdd = vsa_add;
	mod->VSAGetMode = vsa_getmode;
	mod->SAAddPCMData = sa_addpcmdata;
	mod->VSAAddPCMData = vsa_addpcmdata;
	mod->hMainWindow = hMainWindow;
	mod->SetInfo = NULL;
	mod->UsesOutputPlug &= ~(2 | 4 | 8 | 16);

	if ( !_wcsicmp(filename, L"in_mjf.dll") )
	{
		mod->IsOurFile = myisourfile;
	}

	// add 5.66+ to do better error handling on issues, etc
	if ( mod->version & IN_INIT_RET )
	{
		mod->service = WASABI_API_SVC;
	}
	int ret = mod->Init();
	if ( (mod->version & IN_INIT_RET) && (ret == IN_INIT_FAILURE) )
	{
		FreeModule(hLib);
		return;
	}

	in_modules.push_back(mod);

	if ( mod->SetInfo )
	{
		char* p = (char*)mod->SetInfo;
		if ( p && *p )
		{
			StringCchCatA(metric_plugin_list, 512, ":");
			StringCchCatA(metric_plugin_list, 512, p);
		}
	}

	mod->SetInfo = setinfo;

	if ( !g_has_video_plugin )
	{
		int (*gefiW)(const wchar_t* fn, const char* data, wchar_t* dest, int destlen);
		gefiW = (int(__cdecl*)(const wchar_t*, const char*, wchar_t*, int))GetProcAddress(hLib, "winampGetExtendedFileInfoW");
		if ( gefiW )
		{
			wchar_t dest[16] = { 0 };
			gefiW(L"", "type", dest, 16);
			if ( _wtoi(dest) == 1 ) g_has_video_plugin = 1;
		}

		int (*gefi)(const char* fn, const char* data, char* dest, int destlen);
		gefi = (int(__cdecl*)(const char*, const char*, char*, int))GetProcAddress(hLib, "winampGetExtendedFileInfo");
		if ( gefi )
		{
			char dest[16] = { 0 };
			gefi("", "type", dest, 16);
			if ( atoi(dest) == 1 ) g_has_video_plugin = 1;
		}
	}
}

int in_init()
{
	// input plugins require the main window, at least for IPC calls
	if ( !CreateMainWindow() )
		return FALSE;

	int i = 0, count = sizeof(preload) / sizeof(PLUGINORDER);

	for ( ; i < count; i++ )
		LoadInputPlugin(preload[i].name);

	WIN32_FIND_DATAW d = { 0 };
	wchar_t dirstr[MAX_PATH] = { 0 };

	PathCombineW(dirstr, PLUGINDIR, L"IN_*.DLL");
	HANDLE h = FindFirstFileW(dirstr, &d);
	if ( h != INVALID_HANDLE_VALUE )
	{
		do
		{
			for ( i = 0; i < count && (preload[i].found || lstrcmpiW(preload[i].name, d.cFileName)); i++ );
			if ( i == count ) LoadInputPlugin(d.cFileName);
			else preload[i].found = true;
		} while ( FindNextFileW(h, &d) );
		FindClose(h);
	}

	if ( (g_no_video_loaded = _r_i("no_video", 0)) ) g_has_video_plugin = 0;

	return TRUE;
}

In_Module* g_in_infobox = 0;

void in_deinit()
{
	size_t x = in_modules.size();
	while ( x-- )
	{
		In_Module*& mod = in_modules[x];
		// make sure there's something valid due to the dynamic unload changes in 5.5+
		if ( mod != g_in_infobox && mod )
		{
			//HINSTANCE hLib = mod->hDllInstance;
			mod->Quit();

			//FreeLibrary(hLib); // benski> we're just going to let it leak because it might be subclassing
			mod = 0;
		}
	}
	in_modules.clear();
}

In_Module* in_setmod_noplay(const wchar_t* filename, int* start_offs)
{
	size_t x;
	char ext[128] = { 0 };
	extension_ex(AutoChar(filename), ext, sizeof(ext));
	for ( x = start_offs ? *start_offs : 0; x < in_modules.size(); x++ )
	{
		if ( InW_IsOurFile(in_modules[x], filename) )
		{
			if ( start_offs )
				*start_offs = (int)x;

			if ( !in_modules[x]->hMainWindow )
				in_modules[x]->hMainWindow = hMainWindow;

			return in_modules[x];
		}
	}
	for ( x = start_offs ? *start_offs : 0; x < in_modules.size(); x++ )
	{
		if ( in_modules[x] )
		{
			char* allExtension = in_modules[x]->FileExtensions;
			while ( allExtension && *allExtension )
			{
				char* splitterBuffer = allExtension;
				char* extensionBuffer;
				do
				{
					// string split on ';'
					char currentExtension[20] = { 0 };
					lstrcpynA(currentExtension, splitterBuffer, 20);
					if ( (extensionBuffer = strstr(splitterBuffer, ";")) )
					{
						if ( (extensionBuffer - splitterBuffer) < 15 )
							currentExtension[extensionBuffer - splitterBuffer] = 0;
					}
					//else
						//d[lstrlen(splitterBuffer)] = 0;

					if ( !lstrcmpiA(ext, currentExtension) )
					{
						if ( start_offs )
							*start_offs = (int)x;
						if ( !in_modules[x]->hMainWindow )
							in_modules[x]->hMainWindow = hMainWindow;

						return in_modules[x];
					}
					splitterBuffer = extensionBuffer + 1;
				} while ( extensionBuffer );

				allExtension += lstrlenA(allExtension) + 1;

				if ( !*allExtension )
					break;

				allExtension += lstrlenA(allExtension) + 1;
			}
		}
	}

	if ( start_offs )
	{
		*start_offs = -1;
		return 0;
	}

	{
		static int r;
		const wchar_t* p;

		if ( PathFindExtensionW(filename)[0] && (p = wcsstr(filename, L"://")) && (p = wcsstr(p, L"?")) )
		{
			wchar_t* d = _wcsdup(filename);
			In_Module* v;
			d[p - filename] = 0;
			v = in_setmod_noplay(d, 0);
			free(d);
			return v;
		}

		if ( !config_defext[0] || config_defext[0] == ' ' )
			StringCchCopyA(config_defext, 32, "mp3");

		if ( !r )
		{
			wchar_t a[128] = L"hi.";
			In_Module* v;
			MultiByteToWideCharSZ(CP_ACP, 0, config_defext, -1, a + 3, 120);
			r = 1;
			v = in_setmod_noplay(a, 0);
			r = 0;
			return v;
		}
		else
			return 0;
	}
}

In_Module* in_setmod(const wchar_t* filename)
{
	In_Module* i = in_setmod_noplay(filename, 0);
	if ( !i ) return 0;
	if ( i->UsesOutputPlug & IN_MODULE_FLAG_USES_OUTPUT_PLUGIN )
	{
		int t;
		for ( t = 0; out_modules[t] && _stricmp(config_outname, (char*)out_modules[t]->id); t++ );
		if ( !out_modules[t] )
		{
			for ( t = 0; out_modules[t]; t++ )
			{
				if ( !_stricmp("out_ds.dll", (char*)out_modules[t]->id) )
				{
					break;
				}
				else if ( !_stricmp("out_wave.dll", (char*)out_modules[t]->id) )
				{
					break;
				}
			}

			if ( !out_modules[t] )
			{
				LPMessageBox(DIALOG_PARENT(hMainWindow), IDS_NOOUTPUT, IDS_ERROR, MB_OK | MB_ICONERROR);
				return (In_Module*)-31337;
			}
		}

		// TODO only call out_changed(..) if we are different from before
		//		though currently changing the output prefs and playing a
		//		new track will generate a change (which is expected but
		//		it might be assumed to be wrong so may need to document)
		int changed = 0;
		if ( !i->outMod || i->outMod && i->outMod->hDllInstance != out_modules[t]->hDllInstance )
		{
			changed = 1;
			if ( i->outMod ) out_changed(i->outMod->hDllInstance, OUT_UNSET | OUT_PLAYBACK);
		}
		i->outMod = out_modules[t];
		i->outMod->hMainWindow = hMainWindow;
		if ( changed ) out_changed(i->outMod->hDllInstance, OUT_SET | OUT_PLAYBACK);
	}
	else
	{
		if ( i->outMod ) out_changed(i->outMod->hDllInstance, OUT_UNSET | OUT_PLAYBACK);
		i->outMod = NULL;
	}
	return i;
}

void in_flush(int ms)
{
	if ( in_mod && in_mod->outMod )
	{
		in_mod->outMod->Flush(ms);
	}
}

int in_getouttime(void)
{
	if ( in_mod ) return in_mod->GetOutputTime();
	return 0;
}

int in_getlength(void)
{
	if ( in_mod )
	{
		int t = in_mod->GetLength() / 1000;
		if ( t < 1 && t != -1 )
			t = 1;

		return t;
	}

	return -1;
}

void in_pause(int p)
{
	if ( in_mod )
	{
		if ( p )
			in_mod->Pause();
		else
			in_mod->UnPause();
	}
}

void in_setvol(int v)
{
	if ( in_mod )
		in_mod->SetVolume(v);

	if ( config_eq_ws && config_eq_open )
		draw_eq_tbar(GetForegroundWindow() == hEQWindow ? 1 : (config_hilite ? 0 : 1));

	PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_VOLUME, IPC_CB_MISC);


	//////CefRefPtr<wa_Cef_App> l_CefApp = wa_Cef_App::GetInstance();

	//////if ( l_CefApp.get() != nullptr )
	//////{
	//////	l_CefApp.get()->setVolume( v );
	//////}

}

void in_setpan(int p)
{
	if ( in_mod )
		in_mod->SetPan(p);

	if ( config_eq_ws && config_eq_open )
		draw_eq_tbar(GetForegroundWindow() == hEQWindow ? 1 : (config_hilite ? 0 : 1));

	PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_VOLUME, IPC_CB_MISC);
}

int in_seek(int time_in_ms)
{
	if ( in_mod )
		in_mod->SetOutputTime(time_in_ms);

	return 0;
}

extern "C" int g_need_trusted_dsp = 0;

int in_open(const wchar_t* fn)
{
	in_mod = in_setmod(fn);
	if ( (int)in_mod <= 0 ) return (!in_mod ? 1 : *(int*)in_mod);
	sa_setthread(config_sa);
	in_setvol(config_volume);
	in_setpan(config_pan);

	dsp_in_init = 0;
	dsp_init_mod = in_mod;
	filter_srate = 0;

	int r = InW_Play(in_mod, fn);
	return r;
}

void in_close(void)
{
	if ( in_mod )
		in_mod->Stop();

	if ( NULL != eq )
	{
		free(eq);
		eq = NULL;
	}

	timingPrint();
}

enum FileInfoMode
{
	FILEINFO_MODE_0,
	FILEINFO_MODE_1,
};

int ModernInfoBox(In_Module* in, FileInfoMode mode, const wchar_t* filename, HWND parent);
typedef int(__cdecl* UseUnifiedFileInfoDlg)(const wchar_t* fn);

int in_infobox(HWND hwnd, const wchar_t* fn)
{
	const wchar_t* p = wcsstr(fn, L"aol.com/uvox");
	if ( p ) return 0;

	if ( g_in_infobox ) return 0;

	In_Module* mod = in_setmod_noplay(fn, 0);
	if ( !mod ) return 0;

	g_in_infobox = mod;

	UseUnifiedFileInfoDlg uufid = (UseUnifiedFileInfoDlg)GetProcAddress(mod->hDllInstance, "winampUseUnifiedFileInfoDlg");

	// focus often gets reverted back to the main window after this dialog
	// so we will remember what window used to have focus
	HWND oldFocus = GetFocus();
	int a = 0;

	int ret = 0;
	if ( uufid ) {
		ret = uufid(fn);
		if ( ret == 1 ) {
			a = ModernInfoBox(mod, FILEINFO_MODE_0, fn, hwnd);
		}
		else if ( ret == 2 )
			a = ModernInfoBox(mod, FILEINFO_MODE_1, fn, hwnd);
	}

	if ( ret == 0 ) {
		// added 5.66 so plug-ins can get a hint that something may change...
		SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)fn, IPC_FILE_TAG_MAY_UPDATEW);
		a = InW_InfoBox(mod, fn, hwnd);
	}

	SetFocus(oldFocus);

	if ( !a )
	{
		SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)fn, IPC_FILE_TAG_MAY_HAVE_UPDATEDW);
	}

	g_in_infobox = 0;

	return a;
}

static void AddFilterString(char* p, size_t size, const char* a)
{
	while ( a && *a )
	{
		char* t = 0;
		StringCchCatExA(p, size, ";*.", &t, &size, 0);
		t = p + lstrlenA(p);
		while ( a && *a && *a != ';' ) *t++ = *a++;
		*t = 0;
		if ( a ) a++;

		if ( a && a[-1] ) continue;
		if ( !*a ) break;
		if ( a ) a += lstrlenA(a) + 1;
	}
}

char* in_getfltstr()
{
	int in_mod = -1;
	int in_wave = -1;
	size_t size = 256 * 1024;
	char* buf = (char*)GlobalAlloc(GPTR, size); // this is gay, should have a growing buffer or somethin.
	char* p = buf;
	size_t x;
	getString(IDS_ALLTYPES, p, size);
	p = p + lstrlenA(p) + 1;
	wchar_t playlistString[1024] = { 0 };
	playlistManager->GetFilterList(playlistString, 1024);
	WideCharToMultiByteSZ(CP_ACP, 0, playlistString, -1, p, (int)size, 0, 0);
	for ( x = 0; x < in_modules.size(); x++ )
	{
		char* a = in_modules[x]->FileExtensions;
		if ( a && *a )
		{
			/* we want to skip in_mod and in_wave because they have TOO MANY extensions and we are limited to MAX_PATH (260)
			   we'll tack them at the end just in case we have enough room for 'em */
			if ( in_mod < 0 && !strncmp(a, "mod;", 4) )
				in_mod = (int)x;
			else if ( in_wave < 0 && strstr(a, "aiff") )  // detection for in_wave.  not the best but should work
				in_wave = (int)x;
			else AddFilterString(p, size, a);
		}
	}

	/* add in_wave and in_mod last */
	if ( in_wave >= 0 )
		AddFilterString(p, size, in_modules[in_wave]->FileExtensions);

	if ( in_mod >= 0 ) // fuck you in_mod :)
		AddFilterString(p, size, in_modules[in_mod]->FileExtensions);

	p = p + lstrlenA(p) + 1;
	getString(IDS_PLAYLISTSTRING, p, size - (p - buf));
	p += lstrlenA(p) + 1;
	playlistManager->GetFilterList(playlistString, 1024);
	WideCharToMultiByteSZ(CP_ACP, 0, playlistString, -1, p, (int)size, 0, 0);
	p += lstrlenA(p) + 1;
	for ( x = 0; x < in_modules.size(); x++ )
	{
		char* a = in_modules[x]->FileExtensions;
		while ( a && *a )
		{
			char* b = a;
			a += lstrlenA(a) + 1;
			if ( !*a ) break;
			StringCchCopyExA(p, size, a, &p, &size, 0);
			p++;
			{
				do
				{
					char* c;
					StringCchCopyA(p, size, "*.");
					StringCchCatA(p, size, b);
					if ( (b = strstr(b, ";")) ) b++;
					if ( (c = strstr(p, ";")) ) c[1] = 0;

					p += lstrlenA(p);
				} while ( b );
				p++;
				a += lstrlenA(a) + 1;
			}
		}
	}
	StringCchCopyExA(p, size, getString(IDS_OFD_ALL_FILES, NULL, 0), &p, &size, 0);
	p++; size--;
	lstrcpynA(p, "*.*", (int)size);
	p += 3;
	*p = 0;

	{
		char* newbuf;
		size = p + 5 - buf;
		newbuf = (char*)GlobalAlloc(GPTR, size);
		memcpy(newbuf, buf, size);
		GlobalFree(buf);
		return newbuf;
	}
}

static void AddFilterStringW(wchar_t* p, size_t& size, const char* a, BOOL skip)
{
	while ( a && *a )
	{
		wchar_t* t = 0;
		StringCchCatExW(p, size, ((*p) ? L";*." : L"*."), &t, &size, 0);
		size_t extsize = 0;
		while ( a[extsize] && a[extsize] != ';' )
			extsize++;

		*(t += MultiByteToWideCharSZ(CP_ACP, 0, a, (int)extsize, t, (int)size) - 1);
		a += extsize + 1;

		if ( a[-1] ) continue;
		if ( !*a ) break;
		a += lstrlenA(a) + 1;

		// limit the length of the filter to fit into MAX_PATH otherwise
		// if things go past this then mainly for the all supported type
		// it can sometimes act like *.* due to where the filter is cut
		if ( !skip && lstrlenW(p) >= MAX_PATH )
		{
			// if we end with a . then need to fake things to act like a new 
			// filter since windows will interpret the . as a *.* which is bad
			if ( *(p + MAX_PATH) == L'.' )
			{
				*(p + MAX_PATH - 1) = 0;
				*(p + MAX_PATH) = 0;
			}
		}
	}
}

static wchar_t* inc(wchar_t* p, size_t& size, int extra = 0)
{
	int len = lstrlenW(p) + extra;
	size -= len;
	p += len;
	return p;
}

wchar_t* in_getfltstrW(BOOL skip)
{
	int in_mod = -1;
	int in_wave = -1;
	size_t size = 256 * 1024;
	wchar_t* buf = (wchar_t*)GlobalAlloc(GPTR, size * sizeof(wchar_t)); // this is gay, should have a growing buffer or somethin.
	wchar_t* p = buf, * ps;
	*p = 0;
	size_t x = 0;

	{
		int cnt = lstrlenW(getStringW(IDS_ALLTYPES, buf, size)) + 1;
		p += cnt; size -= cnt;
		if ( playlistManager )
		{
			playlistManager->GetExtensionList(p, size);
			size -= lstrlenW(p);
		}
	}
	ps = p;

	for ( x = 0; x < in_modules.size(); x++ )
	{
		char* a = in_modules[x]->FileExtensions;
		if ( a && *a )
		{
			/* we want to skip in_mod and in_wave because they have TOO MANY extensions and we are limited to MAX_PATH (260)
			   we'll tack them at the end just in case we have enough room for 'em */
			if ( in_mod < 0 && !strncmp(a, "mod;", 4) )
				in_mod = (int)x;
			else if ( in_wave < 0 && strstr(a, "aiff") )  // detection for in_wave.  not the best but should work
				in_wave = (int)x;
			else AddFilterStringW(p, size, a, skip);
		}
	}

	/* add in_wave and in_mod last */
	if ( in_wave >= 0 )
		AddFilterStringW(p, size, in_modules[in_wave]->FileExtensions, skip);

	if ( in_mod >= 0 ) // fuck you in_mod :)
		AddFilterStringW(p, size, in_modules[in_mod]->FileExtensions, skip);

	if ( *p )
		p += lstrlenW(p) + 1; // don't decrement size here cause it was done already

	// uppercase the extensions so is consistent as can be
	CharUpperBuffW(ps, 256 * 1024 - (p - ps));

	if ( playlistManager )
	{
		wchar_t ext[512] = { 0 };
		playlistManager->GetExtensionList(ext, 512);
		StringCchPrintfW(p, size, getStringW(IDS_PLAYLISTSTRING_NEW, NULL, 0), ext);
		int cnt = lstrlenW(p) + 1;
		p += cnt; size -= cnt;
		StringCchCatW(p, size, ext);
		p = inc(p, size, 1);
	}

	for ( x = 0; x < in_modules.size(); x++ )
	{
		char* a = in_modules[x]->FileExtensions;
		while ( a && *a )
		{
			char* b = a;
			a += lstrlenA(a) + 1;
			if ( !*a ) break;
			// adjust down by 1 so that we have the actual string length exluding null termination
			int cnt = MultiByteToWideCharSZ(CP_ACP, 0, a, -1, p, (int)size) - 1;
			p += cnt; size -= cnt;
			{
				do
				{
					wchar_t* c = 0;
					StringCchCopyW(p, size, L"*.");
					StringCchCatW(p, size, AutoWide(b));
					if ( (b = strstr(b, ";")) ) b++;
					if ( (c = wcsstr(p, L";")) ) c[1] = 0;

					p = inc(p, size);
				} while ( b );
				p++;
				size--;
				a += lstrlenA(a) + 1;
			}
		}
	}
	StringCchCopyExW(p, size, getStringW(IDS_OFD_ALL_FILES, NULL, 0), &p, &size, 0);
	p++; size--;
	lstrcpynW(p, L"*.*", (int)size);
	p += 3;
	*p = 0;

	{
		wchar_t* newbuf = 0;
		size = p + 5 - buf;
		newbuf = (wchar_t*)GlobalAlloc(GPTR, size * sizeof(wchar_t));
		memcpy(newbuf, buf, size * sizeof(wchar_t));
		GlobalFree(buf);
		return newbuf;
	}
}

char* in_getextlist()
{
	size_t x;
	char* mem = NULL, * p;
	int size = 1024;
	std::vector<LPSTR> exts;

	for ( x = 0; x != in_modules.size(); x++ )
	{
		char* a = in_modules[x]->FileExtensions;
		while ( a && *a )
		{
			char* b = a, * c = 0;
			do
			{
				c = strstr(b, ";");
				if ( c )
				{
					char temp[32] = { 0 };
					lstrcpynA(temp, b, c - b + 1);
					int count = CharUpperBuffA(temp, ARRAYSIZE(temp));

					bool skip = false;
					for ( size_t i = 0; i < exts.size(); i++ )
					{
						if ( !stricmp(exts[i], temp) )
						{
							skip = true;
							break;
						}
					}
					if ( !skip )
					{
						exts.push_back(_strdup(temp));
						size += count + 2;
					}
				}
				else
				{
					char temp[32] = { 0 };
					lstrcpynA(temp, b, c - b + 1);
					int count = CharUpperBuffA(temp, ARRAYSIZE(temp));

					bool skip = false;
					for ( size_t i = 0; i < exts.size(); i++ )
					{
						if ( !stricmp(exts[i], temp) )
						{
							skip = true;
							break;
						}
					}
					if ( !skip )
					{
						exts.push_back(_strdup(temp));
						size += count + 2;
					}
				}
				b = c + 1;
			} while ( c && *c );

			a += lstrlenA(a) + 1;
			if ( !*a ) break;
			a += lstrlenA(a) + 1;
		}
	}


	if ( size > 0 )
	{
		p = mem = (char*)GlobalAlloc(GPTR, size);

		for ( x = 0; x != exts.size(); x++ )
		{
			char* e = exts[x];
			if ( e && *e )
			{
				lstrcpynA(p, e, 32);
				p += lstrlenA(p) + 1;
				free(e);
			}
		}

		*p = 0;
	}

	return mem;
}

wchar_t* in_getextlistW()
{
	size_t x = 0;
	wchar_t* mem = NULL, * p = 0;
	int size = 0;
	std::vector<LPWSTR> exts;

	for ( x = 0; x != in_modules.size(); x++ )
	{
		char* a = in_modules[x]->FileExtensions;
		while ( a && *a )
		{
			char* b = a, * c;
			do
			{
				wchar_t temp[32] = { 0 };
				c = strstr(b, ";");
				if ( c )
				{
					int count = MultiByteToWideChar(CP_ACP, 0, b, c - b, 0, 0);
					MultiByteToWideChar(CP_ACP, 0, b, c - b, temp, count);
					CharUpperBuffW(temp, ARRAYSIZE(temp));

					bool skip = false;
					for ( size_t i = 0; i < exts.size(); i++ )
					{
						if ( !wcsicmp(exts[i], temp) )
						{
							skip = true;
							break;
						}
					}
					if ( !skip )
					{
						exts.push_back(_wcsdup(temp));
						size += count + 2;
					}
				}
				else
				{
					int count = MultiByteToWideChar(CP_ACP, 0, b, -1, 0, 0);
					MultiByteToWideChar(CP_ACP, 0, b, -1, temp, count);
					CharUpperBuffW(temp, ARRAYSIZE(temp));

					bool skip = false;
					for ( size_t i = 0; i < exts.size(); i++ )
					{
						if ( !wcsicmp(exts[i], temp) )
						{
							skip = true;
							break;
						}
					}
					if ( !skip )
					{
						exts.push_back(_wcsdup(temp));
						size += count + 2;
					}
				}
				b = c + 1;
			} while ( c && *c );

			a += lstrlenA(a) + 1;
			if ( !*a ) break;
			a += lstrlenA(a) + 1;
		}
	}

	if ( size > 0 )
	{
		p = mem = (wchar_t*)GlobalAlloc(GPTR, size * sizeof(wchar_t));

		for ( x = 0; x != exts.size(); x++ )
		{
			wchar_t* e = exts[x];
			if ( e && *e )
			{
				lstrcpynW(p, e, 32);
				p += lstrlenW(p) + 1;
				free(e);
			}
		}
		*p = 0;
	}

	return mem;
}

static void vissa_init(int maxlatency_in_ms, int srate)
{
	g_srate_exact = srate;
	int nf = MulDiv(maxlatency_in_ms * 4, srate, 450000);
	//int nf = maxlatency_in_ms/5;
	sa_init(nf);
	vsa_init(nf);
	vu_init(nf, srate);
}

static void vissa_deinit()
{
	sa_deinit();
	vsa_deinit();
	vu_deinit();
}

static void setinfo(int bitrate, int srate, int stereo, int synched)
{
	int last_brate = g_brate, last_srate = g_srate, last_nch = g_nch;

	if ( stereo != -1 )
	{
		g_nch = stereo;
		// dynamic channels
		if ( g_nch > 0 )
		{
			if ( NULL != eq ) { free(eq); eq = NULL; }
			eq = (eq10_t*)calloc(g_nch, sizeof(eq10_t));
			if ( NULL == eq )
			{
				// bad. Need to handle this.
				g_nch = 0;
			}
			eq_set(config_use_eq, (char*)eq_tab, config_preamp);
		}
	}

	if ( bitrate != -1 && srate != -1 )
	{
		g_need_titleupd = 1;
	}

	if ( bitrate != -1 )
		g_brate = bitrate;
	if ( srate != -1 )
	{
		g_srate = srate;
		switch ( srate )
		{
		case 11:
			g_srate_exact = 11025; break;
		case 22:
			g_srate_exact = 22050; break;
		case 44:
			g_srate_exact = 44100; break;
		case 88:
			g_srate_exact = 88200; break;
		default:
			g_srate_exact = srate * 1000; break;
		}
	}

	if ( bitrate != -1 || srate != -1 || stereo != -1 )
	{
		static unsigned int last_t;
		unsigned int now = GetTickCount();

		//detect wrap with the first one
		if ( now < last_t || now > last_t + 500 || last_brate != g_brate || last_srate != g_srate || last_nch != g_nch )
		{
			last_t = now;
			PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_INFO, IPC_CB_MISC);
		}
	}

	if ( synched != -1 || stereo != -1 )
	{
		g_need_infoupd = synched | (stereo != -1 ? 8 : 4) | (g_need_infoupd & 8);
	}
}

static int sa_getmode()
{
	if ( sa_override ) return 3;

	if ( sa_curmode == 4 && !config_mw_open && config_pe_open )
		return 1;
	return sa_curmode;
}

static int eq_isactive()
{
	int r = dsp_isactive();
	if ( r ) return 1;
	if ( in_mod && !(in_mod->UsesOutputPlug & IN_MODULE_FLAG_REPLAYGAIN) && config_replaygain.GetBool() && (config_replaygain_non_rg_gain.GetFloat() != 0) ) return 1;
	if ( in_mod && !(in_mod->UsesOutputPlug & IN_MODULE_FLAG_REPLAYGAIN_PREAMP) && config_replaygain.GetBool() && (config_replaygain_preamp.GetFloat() != 0) ) return 1;
	if ( filter_enabled ) return 2;
	return 0;
}

static float eq_lookup1[64] = {
	4.000000f, 3.610166f, 3.320019f, 3.088821f, 2.896617f,
	2.732131f, 2.588368f, 2.460685f, 2.345845f, 2.241498f,
	2.145887f, 2.057660f, 1.975760f, 1.899338f, 1.827707f,
	1.760303f, 1.696653f, 1.636363f, 1.579094f, 1.524558f,
	1.472507f, 1.422724f, 1.375019f, 1.329225f, 1.285197f,
	1.242801f, 1.201923f, 1.162456f, 1.124306f, 1.087389f,
	1.051628f, 1.000000f, 0.983296f, 0.950604f, 0.918821f,
	0.887898f, 0.857789f, 0.828454f, 0.799853f, 0.771950f,
	0.744712f, 0.718108f, 0.692110f, 0.666689f, 0.641822f,
	0.617485f, 0.593655f, 0.570311f, 0.547435f, 0.525008f,
	0.503013f, 0.481433f, 0.460253f, 0.439458f, 0.419035f,
	0.398970f, 0.379252f, 0.359868f, 0.340807f, 0.322060f,
	0.303614f, 0.285462f, 0.267593f, 0.250000f
};

///////////////// EQ CODE /////////////////////////

static int __inline float_to_int(double a)
{
	a += ((((65536.0 * 65536.0 * 16) + (65536.0 * 0.5)) * 65536.0));
	return ((*(int*)&a) - 0x80000000);
}

float* splbuf = 0;
int splbuf_alloc = 0;
float eqt[10] = { 0 };

static void FillFloat(float* floatBuf, void* samples, size_t bps, size_t numSamples, size_t numChannels, float preamp)
{
	nsutil_pcm_IntToFloat_Interleaved_Gain(floatBuf, samples, (int)bps, numSamples * numChannels, preamp);
}

static void FillSamples(void* samples, float* floatBuf, size_t bps, size_t numSamples, size_t numChannels)
{
	nsutil_pcm_FloatToInt_Interleaved(samples, floatBuf, (int)bps, numSamples * numChannels);
}

static float NonReplayGainAdjust()
{
	if ( !(in_mod->UsesOutputPlug & IN_MODULE_FLAG_REPLAYGAIN) && config_replaygain.GetBool() )
		return pow(10.0f, (float)config_replaygain_non_rg_gain / 20.0f);
	else
		return 1.0f;
}

static float ReplayGainPreamp()
{
	if ( !(in_mod->UsesOutputPlug & IN_MODULE_FLAG_REPLAYGAIN_PREAMP) && config_replaygain.GetBool() )
		return pow(10.0f, (float)config_replaygain_preamp / 20.0f);
	else
		return 1.0f;
}

static int eq_dosamples_4front(short* samples, int numsamples, int bps, int nch, int srate)
{
	g_srate_exact = srate;
	//char *csamples = (char *)samples;
	short* oldsamples = samples;

	if ( filter_enabled && in_mod && !(in_mod->UsesOutputPlug & IN_MODULE_FLAG_EQ) )
	{
		if ( !eq || filter_srate != srate )
		{
			if ( !eq ) eq = (eq10_t*)calloc(nch, sizeof(eq10_t));
			eq10_setup(eq, nch, (float)srate); // initialize
			for ( int x = 0; x < 10; x++ )
				eq10_setgain(eq, nch, x, eqt[x]);
			filter_srate = srate;
		}
		if ( splbuf_alloc < numsamples * nch )
		{
			int new_splbuf_alloc = numsamples * nch;
			float* new_splbuf = (float*)realloc(splbuf, 2 * sizeof(float) * new_splbuf_alloc);
			if ( new_splbuf )
			{
				splbuf_alloc = new_splbuf_alloc;
				splbuf = new_splbuf;
			}
		}
		if ( splbuf && eq )
		{
			int y = nch * numsamples;
			FillFloat(splbuf, samples, bps, numsamples, nch, preamp_val * NonReplayGainAdjust() * ReplayGainPreamp());
			for ( int x = 0; x < nch; x++ )
			{
				eq10_processf(eq + x, splbuf, splbuf + y, numsamples, x, nch);
			}
			FillSamples(samples, splbuf + y, bps, numsamples, nch);
		}
	}
	else if ( !(in_mod->UsesOutputPlug & IN_MODULE_FLAG_REPLAYGAIN) && config_replaygain.GetBool() && (config_replaygain_non_rg_gain.GetFloat() != 0) )
	{
		if ( splbuf_alloc < numsamples * nch )
		{
			int new_splbuf_alloc = numsamples * nch;
			float* new_splbuf = (float*)realloc(splbuf, 2 * sizeof(float) * new_splbuf_alloc);
			if ( new_splbuf )
			{
				splbuf_alloc = new_splbuf_alloc;
				splbuf = new_splbuf;
			}
		}
		if ( splbuf )
		{
			FillFloat(splbuf, samples, bps, numsamples, nch, NonReplayGainAdjust() * ReplayGainPreamp());
			FillSamples(samples, splbuf, bps, numsamples, nch);
		}
	}
	else if ( !(in_mod->UsesOutputPlug & IN_MODULE_FLAG_REPLAYGAIN_PREAMP) && config_replaygain.GetBool() && (config_replaygain_preamp.GetFloat() != 0) )
	{
		if ( splbuf_alloc < numsamples * nch )
		{
			int new_splbuf_alloc = numsamples * nch;
			float* new_splbuf = (float*)realloc(splbuf, 2 * sizeof(float) * new_splbuf_alloc);
			if ( new_splbuf )
			{
				splbuf_alloc = new_splbuf_alloc;
				splbuf = new_splbuf;
			}
		}
		if ( splbuf )
		{
			FillFloat(splbuf, samples, bps, numsamples, nch, ReplayGainPreamp());
			FillSamples(samples, splbuf, bps, numsamples, nch);
		}
	}
	else
		filter_srate = 0;
	return dsp_dosamples(oldsamples, numsamples, bps, nch, srate);
}

static __inline double VALTODB(int v)
{
	v -= 31;
	if ( v < -31 ) v = -31;
	if ( v > 32 ) v = 32;

	if ( v > 0 ) return -12.0 * (v / 32.0);
	else if ( v < 0 )
	{
		return -12.0 * (v / 31.0);
	}
	return 0.0f;
}

static void eq_set_4front(char data[10])
{
	if ( !eq )
		return;

	for ( int x = 0; x < 10; x++ )
	{
		eqt[x] = (float)VALTODB(data[x]);
		if ( filter_srate ) eq10_setgain(eq, g_nch, x, eqt[x]);
	}
}

static bool eq_do_first = true;
static int startup_config_eq_type = EQ_TYPE_4FRONT;
void benskiQ_eq_set(char data[10]);
void eq_set(int on, char data[10], int preamp)
{
	if ( eq_do_first )
	{
		startup_config_eq_type = config_eq_type;
		eq_do_first = false;
	}

	int x;
	if ( in_mod && in_mod->EQSet ) in_mod->EQSet(on, data, preamp);

	for ( x = 9; x >= 0 && data[x] == 31; x++ );
	if ( x >= 0 )
		filter_top2 = x;

	for ( x = 0; x < 10 && data[x] == 31; x++ );
	if ( !on || (preamp == 31 && x == 10) )
	{
		filter_enabled = 0;
	}
	else filter_enabled = 1;
	preamp_val = (float)eq_lookup1[preamp];

	if ( startup_config_eq_type == EQ_TYPE_4FRONT )
		eq_set_4front(data);
	if ( startup_config_eq_type == EQ_TYPE_CONSTANT_Q )
		benskiQ_eq_set(data);
}

static int eq_dosamples(short* samples, int numsamples, int bps, int nch, int srate)
{
	if ( eq_do_first )
	{
		startup_config_eq_type = config_eq_type;
		eq_do_first = false;
	}
	if ( startup_config_eq_type == EQ_TYPE_4FRONT )
		return eq_dosamples_4front(samples, numsamples, bps, nch, srate);
	else
		return benskiQ_eq_dosamples(samples, numsamples, bps, nch, srate);
}