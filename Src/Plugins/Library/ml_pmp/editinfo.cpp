#include "main.h"
#include "DeviceView.h"
#include "api__ml_pmp.h"
#include <api/service/waServiceFactory.h>
#include <api/service/svcs/svc_imgload.h>
#include <api/service/svcs/svc_imgwrite.h>
#include <api/memmgr/api_memmgr.h>
#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/bltcanvas.h>

extern C_ItemList devices;
extern HWND hwndMediaView;

static C_ItemList * editItems;
static Device * editDevice;

typedef struct {
	int w;
	int h;
	ARGB32 * data;
} editinfo_image;

static INT_PTR CALLBACK editInfo_commit_dialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	static int i;
	switch(uMsg) {
		case WM_INITDIALOG:
			i=0;
			SetWindowText(hwnd,WASABI_API_LNGSTRINGW(IDS_SETTING_METADATA));
			SendDlgItemMessage(hwnd,IDC_PROGRESS,PBM_SETRANGE,0,MAKELPARAM(0, editItems->GetSize()));
			SetTimer(hwnd,1,5,NULL);

			if (FALSE != CenterWindow(hwnd, (HWND)lParam))
			SendMessage(hwnd, DM_REPOSITION, 0, 0L);

			break;
		case WM_TIMER:
			if(wParam == 1) {
				KillTimer(hwnd,1);
				HWND hwndDlg = GetParent(hwnd);
				editinfo_image * image = (editinfo_image *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				SendDlgItemMessage(hwnd,IDC_PROGRESS,PBM_SETPOS,i,0);
				if(i < editItems->GetSize()) {
					int metadata_edited=0;
					songid_t song=(songid_t)editItems->Get(i);
					time_t t; time(&t);
					editDevice->setTrackLastUpdated(song,t);
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_ARTIST)) {
						wchar_t blah[512] = {0};
						GetDlgItemText(hwndDlg,IDC_EDIT_ARTIST,blah,sizeof(blah)/sizeof(wchar_t)-1);
						blah[sizeof(blah)/sizeof(wchar_t)-1]=0;
						editDevice->setTrackArtist(song,blah);
						metadata_edited++;
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_TITLE)) {
						wchar_t blah[512] = {0};
						GetDlgItemText(hwndDlg,IDC_EDIT_TITLE,blah,sizeof(blah)/sizeof(wchar_t)-1);
						blah[sizeof(blah)/sizeof(wchar_t)-1]=0;
						editDevice->setTrackTitle(song,blah);
						metadata_edited++;
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_ALBUM)) {
						wchar_t blah[512] = {0};
						GetDlgItemText(hwndDlg,IDC_EDIT_ALBUM,blah,sizeof(blah)/sizeof(wchar_t)-1);
						blah[sizeof(blah)/sizeof(wchar_t)-1]=0;
						editDevice->setTrackAlbum(song,blah);
						metadata_edited++;
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_GENRE)) {
						wchar_t blah[512] = {0};
						GetDlgItemText(hwndDlg,IDC_EDIT_GENRE,blah,sizeof(blah)/sizeof(wchar_t)-1);
						blah[sizeof(blah)/sizeof(wchar_t)-1]=0;
						editDevice->setTrackGenre(song,blah);
						metadata_edited++;
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_TRACK)) {
						wchar_t blah[64] = {0};
						GetDlgItemText(hwndDlg,IDC_EDIT_TRACK,blah,sizeof(blah)/sizeof(wchar_t)-1);
						blah[sizeof(blah)/sizeof(wchar_t)-1]=0;
						int n=_wtoi(blah);
						if (n <= 0) n=-1;
						editDevice->setTrackTrackNum(song,n);
						metadata_edited++;
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_DISC)) {
						wchar_t blah[64] = {0};
						GetDlgItemText(hwndDlg,IDC_EDIT_DISC,blah,sizeof(blah)/sizeof(wchar_t)-1);
						blah[sizeof(blah)/sizeof(wchar_t)-1]=0;
						int n=_wtoi(blah);
						if (n <= 0) n=-1;
						editDevice->setTrackDiscNum(song,n);
						metadata_edited++;
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_YEAR)) {
						wchar_t blah[64] = {0};
						GetDlgItemText(hwndDlg,IDC_EDIT_YEAR,blah,sizeof(blah)/sizeof(wchar_t)-1);
						blah[sizeof(blah)/sizeof(wchar_t)-1]=0;
						int n=_wtoi(blah);
						if (n <= 0) n=-1;
						editDevice->setTrackYear(song,n);
						metadata_edited++;
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_ALBUMARTIST)) {
						wchar_t blah[512] = {0};
						GetDlgItemText(hwndDlg,IDC_EDIT_ALBUMARTIST,blah,sizeof(blah)/sizeof(wchar_t)-1);
						blah[sizeof(blah)/sizeof(wchar_t)-1]=0;
						editDevice->setTrackAlbumArtist(song,blah);
						metadata_edited++;
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_PUBLISHER)) {
						wchar_t blah[512] = {0};
						GetDlgItemText(hwndDlg,IDC_EDIT_PUBLISHER,blah,sizeof(blah)/sizeof(wchar_t)-1);
						blah[sizeof(blah)/sizeof(wchar_t)-1]=0;
						editDevice->setTrackPublisher(song,blah);
						metadata_edited++;
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_COMPOSER)) {
						wchar_t blah[512] = {0};
						GetDlgItemText(hwndDlg,IDC_EDIT_COMPOSER,blah,sizeof(blah)/sizeof(wchar_t)-1);
						blah[sizeof(blah)/sizeof(wchar_t)-1]=0;
						editDevice->setTrackComposer(song,blah);
						metadata_edited++;
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_CHECK_ALBUMART)) {
						if(!image) editDevice->setArt(song,NULL,0,0);
						else editDevice->setArt(song,image->data,image->w,image->h);
					}

					if (metadata_edited)
					{
						editDevice->extraActions(DEVICE_DONE_SETTING, (intptr_t)song,0,0);
					}
					SetTimer(hwnd,1,5,NULL);
				}
				else {
					editDevice->commitChanges();
					EndDialog(hwnd,0);
				}
				i++;
			}
			break;
		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_ABORT) {
				editDevice->commitChanges();
				EndDialog(hwnd,-1);
			}
			break;
	}
	return 0;
}

static ARGB32 * loadImg(const void * data, int len, int *w, int *h) {
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = plugin.service->service_getNumServices(imgload);
	for(int i=0; i<n; i++) {
		waServiceFactory *sf = plugin.service->service_enumService(imgload,i);
		if(sf) {
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if(l) {
				if(l->testData(data,len)) {
					ARGB32* ret = l->loadImage(data,len,w,h);
					sf->releaseInterface(l);
					return ret;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

static void * loadFile(const wchar_t * file, int &len) {	
	len=0;
	FILE * f = _wfopen(file,L"rb");
	if(!f) return 0;
	fseek(f,0,2);
	len = ftell(f);
	if(!len) {fclose(f); return 0;}
	fseek(f,0,0);
	void * data = calloc(len, sizeof(void*));
	fread(data,len,1,f);
	fclose(f);
	return data;
}

static ARGB32 * loadImgFromFile(const wchar_t * file, int *w, int *h) {
	int len;
	void * d = loadFile(file,len);
	if(!d) return 0;
	ARGB32 * im = loadImg(d,len,w,h);
	free(d);
	return im;
}

static void * writeImg(const ARGB32 *data, int w, int h, int *length, const wchar_t *ext) {
	if(!ext || !*ext) return NULL;
	if(*ext == L'.') ext++;
	FOURCC imgwrite = svc_imageWriter::getServiceType();
	int n = plugin.service->service_getNumServices(imgwrite);
	for(int i=0; i<n; i++) {
		waServiceFactory *sf = plugin.service->service_enumService(imgwrite,i);
		if(sf) {
			svc_imageWriter * l = (svc_imageWriter*)sf->getInterface();
			if(l) {
				if(wcsstr(l->getExtensions(),ext)) {
					void* ret = l->convert(data,32,w,h,length);
					sf->releaseInterface(l);
					return ret;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

static void writeFile(const wchar_t *file, void * data, int length) {
	FILE *f = _wfopen(file,L"wb");
	if(!f) return;
	fwrite(data,length,1,f);
	fclose(f);
}

static void writeImageToFile(ARGB32 * img, int w, int h, const wchar_t *file) {
	int length=0;
	void * data = writeImg(img,w,h,&length,wcsrchr(file,L'.'));
	if(data) {
		writeFile(file,data,length);
		WASABI_API_MEMMGR->sysFree(data);
	}
}

static void enableArt(HWND hwndDlg, bool combo, BOOL enable) {
	if(combo) EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_ALBUMART),enable);
	EnableWindow(GetDlgItem(hwndDlg,IDC_ARTINFO),enable);
	EnableWindow(GetDlgItem(hwndDlg,IDC_PICTUREHOLDER),enable);
	EnableWindow(GetDlgItem(hwndDlg,IDC_ART_CHANGE),enable);
	EnableWindow(GetDlgItem(hwndDlg,IDC_ART_CLEAR),enable);
}

static int checkEditInfoClick(HWND hwndDlg, POINT p, int item, int check, bool art=false) {
	if(!IsWindowEnabled(GetDlgItem(hwndDlg,check))) return 0;
	RECT r;
	GetWindowRect(GetDlgItem(hwndDlg, item), &r);
	ScreenToClient(hwndDlg, (LPPOINT)&r);
	ScreenToClient(hwndDlg, (LPPOINT)&r.right);
	if (PtInRect(&r, p) && !IsDlgButtonChecked(hwndDlg, check)) {
		CheckDlgButton(hwndDlg, check, TRUE);
		if(art) enableArt(hwndDlg,false,TRUE);
		else EnableWindow(GetDlgItem(hwndDlg, item), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
		PostMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, item), TRUE);
		return 1;
	}
	return 0;
}

static HBITMAP getBitmap(const editinfo_image * image, HWND parent) {
	BITMAPINFO info={0};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = image->w;
	info.bmiHeader.biHeight = -image->h;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	HDC dc = GetDC(parent);
	HBITMAP bm = CreateCompatibleBitmap(dc,image->w,image->h);
	SetDIBits(dc,bm,0,image->h,image->data,&info,DIB_RGB_COLORS);
	ReleaseDC(parent,dc);
	return bm;
}

static HBITMAP getBitmap(pmpart_t art, Device * dev, HWND parent) {
	int w,h;
	editDevice->getArtNaturalSize(art,&w,&h);
	if(w == 0 || h == 0) return NULL;
	ARGB32 * bits = (ARGB32 *)calloc(w * h, sizeof(ARGB32));
	if(!bits) return NULL;
	editDevice->getArtData(art,bits);
	editinfo_image im={w,h,bits};
	HBITMAP bm = getBitmap(&im,parent);
	free(bits);
	return bm;
}

static void setBitmap(const editinfo_image * image, HWND hwndDlg, bool init=false) {
	if(!init) {
		CheckDlgButton(hwndDlg,IDC_CHECK_ALBUMART,BST_CHECKED);
		enableArt(hwndDlg,false,TRUE);
		SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_ART_CLEAR,0),0);
	}
	HQSkinBitmap temp(image->data, image->w, image->h); // wrap into a SkinBitmap (no copying involved)
	BltCanvas newImage(90,90);
	temp.stretch(&newImage, 0, 0, 90, 90);
	editinfo_image i = {90,90,(ARGB32*)newImage.getBits()};
	HBITMAP bm = getBitmap(&i,hwndDlg);
	HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_PICTUREHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bm);
	if(bmold) DeleteObject(bmold);
	SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG_PTR)image);
	wchar_t buf[100] = {0};
	wsprintf(buf,L"%dx%d",image->w,image->h);
	SetDlgItemText(hwndDlg,IDC_ARTINFO,buf);
}

static void GetSize(HBITMAP bm,int &w, int &h,HWND hwndDlg) {
	HDC dc = GetDC(hwndDlg);
	BITMAPINFO info={0};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(dc,bm,0,0,NULL,&info,DIB_RGB_COLORS);
	w = abs(info.bmiHeader.biWidth);
	h = abs(info.bmiHeader.biHeight);
	ReleaseDC(hwndDlg,dc);
}

static void setBitmap(HBITMAP bm, HWND hwndDlg) {
	editinfo_image* image = (editinfo_image*)calloc(1, sizeof(editinfo_image));
	GetSize(bm,image->w,image->h,hwndDlg);
	image->data = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(sizeof(ARGB32)*image->w*image->h);
	BITMAPINFO info={0};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = image->w;
	info.bmiHeader.biHeight = -image->h;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	HDC dc = GetDC(hwndDlg);
	GetDIBits(dc,bm,0,image->h,image->data,&info,DIB_RGB_COLORS);
	ReleaseDC(hwndDlg,dc);
	setBitmap(image,hwndDlg);
}

static INT_PTR CALLBACK editInfo_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
		{
			wchar_t last_artist[2048]=L"", last_title[2048]=L"", last_album[2048]=L"", last_genre[2048]=L"", last_albumartist[2048]=L"", last_publisher[2048]=L"", last_composer[2048]=L"";
			pmpart_t last_albumart=NULL;
			int last_year=-1, last_track=-1, last_disc=-1;
			bool disable_artist=0, disable_title=0, disable_album=0, disable_genre=0, disable_year=0, disable_track=0, disable_disc=0, disable_albumartist=0, disable_publisher=0, disable_composer=0, disable_albumart=0;
			int fieldBits = (int)editDevice->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
			if(!fieldBits) fieldBits = -1;
			disable_artist = (fieldBits & SUPPORTS_ARTIST)==0;
			disable_title = (fieldBits & SUPPORTS_TITLE)==0;
			disable_album = (fieldBits & SUPPORTS_ALBUM)==0;
			disable_genre = (fieldBits & SUPPORTS_GENRE)==0;
			disable_year = (fieldBits & SUPPORTS_YEAR)==0;
			disable_track = (fieldBits & SUPPORTS_TRACKNUM)==0;
			disable_disc = (fieldBits & SUPPORTS_DISCNUM)==0;
			disable_albumartist = (fieldBits & SUPPORTS_ALBUMARTIST)==0;
			disable_publisher = (fieldBits & SUPPORTS_PUBLISHER)==0;
			disable_composer = (fieldBits & SUPPORTS_COMPOSER)==0;
			disable_albumart = (fieldBits & SUPPORTS_ALBUMART)==0;

			EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_ARTIST),!disable_artist);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_TITLE),!disable_title);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_ALBUM),!disable_album);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_GENRE),!disable_genre);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_YEAR),!disable_year);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_TRACK),!disable_track);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DISC),!disable_disc);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_ALBUMARTIST),!disable_albumartist);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_PUBLISHER),!disable_publisher);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_COMPOSER),!disable_composer);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_ALBUMART),!disable_albumart);
			//enableArt(hwndDlg,true,!disable_albumart);

			int l=editItems->GetSize();
			for(int i=0;i<l;i++) {
				wchar_t buf[2048]=L"";
				songid_t song=(songid_t)editItems->Get(i);
				if(!disable_artist)
				{
					buf[0]=0;
					editDevice->getTrackArtist(song,buf,sizeof(buf)/sizeof(wchar_t));
					if(!buf[0]);
					else if(!last_artist[0]) lstrcpyn(last_artist,buf,2048);
					else if(lstrcmp(buf,last_artist)) disable_artist=true;
				}
				if(!disable_albumartist)
				{
					buf[0]=0;
					editDevice->getTrackAlbumArtist(song,buf,sizeof(buf)/sizeof(wchar_t));
					if(!buf[0]);
					else if(!last_albumartist[0]) lstrcpyn(last_albumartist,buf,2048);
					else if(lstrcmp(buf,last_albumartist)) disable_albumartist=true;
				}
				if(!disable_publisher)
				{
					buf[0]=0;
					editDevice->getTrackPublisher(song,buf,sizeof(buf)/sizeof(wchar_t));
					if(!buf[0]);
					else if(!last_publisher[0]) lstrcpyn(last_publisher,buf,2048);
					else if(lstrcmp(buf,last_publisher)) disable_publisher=true;
				}
				if(!disable_composer)
				{
					buf[0]=0;
					editDevice->getTrackComposer(song,buf,sizeof(buf)/sizeof(wchar_t));
					if(!buf[0]);
					else if(!last_composer[0]) lstrcpyn(last_composer,buf,2048);
					else if(lstrcmp(buf,last_composer)) disable_composer=true;
				}
				if(!disable_title)
				{
					buf[0]=0;
					editDevice->getTrackTitle(song,buf,sizeof(buf)/sizeof(wchar_t));
					if(!buf[0]);
					else if(!last_title[0]) 
					lstrcpyn(last_title,buf,2048);
					else if(lstrcmp(buf,last_title)) disable_title=true;
				}
				if(!disable_album)
				{
					buf[0]=0;
					editDevice->getTrackAlbum(song,buf,sizeof(buf)/sizeof(wchar_t));
					if(!buf[0]);
					else if(!last_album[0]) lstrcpyn(last_album,buf,2048);
					else if(lstrcmp(buf,last_album)) disable_album=true;
				}
				if(!disable_genre)
				{
					buf[0]=0;
					editDevice->getTrackGenre(song,buf,sizeof(buf)/sizeof(wchar_t));
					if(!buf[0]);
					else if(!last_genre[0]) lstrcpyn(last_genre,buf,2048);
					else if(lstrcmp(buf,last_genre)) disable_genre=true;
				}
				if(!disable_year)
				{
					int val=editDevice->getTrackYear(song);
					if(val <= 0);
					else if(last_year==-1) last_year=val;
					else if(last_year!=val) disable_year=true;
				}
				if(!disable_track)
				{
					int val=editDevice->getTrackTrackNum(song);
					if(val <= 0);
					else if(last_track==-1) last_track=val;
					else if(last_track!=val) disable_track=true;
				}
				if(!disable_disc)
				{
					int val=editDevice->getTrackDiscNum(song);
					if(val <= 0);
					else if(last_disc==-1) last_disc=val;
					else if(last_disc!=val) disable_disc=true;
				}	
				if(!disable_albumart)
				{
					pmpart_t a = editDevice->getArt(song);
					if(!a);
					else if(!last_albumart) { editDevice->releaseArt(last_albumart); last_albumart = a; }
					else if(!editDevice->artIsEqual(a,last_albumart)) { disable_albumart=true; editDevice->releaseArt(a); editDevice->releaseArt(last_albumart); last_albumart=0; }
				}
			}
			if(!disable_artist && last_artist) SetDlgItemText(hwndDlg,IDC_EDIT_ARTIST,last_artist);
			if(!disable_albumartist && last_albumartist) SetDlgItemText(hwndDlg,IDC_EDIT_ALBUMARTIST,last_albumartist);
			if(!disable_publisher && last_publisher) SetDlgItemText(hwndDlg,IDC_EDIT_PUBLISHER,last_publisher);
			if(!disable_composer && last_composer) SetDlgItemText(hwndDlg,IDC_EDIT_COMPOSER,last_composer);
			if(!disable_title && last_title) SetDlgItemText(hwndDlg,IDC_EDIT_TITLE,last_title);
			if(!disable_album && last_album) SetDlgItemText(hwndDlg,IDC_EDIT_ALBUM,last_album);
			if(!disable_genre && last_genre) SetDlgItemText(hwndDlg,IDC_EDIT_GENRE,last_genre);
			if(!disable_year && last_year>0) {
				wchar_t tmp[64] = {0};
				wsprintf(tmp,L"%d",last_year);
		        SetDlgItemText(hwndDlg,IDC_EDIT_YEAR,tmp);
			}
			if(!disable_track && last_track>0) {
				wchar_t tmp[64] = {0};
				wsprintf(tmp,L"%d",last_track);
				SetDlgItemText(hwndDlg,IDC_EDIT_TRACK,tmp);
			}
			if(!disable_disc && last_disc>0) {
				wchar_t tmp[64] = {0};
				wsprintf(tmp,L"%d",last_disc);
				SetDlgItemText(hwndDlg,IDC_EDIT_DISC,tmp);
			}
			if(!disable_albumart && last_albumart) {
				// save copy of image
				editinfo_image* image = (editinfo_image*)calloc(1, sizeof(editinfo_image));
				editDevice->getArtNaturalSize(last_albumart,&image->w,&image->h);
				image->data = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(sizeof(ARGB32)*image->w*image->h);
				editDevice->getArtData(last_albumart,image->data);
				setBitmap(image,hwndDlg,true);
				editDevice->releaseArt(last_albumart);
			}
			else SetWindowLongPtr(hwndDlg,GWLP_USERDATA,0);

			if (FALSE != CenterWindow(hwndDlg, (HWND)lParam))
				SendMessage(hwndDlg, DM_REPOSITION, 0, 0L);

		}
		break;
		case WM_LBUTTONDOWN:
		{
			POINTS p = MAKEPOINTS(lParam);
			POINT p2 = {p.x, p.y};
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_ARTIST, IDC_CHECK_ARTIST)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_TITLE, IDC_CHECK_TITLE)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_ALBUM, IDC_CHECK_ALBUM)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_TRACK, IDC_CHECK_TRACK)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_DISC, IDC_CHECK_DISC)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_GENRE, IDC_CHECK_GENRE)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_YEAR, IDC_CHECK_YEAR)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_ALBUMARTIST, IDC_CHECK_ALBUMARTIST)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_COMPOSER, IDC_CHECK_COMPOSER)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_PUBLISHER, IDC_CHECK_PUBLISHER)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_PICTUREHOLDER, IDC_CHECK_ALBUMART,true)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_ART_CLEAR, IDC_CHECK_ALBUMART,true)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_ART_CHANGE, IDC_CHECK_ALBUMART,true)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_ARTINFO, IDC_CHECK_ALBUMART,true)) break;
		}
		break;
		case WM_RBUTTONDOWN:
		{
			POINTS pts = MAKEPOINTS(lParam);
			POINT p = {pts.x, pts.y};
			RECT r;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_PICTUREHOLDER), &r);
			ScreenToClient(hwndDlg, (LPPOINT)&r);
			ScreenToClient(hwndDlg, (LPPOINT)&r.right);
			if(PtInRect(&r, p)) { // right click on picture holder
				extern HMENU m_context_menus;
				HMENU menu = GetSubMenu(m_context_menus, 11);
				POINT p;
				GetCursorPos(&p);
				wchar_t artist[256]=L"";
				wchar_t album[256]=L"";
				GetDlgItemText(hwndDlg,IDC_EDIT_ALBUMARTIST,artist,256);
				if(!artist[0])
					GetDlgItemText(hwndDlg,IDC_EDIT_ARTIST,artist,256);
				GetDlgItemText(hwndDlg,IDC_EDIT_ALBUM,album,256);

				bool canpaste=(!!IsClipboardFormatAvailable(CF_DIB));
				bool hasimage= (GetWindowLongPtr(hwndDlg,GWLP_USERDATA) != 0);
				EnableMenuItem(menu, ID_ARTEDITMENU_PASTE, MF_BYCOMMAND | (canpaste?MF_ENABLED:MF_GRAYED));
				EnableMenuItem(menu, ID_ARTEDITMENU_COPY, MF_BYCOMMAND | (hasimage?MF_ENABLED:MF_GRAYED));
				EnableMenuItem(menu, ID_ARTEDITMENU_DELETE, MF_BYCOMMAND | (hasimage?MF_ENABLED:MF_GRAYED));
				EnableMenuItem(menu, ID_ARTEDITMENU_SAVEAS, MF_BYCOMMAND | (hasimage?MF_ENABLED:MF_GRAYED));

				// disabled 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
				#if 0
				bool candownload = (album[0] || artist[0]);
				EnableMenuItem(menu, ID_ARTEDITMENU_DOWNLOAD, MF_BYCOMMAND | (candownload?MF_ENABLED:MF_GRAYED));
				#else
				DeleteMenu(menu, ID_ARTEDITMENU_DOWNLOAD, MF_BYCOMMAND);
				#endif

				int r = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, p.x, p.y, 0, hwndDlg, NULL);
				switch(r) {
					// disabled 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
					#if 0
					case ID_ARTEDITMENU_DOWNLOAD:
					{
						artFetchData d = {sizeof(d),hwndDlg,artist,album,0};
						int r = (int)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(LPARAM)&d,IPC_FETCH_ALBUMART);
						if(r == 0 && d.imgData && d.imgDataLen) // success, save art in correct location
						{
							int w=0,h=0;
							ARGB32* bytes = loadImg(d.imgData,d.imgDataLen,&w,&h);
							if(bytes)
							{
								editinfo_image *image = (editinfo_image *)calloc(1, sizeof(editinfo_image));
								image->data = bytes;
								image->w = w;
								image->h = h;
								setBitmap(image,hwndDlg);
							}
							WASABI_API_MEMMGR->sysFree(d.imgData);
						}
					}
					break;
					#endif
					case ID_ARTEDITMENU_COPY:
					{
						if (!OpenClipboard(hwndDlg)) break;
						EmptyClipboard(); 
						HBITMAP bm = getBitmap((editinfo_image*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA),hwndDlg);
						SetClipboardData(CF_BITMAP,bm);
						CloseClipboard();
					}
					break;
					case ID_ARTEDITMENU_PASTE:
					{
						if (!OpenClipboard(hwndDlg)) break;
						HBITMAP bm = (HBITMAP)GetClipboardData(CF_BITMAP);
						if(bm) setBitmap(bm,hwndDlg);
						CloseClipboard();
					}
					break;
					case ID_ARTEDITMENU_DELETE:
						CheckDlgButton(hwndDlg,IDC_CHECK_ALBUMART,BST_CHECKED);
						enableArt(hwndDlg,false,TRUE);
						SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_ART_CLEAR,0),0);
						break;
					case ID_ARTEDITMENU_SAVEAS:
					{

						static int tests_run = 0;
						wchar_t file[1024] = {0};
						static wchar_t filter[1024] = {0}, *sff = filter;
						OPENFILENAME fn = {0};
						fn.lStructSize = sizeof(fn);
						fn.hwndOwner = hwndDlg;
						fn.lpstrFile = file;
						fn.nMaxFile = 1020;

						if(!tests_run)
						{
							tests_run = 1;
							FOURCC imgload = svc_imageLoader::getServiceType();
							int n = plugin.service->service_getNumServices(imgload);
							size_t size = 1024;
							for (int i = 0, j = 0; i<n; i++)
							{
								waServiceFactory *sf = plugin.service->service_enumService(imgload,i);
								if (sf)
								{
									svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
									if (l)
									{
										wchar_t *tests[] = {L"*.jpg",L"*.png",L"*.gif",L"*.bmp"};
										for(int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++)
										{
											if (l->isMine(tests[i]))
											{
												j++;
												int len = 0, tests_str[] = {IDS_JPEG_FILE,IDS_PNG_FILE,IDS_GIF_FILE,IDS_BMP_FILE};
												WASABI_API_LNGSTRINGW_BUF(tests_str[i],sff,size);
												size-=(len = lstrlenW(sff)+1);
												sff+=len;
												lstrcpynW(sff,tests[i],size);
												size-=(len = lstrlenW(sff)+1);
												sff+=len;
											}
										}
										sf->releaseInterface(l);
									}
								}
							}
						}

						fn.lpstrFilter = filter;
						fn.Flags = OFN_OVERWRITEPROMPT;
						GetSaveFileName(&fn);
						int l = wcslen(file);
						if(l>4 && file[l-4]==L'.'); // we have an extention
						else switch(fn.nFilterIndex) {
							case 1: StringCchCat(file, ARRAYSIZE(file), L".jpg"); break;
							case 2: StringCchCat(file, ARRAYSIZE(file), L".png"); break;
							case 3: StringCchCat(file, ARRAYSIZE(file), L".gif"); break;
							case 4: StringCchCat(file, ARRAYSIZE(file), L".bmp"); break;
						}
						editinfo_image *image = (editinfo_image *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
						writeImageToFile(image->data,image->w,image->h,file);
					}
					break;
				}
			}
		}
		break;
		case WM_COMMAND:
#define HANDLE_CONTROL(item, check) { int enabled = IsDlgButtonChecked(hwndDlg, check); EnableWindow(GetDlgItem(hwndDlg, item), enabled); EnableWindow(GetDlgItem(hwndDlg, IDOK), enabled); }
			switch(LOWORD(wParam)) {

				case IDC_CHECK_ARTIST: HANDLE_CONTROL(IDC_EDIT_ARTIST, IDC_CHECK_ARTIST); break;
				case IDC_CHECK_TITLE: HANDLE_CONTROL(IDC_EDIT_TITLE, IDC_CHECK_TITLE); break;
				case IDC_CHECK_ALBUM: HANDLE_CONTROL(IDC_EDIT_ALBUM, IDC_CHECK_ALBUM); break;
				case IDC_CHECK_ALBUMARTIST: HANDLE_CONTROL(IDC_EDIT_ALBUMARTIST, IDC_CHECK_ALBUMARTIST); break;
				case IDC_CHECK_COMPOSER: HANDLE_CONTROL(IDC_EDIT_COMPOSER, IDC_CHECK_COMPOSER); break;
				case IDC_CHECK_PUBLISHER: HANDLE_CONTROL(IDC_EDIT_PUBLISHER, IDC_CHECK_PUBLISHER); break;
				case IDC_CHECK_TRACK: HANDLE_CONTROL(IDC_EDIT_TRACK, IDC_CHECK_TRACK); break;
				case IDC_CHECK_DISC: HANDLE_CONTROL(IDC_EDIT_DISC, IDC_CHECK_DISC); break;
				case IDC_CHECK_GENRE: HANDLE_CONTROL(IDC_EDIT_GENRE, IDC_CHECK_GENRE); break;
				case IDC_CHECK_YEAR: HANDLE_CONTROL(IDC_EDIT_YEAR, IDC_CHECK_YEAR); break;
				//case IDC_CHECK_COMMENT: HANDLE_CONTROL(IDC_EDIT_COMMENT, IDC_CHECK_COMMENT); break;
				//case IDC_CHECK_CATEGORY: HANDLE_CONTROL(IDC_EDIT_CATEGORY, IDC_CHECK_CATEGORY); break;
				//case IDC_CHECK_DIRECTOR: HANDLE_CONTROL(IDC_EDIT_DIRECTOR, IDC_CHECK_DIRECTOR); break;
				//case IDC_CHECK_PRODUCER: HANDLE_CONTROL(IDC_EDIT_PRODUCER, IDC_CHECK_PRODUCER); break;
				//case IDC_CHECK_BPM: HANDLE_CONTROL(IDC_EDIT_BPM, IDC_CHECK_BPM); break;
				//case IDC_CHECK_RATING: HANDLE_CONTROL(IDC_COMBO_RATING, IDC_CHECK_RATING); break;
				case IDC_CHECK_ALBUMART:
				{
					int enabled = IsDlgButtonChecked(hwndDlg, IDC_CHECK_ALBUMART);
					enableArt(hwndDlg, false, enabled);
					EnableWindow(GetDlgItem(hwndDlg, IDOK), enabled);
					break;
				}
				case IDC_ART_CLEAR:
				{
					editinfo_image* image = (editinfo_image*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
					if(image) {
						if(image->data) WASABI_API_MEMMGR->sysFree(image->data);
						free(image);
					}
					SetDlgItemText(hwndDlg,IDC_ARTINFO,WASABI_API_LNGSTRINGW(IDS_NO_IMAGE));
					SetWindowLongPtr(hwndDlg,GWLP_USERDATA,0);
					HBITMAP old = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_PICTUREHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)0);
					DeleteObject(old);
				}
				break;
				case IDC_ART_CHANGE:
				{
					static wchar_t fileExtensionsString[MAX_PATH] = {0};
					wchar_t file[1024]=L"";
					OPENFILENAME fn = {0};
					fn.lStructSize = sizeof(fn);
					fn.hwndOwner = hwndDlg;
					fn.lpstrFile = file;
					fn.nMaxFile = 1024;

					if(!fileExtensionsString[0])
					{
						WASABI_API_LNGSTRINGW_BUF(IDS_IMAGE_FILES,fileExtensionsString,MAX_PATH);
						wchar_t *temp=fileExtensionsString+lstrlenW(fileExtensionsString) + 1;

						// query the available image loaders and build it against the supported formats
						FOURCC imgload = svc_imageLoader::getServiceType();
						int n = plugin.service->service_getNumServices(imgload);
						for (int i=0; i<n; i++)
						{
							waServiceFactory *sf = plugin.service->service_enumService(imgload,i);
							if (sf)
							{
								svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
								if (l)
								{
									wchar_t *tests[] = {L"*.jpg",L"*.png",L"*.gif",L"*.bmp"};
									for(int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++)
									{
										if (l->isMine(tests[i]))
										{
											StringCchCatW(temp,MAX_PATH,tests[i]);
											StringCchCatW(temp,MAX_PATH,L";");
										}
									}
									sf->releaseInterface(l);
								}
							}
						}
						*(temp = temp + lstrlenW(temp) + 1) = 0;
					}

					fn.lpstrFilter = fileExtensionsString;
					fn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					GetOpenFileName(&fn);
					editinfo_image *image = (editinfo_image *)calloc(1, sizeof(editinfo_image));
					image->data = loadImgFromFile(file,&image->w,&image->h);
					if(!image->data) { free(image); break; }
					setBitmap(image,hwndDlg);
				}
				break;
				case IDOK:
				{
					if(WASABI_API_DIALOGBOXPARAMW(IDD_PROGRESS,hwndDlg,editInfo_commit_dialogProc, (LPARAM)hwndDlg) == 0)
						EndDialog(hwndDlg,0);
					DeviceView * editDeviceView=NULL;
					for(int i=0; i < devices.GetSize(); i++) if(((DeviceView*)devices.Get(i))->dev == editDevice) editDeviceView = (DeviceView*)devices.Get(i);
					if(editDeviceView) editDeviceView->DevicePropertiesChanges();
					SendMessage(hwndMediaView,WM_USER+2,1,0);
				}
				break;
				case IDCANCEL:
					EndDialog(hwndDlg,0);
				break;
			}
		break;
		case WM_DESTROY:
		{
			editinfo_image* image = (editinfo_image*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			if(image) {
				if(image->data) WASABI_API_MEMMGR->sysFree(image->data);
				free(image);
			}
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,0);
			HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_PICTUREHOLDER,STM_GETIMAGE,IMAGE_BITMAP,0);
			if(bmold) DeleteObject(bmold);
		}
		break;
	}
	return FALSE;
}

void editInfo(C_ItemList * items, Device * dev, HWND centerWindow) {
	editItems = items;
	editDevice = dev;
	WASABI_API_DIALOGBOXPARAMW(IDD_EDIT_INFO, plugin.hwndLibraryParent, editInfo_dialogProc, (LPARAM)centerWindow);
}