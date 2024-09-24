#include "main.h"
#include "resource.h"
//#include <qedit.h>
#include "header_asf.h"
#include "header_avi.h"
#include "header_mpg.h"
#include "header_wav.h"
#include "../nu/AutoChar.h"
#include "../Agave/Language/api_language.h"

#include <strsafe.h>

static const wchar_t *m_fn;

extern const wchar_t *extension(const wchar_t *fn);
extern wchar_t m_lastfn[];
extern unsigned int m_nbframes;
extern DWORD m_avgfps_start;

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

int GetFileLength(const wchar_t *filename);
void getInfo(const wchar_t *fn, wchar_t *linetext, int linetextCch, wchar_t *fulltext, int fulltextCch, int *bitrate, int *channel)
{
	wchar_t tmp[512] = {0, };
	wchar_t tmpline[512] = {0, };
	int br = 0;

	const wchar_t *ext = extension(fn);
	__int64 size = 0;
	HANDLE hFile = CreateFileW(fn, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		size = FileSize64(hFile);
		CloseHandle(hFile);
	}
	int l = GetFileLength(fn);
	wchar_t *pTmpLine = tmpline, *pTmp = tmp;
	size_t pTmpLineSize = 512, pTmpSize = 512;

	if (!_wcsicmp(ext, L"asf") || !_wcsicmp(ext, L"wmv") || !_wcsicmp(ext, L"wma"))
	{
		HeaderAsf ha;
		if (ha.getInfos(fn))
		{
			StringCchCopyExW(pTmpLine, pTmpLineSize, L"DShow (WMV): ", &pTmpLine, &pTmpLineSize, 0);
			if (ha.has_video)
			{
				StringCchPrintfExW(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRINGW(IDS_VIDEO_SIZE), ha.video_w, ha.video_h);
				StringCchPrintfExW(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRINGW(IDS_VIDEO), ha.video_w, ha.video_h);
			}
			/* benski> cut
			if (ha.has_audio)
			{
				char type[16] = {0};
				StringCchPrintfEx(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRING(IDS_AUDIO), ha.audio_srate, ha.audio_bps, ha.audio_nch);
				StringCchPrintfEx(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRING(IDS_AUDIO_S), ha.audio_srate / 1000, ha.audio_bps, WASABI_API_LNGSTRING_BUF((ha.audio_nch == 2 ? IDS_STEREO : IDS_MONO),type,16));
			}
			int l = ha.length / 1000;
			*/

		}
	}
	else if (!_wcsicmp(ext, L"avi") || !_wcsicmp(ext, L"divx"))
	{
		HeaderAvi ha;
		if (ha.getInfos(fn))
		{

			StringCchCopyExW(pTmpLine, pTmpLineSize, L"DShow (AVI): ", &pTmpLine, &pTmpLineSize, 0);
			if (ha.has_video)
			{
				StringCchPrintfExW(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRINGW(IDS_VIDEO_SIZE), ha.video_w, ha.video_h);
				StringCchPrintfExW(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRINGW(IDS_VIDEO), ha.video_w, ha.video_h);
			}
			if (ha.has_audio)
			{
				wchar_t type[16] = {0};
				StringCchPrintfExW(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRINGW(IDS_AUDIO), ha.audio_srate, ha.audio_bps, ha.audio_nch);
				StringCchPrintfExW(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRINGW(IDS_AUDIO_S), ha.audio_srate / 1000, ha.audio_bps, WASABI_API_LNGSTRINGW_BUF((ha.audio_nch == 2 ? IDS_STEREO : IDS_MONO),type,16));
			}
			/* benski> cut
			int l = ha.length / 1000;
			if (l)
			{
				br = (int)(((double)size * 8 / 1000) / l);
				StringCchPrintfEx(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRING(IDS_LENGTH), l / 3600, (l / 60) % 60, l % 60, br);
				StringCchPrintfEx(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRING(IDS_DURATION), l / 3600, (l / 60) % 60, l % 60, br);
			}
			*/
		}
		if (channel)
		{
			*channel = ha.audio_nch;
		}
	}
	else if (!_wcsicmp(ext, L"mpg") || !_wcsicmp(ext, L"mpeg"))
	{
		HeaderMpg ha;
		if (ha.getInfos(fn))
		{
			StringCchCopyExW(pTmpLine, pTmpLineSize, L"DShow (MPEG): ", &pTmpLine, &pTmpLineSize, 0);
			if (ha.has_video)
			{
				StringCchPrintfExW(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRINGW(IDS_VIDEO_SIZE_KBPS), ha.video_w, ha.video_h, ha.video_bitrate / 1000);
				StringCchPrintfExW(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRINGW(IDS_VIDEO_KBPS), ha.video_w, ha.video_h, ha.video_bitrate / 1000);
			}
			if (ha.has_audio)
			{
				wchar_t type[16] = {0};
				StringCchPrintfExW(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRINGW(IDS_AUDIO_KBPS), ha.audio_srate, ha.audio_bps, ha.audio_nch, ha.audio_bitrate / 1000);
				StringCchPrintfExW(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRINGW(IDS_AUDIO_S), ha.audio_srate / 1000, ha.audio_bps, WASABI_API_LNGSTRINGW_BUF((ha.audio_nch == 2 ? IDS_STEREO : IDS_MONO),type,16), ha.audio_bitrate / 1000);
			}
			/* benski> cut
			int l = ha.length / 1000;
			if (l)
			{
				br = (int)(((double)size * 8 / 1000) / l);
				StringCchPrintfEx(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRING(IDS_LENGTH), l / 3600, (l / 60) % 60, l % 60, br);
				StringCchPrintfEx(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRING(IDS_DURATION), l / 3600, (l / 60) % 60, l % 60, br);
			}
			*/
		}
	}
	else if (!_wcsicmp(ext, L"wav"))
	{
		HeaderWav ha;
		if (ha.getInfos(fn))
		{

			StringCchCopyExW(pTmpLine, pTmpLineSize, L"DShow (WAV): ", &pTmpLine, &pTmpLineSize, 0);
			if (ha.has_audio)
			{
				char type[16] = {0};
				StringCchPrintfExW(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRINGW(IDS_AUDIO_CH), ha.audio_srate, ha.audio_bps, ha.audio_nch);
				StringCchPrintfExW(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRINGW(IDS_AUDIO_S), ha.audio_srate / 1000, ha.audio_bps, WASABI_API_LNGSTRING_BUF((ha.audio_nch == 2 ? IDS_STEREO : IDS_MONO),type,16));
			}
			/* benski> cut
			int l = ha.length / 1000;
			if (l)
			{
				br = (int)(((double)size * 8 / 1000) / l);
				StringCchPrintfEx(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRING(IDS_LENGTH), l / 3600, (l / 60) % 60, l % 60, br);
				StringCchPrintfEx(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRING(IDS_DURATION), l / 3600, (l / 60) % 60, l % 60, br);
			}
			else
				br =  MulDiv(ha.audio_bps*ha.audio_nch, ha.audio_srate, 1000);
			*/
		}

	}

			if (l != -1000)
			{
				br = (int)(((double)size * 8.0) / l);
				l/=1000;
				StringCchPrintfExW(pTmp, pTmpSize, &pTmp, &pTmpSize, 0, WASABI_API_LNGSTRINGW(IDS_LENGTH), l / 3600, (l / 60) % 60, l % 60, br);
				StringCchPrintfExW(pTmpLine, pTmpLineSize, &pTmpLine, &pTmpLineSize, 0, WASABI_API_LNGSTRINGW(IDS_DURATION), l / 3600, (l / 60) % 60, l % 60, br);
			}
			

	if (linetext)
		lstrcpynW(linetext, tmpline, linetextCch);
	if (fulltext)
		lstrcpynW(fulltext, tmp, fulltextCch);
	if (bitrate)
		*bitrate = br;
}

INT_PTR CALLBACK infoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemTextW(hwndDlg, IDC_FILENAME, m_fn);
		SetDlgItemTextW(hwndDlg, IDC_INFO, WASABI_API_LNGSTRINGW(IDS_NO_AVAILABLE_INFO));
		SetDlgItemTextW(hwndDlg, IDC_FPS, L"");

		{
			wchar_t text[512] = {0};
			getInfo(m_fn, NULL, 0, text, 512, NULL, NULL);
			SetDlgItemTextW(hwndDlg, IDC_INFO, text);
		}

		if (!_wcsicmp(m_fn, m_lastfn) && pGraphBuilder)
		{
			CComPtr<IEnumFilters> pEnumFilters;
			pGraphBuilder->EnumFilters(&pEnumFilters);
			if (!pEnumFilters) break;

			pEnumFilters->Reset();
			do
			{
				ULONG nfetched = 0;
				CComPtr<IBaseFilter> pFilter;
				pEnumFilters->Next(1, &pFilter, &nfetched);
				if (!nfetched)
					break;

				FILTER_INFO fi;
				pFilter->QueryFilterInfo(&fi);

				if (!_wcsicmp(fi.achName, L"Null Audio")
				        || !_wcsicmp(fi.achName, L"Null Video")
				        || !_wcsicmp(fi.achName, m_fn))
				{
					if (fi.pGraph)
						fi.pGraph->Release();
					continue;
				}

				LRESULT a = SendDlgItemMessageW(hwndDlg, IDC_FILTERLIST, LB_ADDSTRING, 0, (LPARAM)fi.achName);
				SendDlgItemMessage(hwndDlg, IDC_FILTERLIST, LB_SETITEMDATA, a, (LPARAM)(IBaseFilter *)pFilter); //FUCKO: if playback changes

				if (fi.pGraph)
					fi.pGraph->Release();
			}
			while (1);

			SendMessage(hwndDlg, WM_TIMER, 0x123, 0);
			SetTimer(hwndDlg, 0x123, 500, NULL);
		}

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_FILTERLIST:
			if (HIWORD(wParam) == LBN_DBLCLK)
			{
				LRESULT sel = SendDlgItemMessage(hwndDlg, IDC_FILTERLIST, LB_GETCURSEL, 0, 0);
				if (sel == LB_ERR) break;

				CComPtr<IBaseFilter> pFilter;
				pFilter = (IBaseFilter *)SendDlgItemMessage(hwndDlg, IDC_FILTERLIST, LB_GETITEMDATA, sel, 0);

				CComQIPtr<ISpecifyPropertyPages> pSPP(pFilter);
				if (!pSPP) break;

				CAUUID pages;
				pSPP->GetPages(&pages);

				IBaseFilter *f = pFilter;
				OleCreatePropertyFrame(
				    hwndDlg, 30, 30, NULL,
				    1, (IUnknown**)&f,
				    pages.cElems, pages.pElems,
				    0, 0, NULL);
				CoTaskMemFree(pages.pElems);
			}
			break;
		case IDOK:
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
		}
		break;
	case WM_TIMER:
		if (wParam == 0x123 && pGraphBuilder && !_wcsicmp(m_fn, m_lastfn))
		{
			DWORD t = GetTickCount() - m_avgfps_start;
			if (t)
			{
				wchar_t tmp[512] = {0};
				StringCchPrintfW(tmp, 512, WASABI_API_LNGSTRINGW(IDS_AVERAGE_FPS), (double)m_nbframes*1000 / t);
				SetDlgItemTextW(hwndDlg, IDC_FPS, tmp);
			}
		}
		break;
	}
	return FALSE;
}

void doInfo(HINSTANCE hInstance, HWND hwndParent, const wchar_t *fn)
{
	static int running;
	if (running) return ;
	running = 1;
	m_fn = fn;
	WASABI_API_DIALOGBOXW(IDD_FILEINFO, hwndParent, infoProc);
	running = 0;
}