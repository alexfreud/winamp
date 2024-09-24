#ifndef NULLSOFT_GRACENOTE_GRACENOTEAPI_H
#define NULLSOFT_GRACENOTE_GRACENOTEAPI_H

#include "api_gracenote.h"
#include "../nu/AutoLock.h"

class GracenoteApi : public api_gracenote
{
public:
	GracenoteApi();
	~GracenoteApi();
	ICDDBControl2 *GetCDDB();
	ICDDBMusicIDManager3 *GetMusicID(); // TODO: might need to instantiate separate objects because each manager can only have 1 event handler
	//ICddbPlaylist25Mgr *GetPlaylistManager();
	//int GetPlaylistManagerWithMLDBManager(ICddbPlaylist25Mgr **playlistMg, ICddbMLDBManager **mldbMgr);
	int GetPlaylistManager(ICddbPlaylist25Mgr **playlistMg, ICddbMLDBManager **mldbMgr);
	ICddbMLDBManager *GetMLDBManager();
	void Close();

	/* Some utility functions */
	HRESULT CreateFingerprint(ICDDBMusicIDManager *musicID, api_decodefile *decodeApi, ICddbFileInfo *info, const wchar_t *filename, long *killswitch);
private:
	bool cddbInitialized, playlistInitialized;

	ICDDBControl2 *pCDDBControl;

	Nullsoft::Utility::LockGuard cddbGuard;
protected:
	RECVS_DISPATCH;
};

extern GracenoteApi gracenoteApi;
#endif