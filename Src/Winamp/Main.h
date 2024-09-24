#ifndef _MAIN_H_
#define _MAIN_H_
#ifdef __cplusplus

//#pragma warning(error:4311)
extern "C"
{
#endif
#include <windows.h>
//#include <windowsx.h>
#include "wasabicfg.h"
#include "wa_ipc.h"
#include "config.h"
#include "dpi.h"
#ifndef __cplusplus
#include "Plush/plush.h"
#endif

#define APSTUDIO_INVOKED
#include "resource.h"

#include <mbstring.h>
#include <shlwapi.h>
#include <stdio.h>
#include <strsafe.h>
#include <shlobj.h>
#include <commctrl.h>
#include <shobjidl.h>
#include <shellapi.h>
#include "in2.h"
#include "strutil.h"
#include "../nu/trace.h"
#ifndef NO_INPLACE_RESOLVE

#if defined(_WIN64)
 #include "../Elevator/IFileTypeRegistrar_64.h"
#else
 #include "../Elevator/IFileTypeRegistrar_32.h"
#endif

#endif
	/* configuration */

	//#undef MAX_PATH
	//#define MAX_PATH 512
#define OFFSCREEN_Y_POS -30000
#define MAX_URL 4096
extern HINSTANCE language_pack_instance;
#define UPDATE_DISPLAY_TIMER 38
#define STATS_TIMER 64

#define SPLASH_DELAY 2000
#define APP_NAME "Winamp"
#ifdef __alpha
#define APP_VERSION_PLATFORM "(AXP)"
#elif defined(_WIN64)
#define APP_VERSION_PLATFORM "(x64)"
#elif defined(_WIN32_WINCE)
#define APP_VERSION_PLATFORM "(CE)"
#else
#define APP_VERSION_PLATFORM "(x86)"
#endif
	extern const char app_name[], app_version[], app_version_string[];
#define BIGINT 1000000000

#define MAINMENU_OPTIONS_BASE (11+g_mm_optionsbase_adj)
#define WINDOWMENU_FFWINDOWS_BASE (3+g_mm_ffwindowsbase_adj)

#define HIDDEN_TRAP -33

#define WINDOW_WIDTH 275
#define WINDOW_HEIGHT 116


#define FALLBACK_FONT L"Arial"
//#define DEFAULT_FONT "Arial Unicode MS"
	#define DEFAULT_FONT L"Arial"
	//extern prefsDlgRec *g_piprefsdlgs;

#define CAPTION_SIZE (MAX_PATH+128)

#define AUDITSIZE 10

//#define CLASSIC_SKIN_NAME L"Winamp Classic"
#define MODERN_SKIN_NAME	L"Winamp Modern"
#define BENTO_SKIN_NAME		L"Bento"
#define BIG_BENTO_SKIN_NAME L"Big Bento"

	/***********************
		*** about.c
		*/
	void about_dialog(void);


	extern int about_lastpage;
	extern HWND about_hwnd;


	/***********************
	*** about2.c
	*/
	void About2_Start(HWND hwndParent);
	void About2_Kill();

	/***********************
	*** bookmark.c
	*/
	void Bookmark_additem(wchar_t *fn, wchar_t *ft);
	void Bookmark_AddCommandline(wchar_t *commandLine);

	/***********************
	*** burn.cpp
	*/
	int burn_start(burnCDStruct *param);
	unsigned int burn_doBurn(char *cmdline, HWND winampWnd, HINSTANCE winampInstance);

	/***********************
	*** config.c 
	*/ 
	// configuration variables/with defaults
	#ifndef NO_INPLACE_RESOLVE
	int GetRegistrar(IFileTypeRegistrar **registrar, BOOL use_fallback);
	#endif
	void init_config();
	void setup_config(void);
	void config_write(int);
	void config_read(int);
	BOOL config_setup_filetypes(int mode);
	void config_adddesktop(void);
	int config_isregistered(wchar_t *);
	BOOL config_registermediaplayer(DWORD accessEnabled);
	BOOL config_register_capability(wchar_t *ext, int mode);
	void config_register(wchar_t *ext, int reg);
	void config_setinifile(wchar_t *inifile);
	void config_setinidir(const wchar_t *inidir);
	void config_setm3udir(const wchar_t *m3udir);
	void config_setm3ubase(const wchar_t *m3ubase);
	void config_load_langpack_var(void);
	void config_save_langpack_var(void);
	int config_isdircontext(void);
	BOOL config_setup_filetype(const wchar_t *winamp_file, const wchar_t *name, BOOL use_fallback);
	BOOL config_adddircontext(BOOL use_fallback);
	int config_iscdplayer(void);
	BOOL config_removedircontext(BOOL use_fallback);
	BOOL config_regcdplayer(int reg, int mode);
	void config_remove_winamp_keys(void);
	void config_agent_add(void);
	void config_agent_remove(void);
	void regmimetype(const wchar_t *mtype, const wchar_t *programname, const wchar_t *ext, int nsonly);
	void RemoveRegistrar();

	/***********************
	*** convert.cpp
	*/
	int convert_file(convertFileStruct *cfs);
	void convert_end(convertFileStruct *cfs);
	int convert_fileW(convertFileStructW *cfs);
	void convert_endW(convertFileStructW *cfs);
	HWND convert_config(convertConfigStruct *ccs);
	void convert_config_end(convertConfigStruct *ccs);
	void convert_enumfmts(converterEnumFmtStruct *cefs);
	void convert_setPriority(convertSetPriority *csp);
	void convert_setPriorityW(convertSetPriorityW *csp);
	int convert_setConfigItem(convertConfigItem *cci);
	int convert_getConfigItem(convertConfigItem *cci);
	int convert_file_test(convertFileStructW *cfs);

	/***********************
	*** dde.c 
	*/
	int dde_addstart(HWND hwnd);
	void dde_delstart(void);
	int dde_isquicklaunchavailable(void);
	void dde_addquicklaunch(HWND hwnd);
	void dde_adddesktop(HWND hwnd);
	void dde_remdesktop(void);
	void dde_remquicklaunch(void);

	/***********************
	*** dock.c
	*/
	void FixMainWindowRect(RECT *r);
	void EstMainWindowRect(RECT *r);
	void EstEQWindowRect(RECT *r);
	void EstPLWindowRect(RECT *r);
	void EstVidWindowRect(RECT *r);
	void SetVidWindowRect(RECT *r);
	void SetMainWindowRect(RECT *r);
	void SetEQWindowRect(RECT *r);
	void SetPLWindowRect(RECT *r);
	void MoveRect(RECT *r, int x, int y);
	int IsWindowAttached(RECT rc, RECT rc2);
	void SnapWindowToWindow(RECT *rcSrc, RECT rcDest);
	void AdjustSnap(RECT old1, RECT old2, RECT *new1, RECT *new2);
	int IsPointInRect(int x, int y, RECT *r);
	void FixOverlaps(RECT *r1, RECT *r2);


	/***********************
	*** draw.c 
	*/
	extern int (WINAPI *jtf_drawtext)(HDC, LPCWSTR, int, LPRECT, UINT); 
	void draw_set_plbm(HBITMAP);
	extern int pe_fontheight;
	extern HPALETTE draw_hpal;
	void draw_firstinit(void);
	void draw_finalquit(void);
	void draw_init(void);
	void draw_paint(HWND hwnd);
	void draw_printclient(HDC hdc, LPARAM drawingOptions);
	void draw_kill(void);
	void draw_clear(void);
	void draw_monostereo(int value); // 0 is neither, 1 is mono, 2 is stereo
	void draw_shuffle(int on, int pressed);
	void draw_repeat(int on, int pressed);
	void draw_eqplbut(int eqon, int eqpressed, int plon, int plpressed);
	void draw_volumebar(int volume, int pressed); // volume is 0-51
	void draw_panbar(int volume, int pressed);
	void draw_songname(const wchar_t *name, int *position, int songlen);
	void draw_positionbar(int position, int pressed); // position is 0-256
	void draw_bitmixrate(int bitrate, int mixrate);
	void draw_buttonbar(int buttonpressed); // starts at 0 with leftmost, -1 = no button
	void draw_playicon(int whichicon); // 0 = none, 1 = play, 2 = stop, 4 = pause
	void draw_time(int minutes, int seconds, int clear);
	void draw_sa(unsigned char *values, int draw); // array of 12 bands, starting with leftmost, of 0..7
	void draw_tbuttons(int b1, int b2, int b3, int b4);
	void draw_setnoupdate(int);
	void draw_tbar(int active, int windowshade, int egg);
	void draw_eject(int pressed);
	void draw_clutterbar(int enable);
	void update_volume_text(int songlen);
	void update_panning_text(int songlen);

	/*****************
	*** draw_eq.c 
	*/
	extern int eq_init;
	void draw_eq_init();
	void draw_eq_kill();
	void draw_paint_eq(HWND hwnd);
	void draw_printclient_eq(HDC hdc, LPARAM /*drawingOptions*/);
	void draw_eq_tbutton(int b3, int wsb);
	void draw_eq_slid(int which, int pos, int pressed); // left to right, 0-64
	void draw_eq_tbar(int active);
	void draw_eq_onauto(int on, int autoon, int onpressed, int autopressed);
	void draw_eq_presets(int pressed);
	void draw_eq_graphthingy(void);

	/*****************
	*** draw_pe.c 
	*** for functions that accept HDC as the first parameter,
	*** hdc can be NULL, and it will use the Window DC
	*/
	extern int pe_init;
	void draw_pe_init();
	void draw_pe_kill();
	void draw_pe_tbutton(int b2, int b3, int b2_ws);
	void draw_paint_pe(HWND hwnd);
	void draw_printclient_pe(HWND hwnd, HDC hdc, LPARAM drawingOptions);
	void draw_reinit_plfont(int notifyOthers);
	void draw_pe_tbar(HWND hwnd, HDC hdc, int state);
	void draw_pe_vslide(HWND hwnd, HDC hdc, int pushed, int pos); // pos 0..100
	void draw_pe_timedisp(HDC hdc, int minutes, int seconds, int tlm, int clear); 

	void draw_pe_addbut(int which); // -1 = none, 0 = file, 1 = dir, 2 = loc
	void draw_pe_rembut(int which); // -1 = none, 0 = sel, 1 = crop, 2 = all
	void draw_pe_selbut(int which); // -1 = none, 0 = all, 1 = none, 2=inv
	void draw_pe_miscbut(int which); // -1 = none, 0 = inf, 1 = sort, 2=misc
	void draw_pe_iobut(int which); // -1 = none, 0 = load, 1=save, 2=clear

	/*****************
	*** draw_vw.c 
	*/
	extern int vw_init;
	void draw_vw_init();
	void draw_vw_kill();
	void draw_vw_tbar(int state);
	void draw_vw(HDC hdcout);
	void draw_paint_vw(HWND hwnd);
	void draw_vw_tbutton(int b3);
	void draw_vw_mbuts(int whichb);
	void draw_vw_info(wchar_t *t, int erase);

	/*****************
	*** dsp.c 
	*/
	void dsp_init(void);
	void dsp_quit(void);
	int dsp_dosamples(short int *samples, int numsamples, int bps, int nch, int srate);
	int dsp_isactive(void);

	/*****************
	*** eq.c 
	*/
	extern unsigned char eq_tab[10];
	void eq_dialog(HWND, int);
	void eq_autoload(const char *mp3fn);
	LRESULT CALLBACK EQ_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


	/*****************
	*** embedwnd.cpp
	*/
	#ifdef __cplusplus
	typedef struct _EMBEDWND
	{
		HWND hLastFocus;
	} EMBEDWND;
	#include "../nu/CGlobalAtom.h"
	static CGlobalAtom EMBEDWND_PROPW(L"EMBDEWND");
	#define GetEmbedWnd(__hwnd) ((EMBEDWND*)GetPropW(__hwnd, EMBEDWND_PROPW))
	#endif

	extern LRESULT CALLBACK emb_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	extern HWND embedWindow(embedWindowState *state);
	extern void SnapWindowToAllWindows(RECT *outrc, HWND hwndNoSnap);
	extern BOOL SnapToScreen(RECT *outrc);

	extern CRITICAL_SECTION embedcs;
	extern embedWindowState *embedwndlist; // linked list
	extern int embedwndlist_cnt;
#define EMBED_STATE_EXTRA_LINK 0
#define EMBED_STATE_EXTRA_ATTACHED 1
#define EMBED_STATE_EXTRA_GUID 4 // note this reserved 5-7 also
#define EMBED_STATE_EXTRA_REPARENTING 62
#define EMBED_STATE_EXTRA_FFROOTWND 63

	/*****************
	*** equi.c 
	*/
	extern int minimize_hack_winamp;
	extern int do_volbar_active, do_panbar_active;

	void eq_ui_handlecursor(void);
	void equi_handlemouseevent(int x, int y, int type, int stats);
	void ui_handlecursor(void);

	BOOL DoTrackPopup(HMENU hMenu, UINT fuFlags, int x, int y, HWND hwnd);

	/*****************
	*** gen.cpp
	*/

	void load_genplugins();
	void unload_genplugins();

	/*****************
	*** html.c 
	*/
	void doHtmlPlaylist(void);

	/*****************
	*** http.c 
	*/
	int httpRetrieveFile(HWND hwnd, const char *url, char *file, char *dlgtitle);
	int httpRetrieveFileW(HWND hwnd, const char *url, const wchar_t *file, const wchar_t *dlgtitle);

	/*****************
	*** in.c 
	*/

	extern In_Module *in_mod;
	int in_init();
	void in_deinit();
	In_Module *in_setmod(wchar_t *filename);
	In_Module *in_setmod_noplay(const wchar_t *filename, int *start_offs); // starts at *start_offs, sets *start_offs with the value of the module used
	char *in_getfltstr(void);
	wchar_t *in_getfltstrW(BOOL skip);
	char *in_getextlist(void);
	wchar_t *in_getextlistW();
	int in_getouttime(void);
	int in_getlength(void);
	void in_pause(int p);
	int in_seek(int time_in_ms);
	int in_open(const wchar_t *fn);
	void in_setvol(int v);
	void in_setpan(int p);
	void in_close(void);
	int in_infobox(HWND hwnd, const wchar_t *fn);
	int in_get_extended_fileinfoW(const wchar_t *fn, const wchar_t *metadata, wchar_t *dest, size_t destlen);
	int in_get_extended_fileinfo(const char *fn, const char *metadata, char *dest, size_t destlen);
	int in_set_extended_fileinfo(const char *fn, const char *metadata, char *dest);
	int in_set_extended_fileinfoW(const wchar_t *fn, const wchar_t *metadata, wchar_t *data);
	int in_write_extended_fileinfo();
	void eq_set(int on, char data[10], int preamp);
	void in_flush(int ms);

	/**************
	*** jump.c 
	*/
	void SetJumpComparator(void *functionPtr);
	void SetJumpComparatorW(void *functionPtr);
	int jump_dialog(HWND hwnd);
	int jump_file_dialog(HWND hwnd);



	/***************
	*** m3u.c
	*/
	int savem3ufn(const wchar_t *filename, int rel, int useBase);

	/* main.c */

#include "buildType.h"

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x20A
#endif
	extern UINT songChangeBroadcastMessage;
	extern HMENU main_menu, top_menu, systray_menu, g_submenus_bookmarks1,
				 g_submenus_bookmarks2, g_submenus_skins1, g_submenus_skins2,
				 g_submenus_vis, g_submenus_options, g_submenus_lang,
				 g_submenus_play, v5_top_menu;
	extern int unique_loword_command;
	extern int g_submenus_lang_id;
	extern char g_audiocdletter[];
	extern int g_audiocdletters;
	extern int g_dropaot_timer_set;
	extern int is_install;
	extern int g_fsapp;
	extern int g_restoreaot_timer_set;
	extern int bNoHwndOther;
	extern int g_safeMode;
	void tealike_crappy_code(unsigned long v[2], unsigned long k[4]);
	void UpdateAudioCDMenus(HMENU hmenu);
	void MoveOffscreen(HWND hwnd);
	void ResolveEnvironmentVariables(wchar_t *string, size_t stringSize);
	extern int g_main_created;
	extern int g_noreg;
	extern wchar_t szAppName[64]; //	window class name, generated on the fly.
	extern int g_fullstop;
	extern int vis_fullscreen;
	extern int stat_isit; // used for faster version checking
	extern int g_mm_ffoptionsbase_adj;
	extern int no_notify_play;
	extern int last_no_notify_play;
	extern int disable_skin_cursors;
	extern int disable_skin_borders;
	extern int g_exit_disabled;
	extern UINT g_scrollMsg;
	LRESULT CALLBACK Main_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	extern int Ipc_WindowToggle(INT_PTR which, INT_PTR how);
	extern int eggstat;
	extern int last_brate, g_need_titleupd, g_need_infoupd;
	extern int g_stopaftercur;
	extern int g_has_deleted_current;

	extern int g_has_video_plugin, g_no_video_loaded;
	extern char *audits[AUDITSIZE];
	extern int audit_ptr;

	extern HWND hTooltipWindow, hEQTooltipWindow, hVideoTooltipWindow, hPLTooltipWindow/*, hPL2TooltipWindow*/;
	extern HWND hMainWindow, hEQWindow, hPLWindow, /*hMBWindow, */hVideoWindow;
	extern HINSTANCE hMainInstance;
	extern HANDLE hMainThread;
	extern DWORD mainThreadId;
	extern int paused;
	extern int playing;
	extern wchar_t caption[CAPTION_SIZE];
	extern wchar_t FileName[FILENAME_SIZE];
	extern wchar_t FileTitle[FILETITLE_SIZE];
	extern wchar_t FileTitleNum[FILETITLE_SIZE];
	extern char *app_date;
	extern int g_srate, g_brate, g_nch, g_srate_exact;

	extern int g_mm_optionsbase_adj;
	extern int g_mm_ffwindowsbase_adj;
	extern HWND g_dialog_box_parent;
	extern int g_restartonquit;
	extern char playlist_custom_font[128];
	extern wchar_t playlist_custom_fontW[128];
	extern int config_custom_plfont;
	extern int disable_skin_cursors;
	LRESULT sendMlIpc(int msg, WPARAM param);
	extern HWND hExternalVisWindow;
	int CALLBACK WINAPI BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
	int CALLBACK WINAPI BrowseCallbackProc_Download(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
	BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam);
	int Main_OnClose(HWND hwnd);
#define DIALOG_PARENT(w) (g_dialog_box_parent?g_dialog_box_parent:w)
	int CreateMainWindow();
	void BuildAppName();
	WPARAM WinampMessageLoop();


	/***************
	*** metrics.c
	*/
	#define METRICS_EMAIL			0x0001
	#define METRICS_COUNTRY			0x0002
	#define METRICS_ANNOUNCEMENTS	0x0003
	#define METRICS_GENDER			0x0004

	INT GetMetricsValueW(const char *data, const char *pszType, void *pDest, int cbDest);
	BOOL SetMetricsValueW(const char *data, const char *pszType, const void *pVal, int cbVal);
	BOOL SendMetrics(const char *data, HWND hwndParent);
	INT GetMetricsSize(const char *data);


	/***************
	*** verchk.c
	*/
	void newversioncheck(void);
	void Ping(const char *url);

	/***************
		*** ole.cpp
		*/
	void InitDragDrops();
	void UninitDragDrops();
	void Ole_initDragDrop(void);
	void Ole_uninitDragDrop(void);
	void *Ole_getDropTarget(void);

	/***************
	*** options.c 
	*/
#define PREFS_UNICODE ((_prefsDlgRec *)1)
#define PREFS_ACP ((_prefsDlgRec *)0)
	extern int g_taskbar_dirty;
	extern intptr_t prefs_last_page;
	extern RECT prefs_rect, alt3_rect, ctrle_rect, about_rect,
				loc_rect, time_rect, load_rect, editinfo_rect;
	extern HWND prefs_hwnd;
	void prefs_dialog(int modal);
	void prefs_liveDlgAdd(prefsDlgRec * p);
	void prefs_liveDlgRemove(prefsDlgRec * p);
	void prefs_liveDlgUpdate(prefsDlgRec * p);
	void SetDialogBoxFromFile(FILE *fp, HWND hwndDlg, int id);
	/***************
	*** options_playlist.cpp
	*/
	void UpdatePlaylistFontSizeText(void);
	void UpdateManualAdvanceState(void);
	/***************
	*** options_skin.cpp
	*/
	typedef struct _ENUMSKIN
	{
		const wchar_t	*pszFileName;
		const wchar_t	*pszName;
		int				nType;
		int				bActive;
	} ENUMSKIN;

	#define SKIN_FILETYPE_EMBED	3
	#define SKIN_FILETYPE_DIR	0
	#define SKIN_FILETYPE_ZIP	1
	#define SKIN_FILETYPE_WSZ	2
	#define SKIN_FILETYPE_WAL	4
	

	typedef int (CALLBACK *ENUMSKINPROC)(ENUMSKIN* /*pes*/, void * /*user*/); // return 0 to stop enumeration
	int EnumerateSkins(ENUMSKINPROC fnEnumSkin, void *user);
	/***************
	*** options_general.cpp
	*/
	typedef struct _ENUMLANG
	{
		const wchar_t	*pszFileName;
		const wchar_t	*pszName;
		int				nType;
		int				bActive;
	} ENUMLANG;

	//#define LANG_FILETYPE_LNG	1	// DEPRECATED: no longer a supported type
	#define LANG_FILETYPE_WLZ	2
	#define LANG_FILETYPE_EMBED	3
	#define LANG_FILETYPE_ZIP	4	// added 5.66
	#define LANG_FILETYPE_DIR	0	// added 5.66
	
	typedef int (CALLBACK *ENUMLANGPROC)(ENUMLANG* /*pel*/, void * /*user*/); // return 0 to stop enumeration
	int EnumerateLanguages(ENUMLANGPROC fnEnumLang, void *user);
	void LangSwitchToLangPrompt(HWND hwndDlg, wchar_t* newLang);
	/***************
	*** out.c 
	*/
	extern Out_Module *out_modules[32];
	extern Out_Module *out_mod;
	void out_init();
	void out_deinit();
	void out_setwnd();
	void out_changed(HINSTANCE hLib, int enabled);

	/***************
	*** peui.c
	*/
	void peui_handlemouseevent(HWND hwnd, int x, int y, int type, int stats);
	void pe_ui_handlecursor(HWND hwnd);


	/****************
	*** play.c 
	*/
	void getNewFile(int, HWND, const wchar_t *);
	LRESULT getNewLocation(int, HWND); // if the int param is -1, returns a HGLOBAL to the thing instead of acting on it
	void StartPlaying();
	void StopPlaying(int);
	void PausePlaying();
	void UnPausePlaying();
	void PlayThing(const char *thing, int clearlist);
	void BeginPlayback();


	/***************
	*** playlist.c 
	*/
	__declspec(dllexport) extern wchar_t *(*plstring_wcsdup)(const wchar_t *str);
	__declspec(dllexport) extern wchar_t *(*plstring_malloc)(size_t str_size);
	__declspec(dllexport) extern void (*plstring_release)(wchar_t *str);
	__declspec(dllexport) extern void (*plstring_retain)(wchar_t *str);

	int LoadPlaylistByExtension(const wchar_t *fn, const wchar_t *ext, int whattodo, int useBase);
	int PlayList_getitem2W(int position, wchar_t *filename, wchar_t *filetitle);
	int PlayList_getitem_jtfW(int position, wchar_t *str);
	void PlayList_getcurrent_tupdate(wchar_t *FileName, wchar_t *FileTitle);
	int IsPlaylistExtension(const wchar_t *extension);
	int IsPlaylistExtensionA(const char *extension);
	int LoadPlaylist(const wchar_t *filename, int whattodo, int doMIMEcheck); // returns -1 if not a playlist, 0 on success, 1 on failure
	void plstring_init();

	extern wchar_t playlistStr[19];

	void PlayList_UpdateTitle(const wchar_t *filename);
	int PlayList_GetNextSelected(int start);
	int PlayList_GetSelectedCount();
	void PlayList_updateitem(int position);
	void PlayList_getcurrent_onstop(wchar_t *filename, wchar_t *filetitle);
	int PlayList_gethidden(int pos);
	int PlayList_ishidden(int pos);
	int PlayList_alldone(int pos);
	int PlayList_hasanycurtain(int pos);
	int PlayList_current_hidden(void);
	const char *PlayList_getcurtain(int pos);
	const char *PlayList_getExtInf( int pos );
	//const char *PlayList_getbrowser(int pos);
	void PlayList_resetcurrent(void);
	int PlayList_getitem(int position, wchar_t *filename, wchar_t *filetitle);
	int PlayList_getitem2(int position, char *filename, char *filetitle);
	int PlayList_getitem3(int position, char *filetitle, char *filelength);
	int PlayList_getitem3W(int position, wchar_t *filetitle, wchar_t *filelength);
	int PlayList_getitem_pl(int position, wchar_t *);
	int PlayList_getlength(void);
	int PlayList_deleteitem(int item);
	void PlayList_delete(void);
	void PlayList_destroy(void);
	void PlayList_append(const wchar_t *filename, int is_nde_string);
	void PlayList_appendthing(const wchar_t *url, int doMIMEcheck, int is_nde_string);
	void PlayList_append_withinfo(const wchar_t *filename, const wchar_t *title, const wchar_t *ext, int length, int is_nde_string);
	void PlayList_append_withinfo_curtain(const wchar_t *filename, const wchar_t *title, int length, char *curtain, const wchar_t *ext, int is_nde_string);
	void PlayList_append_withinfo_hidden(const wchar_t *filename, const wchar_t *title, int length, char *curtain/*, char *browser*/);
	void PlayList_getcurrent(wchar_t *filename, wchar_t *filetitle, wchar_t *filetitlenum);
	void PlayList_GetCurrentTitle(wchar_t *filetitle, int cchLen);
	void PlayList_setcurrent(const wchar_t *filename, wchar_t *filetitle);
	void PlayList_swap(int e1, int e2);
	int PlayList_setposition(int pos);
	int PlayList_advance(int byval);
	int PlayList_getPosition();
	int PlayList_getNextPosition();
	void PlayList_addfromdlg(const wchar_t *fns); // replace with api_playlistmanager
	void PlayList_refreshtitle(void);
	wchar_t *PlayList_gettitle(const wchar_t *filename, int useID3);
	const wchar_t *PlayList_GetCachedTitle(const wchar_t *filename);
	int PlayList_randpos(int);
	void PlayList_randomize(void);
	void PlayList_reverse(void);
	void PlayList_sort(int, int start_p);
	void PlayList_adddir(const wchar_t *path, int recurse); // returns 0 if path was invalid // replace with api_playlistmanager
	void PlayList_updaterandpos(void);
	void PlayList_makerelative(const wchar_t *listfile, wchar_t *filename, int useBase);
	int PlayList_getsonglength(int x);
	int PlayList_gettotallength(void);
	int PlayList_getcurrentlength(void);
	int PlayList_getselect(int);
	int PlayList_getselect2(int x, wchar_t *filename);
	void PlayList_setselect(int, int);
	void PlayList_setlastlen(int x);
	void PlayList_setitem(int x, const wchar_t *filename, wchar_t *filetitle);
	void PlayList_saveend(int start);
	void PlayList_restoreend(void);
	void PlayList_setcached(int x, int cached);
	int PlayList_getcached(int x);
	void PlayList_SetLastItem_RepeatCount( int count);
	int PlayList_getrepeatcount(int pos);
	void PlayList_terminate_lasthidden(void);
	void PlayList_SetLastItem_Range(unsigned long starttime, unsigned long endtime);
	unsigned long PlayList_GetItem_Start(int pos);
	unsigned long PlayList_GetItem_End(int pos);
	void PlayList_insert(int position, const wchar_t *filename);

	/**************
	*** pledit.c
	*/
	LRESULT CALLBACK PE_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	void pleditDlg(HWND, int);
	void plEditRefresh(void);
	void plEditSelect(int song);
	extern int pledit_disp_offs;
	void PE_Cmd(windowCommand *wc);

	/***************
	*** pls.c 
	*/
	void saveplsfn(const wchar_t *fn);
	int loadpls(HWND, int);
	int savepls(HWND hwnd);

	/************************
	*** sa.c 
	*/
	extern volatile int sa_curmode;
	void SpectralAnalyzer_Create();
	void SpectralAnalyzer_Destroy();
	void sa_setthread(int enabled);
	void sa_deinit(void);
	void sa_init(int numframes);
	int sa_add(char *values, int timestamp, int csa);
	char *sa_get(int timestamp, int csa, char data[75*2 + 8]);

	/************************
	*** set.c 
	*/
	void set_caption(int alt_cb, wchar_t *format, ...);
	void set_aot(int);
	void set_priority(void);
	void set_taskbar(void);
	void set_visopts(void);
	void do_caption_autoscroll(void);
	void set_pl_wnd_tooltip(void);
	void set_vid_wnd_tooltip(void);

	/***********************
	*** shell.cpp 
	*/
	HRESULT ResolveShortCut(HWND hwnd, LPCWSTR pszShortcutFile, LPWSTR pszPath);
	//void CreateShortCut(HWND hwnd, LPCSTR pszShortcutFile, LPCSTR pszExe, LPCSTR, int);
	void Shell_Free(void *p);

	/************************
	*** skins.c 
	*/
	BOOL _cleanupDirW(const wchar_t *dir);
	extern int g_skinloadedmanually, g_skinmissinggenff;
	void CreateDirectoryForFileW(wchar_t *fn, wchar_t *base);	// shared for use with wlz files
	void Skin_Random(void);
	void Skin_Load(void);
	void Skin_CleanupZip(void);
	void Skin_CleanupAfterCrash(void);
	BOOL Skin_Check_Modern_Support();
	int Skin_GetRegionPointList(int eq, int **points, int **counts);
	/* The indices of Skin_PLColors:
	0 - text color - playlist
	1 - text color - currently playing playlist entry
	2 - background color - playlist
	3 - background color - selected playlist entry
	4 - text color - video window info, minibrowser info 
	5 - background - video window info, minibrowser info */
	extern int Skin_PLColors[6], Skin_UseGenNums;
	extern char Skin_PLFont[128];
	extern wchar_t Skin_PLFontW[128];
	extern HWND skin_hwnd;
#define N_CURSORS 29
	extern HCURSOR Skin_Cursors[N_CURSORS];


	/*************************
	*** splash.c 
	*/
	void splashDlg(int wait_in_ms);


	/*************************
	*** stats.c 
	*/
	void Stats_OnPlay(const wchar_t *playstring);
	void stats_write(void); // sets some final stats and writes to disk
	void stats_getuidstr(char *str);
	void stats_init();
	void stats_save(); // just writes to disk

	/***********************
	*** systray.c 
	*/
	extern int systray_intray;
	void systray_minimize(wchar_t *tip);
	void systray_restore(void);

	/* ui.c */
	extern int ui_songposition;
	extern int ui_songposition_dir;
	extern int ui_songposition_tts;
	extern int do_posbar_active;

	void ui_doscrolling();
	void ui_handlemouseevent(int x, int y, int type, int stats); // x,y, -1=up,0=move,1=down, stats = kbstats
	void ui_reset();
	void ui_drawtime(int time_elapsed, int mode); // mode=0 means called from timer, mode=1 forced

	/***********************
	*** util.c 
	*/
	LPCWSTR RepairMutlilineString(LPWSTR pszBuffer, INT cchBufferMax);
	void recent_add(const wchar_t *loc);
	int IsUrl(const char *url);
	//int IsCharSpace(char digit);
	//int IsCharSpaceW(wchar_t digit);
	int IsCharDigit(char digit);
	int IsCharDigitW(wchar_t digit);
	void mbprintf(char *file, int line, char *format, ...);
	void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void link_startsubclass(HWND hwndDlg, UINT id);
	void myOpenURL(HWND, const wchar_t *url);
	void myOpenURLWithFallback(HWND, wchar_t *url, wchar_t *fallbackUrl);
	char *my_strdup(char *s);
	int geticonid(int x);
	int isInetAvailable(void);
	unsigned int getDay(void);
	const wchar_t *scanstr_backcW(const wchar_t *str, const wchar_t *toscan, const wchar_t *defval);
	void getViewport(RECT *r, HWND wnd, int full, RECT *sr);
	BOOL windowOffScreen(HWND hwnd, POINT pt);

	LPCWSTR BuildFullPath(LPCWSTR pszPathRoot, LPCWSTR pszPath, LPWSTR pszDest, INT cchDest); // 
	INT ComparePath(LPCWSTR pszPath1, LPCWSTR pszPath2, LPCWSTR pszPathRoot);  // 
	BOOL DisabledWindow_OnMouseClick(HWND hwnd); // call it in WM_SETCURSOR

	/***************
	*** functions related to uxtheme.dll
	*/

	
	#define ETDT_DISABLE					0x01
	#define ETDT_ENABLE						0x02
	#define ETDT_USETABTEXTURE				0x04
	#define ETDT_ENABLETAB					(ETDT_ENABLE | ETDT_USETABTEXTURE)
	#define ETDT_USEAEROWIZARDTABTEXTURE		0x08
	#define ETDT_ENABLEAEROWIZARDTAB			(ETDT_ENABLE | ETDT_USEAEROWIZARDTABTEXTURE)
	#define ETDT_VALIDBITS					(ETDT_DISABLE | ETDT_ENABLE | ETDT_USETABTEXTURE | ETDT_USEAEROWIZARDTABTEXTURE)

	int IsWinXPTheme(void);
	void DoWinXPStyle(HWND tab);
	int IsAero(void);
	BOOL IsVista(void);
	BOOL IsVistaOrLower(void);
	BOOL IsWin8(void);

	HRESULT WAEnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);

	/***************
	*** lang.cpp
	*/
	HINSTANCE Lang_InitLangSupport(HINSTANCE, const GUID);
	void Lang_FollowUserDecimalLocale(void);
	void Lang_CleanupZip(void);
	void Lang_CleanupAfterCrash(void);
	void Lang_EndLangSupport(void);
	HINSTANCE Lang_FakeWinampLangHInst(HINSTANCE adjustedHInst);
	void Lang_LocaliseAgentOnTheFly(BOOL refresh);

	int extract_wlz_to_dir(wchar_t* readme_only_wlz_extraction, BOOL *skip);
	char *getString(UINT uID, char *str, size_t maxlen);
	wchar_t *getGUIDstr(const GUID, wchar_t *target);

	int LPMessageBox(HWND parent, UINT idMessage, UINT idTitle, UINT type);
	wchar_t *getStringW(UINT uID, wchar_t *str, size_t maxlen);
	#define LPCreateDialog(id, parent, proc) \
			LPCreateDialogParam(id, parent, (DLGPROC)proc, 0)
	HWND LPCreateDialogParam(int id, HWND parent, DLGPROC proc, LPARAM param);
	#define LPDialogBox(id, parent, proc) \
			LPDialogBoxParam(id, parent, (DLGPROC)proc, 0)
	INT_PTR LPDialogBoxParam(int id, HWND parent, DLGPROC proc, LPARAM param);
	#define LPDialogBoxW(id, parent, proc) \
			LPDialogBoxParamW(id, parent, (DLGPROC)proc, 0)
	INT_PTR LPDialogBoxParamW(int id, HWND parent, DLGPROC proc, LPARAM param);
	#define LPCreateDialogW(id, parent, proc) \
			LPCreateDialogParamW(id, parent, (DLGPROC)proc, 0)
	HWND LPCreateDialogParamW(int id, HWND parent, DLGPROC proc, LPARAM param);
	HMENU LPLoadMenu(UINT id);

	/***************
	*** video.cpp
	*/
	extern int g_video_numaudiotracks;
	extern int g_video_curaudiotrack;
	void Browser_Create();
	void Browser_Destroy();
	extern wchar_t vidoutbuf_save[1024];
	extern int is_fullscreen_video;
	int ShowVideoWindow(int init_state);
	void HideVideoWindow(int autoStop);
	LRESULT CALLBACK video_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	int video_isVideoPlaying();
	int videoIsFullscreen();
	void videoToggleFullscreen();
	void videoForceFullscreenOff();
	HWND video_Create();

	/***************
	*** video_ipc.cpp
	*/
	int WINAPI VideoIPCProcedure(int which, WPARAM data, LRESULT *returnValue);

	/***
	** videoui.c
	*/
	void videoui_handlemouseevent(int x, int y, int type, int stats);

	/***********************
	*** vis.c 
	*/
	int  vis_running();
	void vis_start(HWND, wchar_t*);
	void vis_stop();
	void vsa_init(int numframes);
	void vsa_deinit(void);
	void vis_setinfo(int srate, int nch);
	int  vsa_getmode(int *sp, int *wa);
	void vis_setprio(void);
	int  vsa_add(void *data, int timestamp);
	int  sa_add(char *values, int timestamp, int csa);
	void sa_addpcmdata(void *PCMData, int nch, int bps, int timestamp);
	void vsa_addpcmdata(void *PCMData, int nch, int bps, int timestamp);

	LRESULT CALLBACK VIS_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	extern HWND hVisWindow, hPLVisWindow;

	void vis_setextwindow(HWND hwnd);

	/***********************
	*** rand.cpp
	*/
	int warand();
	float warandf();

	/*******************
	*** ipc.cpp
	*/
	LRESULT Main_OnIPC(HWND hwnd, int which, WPARAM data);
	LRESULT wa_register_ipc(WPARAM data);

	/*******************
	*** fullscreen.cpp
	*/
	void BeginFullscreenAppMonitor();
	void EndFullscreenAppMonitor();
	void restoreAOT();
	void dropAOT();

	/*******************
	*** main_mouse.cpp
	*/
	int Main_OnRButtonUp(HWND hwnd, int x, int y, UINT flags);
	int Main_OnLButtonUp(HWND hwnd, int x, int y, UINT flags);
	int Main_OnCaptureChanged(HWND hwnd);
	int Main_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	int Main_OnLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	int Main_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);

	/*******************
	*** main_nonclient.cpp
	*/
	UINT Main_OnNCHitTest(HWND hwnd, int x, int y);
	BOOL Main_OnNCActivate(HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized);
	UINT Main_OnNCCalcSize(HWND hwnd, BOOL fCalcValidRects, NCCALCSIZE_PARAMS * lpcsp);

	/*******************
	*** main_display.cpp
	*/
	int Main_OnGetText(wchar_t *text, int sizeCch);
	int Main_OnDisplayChange(HWND hwnd);
	int Main_OnQueryNewPalette(HWND hwnd);
	int Main_OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange);

	/*******************
	*** main_buttons.cpp
	*/
	int Main_OnButton1(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	int Main_OnButton2(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	int Main_OnButton3(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	int Main_OnButton4(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	int Main_OnButton5(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	/*******************
	*** main_close.cpp
	*/
	void Main_OnEndSession(HWND hwnd, BOOL fEnding);
	int Main_OnClose(HWND hwnd);
	int Main_OnDestroy(HWND hwnd);

	/*******************
	*** main_init.cpp
	*/
	void RegisterWinamp();
	BOOL InitApplication(HINSTANCE hInstance);
	BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);

	/*******************
	*** browser.cpp
	*/
	/*
	void LaunchBrowser(const char *url);
	void OpenBrowser();
	void Browser_toggleVisible(int showing);
	void CloseBrowser();
	void Browser_init();
	void Browser_kill();
	*/

	/*******************
	*** ExternalCOM.cpp
	*/
	DISPID __cdecl JSAPI1_GenerateUniqueDispatchId();
	HRESULT __cdecl JSAPI1_Initialize();
	HRESULT __cdecl JSAPI1_Uninitialize();
	HRESULT __cdecl JSAPI1_SkinChanged();
	HRESULT __cdecl JSAPI1_CurrentTitleChanged();

	/*******************
	*** SkinUtils.cpp
	*/
	const char *GetFontName();
	const wchar_t *GetFontNameW();
	int GetFontSize();

	/***
	*** ASXv2.cpp
	*/
	void loadasxv2fn(const wchar_t *filename, int whattodo);
	/*******************
		*** Wasabi.cpp
		*/
	void Wasabi_Load();
	void Wasabi_Unload();
	void Wasabi_FindSystemServices();
	void Wasabi_ForgetSystemServices();

	/*******************
		*** W5S.cpp
		*/
	void w5s_init();
	void w5s_deinit();

	/*******************
		*** vu.cpp
		*/
	void vu_init(int numframes, int srate);
	void vu_deinit();
	void VU_Create();
	void VU_Destroy();
	int vu_add(char *values, int timestamp);
	int export_vu_get(int channel);
	void calcVuData(unsigned char *out, char *data, const int channels, const int bits);

	// PlayQueue.cpp
	int PlayQueue_OnEOF();

	// paths.cpp
	BOOL UtilGetSpecialFolderPath(HWND hwnd, TCHAR *path, int folder);

	// cmdline.cpp
	void GetParameter(const wchar_t *commandLine, wchar_t *yourBuffer, size_t yourBufferSize);

	// unsorted
	void readwrite_client_uid(int isWrite, wchar_t uid_str[64]);
	BOOL read_compatmode();

	extern char metric_plugin_list[];
	char *export_sa_get_deprecated();
	char *export_sa_get(char data[75*2 + 8]);
	void export_sa_setreq(int);
	extern HWND jump_hwnd, jump_hwnd2;
	wchar_t *remove_urlcodesW(wchar_t *p);
	int PlayList_get_lastlen();
	HBITMAP draw_LBitmap(LPCTSTR bmname, const wchar_t *filename);
	extern void draw_mb_info(char *t, int erase);
	extern void FormString(char *in, char *out, int maxlen);
	extern int plmodified, plcleared, plneedsave;
	extern int g_has_deleted_current;
	void peui_reset(HWND hwnd);
	extern int volatile sa_override;
	extern int deferring_show;
	extern int g_showcode;

	extern int eq_startuphack, pe_startuphack;
	extern int g_skinloadedmanually;
	void vis_init();
	void resizeMediaWnd (HWND hwnd);
	extern int g_skinloadedmanually;
	int writeEQfile_init(wchar_t *file, char *name, unsigned char *tab);
	extern int playlist_open(HWND hwnd);
	extern int setPlRating(int rating);
	extern int setCurrentRating(int rating);
	extern int got_ml;
	int peui_isrbuttoncaptured();
	extern void makeurlcodes(char *in, char *out);
	extern HIMAGELIST toolbarIcons;
	extern struct ITaskbarList3* pTaskbar3;
	void OnTaskbarButtonCreated(BOOL force);
	LRESULT Main_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
	LRESULT Main_OnWASystray(HWND hwnd, int id);
	LRESULT Main_OnWAMPEGEOF(HWND hwnd);
	LRESULT Main_OnSize(HWND hwnd, UINT state, int cx, int cy);
	LRESULT Main_OnTimer(HWND hwnd, UINT id);
	LRESULT Main_OnDropFiles(HWND hwnd, HDROP hdrop);
	LRESULT Main_OnCopyData(HWND hwnd, COPYDATASTRUCT *cds);
	extern int m_converting;
	void DoInstall(int is_install);
	wchar_t *ParseParameters(wchar_t *lpszCmdParam, int *bAdd, int *bBookmark, int *bHandle, int *nCmdShow, int *bCommand, int *bCmdParam, int *bAllowCompat);
	#ifdef BETA
	void ParseParametersExpired(wchar_t *lpszCmdParam);
	#endif
	void reg_associated_filetypes(int force);
	wchar_t *FindNextCommand(wchar_t *cmdLine);
	void parseCmdLine(wchar_t *cmdline, HWND hwnd);
	wchar_t *CheckFileBase(wchar_t *lpszCmdParam, HWND hwnd_other, int *exit, int mode);
	#define CheckSkin(lpszCmdParam, hwnd_other, exit) CheckFileBase(lpszCmdParam, hwnd_other, exit, 0)
	#define CheckLang(lpszCmdParam, hwnd_other, exit) CheckFileBase(lpszCmdParam, hwnd_other, exit, 1)

	void LoadPathsIni();
	LRESULT Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	extern int g_BookmarkTop, g_SkinTop, g_LangTop;
	extern int g_open_ml_item_in_pe;
	void RegisterConfigGroups();
	void CopyExtendedFileInfo(const wchar_t *source, const wchar_t *destination);

	int ResizeComboBoxDropDown(HWND hwndDlg, UINT id, const char* str, int width);
	int ResizeComboBoxDropDownW(HWND hwndDlg, UINT id, const wchar_t *str, int width);

	// creditsrend.c

	void render_togglecredits();
	void render_quit(void);
	void render_render(unsigned char *framebuffer, HDC hdc);
	void render_init(int w, int h, char *pal);

	// InW.cpp
	int InW_IsOurFile(In_Module *mod, const wchar_t *filename);
	int InW_Play(In_Module *mod, const wchar_t *filename);
	int InW_InfoBox(In_Module *mod, const wchar_t *filename, HWND parent);
	void InW_GetFileInfo(In_Module *mod, const wchar_t *filename, wchar_t *title, int *length);

	// conversions.cpp
	void Float32_To_Int16_Clip(void *destinationBuffer, signed int destinationStride, void *sourceBuffer, signed int sourceStride, unsigned int count);
	void Float32_To_Int24_Clip(void *destinationBuffer, signed int destinationStride, void *sourceBuffer, signed int sourceStride, unsigned int count);

	HANDLE DuplicateCurrentThread();

	/* dwm.cpp */
	extern BOOL atti_present;
	void DisableVistaPreview();
	void DoTheVistaVideoDance();
	void RegisterThumbnailTab(HWND hWnd);
	void UnregisterThumbnailTab(HWND hWnd);
	void OnIconicThumbnail(int width, int height);
	void OnThumbnailPreview();
	void RefreshIconicThumbnail();

	/* handler.cpp */
	int HandleFilename(const wchar_t *filename);

	/* IVideoD3DOSD.cpp */
	HMODULE FindD3DX9();

	/* AlbumArt.cpp */
	void CleanNameForPath(wchar_t *name);

	/* application.cpp */
	BOOL IsDirectMouseWheelMessage(const UINT uMsg);
	BOOL DirectMouseWheel_EnableConvertToMouseWheel(HWND hwnd, BOOL enable);
	BOOL DirectMouseWheel_IsConvertToMouseWheelEnabled(HWND hwnd);
	BOOL DirectMouseWheel_ProcessDialogMessage(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam, const int controls[], int controlslen);
	HWND ActiveChildWindowFromPoint(HWND hwnd, POINTS cursor_s, const int *controls, size_t controlsCount);

#ifdef __cplusplus
} // extern "C"
#endif
#endif