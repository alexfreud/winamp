/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description:
** Author:
** Created:
**/

#include "main.h"
#include "resource.h"
#include "../nsv/enc_if.h"
#include "../nu/threadname.h"
#include "../nu/AutoWideFn.h"
#include "../nu/AutoCharFn.h"
#include "DecodeFile.h"

extern DecodeFile *decodeFile;

static wchar_t DLL_Dir[MAX_PATH];

static intptr_t getEncoderFromFolder(const wchar_t *spec, int bps, int nch, int srate, int dstf, const wchar_t *curdir, int create, HMODULE *pmod, HWND hParent, converterEnumFmtStruct *enumCrap, char * inifile)
{
	WIN32_FIND_DATAW fd = {0};
	wchar_t buf[MAX_PATH*2 + 1] = {0};

	PathCombineW(buf, curdir, spec);

	if (pmod) *pmod = NULL;

	HANDLE h = FindFirstFileW(buf, &fd);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			PathCombineW(buf, curdir, fd.cFileName);

			HMODULE mod = LoadLibraryW(buf);
			if (mod)
			{
				// passes winamp's hwnd to the encoder (if supporting it)
				void (*swh)(HWND hwnd);
				*((void **)&swh) = GetProcAddress(mod, "SetWinampHWND");
				if (swh)
				{
					swh(hMainWindow);
				}

				if (enumCrap)
				{
					unsigned int (*gat)(int idx, char *desc);
					*((void **)&gat) = GetProcAddress(mod, "GetAudioTypes3");
					if (gat)
					{
						int i = 0;
						for (;;)
						{
							char desc[1024] = {0};
							unsigned int type = gat(i++, desc);
							if (!type) break;
							enumCrap->enumProc(enumCrap->user_data, desc, type);
						}
					}
				}
				else
				{
					void (*ExtAudio3)(HWND hwndParent, int *ex, int ex_len);
					*((void **)&ExtAudio3) = GetProcAddress(mod, "ExtAudio3");
					if (ExtAudio3) ExtAudio3(hMainWindow, NULL, 0);

					AudioCoder *ac = 0;
					AudioCoder *(*ca)(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile);
					*((void **)&ca) = GetProcAddress(mod, "CreateAudio3");

					if (create == 0)
					{
						HWND (*ca)(HWND hwndParent, HINSTANCE hinst, unsigned int outt, char *configfile);
						*((void**)&ca) = GetProcAddress(mod, "ConfigAudio3");
						if (ca)
						{
							HWND h = ca(hParent, mod, dstf, inifile?inifile:INI_FILEA);
							if (h)
							{
								*pmod = mod;
								return (intptr_t)h;
							}
						}
					}

					//if (ca && (ac=ca(nch,srate,bps,srct,outt,configfile))) return ac;
					if (create == 1 && ca && (ac = ca(nch, srate, bps, mmioFOURCC('P', 'C', 'M', ' '), (unsigned int *) & dstf, inifile?inifile:INI_FILEA))) //FUCKO: input format
					{
						*pmod = mod;
						return (intptr_t)ac;
					}
					if (create == 2) {
						unsigned int (*gat)(int idx, char *desc);
						*((void **)&gat) = GetProcAddress(mod, "GetAudioTypes3");
						if (gat)
						{
							int i = 0;
							for (;;)
							{
								char desc[1024] = {0};
								unsigned int type = gat(i++, desc);
								if (!type) break;
								if (type == dstf) {
									*pmod = mod;
									return 0;
								}
							}
						}
					}
				}
				FreeLibrary(mod);
			}
		}
		while (FindNextFileW(h, &fd));
		FindClose(h);
	}
	return 0;
}

static intptr_t getEncoder(int bps, int nch, int srate, int *destformat, int create, HMODULE *pmod, HWND parent, converterEnumFmtStruct *enumCrap = 0,char * inifile=0)
{
	HKEY hKey = NULL;

	if (!DLL_Dir[0] && RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion",
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD l = sizeof(DLL_Dir);
		DWORD t = 0;
		if (RegQueryValueExW(hKey, L"CommonFilesDir", NULL, &t, (LPBYTE)DLL_Dir, &l ) != ERROR_SUCCESS || t != REG_SZ)
			DLL_Dir[0] = 0;
		PathAppendW(DLL_Dir, L"NSV");
		RegCloseKey(hKey);
	}

	if (!DLL_Dir[0]) GetTempPathW(sizeof(DLL_Dir)/sizeof(*DLL_Dir), DLL_Dir);

	//look in plugins folder

	int ret;
	if (ret = getEncoderFromFolder(L"enc_*.dll", bps, nch, srate, destformat[0], PLUGINDIR, create, pmod, parent, enumCrap,inifile)) 
		return ret;

	if (GetPrivateProfileIntW(AutoWide(app_name), L"scannsv", 0, INI_FILE))
	{
		//look in common files folder
		if (ret = getEncoderFromFolder(L"nsv_coder_*.dll", bps, nch, srate, destformat[0], DLL_Dir, create, pmod, parent, enumCrap,inifile)) 
			return ret;
	}

	return 0;
}

static DWORD WINAPI convertThread(void *param)
{
	convertFileStruct *cfs = (convertFileStruct *)param;
	ifc_audiostream *decoder = cfs->decoder;
	HANDLE fh = cfs->file_handle;
	AudioCoder *ac = cfs->audio_coder;
	HMODULE mod = cfs->encoder_mod;
	int destformat = cfs->destformat[0];
	int bps = cfs->bps;
	int nch = cfs->channels;
	//int srate = cfs->sample_rate;

	size_t bytes_per_packet = nch*(bps/8);
	size_t ret = 0;
	SetThreadName((DWORD)-1, "Transcode");
	cfs->bytes_done = 0;
	cfs->bytes_out = 0;
	DWORD laststatpost = 0;
	do
	{
		int error=0;
		char buf[65536] = {0};
		size_t buf_size = sizeof(buf);
		buf_size -= (buf_size % bytes_per_packet); // don't read half a sample or only some of the channels!
		ret = decoder->ReadAudio(buf, buf_size, &cfs->killswitch, &error);

		if (destformat == mmioFOURCC('P', 'C', 'M', ' ') /* || destformat==mmioFOURCC('W','A','V',' ')*/)
		{
			//FUCKO: resample in desired format
			DWORD a = 0;
			if (ret > 0) WriteFile(fh, buf, (DWORD)ret, &a, NULL);
			cfs->bytes_out += a;
		}
		else
		{
			int framepos = 0;
			int avail = (int) ret;
			char *in = buf;
			char out[32768] = {0};

			// WM encoding needs to know that you're going to be done, before you stop calling Encode(...)
			if ( ret == 0 )
			{
				if (ac && mod)
				{
					void (*finish)(const char *filename, AudioCoder *coder);
					*((void **)&finish) = GetProcAddress(mod, "PrepareToFinish");
					if (finish)
					{
						finish(cfs->destfile, ac);
					}
				}
			}

			for (;;)
			{
				int in_used = 0;
				int v = ac->Encode(framepos++, in, avail, &in_used, out, sizeof(out));
				if (v > 0)
				{
					DWORD a = 0;
					WriteFile(fh, out, v, &a, NULL);
					cfs->bytes_out += v;
				}
				if (in_used > 0)
				{
					avail -= in_used;
					in += in_used;
				}
				if (!v && !in_used) break;
			}
		}
		cfs->bytes_done += (int)ret;

		if (GetTickCount() - laststatpost > 1000)
		{
			SendMessageW(cfs->callbackhwnd, WM_WA_IPC, (int)((double)cfs->bytes_done*100.0 / (double)cfs->bytes_total), IPC_CB_CONVERT_STATUS);
			laststatpost = GetTickCount();
		}
	}
	while (!cfs->killswitch && ret > 0);

	CloseHandle(fh);

	if (ac && mod)
	{
		void (*finish)(const char *filename, AudioCoder *coder);
		*((void **)&finish) = GetProcAddress(mod, "FinishAudio3");
		if (finish)
		{
			finish(cfs->destfile, ac);
		}
	}

	decodeFile->CloseAudio(decoder);

	if (!cfs->killswitch) PostMessageW(cfs->callbackhwnd, WM_WA_IPC, 0, IPC_CB_CONVERT_DONE);

	return 1;
}

static DWORD WINAPI convertThreadW(void *param)
{
	convertFileStructW *cfs = (convertFileStructW *)param;
	ifc_audiostream *decoder = (ifc_audiostream *)cfs->decoder;
	HANDLE fh = cfs->file_handle;
	AudioCoder *ac = cfs->audio_coder;
	HMODULE mod = cfs->encoder_mod;
	int destformat = cfs->destformat[0];
	int bps = cfs->bps;
	int nch = cfs->channels;
	//int srate = cfs->sample_rate;

	size_t bytes_per_packet = nch*(bps/8);
	size_t ret = 0;
	SetThreadName((DWORD)-1, "Transcode");
	cfs->bytes_done = 0;
	cfs->bytes_out = 0;
	DWORD laststatpost = 0;
	do
	{
		int error=0;
		char buf[65536] = {0};
		size_t buf_size = sizeof(buf);
		buf_size -= (buf_size % bytes_per_packet); // don't read half a sample or only some of the channels!
		ret = decoder->ReadAudio(buf, buf_size, &cfs->killswitch, &error);

		if (destformat == mmioFOURCC('P', 'C', 'M', ' ') /* || destformat==mmioFOURCC('W','A','V',' ')*/)
		{
			//FUCKO: resample in desired format
			DWORD a = 0;
			if (ret > 0) WriteFile(fh, buf, (DWORD)ret, &a, NULL);
			cfs->bytes_out += a;
		}
		else
		{
			int framepos = 0;
			int avail = (int) ret;
			char *in = buf;
			char out[32768] = {0};

			// WM encoding needs to know that you're going to be done, before you stop calling Encode(...)
			if ( ret == 0 )
			{
				if (ac && mod)
				{
					// try unicode first
					void (*finishW)(const wchar_t *filename, AudioCoder *coder);
					*((void **)&finishW) = GetProcAddress(mod, "PrepareToFinishW");
					if (finishW)
					{
						finishW(cfs->destfile, ac);
					}
					else // otherwise, pass it the 8.3 filename
					{
						void (*finish)(const char *filename, AudioCoder *coder);
						*((void **)&finish) = GetProcAddress(mod, "PrepareToFinish");
						if (finish)
						{
							finish(AutoCharFn(cfs->destfile), ac);
						}
					}
				}
			}

			for (;;)
			{
				int in_used = 0;
				int v = ac->Encode(framepos++, in, avail, &in_used, out, sizeof(out));
				if (v > 0)
				{
					DWORD a = 0;
					WriteFile(fh, out, v, &a, NULL);
					cfs->bytes_out += v;
				}
				if (in_used > 0)
				{
					avail -= in_used;
					in += in_used;
				}
				if (!v && !in_used) break;
			}
		}
		cfs->bytes_done += (int)ret;

		if (GetTickCount() - laststatpost > 1000)
		{
			SendMessageW(cfs->callbackhwnd, WM_WA_IPC, (int)((double)cfs->bytes_done*100.0 / (double)cfs->bytes_total), IPC_CB_CONVERT_STATUS);
			laststatpost = GetTickCount();
		}
	}
	while (!cfs->killswitch && ret > 0);

	CloseHandle(fh);

	if (ac && mod)
	{
		void (*finishW)(const wchar_t *filename, AudioCoder *coder);
		*((void **)&finishW) = GetProcAddress(mod, "FinishAudio3W");
		if (finishW)
		{
			finishW(cfs->destfile, ac);
		}
		else // otherwise, try the 8.3 filename
		{
			void (*finish)(const char *filename, AudioCoder *coder);
			*((void **)&finish) = GetProcAddress(mod, "FinishAudio3");
			if (finish)
			{
				finish(AutoCharFn(cfs->destfile), ac);
			}
		}
	}

	decodeFile->CloseAudio(decoder);

	if (!cfs->killswitch) PostMessageW(cfs->callbackhwnd, WM_WA_IPC, 0, IPC_CB_CONVERT_DONE);

	return 1;
}

// due to the language support, we can't just now return the string in cfs->error
// but instead have to have it in a static string so it can be accessed once we
// have returned without issues from later use of getString and it's buffer usage
static char errorStr[2048];
int convert_file(convertFileStruct *cfs)
{
	// clear the buffer on starting otherwise we may return an invalid error message
	//memset(&errorStr, 0, sizeof(errorStr));
	errorStr[0]=0;

	if (cfs->destfile && cfs->sourcefile && !_stricmp(cfs->destfile, cfs->sourcefile))
	{
		cfs->error = getString(IDS_CONV_SRC_EQUALS_DEST,errorStr,2048);
		return 0;
	}

	AudioParameters parameters;
	ifc_audiostream *decoder = decodeFile->OpenAudioBackground(AutoWideFn(cfs->sourcefile), &parameters);
	cfs->bytes_total= (int)(parameters.sizeBytes?parameters.sizeBytes:-1);

	if (!decoder)
	{
		switch(parameters.errorCode)
		{
			case API_DECODEFILE_UNSUPPORTED:
				cfs->error = getString(IDS_CONV_DECODER_MISSING,errorStr,2048);
				return 0;
			case API_DECODEFILE_NO_INTERFACE:
				cfs->error = getString(IDS_CONV_INPUT_PLUGIN_NOT_SUPPORTING,errorStr,2048);
				return 0;
			case API_DECODEFILE_NO_RIGHTS:
				cfs->error = getString(IDS_CONV_DRM_DECODE_FAIL,errorStr,2048);
				return 0;
			case API_DECODEFILE_FAIL_NO_WARN:
				return 0;
			default:
				cfs->error = getString(IDS_CONV_ERROR_OPEN_FILE,errorStr,2048);
				return 0;
		}
	}
	
	cfs->decoder=0;
	cfs->convert_thread=0;
	cfs->file_handle=0;
	cfs->audio_coder=0;
	cfs->encoder_mod=0;
	cfs->bps=0;
	cfs->channels=0;
	cfs->sample_rate=0;

	//find the encoding DLL
	if (cfs->destformat[0] != mmioFOURCC('P', 'C', 'M', ' '))
	{
		HMODULE mod = NULL;
		char * inifile = NULL;
		if(cfs->destformat[6] == mmioFOURCC('I','N','I',' ')) inifile = (char*)cfs->destformat[7];
		AudioCoder *ac = (AudioCoder *)getEncoder(parameters.bitsPerSample, parameters.channels, parameters.sampleRate, (int *) & cfs->destformat, 1, &mod, NULL,0, inifile);
		if (!ac)
		{
			decodeFile->CloseAudio(decoder);
			cfs->error = getString(IDS_CONV_ERROR_OPEN_ENCODER,errorStr,2048);
			return 0;
		}
		cfs->audio_coder = ac;
		cfs->encoder_mod = mod;
	}

	cfs->killswitch = 0;
	cfs->decoder = decoder;

	cfs->bps = parameters.bitsPerSample;
	cfs->channels = parameters.channels;
	cfs->sample_rate = parameters.sampleRate;

	//open destination file
	HANDLE fh = CreateFileA(cfs->destfile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( fh == INVALID_HANDLE_VALUE )
	{
		decodeFile->CloseAudio(decoder);
		delete cfs->audio_coder;
		cfs->audio_coder = 0;
		cfs->error = getString(IDS_CONV_ERROR_OPEN_DEST,errorStr,2048);
		return 0;
	}
	cfs->file_handle = fh;

	DWORD id = 0;
	cfs->convert_thread = CreateThread(NULL, 0, convertThread, cfs, 0, &id);

	return 1;
}

static wchar_t errorStrW[2048];
int convert_fileW(convertFileStructW *cfs)
{
	// clear the buffer on starting otherwise we may return an invalid error message
	memset(&errorStrW, 0, sizeof(errorStrW));

	if (cfs->destfile && cfs->sourcefile && !_wcsicmp(cfs->destfile, cfs->sourcefile))
	{
		cfs->error = getStringW(IDS_CONV_SRC_EQUALS_DEST,errorStrW,2048);
		return 0;
	}

	AudioParameters parameters;
	ifc_audiostream *decoder = decodeFile->OpenAudioBackground(cfs->sourcefile, &parameters);
	cfs->bytes_total= (int)(parameters.sizeBytes?parameters.sizeBytes:-1);

	if (!decoder)
	{
		switch(parameters.errorCode)
		{
			case API_DECODEFILE_UNSUPPORTED:
				cfs->error = getStringW(IDS_CONV_DECODER_MISSING,errorStrW,2048);
				return 0;
			case API_DECODEFILE_NO_INTERFACE:
				cfs->error = getStringW(IDS_CONV_INPUT_PLUGIN_NOT_SUPPORTING,errorStrW,2048);
				return 0;
			case API_DECODEFILE_NO_RIGHTS:
				cfs->error = getStringW(IDS_CONV_DRM_DECODE_FAIL,errorStrW,2048);
				return 0;
			case API_DECODEFILE_FAIL_NO_WARN:
				return 0;
			default:
				cfs->error = getStringW(IDS_CONV_ERROR_OPEN_FILE,errorStrW,2048);
				return 0;
		}
	}

	cfs->decoder=0;
	cfs->convert_thread=0;
	cfs->file_handle=0;
	cfs->audio_coder=0;
	cfs->encoder_mod=0;
	cfs->bps=0;
	cfs->channels=0;
	cfs->sample_rate=0;

	//find the encoding DLL
	if (cfs->destformat[0] != mmioFOURCC('P', 'C', 'M', ' '))
	{
		HMODULE mod = NULL;
		char * inifile = NULL;
		if(cfs->destformat[6] == mmioFOURCC('I','N','I',' ')) inifile = (char*)cfs->destformat[7];
		AudioCoder *ac = (AudioCoder *)getEncoder(parameters.bitsPerSample, parameters.channels, parameters.sampleRate, (int *) & cfs->destformat, 1, &mod, NULL,0, inifile);
		if (!ac)
		{
			decodeFile->CloseAudio(decoder);
			cfs->error = getStringW(IDS_CONV_ERROR_OPEN_ENCODER,errorStrW,2048);
			return 0;
		}
		cfs->audio_coder = ac;
		cfs->encoder_mod = mod;
	}

	cfs->killswitch = 0;
	cfs->decoder = decoder;

	cfs->bps = parameters.bitsPerSample;
	cfs->channels = parameters.channels;
	cfs->sample_rate = parameters.sampleRate;

	//open destination file
	HANDLE fh = CreateFileW(cfs->destfile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( fh == INVALID_HANDLE_VALUE )
	{
		decodeFile->CloseAudio(decoder);
		delete cfs->audio_coder;
		cfs->audio_coder = 0;
		FreeLibrary(cfs->encoder_mod);
		cfs->encoder_mod = 0;
		cfs->error = getStringW(IDS_CONV_ERROR_OPEN_DEST,errorStrW,2048);
		return 0;
	}
	cfs->file_handle = fh;

	DWORD id = 0;
	cfs->convert_thread = CreateThread(NULL, 0, convertThreadW, cfs, 0, &id);

	return 1;
}

int convert_file_test(convertFileStructW *cfs)
{
	// clear the buffer on starting otherwise we may return an invalid error message
	errorStrW[0]=0;
	AudioCoder *ac=0;
	HMODULE mod=0;

	if (cfs->destfile && cfs->sourcefile && !_wcsicmp(cfs->destfile, cfs->sourcefile))
	{
		cfs->error = getStringW(IDS_CONV_SRC_EQUALS_DEST,errorStrW,2048);
		return 0;
	}

	AudioParameters parameters;
	ifc_audiostream *decoder = decodeFile->OpenAudioBackground(cfs->sourcefile, &parameters);
	cfs->bytes_total= (int)(parameters.sizeBytes?parameters.sizeBytes:-1);

	if (!decoder)
	{
		switch(parameters.errorCode)
		{
			case API_DECODEFILE_UNSUPPORTED:
				cfs->error = getStringW(IDS_CONV_DECODER_MISSING,errorStrW,2048);
				return 0;
			case API_DECODEFILE_NO_INTERFACE:
				cfs->error = getStringW(IDS_CONV_INPUT_PLUGIN_NOT_SUPPORTING,errorStrW,2048);
				return 0;
			case API_DECODEFILE_NO_RIGHTS:
				cfs->error = getStringW(IDS_CONV_DRM_DECODE_FAIL,errorStrW,2048);
				return 0;
			case API_DECODEFILE_FAIL_NO_WARN:
				return 0;
			default:
				cfs->error = getStringW(IDS_CONV_ERROR_OPEN_FILE,errorStrW,2048);
				return 0;
		}
	}

	decodeFile->CloseAudio(decoder);
	cfs->decoder=0;
	cfs->convert_thread=0;
	cfs->file_handle=0;
	cfs->audio_coder=0;
	cfs->encoder_mod=0;
	cfs->bps=0;
	cfs->channels=0;
	cfs->sample_rate=0;

	//find the encoding DLL
	if (cfs->destformat[0] != mmioFOURCC('P', 'C', 'M', ' '))
	{
		char * inifile = NULL;
		if(cfs->destformat[6] == mmioFOURCC('I','N','I',' ')) inifile = (char*)cfs->destformat[7];
		ac = (AudioCoder *)getEncoder(parameters.bitsPerSample, parameters.channels, parameters.sampleRate, (int *) & cfs->destformat, 1, &mod, NULL,0, inifile);
		if (!ac)
		{
			cfs->error = getStringW(IDS_CONV_ERROR_OPEN_ENCODER,errorStrW,2048);
			return 0;
		}
		cfs->audio_coder = ac;
		cfs->encoder_mod = mod;
	}

	cfs->killswitch = 0;
	cfs->decoder = decoder;

	cfs->bps = parameters.bitsPerSample;
	cfs->channels = parameters.channels;
	cfs->sample_rate = parameters.sampleRate;

	//open destination file
	HANDLE fh = CreateFileW(cfs->destfile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( fh == INVALID_HANDLE_VALUE )
	{
		delete ac;
		cfs->audio_coder = 0;
		FreeLibrary(mod);
		cfs->encoder_mod = 0;
		cfs->error = getStringW(IDS_CONV_ERROR_OPEN_DEST,errorStrW,2048);
		return 0;
	}

	delete ac;
	cfs->audio_coder = 0;
	FreeLibrary(mod);
	cfs->encoder_mod = 0;
	CloseHandle(fh);

	return 1;
}

void convert_end(convertFileStruct *cfs)
{
	HANDLE handle = cfs->convert_thread;
	cfs->killswitch = 1;
	if (handle && handle != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(handle, 20000);
		CloseHandle(handle);
		cfs->convert_thread = INVALID_HANDLE_VALUE;
	}

	delete(cfs->audio_coder);
	cfs->audio_coder = 0;

	HMODULE mod = cfs->encoder_mod;
	if (mod)
	{
		FreeLibrary(mod);
		cfs->encoder_mod = 0;
	}
}

void convert_endW(convertFileStructW *cfs)
{
	HANDLE handle = cfs->convert_thread;
	cfs->killswitch = 1;
	if (handle && handle != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(handle, 20000);
		CloseHandle(handle);
		cfs->convert_thread = INVALID_HANDLE_VALUE;
	}

	delete(cfs->audio_coder);
	cfs->audio_coder = 0;

	HMODULE mod = cfs->encoder_mod;
	if (mod)
	{
		FreeLibrary(mod);
		cfs->encoder_mod = 0;
	}
}

void convert_enumfmts(converterEnumFmtStruct *cefs)
{
	//	cefs->enumProc(cefs->user_data, ".WAV output", mmioFOURCC('W', 'A', 'V', ' '));
	int destformat[8] = {0};
	getEncoder(0, 0, 0, (int *)&destformat, 0, NULL, 0, cefs);
}

HWND convert_config(convertConfigStruct *ccs)
{
	HMODULE mod = NULL;
	int destformat[8] = {ccs->format, };
	char * inifile = NULL;
	if(ccs->extra_data[6] == mmioFOURCC('I','N','I',' ')) 
		inifile = (char*)ccs->extra_data[7];
	HWND h = (HWND)getEncoder(0, 0, 0, (int *) & destformat, 0, &mod, ccs->hwndParent,0,inifile);
	ccs->hwndConfig = h;
	ccs->extra_data[0] = (intptr_t)mod;
	return h;
}

void convert_config_end(convertConfigStruct *ccs)
{
	HMODULE mod = (HMODULE)ccs->extra_data[0];
	DestroyWindow(ccs->hwndConfig);
	if (mod) FreeLibrary(mod);
}

void convert_setPriority(convertSetPriority *csp)
{
	if (csp->cfs)
	{
		HANDLE handle = csp->cfs->convert_thread;
		if (handle)
			SetThreadPriority(handle, csp->priority);
		else
		{
			//FUCKO> handle when separate process
		}
	}
}

void convert_setPriorityW(convertSetPriorityW *csp)
{
	if (csp->cfs)
	{
		HANDLE handle = (void *)csp->cfs->convert_thread;
		if (handle)
			SetThreadPriority(handle, csp->priority);
		else
		{
			//FUCKO> handle when separate process
		}
	}
}

int convert_setConfigItem(convertConfigItem *cci) {
	int ret = 0;
	int destformat[8] = {(int)cci->format, };
	HMODULE mod = NULL;
	if (!cci->configfile) cci->configfile=INI_FILEA;
	getEncoder(0,0,0, (int *) & destformat, 2, &mod, NULL,0,cci->configfile);
	if(mod) {
		int (*sci)(unsigned int outt, char *item, char *data, char *configfile);
		*((void **)&sci) = GetProcAddress(mod, "SetConfigItem");
		if(sci) {
			ret = sci(cci->format,cci->item,cci->data,cci->configfile);
		}
		FreeLibrary(mod);
	}
	return ret;
}

int convert_getConfigItem(convertConfigItem *cci) {
	int ret = 0;
	int destformat[8] = {(int)cci->format, };
	HMODULE mod = NULL;
	if (!cci->configfile) cci->configfile=INI_FILEA;
	getEncoder(0,0,0, (int *) & destformat, 2, &mod, NULL,0,cci->configfile);
	if(mod) {
		int (*gci)(unsigned int outt, char *item, char *data, int len, char *configfile);
		*((void **)&gci) = GetProcAddress(mod, "GetConfigItem");
		if(gci) {
			ret = gci(cci->format,cci->item,cci->data,cci->len,cci->configfile);
		}
		FreeLibrary(mod);
	}
	return ret;
}
