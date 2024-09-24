#include "NJBDevice.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"

HWND CreateDummyWindow();
extern HWND mainMessageWindow;

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

static void FillSongFromMeta(BYTE * buf,Song * song) {
  BYTE * ptr = buf;
  short count = 0;
	short type = 0;
	short NameLen = 0;
	long DataLen = 0;
	long lData;

  memcpy(&count, ptr, sizeof(short));
	ptr += sizeof(short);
  
  for(int i=0; i<count; i++)
	{
		memcpy(&type, ptr, sizeof(short));
		ptr += sizeof(short);
		memcpy(&NameLen, ptr, sizeof(short));
		ptr += sizeof(short);
		memcpy(&DataLen, ptr, sizeof(long));
		ptr += sizeof(long);

    char itemname[MAX_PATH] = {0};
		memcpy(itemname, ptr, NameLen);
    itemname[NameLen]=0;

    ptr += NameLen;

    if(type == 1) { // binary
      memcpy(&lData, ptr, min(DataLen,4));
      if (!_stricmp(itemname,LENGTH)) song->length = lData * 1000;
      else if (!_stricmp(itemname,FILESIZE)) song->size = lData;
      else if (!_stricmp(itemname,TRACKNUM)) song->track = lData;
      else if (!_stricmp(itemname,YEAR)) song->year = lData;
      else if (!_stricmp(itemname,TRACKID)) song->trackid = lData;
    } else if(type == 2) { // unicode
      if (!_stricmp(itemname,TITLE)) lstrcpyn(song->title,(WCHAR*)ptr,min((DataLen+2)/2,fieldlen));
      else if (!_stricmp(itemname,ARTIST)) lstrcpyn(song->artist,(WCHAR*)ptr,min((DataLen+2)/2,fieldlen));
      else if (!_stricmp(itemname,ALBUM)) lstrcpyn(song->album,(WCHAR*)ptr,min((DataLen+2)/2,fieldlen));
      else if (!_stricmp(itemname,GENRE)) lstrcpyn(song->genre,(WCHAR*)ptr,min((DataLen+2)/2,fieldlen));
    } else if(type == 0) { // ASCII
      if (!_stricmp(itemname,CODEC)) {
        int l=min(sizeof(song->codec)-1,DataLen);
        memcpy(song->codec,ptr,l);
        song->codec[l]=0;
      }
    }
    ptr += DataLen;
  }
}

static bool GetSong(DAPSDK_ID * item, long id, Song * song) {
  long size;
  if(m_pCTJukebox2->GetItemAttribute(id,(IUnknown*)item,0,&size,NULL) != S_OK) return false;
  BYTE * buf = (BYTE*)calloc(size,sizeof(BYTE));
  if(!buf) return false;
  if(m_pCTJukebox2->GetItemAttribute(id,(IUnknown*)item,size,&size,(IUnknown*)buf) != S_OK) { free(buf); return false; }
  FillSongFromMeta(buf,song);
  free(buf);
  return true;
}

static int song_sortfunc(const void *elem1, const void *elem2) {
  Song *a=(Song *)*(void **)elem1;
  Song *b=(Song *)*(void **)elem2;
  return a->trackid - b->trackid;
}

static Song *BinaryChopFind(int id,Playlist * mpl) {
  Song s;
  s.trackid=id;
  Song * d = &s;
  Song ** ret = (Song**)bsearch(&d,mpl->songs.GetAll(),mpl->songs.GetSize(),sizeof(void*),song_sortfunc);
  return ret?*ret:NULL;
}

static bool GetPlaylist(long id, DAPSDK_ID * item,Playlist * pl, Playlist * mpl) 
{
  pl->dirty=false;
  pl->plid = item->lID;
  lstrcpyn(pl->name,item->bstrName,fieldlen);    
	SysFreeString(item->bstrName);
  DAPSDK_ID song;
  HRESULT hr = m_pCTJukebox2->FindFirstItem(id,(IUnknown*)item,(IUnknown*)&song);
  while(hr == S_OK) {
    Song * s = BinaryChopFind(song.lID,mpl);
    if(s) pl->songs.Add(s);
    hr = m_pCTJukebox2->FindNextItem(id,(IUnknown*)item,(IUnknown*)&song);
  }
  return true;
}

NJBDevice::NJBDevice(long id) : transcoder(NULL)
{
  InitializeCriticalSection(&csRevTransfer);
  InitializeCriticalSection(&csTransfer);
  devices.Add(this);
  
  pmpDeviceLoading load;

  load.dev = this;
  load.UpdateCaption = NULL;
  SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)&load,PMP_IPC_DEVICELOADING);
  if(load.UpdateCaption) {
	load.UpdateCaption(WASABI_API_LNGSTRINGW(IDS_NJB_LOADING),load.context);
  }

  this->id = id;
  transferQueueLength = 0;
  messageWindow = CreateDummyWindow();
  m_pCTJukebox2->SetCallbackWindow2(id,(long)messageWindow);

  BYTE * ptr = NULL;
  if(m_pCTJukebox2->GetDeviceProperties(id,kDeviceSerialNumberValue,(IUnknown*)ptr) == S_OK) {
    memcpy(serial,ptr,16);
    //free(ptr);
  }

  DAPSDK_ID item,parent,root;
  Playlist * mpl = new Playlist;
  BSTR name=NULL;
  m_pCTJukebox2->GetDeviceProperties(id,kDeviceNameString,(IUnknown*)&name);
  lstrcpyn(mpl->name,name?name:L"Creative Jukebox",fieldlen);
	SysFreeString(name);

  // search for tracks...
  parent.lID = ALLTRACKSKEY;
  parent.lType = kAudioTrackType;
  HRESULT hr = m_pCTJukebox2->FindFirstItem(id,(IUnknown*)&parent,(IUnknown*)&item);
  
  while(hr == S_OK) {
    // add track
    Song * song = new Song;
    if(GetSong(&item,id,song)) {
      mpl->songs.Add(song);
      song->trackid = item.lID;
    }
    else delete song;
    hr = m_pCTJukebox2->FindNextItem(id,(IUnknown*)&parent,(IUnknown*)&item);
  }
  qsort(mpl->songs.GetAll(),mpl->songs.GetSize(),sizeof(void*),song_sortfunc); // sort the master playlist by trackid, so we can find stuff later using binary chop
  playlists.Add(mpl);

  // search for playlists...
  hr = m_pCTJukebox2->FindFirstRootItem(id,(IUnknown*)&root);
  while(hr == S_OK) {
    if(_wcsicmp(root.bstrName,L"PLAY LISTS")==0) {
      playlistRoot.bstrName = L"PLAY LISTS";
      playlistRoot.lID = root.lID;
      playlistRoot.lType = root.lType;
      HRESULT hr = m_pCTJukebox2->FindFirstParentItem(id,(IUnknown*)&root,(IUnknown*)&parent);
      while(hr == S_OK) {
        Playlist * pl = new Playlist;
        if(GetPlaylist(id,&parent,pl,mpl)) playlists.Add(pl);
        else delete pl;
        hr = m_pCTJukebox2->FindNextParentItem(id,(IUnknown*)&root,(IUnknown*)&parent);
      }
    }
		SysFreeString(root.bstrName);
    hr = m_pCTJukebox2->FindNextRootItem(id,(IUnknown*)&root);
  }
  
  SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICECONNECTED);
  //transcoder = NULL;
  transcoder = (Transcoder*)SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_GET_TRANSCODER);
  if(transcoder) {
    transcoder->AddAcceptableFormat(L"mp3");
    //transcoder->AddAcceptableFormat(L"wav");
    transcoder->AddAcceptableFormat(L"wma");
  }
}

NJBDevice::~NJBDevice()
{
	 m_pCTJukebox2->SetCallbackWindow2(id, (long)mainMessageWindow);
  DestroyWindow(messageWindow);
  messageWindow = NULL;
  Playlist * mpl = (Playlist *)playlists.Get(0);
  int l = mpl->songs.GetSize();
  for(int i=0; i<l; i++) delete (Song *)mpl->songs.Get(i);
  l = playlists.GetSize();
  for(int i=0; i<l; i++) delete (Playlist *)playlists.Get(i);

  for(int i=0; i<devices.GetSize(); i++) if(devices.Get(i) == this) { devices.Del(i); break; }
  DeleteCriticalSection(&csRevTransfer);
  DeleteCriticalSection(&csTransfer);
  if(transcoder) SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)transcoder,PMP_IPC_RELEASE_TRANSCODER);
}

__int64 NJBDevice::getDeviceCapacityAvailable() {
  DAPSDK_STORAGE_INFO s;
  ULARGE_INTEGER ret;
  m_pCTJukebox2->GetDeviceProperties(id,kStorageInfoStruct,(IUnknown*)&s);
  ret.LowPart = s.freeL;
  ret.HighPart = s.freeH;
  return ret.QuadPart;
}

__int64 NJBDevice::getDeviceCapacityTotal() {
  DAPSDK_STORAGE_INFO s;
  ULARGE_INTEGER ret;
  m_pCTJukebox2->GetDeviceProperties(id,kStorageInfoStruct,(IUnknown*)&s);
  ret.LowPart = s.totalL;
  ret.HighPart = s.totalH;
  return ret.QuadPart;
}

void NJBDevice::Eject() {
  Close();
}

void NJBDevice::Close() 
{
  commitChanges();
  SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
	if(devices.GetSize() == 1)
		m_pCTJukebox2->SetCallbackWindow2(0,(long)mainMessageWindow);
  delete this;
}

static BYTE * setAttrib(BYTE * ptr,short type, char * name, BYTE * data, int datalen) {
  short namelen = (short)strlen(name);
  memcpy(ptr,&type,2);      ptr += 2;
  memcpy(ptr,&namelen,2);   ptr += 2;
  memcpy(ptr,&datalen,4);   ptr += 4;
  memcpy(ptr,name,namelen); ptr += namelen;
  memcpy(ptr,data,datalen); ptr += datalen;
  return ptr;
}

static BYTE * makeMetaFromItemRecord(itemRecordW * item, wchar_t * file, long * size) {
  char codec[4]="WAV";
  wchar_t * ext = wcsrchr(file,L'.') + 1;
  if(!_wcsicmp(ext,L"mp3")) strncpy(codec,"MP3",3);
  else if(!_wcsicmp(ext,L"wma")) strncpy(codec,"WMA",3);

  if(!item->album) item->album = _wcsdup(L"");
  if(!item->artist) item->artist = _wcsdup(L"");
  if(!item->title) item->title = _wcsdup(L"");

  *size = (long)(2/*count*/+2/*type*/+6/*namelen+datalen*/+strlen(TITLE)+2*wcslen(item->title)
		+2+6+strlen(ALBUM)+2*wcslen(item->album)
		+2+6+strlen(ARTIST)+2*wcslen(item->artist)
		+2+6+strlen(CODEC)+strlen(codec)    
		+2+6+strlen(FILESIZE)+sizeof(long)
		+2+6+strlen(LENGTH)+sizeof(long));
  
  int count = 6;
  if (item->year > 0 ){
		*size+=2+6+(long)strlen(YEAR)+sizeof(short);
    count++;
  }
  if (item->genre) {
		*size+=(long)(2+6+strlen(GENRE)+2*wcslen(item->genre));
    count++;
  }
  if (item->track>0) {
    *size+= (long)(2+6+strlen(TRACKNUM)+sizeof(short));
    count++;
  }
  BYTE *buf = (BYTE*)calloc(1,*size);
  BYTE *ptr = buf;
	memcpy(ptr, &count, sizeof(short));
	ptr += sizeof(short);
  
  ptr = setAttrib(ptr,2,TITLE,(BYTE*)((wchar_t*)(item->title)),(int)wcslen(item->title)*2);
  ptr = setAttrib(ptr,2,ARTIST,(BYTE*)((wchar_t*)(item->artist)), (int)wcslen(item->artist)*2);
  ptr = setAttrib(ptr,2,ALBUM,(BYTE*)((wchar_t*)(item->album)), (int)wcslen(item->album)*2);
  if(item->genre) ptr = setAttrib(ptr,2,GENRE,(BYTE*)((wchar_t*)(item->genre)), (int)wcslen(item->genre)*2);
  short v = item->track;
  if(item->track>0) ptr = setAttrib(ptr,1,TRACKNUM,(BYTE*)&v,2);
  v = item->year;
  if(item->year>0) ptr = setAttrib(ptr,1,YEAR,(BYTE*)&v,2);
  ptr = setAttrib(ptr,0,CODEC,(BYTE*)codec,(int)strlen(codec));
  ptr = setAttrib(ptr,1,LENGTH,(BYTE*)&item->length,4);
  __int64 filesize = fileSize(file);
  ptr = setAttrib(ptr,1,FILESIZE,(BYTE*)&filesize,4);

  return buf;
}

BOOL NJBDevice::WindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
  case WM_USER: // start add item
    if(transferItem.status == 0) {
      long size;
      BYTE * buf = makeMetaFromItemRecord(const_cast<itemRecordW *>(transferItem.track),transferItem.file,&size);
      HRESULT hr = m_pCTJukebox2->AddItem(id,kAudioTrackType,SysAllocString(transferItem.file),size,(IUnknown*)buf);
      transferItem.meta = buf;
      if(hr != S_OK) this->WindowMessage(hwnd,WM_DAPSDK_ADDITEM_COMPLETE,-1,0);
    }
    break;
  case WM_DAPSDK_ADDITEM_PROGRESS:
    {
      wchar_t buf[100] = {0};
	  wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING_PERCENT),(int)wParam);
      transferItem.callback(transferItem.callbackContext,buf);
    }
    break;
  case WM_DAPSDK_ADDITEM_COMPLETE:
    if(wParam == 0) {
	  transferItem.callback(transferItem.callbackContext,WASABI_API_LNGSTRINGW(IDS_DONE));
      Song * song = new Song;
      song->trackid = (int)lParam;
      FillSongFromMeta(transferItem.meta,song);
      song->track = transferItem.track->track;
      song->year = transferItem.track->year;
      *transferItem.songid = (songid_t)song;
    }
    else {
	  transferItem.callback(transferItem.callbackContext,WASABI_API_LNGSTRINGW(IDS_ERROR));
    }
    transferItem.status = (wParam==0?1:2);
    free(transferItem.meta);
    break;
  case WM_USER+1: // start get item
    if(revTransferItem.status == 0) {
      Song * song = (Song*)*revTransferItem.songid;
      DAPSDK_ID item = {song->trackid,kAudioTrackType,song->title};
      // memory allocated by SysAllocString is freed by COM (why, i don't know)
      HRESULT hr = m_pCTJukebox2->GetItem(id,SysAllocString(revTransferItem.file),(IUnknown*)&item);
      if(hr != S_OK) WindowMessage(hwnd,WM_DAPSDK_GETITEM_COMPLETE,-1,0);
    }
    break;
  case WM_DAPSDK_GETITEM_PROGRESS:
    {
      wchar_t buf[100] = {0};
	  wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING_PERCENT),(int)wParam);
      revTransferItem.callback(revTransferItem.callbackContext,buf);
    }
    break;
  case WM_DAPSDK_GETITEM_COMPLETE:
	  revTransferItem.callback(revTransferItem.callbackContext,
							   WASABI_API_LNGSTRINGW((wParam==0?IDS_DONE:IDS_ERROR)));
    revTransferItem.status = (wParam==0?1:2);
    break;
  }
  return 0;
}
//p75
int NJBDevice::transferTrackToDevice(const itemRecordW * track,void * callbackContext,void (*callback)(void * callbackContext, wchar_t * status),songid_t * songid,int * killswitch) {
  wchar_t file[2048] = {0};
  wcsncpy(file,track->filename,2048);
  bool deletefile = false;
  if(transcoder) if(transcoder->ShouldTranscode(file)) {
    wchar_t newfile[MAX_PATH] = {0};
    wchar_t ext[10] = {0};
    transcoder->CanTranscode(file,ext);
    transcoder->GetTempFilePath(ext,newfile);
    if(transcoder->TranscodeFile(file,newfile,killswitch,callback,callbackContext)) return -1;
    wcsncpy(file,newfile,2048);
    deletefile=true;
  }
  EnterCriticalSection(&csTransfer);
  callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING));
  transferItem.file = file;
  transferItem.callback = callback;
  transferItem.callbackContext = callbackContext;
  transferItem.status=0; // in progress
  transferItem.killswitch = killswitch;
  transferItem.songid = songid;
  transferItem.track = track;

  //now start the transfer
  PostMessage(messageWindow,WM_USER,0,0);

  while(transferItem.status==0) Sleep(10); // wait for transfer

  // transfer completed
  int ret = transferItem.status==1?0:-1;

  LeaveCriticalSection(&csTransfer);
  if(deletefile) _wunlink(file);
  return ret;
}

int NJBDevice::trackAddedToTransferQueue(const itemRecordW * track) {
  __int64 l;
  if(transcoder && transcoder->ShouldTranscode(track->filename)) {
    int k = transcoder->CanTranscode(track->filename);
    if(k == -1) return -2;
    if(k == 0) l = fileSize(track->filename);
    else l = (__int64)k;
  } else {
    wchar_t * ext = wcsrchr(track->filename,L'.');
    if(!ext) return -2;
    if(_wcsicmp(ext,L".mp3") && _wcsicmp(ext,L".wma") && _wcsicmp(ext,L".wav")) return -2;
    l = fileSize(track->filename);
  }
  if(transferQueueLength + l + 1000000 > getDeviceCapacityAvailable())
    return -1;
  else {
    transferQueueLength += l;
    return 0;
  }
}

void NJBDevice::trackRemovedFromTransferQueue(const itemRecordW * track) {
  __int64 l = (__int64)fileSize(track->filename);
  if(transcoder && transcoder->ShouldTranscode(track->filename)) {
    int k = transcoder->CanTranscode(track->filename);
    if(k != -1 && k != 0) l = (__int64)k;
  }
  transferQueueLength -= l;
}

__int64 NJBDevice::getTrackSizeOnDevice(const itemRecordW * track) {
  if(transcoder && transcoder->ShouldTranscode(track->filename)) {
    int k = transcoder->CanTranscode(track->filename);
    if(k != -1 && k != 0) return k;
  }
  wchar_t * ext = wcsrchr(track->filename,L'.');
  if(!ext) return 0;
  if(_wcsicmp(ext,L".mp3") && _wcsicmp(ext,L".wma") && _wcsicmp(ext,L".wav")) return 0;
  return fileSize(track->filename);
}

void NJBDevice::deleteTrack(songid_t songid) {
  Song * s = (Song*)songid;
  for(int i=0; i<playlists.GetSize(); i++) {
    Playlist * pl = (Playlist *)playlists.Get(i);
    int l = pl->songs.GetSize();
    while(l-- > 0) if(pl->songs.Get(l) == (void*)s) { pl->songs.Del(l); if(i>0) pl->dirty=true; }
  }
  DAPSDK_ID item = {s->trackid,kAudioTrackType,s->title};
  m_pCTJukebox2->DeleteItem(id,(IUnknown*)&item);
  delete s;
}

void NJBDevice::commitChanges() {
  for(int i=1; i<playlists.GetSize(); i++) {
    Playlist * pl = (Playlist *)playlists.Get(i);
    if(pl->dirty) {
      pl->dirty = false;
      DAPSDK_ID parentold = {pl->plid,kPlaylistType,pl->name};
      m_pCTJukebox2->DeleteParentItem(id,(IUnknown*)&parentold);
      DAPSDK_ID parent = {0,kPlaylistType,_wcsdup(pl->name)};
      m_pCTJukebox2->AddParentItem(id,(IUnknown*)&playlistRoot,(IUnknown*)&parent);
      pl->plid = parent.lID;
      long l = pl->songs.GetSize();
      DAPSDK_ID * list = (DAPSDK_ID *)calloc(sizeof(DAPSDK_ID),l);
      for(int j=0; j<l; j++) {
        Song * s = (Song*)pl->songs.Get(j);
        if(s) {
          list[j].lID = s->trackid;
          list[j].lType = kAudioTrackType;
          list[j].bstrName = SysAllocString(s->title);
        }
      }
      m_pCTJukebox2->AddItemsToParentItem(id,(IUnknown*)&parent,l,(IUnknown*)list);
      free(list);
    }
  }
}

int NJBDevice::getPlaylistCount() {
  return playlists.GetSize();
}

void NJBDevice::getPlaylistName(int playlistnumber, wchar_t * buf, int len) {
  Playlist *  pl = (Playlist *)playlists.Get(playlistnumber);
  lstrcpyn(buf,pl->name,len);
}

int NJBDevice::getPlaylistLength(int playlistnumber) {
  Playlist *  pl = (Playlist *)playlists.Get(playlistnumber);
  return pl->songs.GetSize();
}

songid_t NJBDevice::getPlaylistTrack(int playlistnumber,int songnum) {
  Playlist *  pl = (Playlist *)playlists.Get(playlistnumber);
  return (songid_t) pl->songs.Get(songnum);
}

void NJBDevice::setPlaylistName(int playlistnumber, const wchar_t * buf) {
  Playlist * pl = (Playlist *)playlists.Get(playlistnumber);
  lstrcpyn(pl->name,buf,fieldlen);
  DAPSDK_ID item = {pl->plid,kPlaylistType,pl->name};
	BSTR name = SysAllocString(buf);
  m_pCTJukebox2->RenameParentItem(id,(IUnknown*)&item, name);
	SysFreeString(name);
}

void NJBDevice::playlistSwapItems(int playlistnumber, int posA, int posB) {
  Playlist * pl = (Playlist *)playlists.Get(playlistnumber);
  void * a = pl->songs.Get(posA);
  void * b = pl->songs.Get(posB);
  pl->songs.Set(posA,b);
  pl->songs.Set(posB,a);
  pl->dirty = true;
}

static int sortby;
#define RETIFNZ(v) if ((v)!=0) return v;
#define STRCMP_NULLOK _wcsicmp

static int sortFunc(const void *elem1, const void *elem2)
{
  int use_by = sortby;
  Song *a=(Song *)*(void **)elem1;
  Song *b=(Song *)*(void **)elem2;

  // this might be too slow, but it'd be nice
  int x;
  for (x = 0; x < 5; x ++)
  {
    if (use_by == SORTBY_TITLE) // title -> artist -> album -> disc -> track
    {
      int v=STRCMP_NULLOK(a->title,b->title);
      RETIFNZ(v)
      use_by=SORTBY_ARTIST;
    }
    else if (use_by == SORTBY_ARTIST) // artist -> album -> disc -> track -> title
    {
      int v=STRCMP_NULLOK(a->artist,b->artist);
      RETIFNZ(v)
      use_by=SORTBY_ALBUM;
    }
    else if (use_by == SORTBY_ALBUM) // album -> disc -> track -> title -> artist
    {
      int v=STRCMP_NULLOK(a->album,b->album);
      RETIFNZ(v)
      use_by=SORTBY_DISCNUM;
    }
    else if (use_by == SORTBY_TRACKNUM) // track -> title -> artist -> album -> disc
    {
      int v1=a->track;
      int v2=b->track;
      if (v1<0)v1=0;
      if (v2<0)v2=0;
      RETIFNZ(v1-v2)
      use_by=SORTBY_TITLE;     
    }
    else if (use_by == SORTBY_GENRE) // genre -> artist -> album -> disc -> track
    {
      int v=STRCMP_NULLOK(a->genre,b->genre);
      RETIFNZ(v)
      use_by=SORTBY_ARTIST;
    }
    else break; // no sort order?
  } 

  return 0;
}
#undef RETIFNZ
#undef STRCMP_NULLOK


void NJBDevice::sortPlaylist(int playlistnumber, int sortBy) {
  sortby = sortBy;
  Playlist * pl = (Playlist *)playlists.Get(playlistnumber);
  qsort(pl->songs.GetAll(),pl->songs.GetSize(),sizeof(void*),sortFunc);
  pl->dirty=true;
}

void NJBDevice::addTrackToPlaylist(int playlistnumber, songid_t songid) {
		Playlist * pl = (Playlist *)playlists.Get(playlistnumber);
		pl->songs.Add((void*)songid);
		pl->dirty = true;
}

void NJBDevice::removeTrackFromPlaylist(int playlistnumber, int songnum) {
  Playlist * pl = (Playlist *)playlists.Get(playlistnumber);
  pl->songs.Del(songnum);
  pl->dirty = true;
}

void NJBDevice::deletePlaylist(int playlistnumber) {
  Playlist * pl = (Playlist *)playlists.Get(playlistnumber);
  DAPSDK_ID parent = {pl->plid,kPlaylistType,pl->name};
  m_pCTJukebox2->DeleteParentItem(id,(IUnknown*)&parent);
  playlists.Del(playlistnumber);
  delete pl;
}

int NJBDevice::newPlaylist(const wchar_t * name) {
  Playlist * pl = new Playlist;
  pl->dirty = false;
  lstrcpyn(pl->name,name,fieldlen);
  DAPSDK_ID parent = {0,kPlaylistType,pl->name};
  m_pCTJukebox2->AddParentItem(id,(IUnknown*)&playlistRoot,(IUnknown*)&parent);
  pl->plid = parent.lID;
  playlists.Add(pl);
  return playlists.GetSize() - 1;
}

void NJBDevice::getTrackArtist(songid_t songid, wchar_t * buf, int len) {lstrcpyn(buf,((Song*)songid)->artist,len);}
void NJBDevice::getTrackAlbum(songid_t songid, wchar_t * buf, int len) {lstrcpyn(buf,((Song*)songid)->album,len);}
void NJBDevice::getTrackTitle(songid_t songid, wchar_t * buf, int len) {lstrcpyn(buf,((Song*)songid)->title,len);}
void NJBDevice::getTrackGenre(songid_t songid, wchar_t * buf, int len) {lstrcpyn(buf,((Song*)songid)->genre,len);}
int NJBDevice::getTrackTrackNum(songid_t songid) {return ((Song*)songid)->track;}
int NJBDevice::getTrackDiscNum(songid_t songid) {return -1;}
int NJBDevice::getTrackYear(songid_t songid) {return ((Song*)songid)->year;}
__int64 NJBDevice::getTrackSize(songid_t songid) {return ((Song*)songid)->size;}
int NJBDevice::getTrackLength(songid_t songid) {return ((Song*)songid)->length;}
int NJBDevice::getTrackBitrate(songid_t songid) {return -1;}
int NJBDevice::getTrackPlayCount(songid_t songid) {return -1;}
int NJBDevice::getTrackRating(songid_t songid) {return -1;}
__time64_t NJBDevice::getTrackLastPlayed(songid_t songid) {return -1;}
__time64_t NJBDevice::getTrackLastUpdated(songid_t songid) {return -1;}

void NJBDevice::getTrackExtraInfo(songid_t songid, const wchar_t * field, wchar_t * buf, int len) {
  if(!wcscmp(field,L"ext")) {
    Song * s = (Song *)songid;
    lstrcpyn(buf,(wchar_t*)AutoWide(s->codec),len);
    wchar_t * p = buf;
    while(p && *p) { *p=towlower(*p); p++; }
  }
}

void NJBDevice::setTrackArtist(songid_t songid, const wchar_t * value) {
  Song * s = (Song *)songid;
  lstrcpyn(s->artist,value,fieldlen);
  DAPSDK_ID item = {s->trackid,kAudioTrackType,s->title};
  m_pCTJukebox2->SetItemAttribute(id,(IUnknown*)&item,L"ARTIST",2,(long)wcslen(value)*2+2,(IUnknown*)value);
}

void NJBDevice::setTrackAlbum(songid_t songid, const wchar_t * value) {
  Song * s = (Song *)songid;
  lstrcpyn(s->album,value,fieldlen);
  DAPSDK_ID item = {s->trackid,kAudioTrackType,s->title};
  m_pCTJukebox2->SetItemAttribute(id,(IUnknown*)&item,L"ALBUM",2, (long)wcslen(value)*2+2,(IUnknown*)value);
}


void NJBDevice::setTrackTitle(songid_t songid, const wchar_t * value) {
  Song * s = (Song *)songid;
  lstrcpyn(s->title,value,fieldlen);
  DAPSDK_ID item = {s->trackid,kAudioTrackType,s->title};
  m_pCTJukebox2->SetItemAttribute(id,(IUnknown*)&item,L"TITLE",2, (long)wcslen(value)*2+2,(IUnknown*)value);
}


void NJBDevice::setTrackGenre(songid_t songid, const wchar_t * value) {
  Song * s = (Song *)songid;
  lstrcpyn(s->genre,value,fieldlen);
  DAPSDK_ID item = {s->trackid,kAudioTrackType,s->title};
  m_pCTJukebox2->SetItemAttribute(id,(IUnknown*)&item,L"GENRE",2, (long)wcslen(value)*2+2,(IUnknown*)value);
}


void NJBDevice::setTrackTrackNum(songid_t songid, int value) {
  Song * s = (Song *)songid;
  s->track = value;
  DAPSDK_ID item = {s->trackid,kAudioTrackType,s->title};
  m_pCTJukebox2->SetItemAttribute(id,(IUnknown*)&item,L"TRACK NUM",1,sizeof(short),(IUnknown*)&value);
}

void NJBDevice::setTrackYear(songid_t songid, int value) {
  Song * s = (Song *)songid;
  s->year = value;
  DAPSDK_ID item = {s->trackid,kAudioTrackType,s->title};
  m_pCTJukebox2->SetItemAttribute(id,(IUnknown*)&item,L"YEAR",1,sizeof(short),(IUnknown*)&value);
}

int NJBDevice::copyToHardDrive(songid_t s,wchar_t * path,void * callbackContext,void (*callback)(void * callbackContext, wchar_t * status),int * killswitch) {
  EnterCriticalSection(&csRevTransfer);

  callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING));
  Song * song = (Song*)s;
  wcscat(path,L".");
  wcscat(path,AutoWide(song->codec));

  wchar_t *p = wcsrchr(path,L'.');
  while(p && *p) { *p = towlower(*p); p++; }

  revTransferItem.callback = callback;
  revTransferItem.callbackContext = callbackContext;
  revTransferItem.killswitch = killswitch;
  revTransferItem.songid = &s;
  revTransferItem.file = path;
  revTransferItem.status = 0;

  PostMessage(messageWindow,WM_USER+1,0,0);
  
  while(revTransferItem.status==0) Sleep(10); // wait for transfer
  
  int ret = revTransferItem.status==1?0:-1;
  
  LeaveCriticalSection(&csRevTransfer);
  return ret;
}

intptr_t NJBDevice::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4) {
  switch(param1) {
    case DEVICE_SET_ICON:
      {
        MLTREEIMAGE * i = (MLTREEIMAGE*)param2;
        i->hinst = plugin.hDllInstance;
        i->resourceId = IDR_ZEN_ICON;
      }
      break;
    case DEVICE_SUPPORTED_METADATA: 
      return 0x3ef;
  }
  return 0;
}