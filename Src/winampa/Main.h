#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define DO_LOG

extern char g_uniqueidstring[33];
extern int g_uniqueidint[4];
extern int g_oid;

// inet.c
int isInetAvailable(void);
void fixstring(char *out, char *in, int maxout);
void fixstring_slashconv(char *out, char *in, int maxout);

// systray.c
BOOL systray_add(HWND hwnd, HICON hIcon, LPWSTR lpszTip);
BOOL systray_mod(HWND hwnd, HICON hIcon, LPWSTR lpszTip);
BOOL systray_del(HWND hwnd);
BOOL systray_isintray(void);

extern int config_systray_icon,config_check_nvc;


// donvc.c
void do_check_nvc(void);

// wms.c
void kill_server(void);
char *run_server(int port, int maxcon, int maxcps);
void set_maxcps(int maxcps);
void set_maxcon(int maxcon);
extern char g_password[64];

// wmsdb.c
void wmsdbFlush(void);
int wmsdbAddFiles(char *pathlist);
void wmsdbGetFile(int p, char *data, int *len, char *meta);
int wmsdbGetNumFiles(void);

// wmsyp.c
void wmsyp_start(void);
void wmsyp_quit(void);

extern wchar_t ini_file[MAX_PATH],wms_file[MAX_PATH],indx_file[MAX_PATH],winamp_exe_file[MAX_PATH];

// mp3info.c
void mp3_getmetainfo(HANDLE hf,char *meta);

// wavinfo.c
void wav_getmetainfo(char *filename,char *meta);

#ifdef DO_LOG
void do_log_print(char *);
#else
#define do_log_print(x)
#endif

#ifdef __cplusplus
}
#endif