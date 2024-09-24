#include "main.h"
#include "device.h"
#include "XMLString.h"
#include "api.h"
#include "../xml/obj_xml.h"
#include "../xml/ifc_xmlreaderparams.h"
#include <api/service/waServiceFactory.h>
#include "SongListDownloader.h"
#include "SongDownloader.h"
#include "RenameDownloader.h"
#include "resource.h"
#include "PlaylistSync.h"
#include "nu/AutoWide.h"
#include "images.h"
#include <mmsystem.h> // for mmioFOURCC
#include <strsafe.h>
#include <shlwapi.h>

TemplateDevice::TemplateDevice(WifiDevice *device, const char *root_url, DeviceInfo *in_device_info, TrackList *track_list, PlaylistsList *playlists_list) 
: url(strdup(root_url))
{
	DeviceInfo_Copy(&device_info, in_device_info);
	//tracks.own(*track_list);
	for (auto track : tracks)
	{
		delete track;
	}
	tracks.clear();
	tracks.assign(track_list->begin(), track_list->end());
	track_list->clear();


	//playlists.own(*playlists_list);
	for (auto playlist : playlists)
	{
		delete playlist;
	}
	playlists.clear();
	playlists.assign(playlists_list->begin(), playlists_list->end());
	playlists_list->clear();
	
	transcoder=0;
	transferQueueLength=0;

	transcoder = (Transcoder*)SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)this,PMP_IPC_GET_TRANSCODER);
	if(transcoder)
	{
		transcoder->AddAcceptableFormat(L"m4a");
		transcoder->AddAcceptableFormat(L"mp3");
		transcoder->AddAcceptableFormat(L"wav");
		transcoder->AddAcceptableFormat(L"m4v");
		transcoder->AddAcceptableFormat(L"mp4");
		transcoder->AddAcceptableFormat(L"avi");
		transcoder->AddAcceptableFormat(L"3gp");
		transcoder->AddAcceptableFormat(L"mid");
		transcoder->AddAcceptableFormat(L"ogg");
	}
}

TemplateDevice::~TemplateDevice()
{
	free(url);

	//tracks.deleteAll();
	for (auto track : tracks)
	{
		delete track;
	}
	tracks.clear();

	//playlists.deleteAll();
	for (auto playlist : playlists)
	{
		delete playlist;
	}
	playlists.clear();

	if (transcoder)
		SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)transcoder,PMP_IPC_RELEASE_TRANSCODER);
	transcoder=0;

}

__int64 TemplateDevice::getDeviceCapacityAvailable()  // in bytes
{
	return device_info.total_space - device_info.used_space;
}

__int64 TemplateDevice::getDeviceCapacityTotal()
{
	return device_info.total_space;
}

void TemplateDevice::Eject() 
{
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
}

void TemplateDevice::Close()
{
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
} 

void TemplateDevice::CloseAsync()
{
	PostMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
}

int PostFile(const char *url, const wchar_t *filename, const itemRecordW *track, obj_xml *parser, int *killswitch,
						 void (*callback)(void *callbackContext, wchar_t *status), void *context, char *new_item_id, size_t new_item_id_len);

int PostAlbumArt(const char *url, const itemRecordW *track, obj_xml *parser, int *killswitch, void (*callback)(void *callbackContext, wchar_t *status), void *context);


static int64_t FileSize64(const wchar_t * filename)
{
	WIN32_FIND_DATA f={0};
	HANDLE h = FindFirstFileW(filename,&f);
	if(h == INVALID_HANDLE_VALUE) return -1;
	FindClose(h);
	ULARGE_INTEGER i;
	i.HighPart = f.nFileSizeHigh;
	i.LowPart = f.nFileSizeLow;
	return i.QuadPart;
}

// return 0 for success, -1 for failed or cancelled
int TemplateDevice::transferTrackToDevice(const itemRecordW *track, // the track to transfer
																					void * callbackContext, //pass this to the callback
																					void (*callback)(void *callbackContext, wchar_t *status),  // call this every so often so the GUI can be updated. Including when finished!
																					songid_t * songid, // fill in the songid when you are finished
																					int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
																					)
{
	wchar_t newfile[MAX_PATH] = {0};
	wchar_t *filename = track->filename;
	bool delete_file = false;
	if(transcoder && transcoder->ShouldTranscode(track->filename)) 
	{
		wchar_t ext[10] = {0};
		int r = transcoder->CanTranscode(track->filename, ext, track->length);
		if(r != 0 && r != -1)
		{
			transcoder->GetTempFilePath(ext,newfile);
			if(transcoder->TranscodeFile(track->filename,newfile,killswitch,callback,callbackContext)) return -1;
			filename = newfile;
			delete_file=true;
		}
	}

	char new_item_id[512] = {0};
	char upload_url[555] = {0};
	StringCbPrintfA(upload_url, sizeof(upload_url), "%s/upload", url);
	if (PostFile(upload_url, filename, track, 0, killswitch, callback, callbackContext, new_item_id, 512) == 0 && new_item_id[0])
	{
		StringCbPrintfA(upload_url, sizeof(upload_url), "%s/albumart/%s", url, new_item_id);
		PostAlbumArt(upload_url, track, 0, killswitch, callback, callbackContext);
		callback(callbackContext, WASABI_API_LNGSTRINGW(IDS_COMPLETED));
		WifiTrack *new_track = new WifiTrack(new_item_id, track, filename);
		*songid = (songid_t)new_track;
		device_info.used_space += FileSize64(filename); // TODO: count album art also.  or re-query for device info
		if (delete_file)
			DeleteFile(filename);
		return 0;	
	}
	else
	{
		callback(callbackContext, L"Failed");
		if (delete_file)
			DeleteFile(filename);
		return -1;
	}	
}


int TemplateDevice::trackAddedToTransferQueue(const itemRecordW *track)
{
		// return 0 to accept, -1 for "not enough space", -2 for "incorrect format"
	  __int64 l;
  if(transcoder && transcoder->ShouldTranscode(track->filename)) 
	{
    int k = transcoder->CanTranscode(track->filename, 0, track->length);
    if(k == -1) return -2;
    if(k == 0) l = (__int64)FileSize64(track->filename);
    else l = (__int64)k;
  } 
	else 
	{
    l = FileSize64(track->filename);
  }
  int64_t avail = getDeviceCapacityAvailable();
  int64_t cmp = transferQueueLength;
  cmp += l;
  cmp += 3000000LL;
  
  if(cmp > avail)
    return -1;
  else 
	{
    transferQueueLength += l;
    return 0;
  }
} 

void TemplateDevice::trackRemovedFromTransferQueue(const itemRecordW *track)
{
	 int64_t l = FileSize64(track->filename);
  if(transcoder && transcoder->ShouldTranscode(track->filename)) 
	{
    int k = transcoder->CanTranscode(track->filename, 0, track->length);
    if(k != -1 && k != 0) l = (__int64)k;
  }
  transferQueueLength -= l;

} 

// return the amount of space that will be taken up on the device by the track (once it has been tranferred)
// or 0 for incompatable. This is usually the filesize, unless you are transcoding. An estimate is acceptable.
__int64 TemplateDevice::getTrackSizeOnDevice(const itemRecordW *track)
{
	if(transcoder && transcoder->ShouldTranscode(track->filename))
	{
		int k = transcoder->CanTranscode(track->filename, 0, track->length);
		if(k != -1 && k != 0) return k;
	}
	return track->filesize;
} 

int HTTP_Delete(const char *url);
void TemplateDevice::deleteTrack(songid_t songid)
{
	// physically remove from device. Be sure to remove it from all the playlists!
	WifiTrack *track = (WifiTrack *)songid;
	char delete_url[1024] = {0};
	StringCbPrintfA(delete_url, sizeof(delete_url), "%s/file/%S", url, track->id);
	HTTP_Delete(delete_url);
again1:
	for (WifiPlaylist::TrackList::iterator itr2=tracks.begin(); itr2 != tracks.end(); itr2++)
	{
		WifiTrack *trackitr = *itr2;
		if (!wcscmp(trackitr->id, track->id))
		{
			tracks.erase(itr2);
			if (trackitr != track)
				delete trackitr;
			goto again1; // iterator was invalidated
		}
	}

	for (PlaylistsList::iterator itr=playlists.begin();itr!=playlists.end();itr++)
	{
		WifiPlaylist *playlist = *itr;
again2:
		for (WifiPlaylist::TrackList::iterator itr2=playlist->tracks.begin(); itr2 != playlist->tracks.end(); itr2++)
		{
			WifiTrack *trackitr = *itr2;
			if (!wcscmp(trackitr->id, track->id))
			{
				playlist->tracks.erase(itr2);
				if (trackitr != track)
					delete trackitr;
				goto again2; // iterator was invalidated
			}
		}
	}
	delete track;
} 


void TemplateDevice::commitChanges()
{
	// optional. Will be called at a good time to save changes
}

int TemplateDevice::getPlaylistCount()
{
	// always at least 1. playlistnumber 0 is the Master Playlist containing all tracks.
	return 1 + (int)playlists.size();
} 

// PlaylistName(0) should return the name of the device.
void TemplateDevice::getPlaylistName(int playlistnumber, wchar_t *buf, int len)
{
	if (playlistnumber == 0)
	{
		StringCchCopy(buf, len, device_info.name);
	}
	else
	{
		WifiPlaylist *playlist = playlists[playlistnumber-1];
		StringCchCopy(buf, len, playlist->name);
	}

}
int TemplateDevice::getPlaylistLength(int playlistnumber)
{
	if (playlistnumber == 0)
	{
		size_t size = tracks.size();
		return (int)size;
	}
	else
	{
		WifiPlaylist *playlist = playlists[playlistnumber-1];
		size_t size = playlist->tracks.size();
		return (int)size;
	}
}

songid_t TemplateDevice::getPlaylistTrack(int playlistnumber,int songnum)
{
	if (playlistnumber == 0)
	{
		WifiTrack *track = tracks[songnum];
		return (songid_t)track;
	}
	else
	{
		WifiPlaylist *playlist = playlists[playlistnumber-1];
		WifiTrack *track = playlist->tracks[songnum];
		return (songid_t)track;
	}

} 

void TemplateDevice::setPlaylistName(int playlistnumber, const wchar_t *buf)
{
	if (playlistnumber == 0) // playlist 0 is the device itself
	{
		RenameDevice(url, buf);
		StringCbCopy(device_info.name, sizeof(device_info.name), buf);
	}
	else
	{
		WifiPlaylist *playlist = playlists[playlistnumber-1];
		playlist->SetName(buf);
		Sync_RenamePlaylist(url, playlist->id, buf);
	}
} 

void TemplateDevice::playlistSwapItems(int playlistnumber, int posA, int posB)
{
	// swap the songs at position posA and posB
	// TODO: implement
}

void TemplateDevice::sortPlaylist(int playlistnumber, int sortBy)
{
	// TODO: implement
}

void TemplateDevice::addTrackToPlaylist(int playlistnumber, songid_t songid)
{	
		// adds songid to the end of the playlist
	WifiTrack *track = (WifiTrack *)songid;
	if (playlistnumber == 0)
	{
		tracks.push_back(track);
	}
	else
	{
		playlists[playlistnumber - 1]->tracks.push_back(new WifiTrack(*track));
		Sync_AddToPlaylist(url, playlists[playlistnumber-1]->id, track->id);
	}

}

void TemplateDevice::removeTrackFromPlaylist(int playlistnumber, int songnum)
{
		//where songnum is the position of the track in the playlist
	if (playlistnumber == 0)
	{
		tracks.erase(tracks.begin() + songnum);
	}
	else
	{
		WifiPlaylist *playlist = playlists[playlistnumber-1];
		WifiTrack *track = playlist->tracks[songnum];
		Sync_RemoveFromPlaylist(url, playlist->id, track->id);
	}
} 

void TemplateDevice::deletePlaylist(int playlistnumber)
{
	if (playlistnumber == 0)
	{
	}
	else
	{
		WifiPlaylist *playlist = playlists[playlistnumber-1];
		Sync_DeletePlaylist(url, playlist->id);
		playlists.erase(playlists.begin() + playlistnumber-1);		
	}
}

int TemplateDevice::newPlaylist(const wchar_t *name)
{
	// create empty playlist, returns playlistnumber. -1 for failed.
	WifiPlaylist *new_playlist = Sync_NewPlaylist(url, name);
	if (new_playlist)
	{
		playlists.push_back(new_playlist);
		return (int)playlists.size();
	}
	return -1;
} 

void TemplateDevice::getTrackArtist(songid_t songid, wchar_t *buf, int len)
{
	WifiTrack *track = (WifiTrack *)songid;
	StringCchCopy(buf, len, track->artist);
}

void TemplateDevice::getTrackAlbum(songid_t songid, wchar_t *buf, int len)
{
	WifiTrack *track = (WifiTrack *)songid;
	StringCchCopy(buf, len, track->album);
}

void TemplateDevice::getTrackTitle(songid_t songid, wchar_t *buf, int len)
{
	WifiTrack *track = (WifiTrack *)songid;
	StringCchCopy(buf, len, track->title);
}

int TemplateDevice::getTrackTrackNum(songid_t songid)
{
	WifiTrack *track = (WifiTrack *)songid;
	return track->track;
}
int TemplateDevice::getTrackDiscNum(songid_t songid)
{
	// TODO: implement
	return 0;
}
void TemplateDevice::getTrackGenre(songid_t songid, wchar_t * buf, int len)
{
	buf[0]=0;
}

int TemplateDevice::getTrackYear(songid_t songid)
{
	WifiTrack *track = (WifiTrack *)songid;
	return track->year;
}

__int64 TemplateDevice::getTrackSize(songid_t songid)
{
	WifiTrack *track = (WifiTrack *)songid;
	return track->size;
}

int TemplateDevice::getTrackLength(songid_t songid)
{
	WifiTrack *track = (WifiTrack *)songid;
	return track->duration;
} 

int TemplateDevice::getTrackBitrate(songid_t songid)
{
	return 128;
} 

int TemplateDevice::getTrackPlayCount(songid_t songid)
{
	return 0;
}

int TemplateDevice::getTrackRating(songid_t songid)
{
	return 0;
}

__time64_t TemplateDevice::getTrackLastPlayed(songid_t songid)
{
	return 0;
} 

__time64_t TemplateDevice::getTrackLastUpdated(songid_t songid)
{
	WifiTrack *track = (WifiTrack *)songid;
	return track->last_updated;
} 

void TemplateDevice::getTrackAlbumArtist(songid_t songid, wchar_t *buf, int len)
{
	buf[0]=0;
}

void TemplateDevice::getTrackPublisher(songid_t songid, wchar_t *buf, int len)
{
	buf[0]=0;
}

void TemplateDevice::getTrackComposer(songid_t songid, wchar_t *buf, int len)
{
	WifiTrack *track = (WifiTrack *)songid;
	StringCchCopy(buf, len, track->composer);
}

int TemplateDevice::getTrackType(songid_t songid)
{
	return 0;
}
void TemplateDevice::getTrackExtraInfo(songid_t songid, const wchar_t *field, wchar_t *buf, int len) 
{
	// TODO: implement
	//optional
} 

// feel free to ignore any you don't support
void TemplateDevice::setTrackArtist(songid_t songid, const wchar_t *value)
{
	// TODO: implement

}
void TemplateDevice::setTrackAlbum(songid_t songid, const wchar_t *value)
{
	// TODO: implement

}
void TemplateDevice::setTrackTitle(songid_t songid, const wchar_t *value)
{
	// TODO: implement

}
void TemplateDevice::setTrackTrackNum(songid_t songid, int value)
{
	// TODO: implement

}
void TemplateDevice::setTrackDiscNum(songid_t songid, int value)
{
	// TODO: implement

}
void TemplateDevice::setTrackGenre(songid_t songid, const wchar_t *value)
{
	// TODO: implement

}
void TemplateDevice::setTrackYear(songid_t songid, int year)
{
	// TODO: implement

}
void TemplateDevice::setTrackPlayCount(songid_t songid, int value)
{
	// TODO: implement

}
void TemplateDevice::setTrackRating(songid_t songid, int value)
{
	// TODO: implement

}
void TemplateDevice::setTrackLastPlayed(songid_t songid, __time64_t value)
{
	// TODO: implement

} // in unix time format
void TemplateDevice::setTrackLastUpdated(songid_t songid, __time64_t value)
{
	// TODO: implement

} // in unix time format
void TemplateDevice::setTrackAlbumArtist(songid_t songid, const wchar_t *value)
{
	// TODO: implement

}
void TemplateDevice::setTrackPublisher(songid_t songid, const wchar_t *value)
{
	// TODO: implement

}
void TemplateDevice::setTrackComposer(songid_t songid, const wchar_t *value)
{
	// TODO: implement

}
void TemplateDevice::setTrackExtraInfo(songid_t songid, const wchar_t *field, const wchar_t *value) 
{
	// TODO: implement

} //optional

bool TemplateDevice::playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue)
{
	if(!enqueue) //clear playlist
	{ 
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
	}

	for(int i=0; i<listLength; i++) 
	{
		WifiTrack*curSong = (WifiTrack *)songidList[i];

		if (curSong)
		{
			wchar_t fn[1024] = {0};

			if (curSong->mime_type && !_wcsicmp(curSong->mime_type, L"audio/mp4"))
				StringCbPrintf(fn, sizeof(fn), L"%S/file/%s?=.m4a", url, curSong->id);
			else if (curSong->mime_type && !_wcsicmp(curSong->mime_type, L"audio/x-ms-wma"))
				StringCbPrintf(fn, sizeof(fn), L"%S/file/%s?=.wma", url, curSong->id);
			else if (curSong->mime_type && (!_wcsicmp(curSong->mime_type, L"application/ogg") || !_wcsicmp(curSong->mime_type, L"audio/ogg")))
				StringCbPrintf(fn, sizeof(fn), L"%S/file/%s?=.ogg", url, curSong->id);
			else
				StringCbPrintf(fn, sizeof(fn), L"%S/file/%s", url, curSong->id);
			enqueueFileWithMetaStructW s={0};
			s.filename = fn;
			s.title    = _wcsdup(curSong->title);
			s.ext      = NULL;
			s.length   = curSong->duration/1000;

			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
		}
		else
		{
			//char titleStr[32];
			//MessageBoxA(plugin.hwndWinampParent,WASABI_API_LNGSTRING(IDS_CANNOT_OPEN_FILE),
			//					WASABI_API_LNGSTRING_BUF(IDS_ERROR,titleStr,32),0);
		}
	}

	if(!enqueue) 
	{ 
		//play item startPlaybackAt
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,startPlaybackAt,IPC_SETPLAYLISTPOS);
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); //stop
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); //play
	}
	return true;
}

static const intptr_t encoder_blacklist[] = 
{
	mmioFOURCC('W','M','A',' '),
	mmioFOURCC('A','A','C','H'),
	mmioFOURCC('A','A','C','P'),
	mmioFOURCC('A','A','C','r'),
	mmioFOURCC('F','L','A','C'),
	mmioFOURCC('M','P','2',' '),
	mmioFOURCC('A','D','T','S'),
};


intptr_t TemplateDevice::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4)
{
	switch(param1) 
	{
	case DEVICE_SET_ICON: // icons
		{
			MLTREEIMAGE * i = (MLTREEIMAGE*)param2;
			const ModelInfo *modelInfo;
			i->hinst = plugin.hDllInstance;
			
			modelInfo = device_info.modelInfo;
			if (NULL == modelInfo || NULL == modelInfo->smallIcon)
			{
				modelInfo = GetDefaultModelInfo();
				if (NULL == modelInfo)
					break;
			}

			i->resourceId = (int)(intptr_t)modelInfo->smallIcon;
		}
		break;
	case DEVICE_CAN_RENAME_DEVICE:
		return 1;
	case DEVICE_GET_ICON:
		ModelInfo_GetIconPath(device_info.modelInfo, (int)param2, (int)param3, (wchar_t*)param4, 260, TRUE);
		break;
	case DEVICE_GET_CONNECTION_TYPE:
		{
			const char **type = (const char **)param2;
			*type = "WiFi";
			return 1;
		}
	case DEVICE_SUPPORTS_PODCASTS:
		return 1; // we don't support podcasts
	case DEVICE_GET_MODEL:
		ModelInfo_CopyDisplayName(device_info.modelInfo, (wchar_t*)param2, param3);
		return 1;
		case DEVICE_SUPPORTED_METADATA:
		{
			intptr_t supported = SUPPORTS_ARTIST | SUPPORTS_ALBUM | SUPPORTS_TITLE | SUPPORTS_TRACKNUM /*| SUPPORTS_DISCNUM | SUPPORTS_GENRE */|
				SUPPORTS_YEAR | SUPPORTS_SIZE | SUPPORTS_LENGTH /*| SUPPORTS_BITRATE */| SUPPORTS_LASTUPDATED /*| SUPPORTS_ALBUMARTIST */|
				SUPPORTS_COMPOSER /*| SUPPORTS_PUBLISHER | SUPPORTS_ALBUMART*/;
			return supported;
		}
		break;
			case DEVICE_VETO_ENCODER:
		{
			for (size_t i=0;i<sizeof(encoder_blacklist)/sizeof(*encoder_blacklist);i++)
			{
				if (param2 == encoder_blacklist[i])
					return 1;
			}
		}
		return 0;
	}
	

	// TODO: implement more
	return 0;
}

bool TemplateDevice::copyToHardDriveSupported()
{
	return true;
}

__int64 TemplateDevice::songSizeOnHardDrive(songid_t song)
{
	WifiTrack *track = (WifiTrack *)song;
	return track->size;
} 

int TemplateDevice::copyToHardDrive(songid_t song, // the song to copy
																		wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
																		void * callbackContext, //pass this to the callback
																		void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
																		int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
																		)
{
	WifiTrack *track = (WifiTrack *)song;
	char download_url[1024] = {0};
	StringCbPrintfA(download_url, sizeof(download_url), "%s/file/%S", url, track->id);
	HANDLE event = CreateEvent(0, FALSE, FALSE, 0);

	if (!_wcsicmp(track->mime_type, L"audio/mpeg"))
		wcsncat(path, L".mp3", MAX_PATH);
	else if (!_wcsicmp(track->mime_type, L"audio/mp4"))
		wcsncat(path, L".m4a", MAX_PATH);
	else if (!_wcsicmp(track->mime_type, L"audio/x-ms-wma"))
		wcsncat(path, L".wma", MAX_PATH);
	else if (!_wcsicmp(track->mime_type, L"application/ogg") || !_wcsicmp(track->mime_type, L"audio/ogg") )
		wcsncat(path, L".ogg", MAX_PATH);
	// TODO: more

	SongDownloader *song_downloader = new SongDownloader(path, event, callback, callbackContext);
	song_downloader->AddRef();
	WAC_API_DOWNLOADMANAGER->DownloadEx(download_url, song_downloader, api_downloadManager::DOWNLOADEX_CALLBACK);
	WaitForSingleObject(event, INFINITE);
		song_downloader->Release();
	return 0; // TODO: check error code
} 

// art functions
void TemplateDevice::setArt(songid_t songid, void *buf, int w, int h)
{
	//buf is in format ARGB32*
	// TODO: implement

}

pmpart_t TemplateDevice::getArt(songid_t songid)
{
	// TODO: implement
	return 0;
}

void TemplateDevice::releaseArt(pmpart_t art)
{
	// TODO: implement

}
int TemplateDevice::drawArt(pmpart_t art, HDC dc, int x, int y, int w, int h)
{
	// TODO: implement
	return 0;
}

void TemplateDevice::getArtNaturalSize(pmpart_t art, int *w, int *h)
{
	// TODO: implement

}
void TemplateDevice::setArtNaturalSize(pmpart_t art, int w, int h)
{
	// TODO: implement

}
void TemplateDevice::getArtData(pmpart_t art, void* data)
{
	// data ARGB32* is at natural size
	// TODO: implement
} 

bool TemplateDevice::artIsEqual(pmpart_t a, pmpart_t b)
{
	// TODO: implement
	return false;
}

