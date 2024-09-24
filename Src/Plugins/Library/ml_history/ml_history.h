#ifndef ML_HISTORY_MAIN_H
#define ML_HISTORY_MAIN_H

#include "main.h"
#include <windows.h>
#include <commctrl.h>
#include "..\..\General\gen_ml/gaystring.h"
#include "..\..\General\gen_ml/config.h"
#include "../nde/nde_c.h"

#define HISTORYVIEW_COL_LASTPLAYED 0
#define HISTORYVIEW_COL_PLAYCOUNT 1
#define HISTORYVIEW_COL_TITLE 2
#define HISTORYVIEW_COL_LENGTH 3
#define HISTORYVIEW_COL_FILENAME 4
#define HISTORYVIEW_COL_OFFSET 5

#define UPDATE_QUERY_TIMER_ID 505

extern int ml_history_tree;
HWND onTreeViewSelectChange(HWND hwnd);

int history_init();
void history_quit();

int openDb();
void closeDb(bool clear_dirty=true);
extern wchar_t g_tableDir[];
extern C_Config *g_config;

extern CRITICAL_SECTION g_db_cs;
extern nde_database_t g_db;
extern nde_table_t g_table;
extern int g_table_dirty;

inline BOOL WINAPI IsCharSpaceW(wchar_t c) { return (c == L' ' || c == L'\t'); }
inline bool IsTheW(const wchar_t *str) { if (str && (str[0] == L't' || str[0] == L'T') && (str[1] == L'h' || str[1] == L'H') && (str[2] == L'e' || str[2] == L'E') && (str[3] == L' ')) return true; else return false; }
#define SKIP_THE_AND_WHITESPACE(x) { wchar_t *save##x=(wchar_t*)x; while (IsCharSpaceW(*x) && *x) x++; if (IsTheW(x)) x+=4; while (IsCharSpaceW(*x)) x++; if (!*x) x=save##x; }

void history_bgQuery_Stop();
void history_onFile(const wchar_t *fn, int offset);
int retrieve_offset(const wchar_t *fn);
BOOL CALLBACK view_historyDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
BOOL CALLBACK view_errorinfoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

void db_setFieldInt(nde_scanner_t s, unsigned char id, int data);
void db_setFieldString(nde_scanner_t s, unsigned char id, const wchar_t *data);
void makeFilename2(const wchar_t *filename, wchar_t *filename2, int filename2_len);
void queryStrEscape(const char *p, GayString &str);
INT_PTR pluginHandleIpcMessage(int msg, INT_PTR param);

//prefs.cpp
BOOL CALLBACK PrefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

#include "history.h"

void allocRecentRecordList(historyRecordList *obj, int newsize, int granularity=512);
void emptyRecentRecordList(historyRecordList *obj);
void recentScannerRefToObjCacheNFN(nde_scanner_t s, historyRecordList *obj);
void sortResults(historyRecordList *obj, int column, int dir);
void freeRecentRecordList(historyRecordList *obj);
void saveQueryToList(nde_scanner_t s, historyRecordList *obj);

#endif ML_HISTORY_MAIN_H