#include <windows.h>

// LGIVEN Mods 4-10-05
#include "main.h"
#include <wmsdk.h>
#include "../nu/AutoWide.h"
#include "../nu/ns_wc.h"
#include "../Agave/Language/api_language.h"
#include <MMSystem.h>
#include <assert.h>

#define MAX_PASSES 1 // limited to 1pass encoding until we work out some code for 2pass encoding

// LGIVEN Mods 4-10-05
void readconfig(char *configfile, configtype *cfg)
{
	cfg->config_bitrate = 0;
	cfg->config_bitsSample = 0;
	cfg->config_nch = 0;
	cfg->config_samplesSec = 0;
	cfg->config_encoder = 0;
	cfg->config_vbr = 0;
	cfg->config_passes = 1;
	if (configfile)
	{
		GetPrivateProfileStructA("audio_wma", "conf", cfg, sizeof(configtype), configfile);
	}
}

void writeconfig(char *configfile, configtype *cfg)
{
	if (configfile)
	{
		WritePrivateProfileStructA("audio_wma", "conf", cfg, sizeof(configtype), configfile);
	}
}

// New global table for channels,samplerates and bitrates
static EncoderType* encs = NULL;     // Pointer to the TABLE with all config data
// Globals store current selections from Config Dialog
// Number of encoders
static int encNumbs = 0;    // Total number of encoders installed WMA

// New routine to read all config info from WMA encoder and load tables

static BOOL loadWMATables()
{
	IWMProfileManager *profileManager;
	IWMProfileManager2 *profileManager2;
	IWMCodecInfo3 *codecInfo;
	WAVEFORMATEX *pwave;
	HRESULT hr;
	int legalFormats = 0;

	WMCreateProfileManager(&profileManager);
	profileManager->QueryInterface(&profileManager2);
	profileManager2->SetSystemProfileVersion(WMT_VER_9_0);

	profileManager->QueryInterface(&codecInfo);
	// Get the number of AUDIO Codecs
	DWORD numCodecs = 0;
	codecInfo->GetCodecInfoCount(WMMEDIATYPE_Audio, &numCodecs);
	// If there are no encoders, just return
	if (numCodecs == 0)
	{
		return false;
	}
	// Allocate structs for codecs and zero them all
	encs = (EncoderType *) calloc(numCodecs * 4, sizeof(struct EncoderType));
	if (encs != NULL)
	{
		encNumbs = numCodecs * 4;
	}
	else
	{
		wchar_t titleStr[32] = {0};
		MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_CANNOT_ALLOCATE_MEM),
				    WASABI_API_LNGSTRINGW_BUF(IDS_WMA_ENCODER_ERROR,titleStr,32), MB_OK);
		return false;
	}
	// Now cycle through the codecs
	EncoderType* encp = encs;
	for (BOOL isVBR = 0;isVBR != 2;isVBR++)
		for (DWORD numPasses = 1;numPasses <= MAX_PASSES;numPasses++)
			for (DWORD i = 0;i != numCodecs;i++)
			{
				wchar_t codecName[5000] = {0};
				DWORD codecNameSize = 5000;

				codecInfo->SetCodecEnumerationSetting(WMMEDIATYPE_Audio, i, g_wszVBREnabled, WMT_TYPE_BOOL, (BYTE *)&isVBR, sizeof(BOOL));
				codecInfo->SetCodecEnumerationSetting(WMMEDIATYPE_Audio, i, g_wszNumPasses, WMT_TYPE_DWORD, (BYTE *)&numPasses, sizeof(DWORD));
				codecInfo->GetCodecName(WMMEDIATYPE_Audio, i, codecName, &codecNameSize);
				// Get the number of formats for this codec
				DWORD formatCount = 0;
				hr = codecInfo->GetCodecFormatCount( WMMEDIATYPE_Audio, i, &formatCount );
				if (FAILED(hr))
				{
					continue;
				}
				else if (formatCount == 0)
				{
					continue;
				}
				else
				{
					// Fill the EncoderType struct and allocate structs for format info
					// First allocate the space for all the formatType structs
					encp->formats = (formatType *) malloc(formatCount * sizeof(struct formatType));
					if (encp->formats == NULL)
					{
						wchar_t titleStr[32] = {0};
						MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_CANNOT_ALLOCATE_MEM),
								    WASABI_API_LNGSTRINGW_BUF(IDS_WMA_ENCODER_ERROR,titleStr,32), MB_OK);
						return false;
					}
					// Now fill the EncoderType struct with name and info
					encp->encoderName = _wcsdup(codecName);
					encp->numFormats = formatCount;
					encp->offset = i;
					encp->vbr = isVBR;
					encp->numPasses = numPasses;
				}
				// Now cycle through the formats for this codec
				legalFormats = 0;
				formatType *fmts = encp->formats;
				for (DWORD f = 0;f != formatCount;f++)
				{
					wchar_t fDesc[5000] = {0};
					DWORD size = 5000;
					// Get the config info for this encoding format in string format
					IWMStreamConfig *streamConfig;
					codecInfo->GetCodecFormatDesc(WMMEDIATYPE_Audio, i, f, &streamConfig, fDesc, &size);
					// Now get the config info
					IWMMediaProps *props;
					streamConfig->QueryInterface(&props);
					// Get the bitrate
					//DWORD bitRate;
					//streamConfig->GetBitrate(&bitRate);
					// Get the Media Encoder type
					DWORD mediaTypeSize;
					props->GetMediaType(0, &mediaTypeSize);
					WM_MEDIA_TYPE *mediaType = (WM_MEDIA_TYPE *)new char[mediaTypeSize];
					props->GetMediaType(mediaType, &mediaTypeSize);
					// Go get the WAVEFORMATEX Struct from the
					if (mediaType->cbFormat >= sizeof(WAVEFORMATEX))
					{
						pwave = (WAVEFORMATEX*)mediaType->pbFormat;
						if (pwave != NULL)
						{
							// Check to see if this is an A/V codec format
							// If so, do not save it

							/*
							if ((pwave->nAvgBytesPerSec / pwave->nBlockAlign) ==
							    ((pwave->nAvgBytesPerSec >= 3995) ? 5 : 3))
							{
								delete(mediaType);
								props->Release();
								streamConfig->Release();
								continue;
							}*/

							// old way of checking
							if ((wcsstr(fDesc, L"A/V")) != NULL)
							{
								delete[] (mediaType);
								props->Release();
								streamConfig->Release();
								continue;
							}
							// Load the format name
							fmts->formatName = _wcsdup(fDesc);
							// Load all the format values and the offset
							if ((pwave->nAvgBytesPerSec & 0x7FFFFF00) == 0x7FFFFF00)
							{
								fmts->bitrate = (pwave->nAvgBytesPerSec & 0x000000FF);
								fmts->vbr = 1;
							}
							else
							{
								fmts->bitrate = (pwave->nAvgBytesPerSec * 8);
								fmts->vbr = 0;
							}
							fmts->bitsSample = pwave->wBitsPerSample;
							fmts->nChannels = pwave->nChannels;
							fmts->samplesSec = pwave->nSamplesPerSec;
							fmts->offset = f;
						}
						else
						{
							wchar_t titleStr[32] = {0};
							MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_CANNOT_GET_STRUCTURE),
									    WASABI_API_LNGSTRINGW_BUF(IDS_WMA_ENCODER_ERROR,titleStr,32), MB_OK);
							return false;
						}
					}
					else
					{
						wchar_t titleStr[32] = {0};
						MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_CANNOT_GET_ENCODER4_INFO),
									WASABI_API_LNGSTRINGW_BUF(IDS_WMA_ENCODER_ERROR,titleStr,32), MB_OK);
						return false;
					}
					// Set the media type value in the EncoderType struct on first legal format
					if (f == 0)
					{
						encp->mediaType = mediaType->subtype;
					}
					// Now point to the next table block and inc the legal formats count
					fmts++;
					legalFormats++;
					// And release the props and streams structs
					delete[] (mediaType);
					props->Release();
					streamConfig->Release();

				}
				// If there are no legal formats for this codec then skip it
				if (legalFormats == 0)
				{
					delete[] encp->encoderName;
					encp->encoderName = NULL;
					encp->numFormats = legalFormats;
					encp->offset = 0;
				}
				// Else load number of legal formats and save it
				else
				{
					encp->numFormats = legalFormats;
					encp++;
				}
			}
	return true;
}

static int FindFormatNumber(formatType *formats, int numFormats, configtype *cfg)
{
	for (int i = 0;i < numFormats;i++, formats++)
	{
		if (formats->bitrate == cfg->config_bitrate
		        && formats->bitsSample == cfg->config_bitsSample
		        && formats->nChannels == cfg->config_nch
		        && formats->samplesSec == cfg->config_samplesSec)
			return formats->offset;
	}

	return 0;
}

static VOID dumpProfile(char *configfile, BOOL isVBR, DWORD numPasses, int encNumb, int fmtNumb)
{
	// Create a Profile and dump it
	IWMProfileManager *pmgr = NULL;
	IWMProfile *prof = NULL;
	IWMStreamConfig *sconf = NULL;
	IWMCodecInfo3 *cinfo = NULL;
	DWORD ssize;
	wchar_t errorTitle[128] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_WMA_CONFIG_FILE_ERROR,errorTitle,128);

	HRESULT hr = WMCreateProfileManager(&pmgr);
	if (!FAILED(hr))
	{
		hr = pmgr->CreateEmptyProfile(WMT_VER_9_0, &prof);
		if (!FAILED(hr))
		{
			hr = pmgr->QueryInterface(&cinfo);
			if (!FAILED(hr))
			{
				cinfo->SetCodecEnumerationSetting(WMMEDIATYPE_Audio, encNumb, g_wszVBREnabled, WMT_TYPE_BOOL, (BYTE *)&isVBR, sizeof(BOOL));
				cinfo->SetCodecEnumerationSetting(WMMEDIATYPE_Audio, encNumb, g_wszNumPasses, WMT_TYPE_DWORD, (BYTE *)&numPasses, sizeof(DWORD));
				cinfo->GetCodecFormat(WMMEDIATYPE_Audio, encNumb, fmtNumb, &sconf);
				sconf->SetConnectionName(L"enc_wma");
				sconf->SetStreamName(L"enc_wma");
				sconf->SetStreamNumber(1);
				hr = prof->AddStream(sconf);
				if (!FAILED(hr))
				{
					hr = pmgr->SaveProfile(prof, NULL, &ssize);
					if (!FAILED(hr))
					{
						WCHAR* pstring = new WCHAR[ssize];
						if (pstring != NULL)
						{
							hr = pmgr->SaveProfile(prof, pstring, &ssize);
							if (!FAILED(hr))
							{
								wchar_t cstring[4000] = {0};
								wcsncpy(cstring, pstring, 4000 - 1);
								WritePrivateProfileStructW(L"audio_wma", L"profile", cstring, sizeof(cstring) / sizeof(*cstring), AutoWide(configfile));
							}
							else{ MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_SAVE_PROFILE_READ_ERROR), errorTitle, MB_OK); }
						}
						else{ MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_MEM_ALLOCATION_ERROR), errorTitle, MB_OK); }
					}
					else{ MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_PROFILE_SAVE_SIZE_ERROR), errorTitle, MB_OK); }
				}
				else{ MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_CANNOT_READ_AUDIO_STREAM), errorTitle, MB_OK); }
			}
			else{ MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_CANNOT_GET_CODEC_INFO), errorTitle, MB_OK); }
		}
		else{ MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_CANNOT_CREATE_A_PROFILE), errorTitle, MB_OK); }
	}
	else{ MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_CANNOT_CREATE_PROFILE_MANAGER), errorTitle, MB_OK); }
	pmgr->Release();
	prof->Release();
	sconf->Release();
	cinfo->Release();
}

static bool Has(HWND hwndDlg, int item, int data)
{
	int numChoices = SendDlgItemMessage(hwndDlg, item, CB_GETCOUNT, 0, 0);
	for (int i = 0;i < numChoices;i++)
	{
		if (SendDlgItemMessage(hwndDlg, item, CB_GETITEMDATA, i, 0) == data)
			return true;
	}
	return false;
}

static int EncodeSampleFormat(int bps, int numChannels, int sampleRate)
{
	// 20 bits sampleRate
	assert((sampleRate & 0xFFFFF) == sampleRate);
	// 6 bits numChannels
	assert((numChannels & 0x3F) == numChannels);
	// 6 bits bps
	assert((bps & 0x3F) == bps);

	return (sampleRate << 12) | (numChannels << 6) | (bps);
}

static void DecodeSampleFormat(int data, int &bps, int &numChannels, int &sampleRate)
{
	bps = data & 0x3F;
	data >>= 6;
	numChannels = data & 0x3F;
	data >>= 6;
	sampleRate = data;
}

static int EncodeVBR(BOOL isVBR, DWORD numPasses)
{
	// 1 bits VBR
	assert((isVBR & 0x1) == isVBR);
	// 15 bits numPasses
	assert((numPasses & 0x7FFF) == numPasses);

	return (isVBR << 15) | (numPasses);
}

static void DecodeVBR(int data, BOOL &isVBR, DWORD &numPasses)
{
	numPasses = data & 0x7FFF;
	data >>= 15;
	isVBR = data & 0x1;
}

static void AutoSelect(HWND hwndDlg, int dlgItem)
{
	if (SendDlgItemMessage(hwndDlg, dlgItem, CB_GETCURSEL, 0, 0) == CB_ERR)
		SendDlgItemMessage(hwndDlg, dlgItem, CB_SETCURSEL, 0, 0);
}

static EncoderType *FindEncoder(int encoderNumber, BOOL isVBR, DWORD numPasses)
{
	EncoderType* enc = encs;
	for (int i = 0;i < encNumbs;i++, enc++)
	{
		if (enc->encoderName == NULL)
			return 0; //WTF?
		if (enc->offset == encoderNumber && enc->vbr == isVBR && enc->numPasses == numPasses)
			return enc;
	}
	return 0; //WTF?
}

#define MASK_ENCODER 0x1
#define MASK_VBR 0x2
#define MASK_SAMPLE_FORMAT 0x4
#define MASK_BITRATE 0x8
#define MASK_ALL 0xF

static void ResetConfig(HWND hwndDlg, EncoderType *encs, configtype *cfg, int mask)
{
	wchar_t buf1[100] = {0};
	EncoderType* enc = encs;

	if (mask & MASK_ENCODER)	SendDlgItemMessage(hwndDlg, IDC_ENCODER, CB_RESETCONTENT, 0, 0);
	if (mask & MASK_SAMPLE_FORMAT)	SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_RESETCONTENT, 0, 0);
	if (mask & MASK_BITRATE)	SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_RESETCONTENT, 0, 0);
	if (mask & MASK_VBR)	SendDlgItemMessage(hwndDlg, IDC_VBR, CB_RESETCONTENT, 0, 0);

	// reset encoders
	int thisVBR = EncodeVBR(cfg->config_vbr, cfg->config_passes);
	for (int i = 0;i < encNumbs;i++, enc++)
	{
		if (enc->encoderName == NULL)
			break;
		else if ((mask & MASK_ENCODER) && !Has(hwndDlg, IDC_ENCODER, enc->offset))
		{
			int newpos = SendDlgItemMessage(hwndDlg, IDC_ENCODER, CB_ADDSTRING, 0, (LPARAM)enc->encoderName);
			SendDlgItemMessage(hwndDlg, IDC_ENCODER, CB_SETITEMDATA, newpos, enc->offset);

			if (cfg->config_encoder == enc->offset)
			{
				SendDlgItemMessage(hwndDlg, IDC_ENCODER, CB_SETCURSEL, newpos, 0);
			}
		}
		int data = EncodeVBR(enc->vbr, enc->numPasses);
		if ((mask & MASK_VBR) && cfg->config_encoder == enc->offset && !Has(hwndDlg, IDC_VBR, data))
		{
			int newpos = CB_ERR;
			if (enc->vbr == FALSE && enc->numPasses == 1)
				newpos = SendDlgItemMessageW(hwndDlg, IDC_VBR, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_CBR));
			else if (enc->vbr == FALSE && enc->numPasses == 2)
				newpos = SendDlgItemMessageW(hwndDlg, IDC_VBR, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_2_PASS_CBR));
			else if (enc->vbr == TRUE && enc->numPasses == 1)
				newpos = SendDlgItemMessageW(hwndDlg, IDC_VBR, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_VBR));
			else if (enc->vbr == TRUE && enc->numPasses == 2)
				newpos = SendDlgItemMessageW(hwndDlg, IDC_VBR, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_ABR));

			SendDlgItemMessage(hwndDlg, IDC_VBR, CB_SETITEMDATA, newpos, data);

			if (thisVBR == data)
				SendDlgItemMessage(hwndDlg, IDC_VBR, CB_SETCURSEL, newpos, 0);
		}
	}

	AutoSelect(hwndDlg, IDC_ENCODER);
	AutoSelect(hwndDlg, IDC_VBR);
	int pos = SendDlgItemMessage(hwndDlg, IDC_VBR, CB_GETCURSEL, 0, 0);
	int data = SendDlgItemMessage(hwndDlg, IDC_VBR, CB_GETITEMDATA, pos, 0);
	DecodeVBR(data, cfg->config_vbr, cfg->config_passes);

	pos = SendDlgItemMessage(hwndDlg, IDC_ENCODER, CB_GETCURSEL, 0, 0);
	data = SendDlgItemMessage(hwndDlg, IDC_ENCODER, CB_GETITEMDATA, pos, 0);
	cfg->config_encoder = data;

	// Now set up for dialog fill
	enc = FindEncoder(cfg->config_encoder, cfg->config_vbr, cfg->config_passes);

	// Fill the current values
	formatType *fmt = enc->formats;

	int thisSampleFormat = EncodeSampleFormat(cfg->config_bitsSample, cfg->config_nch, cfg->config_samplesSec);
	for (int i = 0;i < enc->numFormats;i++, fmt++)
	{
		int data = EncodeSampleFormat(fmt->bitsSample, fmt->nChannels, fmt->samplesSec);
		// Add channels to list
		if ((mask & MASK_SAMPLE_FORMAT) && !Has(hwndDlg, IDC_SAMPLE_FORMAT, data))
		{
			if (fmt->nChannels == 1)
				wsprintfW(buf1, WASABI_API_LNGSTRINGW(IDS_MONO_INFO), fmt->bitsSample, fmt->samplesSec);
			else if (fmt->nChannels == 2)
				wsprintfW(buf1, WASABI_API_LNGSTRINGW(IDS_STEREO_INFO), fmt->bitsSample, fmt->samplesSec);
			else
				wsprintfW(buf1, WASABI_API_LNGSTRINGW(IDS_CHANNELS_INFO), fmt->bitsSample, fmt->nChannels, fmt->samplesSec);

			int newpos;
			if (fmt->bitsSample)
				newpos = SendDlgItemMessageW(hwndDlg, IDC_SAMPLE_FORMAT, CB_ADDSTRING, 0, (LPARAM)buf1);
			else
				newpos = SendDlgItemMessageW(hwndDlg, IDC_SAMPLE_FORMAT, CB_ADDSTRING, 0, (LPARAM)buf1 + 8); // skip "0 bits, "

			SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_SETITEMDATA, newpos, data);
			// Now set current select for number of channels sample
			if (thisSampleFormat == data)
				SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_SETCURSEL, newpos, 0);
		}
	}

	if (SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_GETCURSEL, 0, 0) == CB_ERR)
	{
		int num = SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_GETCOUNT, 0, 0);
		int defaultSampleFormat = EncodeSampleFormat(16, 2, 44100);
		for (int i = 0;i < num;i++)
		{
			int data = SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_GETITEMDATA, i, 0);
			if (data == defaultSampleFormat)
				SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_SETCURSEL, i, 0);
		}
	}

	AutoSelect(hwndDlg, IDC_SAMPLE_FORMAT);
	pos = SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_GETCURSEL, 0, 0);
	data = SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_GETITEMDATA, pos, 0);
	DecodeSampleFormat(data, cfg->config_bitsSample, cfg->config_nch, cfg->config_samplesSec);

	thisSampleFormat = EncodeSampleFormat(cfg->config_bitsSample, cfg->config_nch, cfg->config_samplesSec);

	// Next Show the Bitrates
	fmt = enc->formats;
	for (int i = 0;i < enc->numFormats;i++, fmt++)
	{
		int data = EncodeSampleFormat(fmt->bitsSample, fmt->nChannels, fmt->samplesSec);
		if (thisSampleFormat == data)
		{
			if ((mask & MASK_BITRATE) && !Has(hwndDlg, IDC_BRATE, fmt->bitrate))
			{
				if (fmt->vbr)
					SetDlgItemTextW(hwndDlg, IDC_STATIC_BITRATE, WASABI_API_LNGSTRINGW(IDS_QUALITY));
				else
					SetDlgItemTextW(hwndDlg, IDC_STATIC_BITRATE, WASABI_API_LNGSTRINGW(IDS_BITRATE));

				wsprintfW(buf1, L"%d", fmt->bitrate);
				int newpos = SendDlgItemMessageW(hwndDlg, IDC_BRATE, CB_ADDSTRING, 0, (LPARAM)buf1);
				SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_SETITEMDATA, newpos, fmt->bitrate);
				// Set the current bit rate
				if (cfg->config_bitrate == fmt->bitrate)
				{
					SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_SETCURSEL, newpos, 0);
				}
			}
		}
	}

	if (SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_GETCURSEL, 0, 0) == CB_ERR)
	{
		int num = SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_GETCOUNT, 0, 0);

		for (int i = 0;i < num;i++)
		{
			int data = SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_GETITEMDATA, i, 0);
			if (data == 50 || (data / 1000 == 128))
				SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_SETCURSEL, i, 0);
		}
	}

	AutoSelect(hwndDlg, IDC_BRATE);

	pos = SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_GETCURSEL, 0, 0);
	data = SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_GETITEMDATA, pos, 0);
	cfg->config_bitrate = data;
}

BOOL CALLBACK ConfigProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	configwndrec *wc = NULL;
	if (uMsg == WM_INITDIALOG)
	{
		// LGIVEN Mod 4-10-05
#if defined(_WIN64)
		SetWindowLong(hwndDlg, GWLP_USERDATA, lParam);
#else
		SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
#endif
		if (lParam)
		{
			// Get the saved params
			wc = (configwndrec*)lParam;

			loadWMATables();
			ResetConfig(hwndDlg, encs , &wc->cfg, MASK_ALL);
		}
		return 1;
	}
	if (uMsg == WM_DESTROY)
	{
#if defined(_WIN64)
		wc = (configwndrec*)SetWindowLong(hwndDlg, GWLP_USERDATA, 0);
#else
		wc = (configwndrec*)SetWindowLong(hwndDlg, GWL_USERDATA, 0);
#endif
		if (wc)
		{
			EncoderType *encoder=FindEncoder(wc->cfg.config_encoder,wc->cfg.config_vbr, wc->cfg.config_passes);
			int formatNumber = FindFormatNumber(encoder->formats, encoder->numFormats, &wc->cfg);
			// Dump the profile in WMA format
			dumpProfile(wc->configfile, wc->cfg.config_vbr, wc->cfg.config_passes, wc->cfg.config_encoder, formatNumber);
			// Write it to config file
			writeconfig(wc->configfile, &wc->cfg);
			free(wc->configfile);
			free(wc);
		}
		return 0;
	}
	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
		case IDC_VBR:
			if ((HIWORD(wParam) == CBN_SELCHANGE))
			{
#if defined(_WIN64)
				wc = (configwndrec*)GetWindowLong(hwndDlg, GWLP_USERDATA);
#else
				wc = (configwndrec*)GetWindowLong(hwndDlg, GWL_USERDATA);
#endif
				int pos = SendDlgItemMessage(hwndDlg, IDC_VBR, CB_GETCURSEL, 0, 0);
				int data = SendDlgItemMessage(hwndDlg, IDC_VBR, CB_GETITEMDATA, pos, 0);
				DecodeVBR(data, wc->cfg.config_vbr, wc->cfg.config_passes);
				ResetConfig(hwndDlg, encs, &wc->cfg, MASK_BITRATE | MASK_SAMPLE_FORMAT);
			}
			break;

		case IDC_SAMPLE_FORMAT:
			if ((HIWORD(wParam) == CBN_SELCHANGE))
			{
#if defined(_WIN64)
				wc = (configwndrec*)GetWindowLong(hwndDlg, GWLP_USERDATA);
#else
				wc = (configwndrec*)GetWindowLong(hwndDlg, GWL_USERDATA);
#endif
				int pos = SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_GETCURSEL, 0, 0);
				int data = SendDlgItemMessage(hwndDlg, IDC_SAMPLE_FORMAT, CB_GETITEMDATA, pos, 0);
				DecodeSampleFormat(data, wc->cfg.config_bitsSample, wc->cfg.config_nch, wc->cfg.config_samplesSec);
				ResetConfig(hwndDlg, encs, &wc->cfg, MASK_BITRATE);
			}
			break;

		case IDC_BRATE:
			if ((HIWORD(wParam) == CBN_SELCHANGE))
			{
#if defined(_WIN64)
				wc = (configwndrec*)GetWindowLong(hwndDlg, GWLP_USERDATA);
#else
				wc = (configwndrec*)GetWindowLong(hwndDlg, GWL_USERDATA);
#endif
				int pos = SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_GETCURSEL, 0, 0);
				int data = SendDlgItemMessage(hwndDlg, IDC_BRATE, CB_GETITEMDATA, pos, 0);
				wc->cfg.config_bitrate = data;
			}
			break;

		case IDC_ENCODER:
			if ((HIWORD(wParam) == CBN_SELCHANGE))
			{
#if defined(_WIN64)
				wc = (configwndrec*)GetWindowLong(hwndDlg, GWLP_USERDATA);
#else
				wc = (configwndrec*)GetWindowLong(hwndDlg, GWL_USERDATA);
#endif
				if (wc)
				{
					int pos = SendDlgItemMessage(hwndDlg, IDC_ENCODER, CB_GETCURSEL, 0, 0);
					int data = SendDlgItemMessage(hwndDlg, IDC_ENCODER, CB_GETITEMDATA, pos, 0);
					wc->cfg.config_encoder = data;
					ResetConfig(hwndDlg, encs, &wc->cfg, MASK_VBR | MASK_SAMPLE_FORMAT | MASK_BITRATE);
				}
			}
			break;

		}
	}
	return 0;
}