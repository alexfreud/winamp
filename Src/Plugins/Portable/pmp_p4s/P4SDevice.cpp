#include "P4SDevice.h"
#include <time.h>
#include "msWMDM_i.c"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../WAT/wa_logger.h"
#include "MyProgress.h"

#include "WMDRMDeviceApp_i.c"

extern C_ItemList devices;
extern HANDLE killEvent;
extern CRITICAL_SECTION csTransfers;

#define plext L"pla"

BOOL FormatResProtocol(const wchar_t *resourceName, const wchar_t *resourceType, wchar_t *buffer, size_t bufferMax);

static BYTE* GetMetadataItem(IWMDMStorage4 * store, const WCHAR * name);
static IWMDMStorage4* GetOrCreateFolder(IWMDMStorage4 * store, wchar_t * name, P4SDevice * dev=NULL, bool album=false, const itemRecordW * item=NULL);

// from http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wmdm/htm/wmdm_format_capability.asp
void FreeFormatCapability(WMDM_FORMAT_CAPABILITY formatCap)
{
    // Loop through all configurations.
    for (UINT i=0; i < formatCap.nPropConfig; i++) 
    {
        // Loop through all descriptions of a configuration and delete
        // the values particular to that description type.
        for (UINT j=0; j < formatCap.pConfigs[i].nPropDesc; j++) 
        {
            switch (formatCap.pConfigs[i].pPropDesc[j].ValidValuesForm)
            {
                case WMDM_ENUM_PROP_VALID_VALUES_ENUM:
                    for (UINT k=0; k < formatCap.pConfigs[i].pPropDesc[j].ValidValues.EnumeratedValidValues.cEnumValues; k++)
                    {
                        PropVariantClear (&(formatCap.pConfigs[i].pPropDesc[j].ValidValues.EnumeratedValidValues.pValues[k]));
                    }
                    CoTaskMemFree(formatCap.pConfigs[i].pPropDesc[j].ValidValues.EnumeratedValidValues.pValues);
                    break;
                case WMDM_ENUM_PROP_VALID_VALUES_RANGE:
                    PropVariantClear (&(formatCap.pConfigs[i].pPropDesc[j].ValidValues.ValidValuesRange.rangeMin));
                    PropVariantClear (&(formatCap.pConfigs[i].pPropDesc[j].ValidValues.ValidValuesRange.rangeMax));
                    PropVariantClear (&(formatCap.pConfigs[i].pPropDesc[j].ValidValues.ValidValuesRange.rangeStep));
                    break;
                case WMDM_ENUM_PROP_VALID_VALUES_ANY:
                    // No dynamically allocated memory for this value.
                default:
                    break;
            }

            // Free the memory for the description name.
            CoTaskMemFree(formatCap.pConfigs[i].pPropDesc[j].pwszPropName);
        }
        // Free the memory holding the array of description items for this configuration.
        CoTaskMemFree(formatCap.pConfigs[i].pPropDesc);
    }

    // Free the memory pointing to the array of configurations.
    CoTaskMemFree(formatCap.pConfigs);
    formatCap.nPropConfig = 0;
}

static HRESULT GetFormatCaps(WMDM_FORMATCODE formatCode, IWMDMDevice3* pDevice)
{
    // Get a list of supported configurations for the format.
    WMDM_FORMAT_CAPABILITY formatCapList;
    HRESULT hr = pDevice->GetFormatCapability(formatCode, &formatCapList);
    if (FAILED(hr)) return E_FAIL;
		if (formatCapList.nPropConfig == 0)
		{
			FreeFormatCapability(formatCapList);
			return E_FAIL; // operation succeeded, but format not supported.
		}

    // TODO: Display the format name.
    // Loop through the configurations and examine each one.
    for (UINT iConfig = 0; iConfig < formatCapList.nPropConfig; iConfig++)
    {
        WMDM_PROP_CONFIG formatConfig = formatCapList.pConfigs[iConfig];

        // Preference level for this configuration (lower number means more preferred).
        // TODO: Display the preference level for this format configuration.

        // Loop through all properties for this configuration and get supported
        // values for the property. Values can be a single value, a range, 
        // or a list of enumerated values.
        for (UINT iDesc = 0; iDesc < formatConfig.nPropDesc; iDesc++)
        {
            WMDM_PROP_DESC propDesc = formatConfig.pPropDesc[iDesc];
            // TODO: Display the property name.

            // Three ways a value can be represented: any, a range, or a list.
            switch (propDesc.ValidValuesForm)
            {
                case WMDM_ENUM_PROP_VALID_VALUES_ANY:
                    // TODO: Display a message indicating that all values are valid.
                    break;
                case WMDM_ENUM_PROP_VALID_VALUES_RANGE:
                    {
                        // List these in the docs as the propvariants set.
                        WMDM_PROP_VALUES_RANGE rng = 
                            propDesc.ValidValues.ValidValuesRange;
                        // TODO: Display the min, max, and step values.
                    }
                    break;
                case WMDM_ENUM_PROP_VALID_VALUES_ENUM:
                    {
                        // TODO: Display a banner for the list of valid values.
											/*
                        WMDM_PROP_VALUES_ENUM list = propDesc.ValidValues.EnumeratedValidValues;
                        PROPVARIANT pVal;
                        for (UINT iValue = 0; iValue < list.cEnumValues; iValue++)
                        {
                            pVal = list.pValues[iValue];
                            // TODO: Display each valid value.
                            PropVariantClear(&pVal);
                            PropVariantInit(&pVal);
                        }*/
                    }

                    break;
                default:
					FreeFormatCapability(formatCapList);
                    return E_FAIL;
                    //break;
            }
        }
    }
    // Now clear the memory used by WMDM_FORMAT_CAPABILITY.
    FreeFormatCapability(formatCapList);
    return hr;
}

static __time64_t wmdmDateTimeToUnixTime(_WMDMDATETIME * t) {
  tm m={0};
  m.tm_hour = t->wHour;
  m.tm_min = t->wMinute;
  m.tm_sec = t->wSecond;
  m.tm_mday = t->wDay;
  m.tm_mon = t->wMonth;
  m.tm_year = t->wYear;
  return _mktime64(&m);
}

HRESULT getMetadata(IWMDMStorage4 *store2,IWMDMMetaData ** meta, bool noMetadata) {
	const wchar_t ** propnames = (const wchar_t**)calloc(15,sizeof(void*));
	propnames[0] = g_wszWMDMFormatCode;
	propnames[1] = g_wszWMDMTitle;
	propnames[2] = g_wszWMDMAuthor;
	propnames[3] = g_wszWMDMAlbumTitle;
	propnames[4] = g_wszWMDMGenre;
	propnames[5] = g_wszWMDMTrack;
	propnames[6] = g_wszWMDMYear;
	propnames[7] = g_wszWMDMFileSize;
	propnames[8] = g_wszWMDMDuration;
	propnames[9] = g_wszWMDMPlayCount;
	propnames[10] = g_wszWMDMUserRating;
	propnames[11] = g_wszWMDMUserLastPlayTime;
	propnames[12] = g_wszWMDMLastModifiedDate;
	propnames[13] = g_wszWMDMAlbumArtist;
	propnames[14] = g_wszWMDMComposer;
	HRESULT h;
	if(noMetadata) {
	    h = store2->GetSpecifiedMetadata(1,(LPCWSTR*)propnames,meta);
	    if(h == WMDM_S_NOT_ALL_PROPERTIES_RETRIEVED) h = S_OK;
		if(h != S_OK) { // ugh. Guess that this is an AAC/M4A. Dirty workaround hack!
			if (SUCCEEDED(store2->CreateEmptyMetadataObject(meta))) {
				h = S_OK;
				DWORD type = WMDM_FORMATCODE_UNDEFINEDAUDIO;
				(*meta)->AddItem(WMDM_TYPE_DWORD,g_wszWMDMFormatCode,(BYTE*)&type,sizeof(type));
			}
		}
	} else {
	    h = store2->GetSpecifiedMetadata(13,(LPCWSTR*)propnames,meta);
	    if(h == WMDM_S_NOT_ALL_PROPERTIES_RETRIEVED) h = S_OK;
	}
	free(propnames);
	return h;
}

void P4SDevice::foundSong(IWMDMStorage4 * store, IWMDMMetaData * meta,bool video,int pl,wchar_t * artist, wchar_t * album, IWMDMStorage4 * alb, IWMDMMetaData * albmeta) {
  Playlist * pls = (Playlist *)playlists.Get(pl);
  Song * song = new Song;
  song->video = video;
  song->meta = meta;
  song->modified = false;
  song->storage = store;
	song->artist = artist;
	song->album = album;
	if(alb && albmeta) {
		song->alb = alb;
		song->albmeta = albmeta;
		alb->AddRef();
		albmeta->AddRef();
	}
	if(song->artist) song->artist = _wcsdup(song->artist);
	if(song->album) song->album = _wcsdup(song->album);
	pls->songs.Add(song);
	store->AddRef();
	meta->AddRef();
	if(noMetadata || video) {
	    wchar_t buf[2048]=L"";
		getTrackAlbum((songid_t)song,buf,2048);
		if(video) meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAlbumTitle,(BYTE*)L"~",4);
		else {
			meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAlbumTitle,(BYTE*)buf,wcslen(buf)*2 + 2);
			buf[0]=0;
			getTrackArtist((songid_t)song,buf,2048);
		}
		meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAuthor,(BYTE*)buf,wcslen(buf)*2 + 2);
		buf[0]=0;
		getTrackTitle((songid_t)song,buf,2048);
		meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMTitle,(BYTE*)buf,wcslen(buf)*2 + 2);
		int n = getTrackTrackNum((songid_t)song);
		meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMTrack,(BYTE*)&n,sizeof(DWORD));
		__int64 s = (__int64)getTrackSize((songid_t)song);
		meta->AddItem(WMDM_TYPE_QWORD,g_wszWMDMFileSize,(BYTE*)&s,sizeof(__int64));
	}
}

void P4SDevice::foundPlaylist(IWMDMStorage4 * store, IWMDMMetaData * meta) {
	DWORD count = 0;
	IWMDMStorage ** stores;
	wchar_t buf[100] = {0};
	store->GetName(buf,100);
	//OutputDebugString(buf);
	if(store->GetReferences(&count,&stores) == S_OK) {
		if(count > 0) {
			Playlist * pl = new Playlist;
			StringCchCopy(pl->name, ARRAYSIZE(pl->name), buf);
			{wchar_t * ext = wcsrchr(pl->name,L'.'); if(ext) *ext=0;}
			pl->storage = store;
			pl->meta = meta;
			meta->AddRef();
			store->AddRef();
			playlists.Add(pl);
			int num = playlists.GetSize()-1;
			for(unsigned int i=0; i<count; i++) {
				IWMDMStorage4 * song=NULL;
				/*{
					wchar_t buf[100] = {0};
					stores[i]->GetName(buf,100);
					OutputDebugString(buf);
				}*/
				if(stores[i]->QueryInterface(&song) == S_OK) if(song) {
					/*
					Song * s = new Song;
					s->modified=false;
					s->storage=song;
					HRESULT h = getMetadata(song,&s->meta);
					if (SUCCEEDED(h)) pl->songs.Add(s);
					else delete s;
					*/
					//pl->songs.Add(song);
					IWMDMMetaData * meta;
					if (SUCCEEDED(getMetadata(song,&meta,noMetadata))) { 
						foundSong(song,meta,false,num); meta->Release();
					}
					song->Release();
				}
				stores[i]->Release();
			}
		} else {
			//OutputDebugString(L"ref count zero");
		}
		CoTaskMemFree(stores);
	} else {
		//OutputDebugString(L"can't get playlist refs");
	}
}

void P4SDevice::traverseStorage(IWMDMStorage * store, int level, wchar_t * artist, wchar_t * album) {
	if(!store) return;
	IWMDMStorage4 * store2=NULL;
	ULONG num;
	//OutputDebugStringA("1A");
	C_ItemList storages;
	{
		IWMDMEnumStorage * enstore=NULL;
		IWMDMStorage * storeold=NULL;
		HRESULT hr = store->EnumStorage(&enstore);
		while(SUCCEEDED(hr)) {
			if (WaitForSingleObject(killEvent,0) == WAIT_OBJECT_0)
			break;
			hr = enstore->Next(1,&storeold,&num);
			if(SUCCEEDED(hr) && storeold) {
				hr = storeold->QueryInterface(&store2);
				storeold->Release();
				if(SUCCEEDED(hr) && store2)  {
					storages.Add(store2);
				}
				else break;
			}
			else break;
		}
		if(enstore) enstore->Release();
	}
	//OutputDebugStringA("2A");
	//while (SUCCEEDED(hr) && enstore) {
	IWMDMStorage4 * alb=NULL;
	IWMDMMetaData * albmeta=NULL;

	for(int i=0; i<storages.GetSize(); i++) {
		wchar_t buf[256]=L"";
		store2 = (IWMDMStorage4*)storages.Get(i);
		store2->GetName(buf,256);
		wchar_t *ext = wcsrchr(buf,L'.');
		if(ext && _wcsicmp(ext,L".alb")==0) {
			alb = store2;
			alb->AddRef();
			alb->GetMetadata(&albmeta);
			break;
		}
	}

	//albfiles.Add(new AlbFile(store2,meta));
	for(int i=0; i<storages.GetSize(); i++) {
		if (WaitForSingleObject(killEvent,0) == WAIT_OBJECT_0)
			break;
		//OutputDebugStringA("1");
		store2 = (IWMDMStorage4*)storages.Get(i);

		if(store2 == alb) continue;

		wchar_t buf[256]=L"";
		//OutputDebugStringA("2");
		store2->GetName(buf,256);
		if(playlistsDir == NULL && _wcsicmp(buf,L"My Playlists")==0) { playlistsDir = store2; store2->AddRef(); }
		if(playlistsDir == NULL && _wcsicmp(buf,L"Playlists")==0) { playlistsDir = store2; store2->AddRef(); }

		DWORD attribs=NULL;
		store2->GetAttributes(&attribs,NULL);
		//OutputDebugStringA("3");
		if(attribs & WMDM_FILE_ATTR_FOLDER) {
			if(level==0) artist=buf;
			else if(level==1) album=buf;
			//OutputDebugStringA("5"); 
			WMDM_STORAGE_ENUM_MODE mode = ENUM_MODE_RAW;
			store2->SetEnumPreference(&mode,0,NULL);
			traverseStorage(store2,level+1,artist,album); 
		} else if(attribs & WMDM_FILE_ATTR_FILE || !attribs) {  
			IWMDMMetaData * meta=NULL;
			//OutputDebugStringA("1");
			HRESULT h = getMetadata(store2,&meta,noMetadata);
			//OutputDebugStringA("2");
			//OutputDebugStringA("4");
			/*{
		        wchar_t buf2[400] = {0};
		        wsprintf(buf2,L"name: %s atr: 0x%x, hr: 0x%x %s",buf,attribs,h,h == E_INVALIDARG?L"POOT":L"");
		        OutputDebugString(buf2);
			}*/
			if(meta && h == S_OK) {
				wchar_t * name=NULL;
				WMDM_TAG_DATATYPE type;
				BYTE * value=NULL;
		        UINT valuelen;
		        if(meta->QueryByName(g_wszWMDMFormatCode,&type,&value,&valuelen) == S_OK && type == WMDM_TYPE_DWORD)
				{
					switch(*(DWORD*)value) { // find out what it is...
						case WMDM_FORMATCODE_ABSTRACTAUDIOVIDEOPLAYLIST:
						case WMDM_FORMATCODE_WPLPLAYLIST:
						case WMDM_FORMATCODE_M3UPLAYLIST:
						case WMDM_FORMATCODE_MPLPLAYLIST:
						case WMDM_FORMATCODE_ASXPLAYLIST:
						case WMDM_FORMATCODE_PLSPLAYLIST:
							foundPlaylist(store2,meta);
							break;
						case WMDM_FORMATCODE_ASF:
						case WMDM_FORMATCODE_AVI:
						case WMDM_FORMATCODE_MPEG:
						case WMDM_FORMATCODE_WMV:
						case WMDM_FORMATCODE_MP2:
						case WMDM_FORMATCODE_3GP:
						case WMDM_FORMATCODE_UNDEFINEDVIDEO:
							foundSong(store2,meta,true);
							break;
						case WMDM_FORMATCODE_UNDEFINED:
							{ //ugh. nokiahack.
								wchar_t * ext = wcsrchr(buf,'.');
								if(!ext) break;
								if(!_wcsicmp(ext,L".mp4") || !_wcsicmp(ext,L".m4a")) foundSong(store2,meta,false);
							}
							break;
						case WMDM_FORMATCODE_AIFF:
						case WMDM_FORMATCODE_WAVE:
						case WMDM_FORMATCODE_MP3:
						case WMDM_FORMATCODE_WMA:
						case WMDM_FORMATCODE_OGG:
						case WMDM_FORMATCODE_AAC:
						case WMDM_FORMATCODE_MP4:
						case WMDM_FORMATCODE_AUDIBLE:
						case WMDM_FORMATCODE_FLAC:
						case WMDM_FORMATCODE_UNDEFINEDAUDIO:
							foundSong(store2,meta,false,0,0,0,alb,albmeta);
							break;
					}
				} else {
					wchar_t * ext = wcsrchr(buf,'.');
					if(ext) {
						bool m = noMetadata;
						noMetadata = true;
						if(!_wcsicmp(ext,L".mp3") || !_wcsicmp(ext,L".wma")) foundSong(store2,meta,false,0,artist,album);
						else if(!_wcsicmp(ext,L".wmv") || !_wcsicmp(ext,L".avi")) foundSong(store2,meta,true,0,artist,album);
						else if(!_wcsicmp(ext,L".asx") || !_wcsicmp(ext,L".pla")) foundPlaylist(store2,meta);
						noMetadata = m;
					}
				}
				meta->Release();
				if(name) CoTaskMemFree(name);
				if(value) CoTaskMemFree(value);
			}
		}
		if(store2) store2->Release();
	}

	if(alb) alb->Release();
	if(albmeta) albmeta->Release();
}

bool P4SDevice::songsEqual(songid_t a, songid_t b) {
	wchar_t ba[1024] = {0};
	wchar_t bb[1024] = {0};
	getTrackTitle(a,ba,1024);
	getTrackTitle(b,bb,1024);
	if(wcscmp(ba,bb)) return false;
	getTrackAlbum(a,ba,1024);
	getTrackAlbum(b,bb,1024);
	if(wcscmp(ba,bb)) return false;
	getTrackArtist(a,ba,1024);
	getTrackArtist(b,bb,1024);
	if(wcscmp(ba,bb)) return false;
	return true;
}

int P4SDevice::songsCmp(songid_t a, songid_t b) {
	int q=0;
	wchar_t ba[1024] = {0};
	wchar_t bb[1024] = {0};
	getTrackTitle(a,ba,1024);
	getTrackTitle(b,bb,1024);
	q=wcscmp(ba,bb); if(q) return q;
	getTrackAlbum(a,ba,1024);
	getTrackAlbum(b,bb,1024);
	q=wcscmp(ba,bb); if(q) return q;
	getTrackArtist(a,ba,1024);
	getTrackArtist(b,bb,1024);
	return wcscmp(ba,bb);
}

P4SDevice * sortDev;

static int song_sortfunc(const void *elem1, const void *elem2) {
	songid_t a = *(songid_t *)elem1;
	songid_t b = *(songid_t *)elem2;
	return sortDev->songsCmp(a,b);
}

extern IWMDRMDeviceApp * DRMDeviceApp;

static songid_t BinaryChopFind(songid_t find,Playlist * mpl, P4SDevice * dev) {
	sortDev = dev;
	songid_t * ret = (songid_t*)bsearch(&find,mpl->songs.GetAll(),mpl->songs.GetSize(),sizeof(songid_t),song_sortfunc);
	return ret?*ret:NULL;
}

P4SDevice::P4SDevice(IWMDMDevice3* pIDevice, bool noMetadata) : transcoder(NULL) {
	error=0;
	musicDir = L"Music";
	videoDir = L"Video";
	lastChange=NULL;
	this->noMetadata = noMetadata;
	if(DRMDeviceApp && DRMDeviceApp->SynchronizeLicenses(pIDevice,NULL,0,0) == S_OK)
	{
		//OutputDebugString(L"sync!"); 
	}
	playlistsDir=NULL;
	transferQueueSize=0;
	Playlist * mpl = new Playlist;
	playlists.Add(mpl);

	WMDevice = pIDevice;
	if (NULL != WMDevice)
	{
		WMDevice->AddRef();
		if (FAILED(WMDevice->GetName(name,100)))
			name[0] = L'\0';
	}
	else
		name[0] = L'\0';

	// give loading indication to the user....
	pmpDeviceLoading load;
	load.dev = this;
	load.UpdateCaption = NULL;
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)&load,PMP_IPC_DEVICELOADING);
	if(load.UpdateCaption) {
	    wchar_t buf[200]=L"";
		wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_LOADING),name);
		load.UpdateCaption(buf,load.context);
	}
	Load();
}

void P4SDevice::Load() {
	Playlist * mpl = (Playlist*)playlists.Get(0);
	HRESULT hr;
	IWMDMEnumStorage * enstore=0;
	IWMDMStorage * store0=0;
	IWMDMStorage4 * store=0;
	hr = WMDevice->EnumStorage(&enstore);
	ULONG num;
	if(!enstore) {delete this; return;}
	hr = enstore->Next(1,&store0,&num);
	if (SUCCEEDED(hr)) hr = store0->QueryInterface(&store);
	if(store0) store0->Release();
	else error=1;
	if(enstore) enstore->Release();
	else error=1;
	if (SUCCEEDED(hr) && !error) {
		wchar_t buf[100] = {0};
		store->GetName(buf,100);
		mpl->storage = store;
		store->AddRef();
		WMDM_STORAGE_ENUM_MODE mode = ENUM_MODE_RAW;
		store->SetEnumPreference(&mode,0,NULL);

		//traverseStorage(store);
		IWMDMStorage * playlists;
		if(store->GetStorage(L"Playlists",&playlists) == S_FALSE) {
			if(store->GetStorage(L"My Playlists",&playlists) == S_FALSE) 
			{ 
				playlists = NULL;
			}
		}
		if(playlists) 
		{
			traverseStorage(playlists);
			playlists->QueryInterface(&playlistsDir);
			playlists->Release(); 
		}

		IWMDMStorage * songs =  NULL;
		if(store->GetStorage(L"Music",&songs) == S_OK) 
		{ 
			traverseStorage(songs); 
			songs->Release(); 
		} 
		else if(store->GetStorage(L"My Music",&songs) == S_OK) 
		{ 
			traverseStorage(songs); 
			songs->Release(); 
			musicDir = L"My Music";
		}
		else if(store->GetStorage(L"Music Files",&songs) == S_OK) 
		{ 
			traverseStorage(songs);
			songs->Release(); 
			musicDir = L"Music Files";
		}
		else { // create a music folder
			IWMDMStorageControl3 * storeControl=NULL;
			store->QueryInterface(&storeControl);
			if(storeControl) {
				IWMDMStorage * newdir=NULL;
				IWMDMMetaData * meta=NULL;
				store->CreateEmptyMetadataObject(&meta);
				storeControl->Insert3(WMDM_MODE_BLOCK|WMDM_CONTENT_FOLDER,WMDM_FILE_ATTR_FOLDER,NULL,musicDir,NULL,NULL,meta,NULL,&newdir);
				if(newdir) newdir->Release();
				else error=1;
				if(meta) meta->Release();
				else error=1;
				storeControl->Release();
			} else error=1;
		}

		if(store->GetStorage(L"Video",&songs) == S_OK) { traverseStorage(songs); songs->Release(); supportsVideo=true; } 
		if(store->GetStorage(L"TV",&songs) == S_OK) { traverseStorage(songs); songs->Release(); supportsVideo=true; } 

		if(!playlists && !songs) traverseStorage(store);
	}
	if(!store) error=1;
	if(error) {delete this; return;}
	if(!playlistsDir) playlistsDir = GetOrCreateFolder(store,L"Playlists");
	if(!playlistsDir) {delete this; return;}//MessageBox(plugin.hwndWinampParent,L"An error has occured whilst trying to initialise the playlists on your device.\nPlaylists will not work correctly.",L"Playlists Error",0);
  
	sortDev = this;
	qsort(mpl->songs.GetAll(),mpl->songs.GetSize(),sizeof(void*),song_sortfunc);

	// Now to recombobulate the playlists (this SUCKS slightly less now)
	for(int i=1; i<playlists.GetSize(); i++) {
		Playlist * pl = (Playlist *)playlists.Get(i);
		int l = pl->songs.GetSize();
		for(int j=0; j<l; j++) {
			songid_t plsong = (songid_t)pl->songs.Get(j);
			songid_t mplsong = BinaryChopFind(plsong,mpl,this);
			if(mplsong) pl->songs.Set(j,(void*)mplsong);
			else { pl->songs.Del(j--); l--; }

			Song * p = (Song *)plsong;
			p->meta->Release();
			p->storage->Release();
			delete p;
		}
	}

	requiresALB=false;
	{ // check for .alb support
		WMDM_FORMAT_CAPABILITY formatCapList;
		HRESULT hr = WMDevice->GetFormatCapability(WMDM_FORMATCODE_ABSTRACTAUDIOALBUM, &formatCapList);
		bool size=false,data=false,format=false;
		if(!FAILED(hr)) {
			for(unsigned int i=0; i<formatCapList.nPropConfig; i++) {
				WMDM_PROP_CONFIG formatConfig = formatCapList.pConfigs[i];
				for(unsigned int j=0; j<formatConfig.nPropDesc; j++) {
					if(!formatConfig.pPropDesc[j].pwszPropName) continue;
					if(wcscmp(formatConfig.pPropDesc[j].pwszPropName,g_wszWMDMAlbumCoverSize)==0) size=true;
					else if(wcscmp(formatConfig.pPropDesc[j].pwszPropName,g_wszWMDMAlbumCoverData)==0) data=true;
					else if(wcscmp(formatConfig.pPropDesc[j].pwszPropName,g_wszWMDMAlbumCoverFormat)==0) format=true;
				}
			}
		}
		if(size && data && format) requiresALB=true;
		FreeFormatCapability(formatCapList);
	}
	
	devices.Add(this);
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICECONNECTED);

	//transcoder = NULL;
	transcoder = (Transcoder*)SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)this,PMP_IPC_GET_TRANSCODER);
	if(transcoder) {
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_MP3,WMDevice))) 
			transcoder->AddAcceptableFormat(L"mp3");
			//transcoder->AddAcceptableFormat(L"wav");
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_WMA,WMDevice))) 
			transcoder->AddAcceptableFormat(L"wma");
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_ASF,WMDevice))) 
			transcoder->AddAcceptableFormat(L"asf");
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_AVI,WMDevice))) 
			transcoder->AddAcceptableFormat(L"avi");
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_OGG,WMDevice))) 
			transcoder->AddAcceptableFormat(L"ogg");
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_FLAC,WMDevice))) 
			transcoder->AddAcceptableFormat(L"flac");
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_AUDIBLE,WMDevice))) 
			transcoder->AddAcceptableFormat(L"aa");
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_MPEG,WMDevice))) {
			transcoder->AddAcceptableFormat(L"mpeg");
			transcoder->AddAcceptableFormat(L"mpg");
		}
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_WMV,WMDevice))) 
			transcoder->AddAcceptableFormat(L"wmv");
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_3GP,WMDevice))) 
			transcoder->AddAcceptableFormat(L"3gp");
		if(SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_MP4,WMDevice))) {
			transcoder->AddAcceptableFormat(L"mp4");
			transcoder->AddAcceptableFormat(L"m4a");
		}
		if(noMetadata || SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_AAC,WMDevice))) {
			transcoder->AddAcceptableFormat(L"m4a");
			transcoder->AddAcceptableFormat(L"mp4");
			transcoder->AddAcceptableFormat(L"aac");
		}
	}
}

static void commitPlaylist(Playlist * pl) {
	int l = pl->songs.GetSize();
	IWMDMStorage ** newOrder = (IWMDMStorage **)calloc(l, sizeof(IWMDMStorage *));
	for(int j=0; j<l; j++) ((Song *)pl->songs.Get(j))->storage->QueryInterface(&newOrder[j]);
	pl->storage->SetReferences(l, newOrder);
	for(int i=0; i<l; i++) newOrder[i]->Release();
	free(newOrder);
	pl->modified = false;
}

P4SDevice::~P4SDevice() {
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
	if(playlistsDir) playlistsDir->Release();
	for(int i=1; i<playlists.GetSize(); i++) {
	    Playlist * pl = (Playlist*)playlists.Get(i);
		if(pl->modified) commitPlaylist(pl);
		pl->storage->Release();
		pl->meta->Release();
	}
	if(playlists.GetSize()) {
		Playlist * pl = (Playlist*)playlists.Get(0);
		pl->storage->Release();
		for(int j=0; j < pl->songs.GetSize(); j++) {
			Song * s = (Song *)pl->songs.Get(j);
			if (s)
			{
				if(s->modified) s->storage->SetMetadata(s->meta);
				s->meta->Release();
				s->storage->Release();
				delete s;
			}
		}
	}

	for(int i=0; i<devices.GetSize(); i++) {
		if(devices.Get(i) == (void*)this) {
			devices.Del(i);
			return;
		}
	}
	WMDevice->Release();
	if(transcoder) SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)transcoder,PMP_IPC_RELEASE_TRANSCODER);
}

 void P4SDevice::Close()
 {
	 delete this;
 }
 
__int64 P4SDevice::getDeviceCapacityAvailable() {
	static __int64 prev;
	IWMDMStorageGlobals * sg = NULL;
	if(!playlistsDir) return 0;
	playlistsDir->GetStorageGlobals(&sg);
	if(sg) {
		ULARGE_INTEGER s;
		sg->GetTotalFree(&s.LowPart,&s.HighPart);
		sg->Release();
		if(s.LowPart == 0 && s.HighPart == 0) return prev;
		return prev = s.QuadPart;
	}
	return prev;
}

__int64 P4SDevice::getDeviceCapacityTotal() {
	static __int64 prev;
	IWMDMStorageGlobals * sg = NULL;
	if(!playlistsDir) return 0;
	playlistsDir->GetStorageGlobals(&sg);
	if(sg) {
		ULARGE_INTEGER s;
		sg->GetTotalSize(&s.LowPart,&s.HighPart);
		sg->Release();
		if(s.LowPart == 0 && s.HighPart == 0) return prev;
		return prev = s.QuadPart;
	}
	return prev;
}

void P4SDevice::deleteTrack(songid_t songid) {
	lastChange=NULL;
	Song * s = (Song*)songid;
	IWMDMStorageControl3 * storeControl=NULL;
	s->storage->QueryInterface(&storeControl);
	if(!storeControl) return;
	for(int i=0; i<playlists.GetSize(); i++) {
		Playlist * pl = (Playlist *)playlists.Get(i);
		int j = pl->songs.GetSize();
		while(j-- > 0) if((Song *)pl->songs.Get(j) == s) { pl->songs.Del(j); pl->modified=true; }
	}
	if(storeControl->Delete(WMDM_MODE_BLOCK,NULL) != S_OK) return;
	s->meta->Release();
	s->storage->Release();
	storeControl->Release();
	delete s;
}

void P4SDevice::commitChanges() {
	for(int i=1; i<playlists.GetSize(); i++) {
		Playlist * pl = (Playlist*)playlists.Get(i);
		if(pl->modified) commitPlaylist(pl);
	}

	Playlist * pl = (Playlist*)playlists.Get(0);
	for(int j=0; j < pl->songs.GetSize(); j++) {
		Song * s = (Song *)pl->songs.Get(j);
		if(s->modified) {
			s->storage->SetMetadata(s->meta);
			s->meta->Release();
			s->storage->GetMetadata(&s->meta);
			s->modified = false;
		}
	}
	lastChange=NULL;
}

static int fileSizeA(char * filename)
{
	FILE * fh = fopen(filename,"rb");
	if(!fh) return -1;
	fseek(fh,0,2); //seek to end;
	int l = ftell(fh);
	fclose(fh);
	return l;
}

#define MKVALIDFN(x) { wchar_t * n = x; while(n && *n == L'.') *(n++)=L'_'; while(n && *n) { if(*n == L'|' || *n == L'\\' || *n == L'/' || *n == L'?' || *n == L'<' || *n == L'>' || *n == L':' || *n == L'*'  || *n == L'"') *n=L'_'; n++; } n = x+wcslen(x)-1; while(n && *n==L'.' && n>=x) *(n--)=0; }

static IWMDMStorage4* GetOrCreateFolder(IWMDMStorage4 * store, wchar_t * name, P4SDevice * dev, bool album, const itemRecordW * item) {
	if(!name[0]) name=L"Blank";
	MKVALIDFN(name);
	if(!name[0]) name=L"Blank";
	if(!store) return NULL;
	IWMDMEnumStorage * enstore=NULL;
	HRESULT hr = store->EnumStorage(&enstore);
	IWMDMStorage * store0;
	IWMDMStorage4 * store4;
	ULONG num;
	if(!enstore) return NULL;
	enstore->Reset();
	hr = enstore->Next(1,&store0,&num);
	while(hr == S_OK) {
		store4=NULL;
		store0->QueryInterface(&store4);
		store0->Release();
		wchar_t buf[100] = {0};
		store4->GetName(buf,100);
		if(_wcsicmp(buf,name) == 0) {
			return store4;
		}
		store4->Release();
		hr = enstore->Next(1,&store0,&num);
	}
	if(enstore) enstore->Release();
	// we must create it!
	store0=store4=NULL;
	IWMDMStorageControl3 * storeControl=NULL;
	store->QueryInterface(&storeControl);
	if(!storeControl) return NULL;
	IWMDMMetaData * meta;
	store->CreateEmptyMetadataObject(&meta);
	storeControl->Insert3(WMDM_MODE_BLOCK|WMDM_CONTENT_FOLDER,WMDM_FILE_ATTR_FOLDER,NULL,name,NULL,NULL,meta,NULL,&store0);
	meta->Release();
	if(!store0) return NULL;
	store0->QueryInterface(&store4);
	storeControl->Release();
	store0->Release();
	store->Release();
	if(album) {
		wchar_t buffer[MAX_PATH];
		IWMDMStorageControl3 * storeControl;
		wsprintf(buffer,L"%s.alb",name);
		store4->QueryInterface(&storeControl);
		IWMDMStorage * newpl=NULL;
		IWMDMStorage4 * newpl4=NULL;
		IWMDMMetaData * meta=NULL;
		store4->CreateEmptyMetadataObject(&meta);
		if (meta)
		{
			DWORD formatCode = WMDM_FORMATCODE_ABSTRACTAUDIOALBUM;
			meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMFormatCode,(BYTE*)&formatCode,sizeof(DWORD));
			meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAlbumTitle,(BYTE*)name,(wcslen(name)*2)+2);
			meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAlbumArtist,(BYTE*)((wchar_t*)item->albumartist),(wcslen(item->albumartist)*2)+2);
			meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMTitle,(BYTE*)name,(wcslen(name)*2)+2);
			meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAuthor,(BYTE*)((wchar_t*)item->artist),(wcslen(item->artist)*2)+2);
			meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMGenre,(BYTE*)((wchar_t*)item->genre),(wcslen(item->genre)*2)+2);
			storeControl->Insert3(WMDM_MODE_BLOCK | WMDM_CONTENT_FILE,0,NULL,buffer,NULL,NULL,meta,NULL,&newpl);
			storeControl->Release();
			if (newpl)
			{
				newpl->QueryInterface(&newpl4);
				newpl->Release();
			}
			if (newpl4)
			{
				newpl4->SetReferences(0,NULL);
				newpl4->SetMetadata(meta);
				newpl4->Release();
			}
			meta->Release();
		}
	}
	return store4;
}

// Disabled for 5.64
// Fixes issues with exit failures and duplicates on transfer (view issue only) and other memory corruption issues
/*typedef struct {
	C_ItemList * playlists;
	void * item;
} addTrackStruct;

void CALLBACK addTrack(ULONG_PTR dwParam) {
	addTrackStruct * a = (addTrackStruct *) dwParam;
	((Playlist*)a->playlists->Get(0))->songs.Add(a->item);
	delete a;
}*/

static void getTime(__time64_t value, _WMDMDATETIME * time) {
	if(!time) return;
	ZeroMemory(time,sizeof(_WMDMDATETIME));
	struct tm * t = _localtime64(&value);
	if(!t) return;
	time->wYear = t->tm_year;
	time->wMonth = t->tm_mon;
	time->wDay = t->tm_mday;
	time->wHour = t->tm_hour;
	time->wMinute = t->tm_min;
	time->wSecond = t->tm_sec;
}

static int atoi_nullok(char * str) {
	if(str) return atoi(str);
	return 0;
}

static IWMDMStorage * storefoo;

#define PHASE_START       1
#define PHASE_INPROGRESS  2
#define PHASE_FINISH      3
#define PHASE_DONE        4
#define PHASE_ERROR       5

extern CSecureChannelClient SAC;
extern int SynchronousProcedureCall(void * p, ULONG_PTR dwParam);

void P4SDevice::doTransfer(TransferItem * t) {
	static wchar_t buf[256];
	static IWMDMStorage4 * store;
	static IWMDMStorageControl3 * control;

	switch(t->phase) {
		case PHASE_START:
		{
			bool video = false;
			DWORD formatCode=0;
			wchar_t * point = wcsrchr(t->file,L'.');
			if(point) {
				if(_wcsicmp(point,L".wma")==0) formatCode = WMDM_FORMATCODE_WMA;
				else if(_wcsicmp(point,L".wav")==0) formatCode = WMDM_FORMATCODE_WAVE;
				else if(_wcsicmp(point,L".ogg")==0) formatCode = WMDM_FORMATCODE_OGG;
				else if(_wcsicmp(point,L".m4a")==0) formatCode = WMDM_FORMATCODE_MP4;
				else if(_wcsicmp(point,L".aac")==0) formatCode = WMDM_FORMATCODE_AAC;
				else if(_wcsicmp(point,L".aa")==0) formatCode = WMDM_FORMATCODE_AUDIBLE;
				else if(_wcsicmp(point,L".flac")==0 || _wcsicmp(point,L".fla")==0) formatCode = WMDM_FORMATCODE_FLAC;
				else if(_wcsicmp(point,L".asf")==0) { video=true; formatCode = WMDM_FORMATCODE_ASF; }
				else if(_wcsicmp(point,L".avi")==0) { video=true; formatCode = WMDM_FORMATCODE_AVI; }
				else if(_wcsicmp(point,L".mpg")==0) { video=true; formatCode = WMDM_FORMATCODE_MPEG; }
				else if(_wcsicmp(point,L".mpeg")==0) { video=true; formatCode = WMDM_FORMATCODE_MPEG; }
				else if(_wcsicmp(point,L".wmv")==0) { video=true; formatCode = WMDM_FORMATCODE_WMV; }
				else if(_wcsicmp(point,L".m4v")==0) { video=true; formatCode = WMDM_FORMATCODE_MP4; }
				else if(_wcsicmp(point,L".mp2")==0) { video=true; formatCode = WMDM_FORMATCODE_MP2; }
				else if(_wcsicmp(point,L".mp4")==0) {
					wchar_t buf[10]=L"0";
					extendedFileInfoStructW m = {t->file,L"type",buf,10};
					SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&m,IPC_GET_EXTENDED_FILE_INFOW);
					formatCode = WMDM_FORMATCODE_MP4;
					video = (buf[0]==L'1');
				}
				else formatCode = WMDM_FORMATCODE_MP3; // mp3 or whatever
			}
			t->video = video;

			store = ((Playlist *)playlists.Get(0))->storage;
			store->AddRef();

		if(video) {
			store = GetOrCreateFolder(store,videoDir,this);
			if(_wcsicmp(t->track->artist,L"")) store = GetOrCreateFolder(store,t->track->artist);
		} else {
	        store = GetOrCreateFolder(store,musicDir,this);
		    if(_wcsicmp(t->track->artist,L"")) store = GetOrCreateFolder(store,t->track->artist);
			else store = GetOrCreateFolder(store,L"No Artist");
			if(_wcsicmp(t->track->album,L"")) store = GetOrCreateFolder(store,t->track->album,this,requiresALB,t->track);
			else store = GetOrCreateFolder(store,L"No Album");
		}
      /*
		DWORD dw;
		do {
			WMDevice->GetStatus(&dw);
			SleepEx(50,true);
		} while(!(dw & WMDM_STATUS_READY));
      */
		if(!store) {
			t->callback(t->callbackContext,WASABI_API_LNGSTRINGW(IDS_COUND_NOT_CREATE_FOLDER));
			*(t->songid)=NULL;
			t->phase=PHASE_ERROR;
			return;
		}

		// create and fill in metadata...

		HRESULT hr;
		hr = store->CreateEmptyMetadataObject(&t->meta);
		if(hr != S_OK) {
			wchar_t buf[100] = {0};
			wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_COULD_NOT_CREATE_METADATA),hr);
			t->callback(t->callbackContext,buf);
			*(t->songid)=NULL;
			t->phase=PHASE_ERROR;
			return;
		}
		t->meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMFormatCode,(BYTE*)&formatCode,sizeof(DWORD));
		if(t->track->artist) t->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAuthor,(BYTE*)((wchar_t*)(t->track->artist)),(wcslen(t->track->artist)*2)+2);
			if(t->track->albumartist) t->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAlbumArtist,(BYTE*)((wchar_t*)(t->track->albumartist)),(wcslen(t->track->albumartist)*2)+2);
			if(t->track->composer) t->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMComposer,(BYTE*)((wchar_t*)(t->track->composer)),(wcslen(t->track->composer)*2)+2);
			if(t->track->album) t->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAlbumTitle,(BYTE*)((wchar_t*)(t->track->album)),(wcslen(t->track->album)*2)+2);
			if(t->track->genre) t->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMGenre,(BYTE*)((wchar_t*)(t->track->genre)),(wcslen(t->track->genre)*2)+2);
			if(t->track->title) t->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMTitle,(BYTE*)((wchar_t*)(t->track->title)),(wcslen(t->track->title)*2)+2);
			if(t->track->track> 0) t->meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMTrack,(BYTE*)&t->track->track,sizeof(DWORD));
			__int64 len = t->track->length;
			len *= 10000000;
			wchar_t buf2[256] = {0};
			t->meta->AddItem(WMDM_TYPE_QWORD,g_wszWMDMDuration,(BYTE*)&len,sizeof(__int64));
			int fs = fileSizeA(AutoChar(t->file));
			len = fs;
			t->meta->AddItem(WMDM_TYPE_QWORD,g_wszWMDMFileSize,(BYTE*)&len,sizeof(__int64));
			wsprintf(buf2,L"%d",t->track->year);
			if(t->track->year > 0) t->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMYear,(BYTE*)buf2,(wcslen(buf2)*2) + 2);
			int v;
			if (t->track->length)
			{
				v = 8*(fs/t->track->length); //atoi_nullok(getRecordExtendedItem(t->track,"BITRATE")) * 1000;
				t->meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMBitrate,(BYTE*)&v,sizeof(DWORD));
			}
			v = t->track->playcount;
			t->meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMPlayCount,(BYTE*)&v,sizeof(DWORD));
			v = t->track->rating;
			if(v>=0 && v<=5) t->meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMUserRating,(BYTE*)&v,sizeof(DWORD));
			_WMDMDATETIME time1={0}, time2={0};
			getTime(t->track->lastplay,&time1);
			t->meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMUserLastPlayTime,(BYTE*)&time1,sizeof(DWORD));
			getTime(t->track->lastupd,&time2);
			t->meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMLastModifiedDate,(BYTE*)&time2,sizeof(DWORD));

			control = NULL;
			store->QueryInterface(&control);
			IWMDMStorage * newstore=NULL;
      
			if(video) wsprintf(buf,L"%s%s",(wchar_t*)(t->track->title),(wchar_t*)(wcsrchr(t->track->filename,'.')));
			else wsprintf(buf,L"%02d - %s%s",t->track->track,(wchar_t*)(t->track->title),(wchar_t*)(wcsrchr(t->track->filename,'.')));
      
			if(video) wsprintf(buf,L"%s%s",(wchar_t*)(t->track->title),wcsrchr(t->file,L'.'));
			else wsprintf(buf,L"%02d - %s%s",t->track->track,(wchar_t*)(t->track->title),wcsrchr(t->file,L'.'));
			MKVALIDFN(buf);

			if(!control) {
				t->callback(t->callbackContext,WASABI_API_LNGSTRINGW(IDS_INCOMPATABLE_DEVICE));
				*(t->songid)=NULL;
				t->phase = PHASE_ERROR;
				return;
			}
      
			t->phase = PHASE_INPROGRESS;
			t->progress = new MyProgress(t);
			storefoo = NULL;

			hr = control->Insert3(WMDM_MODE_BLOCK|WMDM_MODE_TRANSFER_PROTECTED|WMDM_CONTENT_FILE|WMDM_STORAGECONTROL_INSERTAFTER,
								  WMDM_FILE_ATTR_FILE,
								  t->file,
								  buf,
								  NULL,
								  t->progress,
								  t->meta,
								  NULL,
								  &storefoo);
			//OutputDebugString(L"finished insert");

			if (FAILED(hr)) {
				wchar_t buf1[100] = {0};
				wsprintf(buf1,WASABI_API_LNGSTRINGW(IDS_ERROR_IN_INSERT),hr);
				t->callback(t->callbackContext,buf1);
				*(t->songid)=NULL;
				t->phase = PHASE_ERROR;
				return;
			}
		}
		break;
		case PHASE_FINISH:
		//OutputDebugString(L"phase finish start");
		{
		/*
			DWORD dw;
			WMDevice->GetStatus(&dw);
			while(!(dw == WMDM_STATUS_READY)) {
				SleepEx(50,true);
				WMDevice->GetStatus(&dw);
			}
		*/
		}
    
		t->progress->Release();
		control->Release();

		if(storefoo) { /*OutputDebugString(L"storefoo");*/ storefoo->Release(); storefoo=NULL; }
		if(store) {
			IWMDMStorage * store0 = NULL;
			store->GetStorage(buf,&store0);
			store->Release();
			if(store0) {
				IWMDMStorage4 * store4 = NULL;
				store0->QueryInterface(&store4);
				if(store4) {
					store4->AddRef();
					store0->Release();
					store4->SetMetadata(t->meta);
					//t->meta->Release();
					Song * song = new Song;
					song->modified=false;
					song->storage = store4;
					song->meta = t->meta;
					song->video = t->video;
					//((Playlist*)playlists.Get(0))->songs.Add(song);

					// Disabled for 5.64
					// Fixes issues with exit failures and duplicates on transfer (view issue only) and other memory corruption issues
/*
					addTrackStruct * a = new addTrackStruct;
					a->item=song;
					a->playlists = &playlists;
					//PostMessage(plugin.hwndPortablesParent,WM_USER+3,(WPARAM)addTrack,(LPARAM)a);
					SynchronousProcedureCall((void*)addTrack,(ULONG_PTR)a);
*/

					*(t->songid) = (songid_t)song;
					// sort out the album group...
					if(t->track->album && requiresALB) {
						IWMDMStorage * album0=NULL;
						IWMDMStorage * parent=NULL;
						IWMDMStorage4 * parent4=NULL;
						IWMDMStorage4 * album4;
						wchar_t buf[512] = {0};
						wsprintf(buf,L"%s.alb",(wchar_t*)(t->track->album));
						store4->GetParent(&parent);
						if (parent)
						{
							parent->QueryInterface(&parent4);
							parent->Release();
							if (parent4)
							{
								parent4->GetStorage(buf,&album0);
								parent4->Release();
								if(album0) {
									album0->QueryInterface(&album4);
									album0->Release();
									DWORD refc;
									IWMDMStorage ** refs;
									IWMDMStorage ** newrefs;
									album4->GetReferences(&refc,&refs);
									newrefs = (IWMDMStorage **)calloc((refc + 1), sizeof(void*));
									for(DWORD i=0; i<refc; i++) newrefs[i] = refs[i];
									newrefs[refc] = store4;
									refc++;
									album4->SetReferences(refc,newrefs);
									refc--;
									for(DWORD i=0; i<refc; i++) refs[i]->Release();
									free(newrefs);
									CoTaskMemFree(refs);
									song->alb = album4;
									album4->GetMetadata(&song->albmeta);
									//album4->Release();

									int w,h;
									ARGB32 *bits;
									if (AGAVE_API_ALBUMART->GetAlbumArt(t->file, L"cover", &w, &h, &bits) == ALBUMART_SUCCESS)
									{
										setArt((songid_t)song,bits,w,h);
										WASABI_API_MEMMGR->sysFree(bits);
									}
								}
							}
						}
					}
				}
			}
		}
		if(!*t->songid) t->callback(t->callbackContext,WASABI_API_LNGSTRINGW(IDS_UNSPECIFIED_ERROR));
		t->phase = PHASE_DONE;
		//OutputDebugString(L"phase finish finished");
		break;
	}
}

int P4SDevice::transferTrackToDevice(const itemRecordW * track,void * callbackContext,void (*callback)(void * callbackContext, wchar_t * status),songid_t * songid,int * killswitch) {
	wchar_t file[2048] = {0};
	StringCchCopy(file, ARRAYSIZE(file), track->filename);
	bool deletefile = false;
	if(transcoder) if(transcoder->ShouldTranscode(file)) {
		wchar_t newfile[MAX_PATH] = {0};
		wchar_t ext[10] = {0};
		transcoder->CanTranscode(file,ext);
		transcoder->GetTempFilePath(ext,newfile);
		if(transcoder->TranscodeFile(file,newfile,killswitch,callback,callbackContext)) return -1;
		StringCchCopy(file, ARRAYSIZE(file), newfile);
		deletefile=true;
	}

	callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_WAITING_FOR_OTHER_TRANSFERS));
	EnterCriticalSection(&csTransfers); // only one transfer at once, globally :(
	callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING));
	TransferItem t;
	t.file = file;
	t.track = track;
	t.callback = callback;
	t.callbackContext = callbackContext;
	t.killswitch = killswitch;
	t.songid = songid;
	t.dev = this;
	t.phase = PHASE_START;
	t.pc = 0;
	*songid = NULL;
	this->doTransfer(&t); // do the transfer
	if(t.phase == PHASE_FINISH) this->doTransfer(&t); // finish it, if needs be.

	int ret = (*songid)?0:-1;
	if(ret==0) callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_DONE));
	trackRemovedFromTransferQueue(track);
	LeaveCriticalSection(&csTransfers);
	if(deletefile) _wunlink(file);
	return ret;
}

bool extentionSupported(IWMDMDevice3* WMDevice, wchar_t * ext,bool aac_and_m4a_support, bool video_supported) {
	if(!ext) return false;
	bool supported=false;

	if(!_wcsicmp(ext,L".mp3")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_MP3,WMDevice));
	else if(!_wcsicmp(ext,L".wma")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_WMA,WMDevice));
	else if(!_wcsicmp(ext,L".wav")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_WAVE,WMDevice));
	else if(!_wcsicmp(ext,L".aa")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_AUDIBLE,WMDevice));
	else if(!_wcsicmp(ext,L".m4a") || !_wcsicmp(ext,L".aac")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_AAC,WMDevice)) || SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_MP4,WMDevice));
	else if(!_wcsicmp(ext,L".ogg")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_OGG,WMDevice));
	else if(!_wcsicmp(ext,L".flac") || !_wcsicmp(ext,L".fla")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_FLAC,WMDevice));
	else if(!_wcsicmp(ext,L".avi")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_AVI,WMDevice));
	else if(!_wcsicmp(ext,L".mpg") || !_wcsicmp(ext,L".mpeg")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_MPEG,WMDevice));
	else if(!_wcsicmp(ext,L".asf")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_ASF,WMDevice));
	else if(!_wcsicmp(ext,L".wmv")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_WMV,WMDevice));
	else if(!_wcsicmp(ext,L".mp4")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_MP4,WMDevice)) || SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_AAC,WMDevice));
	else if(!_wcsicmp(ext,L".m4v")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_MP4,WMDevice));
	else if(!_wcsicmp(ext,L".mp2")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_MP2,WMDevice));
	else if(!_wcsicmp(ext,L".3gp")) supported = SUCCEEDED(GetFormatCaps(WMDM_FORMATCODE_3GP,WMDevice));
	else return false;

	if(!supported) {
		if(!_wcsicmp(ext,L".mp3") || !_wcsicmp(ext,L".wma") || !_wcsicmp(ext,L".wav")) supported=true;
		if(aac_and_m4a_support && (!_wcsicmp(ext,L".m4a") || !_wcsicmp(ext,L".aac"))) supported=true;
		if(video_supported && (!_wcsicmp(ext,L".asf") || !_wcsicmp(ext,L".avi") || !_wcsicmp(ext,L".mpeg") || !_wcsicmp(ext,L".mpg") || !_wcsicmp(ext,L".wmv"))) supported=true;
	}
	return supported;
}
/*
bool extentionSupported(wchar_t * ext,bool aac_and_m4a_support) {
	if(!ext) return false;
	if(_wcsicmp(ext,L".mp3") && _wcsicmp(ext,L".wma") && _wcsicmp(ext,L".wav")
	   && (aac_and_m4a_support || (_wcsicmp(ext,L".m4a") && _wcsicmp(ext,L".aac")))
      ) return false;
	return true;
}
*/
static __int64 fileSize(wchar_t * filename)
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

int P4SDevice::trackAddedToTransferQueue(const itemRecordW * track) {
	__int64 l;
	if(transcoder && transcoder->ShouldTranscode(track->filename)) {
		int k = transcoder->CanTranscode(track->filename);
		if(k == -1) return -2;
		if(k == 0) l = (__int64)fileSize(track->filename);
		else l = (__int64)k;
	} else {
		wchar_t * ext = wcsrchr(track->filename,'.');
		if(!extentionSupported(WMDevice,ext,noMetadata,supportsVideo)) return -2; // fucko: assumes all noMetadata devices are nokia (which is true for now)
		l = (__int64)fileSize(track->filename);
	}
	__int64 test = l;
	__int64 avail = getDeviceCapacityAvailable();
	test += transferQueueSize;
	//test += (__int64)3000000;
	if(test > avail) return -1;
	transferQueueSize += l;
	return 0;
}

void P4SDevice::trackRemovedFromTransferQueue(const itemRecordW * track) {
	__int64 l = (__int64)fileSize(track->filename);
	if(transcoder && transcoder->ShouldTranscode(track->filename)) {
		int k = transcoder->CanTranscode(track->filename);
		if(k != -1 && k != 0) l = (__int64)k;
	}
	transferQueueSize -= l;
}

__int64 P4SDevice::getTrackSizeOnDevice(const itemRecordW * track) {
	if(transcoder && transcoder->ShouldTranscode(track->filename)) {
		int k = transcoder->CanTranscode(track->filename);
		if(k != -1 && k != 0) return k;
	}
	wchar_t * ext = wcsrchr(track->filename,'.');
	if(!extentionSupported(WMDevice,ext,noMetadata,supportsVideo)) return 0; // fucko: assumes all noMetadata devices are nokia (which is true for now)
	return fileSize(track->filename);
}

int P4SDevice::getPlaylistCount() {
	return playlists.GetSize();
}

static BYTE* GetMetadataItem(IWMDMMetaData *meta, const WCHAR * name) {
	WMDM_TAG_DATATYPE type;
	BYTE * value=NULL;
	UINT len;
	if(!meta) { return (BYTE*)""; } //  OutputDebugString(L"no meta"); 
	if((meta->QueryByName(name,&type,&value,&len)) != S_OK) { return NULL; /*value;*/ } //wchar_t buf[100]; wsprintf(buf,L"meta fail: %x %s",hr,name); OutputDebugString(buf);
	return value;
}

static BYTE* GetMetadataItem(Song * song, const WCHAR * name) {
	WMDM_TAG_DATATYPE type;
	BYTE * value=NULL;
	UINT len;
	if(!song || !(song->meta)) { return (BYTE*)""; } //  OutputDebugString(L"no meta"); 
	if((song->meta->QueryByName(name,&type,&value,&len)) != S_OK) { return NULL; /*value;*/ } //wchar_t buf[100]; wsprintf(buf,L"meta fail: %x %s",hr,name); OutputDebugString(buf);
	return value;
}

void P4SDevice::getPlaylistName(int playlistnumber, wchar_t * buf, int len) 
{
	if (NULL == buf)
		return;

	if(playlistnumber == 0) 
	{
		if (NULL == WMDevice)
			buf[0] = L'\0';
		else
		{
			HRESULT hr;
			hr = WMDevice->GetName(buf, len);
			if (FAILED(hr))
			{
				buf[0] = L'\0';
			}
		}
	}
	else
	{
		StringCchCopy(buf, len, ((Playlist *)playlists.Get(playlistnumber))->name);
	}
}

int P4SDevice::getPlaylistLength(int playlistnumber) {
	if(playlistnumber == -1) return 0;
	return ((Playlist*)playlists.Get(playlistnumber))->songs.GetSize();
}

songid_t P4SDevice::getPlaylistTrack(int playlistnumber,int songnum) {
	if(playlistnumber == -1) return NULL;
	return (songid_t)((Playlist*)playlists.Get(playlistnumber))->songs.Get(songnum);
}

void P4SDevice::setPlaylistName(int playlistnumber, const wchar_t *buf) {
	if(playlistnumber == -1) return;
	IWMDMStorageControl3 * storeControl=NULL;
	Playlist * pl = (Playlist *)playlists.Get(playlistnumber);
	lstrcpyn(pl->name,buf,128);
	pl->storage->QueryInterface(&storeControl);
	pl->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMTitle,(BYTE*)buf,(wcslen(buf)*2)+2);
	pl->storage->SetMetadata(pl->meta);
	wchar_t buffer[256] = {0};
	wsprintf(buffer,L"%s.%s",buf,plext);
	if(storeControl) {
	    storeControl->Rename(WMDM_MODE_BLOCK,buffer,NULL);
		storeControl->Release();
	}
}

int sortby;
//Device * sortDev;

#define SKIP_THE_AND_WHITESPACE(x) { while (!iswalnum(*x) && *x) x++; if (!_wcsnicmp(x,L"the ",4)) x+=4; while (*x == L' ') x++; }
int STRCMP_NULLOK(const wchar_t *pa, const wchar_t *pb) {
	if (!pa) pa=L"";
	else SKIP_THE_AND_WHITESPACE(pa)
	if (!pb) pb=L"";
	else SKIP_THE_AND_WHITESPACE(pb)
	return lstrcmpi(pa,pb);
}
#undef SKIP_THE_AND_WHITESPACE

static int sortFunc(const void *elem1, const void *elem2)
{
	int use_by = sortby;
	songid_t a=(songid_t)*(songid_t *)elem1;
	songid_t b=(songid_t)*(songid_t *)elem2;

#define RETIFNZ(v) if ((v)!=0) return v;

	// this might be too slow, but it'd be nice
	int x;
	for (x = 0; x < 5; x ++)
	{
		if (use_by == SORTBY_TITLE) // title -> artist -> album -> disc -> track
		{
			wchar_t bufa[2048] = {0};
			wchar_t bufb[2048] = {0};
			sortDev->getTrackTitle(a,bufa,2048);
			sortDev->getTrackTitle(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=SORTBY_ARTIST;
		}
		else if (use_by == SORTBY_ARTIST) // artist -> album -> disc -> track -> title
		{
			wchar_t bufa[2048] = {0};
			wchar_t bufb[2048] = {0};
			sortDev->getTrackArtist(a,bufa,2048);
			sortDev->getTrackArtist(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=SORTBY_ALBUM;
		}
		else if (use_by == SORTBY_ALBUM) // album -> disc -> track -> title -> artist
		{
			wchar_t bufa[2048] = {0};
			wchar_t bufb[2048] = {0};
			sortDev->getTrackAlbum(a,bufa,2048);
			sortDev->getTrackAlbum(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=SORTBY_DISCNUM;
	    }
	    else if (use_by == SORTBY_DISCNUM) // disc -> track -> title -> artist -> album
		{
			int v1=sortDev->getTrackDiscNum(a);
			int v2=sortDev->getTrackDiscNum(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=SORTBY_TRACKNUM;
		}
		else if (use_by == SORTBY_TRACKNUM) // track -> title -> artist -> album -> disc
		{
			int v1=sortDev->getTrackTrackNum(a);
			int v2=sortDev->getTrackTrackNum(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=SORTBY_TITLE;     
		}
		else if (use_by == SORTBY_GENRE) // genre -> artist -> album -> disc -> track
		{
			wchar_t bufa[2048] = {0};
			wchar_t bufb[2048] = {0};
			sortDev->getTrackGenre(a,bufa,2048);
			sortDev->getTrackGenre(b,bufb,2048);
			int v=STRCMP_NULLOK(bufa,bufb);
			RETIFNZ(v)
			use_by=SORTBY_ARTIST;
		}
		else if (use_by == SORTBY_PLAYCOUNT) // size -> artist -> album -> disc -> track
		{
			int v1=sortDev->getTrackPlayCount(a);
			int v2=sortDev->getTrackPlayCount(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=SORTBY_ARTIST;
		}
		else if (use_by == SORTBY_RATING) // size -> artist -> album -> disc -> track
		{
			int v1=sortDev->getTrackRating(a);
			int v2=sortDev->getTrackRating(b);
			if (v1<0)v1=0;
			if (v2<0)v2=0;
			RETIFNZ(v1-v2)
			use_by=SORTBY_ARTIST;
		}
		else if (use_by == SORTBY_LASTPLAYED)
		{
			__time64_t la = sortDev->getTrackLastPlayed(a);
			__time64_t lb = sortDev->getTrackLastPlayed(b);
			double t = difftime((time_t)la,(time_t)lb);
			int v = t>0?1:(t<0?-1:0);
			RETIFNZ(v)
			use_by=SORTBY_ARTIST;
		}
		else break; // no sort order?
	}

	return 0;
}

void P4SDevice::sortPlaylist(int playlistnumber, int sortBy) {
	if(playlistnumber == -1) return;
	sortby = sortBy;
	sortDev = this;
	Playlist * pl = (Playlist *)playlists.Get(playlistnumber);
	qsort(pl->songs.GetAll(),pl->songs.GetSize(),sizeof(void*),sortFunc);
	pl->modified = true;
}

void P4SDevice::playlistSwapItems(int playlistnumber, int posA, int posB) {
	if(playlistnumber == -1) return;
	Playlist * pl = (Playlist*)playlists.Get(playlistnumber);
	if(posA >= pl->songs.GetSize() || posB >= pl->songs.GetSize()) return;
	void * a = pl->songs.Get(posA);
	void * b = pl->songs.Get(posB);
	pl->songs.Set(posA,b);
	pl->songs.Set(posB,a);
	pl->modified=true;
}

void P4SDevice::addTrackToPlaylist(int playlistnumber, songid_t songid) {
	if(playlistnumber == -1) return;
	Playlist * pl = (Playlist*)playlists.Get(playlistnumber);
	pl->songs.Add((void*)songid);
	pl->modified=true;
}

void P4SDevice::removeTrackFromPlaylist(int playlistnumber, int songnum) {
	if(playlistnumber == -1) return;
	Playlist * pl = (Playlist*)playlists.Get(playlistnumber);
	pl->songs.Del(songnum);
	pl->modified=true;
}

void P4SDevice::deletePlaylist(int playlistnumber) {
	if(playlistnumber == -1) return;
	IWMDMStorageControl3 * storeControl;
	Playlist * pl = (Playlist *)playlists.Get(playlistnumber);
	pl->storage->QueryInterface(&storeControl);

	storeControl->Delete(WMDM_MODE_BLOCK,NULL);
	pl->meta->Release();
	pl->storage->Release();
	playlists.Del(playlistnumber);

	storeControl->Release();
}

int P4SDevice::newPlaylist(const wchar_t *name) {
	IWMDMStorageControl3 * storeControl;
	if(!playlistsDir) return -1;
	playlistsDir->QueryInterface(&storeControl);
	DWORD dw=WMDM_FORMATCODE_ABSTRACTAUDIOVIDEOPLAYLIST;
	IWMDMMetaData* meta = NULL;
	int ret = -1;
	if (SUCCEEDED(playlistsDir->CreateEmptyMetadataObject(&meta)) && meta)
	{
		meta->AddItem(WMDM_TYPE_DWORD, g_wszWMDMFormatCode, (BYTE *)&dw, sizeof(dw));
		meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMTitle,(BYTE*)name,(wcslen(name)*2)+2);
		IWMDMStorage * newpl=NULL;
		wchar_t buffer[MAX_PATH] = {0};
		wsprintf(buffer,L"%s.%s",name,plext);
		storeControl->Insert3(WMDM_MODE_BLOCK | WMDM_CONTENT_FILE,0,NULL,buffer,NULL,NULL,meta,NULL,&newpl);

		if(newpl) {
			IWMDMStorage4 * newpl4=NULL;
			newpl->QueryInterface(&newpl4);
			Playlist * pl = new Playlist;
			lstrcpyn(pl->name,name,128);
			pl->storage = newpl4;
			pl->modified = false;
			pl->meta = meta;
			playlists.Add(pl);
			ret = playlists.GetSize() - 1;
			newpl->Release();
		}
	}
	storeControl->Release();

	return ret;
}

void P4SDevice::getTrackArtist(songid_t songid, wchar_t * buf, int len) {
	buf[0]=0;
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMAuthor);
	if(b) { lstrcpyn(buf,(wchar_t*)b,len); CoTaskMemFree(b); }
	if(noMetadata || !b) {
		if(!buf[0]) { // guess based upon file path
			Song *s = (Song *)songid;
			if(s->artist) {lstrcpyn(buf,s->artist,len); 
			return;
		}
		IWMDMStorage * p = NULL;
		if(s->storage->GetParent(&p) == S_OK) {
			IWMDMStorage * p2 = NULL;
			IWMDMStorage4 * p3 = NULL;
			if (SUCCEEDED(p->QueryInterface(&p3))) {
				if(p3->GetParent(&p2) == S_OK) {
					p2->GetName(buf,len);
					p2->Release();
				}
				p3->Release();
			}
			p->Release();
		}
	}
}
}

void P4SDevice::getTrackAlbum(songid_t songid, wchar_t * buf, int len) {
	buf[0]=0;
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMAlbumTitle);
	if(b) { lstrcpyn(buf,(wchar_t*)b,len); CoTaskMemFree(b); }
	if(!b || noMetadata || ((Song*)songid)->video) {
		if(!buf[0]) { // guess based upon file path
		Song *s = (Song *)songid;
		if(s->album) {lstrcpyn(buf,s->album,len); return;}
		IWMDMStorage * p = NULL;
		if(s->storage->GetParent(&p) == S_OK) {
			p->GetName(buf,len);
			p->Release();
		}
	}
	}
	if(buf[0] == L'~' && buf[1] == 0) buf[0]=0;
}
void P4SDevice::getTrackTitle(songid_t songid, wchar_t * buf, int len) {
	buf[0]=0;
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMTitle);
	if(b) { lstrcpyn(buf,(wchar_t*)b,len); CoTaskMemFree(b); }
	if(!b || noMetadata || ((Song*)songid)->video) {
		if(!buf[0]) { // guess based upon file name
		Song *s = (Song *)songid;
		wchar_t buf2[256]=L"";
		s->storage->GetName(buf2,256);
		wchar_t * n = wcsrchr(buf2,L'-');
		if(n) {
	        while(n && (*n == L'-' || *n == L' ' || *n == L'_' || *n == L'.')) n++;
			lstrcpyn(buf,n,len);
		} else lstrcpyn(buf,buf2,len);
	}
    wchar_t * ext = wcsrchr(buf,L'.');
    if(ext) *ext=0;
}
}
void P4SDevice::getTrackGenre(songid_t songid, wchar_t * buf, int len) {
	buf[0]=0;
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMGenre);
	if(b) { lstrcpyn(buf,(wchar_t*)b,len); CoTaskMemFree(b); }
}

int P4SDevice::getTrackTrackNum(songid_t songid) {
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMTrack);
	int r = b?(int)*((DWORD*)b):0; if(b) CoTaskMemFree(b); 
	if(r) return r;
	if(!b || noMetadata) { // guess based upon file name
		Song *s = (Song *)songid;
		wchar_t buf2[256]=L"";
		s->storage->GetName(buf2,256);

		wchar_t * n = buf2; //wcschr(buf2,L'-');
		while(n) {
			while((*n == L'-' || *n == L' ' || *n == L'_' || *n == L'.') && *n) n++;
			if(!n) break;
			int m=0; while(*(n+m)>=L'0' && *(n+m)<=L'9') m++;
			if(m == 2) { *(n+m)=0; return _wtoi(n); }
			n = wcschr(n,L'-');
		}
		/*
		wchar_t * n = wcschr(buf2,L'-');
		if(n) {
			*(n--)=0;
			while((*n == L'-' || *n == L' ' || *n == L'_' || *n == L'.') && n > buf2) *(n--)=0;
			return _wtoi(buf2);
		}
		*/
	}
	return 0;
}

int P4SDevice::getTrackYear(songid_t songid)  {
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMYear);
	int r = b?_wtoi((wchar_t*)b):0; if(b) CoTaskMemFree(b); return r;
}
__int64 P4SDevice::getTrackSize(songid_t songid) {
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMFileSize);
	int r = b?(int)*((__int64*)b):0; if(b) CoTaskMemFree(b);
	if(r) return r;
	DWORD high, low;
	((Song *)songid)->storage->GetSize(&low,&high);
	ULARGE_INTEGER u;
	u.HighPart = high;
	u.LowPart = low;
	return u.QuadPart;
}

int P4SDevice::getTrackLength(songid_t songid) {
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMDuration);
	int r = b?(int)(*((__int64*)b)/10000):0; if(b) CoTaskMemFree(b); return r;
}

int P4SDevice::getTrackBitrate(songid_t songid) {
	int len = getTrackLength(songid) / 8000;
	return len?(int)(getTrackSize(songid)/1024) / len:0;
}

int P4SDevice::getTrackPlayCount(songid_t songid) {
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMPlayCount);
	int r = b?(int)*((DWORD*)b):0; if(b) CoTaskMemFree(b); return r;
}

int P4SDevice::getTrackRating(songid_t songid) {
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMUserRating);
	int r = b?(int)*((DWORD*)b):0; if(b) CoTaskMemFree(b); return r;
}

__time64_t P4SDevice::getTrackLastPlayed(songid_t songid) {
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMUserLastPlayTime);
	__time64_t r = b?wmdmDateTimeToUnixTime((_WMDMDATETIME *)b):0; if(b) CoTaskMemFree(b); return r;
}

__time64_t P4SDevice::getTrackLastUpdated(songid_t songid) {
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMLastModifiedDate);
	__time64_t r = b?wmdmDateTimeToUnixTime((_WMDMDATETIME *)b):0; if(b) CoTaskMemFree(b); return r;
}

void P4SDevice::getTrackAlbumArtist(songid_t songid, wchar_t * buf, int len) {
	buf[0]=0;
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMAlbumArtist);
	if(b) { lstrcpyn(buf,(wchar_t*)b,len); CoTaskMemFree(b); }
	if(!buf[0]) getTrackArtist(songid,buf,len);
}

void P4SDevice::getTrackComposer(songid_t songid, wchar_t * buf, int len) {
	buf[0]=0;
	BYTE * b = GetMetadataItem((Song *)songid,g_wszWMDMComposer);
	if(b) { lstrcpyn(buf,(wchar_t*)b,len); CoTaskMemFree(b); }
}

int P4SDevice::getTrackType(songid_t songid) {
	Song * s = (Song *)songid;
	return s->video;
}

void P4SDevice::getTrackExtraInfo(songid_t songid, const wchar_t * field, wchar_t * buf, int len) {
	if(!wcscmp(field,FIELD_EXTENSION)) {
		Song * s = (Song *)songid;
		wchar_t buf2[2048] = {0};
		s->storage->GetName(buf2,2048);
		wchar_t * ext = wcsrchr(buf2,L'.');
		if(ext) { ext++; lstrcpyn(buf,ext,len); }
	}
}

void P4SDevice::PreCommit(Song * s) {
	if(!lastChange) lastChange = s;
	else if(s != lastChange) {
		if(lastChange->modified) {
			lastChange->storage->SetMetadata(lastChange->meta);
			lastChange->meta->Release();
			lastChange->storage->GetMetadata(&lastChange->meta);
			lastChange->modified = false;
		}
		lastChange = s;
	}
}

void P4SDevice::setTrackArtist(songid_t songid, const wchar_t * value) {
	((Song*)songid)->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAuthor,(BYTE*)value,wcslen(value)*2+2);
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackAlbum(songid_t songid, const wchar_t * value) {
	((Song*)songid)->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAlbumTitle,(BYTE*)value,wcslen(value)*2+2);
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackTitle(songid_t songid, const wchar_t * value) {
	((Song*)songid)->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMTitle,(BYTE*)value,wcslen(value)*2+2);
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackGenre(songid_t songid, const wchar_t * value) {
	((Song*)songid)->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMGenre,(BYTE*)value,wcslen(value)*2+2);
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackTrackNum(songid_t songid, int value) {
	((Song*)songid)->meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMTrack,(BYTE*)&value,sizeof(DWORD));
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackYear(songid_t songid, int value) {
	wchar_t buf[10] = {0}; wsprintf(buf,L"%d",value);
	((Song*)songid)->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMYear,(BYTE*)buf,wcslen(buf)*2+2);
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackPlayCount(songid_t songid, int value) {
	((Song*)songid)->meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMPlayCount,(BYTE*)&value,sizeof(DWORD));
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackRating(songid_t songid, int value) {
	((Song*)songid)->meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMUserRating,(BYTE*)&value,sizeof(DWORD));
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackLastPlayed(songid_t songid, __time64_t value) {
	_WMDMDATETIME time={0};
	getTime(value,&time);  
	((Song*)songid)->meta->AddItem(WMDM_TYPE_DATE,g_wszWMDMUserLastPlayTime,(BYTE*)&time,sizeof(_WMDMDATETIME));
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackLastUpdated(songid_t songid, __time64_t value) {
	_WMDMDATETIME time={0};
	getTime(value,&time);  
	((Song*)songid)->meta->AddItem(WMDM_TYPE_DATE,g_wszWMDMLastModifiedDate,(BYTE*)&time,sizeof(_WMDMDATETIME));
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackAlbumArtist(songid_t songid, const wchar_t * value) {
	((Song*)songid)->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAlbumArtist,(BYTE*)value,wcslen(value)*2+2);
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

void P4SDevice::setTrackComposer(songid_t songid, const wchar_t * value) {
	((Song*)songid)->meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMComposer,(BYTE*)value,wcslen(value)*2+2);
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
}

int P4SDevice::copyToHardDrive(songid_t s,wchar_t * path,void * callbackContext,void (*callback)(void * callbackContext, wchar_t * status),int * killswitch)
{
	callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_WAITING));
	EnterCriticalSection(&csTransfers);
	Song * song = (Song*)s;
	IWMDMStorageControl * control;
	if (!SUCCEEDED(song->storage->QueryInterface(&control))) {
		callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_FAILED));
		return -1;
	}
	wchar_t fn[2084] = {0};
	wchar_t *ext = 0;
	if(SUCCEEDED(song->storage->GetName(fn,2084)) && (ext=wcsrchr(fn,L'.'))!=0) 
		wcscat(path,ext);
	int ret=-1;
	callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING));
	TransferItem t={0};
	t.callback = callback;
	t.callbackContext = callbackContext;
	t.killswitch = killswitch;
	t.progress = new MyProgress(&t);
	if(SUCCEEDED(control->Read(WMDM_MODE_BLOCK | WMDM_CONTENT_FILE,path,t.progress,NULL))) ret=0;
	control->Release();
	t.progress->Release();
	callback(callbackContext,WASABI_API_LNGSTRINGW((ret==0?IDS_DONE:IDS_FAILED)));
	LeaveCriticalSection(&csTransfers);
	return ret;
}

static IWMDMStorage4* getAlb(P4SDevice * dev, songid_t songid, bool create) {
	wchar_t alb[1024] = {0};
	dev->getTrackAlbum(songid,alb,1020);
	StringCchCat(alb, ARRAYSIZE(alb), L".alb");
	Song * song = (Song*)songid;
	IWMDMStorage * album0=NULL;
	IWMDMStorage * parent=NULL;
	IWMDMStorage4 * parent4=NULL;
	IWMDMStorage4 * album4=NULL;
	song->storage->GetParent(&parent);
	if (parent)
	{
		parent->QueryInterface(&parent4);
		parent->Release();
		if (parent4)
		{
			parent4->GetStorage(alb,&album0);
			//parent4->Release();
			if(album0) {
				album0->QueryInterface(&album4);
				album0->Release();
			}
		}
	}
	if(album4 || !create) { parent4->Release(); return album4; }
	if(!parent4) return NULL;
	// create my own
	album0=0;
	IWMDMStorageControl3 * storeControl=0;
	parent4->QueryInterface(&storeControl);
	if(storeControl) {
		IWMDMMetaData * meta=0;
		parent4->CreateEmptyMetadataObject(&meta);
		if(meta) {
			DWORD formatCode = WMDM_FORMATCODE_ABSTRACTAUDIOALBUM;
			wchar_t album[256]=L"";
			wchar_t artist[256]=L"";
			wchar_t genre[256]=L"";
			dev->getTrackAlbumArtist(songid,artist,256);
			dev->getTrackAlbum(songid,album,256);
			dev->getTrackGenre(songid,genre,256);
			meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMFormatCode,(BYTE*)&formatCode,sizeof(DWORD));
			meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAlbumTitle,(BYTE*)album,(wcslen(album)*2)+2);
			meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMAlbumArtist,(BYTE*)artist,(wcslen(artist)*2)+2);
			meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMTitle,(BYTE*)album,(wcslen(album)*2)+2);
			meta->AddItem(WMDM_TYPE_STRING,g_wszWMDMGenre,(BYTE*)genre,(wcslen(genre)*2)+2);

			storeControl->Insert3(WMDM_MODE_BLOCK | WMDM_CONTENT_FILE,0,NULL,alb,NULL,NULL,meta,NULL,&album0);
			meta->Release();
		}
		storeControl->Release();
	}
	parent4->Release();
	if(album0) {
		album0->QueryInterface(&album4);
		album0->Release();
		return album4;
	}
	return NULL;
}

void P4SDevice::setArt(songid_t songid, void *buf, int w, int h) { //buf is in format ARGB32*
	if(!songid) return;
	Song * s = (Song *)songid;
	IWMDMStorage4 * album = s->alb;
	if(!album) {
		album = getAlb(this,songid,true);
		if(!album) return;
		s->alb = album;
		s->albmeta = NULL;
		s->alb->GetMetadata(&s->albmeta);
		if(!s->albmeta) return;
	}

	IWMDMMetaData *meta=s->albmeta;
	if(!meta) return;
	IWMDMMetaData *meta2 = ((Song*)songid)->meta;

	if(meta || meta2) {
		if(buf) {
			SkinBitmap art((ARGB32*)buf,w,h);
			w=h=120;
			BltCanvas artc(w,h);
			art.stretch(&artc,0,0,w,h);

			const GUID JPEGwriteguid = { 0x7bc27468, 0x475, 0x4c0d, { 0xae, 0xed, 0xc, 0x51, 0x19, 0x5d, 0xc2, 0xea } };
			svc_imageWriter* jpgWrite=NULL;
			waServiceFactory *sf = plugin.service->service_getServiceByGuid(JPEGwriteguid);
			if(sf) jpgWrite = (svc_imageWriter*)sf->getInterface();
			if(jpgWrite) {
				int length=0;
				void *jpeg = jpgWrite->convert(artc.getBits(),32,w,h,&length);
				if(jpeg) {
					DWORD fmt = WMDM_FORMATCODE_IMAGE_EXIF; // this is the formatcode for jpeg, apparently.
					if(meta) {
						meta->AddItem(WMDM_TYPE_BINARY,g_wszWMDMAlbumCoverData,(BYTE*)jpeg,length);
						meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverWidth,(BYTE*)&w,sizeof(DWORD));
						meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverHeight,(BYTE*)&h,sizeof(DWORD));
						meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverSize,(BYTE*)&length,sizeof(DWORD));
						meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverFormat,(BYTE*)&fmt,sizeof(DWORD));
					}
					if(meta2) {
						meta2->AddItem(WMDM_TYPE_BINARY,g_wszWMDMAlbumCoverData,(BYTE*)jpeg,length);
						meta2->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverWidth,(BYTE*)&w,sizeof(DWORD));
						meta2->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverHeight,(BYTE*)&h,sizeof(DWORD));
						meta2->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverSize,(BYTE*)&length,sizeof(DWORD));
						meta2->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverFormat,(BYTE*)&fmt,sizeof(DWORD));
					}
					WASABI_API_MEMMGR->sysFree(jpeg);
				}
				if (sf) sf->releaseInterface(jpgWrite);
			}
		} else { // remove art
			if(meta) {
				meta->AddItem(WMDM_TYPE_BINARY,g_wszWMDMAlbumCoverData,(BYTE*)0,0);
				meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverWidth,(BYTE*)0,0);
				meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverHeight,(BYTE*)0,0);
				meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverSize,(BYTE*)0,0);
				meta->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverFormat,(BYTE*)0,0);
			}
			if(meta2) {
				meta2->AddItem(WMDM_TYPE_BINARY,g_wszWMDMAlbumCoverData,(BYTE*)0,0);
				meta2->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverWidth,(BYTE*)0,0);
				meta2->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverHeight,(BYTE*)0,0);
				meta2->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverSize,(BYTE*)0,0);
				meta2->AddItem(WMDM_TYPE_DWORD,g_wszWMDMAlbumCoverFormat,(BYTE*)0,0);
			}
		}
		if(meta) {
			album->SetMetadata(meta);
		}
	}
	((Song*)songid)->modified=true;
	PreCommit((Song*)songid);
/*
g_wszWMDMAlbumCoverData Album art JPEG byte blob WMDM_TYPE_BINARY BYTE* 
g_wszWMDMAlbumCoverDuration Album cover duration WMDM_TYPE_DWORD DWORD 
g_wszWMDMAlbumCoverFormat Album art format WMDM_TYPE_DWORD DWORD 
g_wszWMDMAlbumCoverHeight Album art height WMDM_TYPE_DWORD DWORD 
g_wszWMDMAlbumCoverSize Album art size WMDM_TYPE_DWORD DWORD 
g_wszWMDMAlbumCoverWidth Album art width WMDM_TYPE_DWORD DWORD 
*/
}

class Art {
public:
	Art(void * jpegData, int jpegDataLen, int w, int h) : jpegData(jpegData), jpegDataLen(jpegDataLen), w(w), h(h), data(0), resized(0) {
	}
	~Art() {
		CoTaskMemFree(jpegData);
		if(data) WASABI_API_MEMMGR->sysFree(data); data=0;
	}
	ARGB32 * GetImage() {
		if(data) return data;
		const GUID JPEGguid = { 0xae04fb30, 0x53f5, 0x4032, { 0xbd, 0x29, 0x3, 0x2b, 0x87, 0xec, 0x34, 0x04 } };
		svc_imageLoader* jpgLoad=NULL;
		waServiceFactory *sf = plugin.service->service_getServiceByGuid(JPEGguid);
		if(sf) jpgLoad = (svc_imageLoader*)sf->getInterface();
		if(jpgLoad) {
			data = jpgLoad->loadImage(jpegData,jpegDataLen,&w,&h);
			if (sf) sf->releaseInterface(jpgLoad);
		}
		resized=0;
		return data;
	}
	int resized;
	void Resize(int width, int height) {
		if(w == width && h == height) return;
		if(resized) {
			if(data) WASABI_API_MEMMGR->sysFree(data);
			data=0;
			resized=0;
		}
		GetImage();
		if(!data) return;
		SkinBitmap temp(data,w,h);
		BltCanvas newImage(width,height);
		temp.stretch(&newImage,0,0,width,height);
		w=width;
		h=height;
		WASABI_API_MEMMGR->sysFree(data);
		data = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(w*h*sizeof(ARGB32));
		memcpy(data,newImage.getBits(),w*h*sizeof(ARGB32));
		resized=1;
	}
	int getWidth() {return w;}
	int getHeight() {return h;}
	int cmp(Art * art) {
		if(art->jpegDataLen != jpegDataLen) return art->jpegDataLen - jpegDataLen;
		return memcmp(art->jpegData,jpegData,jpegDataLen);
	}
protected:
	void * jpegData;
	int jpegDataLen;
	int w,h;
	ARGB32 *data;
};

pmpart_t P4SDevice::getArt(songid_t songid) {
	if(!songid) return NULL;
	Song* s = (Song*)songid;
	WMDM_TAG_DATATYPE type;
	BYTE * data=NULL;
	UINT length=0;
	IWMDMMetaData *meta = s->albmeta;
	if(!meta) return NULL;

	HRESULT hr = meta->QueryByName(g_wszWMDMAlbumCoverData,&type,&data,&length);

	if(hr == S_OK && data && length) {
		BYTE * b = GetMetadataItem(meta,g_wszWMDMAlbumCoverWidth);
		int w = b?(int)*((DWORD*)b):0;
		if(b) CoTaskMemFree(b);

		b = GetMetadataItem(meta,g_wszWMDMAlbumCoverHeight);
		int h = b?(int)*((DWORD*)b):0;
		if(b) CoTaskMemFree(b);
		//meta->Release();
		if(!w) w=120; // this happens if the device doesn't store the w and h of the image.
		if(!h) h=120; // but it's ok, cause the real values are in the jpeg data, and these can just be a guide.
		return (pmpart_t) new Art(data,length,w,h);
	}
	//meta->Release();
	if(data) CoTaskMemFree(data);
	return NULL;
}

void P4SDevice::releaseArt(pmpart_t art) {
	if(art) delete ((Art *)art);
}

int P4SDevice::drawArt(pmpart_t art0, HDC dc, int x, int y, int w, int h) {
	Art* art = (Art*)art0;
	if(!art) return 0;
	ARGB32 * d = art->GetImage();
	if(!d) return 0;
	SkinBitmap(d, art->getWidth(), art->getHeight()).stretch(&DCCanvas(dc),x,y,w,h); // wrap into a SkinBitmap (no copying involved)
	return 1;
}

void P4SDevice::getArtNaturalSize(pmpart_t art0, int *w, int *h) {
	Art* art = (Art*)art0;
	*w=art->getWidth();
	*h=art->getWidth();
	if(*w==0 || *h==0) *w=*h=120;
}

void P4SDevice::setArtNaturalSize(pmpart_t art0, int w, int h) {
	Art* art = (Art*)art0;
	art->Resize(w,h);
}

void P4SDevice::getArtData(pmpart_t art0, void* data) { // data ARGB32* is at natural size
	Art* art = (Art*)art0;
	int w,h;
	getArtNaturalSize(art0,&w,&h);
	setArtNaturalSize(art0,w,h);
	ARGB32 * d = art->GetImage();
	if(d) memcpy(data,d,w*h*sizeof(ARGB32));
}

bool P4SDevice::artIsEqual(pmpart_t a, pmpart_t b) {
	if(!a || !b) return false;
	return ((Art*)a)->cmp((Art*)b) == 0;
}

extern void checkForDevices();

intptr_t P4SDevice::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4) {
	switch(param1) {
	    case DEVICE_SET_ICON: // icons
		{
	        MLTREEIMAGE * i = (MLTREEIMAGE*)param2;
			if(wcsstr(name,L"Zen")) {
				i->hinst = plugin.hDllInstance;
				i->resourceId = IDR_CREATIVE_ZEN_ICON;
			} else if (wcsstr(name,L"Nokia")) {
				i->hinst = plugin.hDllInstance;
				i->resourceId = IDR_NOKIA_ICON;
			}
			break;
		}
		case DEVICE_SUPPORTED_METADATA:
			return noMetadata ? 0x8f : (0xffef | (requiresALB?SUPPORTS_ALBUMART:0));
		case DEVICE_DOES_NOT_SUPPORT_EDITING_METADATA:
			return noMetadata ? 1 : 0;
		case DEVICE_REFRESH:
			{
				bool nm = noMetadata;
				IWMDMDevice3* d = WMDevice;
				d->AddRef();
				Close();
				new P4SDevice(d,nm);
				d->Release();
			}
			return 0;
		case DEVICE_SUPPORTS_VIDEO:
			return 1;
		case DEVICE_GET_ICON:
		{
			if (param2 <= 16 && param3 <= 16)
			{
				int resourceId;
				wchar_t *buffer;

				if(wcsstr(name,L"Zen")) 
					resourceId = IDR_CREATIVE_ZEN_ICON;
				else if (wcsstr(name,L"Nokia")) 
					resourceId = IDR_NOKIA_ICON;
				else 
					resourceId = 0;
			
				buffer = (wchar_t *)param4;
				if (NULL != buffer && 
					FALSE == FormatResProtocol(MAKEINTRESOURCE(resourceId), RT_RCDATA, buffer, 260))
				{
					buffer[0] = L'\0';
				}
			}
		}
		break;
	}
	return 0;
}