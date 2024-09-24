/*
    Mikmod for Winamp

    By Jake Stine and Justin Frankel.  Really.

    Done in 1998, 1999.  The millenium cometh!
    Done more in 2000.   The millenium passeth by!
    Redone lots in 2001. The REAL millenium cometh!  haha.
*/

#include "api.h"
extern "C" {
#include "main.h"
}
#include <shlobj.h>
#include <commctrl.h>
#include "../../winamp/wa_ipc.h"
#include <strsafe.h>
#include "../nu/AutoWide.h"

#define TabCtrl_InsertItemW(hwnd, iItem, pitem)   \
    (int)SNDMSG((hwnd), TCM_INSERTITEMW, (WPARAM)(int)(iItem), (LPARAM)(const TC_ITEMW *)(pitem))

#define C_PAGES                4

#define CFG_UNCHANGED       (-1)

#define EFFECT_8XX      (1ul<<0)
#define EFFECT_ZXX      (1ul<<1)

#define CPUTBL_COUNT          23
#define MIXTBL_COUNT           7
#define VOICETBL_COUNT         8
#define DEF_SRATE          44100

 
typedef struct tag_cdlghdr
{
    HWND  hwndTab;       // tab control 
    HWND  hwndDisplay;   // current child dialog box 
	int   left,top;
    HWND  apRes[C_PAGES];

} CFG_DLGHDR; 


// Winamp Stuff
UBYTE    config_savestr  = 0;                   // save stream to disk

// Output Settings
UBYTE    config_nch = 2;
UBYTE    config_cpu;

// Player Settings...
// ------------------

int      config_loopcount = 0;    // Auto-looping count, if the song has a natural loop.
uint     config_playflag  = 0;    // See CPLAYFLG_* defines above for flags
int      config_pansep    = 128;  // master panning separation (0 == mono, 128 == stereo, 512 = way-separate)
UBYTE    config_resonance = 1;
int      config_fadeout   = 1000; // fadeout when the song is ending last loop (in ms)

// Mixer Settings...
// -----------------

UBYTE    config_panrev    = 0,    // Reverse panning (left<->right)
         config_interp    = 3;    // interpolation (bit 0) / noclick (bit 1) / cubic (bit 2)
uint     config_srate     = DEF_SRATE;
uint     config_voices    = 96;   // maximum voices player can use.


// Local Crud...
// -------------

static const char   *INI_FILE;
static const char app_name[] = "Nullsoft Module Decoder";

static int    l_quality,   l_srate,    // l_quality indexes mixertbl. l_srate is the actual number.
              l_voices;
static BOOL   l_useres,    l_nch,
              l_iflags;
int config_tsel = 0;


static uint voicetable[VOICETBL_COUNT] =  {  24, 32, 48, 64, 96, 128, 256, 512 };
static uint mixertbl[MIXTBL_COUNT] = 
{ 
    48000,
    44100,
    33075,
    22050,
    16000,
    11025,
    8000,
};


// Define Extended Loader Details/Options
// --------------------------------------

typedef struct _MLCONF
{
    CHAR     *cfgname;         // configuration name prefix.
	INT      desc_id;          // long-style multi-line description!
    CHAR     *exts;
    MLOADER  *loader;          // mikmod loader to register
    BOOL     enabled;
    uint     defpan,           // default panning separation \  range 0 - 128
             pan;              // current panning separation /  (mono to stereo)
    uint     optavail,         // available advanced effects options
             optset;           // current settings for those options.
} MLCONF;


static MLCONF c_mod[] = 
{
    "mod",
	IDS_PROTRACKER_AND_CLONES,
    "mod;mdz;nst",
    &load_mod,
    TRUE,

    0,0,
    EFFECT_8XX, 0,

    "m15",
	IDS_OLD_SKOOL_AMIGA_MODULES,
    "mod;mdz",
    &load_m15,
    TRUE,

    0,0,
    EFFECT_8XX, 0,

    "stm",
	IDS_SCREAM_TRACKER_2XX,
    "stm;stz",
    &load_stm,
    TRUE,

    0,0,
    EFFECT_8XX | EFFECT_ZXX,  0,

    "st3",
	IDS_SCREAM_TRACKER_3XX,
    "s3m;s3z",
    &load_s3m,
    TRUE,

    32,32,
    EFFECT_8XX | EFFECT_ZXX,  0,


    "it",
	IDS_IMPULSE_TRACKER,
    "it;itz",
    &load_it,
    TRUE,

    0,0,
    EFFECT_ZXX,  0,

    "ft2",
	IDS_FASTTRACKER_2XX,
    "xm;xmz",
    &load_xm,
    TRUE,

    0,0,
    0, 0,

    "mtm",
	IDS_MULTITRACKER,
    "mtm",
    &load_mtm,
    TRUE,

    0,0,
    EFFECT_8XX,  0,

    "ult",
	IDS_ULTRA_TRACKER,
    "ult",
    &load_ult,
    TRUE,

    0,0,
    EFFECT_8XX,  0,

    "669",
	IDS_COMPOSER_669,
    "669",
    &load_669,
    TRUE,

    0,0,
    0, 0,

    "far",
	IDS_FARANDOLE_COMPOSER,
    "far",
    &load_far,
    TRUE,

    0,0,
    0, 0,

    "amf",
	IDS_DIGITAL_SOUND_AND_MUSIC_INTERFACE,
    "amf",
    &load_amf,
    TRUE,

    0,0,
    0, 0,

    "okt",
	IDS_AMIGA_OKTALYZER,
    "okt",
    &load_okt,
    TRUE,

    0, 0,
    0, 0,

    "ptm",
	IDS_POLYTRACKER,
    "ptm",
    &load_ptm,
    TRUE,

    0, 0,
    EFFECT_8XX, 0,
};

#define C_NUMLOADERS    (sizeof(c_mod)/sizeof(c_mod[0]))

static MLCONF l_mod[C_NUMLOADERS];                  // local copy, for cancelability


// =====================================================================================
    static int _r_i(char *name, int def)
// =====================================================================================
{
    name += 7;
    return GetPrivateProfileInt(app_name,name,def,INI_FILE);
}
#define RI(x) (( x ) = _r_i(#x,( x )))


// =====================================================================================
    static void _w_i(char *name, int d)
// =====================================================================================
{
    char str[120] = {0};
    StringCchPrintf(str,100,"%d",d);
    name += 7;
    WritePrivateProfileString(app_name,name,str,INI_FILE);
}
#define WI(x) _w_i(#x,( x ))


// =====================================================================================
    static void config_buildbindings(CHAR *datext)
// =====================================================================================
// Creates the binding string.
{
#define SEP_CHAR ';'

    uint i;
    char *base = datext;

    base[0] = 0;

    for (i=0; i<C_NUMLOADERS; i++)
        if (c_mod[i].enabled)
        {
            char *ext = c_mod[i].exts;

            for (;;)
            {
                char temp[64] = {0};
                int len;
                // find next
                char *next = strchr(ext, SEP_CHAR), *s;

                if (next)
                    len = next - ext;
                else len = strlen(ext);
                memcpy(temp, ext, len);
                temp[len++] = SEP_CHAR;
                temp[len] = 0;
                // check for duplicate exts
                s = strstr(base, temp);
                if (!s || (s!=base && *(s-1)!=SEP_CHAR))
                {
                    memcpy(datext, temp, len);
                    datext += len;
                    *datext = 0;                            // prevent using old content
                }
                // advance
                if (!next)
                    break;
                ext = next + 1;
            }
        }

    if (datext > base)
    {
        *(datext-1) = 0;
		StringCchCopy(datext,4096,WASABI_API_LNGSTRING(IDS_MUSIC_MODULES));
    }
}


// =====================================================================================
    static void config_init(void)
// =====================================================================================
{
	INI_FILE = (const char *)SendMessage(mikmod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE);
}


// =====================================================================================
    void config_read(void)
// =====================================================================================
{
    uint   t;

    config_init();
	

	RI(config_savestr);

    RI(config_nch);
    RI(config_srate);
	RI(config_interp);
	
	RI(config_voices);

	RI(config_loopcount);
	RI(config_playflag);
	RI(config_resonance);
	RI(config_fadeout);

	RI(config_pansep);
	RI(config_panrev);

    RI(config_info_x);
    RI(config_info_y);
    RI(config_track);
	RI(config_tsel);

    config_cpu = _mm_cpudetect();

    // Load settings for each of the individual loaders
    // ------------------------------------------------

    for(t=0; t<C_NUMLOADERS; t++)
    {
        CHAR   stmp[72] = {0};

        StringCchPrintf(stmp,72,"%s%s",c_mod[t].cfgname,"enabled");
        c_mod[t].enabled = GetPrivateProfileInt(app_name,stmp,c_mod[t].enabled,INI_FILE);

        StringCchPrintf(stmp,72,"%s%s",c_mod[t].cfgname,"panning");
        c_mod[t].pan = GetPrivateProfileInt(app_name, stmp, c_mod[t].defpan, INI_FILE);
        c_mod[t].pan = _mm_boundscheck(c_mod[t].pan, 0, 128);

        StringCchPrintf(stmp,72,"%s%s",c_mod[t].cfgname,"effects");
        c_mod[t].optset = GetPrivateProfileInt(app_name,stmp,c_mod[t].optset,INI_FILE);

        // configure the loaders
        c_mod[t].loader->enabled  = c_mod[t].enabled;
        c_mod[t].loader->defpan   = c_mod[t].pan;
        c_mod[t].loader->nopaneff = (c_mod[t].optset | EFFECT_8XX) ? FALSE : TRUE;
        c_mod[t].loader->noreseff = (c_mod[t].optset | EFFECT_ZXX) ? FALSE : TRUE;
    }

    config_buildbindings(mikmod.FileExtensions);

    // Bounds checking!
    // ----------------
    // This is important to ensure stability of the product in case some
    // doof goes and starts hacking the ini values carelessly - or if some sort
    // of version conflict or corruption causes skewed readings.

    config_pansep    = _mm_boundscheck(config_pansep, 0, 512);
    config_voices    = _mm_boundscheck(config_voices, 2, 1024);

    config_fadeout   = _mm_boundscheck(config_fadeout, 0, 1000l*1000l);
    config_loopcount = _mm_boundscheck(config_loopcount, -1, 63);
}


// =====================================================================================
    void config_write(void)
// =====================================================================================
{
	uint t = 0;

	WI(config_savestr);

    WI(config_nch);
    WI(config_srate);
	WI(config_interp);
	WI(config_voices);

	WI(config_loopcount);
	WI(config_playflag);
	WI(config_resonance);
	WI(config_fadeout);

	WI(config_pansep);
	WI(config_panrev);

    WI(config_info_x);
    WI(config_info_y);
    WI(config_track);
	WI(config_tsel);

    // Save settings for each of the individual loaders
    // ------------------------------------------------

    for(; t < C_NUMLOADERS; t++)
    {
        CHAR stmp[72] = {0}, sint[12] = {0};

        StringCchPrintf(stmp,72,"%s%s",c_mod[t].cfgname,"enabled");
        StringCchPrintf(sint,12,"%d",c_mod[t].enabled);
        WritePrivateProfileString(app_name,stmp,sint,INI_FILE);

        StringCchPrintf(stmp,72,"%s%s",c_mod[t].cfgname,"panning");
        StringCchPrintf(sint,12,"%d",c_mod[t].pan);
        WritePrivateProfileString(app_name,stmp,sint,INI_FILE);

        StringCchPrintf(stmp,72,"%s%s",c_mod[t].cfgname,"effects");
        StringCchPrintf(sint,12,"%d",c_mod[t].optset);
        WritePrivateProfileString(app_name,stmp,sint,INI_FILE);

        // configure the loaders
        c_mod[t].loader->enabled = c_mod[t].enabled;
        c_mod[t].loader->nopaneff = (c_mod[t].optset | EFFECT_8XX) ? FALSE : TRUE;
        c_mod[t].loader->noreseff = (c_mod[t].optset | EFFECT_ZXX) ? FALSE : TRUE;
    }

    config_buildbindings(mikmod.FileExtensions);
}


static BOOL CALLBACK prefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);


// =====================================================================================
    void __cdecl config(HWND hwndParent)
// =====================================================================================
{
	WASABI_API_DIALOGBOXW(IDD_PREFS,hwndParent,prefsProc);
    config_write();
}


static BOOL CALLBACK tabProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK mixerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK loaderProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void OnSelChanged(HWND hwndDlg, int initonly);

// =====================================================================================
    static void prefsTabInit(HWND hwndDlg, CFG_DLGHDR *pHdr) 
// =====================================================================================
{ 
    DWORD   dwDlgBase = GetDialogBaseUnits(); 
    int     cxMargin = LOWORD(dwDlgBase) / 4,
            cyMargin = HIWORD(dwDlgBase) / 8;
    TC_ITEMW tie;
    int     tabCounter;
    
	tie.mask = TCIF_TEXT | TCIF_IMAGE; 
    tie.iImage = -1; 

    tabCounter = 0;

	SendMessage(mikmod.hMainWindow,WM_WA_IPC,(WPARAM)pHdr->hwndTab,IPC_USE_UXTHEME_FUNC);

	tie.pszText = WASABI_API_LNGSTRINGW(IDS_MIXER); 
	TabCtrl_InsertItemW(pHdr->hwndTab, tabCounter, &tie);
    pHdr->apRes[tabCounter] = WASABI_API_CREATEDIALOGPARAMW(IDD_PREFTAB_MIXER, hwndDlg, mixerProc, IDD_PREFTAB_MIXER);
    SetWindowPos(pHdr->apRes[tabCounter], HWND_TOP, pHdr->left, pHdr->top, 0, 0, SWP_NOSIZE);
	SendMessage(mikmod.hMainWindow,WM_WA_IPC,(WPARAM)pHdr->apRes[tabCounter],IPC_USE_UXTHEME_FUNC);
    ShowWindow(pHdr->apRes[tabCounter++], SW_HIDE);

	tie.pszText = WASABI_API_LNGSTRINGW(IDS_PLAYER);
	TabCtrl_InsertItemW(pHdr->hwndTab, tabCounter, &tie); 
    pHdr->apRes[tabCounter] = WASABI_API_CREATEDIALOGPARAMW(IDD_PREFTAB_DECODER, hwndDlg, tabProc, IDD_PREFTAB_DECODER);
    SetWindowPos(pHdr->apRes[tabCounter], HWND_TOP, pHdr->left, pHdr->top, 0, 0, SWP_NOSIZE);
	SendMessage(mikmod.hMainWindow,WM_WA_IPC,(WPARAM)pHdr->apRes[tabCounter],IPC_USE_UXTHEME_FUNC);
    ShowWindow(pHdr->apRes[tabCounter++], SW_HIDE);

	tie.pszText = WASABI_API_LNGSTRINGW(IDS_LOADERS);
	TabCtrl_InsertItemW(pHdr->hwndTab, tabCounter, &tie); 
    pHdr->apRes[tabCounter] = WASABI_API_CREATEDIALOGPARAMW(IDD_PREFTAB_LOADER, hwndDlg, loaderProc, IDD_PREFTAB_LOADER);
    SetWindowPos(pHdr->apRes[tabCounter], HWND_TOP, pHdr->left, pHdr->top, 0, 0, SWP_NOSIZE);
	SendMessage(mikmod.hMainWindow,WM_WA_IPC,(WPARAM)pHdr->apRes[tabCounter],IPC_USE_UXTHEME_FUNC);
    ShowWindow(pHdr->apRes[tabCounter++], SW_HIDE);

    // Simulate selection of the first item.
    OnSelChanged(hwndDlg,1);
} 


// =====================================================================================
    static void OnSelChanged(HWND hwndDlg, int initonly)
// =====================================================================================
{ 
    CFG_DLGHDR *pHdr = (CFG_DLGHDR *) GetWindowLong(hwndDlg, GWL_USERDATA);
	if(!initonly)
		config_tsel = TabCtrl_GetCurSel(pHdr->hwndTab);
	else
		TabCtrl_SetCurSel(pHdr->hwndTab,config_tsel);

	if(pHdr->hwndDisplay)  ShowWindow(pHdr->hwndDisplay,SW_HIDE);
	ShowWindow(pHdr->apRes[config_tsel],SW_SHOW);
	pHdr->hwndDisplay = pHdr->apRes[config_tsel];

} 


// =====================================================================================
    static void FadeOutSetup(HWND hwndDlg, BOOL enabled)
// =====================================================================================
{
    EnableWindow(GetDlgItem(hwndDlg, IDC_FADEOUT), enabled);
    EnableWindow(GetDlgItem(hwndDlg, IDC_FADESEC), enabled);
}


// =====================================================================================
    static void Stereo_Dependencies(HWND hwndDlg)
// =====================================================================================
// Enable or Disable the options which are dependant on stereo being enabled
{   
    BOOL val = (l_nch==1) ? 0 : 1;
    EnableWindow(GetDlgItem(hwndDlg,OUTMODE_REVERSE), val);
    EnableWindow(GetDlgItem(hwndDlg,OUTMODE_SURROUND),val);
}


// =====================================================================================
    static void SetVoiceList(HWND hwndMisc)
// =====================================================================================
// erg is the current quality mode (indexes mixertbl).
{
    uint     i,k;
    BOOL     picked = FALSE;
    uint     cfgv;

    cfgv = (l_voices==CFG_UNCHANGED) ? config_voices : voicetable[l_voices];

    SendMessage(hwndMisc,CB_RESETCONTENT,0,0);

    for(i=0; i<VOICETBL_COUNT; i++)
    {   CHAR  buf[24] = {0};

        // Find the appropriate megaherz, by doing a pretty wacky little logic snippet
        // which matches up the multipled mhz with the closet legit CPU bracket (as
        // listed in cputable).

        StringCchPrintf(buf,24,"%s%d",(voicetable[i]<100) ? "  " : "", voicetable[i]);

        SendMessage(hwndMisc,CB_ADDSTRING,0,(LPARAM)buf);
        if(!picked && (voicetable[i] >= cfgv))
        {  k = i;  picked = TRUE;  }
    }

    // If picked is false, then set to 96 (default)

    if(!picked) k = 4;

    SendMessage(hwndMisc,CB_SETCURSEL,k,0);
}


// =====================================================================================
    static BOOL cmod_and_the_moo(HWND dagnergit, int *moo, int count, uint flag)
// =====================================================================================
{
    int    l;
    BOOL   oneway, otherway, thisway, thatway;

    oneway = otherway = thisway = thatway = FALSE;

    // Set oneway/otherway true for selected/unselected options.
    // Set thisway/thatway true for avail/nonawail options.

    for(l=0; l<count; l++)
    {   if(l_mod[moo[l]].optavail & flag)
        {   thisway = TRUE;
            if(l_mod[moo[l]].optset & flag) oneway = TRUE; else otherway = TRUE;
        } else thatway = TRUE;
    }

    EnableWindow(dagnergit, thisway);
    SendMessage(dagnergit, BM_SETCHECK, (!thisway || (oneway != otherway)) ? (oneway ? BST_CHECKED : BST_UNCHECKED) : BST_INDETERMINATE, 0);
	return thisway;
}


// =====================================================================================
    static BOOL CALLBACK mixerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
// =====================================================================================
{	

    switch (uMsg)
    {
        // =============================================================================
        case WM_INITDIALOG:
        // =============================================================================
        // Windows dialog box startup message.  This is messaged for each tab form created.
        // Initialize all of the controls on each of those forms!
        {   
            HWND        hwndMisc;
		    CFG_DLGHDR *pHdr = (CFG_DLGHDR *)GetWindowLong(GetParent(hwndDlg), GWL_USERDATA);

            CheckDlgButton(hwndDlg, IDC_SAVESTR, config_savestr ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg,OQ_INTERP,      (config_interp & 1)  ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg,OQ_NOCLICK,     (config_interp & 2)  ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg,OQ_CUBIC,       (config_interp & 4)  ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg,OUTMODE_REVERSE, config_panrev       ? BST_CHECKED : BST_UNCHECKED);

            l_voices  = CFG_UNCHANGED;
            l_srate   = config_srate;
            l_iflags  = config_interp;
            l_nch     = GetNumChannels();
            l_useres  = config_resonance;

            hwndMisc  = GetDlgItem(hwndDlg,OQ_QUALITY);

            {
                uint     erg, i, def=0;
                BOOL     picked = FALSE;

                for(i=0; i<MIXTBL_COUNT; i++)
                {
                    wchar_t  buf[24] = {0};
					StringCchPrintfW(buf,24,L"%d%s", mixertbl[i], mixertbl[i]==DEF_SRATE ? (def=i, WASABI_API_LNGSTRINGW(IDS_DEFAULT)) : L"");
                    SendMessageW(hwndMisc,CB_ADDSTRING,0,(LPARAM)buf);
                    if (!picked && mixertbl[i]<config_srate)
                    {
                        erg = i ? (i-1) : 0;
                        picked = TRUE;
                    }
                }
                // If picked is false, then set to default
                if(!picked) erg = def;
                SendMessage(hwndMisc,CB_SETCURSEL,erg,0);
                l_quality = erg;
            }

            SetVoiceList(GetDlgItem(hwndDlg,IDC_VOICES));
            Stereo_Dependencies(hwndDlg);

			return TRUE;
        }
        break;

        // =============================================================================
        case WM_COMMAND:
        // =============================================================================
        // Process commands and notification messages recieved from our child controls.

            switch(LOWORD(wParam))
            {   case IDOK:
                {   
   				
                    if(l_voices  != CFG_UNCHANGED) config_voices = voicetable[l_voices];

                    config_srate     = l_srate;
                    config_interp    = l_iflags;
                    config_resonance = l_useres;

                    config_panrev   = IsDlgButtonChecked(hwndDlg,OUTMODE_REVERSE)  ? 1 : 0;

	    			config_voices   = _mm_boundscheck(config_voices, 2,1024);

	            			config_savestr  = IsDlgButtonChecked(hwndDlg,IDC_SAVESTR)   ? 1 : 0;
                }
                break;

                // Output Quality / Mixing Performance
                // -----------------------------------
                // From here on down we handle the messages from those controls which affect
                // the performance (and therefore the cpu requirements) of the module decoder.
                // Each one assigns values into a temp variable (l_*) so that if the user
                // cancels the changes are not saved.


                case OQ_INTERP:
				case OQ_CUBIC:
                case OQ_NOCLICK:
                    if(HIWORD(wParam) == BN_CLICKED)
                    {   l_iflags    = IsDlgButtonChecked(hwndDlg,OQ_INTERP)    ? 1 : 0;
                        l_iflags   |= IsDlgButtonChecked(hwndDlg,OQ_NOCLICK)   ? 2 : 0;
						l_iflags   |= IsDlgButtonChecked(hwndDlg,OQ_CUBIC)   ? 4 : 0;
                        SetVoiceList(GetDlgItem(hwndDlg,IDC_VOICES));
                    }
                break;

                case IDC_VOICES:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {   int taxi = SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
                        l_voices = taxi;
                    }
                break;

                case OQ_QUALITY:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {   int taxi = SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
                        l_quality = taxi;
                        l_srate   = mixertbl[l_quality];
                        SetVoiceList(GetDlgItem(hwndDlg,IDC_VOICES));
                    }
                break;
            }
		break;
    }
    return 0;
}


// =====================================================================================
    static BOOL CALLBACK loaderProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
// =====================================================================================
// This is the callback procedure used by each of the three forms under the tab control 
// on the Preferences dialog box.  It handles all the messages for all of the controls
// within those dialog boxes.
{	
    switch (uMsg)
    {
        // =============================================================================
        case WM_INITDIALOG:
        // =============================================================================
        // Windows dialog box startup message.  This is messaged for each tab form created.
        // Initialize all of the controls on each of those forms!
        {   
            HWND        hwndMisc;
		    CFG_DLGHDR *pHdr = (CFG_DLGHDR *)GetWindowLong(GetParent(hwndDlg), GWL_USERDATA);

            uint   i;

            // Set the range on the panning slider (0 to 16)
            
            hwndMisc = GetDlgItem(hwndDlg,IDLDR_PANPOS);
            SendMessage(hwndMisc,TBM_SETRANGEMAX,0,16);
            SendMessage(hwndMisc,TBM_SETRANGEMIN,0,0);
            SendMessage(hwndMisc,TBM_SETPOS, 1, 16);

            // Build our list of loaders in the loader box
            // -------------------------------------------
            // TODO: eventually I would like to display little checkmarks (or llamas or
            // something) in this list to indicate (at a glance) which are enabled and
            // which are not.
            
            hwndMisc = GetDlgItem(hwndDlg,IDLDR_LIST);

            for (i=0; i<C_NUMLOADERS; i++)
            {
                l_mod[i] = c_mod[i];
				SendMessageW(hwndMisc, LB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(c_mod[i].loader->DescStrID));
            }

            SendMessage(hwndMisc, LB_SETCURSEL, 0, 0);
            loaderProc(hwndDlg, WM_COMMAND, (WPARAM)((LBN_SELCHANGE << 16) + IDLDR_LIST), (LPARAM)hwndMisc);

			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(hwndMisc, TRUE);
        }
        return TRUE;

		case WM_DESTROY:
		{
			HWND hwndMisc = GetDlgItem(hwndDlg, IDLDR_LIST);
			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(hwndMisc, FALSE);
		}
		break;

        // =============================================================================
        case WM_COMMAND:
        // =============================================================================
        // Process commands and notification messages recieved from our child controls.

            switch(LOWORD(wParam))
            {
                case IDOK:
                {   uint  i;
                    for (i=0; i<C_NUMLOADERS; i++)
                    {
                        c_mod[i] = l_mod[i];
                        c_mod[i].loader->defpan = c_mod[i].pan;
                    }
                }
                break;

                case IDLDR_LIST:
                    
                    // The Loader Box Update Balloofie
                    // -------------------------------
                    // Updates the various controls on the 'loader tab' dialog box.  Involves
                    // enabling/disabling advanced-effects boxes, checking the Enabled box, and
                    // setting the panning position.  Also: extra care is taken to allow proper
                    // and intuitive support for multiple selections!

                    if(HIWORD(wParam) == LBN_SELCHANGE)
                    {   int    moo[C_NUMLOADERS], count,l;
                        BOOL   oneway, otherway, opt1, opt2;
                        HWND   beiownd;

                        // Fetch the array of selected items!

                        count = SendMessage((HWND)lParam, LB_GETSELITEMS, C_NUMLOADERS, (LPARAM)moo);
                        if(!count || (count == LB_ERR))
                        {
                            // Something's not right, so just disable all the controls.

                            SetWindowTextW(GetDlgItem(hwndDlg, IDLDR_DESCRIPTION), WASABI_API_LNGSTRINGW(IDS_SELECT_ANY_LOADER));
                            EnableWindow(beiownd = GetDlgItem(hwndDlg, IDLDR_PANPOS), FALSE);
                            EnableWindow(beiownd = GetDlgItem(hwndDlg, IDLDR_ENABLED), FALSE);
                            SendMessage(hwndDlg, BM_GETCHECK, BST_UNCHECKED,0);
                            EnableWindow(beiownd = GetDlgItem(hwndDlg, IDLDR_EFFOPT1), FALSE);
                            SendMessage(hwndDlg, BM_GETCHECK, BST_UNCHECKED,0);
                            EnableWindow(beiownd = GetDlgItem(hwndDlg, IDLDR_EFFOPT2), FALSE);
                            SendMessage(hwndDlg, BM_GETCHECK, BST_UNCHECKED,0);
							EnableWindow(beiownd = GetDlgItem(hwndDlg, IDC_DEFPAN), FALSE);

                            break;
                        }

						SetWindowTextW(GetDlgItem(hwndDlg, IDLDR_DESCRIPTION),
									   WASABI_API_LNGSTRINGW(
									   (count==1) ? l_mod[moo[0]].desc_id : IDS_MULTIPLE_ITEMS_SELECTED));

                        // Enabled Box : First of Many
                        // ---------------------------

                        oneway = otherway = FALSE;
                        for(l=0; l<count; l++)
                            if(l_mod[moo[l]].enabled) oneway = TRUE; else otherway = TRUE;

                        EnableWindow(beiownd = GetDlgItem(hwndDlg, IDLDR_ENABLED), TRUE);
                        SendMessage(beiownd, BM_SETCHECK, (oneway != otherway) ? (oneway ? BST_CHECKED : BST_UNCHECKED) : BST_INDETERMINATE, 0);


                        // The PanningPos : Second in Command
                        // ----------------------------------
                        // Only set it if we have a single format selected, otherwise... erm..
                        // do something (to be determined!)

                        beiownd = GetDlgItem(hwndDlg, IDC_DEFPAN);
                        EnableWindow(beiownd, TRUE);
                        beiownd = GetDlgItem(hwndDlg, IDLDR_PANPOS);
                        EnableWindow(beiownd, TRUE);
                        if(count==1)
                            SendMessage(beiownd, TBM_SETPOS, TRUE,  16-((l_mod[moo[0]].pan+1)>>3));


                        // 8xx Panning Disable: Third of Four
                        // Zxx Resonance: All the Duckies are in a Row!
                        // --------------------------------------------

                        opt1 = cmod_and_the_moo(GetDlgItem(hwndDlg, IDLDR_EFFOPT1), moo, count, EFFECT_8XX);
                        opt2 = cmod_and_the_moo(GetDlgItem(hwndDlg, IDLDR_EFFOPT2), moo, count, EFFECT_ZXX);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADV_TEXT_INFO), opt1 || opt2);
                    }
                break;

                case IDLDR_ENABLED:
                case IDC_DEFPAN:
                case IDLDR_EFFOPT1:
                case IDLDR_EFFOPT2:

                    if(HIWORD(wParam) == BN_CLICKED)
                    {   int  moo[C_NUMLOADERS],count;
                        int  res = SendMessage((HWND)lParam,BM_GETCHECK,0,0);
                        
                        count = SendMessage(GetDlgItem(hwndDlg, IDLDR_LIST), LB_GETSELITEMS, C_NUMLOADERS, (LPARAM)moo);


                        switch(res)
                        {
                            case BST_CHECKED:
                                SendMessage((HWND)lParam,BM_SETCHECK,BST_UNCHECKED,0);
                                res = 0;
                            break;

                            case BST_INDETERMINATE:
                            case BST_UNCHECKED:
                                SendMessage((HWND)lParam,BM_SETCHECK,BST_CHECKED,0);
                                res = 1;
                            break;
                        }

                        if (LOWORD(wParam) == IDLDR_ENABLED)
                        {   int   l;
                            for(l=0; l<count; l++) l_mod[moo[l]].enabled = res;
                        }
                        else if (LOWORD(wParam) == IDC_DEFPAN)
                        {
                            int   l;

                            for (l=0; l<count; l++)
                                l_mod[moo[l]].pan = l_mod[moo[l]].defpan;

                            if (count==1)
                                SendMessage(GetDlgItem(hwndDlg, IDLDR_PANPOS), TBM_SETPOS, TRUE,  16-((l_mod[moo[0]].pan+1)>>3));
                        }
                        else
                        {
                            uint  flag = (LOWORD(wParam) == IDLDR_EFFOPT1) ? EFFECT_8XX : EFFECT_ZXX;
                            int   l;

                            for(l=0; l<count; l++)
                            {   if(l_mod[moo[l]].optavail & flag)
                                {   if(res)    
                                        l_mod[moo[l]].optset |= flag;
                                    else
                                        l_mod[moo[l]].optset &= ~flag;
                                }
                            }
                        }
                    }
                break;
            }
        break;

        // =============================================================================
        case WM_HSCROLL:
        // =============================================================================
        // Whitness Stupidness!
        // Microsoft decides it would be this "brilliant move' to make trackbars send 
        // WM_HSCROLL and WM_VSCROLL messages only!  Like, who the hell uses a trackbar
        // as a scroller anyway?  Whatever happened to the 'standard' system of command/
        // notify messages?  Grrr.

        // Oh look, the LOWORD is the command this time, as opposed to notifies, where the
        // HIWORD is the command.  Jesus fucking Christ I'm in loonyland around here.
        
        if(LOWORD(wParam) == TB_THUMBPOSITION)
        {
            int  moo[C_NUMLOADERS],count,l;
            count = SendMessage(GetDlgItem(hwndDlg, IDLDR_LIST), LB_GETSELITEMS, C_NUMLOADERS, (LPARAM)moo);

            for(l=0; l<count; l++)
            {   l_mod[moo[l]].pan = (16 - HIWORD(wParam)) << 3;
                l_mod[moo[l]].pan = _mm_boundscheck(l_mod[moo[l]].pan,0,128);
            }
        }
    }

	const int controls[] = 
	{
		IDC_LOOPS,
		IDC_PANSEP,
		IDLDR_PANPOS,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
	{
		return TRUE;
	}

    return 0;
}


// =====================================================================================
    static void FadeoutSetText(HWND hwndDlg)
// =====================================================================================
{
    CHAR work[32] = {0};
	StringCchPrintf(work, 32, "%.02f",config_fadeout/1000.0f);
    SetDlgItemText(hwndDlg, IDC_FADEOUT, work);
}


// =====================================================================================
    static BOOL CALLBACK tabProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
// =====================================================================================
{	
    switch (uMsg)
    {
        // =============================================================================
        case WM_INITDIALOG:
        // =============================================================================
        // Windows dialog box startup message.  This is messaged for each tab form created.
        // Initialize all of the controls on each of those forms!
        {   
            HWND        hwndMisc;
		    CFG_DLGHDR *pHdr = (CFG_DLGHDR *)GetWindowLong(GetParent(hwndDlg), GWL_USERDATA);
			SetWindowLong(hwndDlg,DWL_USER,lParam);

            switch(lParam)
            {
                case IDD_PREFTAB_DECODER:
                    SendMessage(GetDlgItem(hwndDlg,IDC_FADEOUT), EM_SETLIMITTEXT, 10,0);
                    FadeoutSetText(hwndDlg);

                    hwndMisc = GetDlgItem(hwndDlg,IDC_PANSEP);
                    SendMessage(hwndMisc,TBM_SETRANGEMIN,0,0);
                    SendMessage(hwndMisc,TBM_SETRANGEMAX,0,32);
                    
                    {
                        int  erg  = config_pansep;

                        if (erg <= 128)
                            erg *= 2;
                        else erg = 256 + ((erg - 128) * 256) / 384;
                    
                        SendMessage(hwndMisc,TBM_SETPOS,1, (erg>>4)&31);
                    }

                    hwndMisc = GetDlgItem(hwndDlg, IDC_LOOPS);
                    SendMessage(hwndMisc, TBM_SETRANGEMIN, 0, 0);
                    SendMessage(hwndMisc, TBM_SETRANGEMAX, 0, 64);
                    SendMessage(hwndMisc, TBM_SETTICFREQ, 4, 0);
                    SendMessage(hwndMisc, TBM_SETPOS, 1, config_loopcount>=0 ? config_loopcount : 64);
                    SendMessage(hwndDlg, WM_HSCROLL, 0, (LPARAM)hwndMisc);
                    
                    CheckDlgButton(hwndDlg,IDC_LOOPALL,     (config_playflag & CPLAYFLG_LOOPALL)      ? BST_CHECKED : BST_UNCHECKED);
                    CheckDlgButton(hwndDlg,IDC_CONT_LOOP,   (config_playflag & CPLAYFLG_CONT_LOOP)    ? BST_CHECKED : BST_UNCHECKED);
                    CheckDlgButton(hwndDlg,IDC_PLAYALL,     (config_playflag & CPLAYFLG_PLAYALL)      ? BST_CHECKED : BST_UNCHECKED);
                    CheckDlgButton(hwndDlg,IDC_FADECHECK,   (config_playflag & CPLAYFLG_FADEOUT)      ? BST_CHECKED : BST_UNCHECKED);
                    CheckDlgButton(hwndDlg,IDC_STRIPSILENCE,(config_playflag & CPLAYFLG_STRIPSILENCE) ? BST_CHECKED : BST_UNCHECKED);
                    CheckDlgButton(hwndDlg,IDC_SEEKBYORDERS,(config_playflag & CPLAYFLG_SEEKBYORDERS) ? BST_CHECKED : BST_UNCHECKED);
                    CheckDlgButton(hwndDlg,IDC_RESONANCE,config_resonance ? BST_CHECKED : BST_UNCHECKED);

                    FadeOutSetup(hwndDlg, config_playflag & CPLAYFLG_FADEOUT);
                return TRUE;

            }
        }
        break;

        // =============================================================================
        case WM_COMMAND:
        // =============================================================================
        // Process commands and notification messages recieved from our child controls.

            switch(LOWORD(wParam))
            {
				case IDOK:
                {

                    switch(lParam)
                    {   
                        case IDD_PREFTAB_DECODER:
						{
							CHAR    stmp[32] = {0};
							double  ftmp;
                            config_loopcount = SendMessage(GetDlgItem(hwndDlg, IDC_LOOPS), TBM_GETPOS, 0, 0);
                            if (config_loopcount == 64) config_loopcount = -1;

                            GetDlgItemText(hwndDlg, IDC_FADEOUT, stmp, 12);
                            ftmp = atof(stmp);
                            config_fadeout = (int)(ftmp * 1000l);
	    				    config_fadeout = _mm_boundscheck(config_fadeout, 0, 1000l*1000l);   // bound to 1000 seconds.

                            {
                                int erg = SendMessage(GetDlgItem(hwndDlg,IDC_PANSEP),TBM_GETPOS,0,0)<<4;

                                if (erg <= 256)
                                    erg /= 2;
                                else erg = 128 + ((erg - 256) * 384) / 256;
                            
                                config_pansep = erg;
                            }

	            			config_playflag   = IsDlgButtonChecked(hwndDlg,IDC_LOOPALL)      ? CPLAYFLG_LOOPALL      : 0;
	            			config_playflag  |= IsDlgButtonChecked(hwndDlg,IDC_CONT_LOOP)    ? CPLAYFLG_CONT_LOOP    : 0;
	            			config_playflag  |= IsDlgButtonChecked(hwndDlg,IDC_PLAYALL)      ? CPLAYFLG_PLAYALL      : 0;
	            			config_playflag  |= IsDlgButtonChecked(hwndDlg,IDC_FADECHECK)    ? CPLAYFLG_FADEOUT      : 0;
	            			config_playflag  |= IsDlgButtonChecked(hwndDlg,IDC_STRIPSILENCE) ? CPLAYFLG_STRIPSILENCE : 0;
	            			config_playflag  |= IsDlgButtonChecked(hwndDlg,IDC_SEEKBYORDERS) ? CPLAYFLG_SEEKBYORDERS : 0;
	            			config_resonance  = IsDlgButtonChecked(hwndDlg,IDC_RESONANCE) ? 1 : 0;
						}
                        break;
                    }
                }
                break;

                case IDC_FADECHECK:         // hide/unhide fadeout controls
                    if(HIWORD(wParam) == BN_CLICKED)
                    {   int  res = SendMessage((HWND)lParam,BM_GETCHECK,0,0);
                        FadeOutSetup(hwndDlg, res);
                    }
                break;
            }
		break;

        // =============================================================================
        case WM_HSCROLL:
        // =============================================================================
            switch (GetDlgCtrlID((HWND)lParam))
            {
            case IDC_LOOPS:
                {
                    wchar_t foo[64] = {0};
                    int pos = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
                    HWND hText = GetDlgItem(hwndDlg, IDC_LOOPTEXT);

                    if (!pos)
                        WASABI_API_LNGSTRINGW_BUF(IDS_DO_NOT_LOOP,foo,64);
                    else if (pos == 64)
                        WASABI_API_LNGSTRINGW_BUF(IDS_LOOP_FOREVER,foo,64);
                    else
						StringCchPrintfW(foo, 64, WASABI_API_LNGSTRINGW(IDS_LOOP_X_TIMES), pos + 1);

                    SetWindowTextW(hText, foo);
                }
                break;
            }
        break;

        // =============================================================================
        case WM_NOTIFY:
        // =============================================================================

            switch(LOWORD(wParam))
            {
                case IDC_FADESPIN:
                {
                    NMUPDOWN *mud = (NMUPDOWN *) lParam;
                    
                    if(mud->hdr.code == UDN_DELTAPOS)
                    {
						// bounds check things between 0-1000secs (added in 2.2.6)
                        if(mud->iDelta > 0)
						{
							if(config_fadeout > 0)
								config_fadeout -= 250;
						}
						else
						{
							if(config_fadeout < 1000l*1000l)
								config_fadeout += 250;
						}
                        FadeoutSetText(hwndDlg);
                    }
                }
                return TRUE;
            }
        break;
    }

	const int controls[] = 
	{
		IDC_LOOPS,
		IDC_PANSEP,
		IDLDR_PANPOS,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
	{
		return TRUE;
	}

    return 0;
}


// =====================================================================================
    static BOOL CALLBACK prefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
// =====================================================================================
// This is the procedure which initializes the various forms that make up the tabs in
// our preferences box!  This also contains the message handler for the OK and Cancel
// buttons.  After that, all messaging is handled by the tab forms themselves in tabProc();
{
    switch (uMsg)
    {   
        case WM_INITDIALOG:
        {
            CFG_DLGHDR *pHdr = (CFG_DLGHDR*)calloc(1, sizeof(CFG_DLGHDR));
            SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) pHdr); 
          	pHdr->hwndTab = GetDlgItem(hwndDlg,MM_PREFTAB);
            pHdr->left = 8;
            pHdr->top  = 30;

            prefsTabInit(hwndDlg, pHdr);

			{
			wchar_t title[128] = {0}, temp[128] = {0};
			StringCchPrintfW(title, 128, WASABI_API_LNGSTRINGW(IDS_PREFERENCES_TITLE),WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_MODULE_DECODER_OLD,temp,128));
			SetWindowTextW(hwndDlg,title);
			}
        }
        return FALSE;

	
	    case WM_COMMAND:
			switch (LOWORD(wParam))
			{	case IDOK:
                {
                    // Send an IDOK command to both tabcontrol children to let them know
                    // that the world is about to end!
                    CFG_DLGHDR *pHdr = (CFG_DLGHDR*)GetWindowLong(hwndDlg, GWL_USERDATA);

                    SendMessage(pHdr->apRes[0], WM_COMMAND, (WPARAM)IDOK, (LPARAM)IDD_PREFTAB_MIXER);
                    SendMessage(pHdr->apRes[1], WM_COMMAND, (WPARAM)IDOK, (LPARAM)IDD_PREFTAB_DECODER);
                    SendMessage(pHdr->apRes[2], WM_COMMAND, (WPARAM)IDOK, (LPARAM)IDD_PREFTAB_LOADER);

                    EndDialog(hwndDlg,0);
                return 0;
                }
				
			    case IDCANCEL:
					EndDialog(hwndDlg,0);
				return FALSE;

                case OQ_QUALITY:
                    uMsg = 8;
                break;
			}
		break;

		case WM_NOTIFY:
		{   NMHDR *notice = (NMHDR *) lParam;

            NMHDR *ack;
            uint   k;
            ack = (NMHDR *)lParam;

            if(ack->hwndFrom == GetDlgItem(hwndDlg,OQ_QUALITY))
            {
                switch(ack->code)
                {
                    case CBEN_GETDISPINFO:
                        k = 1;
                    break;
                }
            }

			switch(notice->code)
            {   case TCN_SELCHANGE:
                    OnSelChanged(hwndDlg,0);
			    return TRUE;
			}
		}
	    return FALSE;

        case WM_DESTROY:
        {
            // free local data
            free((CFG_DLGHDR*)GetWindowLong(hwndDlg, GWL_USERDATA));
        }
        break;    
    }
    return FALSE;
}


static int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
{
	MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMSW),0};
	msgbx.lpszText = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon = MAKEINTRESOURCEW(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	msgbx.hwndOwner = parent;
	return MessageBoxIndirectW(&msgbx);
}


// =====================================================================================
    void __cdecl about(HWND hwndParent)
// =====================================================================================
{
	wchar_t message[1024] = {0}, text[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_MODULE_DECODER_OLD,text,1024);
	StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					 mikmod.description, TEXT(__DATE__));
	DoAboutMessageBox(hwndParent,text,message);
}


static const wchar_t *pExtCompress[] = { L"ITZ", L"MDZ", L"S3Z", L"STZ", L"XMZ" };
static const wchar_t *pExtReplace[] = { L"IT", L"MOD", L"S3M", L"STM", L"XM" };

BOOL GetTypeInfo(LPCWSTR pszType, LPWSTR pszDest, INT cchDest) // return TRUE if typ was found ok
{
	DWORD lcid;
	LPCWSTR p(NULL);
	wchar_t buf[128]={0};
	int i;
	BOOL bCompressed(FALSE);

	lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	for (i = sizeof(pExtCompress)/sizeof(wchar_t*) - 1; i >= 0 && CSTR_EQUAL != CompareStringW(lcid, NORM_IGNORECASE, pszType, -1,pExtCompress[i], -1); i--);
	if (-1 != i) 
	{
		pszType = pExtReplace[i];
		bCompressed = TRUE;
	}

	for (i = 0; i < C_NUMLOADERS && !p; i++)
    {
		if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszType, -1, AutoWide(c_mod[i].loader->Type), -1))
		{
			p = WASABI_API_LNGSTRINGW_BUF(c_mod[i].loader->DescStrID, buf, 128);
		}
    }

	if (!p)
	{
		if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszType, -1, L"NST", -1))
		{
			p = WASABI_API_LNGSTRINGW_BUF(IDS_FAMILY_STRING_NOISETRACKER, buf, 128);
		}
	}

	if (p) return (S_OK == StringCchPrintfW(pszDest, cchDest, WASABI_API_LNGSTRINGW((bCompressed?IDS_X_COMPRESSED_MODULE:IDS_X_MODULE)), p));
	return FALSE;
}