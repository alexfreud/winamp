#include "iPodDevice.h"
//#include <assert.h>
#include "..\..\General\gen_ml/itemlist.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../Winamp/wa_ipc.h"
#include <math.h>
#include "../Agave/Language/api_language.h"
#include "api.h"
#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/bltcanvas.h>
#include "iPodSD.h"
#include <strsafe.h>
#include <shlwapi.h>
//#include "../nu/combobox.h"
// needed to query for replaygain stuff

static const GUID playbackConfigGroupGUID = 
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

extern PMPDevicePlugin plugin;
extern time_t mactime_to_wintime (const unsigned long mactime);
extern unsigned long wintime_to_mactime (const __time64_t time);
extern wchar_t* UTF8_to_UTF16(char *str);
extern BOOL EjectVolume(TCHAR cDriveLetter);

extern std::vector<iPodDevice*> iPods;

static __int64 fileSize(const wchar_t * filename);

iPodDevice::iPodDevice(char deviceDrive)
{
	fwid=0;
	info=0;
	artdb=0;
	gapscanner=0;
	transcoder=0;
	image16 = 0;
	image160 = 0;
	drive = deviceDrive;
	driveW = ((int)(drive - 'A')) + L'A';
	transferQueueLength=0;
	db=NULL;
	srand(GetTickCount());
	dirnum = rand() % 20;
	

	{
		wchar_t artwork[] = {driveW,L":\\iPod_Control\\Artwork"};
		_wmkdir(artwork);
		wchar_t device[] = {driveW,L":\\iPod_Control\\Device"};
		_wmkdir(device);
	}

	iPods.push_back(this);

	pmpDeviceLoading load;
	load.dev = this;
	load.UpdateCaption = NULL;
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)&load,PMP_IPC_DEVICELOADING);

	if(load.UpdateCaption) 
	{
		load.UpdateCaption(WASABI_API_LNGSTRINGW(IDS_IPOD_LOADING),load.context);
	}

	info = GetiPodInfo(driveW);
	
	// Get the artwork formats that are supported
	if(info && info->numberOfSupportedFormats >0) 
	{
		// formats are already available, read from the sysinfo xml
		// just use them
		for (int i=0; i<info->numberOfSupportedFormats; i++)
		{
			thumbs.push_back(&info->supportedArtworkFormats[i]);
		}
	}
	else 
	{
		// revert to the static list of supported artwork formats
		const ArtworkFormat* art = GetArtworkFormats(info);
		if(art) for(int i=0; art[i].type != THUMB_INVALID; i++)
		{
			if(art[i].type >= THUMB_COVER_SMALL && art[i].type <= THUMB_COVER_LARGE)
				thumbs.push_back(&art[i]);
		}
	}

	if(!info || parseiTunesDB(thumbs.size()!=0) < 0) 
	{
		//iPods.eraseObject(this);
		auto it = std::find(iPods.begin(), iPods.end(), this);
		if (it != iPods.end())
		{
			iPods.erase(it);
		}

		SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
		delete this;
		return;
	}

	image16 = info->image16;
	image160 = info->image160;

	db->mhsdplaylists->mhlp->SortPlaylists();

	int n = db->mhsdplaylists->mhlp->GetChildrenCount();
	for(int i=0; i<n; i++)
		playlists.Add(db->mhsdplaylists->mhlp->GetPlaylist(i));


	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICECONNECTED);

	transcoder = (Transcoder*)SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)this,PMP_IPC_GET_TRANSCODER);
	if(transcoder)
	{
		transcoder->AddAcceptableFormat(mmioFOURCC('M','4','A',' '));
		transcoder->AddAcceptableFormat(L"mp3");
		//transcoder->AddAcceptableFormat(L"wav");
		transcoder->AddAcceptableFormat(L"m4v");
		transcoder->AddAcceptableFormat(L"m4b");
		transcoder->AddAcceptableFormat(L"aa\0\0");
		transcoder->AddAcceptableFormat(L"mp4");
	}
	if (info->fwid)
	{
		fwid = (uint8_t *)malloc(8);
		memcpy(fwid, info->fwid, 8);
	}
}

iPodDevice::~iPodDevice() {
	if(gapscanner) SendMessage(gapscanner,WM_CLOSE,0,0);
	if(db) delete db; db=NULL;

	char lockPath[] = {drive, ":\\iPod_Control\\iTunes\\iTunesLock"};
	_unlink(lockPath);
	if(transcoder) SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)transcoder,PMP_IPC_RELEASE_TRANSCODER);
	delete info;
	info=0;
	free(fwid);
}

static unsigned char * readFile(char * path, int &len) {
	FILE * f = fopen(path,"rb");
	if(!f) return 0;
	fseek(f,0,2); //seek to end
	int l = ftell(f); //length of file
	unsigned char * data = (unsigned char *)malloc(l);
	if(!data)
	{
		fclose(f);
		return 0;
	}
	fseek(f,0,0);
	if(fread(data,1,l,f) != l) { fclose(f); free(data); return 0; }
	fclose(f);
	len = l;
	return data;
}

static unsigned char * readFile(char * path) {
	int l=0;
	return readFile(path,l);
}

static HANDLE iTunesLock(char drive) { // returns false for unable to aquire lock.
	char lockPath[] = {drive, ":\\iPod_Control\\iTunes\\iTunesLock"};
	HANDLE h=CreateFileA(lockPath,GENERIC_READ,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(h == INVALID_HANDLE_VALUE) return h;
	while(!LockFile(h,0,0,0,0)) Sleep(50);
	return h;
}

static void iTunesUnlock(HANDLE h) {
	UnlockFile(h,0,0,0,0);
	CloseHandle(h);
}

int iPodDevice::parseiTunesDB(bool parseArt) {
	HANDLE hLock = iTunesLock(drive);
	if(hLock == INVALID_HANDLE_VALUE) return -1;
	char dbPath[] = "x:\\iPod_Control\\iTunes\\iTunesDB";
	dbPath[0]=drive;
	unsigned char * data = readFile(dbPath);
	if(data==0) {
		iTunesUnlock(hLock);
		return -1;
	}
	db = new iPod_mhbd;
	int ret = db->parse(data);
	free(data);
	bool changed=false;
	char playcounts[] = "x:\\iPod_Control\\iTunes\\Play Counts";
	playcounts[0]=drive;
	data = readFile(playcounts);
	if(data) {
		iPod_mhdp * mhdp = new iPod_mhdp;
		int l = db->mhsdsongs->mhlt->GetChildrenCount();
		if(mhdp->parse(data) == l) {
			changed=true;
			for(int i=0; i<l; i++) {
				PCEntry p = mhdp->GetPlayCount(i);
				iPod_mhit * mhit = db->mhsdsongs->mhlt->GetTrack(i);
				if(!mhit) continue;
				mhit->bookmarktime = p.bookmarktime;
				mhit->lastplayedtime = p.lastplayedtime;
				mhit->stars = (unsigned char)p.stars;
				mhit->playcount = p.playcount;
				mhit->skipcount = p.skipcount;
				mhit->skippedtime = p.skippedtime;
			}
		}
		delete mhdp;
		free(data);
	}
	_unlink(playcounts);
	db->mhsdplaylists->mhlp->GetPlaylist(0)->mhit = &db->mhsdsongs->mhlt->mhit;
	char otg[] = "x:\\iPod_Control\\iTunes\\OTGPlaylistInfo";
	otg[0]=drive;
	data = readFile(otg);
	if(data) {
		iPod_mhpo * mhpo = new iPod_mhpo;
		mhpo->parse(data);
		mhpo->CreatePlaylistFromOTG(db,L"On The Go");
		changed=true;
		delete mhpo;
		free(data);
	}
	_unlink(otg);
	iTunesUnlock(hLock);
	if(changed) writeiTunesDB();

	if(parseArt) {
		char dbPath[] = "x:\\iPod_Control\\Artwork\\ArtworkDB";
		dbPath[0]=drive;
		int l=0;
		unsigned char * data = readFile(dbPath,l);
		bool createNew=false;
		if(data) {
			artdb = new ArtDB();
			int r = artdb->parse(data,l,driveW);
			if(r<0) {
				delete artdb;
				artdb=NULL;
			}
			free(data);
		} else createNew=true;

		if(createNew) {
			char dir[] = {drive,":\\iPod_Control\\Artwork"};
			CreateDirectoryA(dir,NULL);
			artdb = new ArtDB();
			artdb->makeEmptyDB(driveW);
		}
	}

	return ret;
}

extern bool ParseSysInfoXML(wchar_t drive_letter, char * xml, int xmllen);

static unsigned char *GetFwId(wchar_t drive, unsigned char *fwid)
{
	char xml[65536] = {0};
	if(!ParseSysInfoXML(drive, xml, sizeof(xml)/sizeof(char))) return NULL;
	char *p = strstr(xml,"<key>FireWireGUID</key>");
	if(!p) return 0;
	p = strstr(p,"<string>");
	if(!p) return 0;
	p += strlen("<string>");
	for(int i=0; i<8 && *p; i++) {
		char num[3]={0,0,0};
		num[0] = *(p++);
		num[1] = *(p++);
		fwid[i] = (uint8_t)strtoul(num,NULL,16); 
	}
	return fwid;
}

int iPodDevice::writeiTunesDB() 
{
	char dbPath[] = "x:\\iPod_Control\\iTunes\\iTunesDB"; dbPath[0]=drive;
	char dbPathOld[] = "x:\\iPod_Control\\iTunes\\iTunesDB.old_mlpmp"; dbPathOld[0]=drive;
	char dbPathNew[] = "x:\\iPod_Control\\iTunes\\iTunesDB.new_mlpmp"; dbPathNew[0]=drive;
	if(!db) return -1;
	HANDLE hLock = iTunesLock(drive);
	if(hLock == INVALID_HANDLE_VALUE) return -1;
	uint32_t allocate = (uint32_t)fileSize(AutoWide(dbPath));
	int incr = 10000000;
	bool done=false;
	int i=0;
	int ret=0;
	unsigned char * data;

	while(!done) 
	{
		if(i++ > 10)
		{ 
			iTunesUnlock(hLock);
			return -1;
		}
		allocate += incr;
		data = (unsigned char*)malloc(allocate);
		if(!data) return -1; //what else can we do?

		// TODO: i'd like to cut this but it seems to still be causing problems to parse it from XML
		unsigned char fwid[8]={0};
		GetFwId(driveW, fwid);
#ifdef _DEBUG
		if (memcmp(fwid, this->fwid, 8) || memcmp(fwid, info->fwid, 8))
		{
			DebugBreak();
		}
#endif

		int len = db->write(data,allocate, fwid);
		if(len > 0) {
			_unlink(dbPathOld);
			_unlink(dbPathNew);
			FILE * f = fopen(dbPathNew,"wb");
			if(!f) { 
				iTunesUnlock(hLock);
				return -1;
			}
			fwrite(data,1,len,f);
			fclose(f);
			rename(dbPath,dbPathOld);
			_unlink(dbPath);
			rename(dbPathNew,dbPath);
			done=true;
			ret=len;
		} else free(data);
	}

	
	if(data) 
	{
		if (info->shadow_db_version == 1)
		{
		iTunesSD1 sd;
		int l = sd.write(&db->mhsdsongs->mhlt->mhit, data,allocate);
		if(l>0) 
		{
			char dbPath[] = "x:\\iPod_Control\\iTunes\\iTunesSD"; dbPath[0]=drive;
			FILE * f = fopen(dbPath,"wb");
			if(f) {
				fwrite(data,1,l,f);
				fclose(f);
			}
		}
		}
		else if (info->shadow_db_version == 2)
		{
			iTunesSD2 sd;
			int l = sd.write(db->mhsdsongs->mhlt, db->mhsdplaylists->mhlp, data,allocate);
			if(l>0) 
			{
				char dbPath[] = "x:\\iPod_Control\\iTunes\\iTunesSD"; dbPath[0]=drive;
				FILE * f = fopen(dbPath,"wb");
				if(f) {
					fwrite(data,1,l,f);
					fclose(f);
				}
			}
		}
	}

	if(artdb && data) {
		int l = artdb->write(data,allocate);
		if(l>0) {
			char dbPath[] = "x:\\iPod_Control\\Artwork\\ArtworkDB"; dbPath[0]=drive;
			FILE * f = fopen(dbPath,"wb");
			if(f) {
				fwrite(data,1,l,f);
				fclose(f);
			}
		}
	}

	iTunesUnlock(hLock);
	if(data) free(data);

	return ret;
}

__int64 iPodDevice::getDeviceCapacityAvailable() {
	ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
	wchar_t path[4]=L"x:\\";
	path[0]=drive;
	GetDiskFreeSpaceEx(path,  &tfree, &total, &freeb);
	return freeb.QuadPart;
}

__int64 iPodDevice::getDeviceCapacityTotal() {
	ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
	wchar_t path[4]=L"x:\\";
	path[0]=drive;
	GetDiskFreeSpaceEx(path,  &tfree, &total, &freeb);
	return total.QuadPart;
}

void iPodDevice::Close() 
{
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
	//writeiTunesDB();
	
	//iPods.eraseObject(this);
	auto it = std::find(iPods.begin(), iPods.end(), this);
	if (it != iPods.end())
	{
		iPods.erase(it);
	}

	delete this; 
}

void iPodDevice::Eject() 
{
	//iPods.eraseObject(this);
	auto it = std::find(iPods.begin(), iPods.end(), this);
	if (it != iPods.end())
	{
		iPods.erase(it);
	}

	if(EjectVolume(drive)) 
	{
		SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
		delete this; 
	}
	else 
	{
		wchar_t titleStr[32] = {0};
		iPods.push_back(this);
		MessageBox(plugin.hwndLibraryParent,WASABI_API_LNGSTRINGW(IDS_FAILED_TO_EJECT_IPOD),
			WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,titleStr,32),0);
	}
}

extern int CopyFile(const wchar_t * infile, const  wchar_t * outfile, void * callbackContext, void (*callback)(void * callbackContext, wchar_t * status), int * killswitch);

static __int64 fileSize(const wchar_t * filename)
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

void GetFileInfo(const wchar_t * file, const wchar_t * metadata, wchar_t * buf, int len) {
	buf[0]=0;
	extendedFileInfoStructW m = {file,metadata,buf,(size_t)len};
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&m,IPC_GET_EXTENDED_FILE_INFOW);
}

__int64 GetFileInfoInt64(wchar_t * file, wchar_t * metadata, BOOL *w=NULL) {
	wchar_t buf[100]=L"";
	GetFileInfo(file,metadata,buf,100);
	if(w && buf[0]==0) {(*w) = 0; return 0;}
	return _wtoi64(buf);
}

int GetFileInfoInt(wchar_t * file, wchar_t * metadata, BOOL *w=NULL) {
	wchar_t buf[100]=L"";
	GetFileInfo(file,metadata,buf,100);
	if(w && buf[0]==0) {(*w) = 0; return 0;}
	return _wtoi(buf);
}

int iPodDevice::transferTrackToDevice(const itemRecordW *track,void * callbackContext,void (*callback)(void * callbackContext, wchar_t * status),songid_t * songid,int * killswitch) 
{
	bool transcodefile = false;
	wchar_t outfile[2048] = {0};
	wchar_t infile[MAX_PATH] = {0};
	StringCchCopy(infile, MAX_PATH, track->filename);
	bool nocopy=false;

	iPod_mhit * mhit = db->mhsdsongs->mhlt->NewTrack();
	dirnum = (dirnum + 1) % 20;

	// create the output filename and directory
	wchar_t ext[10]=L"";
	const wchar_t *e = wcsrchr(infile,L'.'); 
	if(e) 
		StringCbCopyW(ext, sizeof(ext), e);

	if(transcoder && transcoder->ShouldTranscode(infile)) {
		int r = transcoder->CanTranscode(infile,ext, track->length);
		if(r != 0 && r != -1) transcodefile = true;
	}

	bool video = !_wcsicmp(ext,L".m4v");

	if(!_wcsicmp(ext,L".mp4")) {
		wchar_t buf[100]=L"0";
		extendedFileInfoStructW m = {infile,L"type",buf,100};
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&m,IPC_GET_EXTENDED_FILE_INFOW);
		if(!wcscmp(buf,L"1")) { video=true; wcsncpy(ext,L".m4v",10); }
		else wcsncpy(ext,L".m4a",10);
	}

	// and the location in the ipod naming scheme
	if (infile[0] == drive && infile[1] && infile[1] == L':')
	{
		// file already on the ipod?  add it directly
		StringCbCopy(outfile, sizeof(outfile), infile);
		nocopy=true;
	}
	else
	{
		StringCbPrintf(outfile,sizeof(outfile),L"%c:\\iPod_Control\\Music\\F%02d\\",(wchar_t)drive,dirnum);
		CreateDirectory(outfile,NULL);
		StringCbPrintf(outfile,sizeof(outfile),L"%c:\\iPod_Control\\Music\\F%02d\\w%05d%s",(wchar_t)drive,dirnum,mhit->id,ext);
	}

	wchar_t location[2048] = {0};
	StringCbCopy(location, sizeof(location), outfile+2);
	int i=0;
	while(location[i] != 0) { if(location[i]==L'\\') location[i]=L':'; i++; }

	{
		wchar_t buf[100]=L"";
		int which = AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_source", 0);
		extendedFileInfoStructW m = {infile,which?L"replaygain_album_gain":L"replaygain_track_gain",buf,100};
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&m,IPC_GET_EXTENDED_FILE_INFOW);
		if(buf[0]) {
			double gain = _wtof(&buf[buf[0]==L'+'?1:0]);
			mhit->soundcheck = (unsigned long)(1000.0 * pow(10.0,-0.1*gain));
		}
	}

	// fill in the new MHIT (track item) with our metadata
	mhit->AddString(MHOD_TITLE)->SetString(track->title);
	mhit->AddString(MHOD_LOCATION)->SetString(location);
	mhit->AddString(MHOD_ALBUM)->SetString(track->album);
	mhit->AddString(MHOD_ARTIST)->SetString(track->artist);
	mhit->AddString(MHOD_GENRE)->SetString(track->genre);
	mhit->AddString(MHOD_COMMENT)->SetString(track->comment);
	mhit->AddString(MHOD_ALBUMARTIST)->SetString(track->albumartist);
	mhit->AddString(MHOD_COMPOSER)->SetString(track->composer);
	mhit->length = (track->length>0)?track->length*1000:0;
	mhit->year = (track->year>0)?track->year:0;
	mhit->tracknum = (track->track>0)?track->track:0;
	mhit->totaltracks = (track->tracks>0)?track->tracks:0;
	mhit->stars = (unsigned char)(mhit->app_rating = track->rating);
	mhit->playcount = mhit->playcount2 = track->playcount;
	mhit->lastplayedtime = wintime_to_mactime(track->lastplay);
	mhit->lastmodifiedtime = wintime_to_mactime(track->lastupd);
	mhit->compilation = track->albumartist && !_wcsicmp(track->albumartist, L"various artists");
	mhit->samplerate = 44100; // TODO: benski> we could query this from the input plugin, but we'd have to be careful with HE-AAC
	mhit->samplerate2 = 44100.0f;
	mhit->mediatype = video?0x02:0x01;
	mhit->movie_flag = video?1:0;
	mhit->cdnum = (track->disc>0)?track->disc:0;
	mhit->totalcds = (track->discs>0)?track->discs:0;
	mhit->BPM=(track->bpm>0)?track->bpm:0;

	wchar_t *pubdate = getRecordExtendedItem(track,L"podcastpubdate");
	if(pubdate && *pubdate) mhit->releasedtime=wintime_to_mactime(_wtoi(pubdate));

	// copy the file over
	int r;
	if(transcodefile)
		r = transcoder->TranscodeFile(infile,outfile,killswitch,callback,callbackContext);
	else if (!nocopy)
		r = CopyFile(infile,outfile,callbackContext,callback,killswitch);
	else
	{
		if (callback)
		{
			wchar_t langtemp[100] = {0};
			callback(callbackContext, WASABI_API_LNGSTRINGW_BUF(IDS_DONE, langtemp, 100));
		}
		r=0;
	}
	if(r == 0) 
	{
		StringCbCopyW(ext, sizeof(ext), wcsrchr(outfile,L'.'));
		if (!_wcsicmp(ext, L".m4a") || !_wcsicmp(ext, L".mp4"))
		{
			mhit->vbr = 0;
			mhit->type = 0;
			mhit->unk14 = 51;
			mhit->filetype = FILETYPE_M4A;
		}
		else if (!_wcsicmp(ext, L".mp3"))
		{
			mhit->type = 1;
			mhit->unk27 = 1;
			mhit->unk14 = 12;
			mhit->AddString(MHOD_FILETYPE)->SetString(L"MPEG audio file");
			mhit->filetype = FILETYPE_MP3;
		}
		else if (!_wcsicmp(ext, L".wav"))
		{
			mhit->filetype = FILETYPE_WAV;
		}
		mhit->samplecount = GetFileInfoInt64(outfile,L"numsamples");
		mhit->pregap = (unsigned long)GetFileInfoInt64(outfile,L"pregap");
		mhit->postgap = (unsigned long)GetFileInfoInt64(outfile,L"postgap");
		mhit->gaplessData = (unsigned long)GetFileInfoInt64(outfile,L"endoffset");
		mhit->trackgapless = 1;

		mhit->size = (unsigned long)fileSize(outfile);
		if (!transcodefile && track->bitrate > 0) 
			mhit->bitrate = track->bitrate;
		else
		{
			mhit->bitrate = (unsigned long)GetFileInfoInt64(outfile,L"bitrate");
			if (!mhit->bitrate)
			{
				if (track->length > 0) 
					mhit->bitrate = (mhit->size / track->length)/125;
				else 
					mhit->bitrate = 128;
			}
		}


		*songid = (songid_t)mhit;
	} 
	else
	{
		DeleteFileW(outfile);
		delete mhit;
	}
	return r;
}

int iPodDevice::trackAddedToTransferQueue(const itemRecordW *track) {
	__int64 l;
	if(transcoder && transcoder->ShouldTranscode(track->filename)) {
		int k = transcoder->CanTranscode(track->filename, 0, track->length);
		if(k == -1) return -2;
		if(k == 0) l = (__int64)fileSize(track->filename);
		else l = (__int64)k;
	} else {
		wchar_t * ext = wcsrchr(track->filename,L'.');
		if(!ext) return -2;
		if(_wcsicmp(ext,L".mp3") && _wcsicmp(ext,L".wav") && _wcsicmp(ext,L".m4a") &&
			_wcsicmp(ext,L".m4b") && _wcsicmp(ext,L".aa") && _wcsicmp(ext,L".m4v") &&
			_wcsicmp(ext,L".mp4")) return -2;

		l = (__int64)fileSize(track->filename);
	}
	__int64 avail = getDeviceCapacityAvailable();
	__int64 cmp = transferQueueLength;
	cmp += l;
	cmp += (__int64)3000000;

	if(cmp > avail)
		return -1;
	else {
		transferQueueLength += l;
		return 0;
	}
}

void iPodDevice::trackRemovedFromTransferQueue(const itemRecordW *track) {
	__int64 l = (__int64)fileSize(track->filename);
	if(transcoder && transcoder->ShouldTranscode(track->filename)) {
		int k = transcoder->CanTranscode(track->filename, 0, track->length);
		if(k != -1 && k != 0) l = (__int64)k;
	}
	transferQueueLength -= l;
}

__int64 iPodDevice::getTrackSizeOnDevice(const itemRecordW *track) {
	if(transcoder && transcoder->ShouldTranscode(track->filename)) {
		int k = transcoder->CanTranscode(track->filename, 0, track->length);
		if(k != -1 && k != 0) return k;
	}
	wchar_t * ext = wcsrchr(track->filename,'.');
	if(!ext) return 0;
	if(_wcsicmp(ext,L".mp3") && _wcsicmp(ext,L".wav") && _wcsicmp(ext,L".m4a") &&
		_wcsicmp(ext,L".m4b") && _wcsicmp(ext,L".aa") && _wcsicmp(ext,L".m4v") && 
		_wcsicmp(ext,L"mp4")) return 0;
	return fileSize(track->filename);
}

void iPodDevice::deleteTrack(songid_t songid) {

	iPod_mhit * mhit = (iPod_mhit *)songid;
	iPod_mhod * mhod = mhit->FindString(MHOD_LOCATION);
	if(!mhod) return;
	wchar_t * t = mhod->str;

	// change ':' to '\\;
	wchar_t * p = t;
	int l = wcslen(t);
	for(int j=0; j<l; j++) if(*(p++)==L':') *(p-1)=L'\\';

	// add drive onto front
	wchar_t file[2048] = L"x:\\";
	file[0] = driveW;
	wcscat(file,t);

	//check this file isn't playing...
	wchar_t* curPlaying = (wchar_t*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GET_PLAYING_FILENAME);
	if(curPlaying && !_wcsicmp(curPlaying,file))  SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); //stop

	//delete :)
	// benski> we might have a file that has been deleted from the disk but not the DB
	if(!DeleteFileW(file) // check for file failure
		&& GetFileAttributes(file) != INVALID_FILE_ATTRIBUTES)  // but only fail if the file actually exists
		return; 

	setArt(songid,NULL,0,0);

	l = playlists.GetSize();
	for(int i=0; i<l; i++) {
		iPod_mhyp * mhyp = ((iPod_mhyp*)playlists.Get(i)); //->DeletePlaylistEntryByID(mhit->id);
		for(unsigned int j=0; j<mhyp->GetMhipChildrenCount(); j++) {
			if(mhyp->GetPlaylistEntry(j)->songindex == mhit->id) mhyp->DeletePlaylistEntry(j);
		}
	}


	db->mhsdsongs->mhlt->DeleteTrackByID(mhit->id);
}

int iPodDevice::getPlaylistCount() {
	return playlists.GetSize();
}

static void readStringMHOD(iPod_mhod * mhod, wchar_t * buf, int len) {
	if(mhod) lstrcpyn(buf,mhod->str,len);
	else buf[0]=0;
}

static void setStringMHOD(iPod_mhod * mhod, const wchar_t *buf) {
	mhod->SetString(buf);
}

void iPodDevice::getPlaylistName(int playlistnumber, wchar_t * buf, int len) {
	iPod_mhod * name = ((iPod_mhyp*)playlists.Get(playlistnumber))->FindString(MHOD_TITLE);
	readStringMHOD(name,buf,len); 
}

int iPodDevice::getPlaylistLength(int playlistnumber) {
	return ((iPod_mhyp*)playlists.Get(playlistnumber))->GetMhipChildrenCount();
}

static iPod_mhit blank;

songid_t iPodDevice::getPlaylistTrack(int playlistnumber,int songnum) {
	int idx  = ((iPod_mhyp*)playlists.Get(playlistnumber))->GetPlaylistEntry(songnum)->songindex;
	iPod_mhlt::mhit_map_t::const_iterator f = db->mhsdsongs->mhlt->mhit.find(idx);
	if(f != db->mhsdsongs->mhlt->mhit.end() && idx) return (songid_t)f->second;
	else {
		iPod_mhip* m = ((iPod_mhyp*)playlists.Get(playlistnumber))->GetPlaylistEntry(songnum);
		blank.DeleteString(4);
		if(m->podcastgroupflag && m->mhod[0]) {			
			iPod_mhod * mh = blank.AddString(4);
			mh->SetString(m->mhod[0]->str);
		}
		return (songid_t)&blank;
	}
}

void iPodDevice::setPlaylistName(int playlistnumber, const wchar_t *buf) {
	iPod_mhod * name = ((iPod_mhyp*)playlists.Get(playlistnumber))->FindString(MHOD_TITLE);
	if(!name) name = ((iPod_mhyp*)playlists.Get(playlistnumber))->AddString(MHOD_TITLE);
	setStringMHOD(name,buf);
	if(playlistnumber == 0) {
		wchar_t volumename[12] = {0};
		const wchar_t * p = buf;
		for(int i=0; i<11;) {
			if(*p!=L' ' && *p!=L'\t') volumename[i++]=*p;
			if(*(p++)==0) break;
		}
		volumename[11]=0;
		char root[] = {drive,":\\"};
		SetVolumeLabel(AutoWide(root),volumename);
	}
}

void iPodDevice::playlistSwapItems(int playlistnumber, int posA, int posB) {
	iPod_mhyp * p = ((iPod_mhyp*)playlists.Get(playlistnumber));
	iPod_mhip * a = p->mhip.at(posA);
	iPod_mhip * b = p->mhip.at(posB);
	if(a && b) {
		p->mhip[posA] = b;
		p->mhip[posB] = a;
	}
}

static iPod_mhyp * sortpl;
static int sortby;
static iPod_mhbd * sortdb;

#define CMPFIELDS(x) { int v=0; iPod_mhod * am = a->FindString(x); iPod_mhod * bm = b->FindString(x); if(am && bm) v = lstrcmpi(am->str,bm->str); else if(am != bm) v = am==NULL?-1:1; if(v!=0) return v<0; }
#define CMPINTFIELDS(x,y) { int v = x-y; if(v!=0) return v<0; }

struct PlaylistItemSort {
	bool operator()(iPod_mhip*& ap,iPod_mhip*& bp) {
		int use_by = sortby;
		iPod_mhit * a = sortdb->mhsdsongs->mhlt->mhit.find(ap->songindex)->second;
		iPod_mhit * b = sortdb->mhsdsongs->mhlt->mhit.find(bp->songindex)->second;

		// this might be too slow, but it'd be nice
		int x;
		for (x = 0; x < 5; x ++)
		{
			if (use_by == SORTBY_TITLE) // title -> artist -> album -> disc -> track
			{
				CMPFIELDS(MHOD_TITLE);
				use_by=SORTBY_ARTIST;
			}
			else if (use_by == SORTBY_ARTIST) // artist -> album -> disc -> track -> title
			{
				CMPFIELDS(MHOD_ARTIST);
				use_by=SORTBY_ALBUM;
			}
			else if (use_by == SORTBY_ALBUM) // album -> disc -> track -> title -> artist
			{
				CMPFIELDS(MHOD_ALBUM);
				use_by=SORTBY_DISCNUM;
			}
			else if (use_by == SORTBY_DISCNUM) // disc -> track -> title -> artist -> album
			{
				CMPINTFIELDS(a->cdnum,b->cdnum);
				use_by=SORTBY_TRACKNUM;
			}
			else if (use_by == SORTBY_TRACKNUM) // track -> title -> artist -> album -> disc
			{
				CMPINTFIELDS(a->tracknum,b->tracknum);
				use_by=SORTBY_TITLE;     
			}
			else if (use_by == SORTBY_GENRE) // genre -> artist -> album -> disc -> track
			{
				CMPFIELDS(MHOD_GENRE);
				use_by=SORTBY_ARTIST;
			}
			else if (use_by == SORTBY_PLAYCOUNT) // size -> artist -> album -> disc -> track
			{
				CMPINTFIELDS(a->playcount,b->playcount);
				use_by=SORTBY_ARTIST;
			}
			else if (use_by == SORTBY_RATING) // size -> artist -> album -> disc -> track
			{
				CMPINTFIELDS(a->stars,b->stars);
				use_by=SORTBY_ARTIST;
			}
			else if (use_by == SORTBY_LASTPLAYED)
			{
				double t = difftime(a->lastplayedtime,b->lastplayedtime);
				if(t != 0) return t>0;
				use_by=SORTBY_ARTIST;
			}
			else break; // no sort order?
		}
		return false;
	}
};

#undef CMPFIELDS
#undef CMPINTFIELDS

void iPodDevice::sortPlaylist(int playlistnumber, int sortby0) {
	sortpl = ((iPod_mhyp*)playlists.Get(playlistnumber));
	sortby = sortby0;
	sortdb = db;
	std::sort(sortpl->mhip.begin(),sortpl->mhip.end(),PlaylistItemSort());
}

void iPodDevice::addTrackToPlaylist(int playlistnumber, songid_t songid) 
{
	iPod_mhit *mhit = (iPod_mhit *)songid;

	if (playlistnumber == 0)
	{
		db->mhsdsongs->mhlt->AddTrack(mhit);
		db->mhsdplaylists->mhlp->GetDefaultPlaylist()->AddPlaylistEntry(NULL, mhit->id);
	}
	else
	{
		((iPod_mhyp*)playlists.Get(playlistnumber))->AddPlaylistEntry(NULL, mhit->id);
	}
}

void iPodDevice::removeTrackFromPlaylist(int playlistnumber, int songnum) {
	((iPod_mhyp*)playlists.Get(playlistnumber))->DeletePlaylistEntry(songnum);
}

void iPodDevice::deletePlaylist(int playlistnumber) {
	iPod_mhyp* p = ((iPod_mhyp*)playlists.Get(playlistnumber));
	playlists.Del(playlistnumber);
	db->mhsdplaylists->mhlp->DeletePlaylistByID(p->playlistID);
}

int iPodDevice::newPlaylist(const wchar_t *name) {
	iPod_mhyp * p = db->mhsdplaylists->mhlp->AddPlaylist();
	playlists.Add(p);
	int ret = db->mhsdplaylists->mhlp->GetChildrenCount() - 1;
	setPlaylistName(ret,name);
	db->mhsdplaylists->mhlp->SortPlaylists();
	return ret;
}

void iPodDevice::getTrackArtist(songid_t songid, wchar_t * buf, int len) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_ARTIST);
	readStringMHOD(mhod,buf,len);
}

void iPodDevice::getTrackAlbum(songid_t songid, wchar_t * buf, int len) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_ALBUM);
	readStringMHOD(mhod,buf,len);
}

void iPodDevice::getTrackTitle(songid_t songid, wchar_t * buf, int len) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_TITLE);
	readStringMHOD(mhod,buf,len);
}

void iPodDevice::getTrackGenre(songid_t songid, wchar_t * buf, int len) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_GENRE);
	readStringMHOD(mhod,buf,len);
}

int iPodDevice::getTrackTrackNum(songid_t songid) {
	return ((iPod_mhit *)songid)->tracknum;
}

int iPodDevice::getTrackDiscNum(songid_t songid) {
	return ((iPod_mhit *)songid)->cdnum;
}

int iPodDevice::getTrackYear(songid_t songid) {
	return (int)(((iPod_mhit *)songid)->year);
}

__int64 iPodDevice::getTrackSize(songid_t songid) {
	return ((iPod_mhit *)songid)->size;
}

int iPodDevice::getTrackLength(songid_t songid) {
	return ((iPod_mhit *)songid)->length;
}

int iPodDevice::getTrackBitrate(songid_t songid) {
	return ((iPod_mhit *)songid)->bitrate;
}

int iPodDevice::getTrackPlayCount(songid_t songid) {
	return ((iPod_mhit *)songid)->playcount;
}

int iPodDevice::getTrackRating(songid_t songid) {
	return ((iPod_mhit *)songid)->stars / 20;
}

__time64_t iPodDevice::getTrackLastPlayed(songid_t songid) {
	return mactime_to_wintime(((iPod_mhit *)songid)->lastplayedtime);
}

__time64_t iPodDevice::getTrackLastUpdated(songid_t songid) {
	return mactime_to_wintime(((iPod_mhit *)songid)->lastmodifiedtime);
}

void iPodDevice::getTrackAlbumArtist(songid_t songid, wchar_t * buf, int len) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_ALBUMARTIST);
	readStringMHOD(mhod,buf,len);
	if(!mhod) getTrackArtist(songid,buf,len);
}
void iPodDevice::getTrackComposer(songid_t songid, wchar_t * buf, int len) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_COMPOSER);
	readStringMHOD(mhod,buf,len);
}

int iPodDevice::getTrackType(songid_t songid) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_LOCATION);
	if(!mhod) return 0;
	wchar_t * ext = wcsrchr(mhod->str,L'.');
	if(!ext) return 0;
	if(!_wcsicmp(ext,L".m4v")) return 1;
	return 0;
}

void iPodDevice::getTrackExtraInfo(songid_t songid, const wchar_t *field, wchar_t * buf, int len) {
	if(!wcscmp(field,FIELD_EXTENSION)) {
		wchar_t buf2[1024]=L"";
		getFilename(buf2,1024,songid);
		wchar_t * ext = wcsrchr(buf2,L'.');
		if(ext) { ext++; lstrcpyn(buf,ext,len); }
	}
}

void iPodDevice::setTrackArtist(songid_t songid, const wchar_t *value) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_ARTIST);
	if(!mhod) mhod = ((iPod_mhit *)songid)->AddString(MHOD_ARTIST);
	setStringMHOD(mhod,value);
}

void iPodDevice::setTrackAlbum(songid_t songid, const wchar_t *value) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_ALBUM);
	if(!mhod) mhod = ((iPod_mhit *)songid)->AddString(MHOD_ALBUM);
	setStringMHOD(mhod,value);
}

void iPodDevice::setTrackTitle(songid_t songid, const wchar_t *value) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_TITLE);
	if(!mhod) mhod = ((iPod_mhit *)songid)->AddString(MHOD_TITLE);
	setStringMHOD(mhod,value);
}

void iPodDevice::setTrackTrackNum(songid_t songid, int value) {
	((iPod_mhit *)songid)->tracknum = value;
}

void iPodDevice::setTrackDiscNum(songid_t songid, int value) {
	((iPod_mhit *)songid)->cdnum = value;
}

void iPodDevice::setTrackGenre(songid_t songid, const wchar_t *value) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_GENRE);
	if(!mhod) mhod = ((iPod_mhit *)songid)->AddString(MHOD_GENRE);
	setStringMHOD(mhod,value);
}

void iPodDevice::setTrackYear(songid_t songid, int value) {
	((iPod_mhit *)songid)->year = (unsigned long)value;
}

void iPodDevice::setTrackPlayCount(songid_t songid, int value) {
	((iPod_mhit *)songid)->playcount = value;
}

void iPodDevice::setTrackRating(songid_t songid, int value) {
	((iPod_mhit *)songid)->app_rating = ((iPod_mhit *)songid)->stars = value*20;
}

void iPodDevice::setTrackLastPlayed(songid_t songid, __time64_t value) {
	((iPod_mhit *)songid)->lastplayedtime = wintime_to_mactime(value);
}

void iPodDevice::setTrackLastUpdated(songid_t songid, __time64_t value) {
	((iPod_mhit *)songid)->lastmodifiedtime = wintime_to_mactime(value);
}

void iPodDevice::setTrackAlbumArtist(songid_t songid, const wchar_t *value) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_ALBUMARTIST);
	if(!mhod) mhod = ((iPod_mhit *)songid)->AddString(MHOD_ALBUMARTIST);
	setStringMHOD(mhod,value);
}

void iPodDevice::setTrackComposer(songid_t songid, const wchar_t *value) {
	iPod_mhod * mhod = ((iPod_mhit *)songid)->FindString(MHOD_COMPOSER);
	if(!mhod) mhod = ((iPod_mhit *)songid)->AddString(MHOD_COMPOSER);
	setStringMHOD(mhod,value);
}

void iPodDevice::getFilename(char * buf, int len, songid_t song) {
	iPod_mhod * mhod = ((iPod_mhit *)song)->FindString(MHOD_LOCATION);
	if(!mhod) {buf[0]=0; return;}
	char * filename = UTF16_to_char(mhod->str,mhod->length);
	char * p = filename;
	buf[0] = drive;
	buf[1] = ':';
	int j=2;
	while(p && *p && j < len-1) { if(*p==':') buf[j]='\\'; else buf[j]=*p; p++; j++; }
	buf[j]=0;
	free(filename);
}

void iPodDevice::getFilename(wchar_t * buf, int len, songid_t song) {
	iPod_mhod * mhod = ((iPod_mhit *)song)->FindString(MHOD_LOCATION);
	if(!mhod) {buf[0]=0; return;}
	wchar_t * filename = mhod->str;
	wchar_t * p = filename;
	buf[0] = drive;
	buf[1] = L':';
	int j=2;
	while(p && *p && j < len-1) { if(*p==L':') buf[j]=L'\\'; else buf[j]=*p; p++; j++; }
	buf[j]=0;
}

typedef struct { songid_t song; Device * dev; const wchar_t * filename; } tagItem;

static wchar_t * tagFunc(const wchar_t * tag, void * p) { //return 0 if not found, -1 for empty tag
	tagItem * s = (tagItem *)p;
	int len = 2048;
	wchar_t * buf = (wchar_t *)malloc(sizeof(wchar_t)*len);
	if (!_wcsicmp(tag, L"artist"))	s->dev->getTrackArtist(s->song,buf,len);
	else if (!_wcsicmp(tag, L"album"))	s->dev->getTrackAlbum(s->song,buf,len);
	else if (!_wcsicmp(tag, L"title"))	s->dev->getTrackTitle(s->song,buf,len);
	else if (!_wcsicmp(tag, L"genre"))	s->dev->getTrackGenre(s->song,buf,len);
	else if (!_wcsicmp(tag, L"year")) 
	{
		int year = s->dev->getTrackYear(s->song);
		if (year>0)
			StringCchPrintf(buf,len,L"%d",year);
		else
			buf[0]=0;
	}
	else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track"))
	{
		int track = s->dev->getTrackTrackNum(s->song);
		if (track>0)
			StringCchPrintf(buf,len,L"%d",track);
		else
			buf[0]=0;
	}
	else if (!_wcsicmp(tag, L"discnumber"))
	{
		int disc = s->dev->getTrackDiscNum(s->song);
		if (disc>0)
			StringCchPrintf(buf,len,L"%d",disc);
		else
			buf[0]=0;
	}
	else if (!_wcsicmp(tag, L"bitrate")) 
	{
		int bitrate = s->dev->getTrackBitrate(s->song);
		if (bitrate>0)
			StringCchPrintf(buf,len,L"%d",bitrate);
		else
			buf[0]=0;
	}
	else if (!_wcsicmp(tag, L"filename")) lstrcpyn(buf,s->filename,len);
	else buf[0]=0;
	return buf;
}

static void tagFreeFunc(wchar_t *tag, void *p) { if(tag) free(tag); }

void getTitle(Device * dev, songid_t song, const wchar_t * filename, wchar_t * buf, int len) {
	buf[0]=0; buf[len-1]=0;
	tagItem item = {song,dev,filename};
	waFormatTitleExtended fmt={filename,0,NULL,&item,buf,len,tagFunc,tagFreeFunc};
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE_EXTENDED);
}

bool iPodDevice::playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue) {
	//char buf[2048]="";
	wchar_t wbuf[2048]=L"";

	if(!enqueue) { //clear playlist
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
		/*int l=SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_PE_GETINDEXTOTAL);
		while(l>=0) SendMessage(plugin.hwndWinampParent,WM_WA_IPC,--l,IPC_PE_DELETEINDEX);*/
	}

	for(int i=0; i<listLength; i++) {
		getFilename(wbuf,2048,songidList[i]);
		//strcpy(buf,AutoChar(wbuf));

		wchar_t title[2048] = {0};
		getTitle(this,songidList[i],wbuf,title,2048);

		/*enqueueFileWithMetaStruct s={buf,strdup(AutoChar(title)),getTrackLength(songidList[i])/1000};
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILE);
		free((void*)s.title);*/

		enqueueFileWithMetaStructW s={wbuf,_wcsdup(title),PathFindExtensionW(wbuf),getTrackLength(songidList[i]) / 1000};
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
		free((void*)s.title);
	}

	if(!enqueue) { //play item startPlaybackAt
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,startPlaybackAt,IPC_SETPLAYLISTPOS);
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); //stop
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); //play
	}
	return true;
}

int iPodDevice::copyToHardDrive(songid_t song, // the song to copy
																wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
																void * callbackContext, //pass this to the callback
																void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
																int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
																) // -1 for failed/not supported. 0 for success.
{
	wchar_t fn[2048] = {0}; // song filename on ipod
	getFilename(fn,2048,song);
	wchar_t * ext = wcsrchr(fn,L'.');
	if(!ext) { callback(callbackContext,WASABI_API_LNGSTRINGW(IDS_INVALID_TRACK)); return -1; }
	wcscat(path,ext);
	return CopyFile(fn,path,callbackContext,callback,killswitch);
}

// art functions
static void fileputinhole(const wchar_t* file, unsigned int pos, int len, void* newdata) {
	__int64 fs = fileSize(file);
	int open_flags = OPEN_EXISTING;
	if(fs <= 0) open_flags = CREATE_NEW;
	HANDLE hw = CreateFile(file,GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,open_flags,0,NULL);
	if(hw == INVALID_HANDLE_VALUE) return;
	SetFilePointer(hw,pos,NULL,FILE_BEGIN);
	DWORD written=0;
	WriteFile(hw,newdata,len,&written,NULL);
	CloseHandle(hw);
}

ArtDataObject * makeThumbMetadata(const ArtworkFormat * thumb, wchar_t drive, Image * image, ArtDB *artdb) {
	wchar_t file[MAX_PATH] = {0};
	wsprintfW(file,L"%c:\\iPod_Control\\Artwork\\F%04d_1.ithmb",drive,thumb->correlation_id);

	bool found=false;
	ArtFile *f=NULL;
	for(size_t i=0; i < artdb->fileListDS->fileList->files.size(); i++) {
		f = artdb->fileListDS->fileList->files[i];
		if(f->corrid == thumb->correlation_id) { found=true; break; }
	}
	if(!found) {
		f = new ArtFile;
		f->corrid = thumb->correlation_id;
		f->imagesize = image->get16BitSize(thumb->row_align, thumb->image_align);
		artdb->fileListDS->fileList->files.push_back(f);
		f->file = _wcsdup(file);
	}

	ArtDataObject * ms = new ArtDataObject;
	ms->type=2;
	ms->image = new ArtImageName;
	ms->image->corrid = thumb->correlation_id;
	ms->image->imgw = thumb->width;
	ms->image->imgh = thumb->height;
	ms->image->imagesize = image->get16BitSize(thumb->row_align, thumb->image_align);

	//__int64 fs = fileSize(file);
	//ms->image->ithmboffset = fs>0?fs:0;
	ms->image->ithmboffset = f->getNextHole(ms->image->imagesize);

	wchar_t buf[100] = {0};
	StringCchPrintf(buf,100,L":F%04d_1.ithmb",thumb->correlation_id);
	ms->image->filename = new ArtDataObject;
	ms->image->filename->type=3;
	ms->image->filename->SetString(buf);
	unsigned short *data = (unsigned short *)calloc(ms->image->imagesize,1);
	image->exportToRGB565((RGB565*)data, thumb->format, thumb->row_align, thumb->image_align);
	fileputinhole(file,ms->image->ithmboffset,ms->image->imagesize,data);
	//writeDataToThumb(file,data,thumb->width * thumb->height);
	free(data);

	f->images.push_back(new ArtFileImage(ms->image->ithmboffset,ms->image->imagesize,1));
	f->sortImages();

	return ms;
}

void GetTempFilePath(wchar_t *path) {
	wchar_t dir[MAX_PATH] = {0};
	GetTempPath(MAX_PATH,dir);
	GetTempFileName(dir,L"ml_pmp",0,path);
}

static void fileclosehole(const wchar_t* file, unsigned int pos, int len) {
	HANDLE hw = CreateFile(file,GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	if(hw == INVALID_HANDLE_VALUE) return;
	HANDLE hr = CreateFile(file,GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	if(hr == INVALID_HANDLE_VALUE) { CloseHandle(hw); return; }
	SetFilePointer(hw,pos,NULL,FILE_BEGIN);
	SetFilePointer(hr,pos+len,NULL,FILE_BEGIN);
	DWORD fs = GetFileSize(hw,NULL);
	if(pos == 0 && len == fs) {	CloseHandle(hr); CloseHandle(hw); _wunlink(file); return; }
	unsigned int p = pos;
	while(1) {
		BYTE buf[65536] = {0};
		DWORD read=0;
		ReadFile(hr,buf,sizeof(buf),&read,NULL);
		if(!read) break;
		DWORD written=0;
		WriteFile(hw,buf,read,&written,NULL);
		if(!written) break;
		p+=read;
		if(p>=fs) break;
	}

	SetFilePointer(hw,fs - len,NULL,FILE_BEGIN);
	SetEndOfFile(hw);
	CloseHandle(hr);
	CloseHandle(hw);

}

static bool replaceart(songid_t songid, wchar_t driveW, ArtDB *artdb, std::vector<const ArtworkFormat*> * thumbs, std::vector<Image*> * images) {
	//return false;
	__int64 dbid = ((iPod_mhit*)songid)->dbid;
	int done=0;
	ArtImageList::ArtImageMapIterator art = artdb->imageListDS->imageList->images.find(dbid);
	if(art != artdb->imageListDS->imageList->images.end() && art->second) { // replace old art
		for(size_t i=0; i!=art->second->dataobjs.size(); i++) if(art->second->dataobjs[i]->image) {
			ArtImageName * in = art->second->dataobjs[i]->image;
			for(size_t j=0; j!=thumbs->size(); j++)
			{
				if(in->corrid == thumbs->at(j)->correlation_id) {
					wchar_t file[MAX_PATH] = {0};
					wsprintfW(file,L"%c:\\iPod_Control\\Artwork\\F%04d_1.ithmb",driveW,in->corrid);
					int size = images->at(j)->get16BitSize(thumbs->at(j)->row_align, thumbs->at(j)->image_align);
					if(size == in->imagesize) {
						unsigned short *data = (unsigned short *)malloc(size);
						images->at(j)->exportToRGB565((RGB565*)data, thumbs->at(j)->format, thumbs->at(j)->row_align, thumbs->at(j)->image_align);
						fileputinhole(file,in->ithmboffset,in->imagesize,data);
						free(data);
						done++;
					}
				}
			}
		}
	}
	return (done == thumbs->size());
}

void iPodDevice::setArt(songid_t songid, void *bits, int w, int h) { //buf is in format ARGB32*

	if(!artdb || !thumbs.size()) return; // art not supported
	iPod_mhit * mhit = (iPod_mhit *)songid;

	if(bits == NULL || w == 0 || h == 0) { // remove art
		ArtImageList::ArtImageMapIterator arti = artdb->imageListDS->imageList->images.find(mhit->dbid);
		if(arti == artdb->imageListDS->imageList->images.end() || !arti->second) return;
		ArtImage * art = arti->second;		
		for(auto j = art->dataobjs.begin(); j!=art->dataobjs.end(); j++) {
			ArtImageName *n = (*j)->image;
			if(n) 
			{
				ArtFile * f = artdb->fileListDS->fileList->getFile(n->corrid);
				if(f) 
				{
					bool found=false;
					for(size_t i=0; i!=f->images.size(); i++) 
					{
						if(!found && f->images[i]->start == n->ithmboffset && --f->images[i]->refcount==0)
						{
							delete f->images[i];
							f->images.erase(f->images.begin() + i);
							i--;
							found=true;
						}
					}
				}
			}
		}
		mhit->mhii_link = 0;
		mhit->artworkcount = 0;
		mhit->hasArtwork = 0;
		artdb->imageListDS->imageList->images.erase(arti);
		delete art;
	} else {
		//setArt(songid,NULL,0,0); // clear old art first

		HQSkinBitmap albumart((ARGB32*)bits, w, h); // wrap image into a bitmap object (no copying done)

		std::vector<Image*> images;
		for(size_t i=0; i!=thumbs.size(); i++) {
			BltCanvas canvas(thumbs[i]->width,thumbs[i]->height);
			albumart.stretch(&canvas, 0, 0, thumbs[i]->width,thumbs[i]->height);
			images.push_back(new Image((ARGB32 *)canvas.getBits(), thumbs[i]->width,thumbs[i]->height));
		}

		if(!replaceart(songid,driveW,artdb,&thumbs,&images)) {
			setArt(songid,NULL,0,0);
			ArtImage * artimg = new ArtImage();
			artimg->songid = mhit->dbid;
			artimg->id = artdb->nextid++;
			artimg->srcImageSize = w*h*4;//0; //fileSize(infile);
			//artimg->srcImageSize = mhit->unk45 = rand();
			mhit->artworksize = 0;
			for(size_t i=0; i!=thumbs.size(); i++)
			{
				artimg->dataobjs.push_back(makeThumbMetadata(thumbs[i],driveW,images[i],artdb));
				mhit->artworksize += thumbs[i]->width * thumbs[i]->height * sizeof(short);
			}
			artdb->imageListDS->imageList->images.insert(ArtImageList::ArtImageMapPair(artimg->songid,artimg));
			mhit->artworkcount = 1;//thumbs.size();
			mhit->hasArtwork = 1;//thumbs.size();
			mhit->mhii_link = artimg->id;
		}

		//images.deleteAll();
		for (auto image : images)
		{
			delete image;
		}
		images.clear();
	}
}

class ipodart_t {
public:
	ipodart_t(ArtImageName *in, wchar_t driveW,int w, int h, const ArtworkFormat* format): w(w),h(h),image(0),error(0),resized(0),format(format) {
		wsprintf(fn,L"%c:\\iPod_Control\\Artwork",driveW);
		wchar_t *p = fn+wcslen(fn);
		in->filename->GetString(p,MAX_PATH - (p - fn));
		while(p && *p) {if(*p == L':') *p=L'\\'; p++;}
		offset = in->ithmboffset;
	}
	~ipodart_t() { if(image) delete image; }
	Image * GetImage() {
		if(image || error) return image;
		int size = Image::get16BitSize(w,h,format->row_align, format->image_align);
		RGB565 * r = (RGB565*)calloc(size,1);
		if(!r) { return 0; error=1; }
		FILE *f = _wfopen(fn,L"rb");
		if(!f) { free(r); error=1; return 0; }
		fseek(f,offset,0);
		if(fread(r,size,1,f) != 1) { free(r); fclose(f); error=1; return 0; }
		fclose(f);
		image = new Image(r,w,h,format->format,format->row_align, format->image_align);
		free(r);
		return image;
	}
	Image * RegetImage() {
		if(image) delete image; image=0;
		return GetImage();
	}
	int GetError() {return error;}
	int getHeight(){if(image) return image->getHeight(); else return h;}
	int getWidth() {if(image) return image->getWidth(); else return w;}
	int resized;
	void Resize(int neww, int newh)
	{
		HQSkinBitmap temp(image->getData(), image->getWidth(), image->getHeight()); // wrap into a SkinBitmap (no copying involved)
		BltCanvas newImage(neww,newh);
		temp.stretch(&newImage, 0, 0, neww, newh);
		delete image;
		image = new Image((ARGB32 *)newImage.getBits(), neww, newh);
		resized=1;
	}
private:
	wchar_t fn[MAX_PATH];
	int offset;
	int w,h;
	Image * image;
	int error;
	const ArtworkFormat* format;
};

pmpart_t iPodDevice::getArt(songid_t songid) {
	if(!artdb) return 0;
	__int64 dbid = ((iPod_mhit*)songid)->dbid;
	ArtImageList::ArtImageMapIterator art = artdb->imageListDS->imageList->images.find(dbid);
	if(art == artdb->imageListDS->imageList->images.end() || !art->second) return 0;
	int l = art->second->dataobjs.size();

	ArtImageName * in=0;
	int w=0,h=0;
	const ArtworkFormat * format=0;

	for(int i=0; i<l; i++) {
		if(art->second->dataobjs[i]->image && (w < art->second->dataobjs[i]->image->imgw || h < art->second->dataobjs[i]->image->imgh)) {
			in = art->second->dataobjs[i]->image;
			w = in->imgw;
			h = in->imgh;
			for(size_t i=0; i < thumbs.size(); i++)
			{
				const ArtworkFormat *f = thumbs.at(i);
				if(f->width == w && f->height == h)
					format = f;
			}
		}
	}
	if(!in || !format) return 0;

	return (pmpart_t)new ipodart_t(in,driveW,w,h,format);
}

void iPodDevice::releaseArt(pmpart_t art) {
	if(!art) return;
	ipodart_t *image = (ipodart_t *)art;
	delete image;
}

int iPodDevice::drawArt(pmpart_t art, HDC dc, int x, int y, int w, int h) {
	Image *image = ((ipodart_t*)art)->GetImage();
	if(!image) return 0;
	HQSkinBitmap temp(image->getData(), image->getWidth(), image->getHeight()); // wrap into a SkinBitmap (no copying involved)
	DCCanvas canvas(dc);
	temp.stretch(&canvas,x,y,w,h);
	return 1;

}

void iPodDevice::getArtNaturalSize(pmpart_t art, int *w, int *h){
	ipodart_t *image = (ipodart_t*)art;
	if(!image) return;
	*h = image->getHeight();
	*w = image->getWidth();
}

void iPodDevice::setArtNaturalSize(pmpart_t art, int w, int h){
	Image *image = ((ipodart_t*)art)->GetImage();
	if(!image) return;
	if(w == image->getWidth() && h == image->getHeight()) return;
	if(((ipodart_t*)art)->resized) {
		image = ((ipodart_t*)art)->RegetImage();
		if(!image) return;
	}
	((ipodart_t*)art)->Resize(w, h);
}

void iPodDevice::getArtData(pmpart_t art, void* data){ // data ARGB32* is at natural size
	Image *image = ((ipodart_t*)art)->GetImage();
	if(!image) return;
	image->exportToARGB32((ARGB32*)data);
}

bool iPodDevice::artIsEqual(pmpart_t at, pmpart_t bt) {
	if(at == bt) return true;
	if(!at || !bt) return false;
	if(((ipodart_t*)at)->getWidth() != ((ipodart_t*)bt)->getWidth()) return false;
	if(((ipodart_t*)at)->getHeight() != ((ipodart_t*)bt)->getHeight()) return false;
	Image *a = ((ipodart_t*)at)->RegetImage();
	Image *b = ((ipodart_t*)bt)->RegetImage();
	if(!a && !b) return true;
	if(!a || !b) return false;
	if(a->getWidth() != b->getWidth()) return false;
	if(b->getHeight() != b->getHeight()) return false;
	return memcmp(a->getData(),b->getData(),a->getWidth()*a->getHeight()*sizeof(ARGB32)) == 0;
}

static INT_PTR CALLBACK gapscan_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	static iPodDevice * dev;
	static int i;
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowPos(hwndDlg,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
		dev = (iPodDevice*)lParam;
		i=0;
		dev->gapscanner = hwndDlg;
		SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_SETRANGE32,0,dev->getPlaylistLength(0));
		SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_SETPOS,0,0);
		SetTimer(hwndDlg,1,100,NULL);
		break;
	case WM_TIMER:
		if(wParam == 1) {
			KillTimer(hwndDlg,1);
			int l = dev->getPlaylistLength(0);
			int j=0;
			for(;;) {
				if(i >= l) return gapscan_dialogProc(hwndDlg,WM_CLOSE,0,0);
				iPod_mhit * mhit = (iPod_mhit *)dev->getPlaylistTrack(0,i++);
				if(!mhit->trackgapless && !mhit->gaplessData) {
					wchar_t artist[50] = {0}, title[50] = {0}, buf[200] = {0};
					dev->getTrackArtist((songid_t)mhit,artist,50);
					dev->getTrackTitle((songid_t)mhit,title,50);
					StringCchPrintf(buf,200,L"%d/%d: %s - %s",i+1,l,artist,title);
					SetDlgItemText(hwndDlg,IDC_CAPTION,buf);
					wchar_t infile[MAX_PATH]=L"";
					dev->getFilename(infile,MAX_PATH,(songid_t)mhit);
					BOOL worked=TRUE;
					mhit->samplecount = GetFileInfoInt64(infile,L"numsamples",&worked);
					mhit->pregap = (unsigned long)GetFileInfoInt64(infile,L"pregap",&worked);
					mhit->postgap = (unsigned long)GetFileInfoInt64(infile,L"postgap",&worked);
					mhit->gaplessData = (unsigned long)GetFileInfoInt64(infile,L"endoffset",&worked);
					mhit->trackgapless = worked?1:0;
					j++;
				} else if(!(i%23)) SetDlgItemText(hwndDlg,IDC_CAPTION,WASABI_API_LNGSTRINGW(IDS_SCANNING));
				if(j > 3 || !(i % 50)) {
					SendDlgItemMessage(hwndDlg,IDC_PROGRESS1,PBM_SETPOS,i,0);
					SetTimer(hwndDlg,1,25,NULL);
					return 0;
				}
			}
		}
		break;
	case WM_CLOSE:
		dev->writeiTunesDB();
		EndDialog(hwndDlg,0);
		dev->gapscanner = NULL;
		break;
	case WM_DESTROY:
		dev->gapscanner = NULL;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
	case IDCANCEL:
		gapscan_dialogProc(hwndDlg,WM_CLOSE,0,0);
		break;
	case IDC_BG:
		ShowWindow(hwndDlg,SW_HIDE);
		break;
		}
		break;
	}
	return 0;
}

static INT_PTR CALLBACK config_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	static iPodDevice * dev;
	switch(uMsg) {
		case WM_INITDIALOG:
			{
				prefsParam* p = (prefsParam*)lParam;
				p->config_tab_init(hwndDlg,p->parent);
				dev = (iPodDevice*)p->dev;
				if(dev->artdb && dev->thumbs.size()) {
					wchar_t inifile[] = {dev->driveW,L":\\iPod_Control\\iTunes\\ml_pmp.ini"};
					ShowWindow(GetDlgItem(hwndDlg,IDC_STATIC_ARTGROUP),SW_SHOWNA);
					ShowWindow(GetDlgItem(hwndDlg,IDC_CHECK_USEART),SW_SHOWNA);
					CheckDlgButton(hwndDlg,IDC_CHECK_USEART,GetPrivateProfileInt(L"ml_pmp",L"albumart",1,inifile));
					//ShowWindow(GetDlgItem(hwndDlg,IDC_COMBO_ARTMODE),SW_SHOWNA);
					/*ComboBox combo(hwndDlg,IDC_COMBO_USEART);
					combo.AddString(L"Add to all tracks");
					combo.AddString(L"Only add to the first track in an album");
					combo.AddString(L"Don't add to any tracks");
					*/
				}
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
		case IDC_SCAN:
			if(dev->gapscanner) ShowWindow(dev->gapscanner,SW_SHOW);
			else WASABI_API_DIALOGBOXPARAM(IDD_GAPSCAN,NULL,gapscan_dialogProc,(LPARAM)dev);
			break;
		case IDC_CHECK_USEART:
			wchar_t inifile[] = {dev->driveW,L":\\iPod_Control\\iTunes\\ml_pmp.ini"}, s[32] = {0};
			StringCchPrintf(s, 32, L"%d", (IsDlgButtonChecked(hwndDlg, IDC_CHECK_USEART)==BST_CHECKED));
			WritePrivateProfileString(L"ml_pmp", L"albumart", s, inifile);
			break;
			}
			break;
	}
	return 0;
}

static const intptr_t encoder_blacklist[] = 
{
	mmioFOURCC('W','M','A',' '),
	mmioFOURCC('A','A','C','H'),
	mmioFOURCC('A','A','C','P'),
	mmioFOURCC('A','A','C','r'),
	mmioFOURCC('F','L','A','C'),
	mmioFOURCC('O','G','G',' '),
	mmioFOURCC('M','P','2',' '),
	mmioFOURCC('M','4','A','H'),
	mmioFOURCC('M','4','A','+'),
	mmioFOURCC('A','D','T','S'),
};

intptr_t iPodDevice::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4) {
	switch(param1) {
		case DEVICE_SET_ICON: // icons
			{
				MLTREEIMAGE * i = (MLTREEIMAGE*)param2;
				i->hinst = plugin.hDllInstance;
				i->resourceId = image16;
			}
			break;
		case DEVICE_GET_ICON:
			{
				if (param2 <= 16 && param3 <= 16)
				{
					// TODO: get the name of the DLL at load time 
					StringCchPrintfW((wchar_t *)param4, 260, L"res://%s/PNG/#%u", L"pmp_ipod.dll", image16);
				}
				else
				{
					// TODO: get the name of the DLL at load time 
					StringCchPrintfW((wchar_t *)param4, 260, L"res://%s/PNG/#%u", L"pmp_ipod.dll", image160);
				}
			}
			break;
		case DEVICE_SUPPORTED_METADATA:
			return 0xffff | (artdb?SUPPORTS_ALBUMART:0);
		case DEVICE_CAN_RENAME_DEVICE:
			return 1;
		case DEVICE_GET_INI_FILE:
			{
				wchar_t inifile[] = {driveW,L":\\iPod_Control\\iTunes\\ml_pmp.ini"};
				wcsncpy((wchar_t*)param2,inifile,MAX_PATH);
			}
			break;
		case DEVICE_GET_PREFS_DIALOG:
			if(param3 == 0) {
				pref_tab * p = (pref_tab *)param2;
				p->hinst = WASABI_API_LNG_HINST;
				p->dlg_proc = config_dialogProc;
				p->res_id = IDD_CONFIG;
				WASABI_API_LNGSTRINGW_BUF(IDS_ADVANCED,p->title,100);
			}
			break;
		case DEVICE_REFRESH:
			{
				char drive = this->drive;
				
				//iPods.eraseObject(this);
				auto it = std::find(iPods.begin(), iPods.end(), this);
				if (it != iPods.end())
				{
					iPods.erase(it);
				}

				SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
				delete this;
				new iPodDevice(drive);
			}
			break;
		case DEVICE_ADDPODCASTGROUP:
			{
				int pos = param3;
				wchar_t * name = (wchar_t *)param4;
				iPod_mhyp* pl = (iPod_mhyp*)playlists.Get(param2);
				pl->podcastflag=1;
				iPod_mhip * mhip = new iPod_mhip();
				mhip->podcastgroupflag=256;
				mhip->podcastgroupref=0;
				iPod_mhod * d = new iPod_mhod();
				d->SetString(name);
				d->type=1;
				mhip->mhod.push_back(d);
				pl->mhip.insert(pl->mhip.begin()+pos,mhip);
			}
			break;
		case DEVICE_ADDPODCASTGROUP_FINISH:
			{
				iPod_mhyp* pl = (iPod_mhyp*)playlists.Get(param2);
				pl->numLibraryMHODs=0x18;
				int groupref=0;
				for(size_t i=0; i < pl->mhip.size(); i++) {
					iPod_mhip * m = pl->mhip[i];
					m->groupid = i+1000000;
					if(m->podcastgroupflag & 256) groupref = m->groupid;
					else m->podcastgroupref = groupref;
				}
			}
			break;
		case DEVICE_SUPPORTS_VIDEO:
			return 1;
		case DEVICE_VETO_ENCODER:
			{
				for (size_t i=0;i<sizeof(encoder_blacklist)/sizeof(*encoder_blacklist);i++)
				{
					// TODO: check device info XML for aacPlus support
					if (param2 == encoder_blacklist[i])
						return 1;
				}
			}
			return 0;
		case DEVICE_GET_MODEL:
			{
				wchar_t *model_buffer = (wchar_t *)param2;
				unsigned int cch = (unsigned int)param3;
				if (!info)
				{
					StringCchCopyW(model_buffer, cch, L"Apple iPod");
				}
				else switch(info->family_id)
				{
					default:
						StringCchCopyW(model_buffer, cch, L"Apple iPod"); 
						break;
					case 3:
						StringCchCopyW(model_buffer, cch, L"Apple iPod Mini"); 
						break;
					case 4:
						StringCchCopyW(model_buffer, cch, L"Apple iPod 4G"); 
						break;
					case 5:
						StringCchCopyW(model_buffer, cch, L"Apple iPod Photo"); 
						break;
					case 6:
						StringCchCopyW(model_buffer, cch, L"Apple iPod 5G"); 
						break;
					case 7:
						StringCchCopyW(model_buffer, cch, L"Apple iPod Nano 1G"); 
						break;
					case 9:
						StringCchCopyW(model_buffer, cch, L"Apple iPod Nano 2G"); 
						break;
					case 11:
						StringCchCopyW(model_buffer, cch, L"Apple iPod Classic"); 
						break;
					case 12:
						StringCchCopyW(model_buffer, cch, L"Apple iPod Fat Nano"); 
						break;
					case 15:
						StringCchCopyW(model_buffer, cch, L"Apple iPod Nano4G"); 
						break;
					case 128:
						StringCchCopyW(model_buffer, cch, L"Apple iPod Shuffle"); 
						break;
					case 130:
						StringCchCopyW(model_buffer, cch, L"Apple iPod Shuffle 2G"); 
						break;
					case 132:
						StringCchCopyW(model_buffer, cch, L"Apple iPod Shuffle 3G"); 
						break;
				}

			}
			return 1;
	}
	return 0;
}