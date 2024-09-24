#include "Main.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"

extern LARGE_INTEGER freq;
Out_Module* out_modules[32] = { 0 };
Out_Module* out_mod = 0;

typedef struct _PLUGINORDER
{
	LPCWSTR	name;
	bool	found;
} PLUGINORDER;

static PLUGINORDER preload[] =
{
	{ L"out_ds.dll",   false },
	{ L"out_wave.dll", false },
	{ L"out_disk.dll", false }
};

int LoadOutputPlugin(const wchar_t* plugin_filename, int index)
{
	wchar_t file[MAX_PATH] = { 0 };
	PathCombineW(file, PLUGINDIR, plugin_filename);

	HINSTANCE hLib = LoadLibraryW(file);
	if ( hLib == NULL )
		return index;


	Out_Module* (*pr)();
	pr = (Out_Module * (__cdecl*)(void)) GetProcAddress(hLib, "winampGetOutModule");
	if ( !pr )
		return index;


	Out_Module* mod = pr();
	if ( mod && (mod->version == OUT_VER || mod->version == OUT_VER_U) )
	{
		AutoChar narrowFn(plugin_filename);
		size_t fileNameSize = lstrlenA(narrowFn);

		if ( g_safeMode )
		{
			if ( !(mod->id == 1471482036 || mod->id == 426119909 || mod->id == 203968848) )
			{
				FreeModule(hLib);
				return index;
			}
		}

		mod->hDllInstance = hLib;
		mod->hMainWindow  = hMainWindow;
		mod->id           = (intptr_t)GlobalAlloc(GPTR, fileNameSize + 1);

		StringCchCopyA((char*)mod->id, fileNameSize + 1, narrowFn);

		mod->Init();
		out_modules[index] = mod;

		return index + 1;
	}


	return index;
}

void out_init()
{
	int index = 0, i = 0, count = sizeof(preload) / sizeof(PLUGINORDER);
	for ( ; i < count; i++ ) index = LoadOutputPlugin(preload[i].name, index);

	WIN32_FIND_DATAW d = { 0 };
	wchar_t dirstr[MAX_PATH] = { 0 };
	PathCombineW(dirstr, PLUGINDIR, L"OUT_*.DLL");
	HANDLE h = FindFirstFileW(dirstr, &d);
	if ( h != INVALID_HANDLE_VALUE )
	{
		do
		{
			for ( i = 0; i < count && (preload[i].found || lstrcmpiW(preload[i].name, d.cFileName)); i++ );
			if ( i == count ) index = LoadOutputPlugin(d.cFileName, index);
			else preload[i].found = true;
		} while ( FindNextFileW(h, &d) && index < 31 );
		FindClose(h);
	}
}

void out_setwnd(void)
{
	for ( int x = 0; out_modules[x]; x++ )
	{
		out_modules[x]->hMainWindow = hMainWindow;
	}
}

void out_changed(HINSTANCE hLib, int enabled)
{
	typedef void(__cdecl* OutModeChange)(int);
	OutModeChange modeChange = (OutModeChange)GetProcAddress(hLib, "winampGetOutModeChange");
	if ( modeChange )
	{
		modeChange(enabled);
	}
}

void out_deinit()
{
	for ( int x = 0; out_modules[x]; x++ )
	{
		//HINSTANCE hLib = out_modules[x]->hDllInstance;
		out_modules[x]->Quit();
		GlobalFree((HGLOBAL)out_modules[x]->id);
		out_modules[x]->id = 0;
		//FreeLibrary(hLib); // benski> we're just going to let it leak because it might be subclassing
		out_modules[x] = 0;
	}
}