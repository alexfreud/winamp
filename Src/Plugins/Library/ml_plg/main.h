#ifndef NULLSOFT_ML_PLG_MAIN_H
#define NULLSOFT_ML_PLG_MAIN_H
#include <windows.h>
#include "playlist.h"
#include "../../General/gen_ml/ml.h"
#include "IDScanner.h"
#include "api__ml_plg.h"
#include "../winamp/wa_ipc.h"
#include "../Agave/Language/api_language.h"
#include "../nu/threadpool/api_threadpool.h"
#include <api/service/waservicefactory.h>
#include "impl_playlist.h"

#define DEFAULT_ML_QUERY "playcount = \"0\" OR lastplay < [1 month ago] AND rating != \"1\" AND rating != \"2\""
#define MAX_ML_QUERY_SIZE 8192
#define MAX_TITLE_SIZE 512

extern winampMediaLibraryPlugin plugin;

//extern int plLength;
extern int plItems;		
extern int plMinutes;		
extern int plMegabytes;
extern int plLengthType;
extern int multipleArtists;
extern int multipleAlbums;
extern int useSeed;

extern int useMLQuery;
//extern wchar_t *customMLQuery;
extern wchar_t mlQuery[];
extern Playlist seedPlaylist;
extern bool isGenerating;

extern IDScanner scanner;

extern ThreadID *plg_thread;
extern bool reset_db_flag;
extern bool run_full_scan_flag;
extern volatile bool run_pass2_flag;

extern HWND hwndDlgCurrent;

INT_PTR CALLBACK PrefsProcedure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GenerateProcedure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ViewProcedure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK BGScanProcedure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddPlaylistDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// util.cpp
int GetFileInfo(const wchar_t *filename, wchar_t *metadata, wchar_t *dest, int len);
int updateFileInfo(const wchar_t *filename, wchar_t *metadata, wchar_t *data);
void WriteFileInfo(const wchar_t *filename);

void Pass1(int *killswitch);
void Pass2(int *killswitch);

bool StartScan();
void StopScan();

int ShutdownScanner(HANDLE handle, void *user_data, intptr_t id);
int ResetDBOnThread(bool silent);		// Goes onto the plg dedicated thread when called
int ResetDB(bool silent);				// For calling functions that are already on the plg dedicated thread
int NukeDB(void);						// For nuking the DB old skool (deleting all the files by force)


// ml_plg.cpp
//void SongSelected(const wchar_t * fn, HWND parent);
void MultipleInstancesWarning(void);
HWND SongsSelected(void);
void WriteSettingsToIni(HWND hwndDlg);

// Dialog manipulation methods
// prefs.cpp & generate.cpp
void ShowErrorDlg(HWND parent);
void SetPlLengthTypeComboToItems(HWND hwndDlg, int value);
void SetPlLengthTypeComboToMinutes(HWND hwndDlg, int value);
void SetPlLengthTypeComboToMegabytes(HWND hwndDlg, int value);
int SetRadioControlsState(HWND hwndDlg);
void BoldStatusText(HWND hwndDlg);
void PopulateResults(Playlist *playlist);
void CantPopulateResults(void);
void SetMarqueeProgress(bool isMarquee);
void SetButtonsEnabledState(bool enabled_flag);
BOOL windowOffScreen(HWND hwnd, POINT pt);

#endif