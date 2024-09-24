#ifndef NULLSOFT_API_QUEUE_H
#define NULLSOFT_API_QUEUE_H

/*
** JTFE v1.0.1 Wasabi API interface
** (Released: 06/01/2010)
**
**
** This header file provides the interfaces implemented by the JTFE plugin for other plugins/services to be able
** to make use of it's queue which allows for Winamp's playback order to be overriden.
**
** To use this api assumes you know already how to make use of the wasabi service based system
** (see the more complete examples provided in the SDK).
**
**
** Example:
**
** The following psuedo code shows how to clear the current queue (if one exists) and will then add in a specifc
** file by the full path passed and also adding a file based on the position in the current playlist.
** 
** if(!WASABI_API_QUEUEMGR) ServiceBuild(WASABI_API_QUEUEMGR,QueueManagerApiGUID);
** if(WASABI_API_QUEUEMGR){
**   // Clear the queue (if one exists)
**   WASABI_API_QUEUEMGR->ClearQueue();
**
**   // Add the full file path (wchar_t_path_to_file)
**   WASABI_API_QUEUEMGR->AddItemToQueue(0,1,wchar_t_path_to_file);
**
**   // Add the first file in the playlist editor into the queue
**   WASABI_API_QUEUEMGR->AddItemToQueue(0,0,0);
** }
**
**
** Notes:
**
** This header only provides access to the functions it exports.  Some actions like the MoveQueuedItem(s) functions
** have not been implemented in this api even though they are internally implemented.  In future releases of the
** plugin it is hoped that these (and another useful/requested) apis will also be implemented and provided.
**
** Changes:
** v1.0.1 - Fixes EnableQueueAdvance(..) and IsQueueAdvanceEnabled(..) interfaces not correctly defined in this file
**        - Fixes crash when calling IsQueueAdvanceEnabled(..) (if previous issue was manually corrected)
**
*/

#if (_MSC_VER <= 1200)
typedef int intptr_t;
#endif

enum PLAYLIST_TYPE {M3U_PLAYLIST=0x0, PLS_PLAYLIST=0x1, M3U8_PLAYLIST=0x2};
enum MOVE_MODE {MOVE_TOP_OF_LIST=-2, MOVE_UP_LIST=-1, MOVE_DOWN_LIST=1, MOVE_END_OF_LIST=2, MOVE_END_TO_START=3 };

#ifdef __cplusplus

#include <bfc/dispatch.h>

class api_queue : public Dispatchable
{
protected:
	api_queue() {}
	~api_queue() {}

public:
	// main handling functions of the queue to add/remove/clear all
	BOOL AddItemToQueue(int item, int update_now, wchar_t* file);
	void RemoveQueuedItem(int item, int no_update=0);
	void ClearQueue(void);

	/*
	** handling functions to allow for querying/manipulating of the queue items
	** note: need to have a consistancy in the functions and all that...
	**
	** use GetNumberOfQueuedItems() and then loop upto that via GetQueuedItemFromIndex(..) to get item id of the queue
	*/
	int GetNumberOfQueuedItems(void);
	int GetQueuedItemFromIndex(int idx);
	wchar_t* GetQueuedItemFilePath(int item);
	int GetQueuedItemPlaylistPosition(int item);
	int IsItemQueuedMultipleTimes(int item, int test_only);	// returns how many times an item is showing in the queue

	// Note: these are to be implemented after JTFE 1.0
	//BOOL MoveQueuedItem(int item, int mode/*MOVE_MODE*/);
	//BOOL MoveQueuedItems(int* items, int mode/*MOVE_MODE*/);

	/*
	** miscellaneous actions available on the queue
	*/
	void RandomiseQueue(void);
	// will ignore the item position and match it up against the current playlist (may cause queued item position merging)
	void RefreshQueue(void);
	BOOL LoadPlaylistIntoQueue(wchar_t* playlist_file, int reset_queue);
	BOOL SaveQueueToPlaylist(wchar_t* playlist_file, int playlist_type/*PLAYLIST_TYPE*/);

	// enables/disables queue advancement and query this state
	int EnableQueueAdvance(int enabled);
	int IsQueueAdvanceEnabled(void);

public:
	DISPATCH_CODES
	{
		API_QUEUE_ADDITEMTOQUEUE = 1,
		API_QUEUE_REMOVEQUEUEDITEM = 2,
		API_QUEUE_CLEARQUEUE = 3,

		API_QUEUE_GETNUMBEROFQUEUEDITEMS = 10,
		API_QUEUE_GETQUEUEDITEMFROMINDEX = 11,
		API_QUEUE_GETQUEUEDITEMFILEPATH = 12,
		API_QUEUE_GETQUEUEDITEMPLAYLISTPOSITION = 13,
		API_QUEUE_ISITEMQUEUEDMULTIPLETIMES = 14,

		// Note: to be implemented after JTFE 1.0
		//API_QUEUE_MOVEQUEUEDITEM = 15,
		//API_QUEUE_MOVEQUEUEDITEMS = 16,

		API_QUEUE_RANDOMISEQUEUE = 20,
		API_QUEUE_REFRESHQUEUE = 21,
		API_QUEUE_LOADPLAYLISTINTOQUEUE = 22,
		API_QUEUE_SAVEQUEUETOPLAYLIST = 23,

		API_QUEUE_ENABLEQUEUEADVANCE = 30,
		API_QUEUE_ISQUEUEADVANCEENABLED = 31
	};
};

inline BOOL api_queue::AddItemToQueue(int item, int update_now, wchar_t* file)
{
	return _call(API_QUEUE_ADDITEMTOQUEUE, (BOOL)0, item, update_now, file);
}

inline void api_queue::RemoveQueuedItem(int item, int no_update)
{
	_voidcall(API_QUEUE_REMOVEQUEUEDITEM, item, no_update);
}

inline void api_queue::ClearQueue(void)
{
	_voidcall(API_QUEUE_CLEARQUEUE);
}

inline int api_queue::GetNumberOfQueuedItems(void)
{
	return _call(API_QUEUE_GETNUMBEROFQUEUEDITEMS, (int)0);
}

inline int api_queue::GetQueuedItemFromIndex(int idx)
{
	return _call(API_QUEUE_GETQUEUEDITEMFROMINDEX, (int)0, idx);
}

inline wchar_t* api_queue::GetQueuedItemFilePath(int item)
{
	return _call(API_QUEUE_GETQUEUEDITEMFILEPATH, (wchar_t*)0, item);
}

inline int api_queue::GetQueuedItemPlaylistPosition(int item)
{
	return _call(API_QUEUE_GETQUEUEDITEMPLAYLISTPOSITION, (int)0, item);
}

inline int api_queue::IsItemQueuedMultipleTimes(int item, int test_only)
{
	return _call(API_QUEUE_ISITEMQUEUEDMULTIPLETIMES, (int)0, item, test_only);
}

inline void api_queue::RandomiseQueue(void)
{
	_voidcall(API_QUEUE_RANDOMISEQUEUE);
}

inline void api_queue::RefreshQueue(void)
{
	_voidcall(API_QUEUE_REFRESHQUEUE);
}

inline int api_queue::LoadPlaylistIntoQueue(wchar_t* playlist_file, int reset_queue)
{
	return _call(API_QUEUE_LOADPLAYLISTINTOQUEUE, (int)0, playlist_file, reset_queue);
}

inline int api_queue::SaveQueueToPlaylist(wchar_t* playlist_file, int playlist_type)
{
	return _call(API_QUEUE_SAVEQUEUETOPLAYLIST, (int)0, playlist_file, playlist_type);
}

inline int api_queue::EnableQueueAdvance(int enabled)
{
	return _call(API_QUEUE_ENABLEQUEUEADVANCE, (int)0, enabled);
}

inline int api_queue::IsQueueAdvanceEnabled(void)
{
	return _call(API_QUEUE_ISQUEUEADVANCEENABLED, (int)0);
}

#endif

// {7DC8C14F-F27F-48e8-A3D1-602BB3196E40}
static const GUID QueueManagerApiGUID = 
{ 0x7dc8c14f, 0xf27f, 0x48e8, { 0xa3, 0xd1, 0x60, 0x2b, 0xb3, 0x19, 0x6e, 0x40 } };


#endif