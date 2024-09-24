// the modern file info box

#include "Main.h"
#include "resource.h"
#include "api.h"
#include "../nu/AutoWide.h"
#include <tataki/export.h>
#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/bltcanvas.h>
#include <api/service/waservicefactory.h>
#include <api/service/svcs/svc_imgload.h>
#include "language.h"
#ifndef IGNORE_API_GRACENOTE
#ifndef _WIN64
#include "../gracenote/api_gracenote.h"
#endif
#endif
#include "../Agave/Language/api_language.h"
#include "decodefile.h"
#include "../nu/AutoLock.h"
#include <api/service/svcs/svc_imgwrite.h>
#include "../Plugins/General/gen_ml/ml.h"
#include <vector>

extern Nullsoft::Utility::LockGuard getMetadataGuard;
#define TEXTBUFFER_MAX	65536

enum FileInfoMode {
	FILEINFO_MODE_0,
	FILEINFO_MODE_1,
};

int ModernInfoBox(In_Module * in, FileInfoMode mode, const wchar_t * filename, HWND parent);

static INT_PTR CALLBACK FileInfo(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK FileInfo_Metadata(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK FileInfo_Artwork(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef HWND (__cdecl *AddUnifiedFileInfoPane)(int n, const wchar_t * filename, HWND parent, wchar_t *name, size_t namelen);

class info_params
{
public:
	const wchar_t * filename;
	In_Module * in;
	std::vector<HWND> tabs;
	FileInfoMode mode;
 	size_t initial_position, length;
	void (*infoBoxExCallback)(size_t position, wchar_t filepath[FILENAME_SIZE]);
private:
	wchar_t *buffer_; // to track allocations made by this object
public:
	info_params(FileInfoMode mode, const wchar_t* filename, In_Module * in)
	{
		if (filename) {
			// In order to prevent jtfe 1.33 from crashing winamp, we need to make sure that 
			// this->filename doesn't point to the begining of the allocated block, to do
			// this, buffer_offset set to 2 (4 bytes).
			size_t buffer_offset = 2; // "voodoo magic"
			size_t filename_length = wcslen(filename);
			size_t buffer_size = filename_length + 1;
			buffer_ = reinterpret_cast<wchar_t*>(malloc((buffer_size + buffer_offset)*sizeof(wchar_t)));
			if (buffer_) {
				wchar_t *filename_buffer = buffer_ + buffer_offset;
				wcsncpy_s(filename_buffer, buffer_size, filename, filename_length);
				filename_buffer[filename_length] = '\0';
				this->filename = filename_buffer;
			} else {
				// oops, looks like we out of memory (extremly rare but possible). Probably best thing,
				// every responsible person should do at this point is to crash app asap,
				// ...but we also can assign filename pointer directly and let nasty things happen 
				// somewhere later (as of writing of this comment, info_params constructed right before 
				// going in to the modal loop, which makes it kind of blocking call).
				this->filename = filename;
			}
		}
		else {
			buffer_ = NULL;
			this->filename = filename;
		}
		
		this->mode = mode;
		this->in = in;
		this->initial_position = 0;
		this->length = 0;
	}

	~info_params() {
		free(buffer_);
	}
};

int ModernInfoBox(In_Module * in, FileInfoMode mode, const wchar_t * filename, HWND parent)
{
	Tataki::Init(WASABI_API_SVC);
 	info_params params(mode, filename, in);
 
 	int ret = (int)LPDialogBoxParamW(IDD_FILEINFO, parent, FileInfo, (LPARAM)&params);
	Tataki::Quit();
  	return ret;
}

static VOID WINAPI OnSelChanged(HWND hwndDlg, HWND hwndTab, info_params * p)
{
	ShowWindow(p->tabs[config_last_fileinfo_page], SW_HIDE);
	EnableWindow(p->tabs[config_last_fileinfo_page], 0);
	config_last_fileinfo_page=TabCtrl_GetCurSel(hwndTab);
	ShowWindow(p->tabs[config_last_fileinfo_page], SW_SHOWNA);
	EnableWindow(p->tabs[config_last_fileinfo_page], 1);
	if (GetFocus() != hwndTab)
	{
		SetFocus(p->tabs[config_last_fileinfo_page]);
	}
}

static HWND CreateTab(FileInfoMode mode, int n, const wchar_t *file, HWND parent, wchar_t * name, size_t namelen, AddUnifiedFileInfoPane aufip)
{
	switch (n)
	{
		case 0:
			if (mode == FILEINFO_MODE_0) {
				getStringW(IDS_BASICINFO, name, namelen);
				return LPCreateDialogW(IDD_FILEINFO_METADATA , parent, (WNDPROC)FileInfo_Metadata);
			} else {
				getStringW(IDS_STREAMINFO, name, namelen);
				return LPCreateDialogW(IDD_FILEINFO_STREAMDATA, parent, (WNDPROC)FileInfo_Metadata);
			}
		case 1:
			getStringW(IDS_ARTWORK,name,namelen);
			return LPCreateDialogW(IDD_FILEINFO_ARTWORK,parent,(WNDPROC)FileInfo_Artwork);
		default:
			if (mode == FILEINFO_MODE_0) {
				getStringW(IDS_ADVANCED,name,namelen);
				if (aufip) return aufip(n-2,file,parent,name,namelen);
			}
			return NULL;
	}
}

static INT_PTR CALLBACK FileInfo(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SetPropW(hwndDlg,L"INBUILT_NOWRITEINFO", (HANDLE)0);

			info_params * p = (info_params *)lParam;
			SetWindowLongPtrW(hwndDlg,GWLP_USERDATA,lParam);

			HWND ok = GetDlgItem(hwndDlg, IDOK);
			if (p->mode == FILEINFO_MODE_0) {
				EnableWindow(ok, TRUE);
				ShowWindow(ok, SW_SHOW);
			} else {
				EnableWindow(ok, FALSE);
				ShowWindow(ok, SW_HIDE);
			}

			SetDlgItemTextW(hwndDlg,IDC_FN,p->filename);

			// added 5.66 so plug-ins can get a hint that something may change...
			SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)p->filename, IPC_FILE_TAG_MAY_UPDATEW);

			AddUnifiedFileInfoPane aufip = (AddUnifiedFileInfoPane)GetProcAddress(p->in->hDllInstance, "winampAddUnifiedFileInfoPane");

			HWND hwndTab = GetDlgItem(hwndDlg,IDC_TAB1);
			wchar_t name[100] = {0};
			TCITEMW tie = {0};
			tie.mask = TCIF_TEXT;
			tie.pszText = name;
			HWND tab=NULL;
			int n=0;

			while (NULL != (tab = CreateTab(p->mode, n, p->filename, hwndDlg, name, 100, aufip)))
			{
				p->tabs.push_back(tab);
				ShowWindow(tab,SW_HIDE);

				if (!SendMessageW(hMainWindow,WM_WA_IPC,IPC_ISWINTHEMEPRESENT,IPC_USE_UXTHEME_FUNC))
					SendMessageW(hMainWindow,WM_WA_IPC,(WPARAM)tab,IPC_USE_UXTHEME_FUNC);

				SendMessageW(hwndTab, TCM_INSERTITEMW, n, (LPARAM)&tie);
				n++;
			}

			RECT r;
			GetWindowRect(hwndTab,&r);
			TabCtrl_AdjustRect(hwndTab,FALSE,&r);
			MapWindowPoints(NULL,hwndDlg,(LPPOINT)&r,2);
			r.left += 3;
			r.top += 2;
			r.right -= 4;

			for (size_t i=0; i < p->tabs.size(); i++)
				SetWindowPos(p->tabs[i],HWND_TOP,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOACTIVATE);

			if (config_last_fileinfo_page >= (int)p->tabs.size()) config_last_fileinfo_page = 0;

			TabCtrl_SetCurSel(hwndTab,config_last_fileinfo_page);

			ShowWindow(p->tabs[config_last_fileinfo_page], SW_SHOWNA);

			// show alt+3 window and restore last position as applicable
			POINT pt = {alt3_rect.left, alt3_rect.top};
			if (!windowOffScreen(hwndDlg, pt) && !IsWindowVisible(hwndDlg))
				SetWindowPos(hwndDlg, HWND_TOP, alt3_rect.left, alt3_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);

			return 1;
		}
		break;
		case WM_NOTIFY:
		{
			LPNMHDR lpn = (LPNMHDR) lParam;
			if (lpn && lpn->code==TCN_SELCHANGE)
			{
				info_params * p = (info_params *)GetWindowLongPtrW(hwndDlg,GWLP_USERDATA);
				OnSelChanged(hwndDlg,GetDlgItem(hwndDlg,IDC_TAB1),p);
			}
		}
		break;
		case WM_USER:
		{
			info_params * p = (info_params *)GetWindowLongPtrW(hwndDlg,GWLP_USERDATA);
			for (size_t i=0; i < p->tabs.size(); i++)
				SendMessageW(p->tabs[i],uMsg,wParam,lParam);
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					Nullsoft::Utility::AutoLock metadata_lock(getMetadataGuard);
					info_params * p = (info_params *)GetWindowLongPtrW(hwndDlg,GWLP_USERDATA);
					for (size_t i=0; i < p->tabs.size(); i++)
					{
						SendMessageW(p->tabs[i],uMsg,wParam,lParam);
					}
					GetWindowRect(hwndDlg, &alt3_rect);
					EndDialog(hwndDlg,0);
				}
				break;
				case IDCANCEL:
				{
					Nullsoft::Utility::AutoLock metadata_lock(getMetadataGuard);
					info_params * p = (info_params *)GetWindowLongPtrW(hwndDlg,GWLP_USERDATA);
					for (size_t i=0; i < p->tabs.size(); i++)
					{
						SendMessageW(p->tabs[i],uMsg,wParam,lParam);
					}
					GetWindowRect(hwndDlg, &alt3_rect);
					EndDialog(hwndDlg,1);
				}
				break;
			}
			break;
		case WM_CLOSE:
			return FileInfo(hwndDlg,WM_COMMAND,MAKEWPARAM(IDCANCEL,0),0);
	}
	return 0;
}

const wchar_t *genres[] =
{
	L"Blues", L"Classic Rock", L"Country", L"Dance", L"Disco", L"Funk", L"Grunge", L"Hip-Hop",
	L"Jazz", L"Metal", L"New Age", L"Oldies", L"Other", L"Pop", L"R&B", L"Rap", L"Reggae", L"Rock",
	L"Techno", L"Industrial", L"Alternative", L"Ska", L"Death Metal", L"Pranks", L"Soundtrack",
	L"Euro-Techno", L"Ambient", L"Trip-Hop", L"Vocal", L"Jazz+Funk", L"Fusion", L"Trance",
	L"Classical", L"Instrumental", L"Acid", L"House", L"Game", L"Sound Clip", L"Gospel", L"Noise",
	L"Alt Rock", L"Bass", L"Soul", L"Punk", L"Space", L"Meditative", L"Instrumental Pop",
	L"Instrumental Rock", L"Ethnic", L"Gothic", L"Darkwave", L"Techno-Industrial",
	L"Electronic", L"Pop-Folk", L"Eurodance", L"Dream", L"Southern Rock", L"Comedy", L"Cult",
	L"Gangsta Rap", L"Top 40", L"Christian Rap", L"Pop/Funk", L"Jungle", L"Native American",
	L"Cabaret", L"New Wave", L"Psychedelic", L"Rave", L"Showtunes", L"Trailer", L"Lo-Fi", L"Tribal",
	L"Acid Punk", L"Acid Jazz", L"Polka", L"Retro", L"Musical", L"Rock & Roll", L"Hard Rock",
	L"Folk", L"Folk-Rock", L"National Folk", L"Swing", L"Fast-Fusion", L"Bebop", L"Latin", L"Revival",
	L"Celtic", L"Bluegrass", L"Avantgarde", L"Gothic Rock", L"Progressive Rock", L"Psychedelic Rock",
	L"Symphonic Rock", L"Slow Rock", L"Big Band", L"Chorus", L"Easy Listening", L"Acoustic", L"Humour",
	L"Speech", L"Chanson", L"Opera", L"Chamber Music", L"Sonata", L"Symphony", L"Booty Bass", L"Primus",
	L"Porn Groove", L"Satire", L"Slow Jam", L"Club", L"Tango", L"Samba", L"Folklore",
	L"Ballad", L"Power Ballad", L"Rhythmic Soul", L"Freestyle", L"Duet", L"Punk Rock", L"Drum Solo",
	L"A Cappella", L"Euro-House", L"Dance Hall", L"Goa", L"Drum & Bass", L"Club-House",
	L"Hardcore", L"Terror", L"Indie", L"BritPop", L"Afro-Punk", L"Polsk Punk", L"Beat",
	L"Christian Gangsta Rap", L"Heavy Metal", L"Black Metal", L"Crossover", L"Contemporary Christian",
	L"Christian Rock", L"Merengue", L"Salsa", L"Thrash Metal", L"Anime", L"JPop", L"Synthpop",
	L"Abstract", L"Art Rock", L"Baroque", L"Bhangra", L"Big Beat", L"Breakbeat", L"Chillout", L"Downtempo", L"Dub", L"EBM", L"Eclectic", L"Electro",
	L"Electroclash", L"Emo", L"Experimental", L"Garage", L"Global", L"IDM", L"Illbient", L"Industro-Goth", L"Jam Band", L"Krautrock", L"Leftfield", L"Lounge",
	L"Math Rock", L"New Romantic", L"Nu-Breakz", L"Post-Punk", L"Post-Rock", L"Psytrance", L"Shoegaze", L"Space Rock", L"Trop Rock", L"World Music", L"Neoclassical",
	L"Audiobook", L"Audio Theatre", L"Neue Deutsche Welle", L"Podcast", L"Indie Rock", L"G-Funk", L"Dubstep", L"Garage Rock", L"Psybient",
	L"Glam Rock", L"Dream Pop", L"Merseybeat", L"K-Pop", L"Chiptune", L"Grime", L"Grindcore", L"Indietronic", L"Indietronica", L"Jazz Rock", L"Jazz Fusion",
	L"Post-Punk Revival", L"Electronica", L"Psychill", L"Ethnotronic", L"Americana", L"Ambient Dub", L"Digital Dub", L"Chillwave", L"Stoner Rock",
	L"Slowcore", L"Softcore", L"Flamenco", L"Hi-NRG", L"Ethereal", L"Drone", L"Doom Metal", L"Doom Jazz", L"Mainstream", L"Glitch", L"Balearic",
	L"Modern Classical", L"Mod", L"Contemporary Classical", L"Psybreaks", L"Psystep", L"Psydub", L"Chillstep", L"Berlin School",
	L"Future Jazz", L"Djent", L"Musique Concrète", L"Electroacoustic", L"Folktronica", L"Texas Country", L"Red Dirt",
	L"Arabic", L"Asian", L"Bachata", L"Bollywood", L"Cajun", L"Calypso", L"Creole", L"Darkstep", L"Jewish", L"Reggaeton", L"Smooth Jazz", 
	L"Soca", L"Spiritual", L"Turntablism", L"Zouk", L"Neofolk", L"Nu Jazz", L"Psychobilly", L"Rockabilly", L"Schlager & Volksmusik",
};

const size_t numberOfGenres = sizeof(genres) / sizeof(genres[0]);

const wchar_t * strfields[] =
{
	L"artist",
	L"album",
	L"albumartist",
	L"title",
	L"year",
	L"genre",
	L"comment",
	L"composer",
	L"publisher",
	L"disc",
	L"track",
	L"bpm",
	L"GracenoteFileID",
	L"GracenoteExtData"
};

const int strfieldctrls[] =
{
	IDC_ARTIST,
	IDC_ALBUM,
	IDC_ALBUM_ARTIST,
	IDC_TITLE,
	IDC_YEAR,
	IDC_GENRE,
	IDC_COMMENT,
	IDC_COMPOSER,
	IDC_PUBLISHER,
	IDC_DISC,
	IDC_TRACK,
	IDC_BPM,
	IDC_GN_FILEID,
	IDC_GN_EXTDATA
};

const int staticstrfieldctrls[] =
{
	IDC_STATIC_ARTIST,
	IDC_STATIC_ALBUM,
	IDC_STATIC_ALBUM_ARTIST,
	IDC_STATIC_TITLE,
	IDC_STATIC_YEAR,
	IDC_STATIC_GENRE,
	IDC_STATIC_COMMENT,
	IDC_STATIC_COMPOSER,
	IDC_STATIC_PUBLISHER,
	IDC_STATIC_DISC,
	IDC_STATIC_TRACK,
	IDC_STATIC_BPM,
	0,
	0
};

const wchar_t * streamStrfields[] =
 {
 	L"streamtitle",
 	L"streamgenre",
 	L"streamurl",
 	L"streamname",
 	L"streamcurrenturl",
 	L"streammetadata",
 };
 
 const int streamStrfieldctrls[] =
 {
 	IDC_TITLE,
 	IDC_GENRE,
 	IDC_URL,
 	IDC_STATION,
 	IDC_PLAY_URL,
 	IDC_EXTRA_METADATA,
 };
 
 const int staticStreamStrfieldctrls[] =
 {
 	IDC_STATIC_TITLE,
 	IDC_STATIC_GENRE,
 	IDC_STATIC_URL,
 	IDC_STATIC_STATION,
 	IDC_STATIC_PLAY_URL,
 	0,
 };
#ifndef IGNORE_API_GRACENOTE
#ifndef _WIN64
static IConnectionPoint *GetConnectionPoint(IUnknown *punk, REFIID riid)
{
	if (!punk)
		return 0;

	IConnectionPointContainer *pcpc;
	IConnectionPoint *pcp = 0;

	HRESULT hr = punk->QueryInterface(IID_IConnectionPointContainer, (void **) & pcpc);
	if (SUCCEEDED(hr))
	{
		pcpc->FindConnectionPoint(riid, &pcp);
		pcpc->Release();
	}
	punk->Release();
	return pcp;
}

class autoTagListen : public _ICDDBMusicIDManagerEvents
{
public:
	autoTagListen(api_gracenote* gn,	ICDDBMusicIDManager3 *musicid, wchar_t *_gracenoteFileId) : hwndDlg(0), h(0), gn(gn),musicid(musicid), done(0), abort(0), gracenoteFileId(_gracenoteFileId)
	{
	}

	HRESULT OnTrackIDStatusUpdate(CddbMusicIDStatus Status, BSTR filename, long* _Abort)
	{
		if (abort) *_Abort=1;
		/*switch(Status)
		{
		case STATUS_MUSICID_Error:
		case STATUS_MUSICID_ProcessingFile:
		case STATUS_MUSICID_LookingUpWaveForm:
		case STATUS_MUSICID_LookingUpText:
		case STATUS_MUSICID_CheckForAbort:
		case STATUS_ALBUMID_Querying:
		case STATUS_ALBUMID_Queried:
		case STATUS_ALBUMID_Processing:
		case STATUS_ALBUMID_Processed:
		case STATUS_MUSICID_AnalyzingWaveForm:
		case STATUS_ALBUMID_QueryingWF:
		}*/
		// do shit
		return S_OK;
	}
	HRESULT OnTrackIDComplete(LONG match_code, ICddbFileInfo* pInfoIn, ICddbFileInfoList* pListOut)
	{
		done = 1;
		if (match_code <= 1)
		{
			if (!abort)
			{
				wchar_t title[16] = {0};
				MessageBoxW(h, getStringW(IDS_NO_MATCH_FOUND, NULL, 0),
							getStringW(IDS_FAILED, title, 16), MB_ICONWARNING);
			}
			EndDialog(hwndDlg,0);
			return S_OK;
		}

		long num = 0;
		pListOut->get_Count(&num);
		if (!num) return S_OK;
		ICddbFileInfoPtr infotag = NULL;
		pListOut->GetFileInfo(1,&infotag);

		if (infotag)
		{
			wchar_t buf[2048] = {0};
			BSTR bstr = buf;

			ICddbFileTagPtr tag = NULL;
			ICddbDisc2Ptr disc = NULL;
			ICddbDisc2_5Ptr disc2_5 = NULL;
			ICddbFileTag2_5Ptr tag2_5 = NULL;

			infotag->get_Tag(&tag);
			infotag->get_Disc(&disc);

			if (tag)
			{
				tag->QueryInterface(&tag2_5);
				disc->QueryInterface(&disc2_5);

#define PUTINFO(id, ctrl) if (tag) { tag->get_ ## id ## (&bstr); SetDlgItemTextW(h, ctrl, bstr); }
#define PUTINFO2(id, ctrl) if (tag2_5) { tag2_5->get_ ## id ## (&bstr); SetDlgItemTextW(h, ctrl, bstr); }
				PUTINFO(LeadArtist, IDC_ARTIST);
				PUTINFO(Album, IDC_ALBUM);
				PUTINFO(Title, IDC_TITLE);
				PUTINFO(Album, IDC_ALBUM);

				if (disc2_5 == NULL
					|| (FAILED(disc2_5->get_V2GenreStringPrimaryByLevel(3, &bstr))
						&& FAILED(disc2_5->get_V2GenreStringPrimaryByLevel(2, &bstr))
						&& FAILED(disc2_5->get_V2GenreStringPrimaryByLevel(1, &bstr))
						&& FAILED(disc2_5->get_V2GenreStringPrimary(&bstr)))
				   )
				{
					PUTINFO(Genre, IDC_GENRE);
				}
				else
				{
					SetDlgItemTextW(h,IDC_GENRE,bstr);
				}
			}

			// sending this will ensure that the genre is applied other than in the metadata page
			SendMessageW(GetParent(h),WM_USER,(WPARAM)strfields[5],(LPARAM)bstr);

			PUTINFO(Year, IDC_YEAR);
			PUTINFO(Label, IDC_PUBLISHER);
			PUTINFO(BeatsPerMinute, IDC_BPM);
			PUTINFO(TrackPosition, IDC_TRACK);
			PUTINFO(PartOfSet, IDC_DISC);
			PUTINFO2(Composer, IDC_COMPOSER);
			PUTINFO2(DiscArtist, IDC_ALBUM_ARTIST);
			PUTINFO(FileId, IDC_GN_FILEID);
			PUTINFO2(ExtDataSerialized, IDC_GN_EXTDATA);

#undef PUTINFO
#undef PUTINFO2
			// CRASH POINT
			// This is the starting cause of crashing in the cddb stuff
			// Without this or with a manual AddRef() before and it'll work ok
			//infotag->Release();
		}
		EndDialog(hwndDlg,0);
		return S_OK;
	}
	HRESULT FillTag(ICddbFileInfo *info, BSTR filename)
	{
		if (info)
		{
#define PUTINFO(id, ctrl) GetDlgItemTextW(h, ctrl, buf, 2048); if(buf[0]) if (infotag) { infotag->put_ ## id ## (buf); }
#define PUTINFO2(id, ctrl) GetDlgItemTextW(h, ctrl, buf, 2048); if(buf[0]) if (tag2_5) { tag2_5->put_ ## id ## (buf); }

			ICddbID3TagPtr infotag = NULL;
			infotag.CreateInstance(CLSID_CddbID3Tag);
			ICddbFileTag2_5Ptr tag2_5 = NULL;
			infotag->QueryInterface(&tag2_5);
			wchar_t buf[2048] = {0};

			PUTINFO(LeadArtist, IDC_ARTIST);
			PUTINFO(Album, IDC_ALBUM);
			PUTINFO(Title, IDC_TITLE);
			PUTINFO(Album, IDC_ALBUM);
			PUTINFO(Genre, IDC_GENRE);
			PUTINFO(Year, IDC_YEAR);
			PUTINFO(Label, IDC_PUBLISHER);
			PUTINFO(BeatsPerMinute, IDC_BPM);
			PUTINFO(TrackPosition, IDC_TRACK);
			PUTINFO(PartOfSet, IDC_DISC);
			PUTINFO2(Composer, IDC_COMPOSER);
			PUTINFO2(DiscArtist, IDC_ALBUM_ARTIST);

			in_get_extended_fileinfoW(filename,L"length",buf,2048);
			tag2_5->put_LengthMS(buf);

			if (gracenoteFileId && *gracenoteFileId)
				infotag->put_FileId(gracenoteFileId);

#undef PUTINFO
#undef PUTINFO2
			info->put_Tag(infotag);
		}
		return S_OK;
	}

	STDMETHODIMP STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObject)
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
	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return 1;
	}
	ULONG STDMETHODCALLTYPE Release(void)
	{
		return 0;
	}
	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
	{
		switch (dispid)
		{
		case 1: // OnTrackIDStatusUpdate, params: CddbMusicIDStatus Status, BSTR filename, long* Abort
		{
			long *abort = pdispparams->rgvarg[0].plVal;
			BSTR filename = pdispparams->rgvarg[1].bstrVal;
			CddbMusicIDStatus status = (CddbMusicIDStatus)pdispparams->rgvarg[2].lVal;
			return OnTrackIDStatusUpdate(status,filename,abort);
		}
		case 3: // OnTrackIDComplete, params: LONG match_code, ICddbFileInfo* pInfoIn, ICddbFileInfoList* pListOut
		{
			IDispatch *disp1 =pdispparams->rgvarg[0].pdispVal;
			IDispatch *disp2 =pdispparams->rgvarg[1].pdispVal;
			long match_code = pdispparams->rgvarg[2].lVal;

			ICddbFileInfoPtr pInfoIn;
			ICddbFileInfoListPtr matchList;
			disp1->QueryInterface(&matchList);
			disp2->QueryInterface(&pInfoIn);

			return OnTrackIDComplete(match_code,pInfoIn,matchList);
		}
		case 10: // OnGetFingerprintInfo
		{
			long *abort = pdispparams->rgvarg[0].plVal;
			IDispatch *disp = pdispparams->rgvarg[1].pdispVal;
			BSTR filename = pdispparams->rgvarg[2].bstrVal;
			ICddbFileInfo *info;
			disp->QueryInterface(&info);
			extern DecodeFile *decodeFile;
			return gn->CreateFingerprint(musicid, decodeFile, info, filename, abort);
		}
		break;
		case 11: // OnGetTagInfo
		{
			//long *Abort = pdispparams->rgvarg[0].plVal;
			IDispatch *disp = pdispparams->rgvarg[1].pdispVal;
			BSTR filename = pdispparams->rgvarg[2].bstrVal;

			ICddbFileInfo *info;
			disp->QueryInterface(&info);
			return FillTag(info, filename);
		}
		break;
		}
		return DISP_E_MEMBERNOTFOUND;
	}
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
	{
		*rgdispid = DISPID_UNKNOWN; return DISP_E_UNKNOWNNAME;
	}
	HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
	{
		return E_NOTIMPL;
	}
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int FAR * pctinfo)
	{
		return E_NOTIMPL;
	}

	HWND hwndDlg,h;
	api_gracenote* gn;
	ICDDBMusicIDManager3 *musicid;
	int done;
	long abort;
	wchar_t *gracenoteFileId;
};

static INT_PTR CALLBACK FileInfo_Autotagging(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		// have the close button disabled otherwise it might cause confusion
		EnableMenuItem(GetSystemMenu(hwndDlg, 0), SC_CLOSE, MF_BYCOMMAND|MF_GRAYED);
		SetWindowLong(hwndDlg,GWLP_USERDATA,lParam);
		autoTagListen * p = (autoTagListen *)lParam;
		p->hwndDlg = hwndDlg;
		if (p->done) EndDialog(hwndDlg,0);
	}
	break;
	}
	return 0;
}
#endif
#endif

LPCWSTR RepairMutlilineString(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer || cchBufferMax < 1)
		return NULL;

	LPWSTR temp = (WCHAR*)calloc(cchBufferMax, sizeof(WCHAR));
	if (NULL == temp) return NULL;

	LPWSTR p1, p2;
	p1 = pszBuffer;
	p2 = temp;

	INT cchLen;
	for (cchLen = 0; L'\0' != *p1 && ((p2 - temp) < cchBufferMax); cchLen++)
	{
		if(*p1 == L'\n')
		{
			*p2++ = L'\r';
			cchLen++;
		}
		*p2++ = *p1++;
	}
	CopyMemory(pszBuffer, temp, sizeof(WCHAR) * cchLen);
	pszBuffer[cchLen] = L'\0';
	free(temp);
	return pszBuffer;
}

static INT_PTR CALLBACK FileInfo_Metadata(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int my_change=0;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		my_change=1;

		int y;
		SendMessageW(GetDlgItem(hwndDlg, IDC_GENRE), CB_RESETCONTENT, (WPARAM)0, 0);
		SendMessageW(GetDlgItem(hwndDlg, IDC_GENRE), CB_SETCURSEL, (WPARAM)-1, 0);
		for (size_t x = 0; x != numberOfGenres; x ++)
		{
			y = SendMessageW(GetDlgItem(hwndDlg, IDC_GENRE), CB_ADDSTRING, 0, (LPARAM)genres[x]);
			SendMessageW(GetDlgItem(hwndDlg, IDC_GENRE), CB_SETITEMDATA, y, x);
		}
		y = SendMessageW(GetDlgItem(hwndDlg, IDC_GENRE), CB_ADDSTRING, 0, (LPARAM)L"");
		SendMessageW(GetDlgItem(hwndDlg, IDC_GENRE), CB_SETITEMDATA, y, 255);

		info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);

		LPWSTR pszBuffer = (LPWSTR)calloc(TEXTBUFFER_MAX, sizeof(WCHAR));

		if (p->mode == FILEINFO_MODE_0) {
			for (int i=0; i<sizeof(strfields)/sizeof(wchar_t*); i++)
			{
				memset(pszBuffer, 0, sizeof(WCHAR) * TEXTBUFFER_MAX);
				int result = in_get_extended_fileinfoW(p->filename,strfields[i],pszBuffer,TEXTBUFFER_MAX);
				SetDlgItemTextW(hwndDlg, strfieldctrls[i], pszBuffer);
				EnableWindow(GetDlgItem(hwndDlg, strfieldctrls[i]), result?TRUE:FALSE);
				EnableWindow(GetDlgItem(hwndDlg, staticstrfieldctrls[i]), result?TRUE:FALSE);
			}
		} else {
			for (int i=0; i<sizeof(streamStrfields)/sizeof(wchar_t*); i++)
			{
				memset(pszBuffer, 0, sizeof(WCHAR) * TEXTBUFFER_MAX);
				int result = in_get_extended_fileinfoW(p->filename,streamStrfields[i],pszBuffer,TEXTBUFFER_MAX);
				if (streamStrfieldctrls[i] != IDC_EXTRA_METADATA) {
					SetDlgItemTextW(hwndDlg, streamStrfieldctrls[i], pszBuffer);
				} else {
					CheckDlgButton(hwndDlg, streamStrfieldctrls[i], result && pszBuffer[0] == L'1' ? TRUE : FALSE);
				}
				EnableWindow(GetDlgItem(hwndDlg, streamStrfieldctrls[i]), result ? TRUE : FALSE);
				EnableWindow(GetDlgItem(hwndDlg, staticStreamStrfieldctrls[i]), result ? TRUE : FALSE);
			}
		}

		pszBuffer[0]=0;
		if (p->mode == FILEINFO_MODE_0) {
			in_get_extended_fileinfoW(p->filename,L"formatinformation",pszBuffer, TEXTBUFFER_MAX);
		} else {
			in_get_extended_fileinfoW(p->filename,L"streaminformation",pszBuffer, TEXTBUFFER_MAX);
		}
		// due to quirks with the more common resource editors, is easier to just store the string
		// internally only with \n and post-process to be \r\n (as here) so it will appear correctly
		// on new lines as is wanted (silly multiline edit controls)
		SetDlgItemTextW(hwndDlg, IDC_FORMATINFO, RepairMutlilineString(pszBuffer, TEXTBUFFER_MAX));

		if (p->mode == FILEINFO_MODE_0) {
			wchar_t tg[64]=L"", ag[64]=L"";
			in_get_extended_fileinfoW(p->filename,L"replaygain_track_gain",tg,64);
			in_get_extended_fileinfoW(p->filename,L"replaygain_album_gain",ag,64);

			if (!tg[0]) getStringW(IDS_NOTPRESENT,tg,64);
			else
			{
				// this isn't nice but it localises the RG values
				// for display as they're saved in the "C" locale
				double value = _wtof_l(tg,langManager->Get_C_NumericLocale());
				StringCchPrintfW(tg,64,L"%-+.2f dB", value);
			}
			if (!ag[0]) getStringW(IDS_NOTPRESENT,ag,64);
			else
			{
				// this isn't nice but it localises the RG values
				// for display as they're saved in the "C" locale
				double value = _wtof_l(ag,langManager->Get_C_NumericLocale());
				StringCchPrintfW(ag,64,L"%-+.2f dB", value);
			}
			wchar_t tgagstr[128] = {0};
			StringCbPrintfW(pszBuffer, TEXTBUFFER_MAX, getStringW(IDS_TRACK_GAIN_AND_ALBUM_GAIN,tgagstr,128),tg,ag);
			SetDlgItemTextW(hwndDlg, IDC_REPLAYGAIN, RepairMutlilineString(pszBuffer, TEXTBUFFER_MAX));
			my_change=0;
			free(pszBuffer);
		} else {
			SetTimer(hwndDlg, 1, 1000, 0);
		}

		// test for the musicid feature not being available and remove
		// the autotag button as required (useful for lite installs)
		#ifndef IGNORE_API_GRACENOTE
		waServiceFactory *factory = WASABI_API_SVC?WASABI_API_SVC->service_getServiceByGuid(gracenoteApiGUID):0;
		api_gracenote* gn = NULL;
		int remove = FALSE;
		if(factory){
			gn = (api_gracenote *)factory->getInterface();
			if(gn){
				ICDDBMusicIDManager3 *musicid = gn->GetMusicID();
				if(musicid){
					musicid->Shutdown();
					musicid->Release();
				}
				else{
					remove = TRUE;
				}
				factory->releaseInterface(gn);
			}
			else{
				remove = TRUE;
			}
		}
		#else
		int remove = TRUE;
		#endif

		// remove or disable the button based on what has been
		// installed or the internet access levels configured
		if (remove)
		{
			DestroyWindow(GetDlgItem(hwndDlg,IDC_AUTOTAG));
		}
		else
		{
			if (!isInetAvailable())
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_AUTOTAG), FALSE);
			}
		}
	}
	break;
		case WM_TIMER:
 		if (wParam == 1) {
 			info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);
 			if (p && p->mode == FILEINFO_MODE_1) {
 				LPWSTR pszBuffer = (LPWSTR)calloc(TEXTBUFFER_MAX, sizeof(WCHAR));
 				if (pszBuffer) {
 					int start = -1, end = 0;
 					SendDlgItemMessage(hwndDlg, IDC_FORMATINFO, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
 					in_get_extended_fileinfoW(p->filename, L"streaminformation", pszBuffer, TEXTBUFFER_MAX);
 					// due to quirks with the more common resource editors, is easier to just store the string
 					// internally only with \n and post-process to be \r\n (as here) so it will appear correctly
 					// on new lines as is wanted (silly multiline edit controls)
 					SetDlgItemTextW(hwndDlg, IDC_FORMATINFO, RepairMutlilineString(pszBuffer, TEXTBUFFER_MAX));
 					SendDlgItemMessage(hwndDlg, IDC_FORMATINFO, EM_SETSEL, start, end);
 
 					for (int i=0; i<sizeof(streamStrfields)/sizeof(wchar_t*); i++) {
 						memset(pszBuffer, 0, sizeof(WCHAR) * TEXTBUFFER_MAX);
 						int result = in_get_extended_fileinfoW(p->filename,streamStrfields[i],pszBuffer,TEXTBUFFER_MAX);
 						if (streamStrfieldctrls[i] != IDC_EXTRA_METADATA) {
 							SendDlgItemMessage(hwndDlg, streamStrfieldctrls[i], EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
 							SetDlgItemTextW(hwndDlg, streamStrfieldctrls[i], pszBuffer);
 							SendDlgItemMessage(hwndDlg, streamStrfieldctrls[i], EM_SETSEL, start, end);
 						} else {
 							CheckDlgButton(hwndDlg, streamStrfieldctrls[i], result && pszBuffer[0] == L'1' ? TRUE : FALSE);
 						}
 						EnableWindow(GetDlgItem(hwndDlg, streamStrfieldctrls[i]), result ? TRUE : FALSE);
 						EnableWindow(GetDlgItem(hwndDlg, staticStreamStrfieldctrls[i]), result ? TRUE : FALSE);
 					}
 				}
 			}
 		}
 		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
#ifndef IGNORE_API_GRACENOTE
#ifndef _WIN64
		case IDC_AUTOTAG:
		{
			ICDDBMusicIDManager3 *musicid = NULL;
			waServiceFactory *factory = WASABI_API_SVC?WASABI_API_SVC->service_getServiceByGuid(gracenoteApiGUID):0;
			api_gracenote* gn = NULL;
			if (factory)
			{
				gn = (api_gracenote *)factory->getInterface();
				if (gn)
				{
					musicid = gn->GetMusicID();
				}
			}
			if (!musicid)
			{
				wchar_t title[32] = {0};
				if (gn)
				{
					if (factory)
					{
						factory->releaseInterface(gn);
					}
					gn = NULL;
				}
				MessageBoxW(hwndDlg, getStringW(IDS_GRACENOTE_TOOLS_NOT_INSTALLED, NULL, 0),
				            getStringW(IDS_ERROR, title, 32),0);
				break;
			}
			// we have the musicid pointer, so lets try and tag this mother.
			info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);
			
			if (NULL == p  || NULL == p->filename || L'\0' == p->filename)
				break;

			// first, see if there's a pre-existing gracenote file id
			wchar_t fileid[1024]=L"";
			extendedFileInfoStructW gracenote_info;
			gracenote_info.filename = p->filename;
			gracenote_info.metadata = L"GracenoteFileID";
			gracenote_info.ret = fileid;
			gracenote_info.retlen = 1024;
			if (0 == SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&gracenote_info, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE))
				fileid[0] = L'\0';

			ICddbFileInfoPtr info = 0;
			info.CreateInstance(CLSID_CddbFileInfo);
			info->put_Filename((wchar_t*)p->filename);
			ICddbFileInfoList *dummy=0;
			long match_code=666;
			IConnectionPoint *icp = GetConnectionPoint(musicid, DIID__ICDDBMusicIDManagerEvents);

			DWORD m_dwCookie = 0;
			autoTagListen listen(gn,musicid, fileid);
			listen.h = hwndDlg;
			if (icp) icp->Advise(static_cast<IDispatch *>(&listen), &m_dwCookie);

			musicid->TrackID(info, MUSICID_LOOKUP_ASYNC|MUSICID_RETURN_SINGLE|MUSICID_GET_TAG_FROM_APP|MUSICID_GET_FP_FROM_APP|MUSICID_PREFER_WF_MATCHES, &match_code, &dummy);
			if (dummy)
				dummy->Release();

			LPDialogBoxParamW(IDD_AUTOTAGGING,hwndDlg,FileInfo_Autotagging,(LPARAM)&listen);

			musicid->Shutdown();
			musicid->Release();
			factory->releaseInterface(gn);
		}
		break;
#endif
#endif

		case IDOK:
			if (!GetPropW(GetParent(hwndDlg),L"INBUILT_NOWRITEINFO"))
			{
				info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);

				LPWSTR pszBuffer = (LPWSTR)calloc(TEXTBUFFER_MAX, sizeof(WCHAR));
				if (pszBuffer)
				{
					for (int i=0; i<sizeof(strfields)/sizeof(wchar_t*); i++)
					{
						if (!GetDlgItemTextW(hwndDlg,strfieldctrls[i],pszBuffer, TEXTBUFFER_MAX))
							pszBuffer[0] = L'\0';
						in_set_extended_fileinfoW(p->filename,strfields[i], pszBuffer);
					}
					free(pszBuffer);
				}

				if (in_write_extended_fileinfo() == 0)
				{
					wchar_t title[256] = {0};
					MessageBoxW(hwndDlg,
								getStringW(IDS_METADATA_ERROR, NULL, 0),
								getStringW(IDS_METADATA_ERROR_TITLE, title, 256),						
								MB_OK | MB_ICONWARNING);
				}
			}
			break;
		default:
			if (!my_change && (HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_EDITUPDATE))
			{
				my_change=1;
				for (int i=0; i<sizeof(strfields)/sizeof(wchar_t*); i++)
				{
					if (LOWORD(wParam) == strfieldctrls[i])
					{
						if (HIWORD(wParam) != CBN_SELCHANGE)
						{
							LPWSTR pszBuffer = (LPWSTR)calloc(TEXTBUFFER_MAX, sizeof(WCHAR));
							if (pszBuffer)
							{
								GetDlgItemTextW(hwndDlg,strfieldctrls[i], pszBuffer, TEXTBUFFER_MAX);
								SendMessageW(GetParent(hwndDlg),WM_USER,(WPARAM)strfields[i],(LPARAM)pszBuffer);
								free(pszBuffer);
							}
						}
						else
						{
							int n = SendMessageW(GetDlgItem(hwndDlg, strfieldctrls[i]), CB_GETCURSEL, 0, 0);
							int m = SendMessageW(GetDlgItem(hwndDlg, strfieldctrls[i]), CB_GETITEMDATA, n, 0);
							if (m>=0 && m<numberOfGenres)
							{
								SendMessageW(GetParent(hwndDlg),WM_USER,(WPARAM)strfields[i],(LPARAM)genres[m]);
							}
							// handles case where genre is cleared
							else if (!n && m == 255)
							{
								SendMessageW(GetParent(hwndDlg),WM_USER,(WPARAM)strfields[i],(LPARAM)L"");
							}
							else if(n == CB_ERR)
							{
								// if we got here then it is likely to be from a genre not in the built in list
								LPWSTR pszBuffer = (LPWSTR)calloc(TEXTBUFFER_MAX, sizeof(WCHAR));
								if (pszBuffer)
								{
									if(GetDlgItemTextW(hwndDlg,strfieldctrls[i], pszBuffer, TEXTBUFFER_MAX)){
										SendMessageW(GetParent(hwndDlg),WM_USER,(WPARAM)strfields[i],(LPARAM)pszBuffer);
									}
									free(pszBuffer);
								}
							}
						}
					}
				}
				my_change=0;
			}
		}
		break;
	case WM_USER:
		if (wParam && lParam && !my_change)
		{
			for (int i=0; i<sizeof(strfields)/sizeof(wchar_t*); i++)
				if (_wcsicmp((wchar_t*)wParam,strfields[i])==0)
					SetDlgItemTextW(hwndDlg,strfieldctrls[i],(wchar_t*)lParam);
		}
		break;
	}
	return 0;
}

static void Adjust(int bmpw, int bmph, int &x, int &y, int &w, int &h)
{
	// maintain 'square' stretching
	double aspX = (double)(w)/(double)bmpw;
	double aspY = (double)(h)/(double)bmph;
	double asp = min(aspX, aspY);
	int newW = (int)(bmpw*asp);
	int newH = (int)(bmph*asp);
	x = (w - newW)/2;
	y = (h - newH)/2;
	w = newW;
	h = newH;
}

static HBITMAP getBitmap(ARGB32 * data, int w, int h, HWND parent)
{
	BITMAPINFO info={0};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = w;
	info.bmiHeader.biHeight = -h;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	HDC dc = GetDC(parent);
	HBITMAP bm = CreateCompatibleBitmap(dc,w,h);
	SetDIBits(dc,bm,0,h,data,&info,DIB_RGB_COLORS);
	ReleaseDC(parent,dc);
	return bm;
}

// these two are also used by AlbumArtRetrival.cpp
ARGB32 * decompressImage(const void *data, int datalen, int * dataW, int * dataH)
{
	ARGB32* ret=NULL;
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = (int) WASABI_API_SVC->service_getNumServices(imgload);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
		if (sf)
		{
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if (l)
			{
				if (l->testData(data,datalen))
				{
					ret = l->loadImage(data,datalen,dataW,dataH);
					sf->releaseInterface(l);
					break;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return ret;
}

HBITMAP getBitmap(ARGB32 * data, int dw, int dh, int targetW, int targetH, HWND parent)
{
	HQSkinBitmap bm(data,dw,dh);
	int x=0,y=0,w=targetW,h=targetH;
	Adjust(dw,dh,x,y,w,h);
	BltCanvas canv(targetW,targetH);
	bm.stretch(&canv,x,y,w,h);
	return getBitmap((ARGB32*)canv.getBits(),targetW,targetH,parent);
}

void EnableArtFrame(HWND hwndDlg, BOOL enable)
{
	const int artFrameElements[] =
	{
		IDC_STATIC_FRAME,
		IDC_ARTHOLDER,
		IDC_BUTTON_CHANGE,
		IDC_BUTTON_SAVEAS,
		IDC_BUTTON_COPY,
		IDC_BUTTON_PASTE,
		IDC_BUTTON_DELETE,
	};
	for (int i=0; i<sizeof(artFrameElements)/sizeof(int); i++)
		EnableWindow(GetDlgItem(hwndDlg,artFrameElements[i]),enable);
	SetDlgItemTextW(hwndDlg,IDC_STATIC_FRAME,getStringW(IDS_NO_IMAGE,0,0));
}

class EditArtItem
{
public:
	ARGB32 * bits;
	int w;
	int h;
	void *data;
	size_t datalen;
	wchar_t *mimetype;
	bool dirty;
	EditArtItem(ARGB32 *bits, int w, int h, bool dirty=false) : bits(bits), w(w), h(h), data(0), datalen(0), mimetype(0), dirty(dirty) {}
	~EditArtItem()
	{
		if (bits) WASABI_API_MEMMGR->sysFree(bits);
		if (mimetype) free(mimetype);
		if (data) WASABI_API_MEMMGR->sysFree(data);
	}
};

static EditArtItem * getCurrentArtItem(HWND hwndDlg)
{
	int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
	if (sel == -1)
		return NULL;
	EditArtItem *e = (EditArtItem *)SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETITEMDATA,sel,0);
	if (e != (EditArtItem *)-1)
		return e;
	return NULL;
}

static void GetSize(HBITMAP bm, int &w, int &h, HWND hwndDlg)
{
	HDC dc = GetDC(hwndDlg);
	BITMAPINFO info={0};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(dc,bm,0,0,NULL,&info,DIB_RGB_COLORS);
	w = abs(info.bmiHeader.biWidth);
	h = abs(info.bmiHeader.biHeight);
	ReleaseDC(hwndDlg,dc);
}

static bool AddNewArtItem(HWND hwndDlg)
{
	wchar_t buf[100]=L"";
	GetDlgItemTextW(hwndDlg,IDC_COMBO_ARTTYPE,buf,100);
	if (!buf[0]) return false;
	int s=SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_FINDSTRINGEXACT, (WPARAM)-1,(LPARAM)buf);

	if (s != LB_ERR && SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCOUNT,0,0))
	{
		// user has selected something already in our list
		SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETCURSEL,s,0);
		SendMessageW(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_ARTLIST,LBN_SELCHANGE),0);
		return true;
	}
	// let's add this new item
	s=SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_ADDSTRING,0,(LPARAM)buf);
	if (s == LB_ERR) return false;
	EditArtItem *e = new EditArtItem(0,0,0);
	SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETITEMDATA,s,(LPARAM)e);
	SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETCURSEL,s,0);
	EnableArtFrame(hwndDlg,TRUE);
	return true;
}

static svc_imageLoader *FindImageLoader(const wchar_t *filespec, waServiceFactory **factory)
{
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = (int) WASABI_API_SVC->service_getNumServices(imgload);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
		if (sf)
		{	
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if (l)
			{
				if (l->isMine(filespec))
				{
					*factory = sf;
					return l;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}


static ARGB32 *loadImgFromFile(const wchar_t *file, int *len, int *w, int *h, wchar_t ** mime, ARGB32** imageData)
{
	waServiceFactory *sf;
	svc_imageLoader *loader = FindImageLoader(file, &sf);
	if (loader)
	{
		HANDLE hf = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (hf != INVALID_HANDLE_VALUE)
		{
			*len = GetFileSize(hf, 0);
			HANDLE hmap = CreateFileMapping(hf, 0, PAGE_READONLY, 0, 0, 0);
			if (hmap)
			{
				void *data = MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 0);
				if (data)
				{
					if (loader->testData(data,*len))
					{
						ARGB32* im = loader->loadImage(data,*len,w,h);
						if (im && !_wcsicmp(loader->mimeType(), L"image/jpeg"))
						{
							*mime = _wcsdup(L"jpg");
							*imageData = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(*len);
							memcpy(*imageData, data, *len);
						}
						UnmapViewOfFile(data);
						CloseHandle(hmap);
						CloseHandle(hf);
						sf->releaseInterface(loader);
						return im;
					}
					UnmapViewOfFile(data);
				}
				CloseHandle(hmap);
			}
			CloseHandle(hf);
		}
		sf->releaseInterface(loader);
	}
	return 0;
}

static void * writeImg(const ARGB32 *data, int w, int h, int *length, const wchar_t *ext)
{
	if (!ext || (ext && !*ext)) return NULL;
	if (*ext == L'.') ext++;
	FOURCC imgwrite = svc_imageWriter::getServiceType();
	int n = (int) WASABI_API_SVC->service_getNumServices(imgwrite);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgwrite,i);
		if (sf)
		{
			svc_imageWriter * l = (svc_imageWriter*)sf->getInterface();
			if (l)
			{
				if (wcsstr(l->getExtensions(),ext))
				{
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

static int writeFile(const wchar_t *file, const void * data, int length)
{
	FILE *f=_wfopen(file,L"wb");
	if (!f) return ALBUMART_FAILURE;
	if (fwrite(data,length,1,f) != 1)
	{
		fclose(f);
		return ALBUMART_FAILURE;
	}
	fclose(f);
	return ALBUMART_SUCCESS;
}

static void UpdateArtItemFrame(HWND hwndDlg)
{
	EditArtItem * e = getCurrentArtItem(hwndDlg);
	if (e)
	{
		wchar_t type[100] = {0}, buf[100] = {0}, *uiType = 0;
		int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
		SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_GETTEXT,sel,(LPARAM)type);

		if (e->mimetype)
		{
			uiType = wcschr(e->mimetype, L'/');
			if (uiType && *uiType)
			{
				uiType++;
			}
		}

		int origin[] = {IDS_ORIGIN_NONE, IDS_ORIGIN_EMBEDDED, IDS_ORIGIN_ALBUM_MATCH, IDS_ORIGIN_NFO,
					IDS_ORIGIN_COVER_MATCH, IDS_ORIGIN_FOLDER_MATCH, IDS_ORIGIN_FRONT_MATCH, IDS_ORIGIN_ARTWORK_MATCH};
		StringCchPrintfW(caption,128,getStringW(IDS_ARTWORK_DETAILS, NULL, 0), type, e->w, e->h,
						 // TODO: review what we set as the type for this from pasting...
						 getStringW(origin[2/*ret*/], buf, sizeof(buf)), (uiType && *uiType ? uiType : e->mimetype));

		SetDlgItemTextW(hwndDlg,IDC_STATIC_FRAME, caption);
	}
	else
	{
		SetDlgItemTextW(hwndDlg,IDC_STATIC_FRAME,getStringW(IDS_NO_IMAGE,0,0));
	}
}

static void writeImageToFile(ARGB32 * img, int w, int h, const wchar_t *file)
{
	int length=0;
	void * data = writeImg(img,w,h,&length,wcsrchr(file,L'.'));
	if (data)
	{
		writeFile(file,data,length);
		WASABI_API_MEMMGR->sysFree(data);
	}
}

static INT_PTR CALLBACK FileInfo_Artwork(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			EnableArtFrame(hwndDlg,FALSE);

			// added 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
			// as doing this means we keep the placeholder incase of a change at a
			// later time and without doing anything to require an translation updates
			HWND wnd = GetDlgItem(hwndDlg, IDC_BUTTON_DOWNLOAD);
			if (IsWindow(wnd)) DestroyWindow(wnd);

			info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);
			if (AGAVE_API_ALBUMART)
			{
				int w = 0, h = 0;
				ARGB32 *bits = NULL;
				if (AGAVE_API_ALBUMART->GetAlbumArt(p->filename, L"cover", &w, &h, &bits) == ALBUMART_SUCCESS)
				{
					WASABI_API_MEMMGR->sysFree(bits);
					int i = SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_ADDSTRING,0,(LPARAM)getStringW(IDS_COVER,NULL,0));
					SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETCURSEL,i,0);
					PostMessageW(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_ARTLIST,LBN_SELCHANGE),0);
				}

				wchar_t *types = NULL;
#if 0 // benski> TODO:
				if (AGAVE_API_ALBUMART->GetAlbumArtTypes(p->filename,&types) == ALBUMART_SUCCESS)
				{
					wchar_t *p = types;
					int sel = 0;
					while (p && *p)
					{
						int is_cover = (!_wcsicmp(p,L"cover") || !_wcsicmp(p,getStringW(IDS_COVER,NULL,0)));
						int i = SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_ADDSTRING,0,(LPARAM)(is_cover?getStringW(IDS_COVER,NULL,0):p));
						if (!_wcsicmp(p,L"cover"))
							sel = i;
						p += wcslen(p) + 1;
					}
					WASABI_API_MEMMGR->sysFree(types);

					SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETCURSEL,sel,0);
					PostMessageW(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_ARTLIST,LBN_SELCHANGE),0);
				}
#endif

				if (AGAVE_API_ALBUMART->GetValidAlbumArtTypes(p->filename,&types) == ALBUMART_SUCCESS)
				{
					wchar_t *p = types;
					int sel = 0;
					while (p && *p)
					{
						int is_cover = (!_wcsicmp(p,L"cover") || !_wcsicmp(p,getStringW(IDS_COVER,NULL,0)));
						int i = SendDlgItemMessageW(hwndDlg,IDC_COMBO_ARTTYPE,CB_ADDSTRING,0,(LPARAM)(is_cover?getStringW(IDS_COVER,NULL,0):p));
						if (is_cover || !_wcsicmp(p, config_artwork_filter))
							sel = i;
						p += wcslen(p) + 1;
					}
					WASABI_API_MEMMGR->sysFree(types);
					SendDlgItemMessage(hwndDlg,IDC_COMBO_ARTTYPE,CB_SETCURSEL,sel,0);
				}
			}
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_ARTLIST:
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);
					wchar_t type[100]=L"";
					int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);

					if (sel == -1)
					{
						HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)0);
						if (bmold) DeleteObject(bmold);
						EnableArtFrame(hwndDlg,FALSE);
						return 0;
					}

					EnableArtFrame(hwndDlg,TRUE);
					SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_GETTEXT,sel,(LPARAM)type);

					int w = 0, h = 0;
					ARGB32 *bits = NULL;
					if (AGAVE_API_ALBUMART && AGAVE_API_ALBUMART->GetAlbumArt(p->filename,(!_wcsicmp(type,getStringW(IDS_COVER,NULL,0))?L"cover":type),&w,&h,&bits) == ALBUMART_SUCCESS)
					{
						HBITMAP bm = getBitmap(bits,w,h,228,228,hwndDlg);
						HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bm);
						if (bmold) DeleteObject(bmold);

						wchar_t caption[128], *mimeType = 0, *uiType = 0;
						int origin[] = {IDS_ORIGIN_NONE, IDS_ORIGIN_EMBEDDED, IDS_ORIGIN_ALBUM_MATCH, IDS_ORIGIN_NFO,
										IDS_ORIGIN_COVER_MATCH, IDS_ORIGIN_FOLDER_MATCH, IDS_ORIGIN_FRONT_MATCH, IDS_ORIGIN_ARTWORK_MATCH};
						int ret = AGAVE_API_ALBUMART->GetAlbumArtOrigin(p->filename,(!_wcsicmp(type,getStringW(IDS_COVER,NULL,0))?L"cover":type), &mimeType);
						if (mimeType)
						{
							uiType = wcschr(mimeType, L'/');
							if (uiType && *uiType)
							{
								uiType++;
							}
						}

						EditArtItem *e = new EditArtItem(bits,w,h);
						if (e)
						{
							size_t len = 0;
							ARGB32 *data = NULL;
							if (AGAVE_API_ALBUMART->GetAlbumArtData(p->filename, L"cover", (void**)&data, &len, NULL) == ALBUMART_SUCCESS)
							{
								e->data = data;
								e->datalen = len;
							}

							wchar_t mime[32] = {L"image/jpeg"};
							if (uiType && *uiType)
							{
								StringCchPrintfW(mime, 32, L"image/%s", uiType);
							}
							else if(mimeType && *mimeType && _wcsicmp(mimeType, L"image/jpeg"))
							{
								lstrcpynW(mime, mimeType, 32);
							}
							if (mime && *mime) e->mimetype = _wcsdup(mime);

							EditArtItem *eold = (EditArtItem *)SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETITEMDATA,sel,0);
							if (eold && eold != (EditArtItem *)-1) delete eold;
							SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_SETITEMDATA,sel,(LPARAM)e);
						}

						wchar_t buf[100] = {0}, buf2[32] = {0};
						StringCchPrintfW(caption,128,getStringW(IDS_ARTWORK_DETAILS, NULL, 0), type, w, h,
										 getStringW(origin[ret], buf, ARRAYSIZE(buf)),
										 (uiType && *uiType ? uiType :
										 (mimeType && *mimeType ? mimeType :
										 getStringW(IDS_UNKNOWN_MIME, buf2, ARRAYSIZE(buf2)))));

						SetDlgItemTextW(hwndDlg,IDC_STATIC_FRAME, caption);

						WASABI_API_MEMMGR->sysFree(mimeType);
					}
				}
				break;
			// disabled 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
			#if 0
			case IDC_BUTTON_DOWNLOAD:
			{
				wchar_t artist[256]=L"";
				wchar_t album[256]=L"";
				info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);
				GetDlgItemTextW(p->tabs[0],IDC_ALBUM_ARTIST,artist,256);
				if (!artist[0]) GetDlgItemTextW(p->tabs[0],IDC_ARTIST,artist,256);
				GetDlgItemTextW(p->tabs[0],IDC_ALBUM,album,256);

				artFetchData d = {sizeof(d),hwndDlg,artist,album,0};
				int r = (int)SendMessageW(hMainWindow,WM_WA_IPC,(LPARAM)&d,IPC_FETCH_ALBUMART);
				if (r == -2) break; // cancel all was pressed
				if (r == 0 && d.imgData && d.imgDataLen) // success
				{
					if (AddNewArtItem(hwndDlg))
					{
						bool success=false;
						EditArtItem *e = getCurrentArtItem(hwndDlg);
						if (e)
						{
							int w=0,h=0;
							ARGB32* data = decompressImage(d.imgData,d.imgDataLen,&w,&h);
							if (data)
							{
								if (e->bits)
								{
									WASABI_API_MEMMGR->sysFree(e->bits);
									e->bits = 0;
								}
								if (e->data)
								{
									WASABI_API_MEMMGR->sysFree(e->data);
									e->data = 0;
									e->datalen = 0;
								}
								e->bits = data;
								e->w = w;
								e->h = h;
								if (e->data) WASABI_API_MEMMGR->sysFree(e->data);
								e->data = d.imgData;
								d.imgData = 0;
								e->datalen = d.imgDataLen;
								if (e->mimetype) free(e->mimetype);
								e->mimetype = _wcsdup(L"jpg");
								e->dirty = true;
								HBITMAP dispbm = getBitmap(e->bits,e->w,e->h,228,228,hwndDlg);
								HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)dispbm);
								if (bmold) DeleteObject(bmold);
								success=true;
							}
						}
						if (!success)
						{
							// fail, remove :(
							int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
							if (sel == -1)
								return 0;
							if (e) delete e;
							SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_DELETESTRING,sel,0);
							EnableArtFrame(hwndDlg,0);
						}
					}
					if (d.imgData)
						WASABI_API_MEMMGR->sysFree(d.imgData);
				}
			}
			break;
			#endif
			case IDC_BUTTON_LOAD:
				if (AddNewArtItem(hwndDlg))
				{
					if (!FileInfo_Artwork(hwndDlg,WM_COMMAND,IDC_BUTTON_CHANGE,0))
					{
						// fail, remove :(
						int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
						if (sel == -1)
							return 0;
						EditArtItem * e = getCurrentArtItem(hwndDlg);
						if (e) delete e;
						SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_DELETESTRING,sel,0);
						EnableArtFrame(hwndDlg,0);
					}
				}
				break;
			case IDC_BUTTON_CHANGE:
			{
				EditArtItem *e = getCurrentArtItem(hwndDlg);
				if (!e) break;
				info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);
				wchar_t file[1024]=L"", folder[MAX_PATH]=L"";
				OPENFILENAMEW fn = {sizeof(OPENFILENAMEW),0};
				// set the ofd to the current file's folder
				lstrcpynW(folder,p->filename,MAX_PATH);
				PathRemoveFileSpecW(folder);
				PathAddBackslashW(folder);
				fn.lpstrInitialDir = folder;
				fn.hwndOwner = hwndDlg;
				fn.lpstrFile = file;
				fn.nMaxFile = 1024;

				static wchar_t fileExtensionsString[MAX_PATH] = {0};
				if(!fileExtensionsString[0])
				{
					getStringW(IDS_IMAGE_FILES,fileExtensionsString,MAX_PATH);
					wchar_t *temp=fileExtensionsString+lstrlenW(fileExtensionsString) + 1;

					// query the available image loaders and build it against the supported formats
					FOURCC imgload = svc_imageLoader::getServiceType();
					int n = (int) WASABI_API_SVC->service_getNumServices(imgload);
					for (int i=0; i<n; i++)
					{
						waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
						if (sf)
						{
							svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
							if (l)
							{
								wchar_t *tests[] = {L"*.jpg",L"*.jpeg",L"*.png",L"*.gif",L"*.bmp"};
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

				fn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOVALIDATE;
				if (GetOpenFileNameW(&fn))
				{
					int len = 0, w = 0, h = 0;
					wchar_t * mime = 0;
					// TODO: benski> save original bits and mime type
					// UPDATE: will only grab the raw data for jpeg files at the moment
					ARGB32 * data = 0, * bits = loadImgFromFile(file,&len,&w,&h,&mime,&data);
					if (bits)
					{
						if (e->bits)
						{
							WASABI_API_MEMMGR->sysFree(e->bits);
							e->bits = 0;
						}
						if (e->data)
						{
							WASABI_API_MEMMGR->sysFree(e->data);
							e->data = 0;
							e->datalen = 0;
						}
						e->bits = bits;
						e->w = w;
						e->h = h;
						if (data)
						{
							e->data = data;
							e->datalen = len;
						}
						if (e->mimetype) free(e->mimetype);
						e->mimetype = mime;
						e->dirty = true;
						HBITMAP dispbm = getBitmap(e->bits,e->w,e->h,228,228,hwndDlg);
						HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)dispbm);
						if (bmold) DeleteObject(bmold);
						return 1;
					}
				}
				break;
			}
			case IDC_BUTTON_SAVEAS:
			{
				EditArtItem *e = getCurrentArtItem(hwndDlg);
				if (!e) break;
				info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);
				wchar_t file[1024]=L"", folder[MAX_PATH]=L"";
				OPENFILENAMEW fn = {sizeof(OPENFILENAMEW),0};
				// set the ofd to the current file's folder
				lstrcpynW(folder,p->filename,MAX_PATH);
				PathRemoveFileSpecW(folder);
				PathAddBackslashW(folder);
				fn.lpstrInitialDir = folder;
				fn.hwndOwner = hwndDlg;
				fn.lpstrFile = file;
				fn.nMaxFile = 1020;

				static wchar_t *mimes[4] = {0};
				wchar_t *tests[] = {L"*.jpg",L"*.png",L"*.gif",L"*.bmp"};
				static int tests_idx[4] = {0,1,2,3}, tests_run = 0, last_filter = -1;
				static wchar_t filter[1024] = {0}, *sff = filter;

				if(!tests_run)
				{
					int def_idx = 0;
					tests_run = 1;
					FOURCC imgload = svc_imageLoader::getServiceType();
					int n = (int) WASABI_API_SVC->service_getNumServices(imgload);
					for (int i = 0, j = 0; i < n; i++)
					{
						waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
						if (sf)
						{
							svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
							if (l)
							{
								int tests_str[] = {IDS_JPEG_FILE,IDS_PNG_FILE,IDS_GIF_FILE,IDS_BMP_FILE};
								size_t size = 1024;
								for(int k = 0; k < ARRAYSIZE(tests); k++)
								{
									if (l->isMine(tests[k]))
									{
										if (!k) def_idx = tests_idx[j] + 1;
										tests_idx[j] = k;
										mimes[j] = _wcsdup(l->mimeType());
										j++;
										int len = 0;
										getStringW(tests_str[k],sff,size);
										size-=(len = lstrlenW(sff)+1);
										sff+=len;
										lstrcpynW(sff,tests[k], (int)size);
										if (!k) lstrcatW(sff, L";*.jpeg");
										size-=(len = lstrlenW(sff)+1);
										sff+=len;
									}
								}
								sf->releaseInterface(l);
							}
						}
					}

					last_filter = _r_i("art_flt",last_filter);
					if (last_filter == -1) last_filter = def_idx;
				}

				fn.lpstrFilter = filter;
				fn.nFilterIndex = last_filter;	// default to *.jpg / last filter
				fn.Flags = OFN_OVERWRITEPROMPT;
				if (GetSaveFileNameW(&fn))
				{
					int l = (int) wcslen(file);
					if (l>4 && file[l-4]==L'.'); // we have an extention
					else StringCchCatW(file,1024,tests[tests_idx[fn.nFilterIndex-1]]+1); // map to the extension to use

					// where possible see if we can just save the image data without conversion
					if (e->data && e->mimetype && !_wcsicmp(mimes[fn.nFilterIndex-1], e->mimetype))
					{
						writeFile(file, e->data, (int)e->datalen);
					}
					else
					{
						// though if we're not sure or it is a different format from what is stored
						// then we'll need to convert (not nice to do) and hope it doesn't fail...
						writeImageToFile(e->bits,e->w,e->h,file);
					}
				}

				last_filter = fn.nFilterIndex;
				_w_i("art_flt",last_filter);
				break;
			}
			case IDC_BUTTON_COPY:
			{
				EditArtItem *e = getCurrentArtItem(hwndDlg);
				if (!e) break;
				if (!OpenClipboard(hwndDlg)) break;
				EmptyClipboard();
				HBITMAP bm = getBitmap(e->bits,e->w,e->h,hwndDlg);
				SetClipboardData(CF_BITMAP,bm);
				CloseClipboard();
				DeleteObject(bm);
				break;
			}
			case IDC_BUTTON_PASTENEW:
				if (AddNewArtItem(hwndDlg))
				{
					if (!FileInfo_Artwork(hwndDlg,WM_COMMAND,IDC_BUTTON_PASTE,0))
					{
						// fail, remove :(
						int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
						if (sel == -1)
							return 0;
						EditArtItem * e = getCurrentArtItem(hwndDlg);
						if (e) delete e;
						SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_DELETESTRING,sel,0);
						EnableArtFrame(hwndDlg,0);
					}
				}
				UpdateArtItemFrame(hwndDlg);
				break;
			case IDC_BUTTON_PASTE:
			{
				EditArtItem *e = getCurrentArtItem(hwndDlg);
				if (!e) break;
				if (!OpenClipboard(hwndDlg)) break;
				HBITMAP bm = (HBITMAP)GetClipboardData(CF_BITMAP);
				if (bm)
				{
					GetSize(bm,e->w,e->h,hwndDlg);
					BITMAPINFO info={0};
					info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					info.bmiHeader.biWidth = e->w;
					info.bmiHeader.biHeight = -e->h;
					info.bmiHeader.biPlanes = 1;
					info.bmiHeader.biBitCount = 32;
					info.bmiHeader.biCompression = BI_RGB;
					HDC dc = GetDC(hwndDlg);
					if (e->bits)
					{
						WASABI_API_MEMMGR->sysFree(e->bits);
						e->bits = 0;
					}
					if (e->data)
					{
						WASABI_API_MEMMGR->sysFree(e->data);
						e->data = 0;
						e->datalen = 0;
					}
					e->bits = (ARGB32*)WASABI_API_MEMMGR->sysMalloc(e->w*e->h*sizeof(ARGB32));
					if (e->mimetype) free(e->mimetype);
					e->mimetype = _wcsdup(L"image/jpeg");
					GetDIBits(dc,bm,0,e->h,e->bits,&info,DIB_RGB_COLORS);
					ReleaseDC(hwndDlg,dc);
					e->dirty=true;
					HBITMAP dispbm = getBitmap(e->bits,e->w,e->h,228,228,hwndDlg);
					HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)dispbm);
					if (bmold) DeleteObject(bmold);
					CloseClipboard();
					UpdateArtItemFrame(hwndDlg);
					return 1;
				}
				CloseClipboard();
				break;
			}
			case IDC_BUTTON_DELETE:
			{
				wchar_t buf[1024] = {0};
				getStringW(IDS_ARTDELETE,buf,1024);
				if (MessageBoxW(hwndDlg,buf,getStringW(IDS_AREYOUSURE,0,0),MB_YESNO|MB_ICONQUESTION) != IDYES) break;
				info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);
				EditArtItem *e = getCurrentArtItem(hwndDlg);
				if (!e) break;
				int sel = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCURSEL,0,0);
				buf[0]=0;
				SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_GETTEXT,sel,(LPARAM)buf);
				AGAVE_API_ALBUMART->DeleteAlbumArt(p->filename,(!_wcsicmp(buf,getStringW(IDS_COVER,NULL,0))?L"cover":buf));
				HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)0);
				if (bmold) DeleteObject(bmold);
				delete e;
				SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_DELETESTRING,sel,0);
				EnableArtFrame(hwndDlg,0);
				break;
			}
			case IDOK:
			{
				info_params * p = (info_params *)GetWindowLongPtrW(GetParent(hwndDlg),GWLP_USERDATA);
				int l = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCOUNT,0,0);
				for (int i=0; i<l; i++)
				{
					EditArtItem *e = (EditArtItem *)SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETITEMDATA,i,0);
					if (e && e->dirty)
					{
						wchar_t buf[1024] = {0};
						SendDlgItemMessageW(hwndDlg,IDC_ARTLIST,LB_GETTEXT,i,(LPARAM)buf);
						if (e->data)
						{
							AGAVE_API_ALBUMART->SetAlbumArt(p->filename,(!_wcsicmp(buf,getStringW(IDS_COVER,NULL,0))?L"cover":buf),
															0,0,e->data,e->datalen,e->mimetype);
						}
						else
						{
							AGAVE_API_ALBUMART->SetAlbumArt(p->filename,(!_wcsicmp(buf,getStringW(IDS_COVER,NULL,0))?L"cover":buf),
															e->w,e->h,e->bits,0,0);
						}
					}
				}
			}
			break;
			}
			break;
		case WM_DESTROY:
		{
			HBITMAP bmold = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_ARTHOLDER,STM_GETIMAGE,IMAGE_BITMAP,0);
			if (bmold) DeleteObject(bmold);
			int l = SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETCOUNT,0,0);
			for (int i=0; i<l; i++)
			{
				EditArtItem *e = (EditArtItem *)SendDlgItemMessage(hwndDlg,IDC_ARTLIST,LB_GETITEMDATA,i,0);
				if (e && e != (EditArtItem *)-1) delete e;
			}

			GetDlgItemTextW(hwndDlg, IDC_COMBO_ARTTYPE, config_artwork_filter, ARRAYSIZE(config_artwork_filter));
		}
		break;
	}
	return 0;
}

static int checkEditInfoClick(HWND hwndDlg, POINT p, int item, int check)
{
	RECT r = {0};
	GetWindowRect(GetDlgItem(hwndDlg, item), &r);
	ScreenToClient(hwndDlg, (LPPOINT)&r);
	ScreenToClient(hwndDlg, (LPPOINT)&r.right);
	if (PtInRect(&r, p) && !IsDlgButtonChecked(hwndDlg, check))
	{
		CheckDlgButton(hwndDlg, check, TRUE);
		if (item == IDC_COMBO_RATING) SendDlgItemMessage(hwndDlg, IDC_COMBO_RATING, CB_SHOWDROPDOWN, TRUE, 0);
		EnableWindow(GetDlgItem(hwndDlg, item), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
		PostMessageW(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, item), (LPARAM)TRUE);
		return 1;
	}
	return 0;
}

// sets part and parts to -1 or 0 on fail/missing (e.g. parts will be -1 on "1", but 0 on "1/")
void ParseIntSlashInt(wchar_t *string, int *part, int *parts)
{
	*part = -1;
	*parts = -1;

	if (string && string[0])
	{
		*part = _wtoi(string);
		while (string && *string && *string != '/')
		{
			string++;
		}
		if (string && *string == '/')
		{
			string++;
			*parts = _wtoi(string);
		}
	}
}

typedef struct
{
	mlQueryStructW item;
	int idx;
	int ml;
} queryStructW;

std::vector<queryStructW*> queryList;
static int got_local_ml = -1;
static size_t m_upd_nb, m_stopped, m_upd_nb_all, m_upd_nb_cur;
#define MAKESAFE(x) ((x)?(x):L"")

static void FreeQueryList()
{
	for ( queryStructW *query : queryList )
	{
		free( query->item.query );

		if ( query->ml )
			sendMlIpc( ML_IPC_DB_FREEQUERYRESULTSW, (WPARAM)query );
		else
			freeRecordList( &query->item.results );

		free( query );
	}

	queryList.clear();
}

void CHECK_AND_COPY(itemRecordW *song, HWND hwndParent, int IDCHECK, int ID, wchar_t *&item)
{
	if (IsDlgButtonChecked(hwndParent, IDCHECK)) {
		wchar_t blah[2048];
		GetDlgItemTextW(hwndParent, ID, blah, 2048);
		if (wcscmp(MAKESAFE(item), blah)) 
		{
			free(item);
			item = _wcsdup(blah);
		}
	}
}

static void CHECK_AND_COPY_EXTENDED(itemRecordW *song, HWND hwndParent, int IDCHECK, int ID, const wchar_t *name)
{
	if (IsDlgButtonChecked(hwndParent, IDCHECK)) {
		wchar_t blah[2048]; 
		GetDlgItemTextW(hwndParent, ID, blah, 2048);
		wchar_t *oldData = getRecordExtendedItem(song, name);
		if (wcscmp(MAKESAFE(oldData), blah)) {
			setRecordExtendedItem(song, name, blah);
		}
	}
}

static INT_PTR CALLBACK updateFiles_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SetWindowTextW(hwndDlg, getStringW(IDS_UPDATING_FILES, NULL, 0));
			SetWindowLong(GetDlgItem(hwndDlg, 1008), GWL_STYLE, (GetWindowLongW(GetDlgItem(hwndDlg, 1008), GWL_STYLE)&~SS_CENTER) | SS_LEFTNOWORDWRAP);
			SetTimer(hwndDlg, 0x123, 30, NULL);
			m_upd_nb = 0;
			m_stopped = 0;
			SendDlgItemMessage(hwndDlg, 1007, PBM_SETRANGE, 0, MAKELPARAM(0, m_upd_nb_all));
			m_upd_nb_cur = 0;
			break;
		}
		case WM_TIMER:
			if (wParam == 0x123 && !m_stopped)
			{
				unsigned int start_t = GetTickCount();
again:
				{
					if (m_upd_nb >= queryList.size())
					{
						//done
						if (got_local_ml == 1) sendMlIpc(ML_IPC_DB_SYNCDB, 0);
						EndDialog(hwndDlg, 1);
						break;
					}

					int i = (int) m_upd_nb++;
					itemRecordW *mlsong = &queryList[i]->item.results.Items[0];
					itemRecordW copy;
					itemRecordW *song = &copy;
					copyRecord(&copy, mlsong);
					HWND hwndParent = GetParent(hwndDlg);

					wchar_t stattmp[512] = {0};
					wchar_t *p = scanstr_backW(song->filename, L"\\", song->filename - 1) + 1;
					wsprintfW(stattmp, getStringW(IDS_UPDATING_X, NULL, 0), p);
					SetDlgItemTextW(hwndDlg, 1008, stattmp);

					SendDlgItemMessage(hwndDlg, 1007, PBM_SETPOS, m_upd_nb_cur, 0);
					m_upd_nb_cur++;

					int updtagz = !!IsDlgButtonChecked(hwndParent, 1052);

					CHECK_AND_COPY(song, hwndParent, IDC_CHECK_ARTIST,      IDC_EDIT_ARTIST, song->artist);
					CHECK_AND_COPY(song, hwndParent, IDC_CHECK_TITLE,       IDC_EDIT_TITLE, song->title);
					CHECK_AND_COPY(song, hwndParent, IDC_CHECK_ALBUM,       IDC_EDIT_ALBUM, song->album);
					CHECK_AND_COPY(song, hwndParent, IDC_CHECK_COMMENT,     IDC_EDIT_COMMENT, song->comment);
					CHECK_AND_COPY(song, hwndParent, IDC_CHECK_GENRE,       IDC_EDIT_GENRE, song->genre);
					CHECK_AND_COPY(song, hwndParent, IDC_CHECK_ALBUMARTIST, IDC_EDIT_ALBUMARTIST, song->albumartist);
					CHECK_AND_COPY(song, hwndParent, IDC_CHECK_PUBLISHER,   IDC_EDIT_PUBLISHER, song->publisher);
					CHECK_AND_COPY(song, hwndParent, IDC_CHECK_COMPOSER,    IDC_EDIT_COMPOSER, song->composer);
					CHECK_AND_COPY(song, hwndParent, IDC_CHECK_CATEGORY,    IDC_EDIT_CATEGORY, song->category);

					CHECK_AND_COPY_EXTENDED(song, hwndParent, IDC_CHECK_DIRECTOR,			IDC_EDIT_DIRECTOR,			L"director");
					CHECK_AND_COPY_EXTENDED(song, hwndParent, IDC_CHECK_PRODUCER,			IDC_EDIT_PRODUCER,			L"producer");
					CHECK_AND_COPY_EXTENDED(song, hwndParent, IDC_CHECK_PODCAST_CHANNEL,	IDC_EDIT_PODCAST_CHANNEL,	L"podcastchannel");

#define CHECK_AND_COPY_EXTENDED_PC(IDCHECK, field, name) \
						wchar_t blah[2048] = {0}; StringCchPrintfW(blah, 2048, L"%d", (IsDlgButtonChecked(hwndParent, IDCHECK) == BST_CHECKED));\
						wchar_t *oldData = getRecordExtendedItem(song, name);\
						if (wcscmp(MAKESAFE(oldData), blah)) { setRecordExtendedItem(song, name, blah); }

					CHECK_AND_COPY_EXTENDED_PC(IDC_CHECK_PODCAST,	MAINTABLE_ID_ISPODCAST,	L"ispodcast");

					if (IsDlgButtonChecked(hwndParent, IDC_CHECK_TRACK))
					{
						wchar_t blah[64] = {0};
						GetDlgItemTextW(hwndParent, IDC_EDIT_TRACK, blah, 64);
						int track, tracks;
						ParseIntSlashInt(blah, &track, &tracks);
						if (tracks <= 0) tracks = -1;
						if (track <= 0) track = -1;

						if (song->track != track || song->tracks != tracks)
						{
							song->track = track;
							song->tracks = tracks;
						}
					}

					if (IsDlgButtonChecked(hwndParent, IDC_CHECK_DISC))
					{
						wchar_t blah[64] = {0};
						GetDlgItemTextW(hwndParent, IDC_EDIT_DISC, blah, 64);
						int disc, discs;
						ParseIntSlashInt(blah, &disc, &discs);
						if (discs <= 0) discs = -1;
						if (disc <= 0) disc = -1;

						if (song->disc != disc || song->discs != discs)
						{
							song->disc = disc;
							song->discs = discs;
						}
					}

					if (IsDlgButtonChecked(hwndParent, IDC_CHECK_YEAR))
					{
						char blah[64] = {0};
						GetDlgItemTextA(hwndParent, IDC_EDIT_YEAR, blah, 64);
						int n = atoi(blah);
						if (n <= 0) n = -1;
						if (song->year != n) song->year = n;
					}

					if (IsDlgButtonChecked(hwndParent, IDC_CHECK_BPM))
					{
						char blah[64] = {0};
						GetDlgItemTextA(hwndParent, IDC_EDIT_BPM, blah, 64);
						int n = atoi(blah);
						if (n <= 0) n = -1;
						if (song->bpm != n) song->bpm = n;
					}

					if (IsDlgButtonChecked(hwndParent, IDC_CHECK_RATING))
					{
						int n = SendDlgItemMessage(hwndParent, IDC_COMBO_RATING, CB_GETCURSEL, 0, 0), rating = -1;
						if (n != CB_ERR && (n >= 0 && n < 5)) rating = 5 - n;
						if (song->rating != rating) song->rating = rating;
					}

					// no need to send this if there's no ml present
					if (got_local_ml == 1) sendMlIpc((!config_upd_mode ? ML_IPC_DB_ADDORUPDATEITEMW : ML_IPC_DB_UPDATEITEMW), (WPARAM)song);

					if (updtagz || !got_local_ml)
					{
						m_stopped = 1;
retry:
						if (in_set_extended_fileinfoW(song->filename, L"title", song->title)) // if this returns 0, then this format doesnt even support extended
						{
							in_set_extended_fileinfoW(song->filename, L"artist", song->artist);
							in_set_extended_fileinfoW(song->filename, L"album", song->album);
							in_set_extended_fileinfoW(song->filename, L"comment", song->comment);
							in_set_extended_fileinfoW(song->filename, L"genre", song->genre);

							wchar_t buf[32] = {0};
							if (song->track > 0)
							{
								if (song->tracks > 0)
									wsprintfW(buf, L"%d/%d", song->track, song->tracks);
								else
									wsprintfW(buf, L"%d", song->track);
							}
							else buf[0] = 0;
							in_set_extended_fileinfoW(song->filename, L"track", buf);

							if (song->year > 0) wsprintfW(buf, L"%d", song->year);
							else buf[0] = 0;
							in_set_extended_fileinfoW(song->filename, L"year", buf);

							if (song->disc > 0)
							{
								if (song->discs > 0)
									wsprintfW(buf, L"%d/%d", song->disc, song->discs);
								else
									wsprintfW(buf, L"%d", song->disc);
							}
							else buf[0] = 0;
							in_set_extended_fileinfoW(song->filename, L"disc", buf);

							if (song->rating > 0) wsprintfW(buf, L"%d", song->rating);
							else buf[0] = 0;
							if (!in_set_extended_fileinfoW(song->filename, L"rating", buf))
							{
								// for ratings, try using the library just to cover all bases for format types
								file_set_ratingW rate = {0};
								rate.fileName = song->filename;
								rate.newRating = song->rating;
								sendMlIpc(ML_IPC_SET_FILE_RATINGW, (WPARAM)&rate);
							}

							if (song->bpm > 0) wsprintfW(buf, L"%d", song->bpm);
							else buf[0] = 0;
							in_set_extended_fileinfoW(song->filename, L"bpm", buf);

							in_set_extended_fileinfoW(song->filename, L"albumartist", song->albumartist);
							in_set_extended_fileinfoW(song->filename, L"publisher", song->publisher);
							in_set_extended_fileinfoW(song->filename, L"composer", song->composer);
							in_set_extended_fileinfoW(song->filename, L"category", song->category);
							in_set_extended_fileinfoW(song->filename, L"director", getRecordExtendedItem(song, L"director"));
							in_set_extended_fileinfoW(song->filename, L"producer", getRecordExtendedItem(song, L"producer"));

							if (in_write_extended_fileinfo() == 0)
							{
								wchar_t tmp[1024] = {0};
								wsprintfW(tmp, getStringW(IDS_ERROR_UPDATING_FILE, NULL, 0), song->filename);
								int ret = MessageBoxW(hwndDlg, tmp, getStringW(IDS_INFO_UPDATING_ERROR, NULL, 0), MB_RETRYCANCEL);
								if (ret == IDRETRY) goto retry;
								if (ret == IDCANCEL)
								{
									EndDialog(hwndDlg, 0);
									freeRecord(&copy);
									break;
								}
							}
						}

						int x = queryList[i]->idx;
						SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)song->filename, IPC_FILE_TAG_MAY_HAVE_UPDATEDW);
						PlayList_setitem(x, song->filename, PlayList_gettitle(song->filename, 1));
						PlayList_setlastlen(x);

						m_stopped = 0;
					}
					freeRecord(&copy);
				}
				if (GetTickCount() - start_t < 30) goto again;
			}
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hwndDlg, 0);
			}
			break;
	}
	return FALSE;
}

INT_PTR CALLBACK EditInfo(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			wchar_t *last_artist = NULL, *last_title = NULL, *last_album = NULL, *last_genre = NULL,
					*last_comment = NULL, *last_albumartist = NULL, *last_composer = NULL,
					*last_publisher = NULL, *last_ispodcast = NULL, *last_podcastchannel = NULL;
			const wchar_t *last_category = NULL, *last_director = NULL, *last_producer = NULL;
			int last_year = -1, last_track = -1, last_disc = -1, last_discs = -1, last_tracks = -1,
				last_bpm = -1, last_rating = -1, disable_artist = 0, disable_title = 0,
				disable_album = 0, disable_genre = 0, disable_year = 0, disable_track = 0,
				disable_comment = 0, disable_disc = 0, disable_albumartist = 0, disable_composer = 0,
				disable_publisher = 0, disable_discs = 0, disable_tracks = 0, disable_category = 0,
				disable_director = 0, disable_producer = 0, disable_bpm = 0, disable_rating = 0,
				disable_ispodcast = 0, disable_podcastchannel = 0, nb = 0;

			if (got_local_ml == -1)
			{
				got_local_ml = (got_ml ? !!GetModuleHandleW(L"ml_local.dll") : 0);
			}

			if (got_local_ml == 1)
			{
				if (GetPrivateProfileIntW(L"gen_ml_config", L"upd_tagz", 1, ML_INI_FILE))
					CheckDlgButton(hwndDlg, 1052, BST_CHECKED);
			}
			else
				ShowWindow(GetDlgItem(hwndDlg, 1052), SW_HIDE);

			EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

			SendDlgItemMessageW(hwndDlg, IDC_COMBO_RATING, CB_ADDSTRING, 0, (LPARAM)L"\u2605\u2605\u2605\u2605\u2605");
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_RATING, CB_ADDSTRING, 0, (LPARAM)L"\u2605\u2605\u2605\u2605");
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_RATING, CB_ADDSTRING, 0, (LPARAM)L"\u2605\u2605\u2605");
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_RATING, CB_ADDSTRING, 0, (LPARAM)L"\u2605\u2605");
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_RATING, CB_ADDSTRING, 0, (LPARAM)L"\u2605");
			SendDlgItemMessageW(hwndDlg, IDC_COMBO_RATING, CB_ADDSTRING, 0, (LPARAM)getStringW(IDS_NO_RATING, NULL, 0));

			FreeQueryList();
			LPWSTR pszBuffer = (LPWSTR)calloc(TEXTBUFFER_MAX, sizeof(WCHAR));
			if (pszBuffer)
			{
				for (int i = 0; i < PlayList_getlength(); i++)
				{
					wchar_t fn[FILENAME_SIZE] = {0};
					if (PlayList_getselect2(i, fn))
					{
						if (!PathIsURLW(fn) || !_wcsnicmp(fn, L"cda://", 6))
						{
							queryStructW *query = (queryStructW *)calloc(1, sizeof(queryStructW));
							if (query)
							{
								queryList.push_back(query);
								query->item.query = _wcsdup(fn);

								// hit the library where possible as it'll be faster (and more integrated)
								if (got_local_ml == 1)
								{
									if (sendMlIpc(ML_IPC_DB_RUNQUERY_FILENAMEW, (WPARAM)query) == 1)
									{
#define SAVE_LAST_STR(last, check, disable) if (!disable && check && check[0]) { if (!last) last = check; else if (wcscmp(check, last)) disable = 1; }
#define SAVE_LAST_INT(last, check, disable) if (!disable && check > 0) { if (last == -1) last = check; else if (last != check) disable = 1; }

										SAVE_LAST_STR(last_artist, query->item.results.Items[0].artist, disable_artist);
										SAVE_LAST_STR(last_title, query->item.results.Items[0].title, disable_title);
										SAVE_LAST_STR(last_album, query->item.results.Items[0].album, disable_album);
										SAVE_LAST_STR(last_comment, query->item.results.Items[0].comment, disable_comment);
										SAVE_LAST_STR(last_genre, query->item.results.Items[0].genre, disable_genre);

										SAVE_LAST_INT(last_year, query->item.results.Items[0].year, disable_year);
										SAVE_LAST_INT(last_track, query->item.results.Items[0].track, disable_track);
										SAVE_LAST_INT(last_tracks, query->item.results.Items[0].tracks, disable_tracks);
										SAVE_LAST_INT(last_disc, query->item.results.Items[0].disc, disable_disc);
										SAVE_LAST_INT(last_discs, query->item.results.Items[0].discs, disable_discs);
										SAVE_LAST_INT(last_rating, query->item.results.Items[0].rating, disable_rating);
										SAVE_LAST_INT(last_bpm, query->item.results.Items[0].bpm, disable_bpm);

										SAVE_LAST_STR(last_albumartist, query->item.results.Items[0].albumartist, disable_albumartist);
										SAVE_LAST_STR(last_composer, query->item.results.Items[0].composer, disable_composer);
										SAVE_LAST_STR(last_publisher, query->item.results.Items[0].publisher, disable_publisher);
										SAVE_LAST_STR(last_category, query->item.results.Items[0].category, disable_category);

#define SAVE_LAST_STR_EXTENDED(last, name, disable) if (!disable) { wchar_t *check = getRecordExtendedItem(&query->item.results.Items[0], name); if (check && check[0]) { if (!last) last = check; else if (wcscmp(check, last)) disable = 1; }};

										SAVE_LAST_STR_EXTENDED(last_director, L"director", disable_director);
										SAVE_LAST_STR_EXTENDED(last_producer, L"producer", disable_producer);
										SAVE_LAST_STR_EXTENDED(last_podcastchannel, L"podcastchannel", disable_podcastchannel);
										SAVE_LAST_STR_EXTENDED(last_ispodcast, L"ispodcast", disable_ispodcast);
										nb++;
										query->ml = 1;
										query->idx = i;
									}
									else
									{
										goto non_ml;
									}
								}
								else
								{
non_ml:
									allocRecordList(&query->item.results, 1, 0);
									query->item.results.Items[0].filename = _wcsdup(fn);

#define SAVE_LAST_STR(last, check, disable) if (!disable && check && check[0]) { if (!last) last = check; else if (wcscmp(check, last)) disable = 1; }
#define SAVE_LAST_INT(last, check, disable) if (!disable && check > 0) { if (last == -1) last = check; else if (last != check) disable = 1; }

									if (in_get_extended_fileinfoW(fn, L"artist", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].artist = _wcsdup(pszBuffer);
									SAVE_LAST_STR(last_artist, query->item.results.Items[0].artist, disable_artist);

									if (in_get_extended_fileinfoW(fn, L"title", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].title = _wcsdup(pszBuffer);
									SAVE_LAST_STR(last_title, query->item.results.Items[0].title, disable_title);

									if (in_get_extended_fileinfoW(fn, L"album", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].album = _wcsdup(pszBuffer);
									SAVE_LAST_STR(last_album, query->item.results.Items[0].album, disable_album);

									if (in_get_extended_fileinfoW(fn, L"comment", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].comment = _wcsdup(pszBuffer);
									SAVE_LAST_STR(last_comment, query->item.results.Items[0].comment, disable_comment);

									if (in_get_extended_fileinfoW(fn, L"genre", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].genre = _wcsdup(pszBuffer);
									SAVE_LAST_STR(last_genre, query->item.results.Items[0].genre, disable_genre);

									if (in_get_extended_fileinfoW(fn, L"year", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].year = _wtoi(pszBuffer);
									SAVE_LAST_INT(last_year, query->item.results.Items[0].year, disable_year);

									if (in_get_extended_fileinfoW(fn, L"track", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].track = _wtoi(pszBuffer);
									SAVE_LAST_INT(last_track, query->item.results.Items[0].track, disable_track);

									if (in_get_extended_fileinfoW(fn, L"tracks", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].tracks = _wtoi(pszBuffer);
									SAVE_LAST_INT(last_tracks, query->item.results.Items[0].tracks, disable_tracks);

									if (in_get_extended_fileinfoW(fn, L"disc", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].disc = _wtoi(pszBuffer);
									SAVE_LAST_INT(last_disc, query->item.results.Items[0].disc, disable_disc);

									if (in_get_extended_fileinfoW(fn, L"discs", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].discs = _wtoi(pszBuffer);
									SAVE_LAST_INT(last_discs, query->item.results.Items[0].discs, disable_discs);

									// for ratings, try using the library just to cover all bases for format types
									if (in_get_extended_fileinfoW(fn, L"rating", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].rating = _wtoi(pszBuffer);
									else
										query->item.results.Items[0].rating = sendMlIpc(ML_IPC_GET_FILE_RATINGW, (WPARAM)fn);
									SAVE_LAST_INT(last_rating, query->item.results.Items[0].rating, disable_rating);

									if (in_get_extended_fileinfoW(fn, L"bpm", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].bpm = _wtoi(pszBuffer);
									SAVE_LAST_INT(last_bpm, query->item.results.Items[0].bpm, disable_bpm);

									if (in_get_extended_fileinfoW(fn, L"albumartist", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].albumartist = _wcsdup(pszBuffer);
									SAVE_LAST_STR(last_albumartist, query->item.results.Items[0].albumartist, disable_albumartist);

									if (in_get_extended_fileinfoW(fn, L"composer", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].composer = _wcsdup(pszBuffer);
									SAVE_LAST_STR(last_composer, query->item.results.Items[0].composer, disable_composer);

									if (in_get_extended_fileinfoW(fn, L"publisher", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].publisher = _wcsdup(pszBuffer);
									SAVE_LAST_STR(last_publisher, query->item.results.Items[0].publisher, disable_publisher);

									if (in_get_extended_fileinfoW(fn, L"category", pszBuffer, TEXTBUFFER_MAX))
										query->item.results.Items[0].category = _wcsdup(pszBuffer);
									SAVE_LAST_STR(last_category, query->item.results.Items[0].category, disable_category);

#define SAVE_LAST_STR_EXTENDED(last, name, disable) if (!disable) { wchar_t *check = getRecordExtendedItem(&query->item.results.Items[0], name); if (check && check[0]) { if (!last) last = check; else if (wcscmp(check, last)) disable = 1; }};

									if (in_get_extended_fileinfoW(fn, L"director", pszBuffer, TEXTBUFFER_MAX))
									{
										setRecordExtendedItem(&query->item.results.Items[0], L"director", pszBuffer);
										SAVE_LAST_STR_EXTENDED(last_director, L"director", disable_director);
									}

									if (in_get_extended_fileinfoW(fn, L"producer", pszBuffer, TEXTBUFFER_MAX))
									{
										setRecordExtendedItem(&query->item.results.Items[0], L"producer", pszBuffer);
										SAVE_LAST_STR_EXTENDED(last_producer, L"producer", disable_producer);
									}

									// not available in non-ml setups
									/*SAVE_LAST_STR_EXTENDED(last_podcastchannel, L"podcastchannel", disable_podcastchannel);
									SAVE_LAST_STR_EXTENDED(last_ispodcast, L"ispodcast", disable_ispodcast);*/
									nb++;
									query->ml = 0;
									query->idx = i;
								}
							}
						}
					}
				}
				free(pszBuffer);
			}

			if (!disable_artist && last_artist) SetDlgItemTextW(hwndDlg, IDC_EDIT_ARTIST, last_artist);
			if (!disable_title && last_title) SetDlgItemTextW(hwndDlg, IDC_EDIT_TITLE, last_title);
			if (!disable_album && last_album) SetDlgItemTextW(hwndDlg, IDC_EDIT_ALBUM, last_album);
			if (!disable_comment && last_comment) SetDlgItemTextW(hwndDlg, IDC_EDIT_COMMENT, last_comment);
			if (!disable_albumartist && last_albumartist) SetDlgItemTextW(hwndDlg, IDC_EDIT_ALBUMARTIST, last_albumartist);
			if (!disable_composer && last_composer) SetDlgItemTextW(hwndDlg, IDC_EDIT_COMPOSER, last_composer);
			if (!disable_publisher && last_publisher) SetDlgItemTextW(hwndDlg, IDC_EDIT_PUBLISHER, last_publisher);
			if (!disable_genre && last_genre) SetDlgItemTextW(hwndDlg, IDC_EDIT_GENRE, last_genre);
			if (!disable_category && last_category) SetDlgItemTextW(hwndDlg, IDC_EDIT_CATEGORY, last_category);
			if (!disable_director && last_director) SetDlgItemTextW(hwndDlg, IDC_EDIT_DIRECTOR, last_director);
			if (!disable_producer && last_producer) SetDlgItemTextW(hwndDlg, IDC_EDIT_PRODUCER, last_producer);
			if (!disable_podcastchannel && last_podcastchannel) SetDlgItemTextW(hwndDlg, IDC_EDIT_PODCAST_CHANNEL, last_podcastchannel);
			if (!disable_ispodcast && last_ispodcast)
			{
				CheckDlgButton(hwndDlg, IDC_CHECK_PODCAST, (_wtoi(last_ispodcast) == 1 ? BST_CHECKED : BST_UNCHECKED));
			}
			if (!disable_year && last_year > 0)
			{
				wchar_t tmp[64] = {0};
				wsprintfW(tmp, L"%d", last_year);
				SetDlgItemTextW(hwndDlg, IDC_EDIT_YEAR, tmp);
			}
			if (!disable_bpm && last_bpm > 0)
			{
				wchar_t tmp[64] = {0};
				wsprintfW(tmp, L"%d", last_bpm);
				SetDlgItemTextW(hwndDlg, IDC_EDIT_BPM, tmp);
			}
			if (!disable_rating)
			{
				if (last_rating > 0 && last_rating <= 5)
				{
					SendDlgItemMessage(hwndDlg, IDC_COMBO_RATING, CB_SETCURSEL, 5 - last_rating, 0);
				}
				else SendDlgItemMessage(hwndDlg, IDC_COMBO_RATING, CB_SETCURSEL, 5, 0);
			}
			if (!disable_track && last_track > 0 && !disable_tracks)
			{
				wchar_t tmp[64] = {0};
				if (!disable_tracks && last_tracks > 0)
					wsprintfW(tmp, L"%d/%d", last_track, last_tracks);
				else
					wsprintfW(tmp, L"%d", last_track);
				SetDlgItemTextW(hwndDlg, IDC_EDIT_TRACK, tmp); 
			}
			if (!disable_disc && last_disc > 0
			    && !disable_discs)
			{
				wchar_t tmp[64] = {0};
				if (!disable_discs && last_discs > 0)
					wsprintfW(tmp, L"%d/%d", last_disc, last_discs);
				else
					wsprintfW(tmp, L"%d", last_disc);
				SetDlgItemTextW(hwndDlg, IDC_EDIT_DISC, tmp);
			}
			wchar_t tmp[512] = {0};
			wsprintfW(tmp, getStringW((nb==1?IDS_X_ITEM_SELECTED:IDS_X_ITEMS_SELECTED), NULL, 0), nb);
			SetDlgItemTextW(hwndDlg, 1051, tmp);

			if (!nb)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, 1052), SW_HIDE);
			}

			// show edit info window and restore last position as applicable
			POINT pt = {editinfo_rect.left, editinfo_rect.top};
			if (!windowOffScreen(hwndDlg, pt) && !IsWindowVisible(hwndDlg))
				SetWindowPos(hwndDlg, HWND_TOP, editinfo_rect.left, editinfo_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
		}
		return 1;
		case WM_COMMAND:
#define HANDLE_CONTROL(item, check) { int enabled = IsDlgButtonChecked(hwndDlg, check); EnableWindow(GetDlgItem(hwndDlg, item), enabled); EnableWindow(GetDlgItem(hwndDlg, IDOK), enabled); }
			switch (LOWORD(wParam))
			{
				case IDC_CHECK_ARTIST: HANDLE_CONTROL(IDC_EDIT_ARTIST, IDC_CHECK_ARTIST); break;
				case IDC_CHECK_TITLE: HANDLE_CONTROL(IDC_EDIT_TITLE, IDC_CHECK_TITLE); break;
				case IDC_CHECK_ALBUM: HANDLE_CONTROL(IDC_EDIT_ALBUM, IDC_CHECK_ALBUM); break;
				case IDC_CHECK_COMMENT: HANDLE_CONTROL(IDC_EDIT_COMMENT, IDC_CHECK_COMMENT); break;
				case IDC_CHECK_ALBUMARTIST: HANDLE_CONTROL(IDC_EDIT_ALBUMARTIST, IDC_CHECK_ALBUMARTIST); break;
				case IDC_CHECK_COMPOSER: HANDLE_CONTROL(IDC_EDIT_COMPOSER, IDC_CHECK_COMPOSER); break;
				case IDC_CHECK_PUBLISHER: HANDLE_CONTROL(IDC_EDIT_PUBLISHER, IDC_CHECK_PUBLISHER); break;
				case IDC_CHECK_TRACK: HANDLE_CONTROL(IDC_EDIT_TRACK, IDC_CHECK_TRACK); break;
				case IDC_CHECK_DISC: HANDLE_CONTROL(IDC_EDIT_DISC, IDC_CHECK_DISC); break;
				case IDC_CHECK_GENRE: HANDLE_CONTROL(IDC_EDIT_GENRE, IDC_CHECK_GENRE); break;
				case IDC_CHECK_YEAR: HANDLE_CONTROL(IDC_EDIT_YEAR, IDC_CHECK_YEAR); break;
				case IDC_CHECK_CATEGORY: HANDLE_CONTROL(IDC_EDIT_CATEGORY, IDC_CHECK_CATEGORY); break;
				case IDC_CHECK_DIRECTOR: HANDLE_CONTROL(IDC_EDIT_DIRECTOR, IDC_CHECK_DIRECTOR); break;
				case IDC_CHECK_PRODUCER: HANDLE_CONTROL(IDC_EDIT_PRODUCER, IDC_CHECK_PRODUCER); break;
				case IDC_CHECK_PODCAST_CHANNEL: HANDLE_CONTROL(IDC_EDIT_PODCAST_CHANNEL, IDC_CHECK_PODCAST_CHANNEL); break;
				case IDC_CHECK_BPM: HANDLE_CONTROL(IDC_EDIT_BPM, IDC_CHECK_BPM); break;
				case IDC_CHECK_RATING: HANDLE_CONTROL(IDC_COMBO_RATING, IDC_CHECK_RATING); break;
				case IDOK:
				{
					if (got_local_ml == 1)
					{
						wchar_t str[4] = {0};
						StringCchPrintfW(str, 4, L"%d", !!IsDlgButtonChecked(hwndDlg, 1052));
						WritePrivateProfileStringW(L"gen_ml_config", L"upd_tagz", str, ML_INI_FILE);
					}

					int ret = LPDialogBoxW(IDD_ADDSTUFF, hwndDlg, updateFiles_dialogProc);
					if (!ret) break;
				}
				case IDCANCEL:
				{
					FreeQueryList();
					GetWindowRect(hwndDlg, &editinfo_rect);
					EndDialog(hwndDlg, 0);
					break;
				}
			}
			break;
		case WM_LBUTTONDOWN:
		{
			POINTS p = MAKEPOINTS(lParam);
			POINT p2 = {p.x, p.y};
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_ARTIST, IDC_CHECK_ARTIST)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_TITLE, IDC_CHECK_TITLE)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_ALBUM, IDC_CHECK_ALBUM)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_COMMENT, IDC_CHECK_COMMENT)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_ALBUMARTIST, IDC_CHECK_ALBUMARTIST)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_COMPOSER, IDC_CHECK_COMPOSER)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_PUBLISHER, IDC_CHECK_PUBLISHER)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_TRACK, IDC_CHECK_TRACK)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_GENRE, IDC_CHECK_GENRE)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_YEAR, IDC_CHECK_YEAR)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_DISC, IDC_CHECK_DISC)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_CATEGORY, IDC_CHECK_CATEGORY)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_DIRECTOR, IDC_CHECK_DIRECTOR)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_PRODUCER, IDC_CHECK_PRODUCER)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_PODCAST_CHANNEL, IDC_CHECK_PODCAST_CHANNEL)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_EDIT_BPM, IDC_CHECK_BPM)) break;
			if (checkEditInfoClick(hwndDlg, p2, IDC_COMBO_RATING, IDC_CHECK_RATING)) break;
		}
		break;
	}
	return FALSE;
}