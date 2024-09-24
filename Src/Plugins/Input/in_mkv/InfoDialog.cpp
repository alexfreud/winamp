#include "MKVInfo.h"
#include "api__in_mkv.h"
#include "../nu/ListView.h"
#include "../nu/AutoWide.h"
#include "resource.h"
#include "main.h"
#include <strsafe.h>


enum
{
	COLUMN_TRACK_TYPE = 0,
	COLUMN_CODEC_NAME = 1,
	COLUMN_CODEC_ID = 2,
	COLUMN_DESCRIPTION = 3,
	COLUMN_STREAM_NAME = 4,
	COLUMN_LANGUAGE = 5,
};


#if 0
static INT_PTR CALLBACK InfoDialog_Metadata(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			nsavi::Metadata *metadata = (nsavi::Metadata *)lParam;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);

			W_ListView list_view(hwndDlg, IDC_TRACKLIST);

			list_view.AddCol(L"Field", 100);
			list_view.AddCol(L"FOURCC", 75);
			list_view.AddCol(L"Value", 250);

			nsavi::Info *info;
			if (metadata->GetInfo(&info) == nsavi::READ_OK)
			{
				int n=0;
				for (nsavi::Info::const_iterator itr = info->begin();itr!=info->end();itr++)
				{
					const wchar_t *field_name = FindKnownName(known_fields, sizeof(known_fields)/sizeof(known_fields[0]), itr->first);

					wchar_t fourcc[5] = {0};
					MakeStringFromFOURCC(fourcc, itr->first);

					if (field_name)
						n= list_view.AppendItem(field_name, 0);
					else
						n= list_view.AppendItem(fourcc, 0);

					list_view.SetItemText(n, 1, fourcc);
					list_view.SetItemText(n, 2, AutoWide(itr->second, CP_UTF8));
				}
			}
		}
		return 1;
	}
	return 0;
}
#endif

static INT_PTR CALLBACK InfoDialog_Tracks(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			MKVInfo *metadata = (MKVInfo *)lParam;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);

			W_ListView list_view(hwndDlg, IDC_TRACKLIST);

			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_TRACK_TYPE), 100);
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_CODEC_NAME), 100);
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_CODEC_ID), 75);			
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_DESCRIPTION), 100);
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_STREAM_NAME), 100);
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_LANGUAGE), 50);

			const nsmkv::Tracks *tracks = metadata->GetTracks();
			if (tracks)
			{
				const nsmkv::TrackEntry *track_entry;
				size_t i=0;
				while (track_entry = tracks->EnumTrack(i++))
				{
					int n;
					switch(track_entry->track_type)
					{
					case mkv_track_type_audio:
						{
							n = list_view.AppendItem(WASABI_API_LNGSTRINGW(IDS_TYPE_AUDIO), 0);
						}
						break;
					case mkv_track_type_video:
						{
							n = list_view.AppendItem(WASABI_API_LNGSTRINGW(IDS_TYPE_VIDEO), 0);

						}
						break;
					case mkv_track_type_subtitle:
						{
							n = list_view.AppendItem(WASABI_API_LNGSTRINGW(IDS_TYPE_SUBTITLE), 0);
						}
						break;
					default:
						{
							wchar_t track_type[64] = {0};
							StringCchPrintf(track_type, 64, L"%X", track_entry->track_type);
							n = list_view.AppendItem(track_type, 0);
						}
						break;
					}
					if (track_entry->codec_id)
						list_view.SetItemText(n, COLUMN_CODEC_ID, AutoWide(track_entry->codec_id, CP_UTF8));

					if (track_entry->codec_name)
					{
						list_view.SetItemText(n, COLUMN_CODEC_NAME, AutoWide(track_entry->codec_name, CP_UTF8));
					}
					else
					{	
						// TODO: enumerate through a list of known codecs
						if (track_entry->codec_id)
							list_view.SetItemText(n, COLUMN_CODEC_NAME, AutoWide(track_entry->codec_id, CP_UTF8));						
					}

					if (track_entry->name)
					{
								list_view.SetItemText(n, COLUMN_STREAM_NAME, AutoWide(track_entry->name, CP_UTF8));						
					}

					if (track_entry->language && stricmp(track_entry->language, "und"))
					{
						list_view.SetItemText(n, COLUMN_LANGUAGE, AutoWide(track_entry->language, CP_UTF8));						
					}
				}
			}
			
		}
		return 1;
	case WM_SIZE:
		{
			RECT r;
			GetClientRect(hwndDlg, &r);
			SetWindowPos(GetDlgItem(hwndDlg, IDC_TRACKLIST), HWND_TOP, r.left, r.top, r.right, r.bottom, SWP_NOACTIVATE);
		}
		break;
	}
	return 0;
}


struct InfoDialogContext
{
	MKVInfo *metadata;
	HWND active_tab;
};

static VOID WINAPI OnSelChanged(HWND hwndDlg, HWND hwndTab, InfoDialogContext *context)
{
	if (context->active_tab)
	{
		DestroyWindow(context->active_tab);
	}
	int selection = TabCtrl_GetCurSel(hwndTab);
	switch(selection)
	{
	case 0:
		context->active_tab = WASABI_API_CREATEDIALOGPARAMW(IDD_TRACKS, hwndDlg, InfoDialog_Tracks, (LPARAM)context->metadata);
		
		break;
	case 1:
//		context->active_tab = WASABI_API_CREATEDIALOGPARAMW(IDD_TRACKS, hwndDlg, InfoDialog_Metadata, (LPARAM)context->metadata);
		break;
	}

	RECT r;
	GetWindowRect(hwndTab,&r);
	TabCtrl_AdjustRect(hwndTab,FALSE,&r);
	MapWindowPoints(NULL,hwndDlg,(LPPOINT)&r,2);

	SetWindowPos(context->active_tab,HWND_TOP,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOACTIVATE);
	ShowWindow(context->active_tab, SW_SHOWNA);

	if (GetFocus() != hwndTab)
	{
		SetFocus(context->active_tab);
	}
}

INT_PTR CALLBACK InfoDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hwndTab = GetDlgItem(hwndDlg,IDC_TAB1);
			InfoDialogContext *context = (InfoDialogContext *)calloc(1, sizeof(InfoDialogContext));
			context->metadata = (MKVInfo *)lParam;
			context->active_tab = 0;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA, (LPARAM)context);
			TCITEMW tie;
			tie.mask = TCIF_TEXT;
			tie.pszText = WASABI_API_LNGSTRINGW(IDS_TAB_TRACKS);
			SendMessageW(hwndTab, TCM_INSERTITEMW, 0, (LPARAM)&tie);
			OnSelChanged(hwndDlg, hwndTab, context);
		}
		return 1;

	case WM_DESTROY:
		{
			InfoDialogContext *context = (InfoDialogContext *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			free(context);
		}
		break;
	case WM_NOTIFY:
		{
			LPNMHDR lpn = (LPNMHDR) lParam;
			if (lpn && lpn->code==TCN_SELCHANGE)
			{
				InfoDialogContext *context = (InfoDialogContext *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				OnSelChanged(hwndDlg,GetDlgItem(hwndDlg,IDC_TAB1),context);
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				EndDialog(hwndDlg,0);
			}
			break;
		case IDCANCEL:
			{
				EndDialog(hwndDlg,1);
			}
			break;
		}
		break;
	}

	return 0;
}