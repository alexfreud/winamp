#ifndef _TRANSCODER_IMP_H_
#define _TRANSCODER_IMP_H_

#include <windows.h>
#include <windowsx.h>
#include "..\..\General\gen_ml/itemlist.h"
#include "../winamp/wa_ipc.h"
#include "..\..\General\gen_ml/ml.h"
#include "DeviceView.h"
#include "nu/AutoWide.h"
#include "nu/AutoChar.h"
#include "transcoder.h"
#include "resource1.h"
#include <wchar.h>
#include <stdio.h>
#include <vector>

#define ENCODER_HIGHLY_PREFERRED 2
#define ENCODER_PREFERRED 1
#define ENCODER_NO_PREFERENCE 0
#define ENCODER_NOT_PREFERRED -1
#define ENCODER_DO_NOT_USE -2

class EncodableFormat 
{
public:
  unsigned int fourcc;
  wchar_t *desc;
  EncodableFormat(unsigned int fourcc,wchar_t *desc) :
    fourcc(fourcc)
  {
    this->desc = _wcsdup(desc);
  }

  ~EncodableFormat()
	{
    free(desc);
  }
};

typedef std::vector<EncodableFormat*> FormatList;


class TranscoderImp : public Transcoder 
{
protected:
	bool TranscoderDisabled;
	bool transrate;
	int transratethresh;
	bool translossless;
	wchar_t caption[100];
	wchar_t inifile[MAX_PATH];
	char inifileA[MAX_PATH];
	HINSTANCE hInst;
	HWND winampParent;
	HWND callbackhwnd;
	std::vector<unsigned int> outformats;
	FormatList formats;
	C_Config * config;
	convertFileStructW cfs;
	void (*callback)(void * callbackContext, wchar_t * status);
	void* callbackContext;
	bool convertDone;
	Device *device;

	void TranscodeProgress(int pc, bool done);
	bool StartTranscode(unsigned int destformat, wchar_t *inputFile, wchar_t *outputfile, bool test=false); // returns true if started ok.
	void EndTranscode();
	bool TestTranscode(wchar_t * file, unsigned int destformat);
	bool FormatAcceptable(wchar_t * format);
	bool FormatAcceptable(unsigned int format);
	int GetOutputFormat(wchar_t * file, int *bitrate=NULL);
	void AddEncodableFormat(const char *desc, unsigned int fourcc);
	void ReloadConfig();

public:
	TranscoderImp(HWND winampParent, HINSTANCE hInst, C_Config * config, Device *device);
	virtual ~TranscoderImp();

	virtual void LoadConfigProfile(wchar_t *profile);
	virtual void AddAcceptableFormat(wchar_t *format);
	virtual void AddAcceptableFormat(unsigned int format);

	// done when file is added to transfer queue
	// returns:
	//  -1 for can't transcode
	//  output file size estimate if can transcode
	// if ext is supplied, it should be a buffer with space for 5 characters, and will be filled with 
	//   the output file type file extention, eg, L".mp3"
	virtual int CanTranscode(wchar_t *file, wchar_t *ext = NULL, int length = -1);

	virtual bool ShouldTranscode(wchar_t * file); // false if no transcoding needed

	virtual int TranscodeFile(wchar_t *inputFile, wchar_t *outputFile, int *killswitch,  void (*callback)(void * callbackContext, wchar_t * status), void* callbackContext, wchar_t * caption=L"Transcoding %d%%");
	// done just before transfer OR in background after file is added to queue
	// extention is added to outputFile, allow 5 extra chars
	// callback, callbackcontext and killswitch should be similar to those passed by ml_pmp

	//virtual int TranscodeFileASync(wchar_t *inputFile, wchar_t *outputFile, int *killswitch, void (*callback)(void * callbackContext, wchar_t * status), void* callbackContext){return 0;};
	//asynchronous version of the above, details obvious

	virtual void GetTempFilePath(const wchar_t *ext, wchar_t *filename); // get a filename which can be used as a staging area. ext should be, i.e L".mp3"

  /* remember to call DestroyWindow on the return value when parent recieves the WM_DESTROY message.
     use like this:
       transcoderConfig = TranscoderImp::ConfigureTranscoder(hwndDlg,L"ml_pmp");
       RECT r;
       GetWindowRect(GetDlgItem(hwndDlg,IDC_PLACEHOLDER),&r);
       ScreenToClient(hwndDlg,(LPPOINT)&r);
       SetWindowPos(transcoderConfig,NULL,r.left,r.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
       ShowWindow(transcoderConfig,SW_SHOWNA);
     where IDC_PLACEHOLDER is an invisible group box of size 259x176 (in dialog units)
  */
	static void* ConfigureTranscoder(wchar_t * configProfile, HWND winampParent, C_Config * config, Device *dev);
	static BOOL transcodeconfig_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
	static void init();
	static void quit();

	friend LRESULT CALLBACK TranscodeMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif //_TRANSCODER_IMP_H_