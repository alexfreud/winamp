#include "main.h"
#include <bfc/dispatch.h>
#include <windowsx.h>
#include "shlobj.h"
#include "..\..\General\gen_ml\ml_ipc_0313.h"
#include "..\..\General\gen_ml\childwnd.h"
#include "../Winamp/wa_dlg.h"
#include "../xml/ifc_xmlreadercallback.h"
#include "../xml/obj_xml.h"
#include <api/service/waServiceFactory.h>
#include "resource.h"
#include "api.h"

typedef void (*ChildResizeFunc)(HWND, ChildWndResizeItem*, int);
static ChildResizeFunc ml_childresize_init=0, ml_childresize_resize=0;

typedef int (*HookDialogFunc)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HookDialogFunc ml_hook_dialog_msg = 0;

typedef void (*DrawFunc)(HWND hwndDlg, int *tab, int tabsize); 
static DrawFunc ml_draw = 0;

LRESULT CALLBACK view_xmlexDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static HWND m_hwnd;

static ChildWndResizeItem xmlwnd_rlist[]={
	{IDC_LIST,0x0011},
	{IDC_LOAD,0x0101},
};

//--------------------------

class ListXMLLoader : public ifc_xmlreadercallback
{
public:
	ListXMLLoader(HWND _hwndList) : hwndList(_hwndList), index(0) {}
	void loadSongFromXml(const wchar_t* filename, const wchar_t* artist, const wchar_t* title) 
	{
		LVITEM lvi = {0, };
		lvi.mask = LVIF_TEXT;
		lvi.iItem = index;
		lvi.pszText = (LPTSTR)filename;
		lvi.cchTextMax = lstrlenW(filename);
		SendMessage(hwndList, LVM_INSERTITEMW, 0, (LPARAM)&lvi);

		lvi.iSubItem = 1;
		lvi.pszText = (LPTSTR)artist;
		lvi.cchTextMax = lstrlenW(artist);
		SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvi);

		lvi.iSubItem = 2;
		lvi.pszText = (LPTSTR)title;
		lvi.cchTextMax = lstrlenW(title);
		SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvi);

		index++;
	}
	void reset() 
	{
		ListView_DeleteAllItems(hwndList);
		index = 0;
	}
	void loadXML(wchar_t []);

	/* XML loader callback */
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
	{
		//read subtags of LIBRARY
		if(!wcsicmp(xmlpath, L"LIBRARY\fSONG"))
		{
			//getItemValue() will return the value for an attribute
			loadSongFromXml(params->getItemValue(L"FILENAME"), params->getItemValue(L"ARTIST"),params->getItemValue(L"TITLE"));
		}
	}

private:
	HWND hwndList;
	int index;

protected: // this is a Wasabi object, so we need to declare a dispatch table
	RECVS_DISPATCH;
};

/* Dispatch table for a Wasabi object */
#define CBCLASS ListXMLLoader
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
END_DISPATCH;
#undef CBCLASS

/* helper function for ListXMLLoader::loadXML */
static bool LoadFile(obj_xml *parser, const wchar_t *filename)
{
	HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return false;

	char data[1024];

	DWORD bytesRead;
	while (true)
	{
		if (ReadFile(file, data, 1024, &bytesRead, NULL) && bytesRead)
		{
			parser->xmlreader_feed(data, bytesRead);
		}
		else
			break;
	}

	CloseHandle(file);
	parser->xmlreader_feed(0, 0);

	return true;
}

void ListXMLLoader::loadXML(wchar_t xmlfile[MAX_PATH] = L"xmltest.xml") 
{
	// reset the listview state
	reset();

	// get an XML parser object from the service manager
	obj_xml *parser=0;
	waServiceFactory *parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
		parser = (obj_xml *)parserFactory->getInterface();

	if (parser)
	{
		//set up a tag that we can read
		//within StartTag(), we can read all subtags of this tag
		parser->xmlreader_registerCallback(L"LIBRARY\f*", this);
		parser->xmlreader_open();

		LoadFile(parser, xmlfile);

		parser->xmlreader_unregisterCallback(this);
		parser->xmlreader_close();
		parserFactory->releaseInterface(parser);
	}	
}
//--------------------------

/* This function gets called directly from gen_ml when it wants our plugin to do something
   we're only handling dialog creation, but there are lots of other messages a full-featured 
	 plugin would deal with */

INT_PTR xmlex_pluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	if (message_type == ML_MSG_TREE_ONCREATEVIEW && param1 == xmlex_treeItem)
	{
		return (INT_PTR)CreateDialog(plugin.hDllInstance, MAKEINTRESOURCE(IDD_VIEW_XMLEX), (HWND)(LONG_PTR)param2, (DLGPROC)view_xmlexDialogProc);
	}
	return 0;
}

static BOOL xmlex_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) 
{
	/* gen_ml has some helper functions to deal with skinned dialogs,
	   we're going to grab their function pointers.
		 for definition of magic numbers, see gen_ml/ml.h	 */
	if (!ml_childresize_init)
	{
		/* skinning helper functions */
		ml_hook_dialog_msg = (HookDialogFunc)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)2, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_draw = (DrawFunc)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)3, ML_IPC_SKIN_WADLG_GETFUNC);

		/* resizing helper functions */
		ml_childresize_init = (ChildResizeFunc)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)32, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_childresize_resize = (ChildResizeFunc)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)33, ML_IPC_SKIN_WADLG_GETFUNC);
	}

	m_hwnd=hwnd;
	HWND listWnd = GetDlgItem(hwnd,IDC_LIST);

	/* add listview columns */
	LVCOLUMN lvc = {0, };
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = (LPTSTR)L"Filename";
	lvc.cx = 250;
	SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)0, (LPARAM)&lvc);

	lvc.pszText = (LPTSTR)L"Artist";
	lvc.cx = 150;
	SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)1, (LPARAM)&lvc);

	lvc.pszText = (LPTSTR)L"Title";
	lvc.cx = 150;
	SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)2, (LPARAM)&lvc);

	/* skin dialog */
	MLSKINWINDOW sw;
	sw.skinType = SKINNEDWND_TYPE_DIALOG;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = hwnd;
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	/* skin listview */
	sw.hwndToSkin = listWnd;
	sw.skinType = SKINNEDWND_TYPE_LISTVIEW;
	sw.style = SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
	MLSkinWindow(plugin.hwndLibraryParent, &sw);
	
	/* skin button */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_LOAD);
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	ml_childresize_init(hwnd, xmlwnd_rlist, sizeof(xmlwnd_rlist) / sizeof(xmlwnd_rlist[0]));

	//all other initialization is done.  lets wait 20ms before we actually do anything with this plugin
	//this way other (more important) things finish before this does
	SetTimer(hwnd,101,20,NULL);

	return FALSE;
}

static BOOL xmlex_OnSize(HWND hwnd, UINT state, int cx, int cy) 
{
	if (state != SIZE_MINIMIZED) 
		ml_childresize_resize(hwnd, xmlwnd_rlist, sizeof(xmlwnd_rlist) / sizeof(xmlwnd_rlist[0]));
	return FALSE;
}

static BOOL xmlex_OnDestroy(HWND hwnd)
{
	m_hwnd=0;
	return FALSE;
}
static void xmlex_OnTimer(HWND hwnd, UINT id)
{
	if (id == 101)
	{
		KillTimer(hwnd,101);
		// populate list with default local file, no pre-loaded xml file if not in working dir (really will only pre-load something in debug mode)
		ListXMLLoader loader(GetDlgItem(hwnd, IDC_LIST));
		loader.loadXML();
	}
}
static BOOL xmlex_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id) {
	case IDC_LOAD: 
		{
			wchar_t filename[256] = L"";
			//browse box supported in windows 2000+
			//if this doesnt work for you (old versions of windows) just know that the file name is set in ofn.lpstrFile which is then moved to filename variable
			OPENFILENAME ofn = {0};
			ofn.lStructSize = sizeof (OPENFILENAME);
			ofn.hwndOwner=hwnd;
			ofn.lpstrFilter=L"XML Files (*.xml)\0*.XML\0\0";
			ofn.lpstrCustomFilter=NULL;
			ofn.nFilterIndex=1;
			ofn.lpstrFile=filename;  //contains file name after user has selected it
			ofn.nMaxFile=MAX_PATH;
			ofn.lpstrFileTitle=NULL;
			ofn.lpstrInitialDir=NULL;
			ofn.Flags=OFN_PATHMUSTEXIST;
			GetOpenFileName(&ofn);
			if(*filename) //do not load on browse -> cancel
			{
				ListXMLLoader loader(GetDlgItem(hwnd, IDC_LIST));
				loader.loadXML(filename);
			}
		}
		break;
	}
	return 0;
}

LRESULT CALLBACK view_xmlexDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	/* first, ask the dialog skinning system if it wants to do anything with the message 
	   the function pointer gets set during WM_INITDIALOG so might be NULL for the first few messages
		 in theory we could grab it right here if we don't have it, but it's not necessary
		 and I wanted to put all the function pointer gathering code in the same place for this example	*/
	if (ml_hook_dialog_msg) 
	{
		INT_PTR a = ml_hook_dialog_msg(hwndDlg, uMsg, wParam, lParam);
		if (a)
			return a;
	}

	switch(uMsg) {
		HANDLE_MSG(hwndDlg, WM_INITDIALOG, xmlex_OnInitDialog);
		HANDLE_MSG(hwndDlg, WM_TIMER, xmlex_OnTimer);
		HANDLE_MSG(hwndDlg, WM_COMMAND, xmlex_OnCommand);
		HANDLE_MSG(hwndDlg, WM_SIZE, xmlex_OnSize);
	case WM_PAINT:
		{
			int tab[] = { IDC_LIST|DCW_SUNKENBORDER};
			ml_draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
		}
		return 0;
		HANDLE_MSG(hwndDlg, WM_DESTROY, xmlex_OnDestroy);

	}
	return FALSE;
}
