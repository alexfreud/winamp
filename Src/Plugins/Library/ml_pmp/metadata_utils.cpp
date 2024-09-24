#include "main.h"
#include "DeviceView.h"
#include "metadata_utils.h"
#include "../nu/sort.h"
#include <shlwapi.h>
#include <strsafe.h>
#include "api__ml_pmp.h"

#define METADATASTRATAGYSWITCH 500

filenameMap **filenameMapping;
int filenameMapLen;
static INT_PTR CALLBACK findingMetadata2_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	static int i;
	static int added;
	switch(uMsg) {
		case WM_INITDIALOG:
			SendDlgItemMessage(hwndDlg, IDC_METADATAPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, filenameMapLen));
			i=0;
			added=0;
			SetTimer(hwndDlg, 1, 10, NULL);

			if (FALSE != CenterWindow(hwndDlg, (HWND)lParam))
				SendMessage(hwndDlg, DM_REPOSITION, 0, 0L);

			SetForegroundWindow(hwndDlg);
			return 0;

		case WM_TIMER:
			if(wParam == 1) {
				KillTimer(hwndDlg, 1);
				filenameMap **map = filenameMapping;
				int len = filenameMapLen;
				for(; i < len; i++) {
					if (i % 25 == 0) SendDlgItemMessage(hwndDlg, IDC_METADATAPROGRESS, PBM_SETPOS, i, 0);

					itemRecordW *result = AGAVE_API_MLDB->GetFile(map[i]->fn);
					map[i]->ice = (itemRecordW*)calloc(sizeof(itemRecordW), 1);
					if (result) {
						copyRecord(map[i]->ice, result);
						AGAVE_API_MLDB->FreeRecord(result);

						if(i % 200 == 0) {
							i++;
							PostMessage(hwndDlg, WM_TIMER, 1, 0);
							return 0;
						}
					} else {
						filenameToItemRecord(map[i]->fn, map[i]->ice); // ugh. Disk intensive.
						SendMessage(plugin.hwndWinampParent, WM_ML_IPC, (WPARAM)map[i]->ice, ML_IPC_DB_ADDORUPDATEITEMW);
						added++;
						i++;
						PostMessage(hwndDlg, WM_TIMER, 1, 0);
						return 0;
					}
				}
				if (added) SendMessage(plugin.hwndWinampParent,WM_ML_IPC,0,ML_IPC_DB_SYNCDB);
				EndDialog(hwndDlg,0);
			}
			break;
	}
	return 0;
}

void mapFilesToItemRecords(filenameMap ** map0, int len, HWND centerWindow) {
	filenameMapping = map0;
	filenameMapLen = len;
	if (filenameMapLen  > 0)
		WASABI_API_DIALOGBOXPARAMW(IDD_GETTINGMETADATA,plugin.hwndWinampParent, findingMetadata2_dlgproc, (LPARAM)centerWindow);
}

C_ItemList * fileListToItemRecords(wchar_t** files,int l, HWND centerWindow) {
	filenameMap ** map = (filenameMap **)calloc(l, sizeof(void*));
	filenameMap * m = (filenameMap *)calloc(l, sizeof(filenameMap));
	for(int i=0; i<l; i++) {
		map[i] = &m[i];
		map[i]->fn = files[i];
	}

	mapFilesToItemRecords(map, l, centerWindow);

	C_ItemList * out = new C_ItemList;
	for(int i=0; i<l; i++)
		out->Add(map[i]->ice);
	free(m);
	free(map);
	return out;
}

C_ItemList * fileListToItemRecords(C_ItemList * fileList, HWND centerWindow) {
	return fileListToItemRecords((wchar_t**)fileList->GetAll(),fileList->GetSize(), centerWindow);
}

typedef struct
{
	songid_t songid;
	Device * dev;
} SortSongItem;

#define RETIFNZ(v) { int zz = (v); if(zz) return zz; }
#define CMPFIELDS(x) { x(a,bufa,256); x(b,bufb,256); int v = lstrcmpiW(bufa,bufb); if(v) return v; }

int __fastcall compareSongs(const void *elem1, const void *elem2, const void *context) {
	songid_t a = *(songid_t*)elem1;
	songid_t b = *(songid_t*)elem2;
	if(a == b) return 0;
	Device * dev = (Device *)context;
	wchar_t bufa[256] = {0};
	wchar_t bufb[256] = {0};
	CMPFIELDS(dev->getTrackArtist)
	CMPFIELDS(dev->getTrackAlbum)
	CMPFIELDS(dev->getTrackTitle)
	int t1 = dev->getTrackTrackNum(a);
	int t2 = dev->getTrackTrackNum(b);
	if(t1>0 && t2>0) RETIFNZ(t1 - t2)
	return 0;
}

#undef CMPFIELDS

static __forceinline int strcmp_nullok(wchar_t * x,wchar_t * y) {
	if(!x) x=L"";
	if(!y) y=L"";
	return lstrcmpiW(x,y);
}

int compareItemRecordAndSongId(itemRecordW * item, songid_t song, Device *dev) 
{
	wchar_t buf[2048] = {0};
	dev->getTrackArtist(song,buf,sizeof(buf)/sizeof(wchar_t));
	RETIFNZ(strcmp_nullok(buf,item->artist));
	dev->getTrackAlbum(song,buf,sizeof(buf)/sizeof(wchar_t));
	RETIFNZ(strcmp_nullok(buf,item->album));
	dev->getTrackTitle(song,buf,sizeof(buf)/sizeof(wchar_t));
	RETIFNZ(strcmp_nullok(buf,item->title));
	int t = dev->getTrackTrackNum(song);
	if(item->track>0 && t>0) RETIFNZ(t - item->track);
	return 0;
}

int compareItemRecords(itemRecordW * a, itemRecordW * b) {
	if(a == b) return 0;
	RETIFNZ(lstrcmpiW(a->artist?a->artist:L"",b->artist?b->artist:L""));
	RETIFNZ(lstrcmpiW(a->album?a->album:L"",b->album?b->album:L""));
	RETIFNZ(lstrcmpiW(a->title?a->title:L"",b->title?b->title:L""));
	if(a->track>0 && b->track>0) RETIFNZ(a->track - b->track);
	return 0;
}

static int sortfunc_ItemRecords_map(const void *elem1, const void *elem2) {
	PlaylistAddItem *a = *(PlaylistAddItem **)elem1;
	PlaylistAddItem *b = *(PlaylistAddItem **)elem2;
	return compareItemRecords(a->item,b->item);
}

static int sortfunc_ItemRecords(const void *elem1, const void *elem2) {
	itemRecordW *a = *(itemRecordW **)elem1;
	itemRecordW *b = *(itemRecordW **)elem2;
	return compareItemRecords(a,b);
}

#undef RETIFNZ

/* Gay Venn Diagram(tm) explaining ProcessDatabaseDifferences. Leave arguments NULL if not required
       ml     device
    /------\ /------\   
   /        X        \   
  /        / \<-------\---songsInML and itemRecordsOnDevice
  \        \ /        /   
   \        X        /
    \------/ \------/
       ^          ^--------songsNotInML
       |--itemRecordsNotOnDevice 
*/
// Runs in O(nlogn) of largest list
void ProcessDatabaseDifferences(Device * dev, C_ItemList * ml0,C_ItemList * itemRecordsOnDevice, C_ItemList * itemRecordsNotOnDevice, C_ItemList * songsInML,  C_ItemList * songsNotInML) {
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

void MapItemRecordsToSongs(Device * dev, PlaylistAddItem ** map, int len, C_ItemList * itemRecordsNotOnDevice) {
	C_ItemList device;
	int l = dev->getPlaylistLength(0);
	int i;
	for(i=0; i<l; i++) device.Add((void*)dev->getPlaylistTrack(0,i));

	qsort(map,len,sizeof(void*),sortfunc_ItemRecords_map);
	nu::qsort(device.GetAll(),device.GetSize(),sizeof(void*),dev,compareSongs);

	int j=0;
	i=0;
	int li = device.GetSize();
	int lj = len;
	while(i<li && j<lj) {
		PlaylistAddItem* p = map[j];
		songid_t s = (songid_t)device.Get(i);
		int cmp = compareItemRecordAndSongId(p->item,s, dev);
		if(cmp == 0) {
			p->songid = s;
			j++;
		}
		else if(cmp > 0) { j++; if(itemRecordsNotOnDevice) itemRecordsNotOnDevice->Add(p->item); }
		else i++;
	}
}

void ProcessDatabaseDifferences(Device * dev, itemRecordListW * ml,C_ItemList * itemRecordsOnDevice, C_ItemList * itemRecordsNotOnDevice, C_ItemList * songsInML,  C_ItemList * songsNotInML) {
	if(!ml) return;
	C_ItemList ml_list;
	for(int i=0; i < ml->Size; i++) ml_list.Add(&ml->Items[i]);
	ProcessDatabaseDifferences(dev,&ml_list,itemRecordsOnDevice,itemRecordsNotOnDevice,songsInML,songsNotInML);
}

typedef struct { songid_t song; Device * dev; const wchar_t * filename; } tagItem;

static wchar_t * tagFunc(const wchar_t * tag, void * p) { //return 0 if not found, -1 for empty tag
	tagItem * s = (tagItem *)p;
	int len = 2048;
	wchar_t * buf = (wchar_t *)calloc(len, sizeof(wchar_t));
	if (buf)
	{
		if (!_wcsicmp(tag, L"artist"))	s->dev->getTrackArtist(s->song,buf,len);
		else if (!_wcsicmp(tag, L"album"))	s->dev->getTrackAlbum(s->song,buf,len);
		else if (!_wcsicmp(tag, L"title"))	s->dev->getTrackTitle(s->song,buf,len);
		else if (!_wcsicmp(tag, L"genre"))	s->dev->getTrackGenre(s->song,buf,len);
		else if (!_wcsicmp(tag, L"year"))	wsprintf(buf,L"%d",s->dev->getTrackYear(s->song));
		else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track"))	wsprintf(buf,L"%d",s->dev->getTrackTrackNum(s->song));
		else if (!_wcsicmp(tag, L"discnumber"))	wsprintf(buf,L"%d",s->dev->getTrackDiscNum(s->song));
		else if (!_wcsicmp(tag, L"bitrate"))	wsprintf(buf,L"%d",s->dev->getTrackBitrate(s->song));
		else if (!_wcsicmp(tag, L"filename"))	lstrcpyn(buf,s->filename,len);
		else if (!_wcsicmp(tag, L"albumartist"))	s->dev->getTrackAlbumArtist(s->song,buf,len);
		else if (!_wcsicmp(tag, L"composer"))	s->dev->getTrackComposer(s->song,buf,len);
		else if (!_wcsicmp(tag, L"publisher"))	s->dev->getTrackPublisher(s->song,buf,len);
		else if (!_wcsicmp(tag, L"mime"))	s->dev->getTrackMimeType(s->song,buf,len);
	}
	return buf;
}

static void tagFreeFunc(wchar_t *tag, void *p) { if(tag) free(tag); }

static time_t FileTimeToUnixTime(FILETIME *ft)
{
  ULARGE_INTEGER end;
  memcpy(&end,ft,sizeof(end));
  end.QuadPart -= 116444736000000000;
  end.QuadPart /= 10000000; // 100ns -> seconds
  return (time_t)end.QuadPart;
}

static __int64 FileSize64(HANDLE file)
{
	LARGE_INTEGER position;
	position.QuadPart=0;
	position.LowPart = GetFileSize(file, (LPDWORD)&position.HighPart); 	
	
	if (position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		return INVALID_FILE_SIZE;
	else
		return position.QuadPart;
}

void GetFileSizeAndTime(const wchar_t *filename, __int64 *file_size, time_t *file_time)
{
	WIN32_FILE_ATTRIBUTE_DATA file_data;
	if (GetFileAttributesExW(filename, GetFileExInfoStandard, &file_data) == FALSE)
	{
		// GetFileAttributesEx failed. that sucks, let's try something else
		HANDLE hFile=CreateFileW(filename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			FILETIME lt;
			if (GetFileTime(hFile,NULL,NULL,&lt))
			{
				*file_time=FileTimeToUnixTime(&lt);
			}
			*file_size=FileSize64(hFile);
			CloseHandle(hFile);
		}
	}
	else
	{
		// success
		*file_time = FileTimeToUnixTime(&file_data.ftLastWriteTime);
		LARGE_INTEGER size64;
		size64.LowPart = file_data.nFileSizeLow;
		size64.HighPart = file_data.nFileSizeHigh;
		*file_size = size64.QuadPart;
	}
}

void getTitle(Device * dev, songid_t song, const wchar_t * filename,wchar_t * buf, int len) {
	buf[0]=0; buf[len-1]=0;
	tagItem item = {song,dev,filename};
	waFormatTitleExtended fmt={filename,0,NULL,&item,buf,len,tagFunc,tagFreeFunc};
	SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&fmt, IPC_FORMAT_TITLE_EXTENDED);
}

#define atoi_NULLOK(s) ((s)?_wtoi(s):0)

void filenameToItemRecord(wchar_t * file, itemRecordW * ice)
{
	int gtrack=0;
	wchar_t *gartist=NULL,*galbum=NULL,*gtitle=NULL;
	wchar_t *guessbuf = guessTitles(file,&gtrack,&gartist,&galbum,&gtitle);
	if(!gartist) gartist=L"";
	if(!galbum) galbum=L"";
	if(!gtitle) gtitle=L"";

	wchar_t buf[512]=L"";
	extendedFileInfoStructW efs={file,NULL,buf,512};

	efs.metadata=L"title"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	if(buf[0]) { ice->title=_wcsdup(buf); gartist=L""; galbum=L""; gtrack=-1;}
	else ice->title=_wcsdup(gtitle);

	efs.metadata=L"album";
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	if(buf[0]) ice->album=_wcsdup(buf);
	else ice->album=_wcsdup(galbum);

	efs.metadata=L"artist"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	if(buf[0]) ice->artist=_wcsdup(buf);
	else ice->artist=_wcsdup(gartist);

	efs.metadata=L"comment"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->comment=_wcsdup(buf);

	efs.metadata=L"genre"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->genre=_wcsdup(buf);

	efs.metadata=L"albumartist"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->albumartist=_wcsdup(buf);

	efs.metadata=L"replaygain_album_gain"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->replaygain_album_gain=_wcsdup(buf);

	efs.metadata=L"replaygain_track_gain"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->replaygain_track_gain=_wcsdup(buf);

	efs.metadata=L"publisher"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->publisher=_wcsdup(buf);

	efs.metadata=L"composer"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->composer=_wcsdup(buf);

	efs.metadata=L"year"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->year=atoi_NULLOK(buf);

	efs.metadata=L"track"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	if(buf[0]) ice->track=atoi_NULLOK(buf);
	else ice->track=gtrack;

	efs.metadata=L"tracks"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->tracks=atoi_NULLOK(buf);

	efs.metadata=L"rating"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->rating=atoi_NULLOK(buf);

	__int64 file_size=INVALID_FILE_SIZE;
	time_t file_time=0;
	GetFileSizeAndTime(file, &file_size, &file_time);
	if (!(file_size == INVALID_FILE_SIZE || file_size == 0))
	{
		ice->filetime=file_time;
		// scales to the kb value this uses
		ice->filesize=(int)(file_size/1024);
		// and since 5.64+ we can also return this as a true value
		StringCchPrintf(buf, sizeof(buf), L"%d", file_size);
		setRecordExtendedItem(ice,L"realsize",buf);
	}

	ice->lastupd=time(NULL);

	efs.metadata=L"bitrate"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->bitrate=atoi_NULLOK(buf);

	efs.metadata=L"type"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->type=atoi_NULLOK(buf);

	efs.metadata=L"disc"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->disc=atoi_NULLOK(buf);

	efs.metadata=L"discs"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->discs=atoi_NULLOK(buf);

	efs.metadata=L"bpm"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	ice->bpm=atoi_NULLOK(buf);

	basicFileInfoStructW b={efs.filename,0};
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&b,IPC_GET_BASIC_FILE_INFOW);
	ice->length=b.length;

	// additional fields to match (if available) with a full library
	// response this is mainly for improving the cloud compatibility
	efs.metadata=L"lossless"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	if(buf[0]) setRecordExtendedItem(ice,L"lossless",buf);

	efs.metadata=L"director"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	if(buf[0]) setRecordExtendedItem(ice,L"director",buf);

	efs.metadata=L"producer"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	if(buf[0]) setRecordExtendedItem(ice,L"producer",buf);

	efs.metadata=L"width"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	if(buf[0]) setRecordExtendedItem(ice,L"width",buf);

	efs.metadata=L"height"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	if(buf[0]) setRecordExtendedItem(ice,L"height",buf);

	efs.metadata=L"mime"; buf[0]=0;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&efs,IPC_GET_EXTENDED_FILE_INFOW);
	if(buf[0]) setRecordExtendedItem(ice,L"mime",buf);

	// Not filled in are: playcount, lastplay
	ice->filename = _wcsdup(file);
	free(guessbuf);
}

void copyTags(itemRecordW * in, wchar_t * out) {
	// check if the old file still exists - if it does, we will let Winamp copy metadata for us
	if (wcscmp(in->filename, out) && PathFileExists(in->filename))
	{
		copyFileInfoStructW copy;
		copy.dest = out;
		copy.source = in->filename;
		if(SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&copy, IPC_COPY_EXTENDED_FILE_INFOW) == 0) // 0 means success
			return;
	}
}