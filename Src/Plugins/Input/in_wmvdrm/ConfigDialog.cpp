#include "Main.h"
#include "resource.h"
#include "../nu/ListView.h"
#include "FileTypes.h"
#include "AutoChar.h"

W_ListView typeList;
FileTypes::TypeList types;
#define CFGSET(hwnd, code, boolval) CheckDlgButton(hwnd, code, ((boolval)?BST_CHECKED:BST_UNCHECKED))

struct SpeakerSetup
{
	int description;
	DWORD value;
};
SpeakerSetup speakerList[] =
{
	{IDS_STEREO,DSSPEAKER_STEREO},
	{IDS_QUADROPHONIC,DSSPEAKER_QUAD},
	{IDS_SURROUND,DSSPEAKER_SURROUND},
	{IDS_5_1,DSSPEAKER_5POINT1},
	{IDS_7_1,DSSPEAKER_7POINT1},
};

void FillFileTypes()
{
	typeList.Clear();

	long attrCount = types.size();
	int pos=0;
	for (long i = 0;i < attrCount;i++)
	{
		typeList.InsertItem(pos, types[i].wtype, 0);
		typeList.SetItemText(pos, 1, types[i].description);
		pos++;
	}	
}

void ResetTypes()
{
	{
		Nullsoft::Utility::AutoLock lock (fileTypes.typeGuard);
		types=fileTypes.types;
	}
		FillFileTypes();
}

void Preferences_Populate(HWND hwndDlg)
{
	CFGSET(hwndDlg, IDC_HTTPMETA, config_http_metadata);

/*	CFGSET(hwndDlg, IDC_SILENT, !config_no_silent);
	CFGSET(hwndDlg, IDC_UNTRUSTED, config_untrusted_ok);
*/
	CFGSET(hwndDlg, IDC_EXTRA_ASX, config_extra_asx_extensions);

	SetDlgItemInt(hwndDlg,IDC_BUFFER_TIME, config_buffer_time, FALSE/*unsigned*/);

	SendMessage(GetDlgItem(hwndDlg, IDC_AUDIO_SPEAKER_COUNT), CB_RESETCONTENT, 0, 0);
	for (int i = 0;i < sizeof(speakerList)/sizeof(speakerList[0]) ;i++)
	{
		SendMessage(GetDlgItem(hwndDlg, IDC_AUDIO_SPEAKER_COUNT), CB_ADDSTRING, 0, (LPARAM) WASABI_API_LNGSTRINGW(speakerList[i].description));
		if (speakerList[i].value == config_audio_num_channels)
			SendMessage(GetDlgItem(hwndDlg, IDC_AUDIO_SPEAKER_COUNT), CB_SETCURSEL, i, 0);
	}
	
	ResetTypes();
}
void Preferences_Init(HWND hwndDlg)
{
	typeList.setwnd(GetDlgItem(hwndDlg, IDC_TYPELIST));
	typeList.AddCol(WASABI_API_LNGSTRINGW(IDS_EXT), 75);
	typeList.AddCol(WASABI_API_LNGSTRINGW(IDS_DESCRIPTION), 250);
	Preferences_Populate(hwndDlg);

	if(config_col1 == -1)
		typeList.AutoSizeColumn(0);
	else
		typeList.SetColumnWidth(0, config_col1);

	if(config_col2 == -1)
		typeList.AutoSizeColumn(1);
	else
		typeList.SetColumnWidth(1, config_col2);

	if (NULL != WASABI_API_APP)
		WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(typeList.getwnd(), TRUE);
}

void Preferences_TypeRemove(HWND hwndDlg)
{
	int next = -1;

	do
	{
		next = typeList.GetNextSelected(next);
		if (next != -1)
		{
			free(types[next].wtype);
			types[next].wtype=0;
		}
	} while (next != -1);

	for (FileTypes::TypeList::iterator itr = types.begin(); itr != types.end(); )
	{
		if (!itr->wtype || !itr->wtype[0])
			types.erase(itr);
		else
			itr++;
	}
}

void OnOk(HWND hwndDlg)
{
	config_http_metadata=IsDlgButtonChecked(hwndDlg, IDC_HTTPMETA)== BST_CHECKED;
//	config_no_silent=IsDlgButtonChecked(hwndDlg, IDC_SILENT) != BST_CHECKED;
//	config_untrusted_ok=IsDlgButtonChecked(hwndDlg, IDC_UNTRUSTED)== BST_CHECKED;
	config_extra_asx_extensions=IsDlgButtonChecked(hwndDlg, IDC_EXTRA_ASX)== BST_CHECKED;
	wchar_t temp[64] = {0};
	
		GetDlgItemText(hwndDlg,IDC_BUFFER_TIME, temp, 64);
		int newBufferTime= _wtoi(temp);
		if (newBufferTime<1000)
			newBufferTime=1000;
		if (newBufferTime>60000)
			newBufferTime=60000;
		config_buffer_time=newBufferTime;

		int numChannels=SendMessage(GetDlgItem(hwndDlg, IDC_AUDIO_SPEAKER_COUNT), CB_GETCURSEL, 0, 0);
	if (numChannels!=CB_ERR)
		config_audio_num_channels = speakerList[numChannels].value;

	fileTypes.SetTypes(types);
}

static INT_PTR CALLBACK AddTypeProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			CFGSET(hwndDlg, IDC_FILEEXTENSION, true);
			CFGSET(hwndDlg, IDC_TYPE_AUDIO, true);
			break;
				case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PROTOCOL:
			SetDlgItemText(hwndDlg, IDC_STATIC_TYPE, WASABI_API_LNGSTRINGW(IDS_PROTOCOL));
			break;
		case IDC_FILEEXTENSION:
			SetDlgItemText(hwndDlg, IDC_STATIC_TYPE, WASABI_API_LNGSTRINGW(IDS_EXTENSION));
			break;
		case IDOK:
			{
			wchar_t type[MAX_PATH] = {0}, description[1024] = {0};
			GetDlgItemText(hwndDlg,IDC_TYPE,type,MAX_PATH);
			GetDlgItemText(hwndDlg,IDC_DESCRIPTION,description,1024);
			bool isProtocol = IsDlgButtonChecked(hwndDlg, IDC_PROTOCOL)== BST_CHECKED;
			int avType = IsDlgButtonChecked(hwndDlg, IDC_TYPE_VIDEO)== BST_CHECKED;
			types.push_back(FileType(type, description, isProtocol, avType));
			EndDialog(hwndDlg, 0);
			}
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
		}
		break;
	
	}
	return FALSE;
}

void AddType(HWND hwndDlg)
{
	WASABI_API_DIALOGBOXW(IDD_ADDTYPE, hwndDlg, AddTypeProc);
}

static size_t editIndex=0;
static INT_PTR CALLBACK EditTypeProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetWindowText(hwndDlg, WASABI_API_LNGSTRINGW(IDS_EDIT_FILE_TYPE));
			CFGSET(hwndDlg, IDC_TYPE_AUDIO, (types[editIndex].avType==FileType::AUDIO));
			CFGSET(hwndDlg, IDC_TYPE_VIDEO, (types[editIndex].avType==FileType::VIDEO));
			CFGSET(hwndDlg, IDC_PROTOCOL, types[editIndex].isProtocol);
			CFGSET(hwndDlg, IDC_FILEEXTENSION, !types[editIndex].isProtocol);
			SetDlgItemText(hwndDlg,IDC_TYPE, types[editIndex].wtype);
			SetDlgItemText(hwndDlg,IDC_DESCRIPTION, types[editIndex].description);
			if (types[editIndex].isProtocol)
				SetDlgItemText(hwndDlg, IDC_STATIC_TYPE, WASABI_API_LNGSTRINGW(IDS_PROTOCOL));
			else
				SetDlgItemText(hwndDlg, IDC_STATIC_TYPE, WASABI_API_LNGSTRINGW(IDS_EXTENSION));
			break;
				case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PROTOCOL:
			SetDlgItemText(hwndDlg, IDC_STATIC_TYPE, WASABI_API_LNGSTRINGW(IDS_PROTOCOL));
			break;
		case IDC_FILEEXTENSION:
			SetDlgItemText(hwndDlg, IDC_STATIC_TYPE, WASABI_API_LNGSTRINGW(IDS_EXTENSION));
			break;
		case IDOK:
			{
			wchar_t type[MAX_PATH] = {0}, description[1024] = {0};
			GetDlgItemText(hwndDlg,IDC_TYPE,type,MAX_PATH);
			GetDlgItemText(hwndDlg,IDC_DESCRIPTION,description,1024);
			bool isProtocol = IsDlgButtonChecked(hwndDlg, IDC_PROTOCOL)== BST_CHECKED;

			types[editIndex].avType = (IsDlgButtonChecked(hwndDlg, IDC_TYPE_VIDEO)== BST_CHECKED);
			if(types[editIndex].wtype)free(types[editIndex].wtype);
			types[editIndex].wtype = _wcsdup(type);

			types[editIndex].isProtocol = isProtocol;
			if(types[editIndex].description)free(types[editIndex].description);
			types[editIndex].description = _wcsdup(description);
			EndDialog(hwndDlg, 0);
			}
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
		}
		break;
	
	}
	return FALSE;
}


void EditType(HWND hwndDlg)
{
	editIndex=-1;
	do
	{
		editIndex=typeList.GetNextSelected(editIndex);
		if (editIndex != -1)
		{
			WASABI_API_DIALOGBOXW(IDD_ADDTYPE, hwndDlg, EditTypeProc);
		}
		else
			return;	
	
	} while (true);
}


void Advanced_Init(HWND hwndDlg)
{
	CFGSET(hwndDlg, IDC_AUDIO_THREAD, config_audio_dedicated_thread);
	CFGSET(hwndDlg, IDC_AUDIO_EARLY, config_audio_early);
	CFGSET(hwndDlg, IDC_AUDIO_OUTOFORDER, config_audio_outoforder);
	CFGSET(hwndDlg, IDC_AUDIO_DROP, config_video_catchup);
	
	CFGSET(hwndDlg, IDC_VIDEO_THREAD, config_video_dedicated_thread);
	CFGSET(hwndDlg, IDC_VIDEO_EARLY, config_video_early);
	CFGSET(hwndDlg, IDC_VIDEO_OUTOFORDER, config_video_outoforder);
	CFGSET(hwndDlg, IDC_VIDEO_NOTIFY, config_video_notifylate);

	CFGSET(hwndDlg, IDC_REALTIME, !config_clock);
	CFGSET(hwndDlg, IDC_LOWMEMORY, config_lowmemory);

	SetDlgItemInt(hwndDlg,IDC_AUDIO_CACHE_FRAMES, config_audio_cache_frames, TRUE/*signed*/);
	SetDlgItemInt(hwndDlg,IDC_VIDEO_CACHE_FRAMES, config_video_cache_frames, TRUE);
	SetDlgItemInt(hwndDlg,IDC_VIDEO_DROP_THRESHOLD, config_video_drop_threshold, TRUE);
	SetDlgItemInt(hwndDlg,IDC_VIDEO_JITTER, config_video_jitter, TRUE);
	SetDlgItemInt(hwndDlg,IDC_AUDIO_EARLYPAD, config_audio_early_pad, TRUE);
	SetDlgItemInt(hwndDlg,IDC_VIDEO_EARLYPAD, config_video_early_pad,TRUE);

}

void Advanced_OnOK(HWND hwndDlg)
{
	config_audio_dedicated_thread=IsDlgButtonChecked(hwndDlg, IDC_AUDIO_THREAD)== BST_CHECKED;
	config_audio_early=IsDlgButtonChecked(hwndDlg, IDC_AUDIO_EARLY)== BST_CHECKED;
	config_audio_outoforder=IsDlgButtonChecked(hwndDlg, IDC_AUDIO_OUTOFORDER)== BST_CHECKED;
	config_video_catchup=IsDlgButtonChecked(hwndDlg, IDC_AUDIO_DROP)== BST_CHECKED;

	config_video_dedicated_thread=IsDlgButtonChecked(hwndDlg, IDC_VIDEO_THREAD)== BST_CHECKED;
	config_video_early=IsDlgButtonChecked(hwndDlg, IDC_VIDEO_EARLY)== BST_CHECKED;
	config_video_outoforder=IsDlgButtonChecked(hwndDlg, IDC_VIDEO_OUTOFORDER)== BST_CHECKED;
	config_video_notifylate=IsDlgButtonChecked(hwndDlg, IDC_VIDEO_NOTIFY)== BST_CHECKED;

	config_clock=IsDlgButtonChecked(hwndDlg, IDC_REALTIME)!= BST_CHECKED;
	config_lowmemory=IsDlgButtonChecked(hwndDlg, IDC_LOWMEMORY)== BST_CHECKED;

	wchar_t temp[64] = {0};
	
	GetDlgItemText(hwndDlg,IDC_AUDIO_CACHE_FRAMES, temp, 64);
	config_audio_cache_frames = _wtoi(temp);
	
	GetDlgItemText(hwndDlg,IDC_VIDEO_CACHE_FRAMES, temp, 64);
	config_video_cache_frames= _wtoi(temp);
	
	GetDlgItemText(hwndDlg,IDC_VIDEO_DROP_THRESHOLD, temp, 64);
	config_video_drop_threshold= _wtoi(temp);
	
	GetDlgItemText(hwndDlg,IDC_VIDEO_JITTER, temp, 64);
	config_video_jitter= _wtoi(temp);
	
	GetDlgItemText(hwndDlg,IDC_AUDIO_EARLYPAD, temp, 64);
	config_audio_early_pad= _wtoi(temp);
	
	GetDlgItemText(hwndDlg,IDC_VIDEO_EARLYPAD, temp, 64);
	config_video_early_pad= _wtoi(temp);
}

static INT_PTR CALLBACK AdvancedDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		Advanced_Init(hwndDlg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			Advanced_OnOK(hwndDlg);
			EndDialog(hwndDlg, 0);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
	
		}
		break;
	}

	return 0;
}

void Advanced(HWND hwndDlg)
{
	WASABI_API_DIALOGBOXW(IDD_ADVANCED, hwndDlg, AdvancedDialogProc);
}

void Preferences_Default(HWND hwndDlg)
{
	if (config_no_video)
	{
		// TODO: ask the user if they want to enable video support
	}
	DefaultConfig();
	fileTypes.LoadDefaults();
	Preferences_Populate(hwndDlg);
}

INT_PTR CALLBACK PreferencesDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			Preferences_Init(hwndDlg);
			break;
		case WM_DESTROY:
			config_col1 = typeList.GetColumnWidth(0);
			config_col2 = typeList.GetColumnWidth(1);

			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(typeList.getwnd(), FALSE);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_ADVANCED:
				Advanced(hwndDlg);
				break;
			case IDC_DEFAULTTYPE:
				Preferences_Default(hwndDlg);
				break;
			case IDC_ADDTYPE:
				AddType(hwndDlg);
				FillFileTypes();
				break;
			case IDC_EDITTYPE:
				EditType(hwndDlg);
				if(typeList.GetNextSelected(editIndex)!=-1)
					FillFileTypes();
				break;
			case IDC_REMOVETYPE:
				Preferences_TypeRemove(hwndDlg);
				FillFileTypes();
				break;
			case IDOK:
				OnOk(hwndDlg);
			case IDCANCEL:
				EndDialog(hwndDlg, 0);
				break;
			}
			break;
	}
	return 0;
}