#ifndef NULLSOFT_ML_PLAYLISTS_CURRENTPLAYLIST_H
#define NULLSOFT_ML_PLAYLISTS_CURRENTPLAYLIST_H

bool currentPlaylist_ImportFromDisk(HWND hwnd);
bool currentPlaylist_ImportFromWinamp(HWND hwnd);
bool CurrentPlaylist_DeleteMissing();
void CurrentPlaylist_Export(HWND dlgparent);
bool CurrentPlaylist_AddLocation(HWND hwndDlg);
bool CurrentPlaylist_AddDirectory(HWND hwndDlg);
bool CurrentPlaylist_AddFiles(HWND hwndDlg);

#endif