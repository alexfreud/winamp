#include "preferences.h"
#include "resource.h"
#include "../nu/ComboBox.h"
#include "../nu/Slider.h"
#include "../Agave/Language/api_language.h"
#include <strsafe.h>
/* note to maintainers:
the combobox code assumes that the options are in numerical order starting from 0
if this assumption ever changes, you'll need to map AAC_MODE_* and AAC_PROFILE_* to/from indices 
there are some asserts to test this (which will only test in debug mode of course)
*/
const int bitrate_slider_precision = 4;

static void SetSliderBitrate(HWND hwnd, unsigned int bitrate)
{
	Slider preset_slider(hwnd, IDC_SLIDER_PRESET);
	int low = (unsigned int)SendMessage(GetDlgItem(hwnd,IDC_SLIDER_PRESET), TBM_GETRANGEMIN, 0, 0);
	int position = (bitrate - low)/bitrate_slider_precision + low;
	preset_slider.SetPosition(position, Slider::REDRAW);
}

unsigned int GetSliderBitrate(HWND hwnd)
{
	int low = (unsigned int)SendMessage(GetDlgItem(hwnd,IDC_SLIDER_PRESET), TBM_GETRANGEMIN, 0, 0);
	int position = SendMessage(GetDlgItem(hwnd,IDC_SLIDER_PRESET),TBM_GETPOS,0,0);
	unsigned int bitrate = (position - low)*bitrate_slider_precision + low;
	return bitrate;
}

void UpdateInfo(HWND hwnd, const AACConfiguration *config)
{
	wchar_t temp[128] = {0};

	switch(AACConfig_GetAOT(config))
	{
	case AUD_OBJ_TYP_LC:
		SetDlgItemText(hwnd, IDC_INFO_MODE, L"MPEG-4 AAC LC");
		break;
	case AUD_OBJ_TYP_HEAAC:
		SetDlgItemText(hwnd, IDC_INFO_MODE, L"MPEG-4 HE-AAC");
		break;
	case AUD_OBJ_TYP_PS:
		SetDlgItemText(hwnd, IDC_INFO_MODE, L"MPEG-4 HE-AACv2");
		break;
	}
	if (config->mode == AAC_MODE_VBR)
	{
		StringCbPrintfW(temp, sizeof(temp), WASABI_API_LNGSTRINGW(IDS_VBR_PRESET), config->preset, AACConfig_GetBitrate(config, 2 /* TODO! */)/1000);
		SetDlgItemText(hwnd, IDC_INFO_BITRATE, temp);
	}
	else
	{
		StringCbPrintfW(temp, sizeof(temp), WASABI_API_LNGSTRINGW(IDS_CBR_PRESET),  AACConfig_GetBitrate(config, 2 /* TODO! */)/1000);
		SetDlgItemText(hwnd, IDC_INFO_BITRATE, temp);
	}
}

/* call this to update the UI according to the configuration values */
void UpdateUI(HWND hwnd, AACConfigurationFile *config)
{
	wchar_t temp[128] = {0};
	ComboBox mode_list(hwnd, IDC_MODE);
	ComboBox profile_list(hwnd, IDC_PROFILE);

	config->changing = true;
	Slider preset_slider(hwnd, IDC_SLIDER_PRESET);
	if (config->config.mode == AAC_MODE_VBR)
	{
		preset_slider.SetRange(1, 6, Slider::REDRAW);
		preset_slider.SetTickFrequency(1);
		preset_slider.SetPosition(config->config.preset, Slider::REDRAW);
		SetDlgItemText(hwnd, IDC_STATIC_PRESET, WASABI_API_LNGSTRINGW(IDS_PRESET));
		StringCbPrintf(temp, sizeof(temp), L"%u", config->config.preset);
		SetDlgItemText(hwnd, IDC_EDIT_PRESET, temp);
		profile_list.SelectItem(AAC_PROFILE_AUTOMATIC);
		EnableWindow(profile_list, FALSE);
		ShowWindow(GetDlgItem(hwnd, IDC_KBPS), SW_HIDE);
	}
	else /* implied: if (config->config.mode == AAC_MODE_CBR) */
	{
		int low, high;
		AACConfig_GetBitrateRange(&config->config, &low, &high);
		low = ((low/1000+3)&~3); /* convert to kbps round up to nearest multiple of 4 */
		high = ((high/1000)&~3); /* convert to kbps round down to nearest multiple of 4 */

		int slider_high = low + (high - low)/bitrate_slider_precision;
		preset_slider.SetRange(low, slider_high, Slider::REDRAW);
		if (config->config.profile >= AAC_PROFILE_HE)
			preset_slider.SetTickFrequency(1);
		else
			preset_slider.SetTickFrequency(4);
		SetSliderBitrate(hwnd, config->config.bitrate);
		config->config.bitrate = GetSliderBitrate(hwnd);
		SetDlgItemText(hwnd, IDC_STATIC_PRESET, WASABI_API_LNGSTRINGW(IDS_BITRATE));
		StringCbPrintf(temp, sizeof(temp), L"%u", config->config.bitrate);
		SetDlgItemText(hwnd, IDC_EDIT_PRESET, temp);
		profile_list.SelectItem(config->config.profile);
		EnableWindow(profile_list, TRUE);
		ShowWindow(GetDlgItem(hwnd, IDC_KBPS), SW_SHOWNA);
	}
	config->changing = false;
}

int Preferences_GetEncoderVersion(char *version_string, size_t cch)
{
	char version[128] = {0};
	MPEG4ENC_GetVersionInfo(version, sizeof(version)/sizeof(*version));
	char *p = version;
	while (p && *p)
	{
		if (*p != '.' && (*p < '0' || *p > '9'))
		{
			*p = 0;
			break;
		}
		p++;
	}

	StringCchPrintfA(version_string, cch, WASABI_API_LNGSTRING(IDS_VERSION), version);

	return 0;
}