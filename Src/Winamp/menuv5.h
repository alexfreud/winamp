#ifndef _MENUV5_H
#define _MENUV5_H
#ifdef __cplusplus
extern "C" {
#endif
extern HMENU v5_top_menu;
void Init_V5_Menu();
int V5_File_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_Play_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_Options_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_Windows_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_Help_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_PE_File_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_PE_Playlist_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_PE_Sort_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_PE_Help_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_ML_File_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_ML_View_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_ML_Help_Menu(HWND hwnd, int x, int y, int width, int height);
int V5_PE_ListOfPlaylists_Menu(int x, int y);
#ifdef __cplusplus
}
#endif
#endif