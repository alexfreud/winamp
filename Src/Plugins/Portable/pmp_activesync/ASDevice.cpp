#include "ASDevice.h"
#include <vector>

static void removebadchars(wchar_t *s) {
	while (s && *s)
	{
		if (*s == L'?' || *s == L'/' || *s == L'\\' || *s == L':' || *s == L'*' || *s == L'\"' || *s == L'<' || *s == L'>' || *s == L'|') 
			*s = L'_';
		s = CharNextW(s);
	}
}

Playlist::Playlist(const wchar_t * path, LPCE_FIND_DATA f) {
	_snwprintf(fn,MAX_PATH,L"%s\\%s",path,f->cFileName);
	wchar_t * ext = wcsrchr(f->cFileName,L'.');
	if(ext) *ext=0;
	lstrcpyn(name,f->cFileName,fieldlen);
}

Playlist::Playlist(const wchar_t * name0) {
	lstrcpyn(name,name0,fieldlen);
	fn[0]=0;
}

#define ASSIGNLARGE(r,h,l) {ULARGE_INTEGER li; li.HighPart = h; li.LowPart = l; r=li.QuadPart;}

Song::Song(const wchar_t * path0, LPCE_FIND_DATA f, bool video) : track(-1), video(video) {
	artist[0]=album[0]=title[0]=fn[0]=0;

	ASSIGNLARGE(size,f->nFileSizeHigh,f->nFileSizeLow);
	// first, fill in artist and album
	wchar_t *path = _wcsdup(path0);
	wchar_t *a = wcsrchr(path,L'\\');
	if(a && a-1 != path) {
		lstrcpyn(album,a+1,fieldlen);
		*a=0;
		a = wcsrchr(path,L'\\');
		if(a && a-1 != path) lstrcpyn(artist,a+1,fieldlen);
	}
	// now parse out the title
	_snwprintf(fn,MAX_PATH,L"%s\\%s",path0,f->cFileName);
	wchar_t * ext = wcsrchr(f->cFileName,L'.');
	if(ext) *ext=0;
	wchar_t * p = f->cFileName;
	if(memcmp(artist,p,wcslen(artist)*sizeof(wchar_t))==0) p+=wcslen(artist);
	while(p && *p && (*p==L'.' || *p==L'_' || *p==L'-' || *p==L' ')) p++;
	track = wcstoul(p,&p,10);
	while(p && *p && (*p==L'.' || *p==L'_' || *p==L'-' || *p==L' ')) p++;
	lstrcpyn(title,p,fieldlen);
	if(title[0]==0) lstrcpyn(title,f->cFileName,fieldlen);
	free(path);
}

Song::Song() : video(false) {}

void ASDevice::Find(const wchar_t * path) {
	wchar_t fpath[MAX_PATH] = {0};
	wsprintf(fpath,L"%s\\*",path);
	CE_FIND_DATA f = {0};
	HANDLE h = pISession->CeFindFirstFile(fpath,&f);
	if(h == INVALID_HANDLE_VALUE) return;
	do {
		if(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			wchar_t path2[MAX_PATH] = {0};
			wsprintf(path2,L"%s\\%s",path,f.cFileName);
			Find(path2);
		}
		else FoundFile(path,&f);
	} while(pISession->CeFindNextFile(h,&f));
	pISession->CeFindClose(h);
}

void ASDevice::FoundFile(const wchar_t * path, LPCE_FIND_DATA f) {
	wchar_t * ext = wcsrchr(f->cFileName,L'.');
	if(!_wcsicmp(ext,L".mp3") || !_wcsicmp(ext,L".wma"))
		playlists[0]->songs.push_back(new Song(path,f,false));
	if(!_wcsicmp(ext,L".avi") || !_wcsicmp(ext,L".wmv") || !_wcsicmp(ext,L".asf") || !_wcsicmp(ext,L".mpg") || !_wcsicmp(ext,L".mpeg"))
		playlists[0]->songs.push_back(new Song(path,f,true));
	else if(!_wcsicmp(ext,L".asx"))
		playlists.push_back(new Playlist(path,f));
}

void fixTagsForXML(wchar_t* dest, const wchar_t *cstr, const int len)
{
	int tindex = 0;
	wchar_t *temp = (wchar_t*)calloc(len, sizeof(wchar_t));
	for(int i=0;i<len && tindex<len;i++)
	{
		switch(cstr[i])
		{
			case(L'&'):
				if(tindex < len-5)
				{
					temp[tindex++] = '&';
					temp[tindex++] = 'a';
					temp[tindex++] = 'm';
					temp[tindex++] = 'p';
					temp[tindex] = ';';
				}
				else  temp[tindex] = ' '; //no room
				break;
			case(L'<'):
			{
				if(tindex < len-4)
				{
					temp[tindex++] = '&';
					temp[tindex++] = 'l';
					temp[tindex++] = 't';
					temp[tindex] = ';';
				}
				else  temp[tindex] = ' '; //no room
				break;
			}
			case(L'>'):
			{
				if(tindex < len-4)
				{
					temp[tindex++] = '&';
					temp[tindex++] = 'g';
					temp[tindex++] = 't';
					temp[tindex] = ';';
				}
				else  temp[tindex] = ' '; //no room
				break;
			}
			case(L'\"'):
			{
				if(tindex < len-4)
				{
					temp[tindex++] = '&';
					temp[tindex++] = 'q';
					temp[tindex++] = 'u';
					temp[tindex++] = 'o';
					temp[tindex++] = 't';
					temp[tindex] = ';';
				}
				else  temp[tindex] = ' '; //no room
				break;
			}
			case(L'\''):
			{
				if(tindex < len-4)
				{
					temp[tindex++] = '&';
					temp[tindex++] = 'a';
					temp[tindex++] = 'p';
					temp[tindex++] = 'o';
					temp[tindex++] = 's';
					temp[tindex] = ';';
				}
				else  temp[tindex] = ' '; //no room
				break;
			}
			default:
			{
				temp[tindex] = cstr[i];
				break;
			}
		}
		if(cstr[i] == 0) break;
		tindex++;
	}
	wcsncpy(dest, temp, len);
	free(temp);
}

void ASDevice::WritePlaylist(Playlist * pl) {
#define CePutws(x,h) pISession->CeWriteFile(h,x,(DWORD)wcslen(x)*sizeof(wchar_t),&w,NULL)
#define CePuts(x,h) pISession->CeWriteFile(h,x,(DWORD)strlen(x)*sizeof(char),&w,NULL)
	HANDLE h = pISession->CeCreateFile(pl->fn,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,TRUNCATE_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(h == INVALID_HANDLE_VALUE) return;
	std::vector<Song*> *songs = &pl->songs;
	int l=(int)songs->size();
	DWORD w;
	CePuts("<asx version=\"3.0\">\r\n",h);
	for(int j=0; j<l; j++) {
		CePuts("<entry><ref href=\"",h);
		wchar_t safe[fieldlen*2] = {0};
		fixTagsForXML(safe,songs->at(j)->fn,sizeof(safe)/sizeof(wchar_t));
		AutoChar fn(safe);
		CePuts(fn,h);
		CePuts("\"/></entry>\r\n",h);
	}
	CePuts("</asx>",h);
	pISession->CeCloseHandle(h);
#undef CePutws
#undef CePuts
}

struct mplSearch { bool operator()(Song*& a,Song*& b) { return _wcsicmp(a->fn,b->fn)<0; } };
struct mplSearch2 { bool operator()(Song*& a,Song* b) { return _wcsicmp(a->fn,b->fn)<0; } };

waServiceFactory *parserFactory;

class plread : public ifc_xmlreadercallback {
public:
	Playlist * pl;
	Playlist * mpl;
	plread(Playlist * pl,Playlist * mpl) : pl(pl),mpl(mpl) {}
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params) {
		if(!wcscmp(xmlpath,L"ASX\fENTRY\fREF")) {
			const wchar_t* path = params->getItemValue(L"HREF");
			int l= (int)mpl->songs.size();
			Song s;
			lstrcpyn(s.fn,path,MAX_PATH);
			std::vector<Song*>::iterator p = std::lower_bound(mpl->songs.begin(),mpl->songs.end(),&s,mplSearch2());
			int f = (int)(p - mpl->songs.begin());
			if(f >= 0 && f < (int)mpl->songs.size()) {
				Song * found = mpl->songs[f];
				if(!_wcsicmp(found->fn,s.fn)) pl->songs.push_back(found);
			}
		}
	}
	RECVS_DISPATCH;
};

#define CBCLASS plread
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
END_DISPATCH;
#undef CBCLASS

void ASDevice::ReadPlaylist(Playlist * pl) {
	if(!parserFactory) return;
	obj_xml * parser = (obj_xml *)parserFactory->getInterface();
	if(!parser) return;
	HANDLE h = pISession->CeCreateFile(pl->fn,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if(h == INVALID_HANDLE_VALUE) { parserFactory->releaseInterface(parser); return; }
	
	plread cb(pl,playlists[0]);
	parser->xmlreader_open();
	parser->xmlreader_registerCallback(L"ASX\fENTRY\f*",&cb);
	
	for(;;) {
		char buf[32768] = {0};
		DWORD read = 0;
		pISession->CeReadFile(h,buf,sizeof(buf),&read,NULL);
		if(read == 0) break;
		parser->xmlreader_feed(buf,read);
	}

	parserFactory->releaseInterface(parser);
	pISession->CeCloseHandle(h);
}

static void findStorageCard(IRAPISession *pISession,wchar_t *storageCard) {
	ULARGE_INTEGER fa={0},rootTotal={0},ft={0};
	pISession->CeGetDiskFreeSpaceEx(L"\\",&fa,&rootTotal,&ft);

	wchar_t *fpath = L"\\*";
	CE_FIND_DATA f;
	HANDLE h = pISession->CeFindFirstFile(fpath,&f);
	if(h == INVALID_HANDLE_VALUE) return;
	do {
		if(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY/* && wcscmp(f.cFileName,L"Storage Card")*/) {
			ULARGE_INTEGER folderTotal={0};
			wchar_t path[MAX_PATH] = L"\\";
			wcscat(path,f.cFileName);
			pISession->CeGetDiskFreeSpaceEx(path,&fa,&folderTotal,&ft);
			if(folderTotal.QuadPart > rootTotal.QuadPart) {
				rootTotal = folderTotal;
				wcsncpy(storageCard,path,MAX_PATH);
			}
		}
	} while(pISession->CeFindNextFile(h,&f));
	pISession->CeFindClose(h);
}

ASDevice::ASDevice(IRAPIDevice *pIDevice,IRAPISession *pISession) : pIDevice(pIDevice), pISession(pISession), transferQueueSize(0) {
	pIDevice->AddRef();
	pIDevice->GetDeviceInfo(&devInfo);

	pmpDeviceLoading load={this,0};
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)&load,PMP_IPC_DEVICELOADING);
	if(load.UpdateCaption) {
		wchar_t buf[200]=L"";
		wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_LOADING),devInfo.bstrName);
		load.UpdateCaption(buf,load.context);
	}
	
	//find where playlists and music are stored...
	wchar_t storageCard[MAX_PATH]=L"";
	findStorageCard(pISession,storageCard);
	wsprintf(musicFolder,L"%s\\Music",storageCard);
	wsprintf(videoFolder,L"%s\\My Documents\\My Videos",storageCard);
	wsprintf(playlistFolder,L"%s\\Playlists",storageCard);
	wcsncpy(playlistFormat,L".asx",16);
	// default values found. Fill in real values
	{
		wchar_t inifile[MAX_PATH] = {0};
		const char * iniDirectory = (const char*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORY);
		wchar_t name[256] = {0};
		lstrcpyn(name,devInfo.bstrName,256);
		removebadchars(name);
		wsprintf(inifile,L"%s\\Plugins\\ml\\ml_pmp_device_%s.ini",(wchar_t*)AutoWide(iniDirectory),name);
		wchar_t * def = _wcsdup(musicFolder);
		GetPrivateProfileString(L"pmp_activesync",L"musicfolder",def,musicFolder,MAX_PATH,inifile);
		free(def); def = _wcsdup(videoFolder);
		GetPrivateProfileString(L"pmp_activesync",L"videofolder",def,videoFolder,MAX_PATH,inifile);
		free(def); def = _wcsdup(playlistFolder);
		GetPrivateProfileString(L"pmp_activesync",L"playlistfolder",def,playlistFolder,MAX_PATH,inifile);
		free(def);
	}
	
	playlists.push_back(new Playlist(devInfo.bstrName));

	Find(musicFolder);
	Find(videoFolder);
	Find(playlistFolder);

	std::sort(playlists[0]->songs.begin(),playlists[0]->songs.end(),mplSearch());
	parserFactory = plugin.service->service_getServiceByGuid(obj_xmlGUID);

	for(unsigned int i=1; i<playlists.size(); i++)
		ReadPlaylist(playlists[i]);

	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICECONNECTED);
	transcoder = (Transcoder*)SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_GET_TRANSCODER);

	transcoder->AddAcceptableFormat(L"mp3");
	transcoder->AddAcceptableFormat(L"wma");
	transcoder->AddAcceptableFormat(L"wmv");
	transcoder->AddAcceptableFormat(L"avi");
	transcoder->AddAcceptableFormat(L"asf");
	transcoder->AddAcceptableFormat(L"mpeg");
	transcoder->AddAcceptableFormat(L"mpg");
}

ASDevice::~ASDevice()
{
	Playlist *mpl = playlists[ 0 ];
	unsigned int l = (unsigned int)mpl->songs.size();
	for ( unsigned int i = 0; i < l; i++ )
	{
		delete mpl->songs[ i ];
	}
	for ( unsigned int j = 0; j < playlists.size(); j++ )
	{
		delete playlists[ j ];
	}
	for ( int k = 0; k < devices.size(); k++ )
	{
		if ( devices[ k ] == this )
		{
			devices.erase( devices.begin() + k );
			k--;
		}
	}
	pIDevice->Release();
	pISession->Release();
	SysFreeString( devInfo.bstrName );
	SysFreeString( devInfo.bstrPlatform );
	SendMessage( plugin.hwndPortablesParent, WM_PMP_IPC, (WPARAM)transcoder, PMP_IPC_RELEASE_TRANSCODER );
}

__int64 ASDevice::getDeviceCapacityAvailable() {
	if(devInfo.dwOsVersionMajor >= 5) {
		ULARGE_INTEGER fa={0},t={0},ft={0};
		pISession->CeGetDiskFreeSpaceEx(musicFolder,&fa,&t,&ft);
		return fa.QuadPart;
	} else {
		STORE_INFORMATION s;
		pISession->CeGetStoreInformation(&s);
		return s.dwFreeSize;
	}
}

__int64 ASDevice::getDeviceCapacityTotal() {
	if(devInfo.dwOsVersionMajor >= 5) {
		ULARGE_INTEGER fa={0},t={0},ft={0};
		pISession->CeGetDiskFreeSpaceEx(musicFolder,&fa,&t,&ft);
		return t.QuadPart;
	} else {
		STORE_INFORMATION s;
		pISession->CeGetStoreInformation(&s);
		return s.dwStoreSize;
	}
}

void ASDevice::Eject() {
	ejectedDevice *e = (ejectedDevice *)calloc(1, sizeof(ejectedDevice));
	e->id = devInfo.DeviceId;
	e->marked = true;
	ejected.push_back(e);
	Close();
}

void ASDevice::Close() {
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
	delete this;
}

int ASDevice::transferTrackToDevice(const itemRecordW * track,void * callbackContext,void (*callback)(void * callbackContext, wchar_t * status),songid_t * songid,int * killswitch) {
	wchar_t ext[10]={0};
	wchar_t file[2048]={0};
	wcsncpy(file,track->filename,2048);
	{wchar_t * e = wcsrchr(file,L'.'); if(e) wcsncpy(ext,e+1,10);}
	
	bool deletefile = false;
	if(transcoder->ShouldTranscode(file)) {
		wchar_t newfile[MAX_PATH] = {0};
		transcoder->CanTranscode(file,ext);
		transcoder->GetTempFilePath(ext,newfile);
		if(transcoder->TranscodeFile(file,newfile,killswitch,callback,callbackContext)) return -1;
		wcsncpy(file,newfile,2048);
		deletefile=true;
	}

	callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING));

	bool video = !_wcsicmp(ext,L"wmv") || !_wcsicmp(ext,L"avi");

	int len = (int)(wcslen(musicFolder)+wcslen(track->artist)+wcslen(track->album)+wcslen(track->title)+100);
	wchar_t *path = (wchar_t*)calloc(len, sizeof(wchar_t));
	wchar_t *artist = _wcsdup(track->artist);
	wchar_t *album = _wcsdup(track->album);
	wchar_t *title = _wcsdup(track->title);
	removebadchars(artist);
	removebadchars(album);
	removebadchars(title);
	if(video) {
		wcsncpy(path,videoFolder,len);
		pISession->CeCreateDirectory(path,NULL);
		wsprintf(path+wcslen(path),L"\\%s - %s.%s",artist,title,ext);
	} else {
		wcsncpy(path,musicFolder,len);
		pISession->CeCreateDirectory(path,NULL);
		wcscat(path,L"\\");
		wcscat(path,artist);
		pISession->CeCreateDirectory(path,NULL);
		wcscat(path,L"\\");	
		wcscat(path,album);
		pISession->CeCreateDirectory(path,NULL);
		wsprintf(path+wcslen(path),L"\\%02d - %s.%s",track->track,title,ext);
	}
	free(artist); free(album); free(title);

	FILE *f = _wfopen(file,L"rb");
	if(!f) { callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_CANNOT_OPEN_LOCAL_FILE)); return -1; }
	HANDLE h = pISession->CeCreateFile(path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if(h == INVALID_HANDLE_VALUE) {
		callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_CANNOT_OPEN_FILE_ON_DEVICE));
		fclose(f);
		return -1;
	}
	
	fseek(f,0,2);
	int error=0;
	int size = ftell(f);
	int pc = size/100;
	fseek(f,0,0);
	int written=0,lastupdate=0;
	for(;;) {
		char buf[32768] = {0};
		int l = (int)fread(buf,1,sizeof(buf),f);
		if(!l) break;
		DWORD wl=0;
		pISession->CeWriteFile(h,buf,l,&wl,NULL);
		if(wl != l) {
			callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_ERROR_WRITING_FILE));
			error=1;
			break;
		}
		written += l;
		if(written - lastupdate > pc) {
			lastupdate = written;
			wchar_t buf[100] = {0};
			wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING_PERCENT),written/(size/100));
			callback(callbackContext,buf);
		}
	}

	fclose(f);
	pISession->CeCloseHandle(h);
	if(deletefile) _wunlink(file);
	if(!error) {
		callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_DONE));
		Song * s = new Song;
		lstrcpyn(s->fn,path,MAX_PATH);
		lstrcpyn(s->album,track->album,fieldlen);
		lstrcpyn(s->artist,track->artist,fieldlen);
		lstrcpyn(s->title,track->title,fieldlen);
		s->track = track->track;
		s->size = size;
		s->video = video;
		*songid = (songid_t)s;
	}
	free(path);
	return error?-1:0;
}

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

static bool extentionSupported(wchar_t * ext) {
  if(!ext) return false;
  bool supported=false;
  if(!_wcsicmp(ext,L".mp3") || !_wcsicmp(ext,L".wma")) supported=true;
	if(!_wcsicmp(ext,L".avi") || !_wcsicmp(ext,L".wmv")) supported=true;
	return supported;
}

int ASDevice::trackAddedToTransferQueue(const itemRecordW * track) {
	__int64 s = getTrackSizeOnDevice(track);
	if(!s) return -2;
	__int64 avail = getDeviceCapacityAvailable();
	__int64 cmp = transferQueueSize;
	cmp += s;
	if(cmp > avail) return -1;
	else {
		transferQueueSize += s;
		return 0;
	}
}

void ASDevice::trackRemovedFromTransferQueue(const itemRecordW * track) {
	transferQueueSize -= getTrackSizeOnDevice(track);
}

__int64 ASDevice::getTrackSizeOnDevice(const itemRecordW * track) {
	if(transcoder->ShouldTranscode(track->filename)) {
		int k = transcoder->CanTranscode(track->filename);
    if(k != -1 && k != 0) return k;
	}
	wchar_t * ext = wcsrchr(track->filename,L'.');
  if(!extentionSupported(ext)) return 0;
  return fileSize(track->filename);
}

void ASDevice::deleteTrack(songid_t songid) {
	Song * song = (Song*)songid;
	if(!pISession->CeDeleteFile(song->fn)) return;
	for(unsigned int i=0; i<playlists.size(); i++) 
	{
		unsigned int l = (unsigned int)playlists[i]->songs.size();
		for(int j=0; j<l; j++) 
		{
			if(playlists[i]->songs[j] == song) 
			{
				playlists[i]->songs.erase(playlists[i]->songs.begin() + j);
				j--;
				l--;
				playlists[i]->dirty=true;
			}
		}
	}
	delete song;
}

void ASDevice::commitChanges() {
	for(unsigned int i=1; i<playlists.size(); i++) if(playlists[i]->dirty) { WritePlaylist(playlists[i]); playlists[i]->dirty=false; }
}

int ASDevice::getPlaylistCount() { return (int)playlists.size(); }
void ASDevice::getPlaylistName(int playlistnumber, wchar_t * buf, int len) { lstrcpyn(buf,playlists[playlistnumber]->name,len); }
int ASDevice::getPlaylistLength(int playlistnumber) { return (int)playlists[playlistnumber]->songs.size(); }
songid_t ASDevice::getPlaylistTrack(int playlistnumber,int songnum) { return (songid_t)playlists[playlistnumber]->songs[songnum]; }

void ASDevice::setPlaylistName(int playlistnumber, const wchar_t * buf) {
	Playlist * pl = playlists[playlistnumber];
	lstrcpyn(pl->name,buf,fieldlen);
	wchar_t * oldname = _wcsdup(pl->fn);
	wchar_t * name = _wcsdup(buf);
	removebadchars(name);
	wsprintf(pl->fn,L"%s\\%s.%s",playlistFolder,name,playlistFormat);
	free(name);
	pISession->CeMoveFile(oldname,pl->fn);
	free(oldname);
}

void ASDevice::playlistSwapItems(int playlistnumber, int posA, int posB) {
	std::vector<Song*> &songs = playlists[playlistnumber]->songs;
	Song * a = songs[posA];
	Song * b = songs[posB];
	songs[posA] = b;
	songs[posB] = a;
	playlists[playlistnumber]->dirty=true;
}

#define CMPFIELDS(x) { int v = lstrcmpi(a->x,b->x); if(v) return v<0; }
#define CMPINTFIELDS(x) { int v = a->x-b->x; if(v) return v<0; }

typedef struct PlaylistItemSort {
	int use_by;
  bool operator()(Song*& a,Song*& b) {
	int x;
    for (x = 0; x < 4; x ++)
    {
      if (use_by == SORTBY_TITLE) // title -> artist -> album -> disc -> track
      {
        CMPFIELDS(title);
        use_by=SORTBY_ARTIST;
      }
      else if (use_by == SORTBY_ARTIST) // artist -> album -> disc -> track -> title
      {
        CMPFIELDS(artist);
        use_by=SORTBY_ALBUM;
      }
      else if (use_by == SORTBY_ALBUM) // album -> disc -> track -> title -> artist
      {
        CMPFIELDS(album);
        use_by=SORTBY_TRACKNUM;
      }
      else if (use_by == SORTBY_TRACKNUM) // track -> title -> artist -> album -> disc
      {
        CMPINTFIELDS(track);
        use_by=SORTBY_TITLE;     
      }
      else break; // no sort order?
    }
    return false;
	}
} PlaylistItemSort;
#undef CMPFIELDS
#undef CMPINTFIELDS

void ASDevice::sortPlaylist(int playlistnumber, int sortBy) {
	PlaylistItemSort sort;
	sort.use_by = sortBy;
	std::sort(playlists[playlistnumber]->songs.begin(),playlists[playlistnumber]->songs.end(),sort);
	playlists[playlistnumber]->dirty=true;
}

void ASDevice::addTrackToPlaylist(int playlistnumber, songid_t songid){
		playlists[playlistnumber]->songs.push_back((Song*)songid);
		playlists[playlistnumber]->dirty=true;
}

void ASDevice::removeTrackFromPlaylist(int playlistnumber, int songnum) {
	playlists[playlistnumber]->songs.erase(playlists[playlistnumber]->songs.begin() + songnum);
	playlists[playlistnumber]->dirty=true;
}

void ASDevice::deletePlaylist(int playlistnumber) {
	pISession->CeDeleteFile(playlists[playlistnumber]->fn);
	delete playlists[playlistnumber];
	playlists.erase(playlists.begin() + playlistnumber);
}

int ASDevice::newPlaylist(const wchar_t * name0) {
	pISession->CeCreateDirectory(playlistFolder,NULL);
	Playlist* pl = new Playlist(name0);
	wchar_t * name = _wcsdup(name0);
	removebadchars(name);
	wsprintf(pl->fn,L"%s\\%s.%s",playlistFolder,name,playlistFormat);
	free(name);
	pl->dirty=true;
	playlists.push_back(pl);
	return (int)playlists.size()-1;
}

void ASDevice::getTrackArtist(songid_t songid, wchar_t * buf, int len) { lstrcpyn(buf,((Song*)songid)->artist,len); }
void ASDevice::getTrackAlbum(songid_t songid, wchar_t * buf, int len) { lstrcpyn(buf,((Song*)songid)->album,len); }
void ASDevice::getTrackTitle(songid_t songid, wchar_t * buf, int len) { lstrcpyn(buf,((Song*)songid)->title,len); }
int ASDevice::getTrackTrackNum(songid_t songid) { return ((Song*)songid)->track; }
__int64 ASDevice::getTrackSize(songid_t songid) { return ((Song*)songid)->size; }
void ASDevice::getTrackExtraInfo(songid_t songid, const wchar_t * field, wchar_t * buf, int len) {
  if(!wcscmp(field,FIELD_EXTENSION)) {
    Song * s = (Song *)songid;
		wchar_t * ext = wcsrchr(s->fn,L'.');
    if(ext) { ext++; lstrcpyn(buf,ext,len); }
  }
}

int ASDevice::copyToHardDrive(songid_t song,wchar_t * path,void * callbackContext,void (*callback)(void * callbackContext, wchar_t * status),int * killswitch) {
	Song * s = (Song*)song;
	wchar_t * ext = wcsrchr(s->fn,L'.');
	if(ext && wcslen(ext)<10) wcscat(path,ext);
	callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING));
	FILE * f = _wfopen(path,L"wb");
	if(!f) {
		callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_CANNOT_OPEN_DESTINATION));
		return -1;
	}

	HANDLE h = pISession->CeCreateFile(s->fn,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if(h == INVALID_HANDLE_VALUE) {
		callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_CANNOT_OPEN_FILE_ON_DEVICE));
		fclose(f);
		return -1;
	}
	
	int error=0;
	int pc = (int)(s->size/100);
	int written=0,lastupdate=0;
	for(;;) {
		char buf[32768] = {0};
		DWORD read=0;
		pISession->CeReadFile(h,buf,sizeof(buf),&read,NULL);
		if(!read) break;
		int wr = (int)fwrite(buf,1,read,f);
		if(wr != read) {
			callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_ERROR_WRITING_FILE));
			error=1;
			break;
		}
		written += read;
		if(written - lastupdate > pc) {
			lastupdate = written;
			wchar_t buf[100] = {0};
			wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING),written/(s->size/100));
			callback(callbackContext,buf);
		}
	}

	pISession->CeCloseHandle(h);
	fclose(f);
	if(!error) {
		callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_DONE));
		return 0;
	} else return -1;
}

static BOOL CALLBACK config_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

intptr_t ASDevice::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4) {
	switch(param1) {
	case DEVICE_SET_ICON:
		{
			MLTREEIMAGE * i = (MLTREEIMAGE*)param2;
			i->hinst = plugin.hDllInstance;
			i->resourceId = IDR_ACTIVESYNC_ICON;
		}
		break;
	case DEVICE_SUPPORTED_METADATA: return 0x8f;
	case DEVICE_DOES_NOT_SUPPORT_EDITING_METADATA: return 1;
	case DEVICE_GET_PREFS_DIALOG:
		if(param3 == 0) {
			pref_tab * p = (pref_tab *)param2;
			p->hinst = WASABI_API_LNG_HINST;
			p->dlg_proc = (DLGPROC)config_dialogProc;
			p->res_id = IDD_CONFIG;
			lstrcpyn(p->title,WASABI_API_LNGSTRINGW(IDS_ADVANCED),100);
		}
		break;
	case DEVICE_SUPPORTS_VIDEO:
		return 1;
	}
	return 0;
}

static BOOL CALLBACK config_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
  static ASDevice * dev;
  switch(uMsg) {
    case WM_INITDIALOG:
      {
        prefsParam* p = (prefsParam*)lParam;
        dev = (ASDevice*)p->dev;
				p->config_tab_init(hwndDlg,p->parent);
				SetDlgItemText(hwndDlg,IDC_FOLDER_MUSIC,dev->musicFolder);
				SetDlgItemText(hwndDlg,IDC_FOLDER_VIDEO,dev->videoFolder);
				SetDlgItemText(hwndDlg,IDC_FOLDER_PLAYLIST,dev->playlistFolder);
			}
			break;
		case WM_DESTROY:
			{
				GetDlgItemText(hwndDlg,IDC_FOLDER_MUSIC,dev->musicFolder,MAX_PATH);
				GetDlgItemText(hwndDlg,IDC_FOLDER_VIDEO,dev->videoFolder,MAX_PATH);
				GetDlgItemText(hwndDlg,IDC_FOLDER_PLAYLIST,dev->playlistFolder,MAX_PATH);
				wchar_t *inifile = (wchar_t*)SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)dev,PMP_IPC_GET_INI_FILE);
#if 1
				wchar_t inifil[MAX_PATH] = {0};
				if(!inifile) {
					inifile=inifil;
					const char * iniDirectory = (const char*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORY);
					wchar_t name[256] = {0};
					lstrcpyn(name,dev->devInfo.bstrName,256);
					removebadchars(name);
					wsprintf(inifile,L"%s\\Plugins\\ml\\ml_pmp_device_%s.ini",(wchar_t*)AutoWide(iniDirectory),name);
				}
#endif
				if(inifile) {
					WritePrivateProfileString(L"pmp_activesync",L"musicfolder",dev->musicFolder,inifile);
					WritePrivateProfileString(L"pmp_activesync",L"videofolder",dev->videoFolder,inifile);
					WritePrivateProfileString(L"pmp_activesync",L"playlistfolder",dev->playlistFolder,inifile);
				}
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_RESCAN:
					config_dialogProc(hwndDlg,WM_DESTROY,0,0);
					dev->Close();
					break;
			}
			break;
	}
	return 0;
}