#pragma once
#include "WifiPlaylist.h"
/* classes and utility functions to notifying the device of playlist modifications */

void Sync_AddToPlaylist(const char *root_url, const wchar_t *playlist_id, const wchar_t *song_id);
void Sync_RemoveFromPlaylist(const char *root_url, const wchar_t *playlist_id, const wchar_t *song_id);
void Sync_DeletePlaylist(const char *root_url, const wchar_t *playlist_id);
WifiPlaylist *Sync_NewPlaylist(const char *root_url, const wchar_t *playlist_name);
void Sync_RenamePlaylist(const char *root_url, const wchar_t *playlist_id, const wchar_t *playlist_name);