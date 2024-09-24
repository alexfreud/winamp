#include "../nsavi/nsavi.h"
#include "api__in_avi.h"
#include "../nu/ListView.h"
#include "../nu/AutoWide.h"
#include "resource.h"
#include "main.h"
#include <strsafe.h>

struct KnownField
{
	uint32_t field;
	wchar_t name[256]; // TODO: change to resource ID
};

static KnownField known_fields[] = 
{
	{nsaviFOURCC('I','S','F','T'), L"Tool"}, // IDS_FIELD_TOOL
	{nsaviFOURCC('I','A','R','T'), L"Artist"}, // IDS_FIELD_ARTIST
	{nsaviFOURCC('I','P','U','B'), L"Publisher"}, // IDS_FIELD_PUBLISHER
	{nsaviFOURCC('I','A','L','B'), L"Album"}, // IDS_FIELD_ALBUM
	{nsaviFOURCC('I','C','O','M'), L"Composer"}, // IDS_FIELD_COMPOSER
	{nsaviFOURCC('I','G','N','R'), L"Genre"}, // IDS_FIELD_GENRE
	{nsaviFOURCC('I','C','M','T'), L"Comment"}, // IDS_FIELD_COMMENT
	{nsaviFOURCC('I','N','A','M'), L"Title"}, // IDS_FIELD_TITLE
	{nsaviFOURCC('I','C','O','P'), L"Copyright"}, // IDS_FIELD_COPYRIGHT
};

static KnownField known_video_codecs[] = 
{
	{nsaviFOURCC('V','P','6','0'), L"On2 VP6"},
	{nsaviFOURCC('V','P','6','1'), L"On2 VP6"},
	{nsaviFOURCC('V','P','6','2'), L"On2 VP6"},

	{nsaviFOURCC('X','V','I','D'), L"MPEG-4 Part 2"},
	{nsaviFOURCC('x','v','i','d'), L"MPEG-4 Part 2"},

	{nsaviFOURCC('d','i','v','x'), L"MPEG-4 Part 2"},
	{nsaviFOURCC('D','I','V','X'), L"MPEG-4 Part 2"},
	{nsaviFOURCC('D','X','5','0'), L"MPEG-4 Part 2"},

	{nsaviFOURCC('m','p','4','v'), L"MPEG-4 Part 2"},

	{nsaviFOURCC('S','E','D','G'), L"MPEG-4 Part 2"},

	{nsaviFOURCC('H','2','6','4'), L"H.264"},

	{nsaviFOURCC('M','J','P','G'), L"Motion JPEG"},

	{nsaviFOURCC('t','s','c','c'), L"TechSmith"},

	{nsaviFOURCC('c','v','i','d'), L"Cinepack"},

	{nsaviFOURCC('M','P','G','4'), L"MS-MPEG-4 v1"},
	{nsaviFOURCC('M','P','4','1'), L"MS-MPEG-4 v1"},
	{nsaviFOURCC('M','P','4','2'), L"MS-MPEG-4 v2"},
	{nsaviFOURCC('M','P','4','3'), L"MS-MPEG-4 v3"},

	{nsavi::video_format_rgb, L"RGB"},
	{nsavi::video_format_rle8, L"8bpp RLE"},
	{nsavi::video_format_rle4, L"4bpp RLE"},
};

static KnownField known_audio_codecs[] = 
{
	{nsavi::audio_format_pcm, L"Wave"},
	{nsavi::audio_format_ms_adpcm, L"Microsoft ADPCM"},
	{nsavi::audio_format_alaw, L"A-law"},
	{nsavi::audio_format_ulaw, L"μ-law"},
	{nsavi::audio_format_ima_adpcm, L"IMA ADPCM"},
	{nsavi::audio_format_truespeech, L"DSP Truespeech"},
	{nsavi::audio_format_mp2, L"MPEG Layer 2"},
	{nsavi::audio_format_mp3, L"MPEG Layer 3"},
	{nsavi::audio_format_a52, L"ATSC A/52 (AC3)"},
	{nsavi::audio_format_aac, L"AAC"},
	{nsavi::audio_format_vorbis, L"Vorbis"},	
	{nsavi::audio_format_speex, L"Speex"},	
	{nsavi::audio_format_extensible, L"Extensible Wave"},
	{nsavi::audio_format_dts, L"DTS"},

};

enum
{
	COLUMN_TRACK_TYPE = 0,
	COLUMN_CODEC_NAME = 1,
	COLUMN_CODEC_ID = 2,
	COLUMN_DESCRIPTION = 3,
	COLUMN_STREAM_NAME = 4,
};

static const wchar_t *FindKnownName(const KnownField *fields, size_t num_fields, uint32_t value)
{
	for (size_t i=0;i!=num_fields;i++)
	{
		if (fields[i].field == value)
		{
			return fields[i].name;
		}
	}
	return 0;
}

static void MakeStringFromFOURCC(wchar_t *str, uint32_t fourcc)
{
	const uint8_t *characters = (const uint8_t *)&(fourcc);
	if (fourcc < 65536)
	{
		StringCchPrintfW(str, 5, L"%X", fourcc);
	}

	else
	{
		str[0] = characters[0];
		str[1] = characters[1];
		str[2] = characters[2];
		str[3] = characters[3];
		str[4] = 0;
	}
}


static INT_PTR CALLBACK InfoDialog_Metadata(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			nsavi::Metadata *metadata = (nsavi::Metadata *)lParam;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);

			W_ListView list_view(hwndDlg, IDC_TRACKLIST);

			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_FIELD), 100);
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_FOURCC), 75);
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_VALUE), 250);

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
					list_view.SetItemText(n, 2, AutoWide(itr->second, CP_ACP/*UTF8*/));
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

void GetVideoCodecName(wchar_t *str, size_t str_cch, nsavi::STRF *stream_format)
{
	nsavi::video_format *format = (nsavi::video_format *)stream_format;
	const wchar_t *codec_name = FindKnownName(known_video_codecs, sizeof(known_video_codecs)/sizeof(known_video_codecs[0]), format->compression);
	if (codec_name)
		StringCchCopy(str, str_cch, codec_name);
	else
		MakeStringFromFOURCC(str, format->compression);
}

void GetVideoCodecDescription(wchar_t *str, size_t str_cch, nsavi::STRF *stream_format)
{
	nsavi::video_format *format = (nsavi::video_format *)stream_format;
	StringCchPrintf(str, str_cch, L"%ux%u", format->width, format->height);
}

void GetAudioCodecName(wchar_t *str, size_t str_cch, nsavi::STRF *stream_format)
{
	nsavi::audio_format *format = (nsavi::audio_format *)stream_format;
	const wchar_t *codec_name = FindKnownName(known_audio_codecs, sizeof(known_audio_codecs)/sizeof(known_audio_codecs[0]), format->format);
	if (codec_name)
		StringCchCopy(str, str_cch, codec_name);
	else
		MakeStringFromFOURCC(str, format->format);
}

void GetAudioCodecDescription(wchar_t *str, size_t str_cch, nsavi::STRF *stream_format)
{
	nsavi::audio_format *format = (nsavi::audio_format *)stream_format;
	if (format->average_bytes_per_second)
	{
		StringCchPrintf(str, str_cch, L"%u %s", format->average_bytes_per_second / 125UL, WASABI_API_LNGSTRINGW(IDS_KBPS));
	}
	else
		str[0]=0;
}

static INT_PTR CALLBACK InfoDialog_Tracks(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			nsavi::Metadata *metadata = (nsavi::Metadata *)lParam;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);

			W_ListView list_view(hwndDlg, IDC_TRACKLIST);

			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_TRACK_TYPE), 100);
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_CODEC_NAME), 100);
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_CODEC_ID), 75);			
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_DESCRIPTION), 100);
			list_view.AddCol(WASABI_API_LNGSTRINGW(IDS_COLUMN_STREAM_NAME), 100);

			nsavi::HeaderList header_list;
			if (metadata->GetHeaderList(&header_list) == nsavi::READ_OK)
			{
				for (size_t i=0;i!=header_list.stream_list_size;i++)
				{
					int n;
					const nsavi::STRL &stream = header_list.stream_list[i];
					switch(stream.stream_header->stream_type)
					{
					case nsavi::stream_type_audio:
						{
							n = list_view.AppendItem(WASABI_API_LNGSTRINGW(IDS_TYPE_AUDIO), 0);
							nsavi::audio_format *format = (nsavi::audio_format *)stream.stream_format;
							wchar_t codec_id[5] = {0};
							MakeStringFromFOURCC(codec_id, format->format);
							list_view.SetItemText(n, COLUMN_CODEC_ID, codec_id);
							const wchar_t *codec_name = FindKnownName(known_audio_codecs, sizeof(known_audio_codecs)/sizeof(known_audio_codecs[0]), format->format);
							if (codec_name)
								list_view.SetItemText(n, COLUMN_CODEC_NAME, codec_name);
							else
								list_view.SetItemText(n, COLUMN_CODEC_NAME, codec_id);

							wchar_t description[256] = {0};
							GetAudioCodecDescription(description, 256, stream.stream_format);
							list_view.SetItemText(n, COLUMN_DESCRIPTION, description);
						}
						break;
					case nsavi::stream_type_video:
						{
							n = list_view.AppendItem(WASABI_API_LNGSTRINGW(IDS_TYPE_VIDEO), 0);
							nsavi::video_format *format = (nsavi::video_format *)stream.stream_format;
							wchar_t fourcc[5] = {0};
							MakeStringFromFOURCC(fourcc, format->compression);
							list_view.SetItemText(n, COLUMN_CODEC_ID, fourcc);
							const wchar_t *codec_name = FindKnownName(known_video_codecs, sizeof(known_video_codecs)/sizeof(known_video_codecs[0]), format->compression);
							if (codec_name)
								list_view.SetItemText(n, COLUMN_CODEC_NAME, codec_name);
							else
								list_view.SetItemText(n, COLUMN_CODEC_NAME, fourcc);
							wchar_t description[256] = {0};
							GetVideoCodecDescription(description, 256, stream.stream_format);
							list_view.SetItemText(n, COLUMN_DESCRIPTION, description);
						}
						break;
					default:
						{
							wchar_t fourcc[5] = {0};
							MakeStringFromFOURCC(fourcc, stream.stream_header->stream_type);
							n = list_view.AppendItem(fourcc, 0);
						}
						break;
					}
					if (stream.stream_name)
					{
						//const char *name = (const char *) (((const uint8_t *)stream.stream_name) + 4);
						// TODO: need AutoWideN before this is safe
						//	list_view.SetItemText(n, COLUMN_STREAM_NAME, AutoWide(name, CP_UTF8));
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
	nsavi::Metadata *metadata;
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
		context->active_tab = WASABI_API_CREATEDIALOGPARAMW(IDD_TRACKS, hwndDlg, InfoDialog_Metadata, (LPARAM)context->metadata);
		break;
	case 1:
		context->active_tab = WASABI_API_CREATEDIALOGPARAMW(IDD_TRACKS, hwndDlg, InfoDialog_Tracks, (LPARAM)context->metadata);
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
			context->metadata = (nsavi::Metadata *)lParam;
			context->active_tab = 0;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA, (LPARAM)context);
			TCITEMW tie = {0};
			tie.mask = TCIF_TEXT;
			tie.pszText = WASABI_API_LNGSTRINGW(IDS_TAB_METADATA);
			SendMessageW(hwndTab, TCM_INSERTITEMW, 0, (LPARAM)&tie);
			tie.pszText = WASABI_API_LNGSTRINGW(IDS_TAB_TRACKS);
			SendMessageW(hwndTab, TCM_INSERTITEMW, 1, (LPARAM)&tie);
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