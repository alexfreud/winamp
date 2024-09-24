#include "DeviceView.h"
#include <time.h>
#include <shlwapi.h>
#include "SkinnedListView.h"
#include "metadata_utils.h"
#include "IconStore.h"
#include "api__ml_pmp.h"
#include "resource1.h"
#include "main.h"

static void TransferCallback(void * callBackContext, wchar_t * status);
extern void TransfersListUpdateItem(CopyInst * item);
void TransfersListUpdateItem(CopyInst * item, DeviceView *view);
extern void TransfersListPushPopItem(CopyInst * item);
void TransfersListPushPopItem(CopyInst * item, DeviceView *view);

extern HWND mainMessageWindow;
extern HANDLE hMainThread;

/*
How to add new ways of copying files.
Subclass CopyInst, over-ride CopyAction and Equals
Optionally over-ride PreCopyAction, PostCopyAction and Cancelled

Add to transfer queue as normal. 
*/

SongCopyInst::SongCopyInst(DeviceView * dev, itemRecordW * song0) 
{
	usesPreCopy = false;
	usesPostCopy = true;
	this->dev = dev;
	equalsType = 0;
	res = 0;
	copyRecord(&song, song0);
	songid = NULL;
	status = STATUS_WAITING;
	// status caption
	WASABI_API_LNGSTRINGW_BUF(IDS_WAITING, statusCaption, sizeof(statusCaption)/sizeof(wchar_t));

	SYSTEMTIME system_time;
	GetLocalTime(&system_time);
	GetTimeFormat(LOCALE_INVARIANT, NULL, &system_time, NULL, lastChanged, sizeof(lastChanged)/sizeof(wchar_t));

	// make the itemRecord a little safer
	if(!song.album) song.album = _wcsdup(L"");
	if(!song.artist) song.artist = _wcsdup(L"");
	if(!song.title) song.title = _wcsdup(L"");
	if(!song.genre) song.genre = _wcsdup(L"");
	if(!song.filename) song.filename = _wcsdup(L"");
	if(!song.comment) song.comment = _wcsdup(L"");
	if(!song.albumartist) song.albumartist = _wcsdup(L"");
	if(!song.publisher) song.publisher = _wcsdup(L"");
	if(!song.composer) song.composer = _wcsdup(L"");

	// track caption
	lstrcpyn(trackCaption, song.artist, 128);

	int l = lstrlen(trackCaption);
	if(128 - l > 1) lstrcpyn(trackCaption + l, L" - ", 128-l);
	l = lstrlen(trackCaption);
	if(128 - l > 1) lstrcpyn(trackCaption + l, song.title, 128 - l);

	// type caption
	WASABI_API_LNGSTRINGW_BUF(IDS_TRANSFER, typeCaption, sizeof(typeCaption)/sizeof(wchar_t));

	// TODO fill out when other actions become done
	// source and destination details
	//this->dev->GetDisplayName(sourceDevice, sizeof(sourceDevice)/sizeof(wchar_t));
	WASABI_API_LNGSTRINGW_BUF(IDS_LOCAL_MACHINE, sourceDevice, ARRAYSIZE(sourceDevice));
	this->dev->GetDisplayName(destDevice, ARRAYSIZE(destDevice));

	lstrcpynW(sourceFile, song.filename, sizeof(sourceFile)/sizeof(wchar_t));
}

SongCopyInst::~SongCopyInst() 
{
	freeRecord(&song);
}

void SongCopyInst::Cancelled() {
	// helps us to do appropriate handling
	if (status == STATUS_TRANSFERRING)
	{
		dev->threadKillswitch = -2;
	}
	status = STATUS_CANCELLED;
	WASABI_API_LNGSTRINGW_BUF(IDS_UPLOAD_CANCELLED, statusCaption, ARRAYSIZE(statusCaption));

	SYSTEMTIME system_time = {0};
	GetLocalTime(&system_time);
	GetTimeFormat(LOCALE_INVARIANT, NULL, &system_time, NULL, lastChanged, ARRAYSIZE(lastChanged));

	dev->dev->trackRemovedFromTransferQueue(&song);
}

static void TransferCallback(void * callBackContext, wchar_t * status) 
{
	CopyInst * c = (CopyInst *)callBackContext;
	if(!wcscmp(status, c->statusCaption)) return;

	if (status && *status) lstrcpyn(c->statusCaption, status, sizeof(c->statusCaption)/sizeof(wchar_t));
	TransfersListUpdateItem(c);
	TransfersListUpdateItem(c, c->dev);
	int pc=0;
	// copes with 'transferring %'
	if(swscanf(status,L"%*s %d%%",&pc))
	{
		if (c->dev->isCloudDevice) cloudTransferProgress = pc;
		else c->dev->currentTransferProgress = pc;
	}
	// copes with 'transferring (%)'
	else if(swscanf(status,L"%*s %*1c %d%%",&pc))
	{
		if (c->dev->isCloudDevice) cloudTransferProgress = pc;
		else c->dev->currentTransferProgress = pc;
	}
	c->dev->UpdateSpaceInfo(TRUE, TRUE);
}

bool SongCopyInst::CopyAction()
{
	int r = dev->dev->transferTrackToDevice(&song,this,TransferCallback,&songid,&dev->threadKillswitch);
	if (r==0 && AGAVE_API_STATS)
		AGAVE_API_STATS->IncrementStat(api_stats::PMP_TRANSFER_COUNT);

	dev->dev->trackRemovedFromTransferQueue(&song);
	return r!=0;
}

void SongCopyInst::PostCopyAction()
{
	if(status == STATUS_DONE && songid) 
	{
		dev->dev->addTrackToPlaylist(0, songid);

		if (dev->metadata_fields & SUPPORTS_ALBUMART)
		{
			int w,h;
			ARGB32 *bits;
			if (AGAVE_API_ALBUMART->GetAlbumArt_NoAMG(song.filename, L"cover", &w, &h, &bits) == ALBUMART_SUCCESS)
			{
				dev->dev->setArt(songid,bits,w,h);
				WASABI_API_MEMMGR->sysFree(bits);
			}
		}
	}
}

bool SongCopyInst::Equals(CopyInst * b) {
	if(this->equalsType == b->equalsType) {
		SongCopyInst * c = (SongCopyInst*)b;
		bool ret = (compareItemRecords(&this->song,&c->song) == 0);

		// for cloud then we do some extra checks to allow different formats
		// and also for sending the same file to a different cloud device...
		if (c->dev->isCloudDevice)
		{
			const wchar_t * mime_1 = getRecordExtendedItem(&c->song, L"mime");
			const wchar_t * mime_2 = getRecordExtendedItem(&this->song, L"mime");
			int mime_match = lstrcmpiW(mime_1 ? mime_1 : L"", mime_2 ? mime_2 : L"");
			int device_match = lstrcmpiW(c->destDevice ? c->destDevice : L"", this->destDevice ? this->destDevice : L"");

			if (!device_match)
			{
				if (!mime_match)
				{
					return ret;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		return ret;
	}
	return false;
}

PlaylistCopyInst::PlaylistCopyInst(DeviceView * dev, itemRecordW * song, wchar_t * plName0, int plid0) : SongCopyInst(dev,song) 
{
	lstrcpyn(plName,plName0,255);
	plid=plid0;
	plAddSongs = NULL;
	usesPreCopy = false;
	usesPostCopy = true;
}

PlaylistCopyInst::~PlaylistCopyInst() 
{
	SongCopyInst::~SongCopyInst();
	if(plAddSongs) delete plAddSongs;
}

bool PlaylistCopyInst::PreCopyAction() {return false;}

void PlaylistCopyInst::PostCopyAction() {
	SongCopyInst::PostCopyAction();
	if(!plAddSongs || plid == -1) return;
	if(plid >= dev->dev->getPlaylistCount()) return;
	int l = plAddSongs->GetSize();
	wchar_t pln[256] = {0};
	dev->dev->getPlaylistName(plid,pln,255);
	if(wcscmp(pln,plName) == 0) {
		if(status == STATUS_DONE && songid) dev->dev->addTrackToPlaylist(plid,songid);
		for(int i=0; i < l; i++) {
			songid_t s = (songid_t)plAddSongs->Get(i);
			if(s) dev->dev->addTrackToPlaylist(plid,s);
		}
	}
}

ReverseCopyInst::ReverseCopyInst(DeviceView * dev, const wchar_t * filepath, const wchar_t * format, songid_t song, bool addToLibrary, bool uppercaseext) : uppercaseext(uppercaseext) {
	usesPreCopy = false;
	usesPostCopy = addToLibrary;
	this->dev = dev;
	equalsType = 1;
	this->songid = song;
	status = STATUS_WAITING;
	WASABI_API_LNGSTRINGW_BUF(IDS_WAITING, statusCaption, sizeof(statusCaption)/sizeof(wchar_t));
	WASABI_API_LNGSTRINGW_BUF(IDS_COPY_TO_LIBRARY, typeCaption, sizeof(typeCaption)/sizeof(wchar_t));
	dev->dev->getTrackArtist(song,trackCaption,128);
	int l = lstrlen(trackCaption);
	if(128 - l > 1) lstrcpyn(trackCaption + l,L" - ",128-l);
	l = lstrlen(trackCaption);
	if(128 - l > 1) dev->dev->getTrackTitle(song,trackCaption + l,128 - l);

	// find path for song
	lstrcpyn(path,format,2036);
	FixReplacementVars(path,2036,dev->dev,song);
	PathCombine(path,filepath,path);
}

bool ReverseCopyInst::Equals(CopyInst *b) {
	if(this->equalsType == b->equalsType) {
		ReverseCopyInst* c = (ReverseCopyInst*)b;
		return (c->dev == this->dev) && (c->songid == this->songid);
	}
	return false;
}

bool ReverseCopyInst::CopyAction() { //Return true if failed.
	wchar_t * lastslash = wcsrchr(path,L'\\');
	if(!lastslash) {
		WASABI_API_LNGSTRINGW_BUF(IDS_INVALID_PATH, statusCaption, sizeof(statusCaption)/sizeof(wchar_t));
		return true;
	}
	*lastslash=0;
	if(RecursiveCreateDirectory(path)) {
		WASABI_API_LNGSTRINGW_BUF(IDS_INVALID_PATH, statusCaption, sizeof(statusCaption)/sizeof(wchar_t));
		return true;
	}
	*lastslash=L'\\';
	// path created, copy file over.
	int r = dev->dev->copyToHardDrive(songid, path, this, TransferCallback, &dev->threadKillswitch);
	return r!=0;
}

void ReverseCopyInst::PostCopyAction() 
{
	itemRecordW ice={0};
	filenameToItemRecord(path,&ice);

	ice.rating = dev->dev->getTrackRating(songid);
	ice.playcount = dev->dev->getTrackPlayCount(songid);
	ice.lastplay = dev->dev->getTrackLastPlayed(songid);
	ice.lastupd = dev->dev->getTrackLastUpdated(songid);
	ice.type = dev->dev->getTrackType(songid);

	SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&ice,ML_IPC_DB_ADDORUPDATEITEMW);
	freeRecord(&ice);
}

ReversePlaylistCopyInst::ReversePlaylistCopyInst(DeviceView * dev, const wchar_t * filepath, const wchar_t * format, songid_t song, wchar_t * playlistFile0, wchar_t * playlistName0, bool last,bool addToLibrary)
	: ReverseCopyInst(dev,filepath,format,song,addToLibrary,false), last(last) {
	lstrcpyn(playlistFile,playlistFile0,MAX_PATH);
	lstrcpyn(playlistName,playlistName0,128);
}

bool ReversePlaylistCopyInst::CopyAction() 
{
	bool r = ReverseCopyInst::CopyAction();
	return r;
}

void ReversePlaylistCopyInst::PostCopyAction()
{
	ReverseCopyInst::PostCopyAction();
	FILE * f = _wfopen(playlistFile,L"at");
	if(f) {
		fputws(L"#EXTINF:",f);
		wchar_t buf[100] = {0};
		wsprintf(buf,L"%d",dev->dev->getTrackLength(songid)/1000);
		fputws(buf,f);
		fputws(L",",f);
		wchar_t title[2048] = {0};
		getTitle(dev->dev,songid,path,title,2048);
		fputws(title,f);
		fputws(L"\n",f);
		fputws(path,f);
		fputws(L"\n",f);
		fclose(f);
	}
	if(last) {
		mlAddPlaylist a = {sizeof(mlAddPlaylist),playlistName,playlistFile,PL_FLAG_SHOW | PL_FLAGS_IMPORT,-1,-1};
		SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&a,ML_IPC_PLAYLIST_ADD);
		_wunlink(playlistFile);
	}
}

// HERE BE DRAGONS
static VOID CALLBACK APC_PreCopy(ULONG_PTR dwParam) {
	CopyInst * a = (CopyInst *)dwParam;
	a->res = a->PreCopyAction()?2:1;
}

static VOID CALLBACK APC_PostCopy(ULONG_PTR dwParam) {
	CopyInst * a = (CopyInst *)dwParam;
	a->PostCopyAction();
	a->res = 1;
}

void CALLBACK TransferNavTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, ULONG elapsed)
{
	TransferContext *context = (TransferContext *)eventId;
	if (context && context->dev->queueTreeItem)
	{
		NAVITEM item;
		item.cbSize = sizeof(NAVITEM);
		item.hItem = context->dev->queueTreeItem;
		item.iSelectedImage = item.iImage = icon_store.GetQueueIcon(context->dev->queueActiveIcon);
		context->dev->queueActiveIcon = (context->dev->queueActiveIcon + 1) % 4;
		item.mask = NIMF_IMAGE | NIMF_IMAGESEL;
		MLNavItem_SetInfo(plugin.hwndLibraryParent, &item);
	}
}

void TransferContext::DoOneTransfer(HANDLE handle)
{
	if (TryEnterCriticalSection(&transfer_lock))
	{
		if(dev->threadKillswitch == 1)
		{
			WASABI_API_THREADPOOL->RemoveHandle(transfer_thread, handle);
			dev->threadKillswitch = 100;
			SetEvent(killer);
			LeaveCriticalSection(&transfer_lock);
			return;
		}

		if (IsPaused())
		{
			LeaveCriticalSection(&transfer_lock);
			return;
		}

		LinkedQueue * txQueue = getTransferQueue(this->dev);
		CopyInst * c = (txQueue ? (CopyInst *)txQueue->Peek() : NULL);
		if (c)
		{
			if (c->res != 2 && c->status != STATUS_CANCELLED)
			{
				c->status = STATUS_TRANSFERRING;
				start = time(NULL);
				TransferCallback(c, WASABI_API_LNGSTRINGW_BUF(IDS_STARTING_TRANSFER, c->statusCaption, sizeof(c->statusCaption)/sizeof(wchar_t)));
				c->res = 0;
				if(c->usesPreCopy) 
					SynchronousProcedureCall(APC_PreCopy,(ULONG_PTR)c);
			}
			if(dev->threadKillswitch)
			{
				WASABI_API_THREADPOOL->RemoveHandle(transfer_thread, handle);
				dev->threadKillswitch = 100;
				SetEvent(killer);
				LeaveCriticalSection(&transfer_lock);
				return;
			}
			if(c->res == 2)
			{ // dupe
				WASABI_API_LNGSTRINGW_BUF((dev->isCloudDevice ? IDS_ALREADY_UPLOADED : IDS_DUPLICATE), c->statusCaption, sizeof(c->statusCaption)/sizeof(wchar_t));
				c->status = STATUS_DONE;
			}
			else if (c->status != STATUS_CANCELLED)
			{
				// do the transfer
				int r = c->CopyAction();
				c->status = (r == -1 ? STATUS_ERROR : STATUS_DONE);

				SYSTEMTIME system_time = {0};
				GetLocalTime(&system_time);
				GetTimeFormat(LOCALE_INVARIANT, NULL, &system_time, NULL, c->lastChanged, sizeof(c->lastChanged)/sizeof(wchar_t));

				// Now do whatever needs to be done post-copy (add to playlist or whatever)
				c->res = 0;
				if(c->usesPostCopy && c->status == STATUS_DONE)
					SynchronousProcedureCall(APC_PostCopy,(ULONG_PTR)c);
				// now work out the moving average time per transfer
				end = time(NULL);
				if(c->status == STATUS_DONE)
				{
					times[numTransfers % AVERAGEBASIS] = (int)((long)end - (long)start);
					numTransfers++;
				}
				int n = min(AVERAGEBASIS,numTransfers);
				if(n > 0)
				{
					int t = 0;
					for(int i = 0; i < n; i++) t += times[i];
					dev->transferRate = ((double)t) / ((double)n);
				}
			}
			if(dev->threadKillswitch == 2)
			{ // a transfer has been cancelled part way through
				dev->threadKillswitch = 0;
				delete txQueue->Poll();
			}
			else
			{
				LinkedQueue * finishedTX = getFinishedTransferQueue(this->dev);
				if (finishedTX)
				{
					txQueue->lock();
					finishedTX->lock();
					finishedTX->Offer(txQueue->Poll());
					finishedTX->unlock();
					txQueue->unlock();
					TransfersListPushPopItem(c);
					TransfersListPushPopItem(c, dev);
				}
			}
			dev->commitNeeded = true;
			if (dev->isCloudDevice) cloudTransferProgress = 0;
			else dev->currentTransferProgress = 0;

			LeaveCriticalSection(&transfer_lock);
			SetEvent(handle);
		}
		else
		{
			if(dev->commitNeeded) 
				PostMessage(mainMessageWindow,WM_TIMER,COMMITTIMERID,0);
			if (dev->isCloudDevice) cloudTransferProgress = 0;
			else dev->currentTransferProgress = 0;
			dev->UpdateActivityState();
			LeaveCriticalSection(&transfer_lock);
		}
	}
}

int TransferThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	TransferContext *context = (TransferContext *)user_data;
	SetTimer(plugin.hwndLibraryParent, (UINT_PTR)user_data, 1000, TransferNavTimer);
	// allows cancels to continue even if a cloud device upload had been cancelled
	if (context->dev->threadKillswitch == -2) context->dev->threadKillswitch = 0;
	context->DoOneTransfer(handle);
	KillTimer(plugin.hwndLibraryParent, (UINT_PTR)user_data);
	return 0;
}

bool TransferContext::IsPaused()
{
	return (paused_all || paused);
}

void TransferContext::Pause()
{
	if (1 == InterlockedIncrement(&paused))
	{
		if(dev->commitNeeded)
			PostMessage(mainMessageWindow,WM_TIMER,COMMITTIMERID,0);

		SetEvent(notifier);
	}
}

void TransferContext::Resume()
{
	if (0 == InterlockedDecrement(&paused))
	{
		SetEvent(notifier);
	}
}

bool TransferContext::IsAllPaused()
{
	return paused_all?true:false;
}

void TransferContext::PauseAll()
{
	InterlockedIncrement(&paused_all);
}

void TransferContext::ResumeAll()
{
	InterlockedDecrement(&paused_all);
}