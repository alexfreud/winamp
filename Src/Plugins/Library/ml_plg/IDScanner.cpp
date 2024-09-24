#include "IDScanner.h"
#include "main.h"
#include "../winamp/wa_ipc.h"
#include "api__ml_plg.h"
#include "playlist.h"
#include <assert.h>
#include <atlbase.h>
#include <strsafe.h>	// include this last

//#define DEBUG_CALLBACKS

IDScanner::IDScanner() : systemCallbacks(0)
{
	musicID=0;
	killswitch=0;
	filesComplete=0;
	filesTotal=0;
	state=STATE_IDLE;
	m_dwCookie=0;
	syscb_registered=false;

	// Create the stack that will hold our batched up files for step 4 processing
	//process_items;
}

IDScanner::~IDScanner()
{
	// ToDo: Make sure we clean up the processing stack here if we need to do that
	//Shutdown();
}

void IDScanner::Shutdown()
{
	if (musicID)
	{
		IConnectionPoint *icp = GetConnectionPoint(musicID, DIID__ICDDBMusicIDManagerEvents);
		if (icp)
		{
			icp->Unadvise(m_dwCookie);
			icp->Release(); 
		}
		musicID->Shutdown();
		musicID->Release();
	}
	musicID=0;

	// Deregister the system callbacks
	WASABI_API_SYSCB->syscb_deregisterCallback(this);
}

static HRESULT FillTag(ICddbFileInfo *info, BSTR filename)
{
	ICddbID3TagPtr infotag = NULL;
	infotag.CreateInstance(CLSID_CddbID3Tag);
	ICddbFileTag2_5Ptr tag2_5 = NULL;
	infotag->QueryInterface(&tag2_5);
	itemRecordW *record = AGAVE_API_MLDB->GetFile(filename);
	if (record && infotag && tag2_5)
	{
		wchar_t itemp[64] = {0};
		if (record->artist)
			infotag->put_LeadArtist(record->artist);

		if (record->album)
			infotag->put_Album(record->album);

		if (record->title)
			infotag->put_Title(record->title);

		if (record->genre)
			infotag->put_Genre(record->genre);

		if (record->track > 0)
			infotag->put_TrackPosition(_itow(record->track, itemp, 10));

// TODO:	if (record->tracks > 0)

		if (record->year > 0)
			infotag->put_Year(_itow(record->year, itemp, 10));

		if (record->publisher)
			infotag->put_Label(record->publisher);

		/*
		if (GetFileInfo(filename, L"ISRC", meta, 512) && meta[0])
			infotag->put_ISRC(meta);
		*/

		if (record->disc > 0)
			infotag->put_PartOfSet(_itow(record->disc, itemp, 10));

		if (record->albumartist)
			tag2_5->put_DiscArtist(record->albumartist);

		if (record->composer)
			tag2_5->put_Composer(record->composer);

		if (record->length > 0)
			tag2_5->put_LengthMS(_itow(record->length*1000, itemp, 10));

		if (record->bpm > 0)
			infotag->put_BeatsPerMinute(_itow(record->bpm, itemp, 10));

		/*
		if (GetFileInfo(filename, L"conductor", meta, 512) && meta[0])
					tag2_5->put_Conductor(meta);
					*/
		AGAVE_API_MLDB->FreeRecord(record);
	}

	if (info) info->put_Tag(infotag);

	return S_OK;
}

void IDScanner::CommitFileInfo(ICddbFileInfo *match)
{
	ICddbFileTagPtr tag;
	match->get_Tag(&tag);

	ICddbDisc2Ptr disc1, disc;
	match->get_Disc(&disc1);

	ICddbDisc2_5Ptr disc2_5;
	ICddbTrackPtr track;
	ICddbTrack2_5Ptr track2;
	if (disc1)
	{
		musicID->GetFullDisc(disc1, &disc);
		if (disc == 0)
			disc=disc1;
		disc->QueryInterface(&disc2_5);
		disc->GetTrack(1, &track);
		if (track)
			track->QueryInterface(&track2);
	}

	CComBSTR file, tagID, extData;
	match->get_Filename(&file);
	tag->get_FileId(&tagID);
	playlistMgr->FileSetTagID(file, tagID, CDDB_UPDATE_NONE);

	ICddbFileTag2_5Ptr tag2;
	tag->QueryInterface(&tag2);
	playlistMgr->FileSetFieldVal(file, gnpl_crit_field_xdev1, L"0"); // mark as done!

	if (tag2) // try tag first
		tag2->get_ExtDataSerialized(&extData);

	if (!extData && track2 != 0)  // WMA files don't get their tag object's extended data set correctly, so fallback to track extended data
		track2->get_ExtDataSerialized(&extData);

	if (!extData && disc2_5 != 0) // finally, fall back to disc extended data
		disc2_5->get_ExtDataSerialized(&extData);

	playlistMgr->FileSetExtDataSerialized(file, extData, CDDB_UPDATE_NONE);

	if (tagID)
		AGAVE_API_MLDB->SetField(file, "GracenoteFileID", tagID);
	if (extData)
		AGAVE_API_MLDB->SetField(file, "GracenoteExtData", extData);

	// TODO: if we don't have an artist & album, we might as well grab this out of the tag now

	// TODO: make thread-safe and optional
	/*
	updateFileInfo(file, L"GracenoteFileID", tagID);
	updateFileInfo(file, L"GracenoteExtData", extData);
	WriteFileInfo(file);
	*/
}

STDMETHODIMP STDMETHODCALLTYPE IDScanner::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	else if (IsEqualIID(riid, __uuidof(_ICDDBMusicIDManagerEvents)))
		*ppvObject = (_ICDDBMusicIDManagerEvents *)this;
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG STDMETHODCALLTYPE IDScanner::AddRef(void)
{
	return 1;
}

ULONG STDMETHODCALLTYPE IDScanner::Release(void)
{
	return 0;
}

HRESULT STDMETHODCALLTYPE IDScanner::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		case 1: // OnTrackIDStatusUpdate, params: CddbMusicIDStatus Status, BSTR filename, long* Abort
		{
			//long *abort = pdispparams->rgvarg[0].plVal;
			// TODO: is this safe to put here?  Or does this make us get partial results
		}
		break;
		case 2: // OnAlbumIDStatusUpdate, params: CddbMusicIDStatus Status, BSTR filename, long current_file, long total_files, long* Abort
		{
			long *abort = pdispparams->rgvarg[0].plVal;
			/*long total_files = pdispparams->rgvarg[1].lVal;
			long current_file= pdispparams->rgvarg[2].lVal;*/
			CddbMusicIDStatus status = (CddbMusicIDStatus)pdispparams->rgvarg[4].lVal;
			BSTR filename = pdispparams->rgvarg[3].bstrVal;

			// TODO: is this safe to put here?  Or does this make us get partial results
			if (killswitch)
				*abort = 1;
		}
		break;
		case 3: // OnTrackIDComplete, params: LONG match_code, ICddbFileInfo* pInfoIn, ICddbFileInfoList* pListOut
			break;
		case 4:
			break;// OnAlbumIDComplete, params: LONG match_code, ICddbFileInfoList* pListIn, ICddbFileInfoLists* pListsOut
		case 5:
			break; // OnGetFingerprint
		case 6:
			break;
		case 7://OnLibraryIDListStarted
			break;
		case 8: // OnLibraryIDListComplete
		{

			long *abort  = pdispparams->rgvarg[0].plVal;
			if (killswitch)
				*abort = 1;
			/*long FilesError =pdispparams->rgvarg[1].lVal;
			long FilesNoMatch=pdispparams->rgvarg[2].lVal;
			long FilesFuzzy=pdispparams->rgvarg[3].lVal;
			long FilesExact=pdispparams->rgvarg[4].lVal;*/
			filesTotal=pdispparams->rgvarg[5].lVal;
			filesComplete=pdispparams->rgvarg[6].lVal;
			IDispatch *disp = pdispparams->rgvarg[7].pdispVal;
			if (disp)
			{
				ICddbFileInfoList* matchList=0;
				disp->QueryInterface(&matchList);
				if (matchList)
				{
					long matchcount;
					matchList->get_Count(&matchcount);
					for (int j = 1;j <= matchcount;j++)
					{
						ICddbFileInfoPtr match;
						matchList->GetFileInfo(j, &match);
						CommitFileInfo(match);
					}

					matchList->Release();
				}
				return S_OK;
			}
			else
				return E_FAIL;

		}
		break;
		case 9: //OnLibraryIDComplete
			break;
		case 10: // OnGetFingerprintInfo
		{
			long *abort = pdispparams->rgvarg[0].plVal;
			IDispatch *disp = pdispparams->rgvarg[1].pdispVal;
			BSTR filename = pdispparams->rgvarg[2].bstrVal;

			ICddbFileInfoPtr info;
			disp->QueryInterface(&info);
			return AGAVE_API_GRACENOTE->CreateFingerprint(musicID, AGAVE_API_DECODE, info, filename, abort);
		}
		break;
		case 11: // OnGetTagInfo
		{
			pdispparams->rgvarg[0].plVal;
			IDispatch *disp = pdispparams->rgvarg[1].pdispVal;
			BSTR filename = pdispparams->rgvarg[2].bstrVal;

			ICddbFileInfoPtr info;
			disp->QueryInterface(&info);
			return FillTag(info, filename);
		}
		break;
	}
	return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE IDScanner::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	*rgdispid = DISPID_UNKNOWN;
	return DISP_E_UNKNOWNNAME;
}

HRESULT STDMETHODCALLTYPE IDScanner::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IDScanner::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

void IDScanner::SetGracenoteData(BSTR filename, BSTR tagID, BSTR extData)
{
	bool foundExt=false;
	if (extData && extData[0])
	{
		playlistMgr->FileSetExtDataSerialized(filename, extData, CDDB_UPDATE_NONE);
		CComBSTR test;
		playlistMgr->FileGetExtDataSerialized(filename, &test, 0); // benski> 24 Jul 2007 - there is a currently a bug that makes this always E_FAIL
		if (test)
			foundExt=true;
	}

	if (!foundExt) // no Extended Data (or invalid), but we have a Tag ID, we'll ask the playlist SDK to do a quick lookup
	{
		playlistMgr->FileSetTagID(filename, tagID, CDDB_UPDATE_EXTENDED);

		// write back to Media Library database
		CComBSTR extData;
		playlistMgr->FileGetExtDataSerialized(filename, &extData, 0); // benski> 24 Jul 2007 - there is a currently a bug that makes this always E_FAIL
		if (extData)
			AGAVE_API_MLDB->SetField(filename, "GracenoteExtData", extData);
	}
	else
		playlistMgr->FileSetTagID(filename, tagID, CDDB_UPDATE_NONE);
}

/*
//void IDScanner::ProcessDatabaseDifferences(Device * dev, C_ItemList * ml0,C_ItemList * itemRecordsOnDevice, C_ItemList * itemRecordsNotOnDevice, C_ItemList * songsInML,  C_ItemList * songsNotInML)
void IDScanner::ProcessDatabaseDifferences(Device * dev, C_ItemList * ml0,C_ItemList * itemRecordsOnDevice, C_ItemList * itemRecordsNotOnDevice, C_ItemList * songsInML,  C_ItemList * songsNotInML)
{
  C_ItemList device2;
	C_ItemList *device0=&device2;

   int l = dev->getPlaylistLength(0);
   for(int i=0; i<l; i++) device0->Add((void*)dev->getPlaylistTrack(0,i));


  qsort(ml0->GetAll(),ml0->GetSize(),sizeof(void*),sortfunc_ItemRecords);
	nu::qsort(device0->GetAll(), device0->GetSize(), sizeof(void*), dev, compareSongs);

  C_ItemList *ml = new C_ItemList;
  C_ItemList *device = new C_ItemList;
  
  int i,j;
  
  {
    itemRecordW * lastice = NULL;
    songid_t lastsong = NULL;
    for(i=0; i<ml0->GetSize(); i++) {
      itemRecordW * it = (itemRecordW*)ml0->Get(i);
      if(lastice) if(compareItemRecords(lastice,it)==0) continue;
      ml->Add(it);
      lastice = it;
    }
    for(i=0; i<device0->GetSize(); i++) {
      songid_t song = (songid_t)device0->Get(i);
      if(lastsong) if(compareSongs((void*)&song,(void*)&lastsong, dev)==0) continue;
      device->Add((void*)song);
      lastsong = song;
    }
  }

  i=0,j=0;
  int li = device->GetSize();
  int lj = ml->GetSize();
  while(i<li && j<lj) {
    itemRecordW * it = (itemRecordW*)ml->Get(j);
    songid_t song = (songid_t)device->Get(i);
    
    int cmp = compareItemRecordAndSongId(it,song, dev);
    if(cmp == 0) { // song on both
      if(itemRecordsOnDevice) itemRecordsOnDevice->Add(it);
      if(songsInML) songsInML->Add((void*)song);
      i++;
      j++;
    }
    else if(cmp > 0) { //song in ml and not on device
      if(itemRecordsNotOnDevice) itemRecordsNotOnDevice->Add(it);
      j++;
    }
    else { // song on device but not in ML
      if(songsNotInML) songsNotInML->Add((void*)song);
      i++;
    }
  }
  // any leftovers?
  if(songsNotInML) while(i<li) { 
    songid_t song = (songid_t)device->Get(i++);
    
    songsNotInML->Add((void*)song); 
  }
  if(itemRecordsNotOnDevice) while(j<lj) {
    itemRecordW * it = (itemRecordW *)ml->Get(j++);
    
    itemRecordsNotOnDevice->Add(it); 
  }
  delete ml; delete device;
}
*/

/*
2-pass strategy
Pass 1:  Find all tracks with Gracenote Extended Data
Pass 2:  Find File ID & extended data by fingerprint
*/
void IDScanner::ScanDatabase()
{
	filesComplete=0;
	filesTotal=0;
	state=STATE_INITIALIZING;
	killswitch=0; // reset just in case

	if (SetupPlaylistSDK())
	{
		// If this is our first time running then lets register the wasabi system callbacks for adding and removing tracks
		if (!syscb_registered)
		{
			WASABI_API_SYSCB->syscb_registerCallback(this);
			syscb_registered = true;
		}

		// Set up the MLDB manager
		InitializeMLDBManager();

		state=STATE_SYNC;
		/* Get a list of files in the media library database */
		itemRecordListW *results = AGAVE_API_MLDB->Query(L"type=0");
		if (results)
		{
			filesTotal=results->Size;
			for (int i=0;i<results->Size;i++)
			{
				if (killswitch)
					break;
				wchar_t * filename = results->Items[i].filename;
				HRESULT hr=playlistMgr->AddEntry(filename);					// Add entry to gracenote DB
				assert(SUCCEEDED(S_OK));
				if (hr == S_OK)
				{
					// Fill in Artist & Album info since we have it in the itemRecordList anyway
					// TODO: probably want to use SKIP_THE_AND_WHITESPACE here
					if (results->Items[i].album && results->Items[i].album[0])
						hr=playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_album_name, results->Items[i].album);

					if (results->Items[i].artist && results->Items[i].artist[0])
						hr=playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_track_artist_name, results->Items[i].artist);

					// Populate title information so that we have more complete data.
					if (results->Items[i].title && results->Items[i].title[0])
						hr=playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_track_name, results->Items[i].title);

					wchar_t storage[64] = {0};
					// Populate the file length in milliseconds
					if (results->Items[i].length > 0)
					{
						_itow(results->Items[i].length,storage, 10);
						hr=playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_file_length, storage);
					}

					// Populate the file size in kilobytes
					if (results->Items[i].filesize > 0)
					{
						_itow(results->Items[i].filesize,storage, 10);
						hr=playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_file_size, storage);
					}

					wchar_t *tagID = getRecordExtendedItem(&results->Items[i], L"GracenoteFileID");
					if (tagID && tagID[0])
					{
						SetGracenoteData(filename, tagID, getRecordExtendedItem(&results->Items[i], L"GracenoteExtData"));
						hr=playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_xdev1, L"0"); // done with this file!
					}
					else
						hr=playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_xdev1, L"1"); // move to phase 1
				}
				filesComplete=i+1;
			}

			AGAVE_API_MLDB->FreeRecordList(results);
			state=STATE_METADATA;
			filesComplete=0;
			if (!killswitch)
				Pass1();
			filesComplete=0;
			state=STATE_MUSICID;
			if (!killswitch)
				Pass2();
			state=STATE_DONE;
			if (!killswitch)
				AGAVE_API_MLDB->Sync();
		}
		// Set the pass 2 flag back so that on next generation we dont try to run it
		run_pass2_flag = false;
	}
	else
		state=STATE_ERROR;
}

bool IDScanner::GetStatus(long *pass, long *track, long *tracks)
{
	*pass = state;
	*track = filesComplete;
	*tracks = filesTotal;
	return true;
}

// System callback handlers from WASABI

FOURCC IDScanner::GetEventType()
{
	return api_mldb::SYSCALLBACK;
}
int IDScanner::notify(int msg, intptr_t param1, intptr_t param2)
{
	wchar_t *filename = (wchar_t *)param1;

	switch (msg)
	{
		case api_mldb::MLDB_FILE_ADDED:
		{

			DebugCallbackMessage(param1, L"File Added: '%s'");

			// Call the add/update function that needs to run on our lonesome playlist generator thread
			WASABI_API_THREADPOOL->RunFunction(plg_thread, IDScanner::MLDBFileAddedOnThread, _wcsdup(filename), (intptr_t)this, api_threadpool::FLAG_REQUIRE_COM_STA);
		}
		break;
		case api_mldb::MLDB_FILE_REMOVED_PRE:
		{
			// We are not concerned with the PRE scenario
			//DebugCallbackMessage(param1, L"File Removed PRE: '%s'");
		}
		break;
		case api_mldb::MLDB_FILE_REMOVED_POST:
		{
			WASABI_API_THREADPOOL->RunFunction(plg_thread, IDScanner::MLDBFileRemovedOnThread, _wcsdup(filename), (intptr_t)this, api_threadpool::FLAG_REQUIRE_COM_STA);
			// We will only care about the post scenario since we just need to remove the file entry from gracenote.
			//DebugCallbackMessage(param1, L"File Removed POST: '%s'");
		}
		break;
		case api_mldb::MLDB_FILE_UPDATED:
		{
			// For now we call the add method even on an update
			WASABI_API_THREADPOOL->RunFunction(plg_thread, IDScanner::MLDBFileAddedOnThread, _wcsdup(filename), (intptr_t)this, api_threadpool::FLAG_REQUIRE_COM_STA);
			//DebugCallbackMessage(param1, L"File Updated: '%s'");
		}
		break;
		case api_mldb::MLDB_CLEARED:
		{
			WASABI_API_THREADPOOL->RunFunction(plg_thread, IDScanner::MLDBClearedOnThread, 0, (intptr_t)this, api_threadpool::FLAG_REQUIRE_COM_STA);
			//DebugCallbackMessage(param1, L"MLDB Cleared");
		}
		break;
		default: return 0;
	}
	return 1;
}

// Outputs a messagebox with a filename to know when callbacks are being triggered
inline void IDScanner::DebugCallbackMessage(const intptr_t text, const wchar_t *message)
{
//#ifdef DEBUG_CALLBACKS
#if defined(DEBUG) && defined(DEBUG_CALLBACKS)
	const int size = MAX_PATH + 256;
	wchar_t *filename = (wchar_t *)text;
	wchar_t buff[size] = {0};

	//wsprintf(buff, size, message, filename);
	StringCchPrintf(buff, size, message, filename);
	MessageBox(0, buff, L"Wasabi Callback Debug", 0);
#endif
}

int IDScanner::MLDBFileAddedOnThread(HANDLE handle, void *user_data, intptr_t id)
{
	if (!playlistMgr) return 0;

	// Variables to hold information about the file query
	wchar_t *filename = (wchar_t *)user_data;
	IDScanner *scanner = (IDScanner *)id;

	wchar_t buff[1024] = {0};
	_ltow(scanner->state, buff, 10);
		
	itemRecordW *result = AGAVE_API_MLDB->GetFile(filename);
	
	HRESULT hr=playlistMgr->AddEntry(filename);	// Add the file entry to the gracenote DB
	
	assert(SUCCEEDED(S_OK));
	
	if (hr == S_OK /*&& results->Size == 1*/)
	{
		// Fill in Artist & Album info since we have it in the itemRecordList anyway
		// TODO: probably want to use SKIP_THE_AND_WHITESPACE here
		if (result->album && result->album[0])
			playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_album_name, result->album);

		if (result->artist && result->artist[0])
			playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_track_artist_name, result->artist);

		// Populate title, file size, and length information so that we have more complete data.
		if (result->title && result->title[0])
			playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_track_name, result->title);


		wchar_t storage[64] = {0};
		// Populate the file length in milliseconds
		if (result->length > 0)
		{
			_itow(result->length,storage, 10);
			playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_file_length, storage);
		}

		// Populate the file size in kilobytes
		if (result->filesize > 0)
		{
			_itow(result->filesize,storage, 10);
			playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_file_size, storage);
		}

		wchar_t *tagID = getRecordExtendedItem(result, L"GracenoteFileID");
		if (tagID && tagID[0])
		{
			scanner->SetGracenoteData(filename, tagID, getRecordExtendedItem(result, L"GracenoteExtData"));
			// We have everything we need at this point in the gracenote DB
			playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_xdev1, L"0"); // done with this file!
		}
		else		// Set it to the final scan 
		{
			playlistMgr->FileSetFieldVal(filename, gnpl_crit_field_xdev1, L"2"); // move to phase 2, we can skip phase 1 
			// Add the current file to the step 4 processing stack
			// TODOD is there a mem leak here??
			ProcessItem *itemz = new ProcessItem();
			itemz->filename = filename;
			//scanner->process_items.push(*itemz);			// Add the current item coming in to the queue

			// Set the flag so that we know we will need to rerun step 4 (pass 2) on a playlist regeneration, this only needs to happen if there is an actual change.
			run_pass2_flag = true;
		}
	}

	if (result)
		AGAVE_API_MLDB->FreeRecord(result);

	// ToDo: We need to do this free when we pop it off of the processing stack later
	free(filename);	// Clean up the user data
	return 0;
}

int IDScanner::MLDBFileRemovedOnThread(HANDLE handle, void *user_data, intptr_t id)
{
	wchar_t *filename = (wchar_t *)user_data;

	HRESULT hr = playlistMgr->DeleteFile(filename);

	if (hr == S_OK)
		return 0;
	else
		return 1;

	free(filename);	// Clean up the user data
}

int IDScanner::MLDBClearedOnThread(HANDLE handle, void *user_data, intptr_t id)
{
	return ResetDB(false);
}

int IDScanner::ProcessStackItems(void)
{
	// ToDo: Run through the stack items and process stage 4 on them
	//this->
	return 1;
}

#define CBCLASS IDScanner
START_DISPATCH;
CB(SYSCALLBACK_GETEVENTTYPE, GetEventType);
CB(SYSCALLBACK_NOTIFY, notify);
END_DISPATCH;
#undef CBCLASS