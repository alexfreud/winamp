#ifndef NULLSOFT_ML_PLG_PLAYLIST_H
#define NULLSOFT_ML_PLG_PLAYLIST_H

#include "../gracenote/gracenote.h"
#include "../../General/gen_ml/ml.h"
#include <bfc/error.h>

#include "impl_playlist.h"

extern ICddbPlaylist25Mgr *playlistMgr;
extern ICddbMLDBManager *mldbMgr;
extern Playlist currentPlaylist;
bool SetupPlaylistSDK();
void ShutdownPlaylistSDK();
int InitializeMLDBManager(void);
int DeleteGracenoteMLDB(bool silent);
int BackupGracenoteMLDB(void);
int RestoreGracenoteMLDB(void);
void playPlaylist(Playlist &pl, bool enqueue, int startplaybackat, /*const wchar_t *seedfn,*/ int useSeed);

void GetTitleFormattingGracenote(const wchar_t *filename, ICddbPL2Result * gracenoteResult, wchar_t * buf, int len);
void GetTitleFormattingML(const wchar_t *filename, itemRecordW *mlResult, wchar_t * buf, int len);

void MoreLikeThisSong(const wchar_t *filename);
void MoreLikeTheseSongs(Playlist *pl);

typedef enum
{
	PL_NOT_INITIALIZED = 0,
	PL_ITEMS = 1,
	PL_MINUTES = 2,
	PL_MEGABYTES = 3
} PlLengthTypeEnum;

#define		PLM_Filename				L"PLM_rec_filename"
#define		PLM_Pathname				L"PLM_rec_pathname"
#define		GRACENOTE_DB_BASE_PATH		L"Gracenote"
#define		GRACENOTE_DB_BACKUP_PATH	L"Gracenote/Backup"

#endif
