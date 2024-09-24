#include "main.h"
#include "Metadata.h"
#include "../Winamp/wa_ipc.h"
// ID3v2 stuff
#include "../id3v2/id3_tag.h"
#include "FactoryHelper.h"
#include "id3.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "AACFrame.h"
#include "LAMEinfo.h"
#include <shlwapi.h>
#include "../nu/ns_wc.h"
#include "../nu/ListView.h"
#include "resource.h"
#include "Stopper.h"
#include "config.h"
#include <strsafe.h>


// TODO: benski> CUT!!!
char g_stream_title[256] = {0};

int fixAACCBRbitrate(int br)
{
	static short brs[] =
	{
		8, 12, 16, 20, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0
	};
	int x;
	for (x = 0; x < sizeof(brs) / sizeof(brs[0]); x ++)
	{
		int delta = (brs[x] * 8) / 128;
		if (delta < 2) delta = 2;
		if (br < brs[x] - delta) break;
		if (br < brs[x] + delta) return brs[x];
	}
	return br;
}

void ConvertTryUTF8(const char *in, wchar_t *out, size_t outlen)
{
	out[0]=0;
	int x = MultiByteToWideCharSZ(CP_UTF8, MB_ERR_INVALID_CHARS, in, -1, out, (int)outlen);
	if (!x)
	{
		if (GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
			MultiByteToWideCharSZ(CP_ACP, 0, in, -1, out, (int)outlen);
		else
			MultiByteToWideCharSZ(CP_UTF8, 0, in, -1, out, (int)outlen);
	}
}

void getfileinfo(const wchar_t *filename, wchar_t *title, int *length_in_ms)
{
	const wchar_t *fn;
	if (length_in_ms) *length_in_ms = -1000;
	if (filename && filename[0])
		fn = filename;
	else
		fn = lastfn;
	if (!_wcsnicmp(fn, L"file://", 7)) fn += 7;
	if (PathIsURL(fn))
	{
		if (title)
		{
			if (fn != filename || !_wcsicmp(fn, lastfn))
			{
				EnterCriticalSection(&g_lfnscs);
				if (lastfn_status[0])
				{
					char buf[4096] = {0};
					StringCchPrintfA(buf, 4096, "[%s] %s", lastfn_status, lastfn_data_ready ? g_stream_title : (char *)AutoChar(fn));
					ConvertTryUTF8(buf, title, 256);
				}
				else
				{
					if (!lastfn_data_ready)
						lstrcpynW(title, fn, 256);
					else
					{
						ConvertTryUTF8(g_stream_title, title, 256);
					}
				}
				LeaveCriticalSection(&g_lfnscs);
				if (length_in_ms) *length_in_ms = getlength();
			}
			else
			{
				lstrcpynW(title, fn, 256);
			}
		}
		return ;
	}
	else
	{
		Metadata info;
		if (info.Open(fn) == METADATA_SUCCESS)
		{
			if (title)
			{
				wchar_t mp3artist[256] = L"", mp3title[256] = L"";
				info.GetExtendedData("artist", mp3artist, 256);
				info.GetExtendedData("title", mp3title, 256);
				if (mp3artist[0] && mp3title[0])
					StringCchPrintfW(title, 256, L"%s - %s", mp3artist, mp3title);
				else if (mp3title[0])
					lstrcpynW(title, mp3title, 256);
				else
				{
					lstrcpynW(title, fn, MAX_PATH);
					PathStripPathW(title);
					PathRemoveExtensionW(title);
				}
			}

			if (fn == filename)
			{
				wchar_t ln[128]=L"";
				info.GetExtendedData("length", ln, 128);
				*length_in_ms = _wtoi(ln);
			}
			else
				*length_in_ms = getlength();
		}
		else if (fn != filename)
			*length_in_ms = getlength();
	}
}

int id3Dlg(const wchar_t *fn, HWND hwnd)
{
	return 1;
}

extern const wchar_t *id3v1_genres[];
extern size_t numGenres;

static int our_change=0;

#define GetMeta(hwnd) (Metadata *)GetPropW(GetParent(hwnd),L"mp3info")
#define SetMeta(hwnd,meta) SetPropW(GetParent(hwnd),L"mp3info",(HANDLE)meta)

static INT_PTR CALLBACK id3v1_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int my_change_v1=0;
	static const int ctrls[] =
	{
		IDC_ID3V11_TRACK,
		IDC_ID3_TITLE,
		IDC_ID3_ARTIST,
		IDC_ID3_ALBUM,
		IDC_ID3_YEAR,
		IDC_ID3_COMMENT,
		IDC_ID3_GENRE,
	};
	static const int strs_lim[] =
	{
		3,
		30,
		30,
		30,
		4,
		28,
		1,
	};
	static const char * strs[] =
	{
		"track",
		"title",
		"artist",
		"album",
		"year",
		"comment",
		"genre",
	};
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		Metadata *meta = GetMeta(hwndDlg);
		if (meta)
			meta->AddRef();
		else
		{
			meta = new Metadata();
			meta->Open((wchar_t*)lParam);
			SetMeta(hwndDlg,meta);
		}

		wchar_t genre_buf[32] = {0};
		meta->id3v1.GetString("genre",genre_buf,32);

		our_change=1;

		SendDlgItemMessage(hwndDlg, IDC_ID3_GENRE, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_ID3_GENRE, CB_SETCURSEL, -1, 0);
		for (size_t x = 0; x != numGenres; x ++)
		{
			int y = (int)SendDlgItemMessage(hwndDlg, IDC_ID3_GENRE, CB_ADDSTRING, 0, (LPARAM)id3v1_genres[x]);
			SendDlgItemMessage(hwndDlg, IDC_ID3_GENRE, CB_SETITEMDATA, y, x);

			if (_wcsicmp(genre_buf,id3v1_genres[x])==0)
				SendDlgItemMessage(hwndDlg, IDC_ID3_GENRE, CB_SETCURSEL, y, 0);
		}

		for (int i=0; i<sizeof(strs)/sizeof(char*); i++)
		{
			// make sure the edit boxes are limited to id3v1 spec sizes (trickier on number type fields)
			wchar_t buf[32] = {0};
			SendDlgItemMessage(hwndDlg,ctrls[i],EM_SETLIMITTEXT,strs_lim[i],0);
			meta->id3v1.GetString(strs[i],buf,32);
			SetDlgItemTextW(hwndDlg,ctrls[i],buf);
		}

		if (meta->id3v1.HasData())
		{
			CheckDlgButton(hwndDlg,IDC_ID3V1,TRUE);
			if (!config_write_id3v1)
			{ // if we have id3v1 writing turned off, disable controls
				for (int i=0; i<sizeof(ctrls)/sizeof(int); i++)
					EnableWindow(GetDlgItem(hwndDlg,ctrls[i]),FALSE);
			}
		}
		else
		{ // no id3v1 tag present
			for (int i=0; i<sizeof(ctrls)/sizeof(int); i++)
				EnableWindow(GetDlgItem(hwndDlg,ctrls[i]),FALSE);

			if (!config_create_id3v1)
			{ // don't allow one to be created if the settings disallow
				EnableWindow(GetDlgItem(hwndDlg,IDC_ID3V1),FALSE);
			}
		}

		our_change=0;
	}
	break;
	case WM_USER:
		if (wParam && lParam && !our_change && !my_change_v1)
		{
			Metadata *meta = GetMeta(hwndDlg);
			if (!meta) break;

			if (!config_write_id3v1)
				break;

			if (!config_create_id3v1 && !meta->id3v1.HasData())
				break;
			
			wchar_t *key = (wchar_t*)wParam;
			wchar_t *value = (wchar_t*)lParam;
			AutoChar keyA(key);
			for (int i=0; i<sizeof(strs)/sizeof(char*); i++)
			{
				if (_stricmp(keyA,strs[i])==0)
				{
					// benski> i don't think this is what we want? meta->SetExtendedData(strs[i],value);
					meta->id3v1.SetString(strs[i], value);
					wchar_t buf[2048]=L"";
					meta->id3v1.GetString(strs[i],buf,2048);

					if (!IsDlgButtonChecked(hwndDlg,IDC_ID3V1))
					{
						// re-enable stuff
						CheckDlgButton(hwndDlg,IDC_ID3V1,TRUE);
						for (int j=0; j<sizeof(ctrls)/sizeof(int); j++)
						{
							EnableWindow(GetDlgItem(hwndDlg,ctrls[j]),TRUE);
							my_change_v1++;
							SetDlgItemTextW(hwndDlg,ctrls[j],L"");
							my_change_v1--;
						}
					}
					my_change_v1++;
					if (ctrls[i] == IDC_ID3_GENRE)
					{
						int n = (int)SendDlgItemMessage(hwndDlg, IDC_ID3_GENRE, CB_FINDSTRINGEXACT, -1, (LPARAM)buf);
						SendDlgItemMessage(hwndDlg, IDC_ID3_GENRE, CB_SETCURSEL, n, 0);
					}
					else
						SetDlgItemTextW(hwndDlg,ctrls[i],buf);
					my_change_v1--;
					break;
				}
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			// this should be done by one pane ONLY. Doesn't matter which one.
			Metadata *meta = GetMeta(hwndDlg);
			if (!meta) break;
	
			Stopper stopper;
			if (!_wcsicmp(lastfn, meta->filename))
				stopper.Stop();
			int ret = meta->Save();
			stopper.Play();

			wchar_t boxtitle[256] = {0};
			switch(ret)
			{
			case SAVE_SUCCESS:
				{
					// cheap way to trigger a metadata reset in a thread-safe manner
					wchar_t d[10] = {0};
					extendedFileInfoStructW reset_info = {0};
					reset_info.filename=L".mp3";
					reset_info.metadata=L"artist";
					reset_info.ret = d;
					reset_info.retlen=10;
					SendMessage(mod.hMainWindow, WM_WA_IPC, (WPARAM)&reset_info, IPC_GET_EXTENDED_FILE_INFOW);
				}
				break;
			case SAVE_ERROR_READONLY:
				MessageBox(hwndDlg, 
					WASABI_API_LNGSTRINGW(IDS_METADATA_ERROR_READONLY),
					WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SAVING_METADATA, boxtitle, 256), 
					MB_OK);
				break;
			case SAVE_ERROR_OPENING_FILE:
				MessageBox(hwndDlg,
					WASABI_API_LNGSTRINGW(IDS_METADATA_ERROR_OPENING_FILE),
					WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SAVING_METADATA, boxtitle, 256), 
					MB_OK);
					break;
			case SAVE_APEV2_WRITE_ERROR:
				MessageBox(hwndDlg, 
					WASABI_API_LNGSTRINGW(IDS_METADATA_ERROR_APEV2),
					WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SAVING_METADATA, boxtitle, 256), 
					MB_OK);
				break;
			case SAVE_LYRICS3_WRITE_ERROR:
				MessageBox(hwndDlg,
					WASABI_API_LNGSTRINGW(IDS_METADATA_ERROR_LYRICS3),
					WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SAVING_METADATA, boxtitle, 256), 
					MB_OK);
				break;
			case SAVE_ID3V1_WRITE_ERROR:
				MessageBox(hwndDlg,
					WASABI_API_LNGSTRINGW(IDS_METADATA_ERROR_ID3V1),
					WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SAVING_METADATA, boxtitle, 256), 
					MB_OK);
				break;
			case SAVE_ID3V2_WRITE_ERROR:
				MessageBox(hwndDlg,
					WASABI_API_LNGSTRINGW(IDS_METADATA_ERROR_ID3V2),
					WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SAVING_METADATA, boxtitle, 256), 
					MB_OK);
				break;
			default:
				MessageBox(hwndDlg,
					WASABI_API_LNGSTRINGW(IDS_METADATA_ERROR_UNSPECIFIED),
					WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SAVING_METADATA, boxtitle, 256), 
					MB_OK);
				break;
			}
		}
		break;
		case IDC_ID3V1_TO_V2:
		{
			my_change_v1=1;
			Metadata *meta = GetMeta(hwndDlg);
			if (!meta) break;
			for (int i=0; i<sizeof(ctrls)/sizeof(int); i++)
			{
				wchar_t buf[2048]=L"";
				GetDlgItemTextW(hwndDlg,ctrls[i],buf,2048);
				meta->id3v2.SetString(strs[i],buf);
				SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)(wchar_t*)AutoWide(strs[i]),(LPARAM)buf);
			}
			my_change_v1=0;
		}
		break;
		case IDC_ID3V1:
		{
			our_change=1;
			BOOL checked = IsDlgButtonChecked(hwndDlg,IDC_ID3V1);
			Metadata *meta = GetMeta(hwndDlg);
			if (!meta) break;
			if (!checked)
				meta->id3v1.Clear();

			for (int i=0; i<sizeof(ctrls)/sizeof(int); i++)
			{
				EnableWindow(GetDlgItem(hwndDlg,ctrls[i]),checked);

				wchar_t buf[2048]=L"";
				if (checked)
				{
					GetDlgItemText(hwndDlg,ctrls[i],buf,2048);
					if (buf[0])
						meta->id3v1.SetString(strs[i],buf);
				}
				SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)(wchar_t*)AutoWide(strs[i]),(LPARAM)buf);
			}
			our_change=0;
		}
		break;
		default:
			if (!our_change && !my_change_v1 && (HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == CBN_SELCHANGE))
			{
				our_change=1;
				for (int i=0; i<sizeof(strs)/sizeof(char*); i++)
				{
					if (LOWORD(wParam) == ctrls[i])
					{
						wchar_t buf[2048]=L"";
						if (HIWORD(wParam) == EN_CHANGE)
							GetDlgItemTextW(hwndDlg,ctrls[i],buf,2048);
						else
						{
							LRESULT n = SendDlgItemMessage(hwndDlg, ctrls[i], CB_GETCURSEL, 0, 0);
							n = SendDlgItemMessage(hwndDlg, ctrls[i], CB_GETITEMDATA, n, 0);
							if (n>=0 && n<(LRESULT)numGenres)
								lstrcpyn(buf,id3v1_genres[n],2048);
						}
						Metadata *meta = GetMeta(hwndDlg);
						if (!meta) break;
						meta->id3v1.SetString(strs[i],buf);
						if (!meta->id3v2.HasData())
							SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)(wchar_t*)AutoWide(strs[i]),(LPARAM)buf);
					}
				}
				our_change=0;
			}
		}
		break;
	case WM_DESTROY:
	{
		Metadata *meta = GetMeta(hwndDlg);
		if (meta) meta->Release();
	}
	break;
	}
	return 0;
}

static INT_PTR CALLBACK id3v2_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int my_change_v2=0;
	static const int ctrls[] =
	{
		IDC_ID3V2_TRACK,
		IDC_ID3V2_TITLE,
		IDC_ID3V2_ARTIST,
		IDC_ID3V2_ALBUM,
		IDC_ID3V2_YEAR,
		IDC_ID3V2_COMMENT,
		IDC_ID3V2_GENRE,
		IDC_ID3V2_COMPOSER,
		IDC_ID3V2_PUBLISHER,
		IDC_ID3V2_MARTIST,
		IDC_ID3V2_RECORD,
		IDC_ID3V2_URL,
		IDC_ID3V2_ENCODER,
		IDC_ID3V2_ALBUM_ARTIST,
		IDC_ID3V2_DISC,
		IDC_TRACK_GAIN,
		IDC_ALBUM_GAIN,
		IDC_ID3V2_BPM,
	};
	static const char * strs[] =
	{
		"track",
		"title",
		"artist",
		"album",
		"year",
		"comment",
		"genre",
		"composer",
		"publisher",
		"originalartist",
		"copyright",
		"url",
		"tool",
		"albumartist",
		"disc",
		"replaygain_track_gain", // 15
		"replaygain_album_gain", // 16
		"bpm",
	};
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		our_change=1;
		SendDlgItemMessage(hwndDlg, IDC_ID3V2_GENRE, CB_RESETCONTENT, 0, 0);
		for (size_t x = 0; x != numGenres; x ++)
		{
			int y = (int)SendDlgItemMessage(hwndDlg, IDC_ID3V2_GENRE, CB_ADDSTRING, 0, (LPARAM)id3v1_genres[x]);
			SendDlgItemMessage(hwndDlg, IDC_ID3V2_GENRE, CB_SETITEMDATA, y, x);
		}

		Metadata *meta = GetMeta(hwndDlg);
		if (meta)
			meta->AddRef();
		else
		{
			meta = new Metadata();
			meta->Open((wchar_t*)lParam);
			SetMeta(hwndDlg,meta);
		}

		for (int i=0; i<sizeof(strs)/sizeof(char*); i++)
		{
			wchar_t buf[32768] = {0};
			meta->id3v2.GetString(strs[i],buf,32768);
			if((i == 15 || i == 16) && buf[0])
			{
				SetDlgItemTextW(hwndDlg,ctrls[i],L"buf");
				// this isn't nice but it localises the RG values
				// for display as they're saved in the "C" locale
				double value = _wtof_l(buf,WASABI_API_LNG->Get_C_NumericLocale());
				StringCchPrintfW(buf,64,L"%-+.2f dB", value);
			}
			SetDlgItemTextW(hwndDlg,ctrls[i],buf);
		}

		if (meta->id3v2.HasData())
			CheckDlgButton(hwndDlg,IDC_ID3V2,TRUE);
		else
			for (int i=0; i<sizeof(ctrls)/sizeof(int); i++)
				EnableWindow(GetDlgItem(hwndDlg,ctrls[i]),FALSE);

		our_change=0;
	}
	break;
	case WM_USER:
		if (wParam && lParam && !our_change && !my_change_v2)
		{
			Metadata *meta = GetMeta(hwndDlg);
			if (!meta) break;
			wchar_t *key = (wchar_t*)wParam;
			wchar_t *value = (wchar_t*)lParam;
			AutoChar keyA(key);
			for (int i=0; i<sizeof(strs)/sizeof(char*); i++)
			{
				if (_stricmp(keyA,strs[i])==0)
				{
					// benski> cut? i don't think this is what we want: meta->SetExtendedData(strs[i],value);
					meta->id3v2.SetString(strs[i], value);
					wchar_t buf[32768]=L"";
					meta->id3v2.GetString(strs[i],buf,32768);

					if (!IsDlgButtonChecked(hwndDlg,IDC_ID3V2))
					{
						// re-enable items
						CheckDlgButton(hwndDlg,IDC_ID3V2,TRUE);
						for (int j=0; j<sizeof(ctrls)/sizeof(int); j++)
						{
							EnableWindow(GetDlgItem(hwndDlg,ctrls[j]),TRUE);
							my_change_v2++;
							SetDlgItemTextW(hwndDlg,ctrls[j],L"");
							my_change_v2--;
						}
					}
					my_change_v2++;
					SetDlgItemTextW(hwndDlg,ctrls[i],buf);
					my_change_v2--;
					break;
				}
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ID3V2_TO_V1:
		{
			Metadata *meta = GetMeta(hwndDlg);
			if (!meta) break;
			my_change_v2=1;
			for (int i=0; i<sizeof(ctrls)/sizeof(int); i++)
			{
				wchar_t buf[32768]=L"";
				GetDlgItemTextW(hwndDlg,ctrls[i],buf,32768);
				meta->id3v1.SetString(strs[i],buf);
				meta->GetExtendedData(strs[i], buf, 32768);
				SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)(wchar_t*)AutoWide(strs[i]),(LPARAM)buf);
			}
			my_change_v2=0;
		}
		break;
		case IDC_ID3V2:
		{
			our_change=1;
			BOOL checked = IsDlgButtonChecked(hwndDlg,IDC_ID3V2);
			Metadata *meta = GetMeta(hwndDlg);
			if (!meta) break;
			if (!checked)
				meta->id3v2.Clear();

			for (int i=0; i<sizeof(ctrls)/sizeof(int); i++)
			{
				EnableWindow(GetDlgItem(hwndDlg,ctrls[i]),checked);

				wchar_t buf[32768]=L"";
				if (checked)
				{
					GetDlgItemText(hwndDlg,ctrls[i],buf,32768);
					if (buf[0])
						meta->id3v2.SetString(strs[i],buf);
				}
				meta->GetExtendedData(strs[i],buf,32768);
				SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)(wchar_t*)AutoWide(strs[i]),(LPARAM)buf);
			}
			our_change=0;
		}
		break;
		case IDOK:
			{
						extern Metadata *m_ext_get_mp3info;
		if (m_ext_get_mp3info)
			m_ext_get_mp3info->Release();
		m_ext_get_mp3info=0;
			}
			break;
		default:
			if (!our_change && !my_change_v2 && (HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_EDITUPDATE))
			{
				our_change=1;
				for (int i=0; i<sizeof(strs)/sizeof(char*); i++)
				{
					if (LOWORD(wParam) == ctrls[i])
					{
						wchar_t buf[32768] = {0};
						if (HIWORD(wParam) == EN_CHANGE)
							GetDlgItemTextW(hwndDlg,ctrls[i],buf,32768);
						else
						{
							LRESULT n = SendMessage(GetDlgItem(hwndDlg, ctrls[i]), CB_GETCURSEL, 0, 0);
							n = SendMessage(GetDlgItem(hwndDlg, ctrls[i]), CB_GETITEMDATA, n, 0);
							if (n>=0 && n<(LRESULT)numGenres)
								lstrcpyn(buf,id3v1_genres[n],32768);
							else{
								GetDlgItemTextW(hwndDlg,ctrls[i],buf,32768);
							}
						}
						Metadata *meta = GetMeta(hwndDlg);
						if (!meta) break;
						meta->id3v2.SetString(strs[i],buf);
						meta->GetExtendedData(strs[i],buf,32768);
						SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)(wchar_t*)AutoWide(strs[i]),(LPARAM)buf);
					}
				}
				our_change=0;
			}
		}
		break;
	case WM_DESTROY:
	{
		Metadata *meta = GetMeta(hwndDlg);
		if (meta)
			meta->Release();
	}
	break;
	}
	return 0;
}

static INT_PTR CALLBACK lyrics3_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static const int ctrls[] =
	{
		IDC_LYRICS3_TITLE,
		IDC_LYRICS3_ARTIST,
		IDC_LYRICS3_ALBUM,
	};
	static const char * strs[] =
	{
		"title",
		"artist",
		"album",
	};
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		Metadata *meta = GetMeta(hwndDlg);
		if (meta)
			meta->AddRef();
		else
		{
			meta = new Metadata();
			meta->Open((wchar_t*)lParam);
			SetMeta(hwndDlg,meta);
		}

		for (int i=0; i<sizeof(strs)/sizeof(char*); i++)
		{
			wchar_t buf[2048] = {0};
			SendDlgItemMessage(hwndDlg,ctrls[i],EM_SETLIMITTEXT,250,0);
			meta->lyrics3.GetString(strs[i],buf,250);
			SetDlgItemTextW(hwndDlg,ctrls[i],buf);
		}

		if (meta->lyrics3.HasData())
			CheckDlgButton(hwndDlg,IDC_LYRICS3,TRUE);
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LYRICS3:
		{
			BOOL checked = IsDlgButtonChecked(hwndDlg,IDC_LYRICS3);
			Metadata *meta = GetMeta(hwndDlg);
			if (!meta) break;
			if (!checked)
				meta->lyrics3.Clear();
			else	// clear the dirty state if re-enabled so we don't lose the lyrics3 tag
				meta->lyrics3.ResetDirty();

			for (int i=0; i<sizeof(ctrls)/sizeof(int); i++)
			{
				EnableWindow(GetDlgItem(hwndDlg,ctrls[i]),checked);

				wchar_t buf[2048]=L"";
				if (checked)
				{
					GetDlgItemText(hwndDlg,ctrls[i],buf,2048);
					if (buf[0])
						meta->lyrics3.SetString(strs[i],buf);
				}
				meta->GetExtendedData(strs[i],buf,2048);
				// if we don't flag this then we can lose info in the id3v1 and v2 tags which is definitely bad
				our_change++;
				SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)(wchar_t*)AutoWide(strs[i]),(LPARAM)buf);
				our_change--;
			}
		}
		break;
		}
		break;
	case WM_DESTROY:
	{
		Metadata *meta = GetMeta(hwndDlg);
		if (meta) meta->Release();
	}
	break;
	}
	return 0;
}
/* ================
APEv2 Editor Tab
================ */



static INT_PTR CALLBACK apev2_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static W_ListView listview;
	static int my_change_ape=0;
	switch (uMsg)
	{
		case WM_NOTIFYFORMAT:
			return NFR_UNICODE;

		case WM_INITDIALOG:
		{
			our_change++;
			Metadata *meta = GetMeta(hwndDlg);
			if (meta)
			{
				meta->AddRef();
			}
			else
			{
				meta = new Metadata();
				meta->Open((wchar_t*)lParam);
				SetMeta(hwndDlg,meta);
			}

			if (meta->apev2.HasData())
				CheckDlgButton(hwndDlg,IDC_APEV2,TRUE);

			listview.setwnd(GetDlgItem(hwndDlg, IDC_APE_LIST));
			listview.SetDoubleBuffered(true);
			listview.AddCol(WASABI_API_LNGSTRINGW(IDS_NAME), 82);
			listview.AddCol(WASABI_API_LNGSTRINGW(IDS_VALUE), 160);

			listview.SetVirtualCount((int)meta->apev2.GetNumItems());
			listview.AutoSizeColumn(0);
			listview.AutoSizeColumn(1);

			SetDlgItemTextW(hwndDlg,IDC_APE_KEY,L"");
			SetDlgItemTextW(hwndDlg,IDC_APE_VALUE,L"");
			EnableWindow(GetDlgItem(hwndDlg,IDC_APE_KEY),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_APE_VALUE),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_APE_DELETE),FALSE);
			our_change--;
			return 1;
		}
		break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_APE_KEY:
				case IDC_APE_VALUE:
					if(HIWORD(wParam) == EN_CHANGE) 
					{
						int selected = listview.GetNextSelected();
						if (selected != LB_ERR)
						{
							listview.RefreshItem(selected);
						}
					}
					else if(HIWORD(wParam) == EN_KILLFOCUS) 
					{
						Metadata *meta = GetMeta(hwndDlg);
						if (!meta) break;

						my_change_ape++;

						char key[1024] = {0};
						wchar_t value[32768] = {0};
						GetDlgItemTextA(hwndDlg, IDC_APE_KEY, key, 1024);
						GetDlgItemText(hwndDlg, IDC_APE_VALUE, value, 32768);
						int selected = listview.GetNextSelected();
						if (selected != LB_ERR)
						{
							meta->apev2.SetKeyValueByIndex(selected, key, value);
						}

						const wchar_t *winamp_key = APE::MapApeKeyToWinampKeyW(key);
						if (winamp_key)
						{
							our_change++;
							SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)winamp_key,(WPARAM)value);
							our_change--;
						}
						my_change_ape--;
					}
					break;

			case IDC_APEV2:
				{
					BOOL checked = IsDlgButtonChecked(hwndDlg,IDC_APEV2);
					Metadata *meta = GetMeta(hwndDlg);
					if (!meta) break;
					if (!checked)
						meta->apev2.MarkClear();
					else	// clear the dirty state if re-enabled so we don't lose the apev2 tag
						meta->apev2.ResetDirty();
				}
				break;

			case IDC_DELETE_ALL:
				{
					Metadata *meta = GetMeta(hwndDlg);
					if (!meta) break;
					my_change_ape++;
					listview.UnselectAll();
					meta->apev2.Clear();
					listview.SetVirtualCount((int)meta->apev2.GetNumItems());
					my_change_ape--;
				}
				break;

			case IDC_APE_ADD:
				{
					Metadata *meta = GetMeta(hwndDlg);
					if (!meta) break;
					int index = listview.GetCount();
					if (meta->apev2.AddItem() == APEv2::APEV2_SUCCESS)
					{
						listview.SetVirtualCount((int)meta->apev2.GetNumItems());
						listview.SetSelected(index);
						listview.ScrollTo(index);
						SetFocus(GetDlgItem(hwndDlg, IDC_APE_KEY));	
					}
				}
				break;

			case IDC_APE_DELETE:
				{
					Metadata *meta = GetMeta(hwndDlg);
					if (!meta) break;
					int selected = listview.GetNextSelected();
					if (selected != LB_ERR)
					{
						listview.UnselectAll();
						meta->apev2.RemoveItem(selected);
						listview.SetVirtualCount((int)meta->apev2.GetNumItems());
					}
				}
				break;
			}
			break;

		case WM_NOTIFY:
		{
			LPNMHDR l=(LPNMHDR)lParam;
			if (l->idFrom==IDC_APE_LIST) switch (l->code)
			{
				case LVN_GETDISPINFO:
				{
					Metadata *meta = GetMeta(hwndDlg);
					if (meta)
					{
						NMLVDISPINFO *lpdi = (NMLVDISPINFO*) l;
						if (lpdi->item.mask & LVIF_TEXT)
						{
							int selected = listview.GetNextSelected();
							switch (lpdi->item.iSubItem)
							{
								case 0:
								{
									if (lpdi->item.iItem == selected)
									{
										GetDlgItemText(hwndDlg, IDC_APE_KEY, lpdi->item.pszText, lpdi->item.cchTextMax);
									}
									else
									{
										const char *key=0;
										meta->apev2.EnumValue(lpdi->item.iItem, &key, 0, 0);
										MultiByteToWideCharSZ(CP_ACP, 0, key?key:"", -1, lpdi->item.pszText, lpdi->item.cchTextMax);
									}
								}
								return 0;

								case 1:
								{
									if (lpdi->item.iItem == selected)
									{
										GetDlgItemText(hwndDlg, IDC_APE_VALUE,  lpdi->item.pszText, lpdi->item.cchTextMax);
									}
									else
									{
										const char *key=0;
										meta->apev2.EnumValue(lpdi->item.iItem, &key, lpdi->item.pszText, lpdi->item.cchTextMax);
									}
								}
								return 0;
							}
						}
					}
				}
				break;

				case LVN_KEYDOWN:
				break;

				case LVN_ITEMCHANGED:
				{
					my_change_ape++;
					LPNMLISTVIEW lv=(LPNMLISTVIEW)lParam;
					if (lv->uNewState & LVIS_SELECTED)
					{
						Metadata *meta = GetMeta(hwndDlg);
						if (meta)
						{
							const char *key=0;
							wchar_t value[32768] = {0};
							meta->apev2.EnumValue(lv->iItem, &key, value, 32768);
							SetDlgItemTextA(hwndDlg,IDC_APE_KEY,key);
							SetDlgItemText(hwndDlg,IDC_APE_VALUE,value);
							BOOL editable = meta->apev2.IsItemReadOnly(lv->iItem)?FALSE:TRUE;
							EnableWindow(GetDlgItem(hwndDlg,IDC_APE_KEY),editable);
							EnableWindow(GetDlgItem(hwndDlg,IDC_APE_VALUE),editable);
							EnableWindow(GetDlgItem(hwndDlg,IDC_APE_DELETE),TRUE);
							listview.RefreshItem(lv->iItem);
						}
					}
					if (lv->uOldState & LVIS_SELECTED)
					{
						SetDlgItemTextW(hwndDlg,IDC_APE_KEY,L"");
						SetDlgItemTextW(hwndDlg,IDC_APE_VALUE,L"");
						EnableWindow(GetDlgItem(hwndDlg,IDC_APE_KEY),FALSE);
						EnableWindow(GetDlgItem(hwndDlg,IDC_APE_VALUE),FALSE);
						EnableWindow(GetDlgItem(hwndDlg,IDC_APE_DELETE),FALSE);
					}
					my_change_ape--;
				}
			}
		}
		break;

		case WM_USER:
			if (wParam && lParam && !our_change && !my_change_ape)
			{
				Metadata *meta = GetMeta(hwndDlg);
				if (meta)
				{
					const wchar_t *keyW = (const wchar_t *)wParam;
					const wchar_t *value = (const wchar_t *)lParam;
					AutoChar key(keyW);
					my_change_ape++;
					meta->apev2.SetString(key, value);
					listview.UnselectAll();
					listview.SetVirtualCount((int)meta->apev2.GetNumItems());
					listview.RefreshAll();
					my_change_ape--;
				}
			}
			break;

		case WM_DESTROY:
		{
			Metadata *meta = GetMeta(hwndDlg);
			if (meta) meta->Release();
		}
		break;
	}
	return 0;
}

extern "C"
{
	// return 1 if you want winamp to show it's own file info dialogue, 0 if you want to show your own (via In_Module.InfoBox)
	// if returning 1, remember to implement winampGetExtendedFileInfo("formatinformation")!
	__declspec(dllexport) int winampUseUnifiedFileInfoDlg(const wchar_t * fn)
	{
		if (!_wcsnicmp(fn, L"file://", 7)) fn += 7;
		if (PathIsURLW(fn)) return 2;
		return 1;
	}

	// should return a child window of 513x271 pixels (341x164 in msvc dlg units), or return NULL for no tab.
	// Fill in name (a buffer of namelen characters), this is the title of the tab (defaults to "Advanced").
	// filename will be valid for the life of your window. n is the tab number. This function will first be
	// called with n == 0, then n == 1 and so on until you return NULL (so you can add as many tabs as you like).
	// The window you return will recieve WM_COMMAND, IDOK/IDCANCEL messages when the user clicks OK or Cancel.
	// when the user edits a field which is duplicated in another pane, do a SendMessage(GetParent(hwnd),WM_USER,(WPARAM)L"fieldname",(LPARAM)L"newvalue");
	// this will be broadcast to all panes (including yours) as a WM_USER.
	__declspec(dllexport) HWND winampAddUnifiedFileInfoPane(int n, const wchar_t * filename, HWND parent, wchar_t *name, size_t namelen)
	{
		if (n == 0)
		{
			SetPropW(parent,L"INBUILT_NOWRITEINFO", (HANDLE)1);
			lstrcpyn(name,L"ID3v1", (int)namelen);
			return WASABI_API_CREATEDIALOGPARAMW(IDD_INFO_ID3V1, parent, id3v1_dlgproc, (LPARAM)filename);
		}
		if (n == 1)
		{
			lstrcpyn(name,L"ID3v2", (int)namelen);
			return WASABI_API_CREATEDIALOGPARAMW(IDD_INFO_ID3V2, parent, id3v2_dlgproc, (LPARAM)filename);
		}
		if (n == 2)
		{
			Metadata *meta = (Metadata *)GetPropW(parent, L"mp3info");
			if (meta->lyrics3.HasData())
			{
				lstrcpyn(name,L"Lyrics3", (int)namelen);
				return WASABI_API_CREATEDIALOGPARAMW(IDD_INFO_LYRICS3, parent, lyrics3_dlgproc, (LPARAM)filename);
			}
			else if (meta->apev2.HasData())
			{
				lstrcpyn(name,L"APEv2", (int)namelen);
				return WASABI_API_CREATEDIALOGPARAMW(IDD_INFO_APEV2, parent, apev2_dlgproc, (LPARAM)filename);
			}
		}
		if (n == 3)
		{
			Metadata *meta = (Metadata *)GetPropW(parent, L"mp3info");
			if (meta->lyrics3.HasData() && meta->apev2.HasData())
			{
				lstrcpyn(name,L"APEv2", (int)namelen);
				return WASABI_API_CREATEDIALOGPARAMW(IDD_INFO_APEV2, parent, apev2_dlgproc, (LPARAM)filename);
			}
		}
		return NULL;
	}
}