#include "main.h"
#include "tagger.h"

#define COL_FILENAME 0
#define COL_STATUS 1

static INT_PTR CALLBACK autotagger_saving_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static const int ids[] =
{
	IDC_TITLE_OLD,
	IDC_TRACK_OLD,
	IDC_ARTIST_OLD,
	IDC_ALBUM_OLD,
	IDC_GENRE_OLD,
	IDC_YEAR_OLD,
	IDC_DISC_OLD,
	IDC_PUBLISHER_OLD,
	IDC_ALBUMARTIST_OLD,
	IDC_COMPOSER_OLD,
	IDC_BPM_OLD,
	IDC_TITLE_NEW,
	IDC_TRACK_NEW,
	IDC_ARTIST_NEW,
	IDC_ALBUM_NEW,
	IDC_GENRE_NEW,
	IDC_YEAR_NEW,
	IDC_DISC_NEW,
	IDC_PUBLISHER_NEW,
	IDC_ALBUMARTIST_NEW,
	IDC_COMPOSER_NEW,
	IDC_BPM_NEW,
	IDC_FILENAME,
};
static const int ids_static[] =
{
	IDC_STATIC_OLDINFO,
	IDC_STATIC_TITLE_OLD,
	IDC_STATIC_TRACK_OLD,
	IDC_STATIC_ARTIST_OLD,
	IDC_STATIC_ALBUM_OLD,
	IDC_STATIC_GENRE_OLD,
	IDC_STATIC_YEAR_OLD,
	IDC_STATIC_DISC_OLD,
	IDC_STATIC_ALBUMARTIST_OLD,
	IDC_STATIC_PUBLISHER_OLD,
	IDC_STATIC_COMPOSER_OLD,
	IDC_STATIC_BPM_OLD,
	IDC_STATIC_NEWINFO,
	IDC_STATIC_DISC_NEW,
	IDC_STATIC_TITLE_NEW,
	IDC_STATIC_TRACK_NEW,
	IDC_STATIC_ARTIST_NEW,
	IDC_STATIC_ALBUM_NEW,
	IDC_STATIC_GENRE_NEW,
	IDC_STATIC_YEAR_NEW,
	IDC_STATIC_ALBUMARTIST_NEW,
	IDC_STATIC_PUBLISHER_NEW,
	IDC_STATIC_COMPOSER_NEW,
	IDC_STATIC_BPM_NEW,
	IDC_STATIC_FILENAME,
};

static bool GetRole(ICddbDisc *disc, BSTR roleId, BSTR *str)
{
	if (!roleId || !*roleId)
		return false;

	if (!disc)
		return false;

	ICddbCreditsPtr credits;
	disc->get_Credits(&credits);
	if (credits)
	{
		long creditCount;
		credits->get_Count(&creditCount);
		for (long c = 0;c < creditCount;c++)
		{
			ICddbCreditPtr credit;
			credits->GetCredit(c + 1, &credit);
			if (credit)
			{
				BSTR thisRole;
				credit->get_Id(&thisRole);
				if (!wcscmp(thisRole, roleId))
				{
					credit->get_Name(str);
					return true;
				}
			}
		}
	}
	return false;
}

static void EnableBottomView(HWND hwndDlg, int enable)
{
	for (int i=0; i<(sizeof(ids_static)/sizeof(int)); i++)
		EnableWindow(GetDlgItem(hwndDlg,ids_static[i]),enable);

	for (int i=0; i<(sizeof(ids)/sizeof(int)); i++)
		SetDlgItemText(hwndDlg,ids[i],L"");
}

INT_PTR CALLBACK autotagger_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG)(LONG_PTR)lParam );
		Tagger *t = (Tagger *)lParam;
		t->hwndDlg = hwndDlg;
		HWND hlist = GetDlgItem( hwndDlg, IDC_LIST );
		t->listview.setwnd( hlist );

		EnableBottomView( hwndDlg, FALSE );

		ICddbFileInfoListPtr infolist;
		infolist.CreateInstance( CLSID_CddbFileInfoList );

		ListView_SetExtendedListViewStyle( GetDlgItem( hwndDlg, IDC_LIST ), LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP );

		t->listview.AddCol( WASABI_API_LNGSTRINGW( IDS_FILENAME ), 475 );
		t->listview.AddCol( WASABI_API_LNGSTRINGW( IDS_STATUS ), 100 );

		for ( size_t i = 0; i < t->list.size(); i++ )
		{
			ICddbFileInfoPtr info;
			info.CreateInstance( CLSID_CddbFileInfo );
			info->put_Filename( (wchar_t *)t->list[ i ]->filename );
			infolist->AddFileInfo( info );
			t->list[ i ]->num = (int)i;
			t->listview.InsertItem( (int)i, t->list[ i ]->filename, (LPARAM)t->list[ i ] );
			t->listview.SetItemText( (int)i, COL_STATUS, WASABI_API_LNGSTRINGW( IDS_WAITING ) );
			ListView_SetCheckState( hlist, (int)i, FALSE );
		}

		t->musicid->LibraryID( infolist, MUSICID_LOOKUP_ASYNC | MUSICID_RETURN_SINGLE | MUSICID_GET_TAG_FROM_APP | MUSICID_GET_FP_FROM_APP | MUSICID_PREFER_WF_MATCHES );
	}
	break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
			case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW lv=(LPNMLISTVIEW)lParam;
				if ((lv->uNewState ^ lv->uOldState) & LVIS_SELECTED)
					SendMessage(hwndDlg,WM_USER,0,0);
			}
			break;
		}
		break;
	case WM_USER:
	{
		int f = ListView_GetNextItem(GetDlgItem(hwndDlg,IDC_LIST),-1,LVIS_FOCUSED);
		if (f < 0)
		{
			EnableBottomView(hwndDlg,FALSE);
			return -1;
		}
		EnableBottomView(hwndDlg,TRUE);
		Tagger * t = (Tagger *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		TagItem * ti = t->list[f];
		SetDlgItemText(hwndDlg,IDC_FILENAME,ti->filename);

		BSTR bstr = 0;
		if (ti->oldTag)
		{
			ICddbFileTag2_5Ptr tag2_5 = NULL;
			ti->oldTag->QueryInterface(&tag2_5);

#define PUTINFO(x,y) ti->oldTag->get_ ## x ##(&bstr); if(bstr && *bstr) SetDlgItemText(hwndDlg,y,bstr); if(bstr) SysFreeString(bstr); bstr=0;
#define PUTINFO2(x,y) if (tag2_5) { tag2_5->get_ ## x ##(&bstr); if(bstr && *bstr) SetDlgItemText(hwndDlg,y,bstr); if(bstr) SysFreeString(bstr); bstr=0; }
			PUTINFO(Title, IDC_TITLE_OLD);
			PUTINFO(TrackPosition, IDC_TRACK_OLD);
			PUTINFO(LeadArtist, IDC_ARTIST_OLD);
			PUTINFO(Album, IDC_ALBUM_OLD);
			PUTINFO(Genre, IDC_GENRE_OLD);
			PUTINFO(Year, IDC_YEAR_OLD);
			PUTINFO(PartOfSet, IDC_DISC_OLD);
			PUTINFO(Label, IDC_PUBLISHER_OLD);
			PUTINFO2(DiscArtist, IDC_ALBUMARTIST_OLD);
			PUTINFO2(Composer, IDC_COMPOSER_OLD);
			PUTINFO(BeatsPerMinute, IDC_BPM_OLD);
#undef PUTINFO
#undef PUTINFO2
		}
		if (ti->newTag)
		{
			ICddbFileTag2_5Ptr tag2_5 = NULL;
			ICddbDisc2_5Ptr pDisc2_5 = NULL;

			ti->newTag->QueryInterface(&tag2_5);
			if (ti->disc)
				ti->disc->QueryInterface(&pDisc2_5);

#define PUTINFO(x,y) ti->newTag->get_ ## x ##(&bstr); if(bstr && *bstr) SetDlgItemText(hwndDlg,y,bstr); if(bstr) SysFreeString(bstr); bstr=0;
#define PUTINFO2(x,y) if (tag2_5) { tag2_5->get_ ## x ##(&bstr); if(bstr && *bstr) SetDlgItemText(hwndDlg,y,bstr); if(bstr) SysFreeString(bstr); bstr=0; }
#define PUTROLE(x,y) if (GetRole(ti->disc, x, &bstr) && bstr && *bstr) SetDlgItemText(hwndDlg,y,bstr); if(bstr) SysFreeString(bstr); bstr=0;
			PUTINFO(Title, IDC_TITLE_NEW);
			PUTINFO(TrackPosition, IDC_TRACK_NEW);
			PUTINFO(LeadArtist, IDC_ARTIST_NEW);
			PUTINFO(Album, IDC_ALBUM_NEW);

			if (pDisc2_5 == NULL
			    || (FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(3, &bstr))
			        && FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(2, &bstr))
			        && FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(1, &bstr))
			        && FAILED(pDisc2_5->get_V2GenreStringPrimary(&bstr)))
			   )
			{
				PUTINFO(Genre, IDC_GENRE_NEW);
			}
			if (bstr && *bstr)
				SetDlgItemText(hwndDlg,IDC_GENRE_NEW,bstr);
			if (bstr) SysFreeString(bstr); bstr=0;

			PUTINFO(Year, IDC_YEAR_NEW);
			PUTINFO(PartOfSet, IDC_DISC_NEW);
			PUTINFO(Label, IDC_PUBLISHER_NEW);
			PUTINFO2(DiscArtist, IDC_ALBUMARTIST_NEW);
			PUTINFO2(Composer, IDC_COMPOSER_NEW);
			PUTINFO(BeatsPerMinute, IDC_BPM_NEW);
#undef PUTINFO
#undef PUTINFO2
#undef PUTROLE
		}
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDC_CHECK_NONE:
			case IDC_CHECK_ALL:
			{
				BOOL check = (LOWORD(wParam) == IDC_CHECK_ALL);
				HWND hlist = GetDlgItem(hwndDlg,IDC_LIST);
				size_t l = ListView_GetItemCount(hlist);
				for (size_t i=0; i < l; i++)
					ListView_SetCheckState(hlist,(int)i,check);
			}
			break;
			case IDOK:
				if (WASABI_API_DIALOGBOXW(IDD_SAVING,hwndDlg,autotagger_saving_dlgproc))
					break;
			case IDCANCEL:
			{
				Tagger * t = (Tagger *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				t->abort=1;
				t->musicid->LibraryIDStop(0);
				//t->musicid->LibraryIDClear();
			}
			EndDialog(hwndDlg,0);
			break;
		}
		break;
	case WM_DESTROY:
	{
		Tagger * t = (Tagger *)(LONG_PTR)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (t->icp) t->icp->Unadvise(t->m_dwCookie);
		t->musicid->Shutdown();
		t->musicid->Release();
		delete t;
	}
	break;
	case WM_CLOSE:
		return SendMessage(hwndDlg,WM_COMMAND,IDCANCEL,0);
	}
	return 0;
}

static INT_PTR CALLBACK autotagger_saving_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int i;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		i=0;
		SetTimer(hwndDlg,0,20,NULL);
		Tagger * t = (Tagger *)(LONG_PTR)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);
		SendDlgItemMessage(hwndDlg,IDC_PROGRESS,PBM_SETRANGE,0,MAKELPARAM(0, t->list.size()));
	}
	break;
	case WM_TIMER:
		if (wParam == 0)
		{
			KillTimer(hwndDlg,0);
			HWND hlist = GetDlgItem(GetParent(hwndDlg),IDC_LIST);
			Tagger * t = (Tagger *)(LONG_PTR)GetWindowLongPtr(GetParent(hwndDlg),GWLP_USERDATA);
			int n = (int)t->list.size();
			for (;;)
			{
				if (i >= n)
				{
					EndDialog(hwndDlg,0);
					return 0;
				}
				if (ListView_GetCheckState(hlist,i))
				{
					TagItem *ti = t->list[i];
					const wchar_t *fn = ti->filename;
					BSTR bstr=0;
					if (ti->newTag)
					{
						ICddbFileTag2_5Ptr tag2_5 = NULL;
						ICddbDisc2_5Ptr pDisc2_5 = NULL;

						ti->newTag->QueryInterface(&tag2_5);
						if (ti->disc)
							ti->disc->QueryInterface(&pDisc2_5);

#define PUTINFO(x,y) ti->newTag->get_ ## x ## (&bstr); if(bstr && *bstr) SetFileInfo(fn,y,bstr); if(bstr) SysFreeString(bstr); bstr=0;
#define PUTINFO2(x,y) if (tag2_5) { tag2_5->get_ ## x ## (&bstr); if(bstr && *bstr) SetFileInfo(fn,y,bstr); if(bstr) SysFreeString(bstr); bstr=0; }
#define PUTROLE(x,y) if (GetRole(ti->disc, x, &bstr) && bstr && *bstr) SetFileInfo(fn,y,bstr); if(bstr) SysFreeString(bstr); bstr=0;
						PUTINFO(LeadArtist, L"artist");
						PUTINFO(Album, L"album");
						PUTINFO(Title, L"title");
						if (pDisc2_5 == NULL
						    || (FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(3, &bstr))
						        && FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(2, &bstr))
						        && FAILED(pDisc2_5->get_V2GenreStringPrimaryByLevel(1, &bstr))
						        && FAILED(pDisc2_5->get_V2GenreStringPrimary(&bstr)))
						   )
						{
							PUTINFO(Genre, L"genre");
						}
						if (bstr && *bstr)
							SetFileInfo(fn,L"genre",bstr);
						if (bstr) SysFreeString(bstr); bstr=0;
						// benski> CUT: PUTINFO(Genre, L"genre");
						PUTINFO(Year, L"year");
						PUTINFO(Label, L"publisher");
						PUTINFO(BeatsPerMinute, L"bpm");
						PUTINFO(TrackPosition, L"track");
						PUTINFO(PartOfSet, L"disc");
						PUTINFO2(Composer, L"composer");
						PUTINFO2(DiscArtist, L"albumartist");
						PUTINFO(ISRC, L"ISRC");
						//PUTROLE(L"147", L"remixing");
						PUTINFO(FileId, L"GracenoteFileID");
						PUTINFO2(ExtDataSerialized, L"GracenoteExtData");

						WriteFileInfo(fn);
#undef PUTINFO
#undef PUTINFO2
#undef PUTROLE
					}
					i++;
					SendDlgItemMessage(hwndDlg,IDC_PROGRESS,PBM_SETPOS,i,0);
					SetTimer(hwndDlg,0,20,NULL);
					return 0;
				}
				else i++;
				SendDlgItemMessage(hwndDlg,IDC_PROGRESS,PBM_SETPOS,i,0);
			}
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL) EndDialog(hwndDlg,1);
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg,1);
		break;
	}
	return 0;
}

TagItem::TagItem(const char * filename0) : freefn(true), num(0)
{
	filename = AutoWideDup(filename0);
	init();
}

TagItem::TagItem(const wchar_t * filename0, bool copy) : freefn(copy), num(0)
{
	filename = copy?_wcsdup(filename0):filename0;
	init();
}

TagItem::~TagItem()
{
	if (freefn) free((void*)filename);
}

void TagItem::init()
{
	last_match = MUSICID_MATCH_NONE;
	oldTag.CreateInstance(CLSID_CddbID3Tag);
}

void TagItem::SetStatus(CddbMusicIDStatus status, HWND hwndDlg)
{
	int s[] = {0, IDS_ERROR, IDS_PROCESSING, IDS_LOOKINGUP, IDS_LOOKINGUP, 0, IDS_QUERYING, IDS_QUERIED, IDS_PROCESSING, IDS_PROCESSED, IDS_ANALYZING, IDS_QUERYING};
	if (status < sizeof(s)/sizeof(int))
	{
		wchar_t buf[100] = {0};
		SetStatus(WASABI_API_LNGSTRINGW_BUF(s[status],buf,100),hwndDlg);
	}
}

void TagItem::SetStatus(const wchar_t *text, HWND hwndDlg)
{
	LVITEM lvi = {0, };
	lvi.iItem = num;
	lvi.iSubItem = COL_STATUS;
	lvi.mask = LVIF_TEXT;
	lvi.pszText = (LPTSTR)text;
	lvi.cchTextMax = lstrlenW(text);
	SendMessageW(GetDlgItem(hwndDlg,IDC_LIST), LVM_SETITEMW, 0, (LPARAM)&lvi);
}

void TagItem::Check(HWND hwndDlg, BOOL check)
{
	HWND hlist = GetDlgItem(hwndDlg,IDC_LIST);
	ListView_SetCheckState(hlist,(WPARAM)num,(LPARAM)check);
	
}

void TagItem::TagUpdate(HWND hwndDlg)
{
	int f = ListView_GetNextItem(GetDlgItem(hwndDlg,IDC_LIST),-1,LVIS_FOCUSED);
	if (f == num)
		SendMessage(hwndDlg,WM_USER,0,0);
}

static IConnectionPoint *GetConnectionPoint(IUnknown *punk, REFIID riid);

Tagger::Tagger(std::vector<TagItem*> &_list, ICDDBMusicIDManager3 *musicid) :  hwndDlg(0), abort(0), musicid(musicid), icp(0), m_dwCookie(0)
{
	//list.own(_list);
	for (auto obj : list)
	{
		delete obj;
	}
	list.clear();
	list.assign(_list.begin(), _list.end());
	_list.clear();


	icp = GetConnectionPoint(musicid, DIID__ICDDBMusicIDManagerEvents);
	if (icp)
	{
		icp->Advise(static_cast<IDispatch *>(this), &m_dwCookie);
		icp->Release();
	}
}

Tagger::~Tagger()
{
}

TagItem *Tagger::FindTagItem( const wchar_t *filename )
{
	if ( !filename )
		return NULL;

	for ( TagItem *l_tag_item : list )
		if ( !_wcsicmp( l_tag_item->filename, filename ) )
			return l_tag_item;

	return NULL;
}

HRESULT Tagger::OnTrackIDStatusUpdate(CddbMusicIDStatus Status, BSTR filename, long* Abort)
{
	TagItem *t = FindTagItem(filename);
	if (t) t->SetStatus(Status,hwndDlg);
	*Abort = abort;
	return S_OK;
}

HRESULT Tagger::OnTrackIDComplete(CddbMusicIDMatchCode match_code, ICddbFileInfo* pInfoIn, ICddbFileInfoList* pListOut)
{
	if (!pInfoIn || !pListOut)
		return S_OK;

	BSTR filename;
	pInfoIn->get_Filename(&filename);

	TagItem *t = FindTagItem(filename);
	if (!t) return S_OK;

	if (match_code == MUSICID_MATCH_FUZZY)
	{
		if (t->last_match == MUSICID_MATCH_EXACT)
			return S_OK;
		t->Check(hwndDlg, FALSE);
	}
	else if (match_code == MUSICID_MATCH_EXACT)
	{
		t->Check(hwndDlg, TRUE);
	}
	else if (match_code == MUSICID_MATCH_NONE)
	{
		if (t->last_match == MUSICID_MATCH_EXACT)
			return S_OK;
	}

	t->last_match = match_code;

	wchar_t buf[100] = {0};

	switch(match_code)
	{
		case MUSICID_MATCH_NONE:
			t->SetStatus(WASABI_API_LNGSTRINGW_BUF(IDS_NOMATCH,buf,100),hwndDlg);
			return S_OK;
		case MUSICID_MATCH_FUZZY:
			t->SetStatus(WASABI_API_LNGSTRINGW_BUF(IDS_FUZZY,buf,100),hwndDlg); 
			break;
		case MUSICID_MATCH_ERROR:
			t->SetStatus(WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,buf,100),hwndDlg); 
			return S_OK;
		default:
			t->SetStatus(WASABI_API_LNGSTRINGW_BUF(IDS_PROCESSED,buf,100),hwndDlg);
			break;
	}
	/*
	if (match_code <= 1)
	{
		t->SetStatus(WASABI_API_LNGSTRINGW_BUF(IDS_NOMATCH,buf,100),hwndDlg); return S_OK;
	}
	else t->SetStatus(WASABI_API_LNGSTRINGW_BUF(IDS_PROCESSED,buf,100),hwndDlg);
*/
	long num;
	pListOut->get_Count(&num);
	if (!num) return S_OK;

	ICddbFileInfoPtr infotag;
	pListOut->GetFileInfo(1,&infotag);
	if (infotag)
	{
		infotag->get_Disc(&t->disc);
		infotag->get_Tag(&t->newTag);
		t->TagUpdate(hwndDlg);
	}

	return S_OK;
}

HRESULT Tagger::OnAlbumIDStatusUpdate(CddbMusicIDStatus Status, BSTR filename, long current_file, long total_files, long* Abort)
{
	TagItem *t = FindTagItem(filename);
	if (t) t->SetStatus(Status,hwndDlg);
	*Abort = abort;
	return S_OK;
}

HRESULT Tagger::OnAlbumIDComplete(LONG match_code, ICddbFileInfoList* pListIn, ICddbFileInfoLists* pListsOut)
{
	if (!pListIn || !pListsOut)
		return 0;

	long c1=0,c2=0;
	pListIn->get_Count(&c1);
	pListsOut->get_Count(&c2);

	if (c2<=0) return 0;

	for (int i=1; i<=c2; i++)
	{
		ICddbFileInfoListPtr list;
		pListsOut->GetFileInfoList(i,&list);
		list->get_Count(&c1);
					
			
		for (int i=1; i<=c1; i++)
		{
			ICddbFileInfoPtr pInfoIn;
			list->GetFileInfo(i,&pInfoIn);

			ICddbFileInfoListPtr pListOut;
			pListOut.CreateInstance(CLSID_CddbFileInfoList);

			ICddbFileInfoPtr temp;
			list->GetFileInfo(i,&temp);
			pListOut->AddFileInfo(temp);
			CddbMusicIDMatchCode match;
			pInfoIn->get_MusicIDMatchCode(&match);
			OnTrackIDComplete(match,pInfoIn,pListOut);
		}
	}
	return S_OK;
}

HRESULT Tagger::OnLibraryIDListStarted(ICddbFileInfoList* pList, long FilesComplete, long FilesTotal, long FilesExact, long FilesFuzzy, long FilesNoMatch, long FilesError, long *Abort)
{
	*Abort = abort;
	ShowWindow(GetDlgItem(hwndDlg,IDC_BRANDTXT),SW_SHOWNA);
	return S_OK;
}

HRESULT Tagger::OnLibraryIDListComplete(ICddbFileInfoList* pList, long FilesComplete, long FilesTotal, long FilesExact, long FilesFuzzy, long FilesNoMatch, long FilesError, long *Abort)
{
	*Abort = abort;
	return S_OK;
}

HRESULT Tagger::OnLibraryIDComplete(long FilesComplete, long FilesTotal, long FilesExact, long FilesFuzzy, long FilesNoMatch, long FilesError)
{
	ShowWindow(GetDlgItem(hwndDlg,IDC_BRANDTXT),SW_HIDE);
	return S_OK;
}

HRESULT Tagger::FillTag(ICddbFileInfo *info, BSTR filename)
{
	TagItem *t = FindTagItem(filename);
	if (!t) return E_FAIL;
	ICddbID3Tag * infotag = t->oldTag;

	ICddbFileTag2_5Ptr tag2_5 = NULL;
	infotag->QueryInterface(&tag2_5);
	itemRecordW *record = AGAVE_API_MLDB->GetFile(filename);
	if (record && infotag && tag2_5)
	{
		wchar_t itemp[64] = {0};
#define PUTINFO(y, x) if(record-> ## x) infotag->put_ ## y ##(record-> ## x)
#define PUTINFOI(y, x) if(record-> ## x > 0) infotag->put_ ## y ##(_itow(record-> ## x,itemp,10))
#define PUTINFO2(y, x) if(record-> ## x) tag2_5->put_ ## y ##(record-> ## x)
		PUTINFO(LeadArtist, artist);
		PUTINFO(Album, album);
		PUTINFO(Title, title);
		PUTINFO(Genre, genre);
		PUTINFOI(Year, year);
		PUTINFO(Label, publisher);
		PUTINFOI(BeatsPerMinute, bpm);
		PUTINFOI(TrackPosition, track);
		PUTINFOI(PartOfSet, disc);
		PUTINFO2(Composer, composer);
		PUTINFO2(DiscArtist, albumartist);
#undef PUTINFO
#undef PUTINFOI
#undef PUTINFO2
		AGAVE_API_MLDB->FreeRecord(record);
	}
	else
	{
		wchar_t buf[2048]=L"";
#define PUTINFO(y, x) buf[0]=0; GetFileInfo(filename,x,buf,2048); if(buf[0]) if (infotag) infotag->put_ ## y ##(buf);
#define PUTINFO2(y, x) buf[0]=0; GetFileInfo(filename,x,buf,2048); if(buf[0]) if (tag2_5) tag2_5->put_ ## y ##(buf);
		PUTINFO(LeadArtist, L"artist");
		PUTINFO(Album, L"album");
		PUTINFO(Title, L"title");
		PUTINFO(Genre, L"genre");
		PUTINFO(Year, L"year");
		PUTINFO(Label, L"publisher");
		PUTINFO(BeatsPerMinute, L"bpm");
		PUTINFO(TrackPosition, L"track");
		PUTINFO(PartOfSet, L"disc");
		PUTINFO2(Composer, L"composer");
		PUTINFO2(DiscArtist, L"albumartist");
#undef PUTINFO
#undef PUTINFO2
	}
	t->TagUpdate(hwndDlg);
	info->put_Tag(infotag);
	return S_OK;
}

// com shit
STDMETHODIMP STDMETHODCALLTYPE Tagger::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG STDMETHODCALLTYPE Tagger::AddRef(void)
{
	return 1;
}

ULONG STDMETHODCALLTYPE Tagger::Release(void)
{
	return 0;
}

HRESULT STDMETHODCALLTYPE Tagger::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
	case 1: // OnTrackIDStatusUpdate, params: CddbMusicIDStatus Status, BSTR filename, long* Abort
	{
		long *abort = pdispparams->rgvarg[0].plVal;
		BSTR filename = pdispparams->rgvarg[1].bstrVal;
		if (!filename) return E_INVALIDARG;
		CddbMusicIDStatus status = (CddbMusicIDStatus)pdispparams->rgvarg[2].lVal;
		return OnTrackIDStatusUpdate(status,filename,abort);
	}
	case 2: // OnAlbumIDStatusUpdate, params: CddbMusicIDStatus Status, BSTR filename, long current_file, long total_files, long* Abort
	{
		long *abort = pdispparams->rgvarg[0].plVal;
		long total_files = pdispparams->rgvarg[1].lVal;
		long current_file= pdispparams->rgvarg[2].lVal;
		CddbMusicIDStatus status = (CddbMusicIDStatus)pdispparams->rgvarg[4].lVal;
		BSTR filename = pdispparams->rgvarg[3].bstrVal;
		if (!filename) return E_INVALIDARG;
		return OnAlbumIDStatusUpdate(status,filename,current_file,total_files,abort);
	}
	break;

	case 3: // OnTrackIDComplete, params: LONG match_code, ICddbFileInfo* pInfoIn, ICddbFileInfoList* pListOut
	{
		IDispatch *disp1 =pdispparams->rgvarg[0].pdispVal;
		IDispatch *disp2 =pdispparams->rgvarg[1].pdispVal;
		if (!disp1 || !disp2) return E_INVALIDARG;
		CddbMusicIDMatchCode match_code = (CddbMusicIDMatchCode)pdispparams->rgvarg[2].lVal;

		ICddbFileInfoPtr pInfoIn;
		ICddbFileInfoListPtr matchList;
		disp1->QueryInterface(&matchList);
		disp2->QueryInterface(&pInfoIn);

		return OnTrackIDComplete(match_code,pInfoIn,matchList);
	}
	break;
	case 4: // OnAlbumIDComplete, params: LONG match_code, ICddbFileInfoList* pListIn, ICddbFileInfoLists* pListsOut
	{
		IDispatch *disp1 =pdispparams->rgvarg[0].pdispVal;
		IDispatch *disp2 =pdispparams->rgvarg[1].pdispVal;
		if (!disp1 || !disp2) return E_INVALIDARG;
		long match_code = pdispparams->rgvarg[2].lVal;

		ICddbFileInfoListPtr pListIn;
		ICddbFileInfoListsPtr pListsOut;
		disp1->QueryInterface(&pListsOut);
		disp2->QueryInterface(&pListIn);

		return OnAlbumIDComplete(match_code,pListIn,pListsOut);
	}
	break;
	case 7: // OnLibraryIDListStarted, params: ICddbFileInfoList* pList, long FilesComplete, long FilesTotal, long FilesExact, long FilesFuzzy, long FilesNoMatch, long FilesError, long *Abort
	{
		IDispatch *disp1 =pdispparams->rgvarg[7].pdispVal;
		if (!disp1) return E_INVALIDARG;
		ICddbFileInfoListPtr pList;
		disp1->QueryInterface(&pList);
		return OnLibraryIDListStarted(pList,pdispparams->rgvarg[6].lVal,pdispparams->rgvarg[5].lVal,pdispparams->rgvarg[4].lVal,pdispparams->rgvarg[3].lVal,pdispparams->rgvarg[2].lVal,pdispparams->rgvarg[1].lVal,pdispparams->rgvarg[0].plVal);
	}
	break;
	case 8: // OnLibraryIDListComplete, params: ICddbFileInfoList* pList, long FilesComplete, long FilesTotal, long FilesExact, long FilesFuzzy, long FilesNoMatch, long FilesError, long *Abort
	{
		IDispatch *disp1 =pdispparams->rgvarg[7].pdispVal;
		if (!disp1) return E_INVALIDARG;
		ICddbFileInfoListPtr pList;
		disp1->QueryInterface(&pList);
		return OnLibraryIDListComplete(pList,pdispparams->rgvarg[6].lVal,pdispparams->rgvarg[5].lVal,pdispparams->rgvarg[4].lVal,pdispparams->rgvarg[3].lVal,pdispparams->rgvarg[2].lVal,pdispparams->rgvarg[1].lVal,pdispparams->rgvarg[0].plVal);
	}
	break;
	case 9: // OnLibraryIDComplete, params: long FilesComplete, long FilesTotal, long FilesExact, long FilesFuzzy, long FilesNoMatch, long FilesError
	{
		return OnLibraryIDComplete(pdispparams->rgvarg[5].lVal,pdispparams->rgvarg[4].lVal,pdispparams->rgvarg[3].lVal,pdispparams->rgvarg[2].lVal,pdispparams->rgvarg[1].lVal,pdispparams->rgvarg[0].lVal);
	}
	break;
	case 10: // OnGetFingerprintInfo
	{
		long *abort = pdispparams->rgvarg[0].plVal;
		IDispatch *disp = pdispparams->rgvarg[1].pdispVal;
		BSTR filename = pdispparams->rgvarg[2].bstrVal;
		if (!disp || !filename) return E_INVALIDARG;

		ICddbFileInfo *info;
		disp->QueryInterface(&info);
		HRESULT hr = AGAVE_API_GRACENOTE->CreateFingerprint(musicid, AGAVE_API_DECODE, info, filename, abort);
		info->Release();
		return hr;
	}
	break;
	case 11: // OnGetTagInfo
	{
		//long *Abort = pdispparams->rgvarg[0].plVal;
		IDispatch *disp = pdispparams->rgvarg[1].pdispVal;
		BSTR filename = pdispparams->rgvarg[2].bstrVal;
		if (!disp || !filename) return E_INVALIDARG;
		ICddbFileInfo *info;
		disp->QueryInterface(&info);
		HRESULT hr = FillTag(info, filename);
		info->Release();
		return hr;
	}
	break;
	}
	return DISP_E_MEMBERNOTFOUND;
}

HRESULT STDMETHODCALLTYPE Tagger::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	*rgdispid = DISPID_UNKNOWN; return DISP_E_UNKNOWNNAME;
}
HRESULT STDMETHODCALLTYPE Tagger::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE Tagger::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

static IConnectionPoint *GetConnectionPoint(IUnknown *punk, REFIID riid)
{
	if (!punk)
		return 0;

	IConnectionPointContainer *pcpc=0;
	IConnectionPoint *pcp = 0;

	HRESULT hr = punk->QueryInterface(IID_IConnectionPointContainer, (void **) & pcpc);
	if (SUCCEEDED(hr))
	{
		pcpc->FindConnectionPoint(riid, &pcp);
		pcpc->Release();
	}
	return pcp;
}
