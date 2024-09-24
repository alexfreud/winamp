#ifndef NULLSOFT_GRACENOTE_API_GRACENOTE_H
#define NULLSOFT_GRACENOTE_API_GRACENOTE_H

/* benski>
 * This API is facilitate initialization of Gracenote objects
 * as well as a few common functions
 *
 * It is _NOT_ meant to be a wrapper around the Gracenote API
 * It simply ensure that all plugins create objects with the same
 * configuration, which improves code maintainability and reduces
 * compiled file sizes.
 */

#include <bfc/dispatch.h>
#include "gracenote.h"

class api_decodefile;

class api_gracenote : public Dispatchable
{
protected:
	api_gracenote() {}
	~api_gracenote() {}

public:
	/* These return Gracenote COM objects.  Since COM handles referencing counting,
	 * you can simply call their Release() method when you are done.
	 */

	ICDDBControl2 *GetCDDB();
	ICDDBMusicIDManager3 *GetMusicID(); // makes a new instance, always
	//ICddbPlaylist25Mgr *GetPlaylistManager(); // makes a new instance, always
	//int GetPlaylistManagerWithMLDBManager(ICddbPlaylist25Mgr **playlistMgr, ICddbMLDBManager **mldbMgr); // makes a new instance, always
	int GetPlaylistManager(ICddbPlaylist25Mgr **playlistMgr, ICddbMLDBManager **mldbMgr); // makes a new instance, always
	ICddbMLDBManager *GetMLDBManager();
	void ReleasePlaylistManager();

	/* Some utility functions */
	HRESULT CreateFingerprint(ICDDBMusicIDManager *musicID, api_decodefile *decodeApi, ICddbFileInfo *info, const wchar_t *filename, long *killswitch);

	DISPATCH_CODES
	{
		API_GRACENOTE_GETCDDB = 10,
		API_GRACENOTE_GETMUSICID=20,
		//API_GRACENOTE_GETPLAYLISTMGR=30,				// Older codes can be removed
		//API_GRACENOTE_GETPLAYLISTMGRWITHMLDBMGR=40,	// ""
		API_GRACENOTE_GETPLAYLISTMGR=40,
		API_GRACENOTE_GETMLDBMGR=50,
		API_GRACENOTE_CREATEFINGERPRINT=1000,
	};
};

inline ICDDBControl2 *api_gracenote::GetCDDB()
{
	return _call(API_GRACENOTE_GETCDDB, (ICDDBControl2 *)0);
}
inline ICDDBMusicIDManager3 *api_gracenote::GetMusicID()
{
	return _call(API_GRACENOTE_GETMUSICID, (ICDDBMusicIDManager3 *)0);
}

/*inline ICddbPlaylist25Mgr *api_gracenote::GetPlaylistManager()
{
	return _call(API_GRACENOTE_GETPLAYLISTMGR, (ICddbPlaylist25Mgr *)0);
}

inline int api_gracenote::GetPlaylistManagerWithMLDBManager(ICddbPlaylist25Mgr **playlistMgr, ICddbMLDBManager **mldbMgr)
{
	return _call(API_GRACENOTE_GETPLAYLISTMGRWITHMLDBMGR, 0, playlistMgr, mldbMgr);
}*/

inline int api_gracenote::GetPlaylistManager(ICddbPlaylist25Mgr **playlistMgr, ICddbMLDBManager **mldbMgr)
{
	return _call(API_GRACENOTE_GETPLAYLISTMGR, 0, playlistMgr, mldbMgr);
}

inline ICddbMLDBManager *api_gracenote::GetMLDBManager()
{
	return _call(API_GRACENOTE_GETMLDBMGR, (ICddbMLDBManager *)0);
}

inline HRESULT api_gracenote::CreateFingerprint(ICDDBMusicIDManager *musicID, api_decodefile *decodeApi, ICddbFileInfo *info, const wchar_t *filename, long *killswitch)
{
	return _call(API_GRACENOTE_CREATEFINGERPRINT, E_FAIL, musicID, decodeApi, info, filename, killswitch);
}


// {877D90AB-FAC1-4366-B3B0-EB177F42CFCE}
static const GUID gracenoteApiGUID =
  { 0x877d90ab, 0xfac1, 0x4366, { 0xb3, 0xb0, 0xeb, 0x17, 0x7f, 0x42, 0xcf, 0xce } };

#endif