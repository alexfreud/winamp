#include "api__ml_pmp.h"
#include "transcoder_imp.h"
#include "nu/ns_wc.h"
#include <shlwapi.h>
#include <strsafe.h>
#include <mmiscapi.h>

extern HWND CreateDummyWindow();

static std::vector<TranscoderImp*> transcoders;

LRESULT CALLBACK TranscodeMsgProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_WA_IPC && ( lParam == IPC_CB_CONVERT_STATUS || lParam == IPC_CB_CONVERT_DONE ) )
	{
		for ( TranscoderImp *t : transcoders )
		{
			if ( t->cfs.callbackhwnd == hwnd )
			{
				t->TranscodeProgress( (int)wParam, lParam == IPC_CB_CONVERT_DONE );
				break;
			}
		}
	}

	return 0;
}

static void fourccToString(unsigned int f, wchar_t * str) {
	wchar_t s[4] = {(wchar_t)(f&0xFF),(wchar_t)((f>>8)&0xFF),(wchar_t)((f>>16)&0xFF),0};
	wcsncpy(str,s,4);
	CharLower(str);
}

static unsigned int stringToFourcc(const wchar_t * str) {
	FOURCC cc = 0;
	char *ccc = (char *)&cc;
	// unrolled loop (this function gets called a lot on sync and autofill)
	if (str[0])
	{
		ccc[0] = (char)str[0];
		if (str[1])
		{
			ccc[1] = (char)str[1];
			if (str[2])
			{
				ccc[2] = (char)str[2];
				if (str[3])
				{
					ccc[3] = (char)str[3];
				}
			}
		}
	}
	CharUpperBuffA(ccc, 4);
	return cc;
}

static bool fourccEqual(unsigned int a, unsigned int b) {
	if((a & 0xFF000000) == 0 || (b & 0xFF000000) == 0)
		return (a & 0x00FFFFFF) == (b & 0x00FFFFFF);
	return a == b;
}

class TranscodeProfileCache {
public:
	unsigned int inputformat;
	unsigned int outputformat;
	int outputbitrate;
	TranscodeProfileCache(unsigned int inputformat,unsigned int outputformat,int outputbitrate) :
	inputformat(inputformat), outputformat(outputformat),outputbitrate(outputbitrate){}
};

static void enumProc(intptr_t user_data, const char *desc, int fourcc) 
{
	((FormatList *)user_data)->push_back(new EncodableFormat((unsigned int)fourcc,AutoWide(desc)));
}

static void BuildEncodableFormatsList(FormatList &list, HWND winampWindow, Device *device) 
{
	converterEnumFmtStruct e = {enumProc,(intptr_t)&list};
	SendMessage(winampWindow,WM_WA_IPC,(WPARAM)&e,IPC_CONVERT_CONFIG_ENUMFMTS);

	// filter out unacceptable formats
	int i = list.size();
	while (i--)
	{
		if (device && device->extraActions(DEVICE_VETO_ENCODER, list[i]->fourcc, 0, 0) == 1)
		{
			list.erase(list.begin() + i);
		}
	}
}

static CRITICAL_SECTION csTranscoder;

void TranscoderImp::init() {
	InitializeCriticalSection(&csTranscoder);
}

void TranscoderImp::quit() {
	DeleteCriticalSection(&csTranscoder);
}

TranscoderImp::TranscoderImp(HWND winampParent, HINSTANCE hInst, C_Config * config, Device *device)
: device(device), config(config), winampParent(winampParent), hInst(hInst)
{
	EnterCriticalSection(&csTranscoder);

	transratethresh = config->ReadInt(L"forcetranscodingbitrate",250);
	transrate = !!config->ReadInt(L"transrate",0);
	translossless = !!config->ReadInt(L"translossless",0);
	TranscoderDisabled = !config->ReadInt(L"enableTranscoder",1);

	int current_fourcc = config->ReadInt(L"lastusedencoder", 0);
	if (current_fourcc == 0)
	{
		// TODO: ask for default from plugin
		config->WriteInt(L"lastusedencoder", ' A4M');
	}

	WASABI_API_LNGSTRINGW_BUF(IDS_TRANSFER_PERCENT,caption,100);
	ZeroMemory(&cfs,sizeof(convertFileStruct));
	this->callback = NULL;
	transcoders.push_back(this);
	callbackhwnd = CreateDummyWindow();
	StringCchCopy(inifile, ARRAYSIZE(inifile), config->GetIniFile());
	WideCharToMultiByteSZ(CP_ACP, 0, inifile, -1, inifileA, MAX_PATH, 0, 0);
	BuildEncodableFormatsList(formats, winampParent,  device);
	LeaveCriticalSection(&csTranscoder);
}

TranscoderImp::~TranscoderImp() 
{
	EnterCriticalSection(&csTranscoder);

	//transcoders.eraseObject(this);
	auto it = std::find(transcoders.begin(), transcoders.end(), this);
	if (it != transcoders.end())
	{
		transcoders.erase(it);
	}

	//formats.deleteAll();
	for (auto format : formats)
	{
		delete format;
	}
	formats.clear();

	DestroyWindow(callbackhwnd);
	LeaveCriticalSection(&csTranscoder);
}

void TranscoderImp::LoadConfigProfile(wchar_t *profile) {
}

void TranscoderImp::ReloadConfig() 
{
	//formats.deleteAll();
	for (auto format : formats)
	{
		delete format;
	}
	formats.clear();

	BuildEncodableFormatsList(formats, winampParent, device);

	transratethresh = config->ReadInt(L"forcetranscodingbitrate",250);
	transrate = !!config->ReadInt(L"transrate",0);
	translossless = !!config->ReadInt(L"translossless",0);
	TranscoderDisabled = !config->ReadInt(L"enableTranscoder",1);
}

void TranscoderImp::AddAcceptableFormat(unsigned int format)
{
	outformats.push_back(format);
}

void TranscoderImp::AddAcceptableFormat(wchar_t *format)
{
	outformats.push_back(stringToFourcc(format));
}

static bool FileExists(const wchar_t *file)
{
	return GetFileAttributesW(file) != INVALID_FILE_ATTRIBUTES;
}

static int getFileLength(const wchar_t * file, HWND winampParent) { // returns length in seconds
	basicFileInfoStructW b={0};
	b.filename=file;
	SendMessage(winampParent,WM_WA_IPC,(WPARAM)&b,IPC_GET_BASIC_FILE_INFOW);
	return b.length;
}

static bool isFileLossless(const wchar_t * file) {
	wchar_t ret[64] = {0};
	if (AGAVE_API_METADATA && AGAVE_API_METADATA->GetExtendedFileInfo(file, L"lossless", ret, 64) && ret[0] == '1')
		return true;
	return false;
}

static int getFileBitrate(const wchar_t * file, HWND winampParent) { // returns bitrate in bits per second.
	int secs = getFileLength(file,winampParent);
	if(!secs) return 0;
	FILE * f = _wfopen(file,L"rb");
	int len = 0;
	if(f) { fseek(f,0,2); len=ftell(f); fclose(f); }
	return (len/secs)*8;
}

void TranscoderImp::GetTempFilePath(const wchar_t *ext, wchar_t *path) {
	wchar_t dir[MAX_PATH] = {0};
	GetTempPath(MAX_PATH,dir);
	GetTempFileName(dir,L"transcode",0,path);
	_wunlink(path);
	wchar_t *e = wcsrchr(path,L'.');
	if(e) *e=0;
	wcscat(path,ext);
	_wunlink(path);
}

void TranscoderImp::TranscodeProgress(int pc, bool done) {
	if(!done) {
		wchar_t buf[128] = {0};
		StringCchPrintf(buf, ARRAYSIZE(buf), caption,pc);
		if(callback) callback(callbackContext,buf);
	}
	else convertDone = done;
}

bool TranscoderImp::StartTranscode(unsigned int destformat, wchar_t *inputFile, wchar_t *outputFile, bool test) {
	cfs.callbackhwnd = callbackhwnd;
	cfs.sourcefile = _wcsdup(inputFile);
	cfs.destfile = _wcsdup(outputFile);
	cfs.destformat[0] = destformat;
	cfs.destformat[6] = mmioFOURCC('I','N','I',' ');
	cfs.destformat[7] = (intptr_t)inifileA;
	cfs.error = L"";
	if(!SendMessage(winampParent,WM_WA_IPC,(WPARAM)&cfs,IPC_CONVERTFILEW)) 
	{
		if(cfs.error && callback) 
			callback(callbackContext,cfs.error);
		return false;
	}
	if(!test)
	{
		convertSetPriorityW csp = {&cfs,THREAD_PRIORITY_NORMAL};
		SendMessage(winampParent, WM_WA_IPC, (WPARAM)&csp, IPC_CONVERT_SET_PRIORITYW);
		TranscodeProgress(0,false);
	}
	return true;
}

void TranscoderImp::EndTranscode()
{
	cfs.callbackhwnd = NULL;
	SendMessage(winampParent,WM_WA_IPC,(WPARAM)&cfs,IPC_CONVERTFILE_END);
	free(cfs.sourcefile);
	free(cfs.destfile);
	ZeroMemory(&cfs,sizeof(convertFileStruct));
}

bool TranscoderImp::TestTranscode(wchar_t * file, unsigned int destformat) 
{
	wchar_t tempfn[MAX_PATH], ext[5]=L".";
	fourccToString(destformat,&ext[1]);
	GetTempFilePath(ext,tempfn);

	convertFileStructW cfs;
	cfs.callbackhwnd = callbackhwnd;
	cfs.sourcefile = file;
	cfs.destfile = tempfn;
	cfs.destformat[0] = destformat;
	cfs.destformat[6] = mmioFOURCC('I','N','I',' ');
	cfs.destformat[7] = (intptr_t)inifileA;
	cfs.error = L"";

	int v = SendMessage(winampParent,WM_WA_IPC,(WPARAM)&cfs,IPC_CONVERT_TEST);
	_wunlink(tempfn);
	return !!v;
}

bool TranscoderImp::FormatAcceptable(unsigned int format) 
{
	int l = outformats.size();
	for(int i=0; i<l; i++)
	{
		if(fourccEqual(outformats[i], format)) 
			return true;
	}
	return false;
}

bool TranscoderImp::FormatAcceptable(wchar_t * format) 
{
	return FormatAcceptable(stringToFourcc(format));
}

int TranscoderImp::GetOutputFormat(wchar_t * file, int *bitrate)
{
	if (!FileExists(file))
		return 0;
	int fourcc = config->ReadInt(L"lastusedencoder",' A4M');

	if (TestTranscode(file,fourcc)) 
	{
        char buf[100]="128";
        convertConfigItem ccs={fourcc,"bitrate",buf,100, inifileA};
        SendMessage(winampParent,WM_WA_IPC,(WPARAM)&ccs,IPC_CONVERT_CONFIG_GET_ITEM);
        int br = atoi(buf)*1000;
        if(bitrate) *bitrate = br;
        return fourcc;
	}
	return 0;
}

bool TranscoderImp::ShouldTranscode(wchar_t * file) 
{
	if(TranscoderDisabled) return false;
	wchar_t * ext = wcsrchr(file,L'.');
	if(ext && FormatAcceptable(&ext[1])) {
		if(transrate && getFileBitrate(file,winampParent) > 1000*transratethresh) 
			return true;
		else if (translossless && isFileLossless(file)) 
			return true;
		else return false;
	}
	return true;
}

int TranscoderImp::CanTranscode(wchar_t * file, wchar_t * ext, int length)
{
	if(TranscoderDisabled) return -1;
	int bitrate;
	unsigned int fmt = GetOutputFormat(file,&bitrate);
	if(fmt) {
		if(ext) {
			ext[0]=L'.'; ext[1]=0;
			char extA[8]=".";
			convertConfigItem c = {fmt,"extension",&extA[1],6,AutoCharDup(config->GetIniFile())};
			SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&c,IPC_CONVERT_CONFIG_GET_ITEM);
			if(extA[1]) wcsncpy(ext,AutoWide(extA), 10);
			else fourccToString(fmt,&ext[1]);
			free(c.configfile);
		}
		if (length <= 0)
			length = getFileLength(file,winampParent);
		return (bitrate/8) * length;  // should transcode
	}
	return -1; // transcoding impossible
}

extern void filenameToItemRecord(wchar_t * file, itemRecordW * ice);
extern void copyTags(itemRecordW * in, wchar_t * out);

int TranscoderImp::TranscodeFile(wchar_t *inputFile, wchar_t *outputFile, int *killswitch, void (*callbackFunc)(void * callbackContext, wchar_t * status), void* callbackContext, wchar_t * caption) 
{
	if(caption) lstrcpyn(this->caption,caption,100);
	this->callback = callbackFunc;
	this->callbackContext = callbackContext;
	convertDone = false;
	int format = GetOutputFormat(inputFile);
	if(!format) return -1;
	if(!StartTranscode(format,inputFile,outputFile))
		return -1;
	while(!convertDone && !(*killswitch)) 
		Sleep(50);
	EndTranscode();

	// copy the tags over
	itemRecordW ice={0};
	filenameToItemRecord(inputFile,&ice);
	copyTags(&ice,outputFile);
	freeRecord(&ice);

	if(convertDone && callback) 
		callback(callbackContext,L"Done");
	this->callback = NULL;
	return convertDone?0:-1;
}

static void doConfigResizeChild(HWND parent, HWND child) 
{
	if (child) 
	{
		RECT r;
		GetWindowRect(GetDlgItem(parent, IDC_ENC_CONFIG), &r);
		ScreenToClient(parent, (LPPOINT)&r);
		SetWindowPos(child, 0, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		ShowWindow(child, SW_SHOWNA);
	}
}

struct ConfigTranscoderParam
{
	ConfigTranscoderParam()
	{
		winampParent=0;
		configfile=0;
		memset(&ccs, 0, sizeof(ccs));
		config=0;
		dev=0;
	}

	~ConfigTranscoderParam()
	{
		//list.deleteAll();
		for (auto l : list)
		{
			delete l;
		}
		list.clear();


		free((char*)ccs.extra_data[7]);
		free(configfile);
	}
	HWND winampParent;
	wchar_t *configfile;
	FormatList list;
	convertConfigStruct ccs;
	C_Config * config;
	Device *dev;
};

static INT_PTR CALLBACK config_dlgproc_transcode_advanced(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static ConfigTranscoderParam *p;
	switch(uMsg)
	{
		case WM_INITDIALOG:
			p = (ConfigTranscoderParam *)lParam;
			SetDlgItemText(hwndDlg,IDC_FORCE_BITRATE,p->config->ReadString(L"forcetranscodingbitrate",L"250"));
			if(p->config->ReadInt(L"transrate",0)) CheckDlgButton(hwndDlg,IDC_CHECK_FORCE,BST_CHECKED);
			if(p->config->ReadInt(L"translossless",0)) CheckDlgButton(hwndDlg,IDC_CHECK_FORCE_LOSSLESS,BST_CHECKED);
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
				{
					wchar_t buf[10]=L"";
					GetDlgItemText(hwndDlg,IDC_FORCE_BITRATE,buf,10);
					p->config->WriteString(L"forcetranscodingbitrate",buf);
					p->config->WriteInt(L"transrate",IsDlgButtonChecked(hwndDlg,IDC_CHECK_FORCE)?1:0);
					p->config->WriteInt(L"translossless",IsDlgButtonChecked(hwndDlg,IDC_CHECK_FORCE_LOSSLESS)?1:0);
				}
				case IDCANCEL:
					EndDialog(hwndDlg,0);
				break;
			}
		break;
	}
	return 0;
}

BOOL TranscoderImp::transcodeconfig_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	static ConfigTranscoderParam *p;
	switch(uMsg) {
		case WM_INITDIALOG:
		{
			p = (ConfigTranscoderParam *)lParam;
			if(p->config->ReadInt(L"enableTranscoder",1)) CheckDlgButton(hwndDlg,IDC_ENABLETRANSCODER,BST_CHECKED);

			BuildEncodableFormatsList(p->list, p->winampParent,  p->dev);
			p->ccs.hwndParent = hwndDlg;

			int encdef = p->config->ReadInt(L"lastusedencoder",0);
			for(size_t i=0; i < p->list.size(); i++) 
			{
				EncodableFormat * f = p->list[i];
				int a = SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_ADDSTRING, 0, (LPARAM)f->desc);
				SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_SETITEMDATA, (WPARAM)a, (LPARAM)f);
				if(i==0 && encdef == 0) encdef = f->fourcc;
				if(f->fourcc == encdef) 
				{
					SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_SETCURSEL, (WPARAM)a, 0);
					p->ccs.format = f->fourcc;
				}
			}

			p->ccs.hwndParent = hwndDlg;
			HWND h = (HWND)SendMessage(p->winampParent, WM_WA_IPC, (WPARAM)&p->ccs, IPC_CONVERT_CONFIG);
			doConfigResizeChild(hwndDlg, h);
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_ENABLETRANSCODER:
					p->config->WriteInt(L"enableTranscoder",IsDlgButtonChecked(hwndDlg,IDC_ENABLETRANSCODER)?1:0);
				break;
				case IDC_ADVANCED:
					return WASABI_API_DIALOGBOXPARAMW(IDD_CONFIG_TRANSCODING_ADVANCED,hwndDlg,config_dlgproc_transcode_advanced,(LPARAM)p);
				case IDC_ENCFORMAT:
					if (HIWORD(wParam) != CBN_SELCHANGE) return 0;
					{
						int sel = SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETCURSEL, 0, 0);
						if (sel != CB_ERR) 
						{
							SendMessage(p->winampParent, WM_WA_IPC, (WPARAM)&p->ccs, IPC_CONVERT_CONFIG_END);
							EncodableFormat * f = (EncodableFormat *)SendDlgItemMessage(hwndDlg, IDC_ENCFORMAT, CB_GETITEMDATA, sel, 0);
							p->ccs.format = f->fourcc;

							HWND h = (HWND)SendMessage(p->winampParent, WM_WA_IPC, (WPARAM)&p->ccs, IPC_CONVERT_CONFIG);
							doConfigResizeChild(hwndDlg, h);
							p->config->WriteInt(L"lastusedencoder",p->ccs.format);
						}
					}
				break;
			}
		break;
		case WM_DESTROY:
		{
			p->config->WriteInt(L"lastusedencoder",p->ccs.format);
			SendMessage(p->winampParent, WM_WA_IPC, (WPARAM)&p->ccs, IPC_CONVERT_CONFIG_END);
			delete p;

			for( TranscoderImp *l_transcoder : transcoders )
				l_transcoder->ReloadConfig();
		}
		break;
	}
	return 0;
}

void* TranscoderImp::ConfigureTranscoder(wchar_t * configProfile, HWND winampParent, C_Config * config, Device *dev)
{
	ConfigTranscoderParam * p = new ConfigTranscoderParam;
	p->config = config;
	p->winampParent=winampParent;
	p->configfile=_wcsdup(config->GetIniFile());
	p->ccs.extra_data[6] = mmioFOURCC('I','N','I',' ');
	p->ccs.extra_data[7] = (int)AutoCharDup(p->configfile);
	p->dev = dev;
	return p;
}