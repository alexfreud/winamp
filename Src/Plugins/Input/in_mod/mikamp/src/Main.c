#include "api.h"
extern "C" {
#include "main.h"
}
#include "log.h"
#include "../../winamp/wa_ipc.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoCharFn.h"
#include <shlwapi.h>
#include <commdlg.h>

extern "C" MMSTREAM *_mm_fopen_rf(const CHAR *fname);	//rf_wrapper.c

//
//  data types and stuff
//

#define SU_POSITION         1
#define SU_TIME             2

#define PPF_CONT_LOOP       1
#define PPF_LOOPALL         2
#define PPF_ADD_TITLE       4

typedef struct
{
    const char *cmd;
    const char *file;
    const char *title;
    int   titleLength;
    int   start;
    int   startUnit;
    int   loops;
    int   flags;
} PlayParams;


// Public Globals!
// ---------------
extern "C" 
{
	UNIMOD           *mf;
	MPLAYER          *mp;
	int               paused;
	int               decode_pos;       // in 1/64th of millisecond
	extern char       cfg_format[];
}


void infobox_setmodule(HWND hwnd);


// Static Globals!
// ---------------

#define SILENCE_THRESHOLD  10800

extern "C" int GetSampleSizeFlag();
static char ERROR_TITLE[64];

static int          is_tempfile = 0;
static char         cmdName[2048], saveName[MAX_PATH];
static char         songTitle[400];                         // as in Winamp
static PlayParams   currParams; 

static HANDLE       thread_handle = INVALID_HANDLE_VALUE;
static volatile int killDecodeThread;
static volatile int seek_needed;

// wasabi based services for localisation support
api_application *WASABI_API_APP = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

extern "C" DWORD WINAPI decodeThread(void *b);
void __cdecl setoutputtime(int time_in_ms);

// =====================================================================================
//  error handling shiz
// =====================================================================================

static int lastError = 0;

__inline void mm_clearerror() { lastError = 0; }

static void mmerr(int crap, const CHAR *crud)
{
	char tmp[128] = {0};
    if (lastError==crap || crap==MMERR_OPENING_FILE)
        return;
	else
	{
		if(!lstrcmpi(crud,"Corrupt file or unsupported module type."))
		{
			WASABI_API_LNGSTRING_BUF(IDS_CORRUPT_UNSUPPORTED_TYPE,tmp,128);
		}
		else
			tmp[0] = 0;
	}

	MessageBox(mikmod.hMainWindow, (tmp[0]?tmp:crud), ERROR_TITLE, MB_ICONERROR);
    lastError = crap;
}


// =====================================================================================
static int __cdecl init(void)
// =====================================================================================
{
	if (!IsWindow(mikmod.hMainWindow))
		return IN_INIT_FAILURE;

	waServiceFactory *sf = mikmod.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	sf = mikmod.service->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mikmod.hDllInstance,InModLangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_MODULE_DECODER),PLUGIN_VER);
	mikmod.description = (char*)szDescription;

	WASABI_API_LNGSTRING_BUF(IDS_MOD_PLUGIN_ERROR,ERROR_TITLE,64);

	_mmerr_sethandler(&mmerr);

    config_read();

    Mikmod_RegisterAllLoaders();
    Mikmod_RegisterDriver(drv_amp);
	//Mikmod_RegisterDriver(drv_buffer);
	return IN_INIT_SUCCESS;
}

// =====================================================================================
static void __cdecl quit()
// =====================================================================================
{  
//LOG    log_exit();
    SL_Cleanup();
}

static MDRIVER  *md;

// =====================================================================================
//  file open shiz
// =====================================================================================

#define IPC_GETHTTPGETTER 240

__inline char *GetFileName(const char *fullname)
{
    const char *c = fullname + strlen(fullname) - 1;

    while (c > fullname)
    {
        if (*c=='\\' || *c=='/')
        {
            c++;
            break;
        }
        c--;
    }

    return (char*)c;
}

char* BuildFilterString(void)
{
	static char filterStr[128] = {0};
	if(!filterStr[0])
	{
		char* temp = filterStr;
		WASABI_API_LNGSTRING_BUF(IDS_ALL_FILES,filterStr,128);
		temp += lstrlen(filterStr)+1;
		lstrcpy(temp, "*.*");
		*(temp = temp + lstrlen(temp) + 1) = 0;
	}
	return filterStr;
}

BOOL GetPlayParams(const char *fileName, BOOL open, PlayParams *params)
{
    mm_clearerror();

    // fill params
    params->cmd         = fileName;
    params->start       = 0;
    params->loops       = config_loopcount;
    params->titleLength = 0;
    params->flags       = 0;

    if (config_playflag & CPLAYFLG_CONT_LOOP)
        params->flags |= PPF_CONT_LOOP;

    if (config_playflag & CPLAYFLG_LOOPALL)
        params->flags |= PPF_LOOPALL;

    if (params->loops == -1)
        params->flags &= ~PPF_CONT_LOOP;

    // check for mod:// prefix
    if (!strncmp(fileName, "mod://", 6))
    {
        const char *c = fileName += 6;

        while (c && *c && *c!=':')
        {
            // jump to
            if (!strncmp(c, "jmp=", 4))
            {
                // jump units
                switch (*(c + 4))
                {
                // position
                case 'p':
                    params->startUnit = SU_POSITION;
                    params->flags &= ~PPF_CONT_LOOP;
                    break;
                // time
                case 't':
                    params->startUnit = SU_TIME;
                    break;
                // invalid
                default:
                    return FALSE;
                }
                params->start = atoi(c + 5);
            }
            // loops
            else if (!strncmp(c, "lop=", 4))
            {
                if (*(c+4) == 'u')
                {
                    params->flags |= PPF_LOOPALL;
                    c++;
                }
                params->loops = atoi(c + 4);
                params->loops = _mm_boundscheck(params->loops, -1, 64);
                if (params->loops == -1)
                    params->flags &= ~PPF_CONT_LOOP;
            }
            // continue after loop
            else if (!strncmp(c, "con=", 4))
            {
                if (atoi(c + 4))
                    params->flags |= PPF_CONT_LOOP;
                else params->flags &= ~PPF_CONT_LOOP;
            }
            // title
            else if (!strncmp(c, "tit=", 4))
            {
                // find string
                const char *p = c + 4;

                if (*p == '+')
                {
                    params->flags |= PPF_ADD_TITLE;
                    c++;
                    p++;
                }

                if (*p++ != '"') return FALSE;

                while (p && *p && *p!='"')
                    p++;

                if (*p != '"') return FALSE;
                // set
                params->title = c + 5;
                params->titleLength = p - c - 5;
                c = p - 3;
            }
            // invalid
            else return FALSE;

            // skip
            c += 4;
            while (c && *c && *c!=',' && *c!=':')
                c++;
            if (*c == ',') c++;
        }

        if (!*c) return FALSE;
        fileName = c + 1;
    }

    params->file = fileName;

    // check for URLs
    if (open)
    {
        saveName[0] = 0;
        is_tempfile = 0;

	    if (!_strnicmp(fileName, "http://", 7) || !_strnicmp(fileName, "https://", 8) ||
            !_strnicmp(fileName, "ftp://", 6))                // FTP is now currently supported, but still...
	    {
            typedef int (__cdecl *HttpRetrieveFile)(HWND hwnd, const char *url, const char *file, const char *dlgtitle);

		    HttpRetrieveFile fileGetter;
		    int t = SendMessage(mikmod.hMainWindow,WM_USER,0,IPC_GETHTTPGETTER);
		    // try to get httpGetter
		    if (!t || t==1)
		    {	
	            MessageBox(mikmod.hMainWindow,
						   WASABI_API_LNGSTRING(IDS_URLS_ONLY_SUPPORTED_IN_2_10_PLUS),
						   ERROR_TITLE, MB_ICONERROR);
	            return FALSE;	
		    }
		    
		    fileGetter = (HttpRetrieveFile)t;
            // save stream if required
		    if (config_savestr)
		    {	
				OPENFILENAME l = {0};
			    lstrcpyn(saveName, GetFileName(fileName), MAX_PATH);
			    l.lStructSize       = sizeof(l);
			    l.hwndOwner         = mikmod.hMainWindow;
			    l.hInstance         = NULL;
			    l.lpstrFilter       = BuildFilterString();
			    l.lpstrCustomFilter = NULL;
			    l.nMaxCustFilter    = 0;
			    l.nFilterIndex      = 0;
			    l.lpstrFile         = saveName;
			    l.nMaxFile          = sizeof(saveName);
			    l.lpstrFileTitle    = 0;;
			    l.nMaxFileTitle     = 0;
			    l.lpstrInitialDir   = NULL;
			    l.lpstrTitle        = WASABI_API_LNGSTRING(IDS_SAVE_MODULE);
			    l.lpstrDefExt       = "mod";
			    l.Flags             = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_OVERWRITEPROMPT; 	        

			    if (!GetSaveFileName(&l)) 
                    saveName[0] = 0;
		    }
		    // generate temp name, if not saving
		    if (!saveName[0])
		    {	
		        char p[MAX_PATH] = {0};

                GetTempPath(sizeof(p), p);
			    GetTempFileName(p, "mod", 0, saveName);
			    is_tempfile = 1;
		    }
		    // get file
		    if (fileGetter(mikmod.hMainWindow, fileName, saveName, WASABI_API_LNGSTRING(IDS_RETRIEVING_MODULE)))
            {
			    is_tempfile = 0;
                saveName[0] = 0;
                return FALSE;
            }
		    params->file = saveName;
	    }
    }
    else
    {
        if (saveName[0] && !_stricmp(fileName, cmdName))
            params->file = saveName;
    }

    return TRUE;
}

static void CleanupTemp()
{
	if (is_tempfile && saveName[0])
	{	
		DeleteFile(saveName);
		is_tempfile = 0;
	}

    saveName[0] = 0;
}

BOOL InitPlayer(UNIMOD *mf, MPLAYER **ps, const PlayParams *params, BOOL quick)
{
    int flags;

    // strip silence
    if (config_playflag & CPLAYFLG_STRIPSILENCE)
        Unimod_StripSilence(mf, SILENCE_THRESHOLD);

    // set flags
	flags = PF_TIMESEEK;
    if (params->flags & PPF_CONT_LOOP) flags |= PF_CONT_LOOP;

    // init player
    if (quick)
        *ps = Player_Create(mf, flags);
    else *ps = Player_InitSong(mf, NULL, flags, config_voices);

    if (!*ps) return FALSE;

    // position seek
    if (params->start && params->startUnit==SU_POSITION)
        Player_SetStartPosition(*ps, params->start);

    // looping
    Player_SetLoopStatus(*ps, params->flags & PPF_LOOPALL, params->loops);

    if (quick || config_playflag&CPLAYFLG_SEEKBYORDERS)
        Player_PredictSongLength(*ps);
    else
    {
        // time calculation & seeking-lookups creation
	    Player_BuildQuickLookups(*ps);

        // fade (needs results of Player_BuildQuickLookups)
	    if (config_playflag & CPLAYFLG_FADEOUT)
            Player_VolumeFadeEx(*ps, MP_VOLUME_CUR, 0, config_fadeout, MP_SEEK_END, config_fadeout);
    }

    // remember song length
	mf->songlen = (*ps)->songlen;

    return TRUE;
}

static UNIMOD *GetModuleInfo(const PlayParams *params)
{
    UNIMOD  *m = mf;

    // check against the current one
    if (!m || _stricmp(cmdName, params->cmd))                // check the whole string, not just file name
    {
        MPLAYER *ps;
		MMSTREAM * fp;

        // load module
        mm_clearerror();
		fp = _mm_fopen_rf(params->file);
		if (!fp) return NULL;
        m  = Unimod_LoadInfo_FP(params->file,fp);
		_mm_fclose(fp);
        if (!m) return NULL;

        // get info and clean up
        if (!InitPlayer(m, &ps, params, TRUE))
        {
            Unimod_Free(m);
            return NULL;
        }
        Player_Free(ps);
    }

    return m;
}

static int __cdecl isourfile(const char *fn)
{
    return !_strnicmp(fn, "mod://", 6);
}

// =====================================================================================
//  helpers
// =====================================================================================

static UNIMOD *FindInfoBox(const char *fileName, HWND *hwnd)
{
    INFOBOX  *cruise;

    for (cruise=infobox_list; cruise; cruise=cruise->next)
        if (!_stricmp(cruise->dlg.module->filename, fileName))
        {
            if (hwnd) *hwnd = cruise->hwnd;
            return cruise->dlg.module;
        }

    return NULL;
}

static BOOL FindInfoBoxPtr(const UNIMOD *mf)
{
    INFOBOX  *cruise;

    for (cruise=infobox_list; cruise; cruise=cruise->next)
        if (cruise->dlg.module == mf)
            return TRUE;

    return FALSE;
}

// =====================================================================================
static int __cdecl play(const char *fileName)
// =====================================================================================
{
    PlayParams params;
    uint       md_mode = 0;

    // parse parameters
    if (!GetPlayParams(fileName, TRUE, &params))
        return 1;

    // save strings locally
    lstrcpyn(cmdName, params.cmd, 2048);
    if (params.titleLength)
        lstrcpyn(songTitle, params.title, min(params.titleLength+1, sizeof(songTitle)));
    else songTitle[0] = 0;

    // save current values
    currParams = params;
    currParams.cmd = params.cmd;
    currParams.title = songTitle;

    // Initialize MDRVER
    // -----------------

	if (config_interp & 1) md_mode |= DMODE_INTERP;
	if (config_interp & 2) md_mode |= DMODE_NOCLICK;
	if (config_interp & 4) md_mode |= DMODE_FIR;
    md_mode |= GetSampleSizeFlag();
    if (AllowSurround())   md_mode |= DMODE_SURROUND;
    if (config_panrev)     md_mode |= DMODE_REVERSE;
    if (config_resonance)  md_mode |= DMODE_RESONANCE;

    md = Mikmod_Init(config_srate, 1000, NULL, GetNumChannels()==1 ? MD_MONO : MD_STEREO, config_cpu, md_mode, &drv_amp);
	if (!md)
    {
        CleanupTemp();
        return 1;
    }

    md->pansep = config_pansep;

    // Register non-interpolation mixers
    // ---------------------------------
    // if the user has disabled interpolation...

    if(!(config_interp & 1))
    {
        VC_RegisterMixer(md->device.vc, &RF_M8_MONO);
        VC_RegisterMixer(md->device.vc, &RF_M16_MONO);
        VC_RegisterMixer(md->device.vc, &RF_M8_STEREO);
        VC_RegisterMixer(md->device.vc, &RF_M16_STEREO);

        VC_RegisterMixer(md->device.vc, &M8_MONO);
        VC_RegisterMixer(md->device.vc, &M16_MONO);
        VC_RegisterMixer(md->device.vc, &M8_STEREO);
        VC_RegisterMixer(md->device.vc, &M16_STEREO);
    }
	else if (config_interp&4)
	{
/*
		VC_RegisterMixerHack(md->device.vc, &M16_MONO_CUBIC);
		VC_RegisterMixerHack(md->device.vc, &M16_STEREO_CUBIC);
		VC_RegisterMixerHack(md->device.vc, &M8_MONO_CUBIC);
		VC_RegisterMixerHack(md->device.vc, &M8_STEREO_CUBIC);
*/
		VC_RegisterMixerHack(md->device.vc, &M16_MONO_FIR);
		VC_RegisterMixerHack(md->device.vc, &M16_STEREO_FIR);
		VC_RegisterMixerHack(md->device.vc, &M8_MONO_FIR);
		VC_RegisterMixerHack(md->device.vc, &M8_STEREO_FIR);
	}

    // LOADING THE SONG
    // ----------------
    // Check through the list of active info boxes for a matching filename.  If found,
    // then we use the already-loaded module information instead!

    {
        HWND hwnd;

        if ((mf=FindInfoBox(params.file, &hwnd)) != NULL)
        {
            MMSTREAM  *smpfp;

            // prepare for reloading
            info_killseeker(hwnd);

            // reload samples
            smpfp = _mm_fopen_rf(params.file);
            Unimod_LoadSamples(mf, md, smpfp);
            _mm_fclose(smpfp);
        }
        // not already loaded
        else
        {
            MMSTREAM  *fp;
            fp = _mm_fopen_rf(params.file);
			if (!fp)
			{
                Mikmod_Exit(md);
                CleanupTemp();
				return -1;
			}
//MMEXPORT UNIMOD  *Unimod_LoadFP(MDRIVER *md, MMSTREAM *modfp, MMSTREAM *smpfp, int mode);
//MMEXPORT UNIMOD  *Unimod_Load(MDRIVER *md, const CHAR *filename);

            mf=Unimod_Load_FP(md, params.file,fp);
			_mm_fclose(fp);
			if (mf==NULL)
            {				
                Mikmod_Exit(md);
                CleanupTemp();
                return -1;
            }
        }
    }

    // file name is stored in module now
    if (!saveName[0])
        params.file = mf->filename;

    // init player
    if (!InitPlayer(mf, &mp, &params, FALSE))
    {
        CleanupTemp();
        return -1;
    }

    Player_Start(mp);

    // set start time
    seek_needed = -1;
    decode_pos  = 0;
    if (params.start && params.startUnit==SU_TIME)
        setoutputtime(params.start*1000);

    // init output & info
    mikmod.outMod->SetVolume(-666);
    mikmod.SetInfo(MulDiv(mf->filesize, 8,  mf->songlen), config_srate/1000, GetNumChannels(), 1);

    // init decoding thread
    {
        DWORD threadid;

        killDecodeThread = 0;
        paused           = 0;
        thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)decodeThread, NULL, 0, &threadid);
        set_priority();
    }

    return 0; 
}

// =====================================================================================
static void __cdecl stop(void)
// =====================================================================================
{
	if (thread_handle != INVALID_HANDLE_VALUE)
    {
        killDecodeThread = 1;
        if (WaitForSingleObject(thread_handle, 2000) == WAIT_TIMEOUT)
        {
            MessageBox(mikmod.hMainWindow,
					   WASABI_API_LNGSTRING(IDS_ERROR_KILLING_DECODING_THREAD),
					   ERROR_TITLE, MB_ICONWARNING);
            TerminateThread(thread_handle, 0);
        }

        CloseHandle(thread_handle);
        thread_handle = INVALID_HANDLE_VALUE;
        CleanupTemp();
    }

    Player_Free(mp);
    mp = NULL;

    // We need to see if mf is in use.  If so, then we can't unload it.
    // Bute we *do* have to unload its samples, because those are not needed.

    if (FindInfoBoxPtr(mf))
        Unimod_UnloadSamples(mf);
    else Unimod_Free(mf);
    
    mf = NULL;

    Mikmod_Exit(md); md = NULL;
    mikmod.SAVSADeInit();
}

// =====================================================================================
//  pausing stuff
// =====================================================================================

static void __cdecl pause(void)    { paused=1; mikmod.outMod->Pause(1); }
static void __cdecl unpause(void)  { paused=0; mikmod.outMod->Pause(0); }
static int  __cdecl ispaused(void) { return paused; }

// =====================================================================================
//  seeking/timing related stuff
// =====================================================================================

static int __cdecl getlength(void)
{
	if (mp)
    {
		if (!(config_playflag & CPLAYFLG_SEEKBYORDERS))
            return mp->songlen;
		else return mf->numpos * 1000;
    }
	else return 0;
}

static int __cdecl getoutputtime(void)
{
	if (!(config_playflag & CPLAYFLG_SEEKBYORDERS))
        return decode_pos/64 + (mikmod.outMod->GetOutputTime() - mikmod.outMod->GetWrittenTime());
	else return mp ? mp->state.sngpos * 1000 : 0;
}

static void __cdecl setoutputtime(int time_in_ms)
{
    seek_needed = time_in_ms;
}

// =====================================================================================
static int __cdecl infobox(const char *fileName, HWND hwnd)
// =====================================================================================
{
    PlayParams params;

    // parse params
    if (!GetPlayParams(fileName, FALSE, &params))
        return 1;

    // First we check our array of loaded dialog boxes.  If there are any filename matches,
    // then we just bring that window to the foreground!

    if (FindInfoBox(params.file, &hwnd) != NULL)
    {
        SetForegroundWindow(hwnd);
        return 0;
    }

    infoDlg(hwnd, GetModuleInfo(&params), TRUE, TRUE);

    return 0;
}

/*extern "C" __declspec(dllexport) int winampGetExtendedFileInfo(const char *fn, const char *data, char *dest, int destlen)
{
	UNIMOD *m=0;
	PlayParams params;
	const char *ret=0;


	if (!_stricmp(data,"TYPE")) 
	{
		dest[0] = '0';
		dest[1] = 0x00;
		return 1;
	}
	if (!_stricmp(data,"FAMILY"))
	{
		LPCTSTR e;
		e = PathFindExtension(fn);
		if (L'.' != *e) return 0;
		e++;
		return GetTypeInfo(e, dest, destlen);
	}
	
	if (!GetPlayParams(fn, FALSE, &params))
		return 0;

	m=GetModuleInfo(&params);
	if (!m)
		return 0;
	
		if (!_stricmp(data,"TITLE"))
    {
        if (!params.titleLength || params.flags&PPF_ADD_TITLE)
					ret = m->songname;
				else
					ret=params.title;
    }
		else if (!_stricmp(data,"PART"))
		{
			if (params.titleLength && params.flags&PPF_ADD_TITLE)
				ret=params.title;
		}
	else if (!_stricmp(data,"ARTIST") )
		ret=m->composer;
	else if (!_stricmp(data,"COMPOSER"))
        ret=m->composer;
	else if (!_stricmp(data,"COMMENT"))
        ret=m->comment;
	else if (!_stricmp(data,"FORMAT") || !_stricmp(data,"MODTYPE"))
        ret=m->modtype;
	else if (!_stricmp(data,"LENGTH"))
	{
		_itoa(m->songlen, dest, 10);
		if (m!=mf) // make sure it's not the currently playing file
			Unimod_Free(m); // in theory this is a race condition
		return 1;
	}
	else
	{
	if (m!=mf) // make sure it's not the currently playing file
			Unimod_Free(m); // in theory this is a race condition
	return 0;
	}

	if (ret)
		lstrcpyn(dest, ret, destlen);
	else
		dest[0]=0;

	if (m!=mf)  // make sure it's not the currently playing file
		Unimod_Free(m); // in theory this is a race condition
	return 1;
}*/

extern "C" __declspec(dllexport) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, int destlen)
{
	UNIMOD *m=0;
	PlayParams params;
	const char *ret=0;

	if (!_stricmp(data,"TYPE")) 
	{
		dest[0] = L'0';
		dest[1] = 0x00;
		return 1;
	}
	if (!_stricmp(data,"FAMILY"))
	{
		LPCWSTR e;
		e = PathFindExtensionW(fn);
		if (L'.' != *e) return 0;
		e++;
		return GetTypeInfo(e, dest, destlen);
	}
	
	if (!GetPlayParams(AutoCharFn(fn), FALSE, &params))
		return 0;

	m=GetModuleInfo(&params);
	if (!m)
		return 0;
	
		if (!_stricmp(data,"TITLE"))
		{
			if (!params.titleLength || params.flags&PPF_ADD_TITLE)
				ret = m->songname;
			else
				ret=params.title;
		}
		else if (!_stricmp(data,"PART"))
		{
			if (params.titleLength && params.flags&PPF_ADD_TITLE)
				ret=params.title;
		}
		else if (!_stricmp(data,"ARTIST") )
			ret=m->composer;
		else if (!_stricmp(data,"COMPOSER"))
			ret=m->composer;
		else if (!_stricmp(data,"COMMENT"))
			ret=m->comment;
		else if (!_stricmp(data,"FORMAT") || !_stricmp(data,"MODTYPE"))
			ret=m->modtype;
		else if (!_stricmp(data,"LENGTH"))
		{
			_itow(m->songlen, dest, 10);
			if (m!=mf) // make sure it's not the currently playing file
				Unimod_Free(m); // in theory this is a race condition
			return 1;
		}
		else
		{
		if (m!=mf) // make sure it's not the currently playing file
				Unimod_Free(m); // in theory this is a race condition
		return 0;
	}

	if (ret)
		lstrcpynW(dest, AutoWide(ret), destlen);
	else
		dest[0]=0;

	if (m!=mf)  // make sure it's not the currently playing file
		Unimod_Free(m); // in theory this is a race condition
	return 1;
}

// =====================================================================================
static void __cdecl getfileinfo(const char *fileName, char *title, int *length_in_ms)
// =====================================================================================
{
    PlayParams params;
    UNIMOD    *m;
    BOOL       unload = FALSE;

    // empty string stands for the current file
    if (fileName!=NULL && *fileName)
    {
        if (!GetPlayParams(fileName, FALSE, &params))
        {
            lstrcpyn(title, fileName, GETFILEINFO_TITLE_LENGTH);
            if (length_in_ms)
                *length_in_ms = -1;
            return;
        }
    }
    else 
			params = currParams;

    // module loaded
   if ((m=FindInfoBox(params.file, NULL))!=NULL || (unload=1, m=GetModuleInfo(&params))!=NULL)
	{
		if (title)
		{
		if (!params.titleLength  || params.flags&PPF_ADD_TITLE)
			lstrcpyn(title, m->songname, GETFILEINFO_TITLE_LENGTH);
		else
			lstrcpyn(title, params.title, GETFILEINFO_TITLE_LENGTH);
		}
        // set playing time
        if (length_in_ms)
            *length_in_ms = m->songlen;

        // clean up
		if (unload && m!=mf)
            Unimod_Free(m);
	}
    // invalid module or smth else
    else 
	{   
        lstrcpyn(title, GetFileName(params.file), GETFILEINFO_TITLE_LENGTH);
        if (length_in_ms)
            *length_in_ms = -1;
	}
}

// =====================================================================================
//  misc stuff
// =====================================================================================

static void __cdecl setvolume(int volume) { mikmod.outMod->SetVolume(volume); }
static void __cdecl setpan(int pan)       { mikmod.outMod->SetPan(pan); }
static void __cdecl eq_set(int on, char data[10], int preamp) {}

static CHAR capnstupid[4096];

// =====================================================================================
In_Module mikmod = 
// =====================================================================================
{
	IN_VER_RET,
    "nullsoft(in_mod.dll)",	// need to set this to some form of valid buffer otherwise in_bass crashes (why it's looking at this i don't know!!)
    0,  // hMainWindow
    0,  // hDllInstance
    capnstupid,
    1,  // is_seekable
	1,  // uses_output_plug
    config,
    about,
    init,
    quit,
    getfileinfo,
    infobox,
    isourfile,
    play,
    pause,
    unpause,
    ispaused,
    stop,

    getlength,
    getoutputtime,
    setoutputtime,

    setvolume,
    setpan,

	0,0,0,0,0,0,0,0,0,  // vis stuff

    0,0,                // dsp shit

    eq_set,

    NULL,               // setinfo
	NULL                // outmod
};

// =====================================================================================
extern "C" __declspec(dllexport) In_Module *__cdecl winampGetInModule2()
// input module getter. the only thing exported from here.
// =====================================================================================
{
    return &mikmod;
}


// =====================================================================================
static DWORD WINAPI decodeThread(void *unused)
// =====================================================================================
{
    int has_flushed = 0;
    
    while (!killDecodeThread) 
    {
        if (seek_needed >= 0)
        {
            int ms = seek_needed;
            seek_needed = -1;

            if (!(config_playflag & CPLAYFLG_SEEKBYORDERS))
            {
                Player_SetPosTime(mp, ms);
			    decode_pos = ms * 64;
            }
            else Player_SetPosition(mp, ms/1000, TRUE);

            mikmod.outMod->Flush(ms);
            if (paused) mikmod.outMod->Pause(1);
        }
        
		if (!Player_Active(mp))
        {
            // check for infinite looping
            // infinite looping is done manually (only here). we check if
            // it was requested and if the loop is required (song ended
            // with loop or unconditional looping is on)
            if (mp->loopcount!=-1 || !(mp->flags&PF_LOOP || mp->state.looping<0))
            {
                if (!has_flushed)
                {
                    has_flushed = 1;
                    mikmod.outMod->Write(NULL, 0);          // write all samples into buffer queue
                }
                if (!mikmod.outMod->IsPlaying()) 
                {
                    PostMessage(mikmod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
                    return 0;
                }
                else mikmod.outMod->CanWrite();             // make sure plug-in can do any extra processing needed
		        Sleep(20);
            }
            else
            {
                Player_Restart(mp, TRUE);
		        decode_pos = mp->state.curtime;
            }
        }
        else
        {
            Mikmod_Update(md);
            Sleep(8);
        }
    }

    return 0;
}


// =====================================================================================
void set_priority(void)     // also used in config.c
// =====================================================================================
{
    if (thread_handle != INVALID_HANDLE_VALUE)
        SetThreadPriority(thread_handle, GetThreadPriorityConfig());
}


BOOL WINAPI DllMain(HANDLE h, DWORD r, void *z)
{
    if (r == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls((HMODULE)h);
	}
	return 1;
}