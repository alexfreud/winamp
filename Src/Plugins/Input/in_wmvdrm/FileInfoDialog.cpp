#include "Main.h"
#include "FileInfoDialog.h"
#include "WMInformation.h"
#include "resource.h"
#include "../nu/AutoChar.h"

#include "WMDRMModule.h"
#include "WMPlaylist.h"

// blah! commented out a load of shit because we can't have the advanced pane edit data cause wma sucks.

class Info
{
public:
	Info(const wchar_t *filename);
	~Info();
	//bool Save(HWND parent);
	int Error();
	int GetNumMetadataItems();
	void EnumMetadata(int n,wchar_t *key,int keylen, wchar_t *val, int vallen);
	//void RemoveMetadata(wchar_t * key);
	//void RemoveMetadata(int n);
	//void SetMetadata(wchar_t *key, wchar_t *val);
	//void SetMetadata(int n, wchar_t *key, wchar_t *val);
	//void SetTag(int n,wchar_t *key); // changes the key name
private:
	WMInformation wminfo;
	const wchar_t *filename;
};

Info::Info(const wchar_t *filename) : wminfo(filename), filename(filename)
{
}

Info::~Info()
{
}
/*
bool Info::Save(HWND parent)
{
	if (!wminfo.MakeWritable(filename))
	{
		wchar_t title[64] = {0};
		if (activePlaylist.IsMe(filename) && mod.playing)
		{
			// TODO: this is a race condition.  we might have stopped in between the above if () and now... 
			int outTime = mod.GetOutputTime();
			winamp.PressStop();
			wminfo.MakeWritable(filename);	

			SendMessage(parent,WM_USER+1,0,0);
			//WriteEditBoxes();
			
			wminfo.Flush();
			wminfo.MakeReadOnly(filename);
			mod.startAtMilliseconds=outTime;
			winamp.PressPlay();
			return true;
		}
		else if (!wminfo.MakeReadOnly(filename))
			MessageBox(parent, WASABI_API_LNGSTRINGW(IDS_UNABLE_TO_WRITE_TO_FILE_DOES_FILE_EXIST),
					   WASABI_API_LNGSTRINGW_BUF(IDS_UNABLE_TO_WRITE_TO_FILE,title,64), MB_OK);
		else
			MessageBox(parent, WASABI_API_LNGSTRINGW(IDS_UNABLE_TO_WRITE_TO_FILE_MAY_NOT_HAVE_ACCESS),
					   WASABI_API_LNGSTRINGW_BUF(IDS_UNABLE_TO_WRITE_TO_FILE,title,64), MB_OK);

		return false;
	}

	SendMessage(parent,WM_USER+1,0,0);
	//WriteEditBoxes();

	if (!wminfo.Flush())
	{
		wchar_t* title = WASABI_API_LNGSTRINGW(IDS_SAVE_FAILED);
		MessageBox(NULL, title, title, MB_OK);
	}

	wminfo.MakeReadOnly(filename);
	return true;
}
*/
int Info::Error()
{
	return wminfo.ErrorOpening();
}

int Info::GetNumMetadataItems()
{
	return wminfo.GetNumberAttributes();
}

void Info::EnumMetadata(int n,wchar_t *key,int keylen, wchar_t *val, int vallen)
{
	if(keylen) key[0]=0;
	if(vallen) val[0]=0;
	wminfo.GetAttribute(n, key, keylen, val, vallen);
}
/*
void Info::RemoveMetadata(wchar_t * key)
{
	wminfo.DeleteAttribute(key);
}

void Info::RemoveMetadata(int n)
{
	wchar_t key[256] = {0};
	EnumMetadata(n,key,256,NULL,0);
	if(key[0])
		RemoveMetadata(key);
}

void Info::SetMetadata(wchar_t *key, wchar_t *val)
{
	wminfo.SetAttribute(key,val);
}

void Info::SetTag(int n, wchar_t *key)
{ // changes the key name
	wchar_t val[2048]=L"";
	wchar_t oldkey[256]=L"";
	EnumMetadata(n,oldkey,256,val,2048);
	RemoveMetadata(oldkey);
	wminfo.SetAttribute(key,val);
}
*/
bool WMTagToWinampTag(wchar_t * tag, int len) 
{
	const wchar_t *f = GetAlias_rev(tag);
	if(f)
	{
		lstrcpyn(tag,f,len);
		return true;
	}
	return false;
}

bool WinampTagToWMTag(wchar_t * tag, int len) 
{
	const wchar_t *f = GetAlias(tag);
	if(f)
	{
		lstrcpyn(tag,f,len);
		return true;
	}
	return false;
}

static INT_PTR CALLBACK ChildProc_Advanced(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	static int ismychange=0;
	switch(msg)
	{
	case WM_NOTIFYFORMAT:
		return NFR_UNICODE;
	case WM_INITDIALOG:
		{
			//SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
			HWND hwndlist = GetDlgItem(hwndDlg,IDC_LIST);
			ListView_SetExtendedListViewStyle(hwndlist, LVS_EX_FULLROWSELECT);
			LVCOLUMN lvc = {0, };
			lvc.mask = LVCF_TEXT|LVCF_WIDTH;
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_NAME);
			lvc.cx = 82;
			ListView_InsertColumn(hwndlist, 0, &lvc);
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_VALUE);
			lvc.cx = 160;
			ListView_InsertColumn(hwndlist, 1, &lvc);

			Info *info = (Info *)lParam;
			int n = info->GetNumMetadataItems();
			for(int i=0; i<n; i++) {
				wchar_t key[512] = {0};
				wchar_t value[2048] = {0};
				info->EnumMetadata(i,key,512,value,2048);
				if(key[0]) {
					LVITEMW lvi={LVIF_TEXT,i,0};
					lvi.pszText = key;
					SendMessage(hwndlist,LVM_INSERTITEMW,0,(LPARAM)&lvi);
					lvi.iSubItem=1;
					lvi.pszText = value;
					SendMessage(hwndlist,LVM_SETITEMW,0,(LPARAM)&lvi);
				}
			}
			ListView_SetColumnWidth(hwndlist,0,LVSCW_AUTOSIZE);
			ListView_SetColumnWidth(hwndlist,1,LVSCW_AUTOSIZE);

			//SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
			//SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
			//EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
			//EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
			//EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);

			delete info;
		}
		break;
	case WM_DESTROY:
		{
			HWND hwndlist = GetDlgItem(hwndDlg,IDC_LIST);
			ListView_DeleteAllItems(hwndlist);
			while(ListView_DeleteColumn(hwndlist,0));
			Info * info = (Info*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			if(info) delete info;
		}
		break;
		/*
	case WM_USER+1:
		{
			Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			info->wminfo.ClearAllAttributes();
			wchar_t key[100] = {0}, value[2048] = {0};
			HWND hlist = GetDlgItem(hwndDlg,IDC_LIST);
			int n = ListView_GetItemCount(hlist);
			for(int i=0; i<n; i++)
			{
				key[0]=value[0]=0;
				LVITEMW lvi={LVIF_TEXT,i,0};
				lvi.pszText=key;
				lvi.cchTextMax=100;
				SendMessage(hlist,LVM_GETITEMW,0,(LPARAM)&lvi);
				lvi.iSubItem = 1;
				lvi.pszText = value;
				lvi.cchTextMax=2048;
				SendMessage(hlist,LVM_GETITEMW,0,(LPARAM)&lvi);
				if(key[0])
					info->SetMetadata(key,value);
			}
		}
		break;
		*/
	case WM_USER:
		if(wParam && lParam && !ismychange)
		{
			wchar_t * value = (wchar_t*)lParam;
			wchar_t tag[100] = {0};
			lstrcpynW(tag,(wchar_t*)wParam,100);
			WinampTagToWMTag(tag,100);
			/*
			Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if(!*value) info->RemoveMetadata(tag);
			else info->SetMetadata(tag,value);
			*/
			HWND hlist = GetDlgItem(hwndDlg,IDC_LIST);
			int n = ListView_GetItemCount(hlist);
			for(int i=0; i<n; i++)
			{
				wchar_t key[100]=L"";
				LVITEMW lvi={LVIF_TEXT,i,0};
				lvi.pszText=key;
				lvi.cchTextMax=100;
				SendMessage(hlist,LVM_GETITEMW,0,(LPARAM)&lvi);
				if(!_wcsicmp(key,tag))
				{
					lvi.iSubItem = 1;
					lvi.pszText = value;
					SendMessage(hlist,LVM_SETITEMW,0,(LPARAM)&lvi);
					if(!*value)
						ListView_DeleteItem(hlist,i);
					/*
					else if(ListView_GetItemState(hlist,i,LVIS_SELECTED))
						SetDlgItemTextW(hwndDlg,IDC_VALUE,value);
						*/
					return 0;
				}
			}
			// bew hew, not found
			LVITEMW lvi={0,0x7FFFFFF0,0};
			n = SendMessage(hlist,LVM_INSERTITEMW,0,(LPARAM)&lvi);
			lvi.mask = LVIF_TEXT;
			lvi.iItem = n;
			lvi.iSubItem = 0;
			lvi.pszText = tag;
			SendMessage(hlist,LVM_SETITEMW,0,(LPARAM)&lvi);
			lvi.iSubItem = 1;
			lvi.pszText = value;
			SendMessage(hlist,LVM_SETITEMW,0,(LPARAM)&lvi);
		}
		break;
		/*
	case WM_NOTIFY:
    {
			LPNMHDR l=(LPNMHDR)lParam;
			if(l->idFrom==IDC_LIST && l->code == LVN_ITEMCHANGED) {
				LPNMLISTVIEW lv=(LPNMLISTVIEW)lParam;
				if(lv->uNewState & LVIS_SELECTED) {
					int n = lv->iItem;
					LVITEMW lvi={LVIF_TEXT,lv->iItem,0};
					wchar_t key[100] = {0};
					wchar_t value[1024] = {0};
					lvi.pszText=key;
					lvi.cchTextMax=100;
					SendMessage(l->hwndFrom,LVM_GETITEMW,0,(LPARAM)&lvi);
					lvi.pszText=value;
					lvi.cchTextMax=1024;
					lvi.iSubItem=1;
					SendMessage(l->hwndFrom,LVM_GETITEMW,0,(LPARAM)&lvi);
					SetDlgItemTextW(hwndDlg,IDC_NAME,key);
					SetDlgItemTextW(hwndDlg,IDC_VALUE,value);
					sel = n;
					EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),TRUE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),TRUE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),TRUE);
				}
				if(lv->uOldState & LVIS_SELECTED) {
					int n = lv->iItem;
					sel = -1;
					SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
					SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
					EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
				}
			}
		}
		break;
		*/
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
			case IDOK:
				{
					//Info * info = (Info*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
					//info->Save(hwndDlg);
				}
				break;
				/*
			case IDC_NAME:
			case IDC_VALUE:
				if(HIWORD(wParam) == EN_CHANGE && sel>=0) {
					wchar_t key[100] = {0};
					wchar_t value[1024] = {0};
					LVITEMW lvi={LVIF_TEXT,sel,0};
					GetDlgItemTextW(hwndDlg,IDC_NAME,key,100);
					GetDlgItemTextW(hwndDlg,IDC_VALUE,value,1024);
					lvi.pszText=key;
					lvi.cchTextMax=100;
					SendMessage(GetDlgItem(hwndDlg,IDC_LIST),LVM_SETITEMW,0,(LPARAM)&lvi);
					lvi.pszText=value;
					lvi.cchTextMax=1024;
					lvi.iSubItem=1;
					SendMessage(GetDlgItem(hwndDlg,IDC_LIST),LVM_SETITEMW,0,(LPARAM)&lvi);
					WMTagToWinampTag(key,100);
					ismychange=1;
					SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)key,(WPARAM)value);
					ismychange=0;
				}
				else if(HIWORD(wParam) == EN_KILLFOCUS && sel>=0) {
					wchar_t key[100] = {0};
					wchar_t value[1024] = {0};
					GetDlgItemTextW(hwndDlg,IDC_NAME,key,100);
					GetDlgItemTextW(hwndDlg,IDC_VALUE,value,1024);
					Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					wchar_t oldkey[100]=L"";
					bool newitem=true;
					if(sel < info->GetNumMetadataItems()) {
						info->EnumMetadata(sel,oldkey,100,0,0);
						newitem=false;
					}

					if(!newitem && wcscmp(oldkey,key)) { // key changed
						info->SetTag(sel,key);
					} else {
						info->SetMetadata(key,value);
					}
					WMTagToWinampTag(key,100);
					ismychange=1;
					SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)key,(WPARAM)value);
					ismychange=0;
				}
				break;
			case IDC_BUTTON_DEL:
				if(sel >= 0) {
					wchar_t tag[100] = {0};
					GetDlgItemTextW(hwndDlg,IDC_NAME,tag,100);
					SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
					SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
					EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
					Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					if(sel < info->GetNumMetadataItems())
						info->RemoveMetadata(sel);
					ListView_DeleteItem(GetDlgItem(hwndDlg,IDC_LIST),sel);
					sel=-1;
					WMTagToWinampTag(tag,100);
					ismychange=1;
					SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)tag,(WPARAM)L"");
					ismychange=0;
				}
				break;
			case IDC_BUTTON_DELALL:
				ListView_DeleteAllItems(GetDlgItem(hwndDlg,IDC_LIST));
				SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
				SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
				EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
				sel=-1;
				{
					Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					int n = info->GetNumMetadataItems();
					while(n>0) {
						--n;
						wchar_t tag[100] = {0};
						info->EnumMetadata(n,tag,100,0,0);
						WMTagToWinampTag(tag,100);
						ismychange=0;
						SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)tag,(WPARAM)L"");
						ismychange=1;
						info->RemoveMetadata(n);
					}
				}
				break;
			case IDC_BUTTON_ADD:
				{
					HWND hwndlist = GetDlgItem(hwndDlg,IDC_LIST);
					LVITEMW lvi={0,0x7FFFFFF0,0};
					int n = SendMessage(hwndlist,LVM_INSERTITEMW,0,(LPARAM)&lvi);
					ListView_SetItemState(hwndlist,n,LVIS_SELECTED,LVIS_SELECTED);
				}
				break;
				*/
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
		if(n == 0) { // add first pane
			//SetPropW(parent,L"INBUILT_NOWRITEINFO", (HANDLE)1);
			Info *info = new Info(filename);
			if(info->Error())
			{
				delete info;
				return NULL;
			}
			return WASABI_API_CREATEDIALOGPARAMW(IDD_INFO,parent,ChildProc_Advanced,(LPARAM)info);
		}
		return NULL;
	}

};

/*  CUT> we're now using the unified file info dlg. I'll leave this commented out incase we want to do an advanced tab later on.

#define CREATEDIALOGBOX DialogBoxParam
#define CLOSEDIALOGBOX(x) EndDialog(x, 0)
extern WMDRM mod;
enum
{
	AttributeColumn = 0,
	ValueColumn = 1,
};

void FileInfoDialog::FillAttributeList()
{
	attributeList.Clear();
	WORD attrCount = wmInfo->GetNumberAttributes();
	wchar_t attrName[32768] = {0}, value[32768] = {0};
	int pos=0;
	for (WORD i = 0;i != attrCount;i++)
	{
		wmInfo->GetAttribute(i, attrName, 32768, value, 32768);
		//		if (!AttributeInStandardEditor(attrName.c_str()))
		attributeList.InsertItem(pos, (wchar_t *)attrName, 0);
		attributeList.SetItemText(pos, 1, (wchar_t *)value);
		pos++;
	}

	attributeList.AutoColumnWidth(1);
}

void FileInfoDialog::FillEditBoxes()
{
	wchar_t temp[32768] = {0};

	wmInfo->GetAttribute(g_wszWMAuthor, temp, 32768);
	SetWindowText(GetDlgItem(fileInfoHWND, IDC_EDIT_ARTIST), temp);

	wmInfo->GetAttribute(g_wszWMTitle, temp, 32768);
	SetWindowText(GetDlgItem(fileInfoHWND, IDC_EDIT_TITLE), temp);

	wmInfo->GetAttribute(g_wszWMAlbumTitle, temp, 32768);
	SetWindowText(GetDlgItem(fileInfoHWND, IDC_EDIT_ALBUM), temp);

	wmInfo->GetAttribute(g_wszWMDescription, temp, 32768);
	SetWindowText(GetDlgItem(fileInfoHWND, IDC_EDIT_COMMENTS), temp);

	wmInfo->GetAttribute(g_wszWMGenre, temp, 32768);
	SetWindowText(GetDlgItem(fileInfoHWND, IDC_EDIT_GENRE), temp);

	wmInfo->GetAttribute(g_wszWMYear, temp, 32768);
	SetWindowText(GetDlgItem(fileInfoHWND, IDC_EDIT_YEAR), temp);

	wmInfo->GetAttribute(g_wszWMTrackNumber, temp, 32768);
	SetWindowText(GetDlgItem(fileInfoHWND, IDC_EDIT_TRACK), temp);

	wmInfo->GetAttribute(g_wszWMPublisher, temp, 32768);
	SetWindowText(GetDlgItem(fileInfoHWND, IDC_EDIT_PUBLISHER), temp);

	wmInfo->GetAttribute(g_wszWMAlbumArtist, temp, 32768);
	SetWindowText(GetDlgItem(fileInfoHWND, IDC_EDIT_ALBUMARTIST), temp);
}

void FileInfoDialog::Init(HWND _hwnd)
{
	fileInfoHWND = _hwnd;

	attributeList.setwnd(GetDlgItem(fileInfoHWND, IDC_METADATALIST));

	attributeList.AddCol(WASABI_API_LNGSTRINGW(IDS_ATTRIBUTE), 150);
	attributeList.AddCol(WASABI_API_LNGSTRINGW(IDS_VALUE), 1);
	attributeList.AutoColumnWidth(1);

	if (fileNameToShow)
		SetWindowText(GetDlgItem(fileInfoHWND, IDC_FILENAME), fileNameToShow);
	else
		SetWindowText(GetDlgItem(fileInfoHWND, IDC_FILENAME), fileName);
	FillEditBoxes();
	FillAttributeList();

	if (wmInfo->NonWritable())
	{
		EnableWindow(GetDlgItem(fileInfoHWND, IDC_APPLY), FALSE);
		EnableWindow(GetDlgItem(fileInfoHWND, IDC_REVERT), FALSE);
		EnableWindow(GetDlgItem(fileInfoHWND, IDOK), FALSE);
		SetDlgItemText(fileInfoHWND, IDCANCEL, WASABI_API_LNGSTRINGW(IDS_CLOSE));
		SendMessage(GetDlgItem(fileInfoHWND, IDC_EDIT_TITLE), EM_SETREADONLY, TRUE, 0);
		SendMessage(GetDlgItem(fileInfoHWND, IDC_EDIT_ARTIST), EM_SETREADONLY, TRUE, 0);
		SendMessage(GetDlgItem(fileInfoHWND, IDC_EDIT_ALBUM), EM_SETREADONLY, TRUE, 0);
		SendMessage(GetDlgItem(fileInfoHWND, IDC_EDIT_COMMENTS), EM_SETREADONLY, TRUE, 0);
		SendMessage(GetDlgItem(fileInfoHWND, IDC_EDIT_GENRE), EM_SETREADONLY, TRUE, 0);
		SendMessage(GetDlgItem(fileInfoHWND, IDC_EDIT_YEAR), EM_SETREADONLY, TRUE, 0);
		SendMessage(GetDlgItem(fileInfoHWND, IDC_EDIT_TRACK), EM_SETREADONLY, TRUE, 0);
		SendMessage(GetDlgItem(fileInfoHWND, IDC_EDIT_PUBLISHER), EM_SETREADONLY, TRUE, 0);
		SendMessage(GetDlgItem(fileInfoHWND, IDC_EDIT_ALBUMARTIST), EM_SETREADONLY, TRUE, 0);
	}
	SetFocus(GetDlgItem(fileInfoHWND, IDCANCEL));
	SendMessage(fileInfoHWND, DM_SETDEFID, GetDlgCtrlID(GetDlgItem(fileInfoHWND, IDCANCEL)),0);
}

void FileInfoDialog::Revert()
{
	FillEditBoxes();
	FillAttributeList();
}

BOOL FileInfoDialog::MetadataList_Notify(NMHDR *header)
{
	switch (header->code)
	{

	case LVN_ITEMCHANGED:
		{
			LPNMLISTVIEW lvNotif = (LPNMLISTVIEW)header;
			if ((lvNotif->uOldState & LVIS_SELECTED)
				&& !(lvNotif->uNewState & LVIS_SELECTED))
			{
				EnableWindow(GetDlgItem(fileInfoHWND, IDC_EDIT),0);
				EnableWindow(GetDlgItem(fileInfoHWND, IDC_DELETE), 0);
			}
			if (lvNotif->uNewState & LVIS_SELECTED)
			{
				if (lvNotif->iItem != -1)
				{
					EnableWindow(GetDlgItem(fileInfoHWND, IDC_EDIT),1);
					EnableWindow(GetDlgItem(fileInfoHWND, IDC_DELETE), 1);
				}

			}
		}
		break;
	}
	return 0;
}

bool FileInfoDialog::AttributeInStandardEditor(const wchar_t *attrName)
{
	return (!wcscmp(attrName, g_wszWMTitle)
		||!wcscmp(attrName, g_wszWMAuthor)
		||!wcscmp(attrName, g_wszWMAlbumTitle)
		||!wcscmp(attrName, g_wszWMDescription)
		||!wcscmp(attrName, g_wszWMGenre)
		||!wcscmp(attrName, g_wszWMYear)
		||!wcscmp(attrName, g_wszWMTrackNumber)
		||!wcscmp(attrName, g_wszWMPublisher)
		|| !wcscmp(attrName, g_wszWMAlbumArtist));
}

void FileInfoDialog::WriteEditBoxHelper(const wchar_t attrName[], DWORD IDC, wchar_t *&temp, int &size)
{
	int thisSize = GetWindowTextLength(GetDlgItem(fileInfoHWND, IDC))+1;
	if (thisSize && thisSize>size)
	{
		if (temp)
			delete[] temp;
		temp = new wchar_t[thisSize];
		size=thisSize;

	}

	GetWindowText(GetDlgItem(fileInfoHWND, IDC), temp, size);
	wmInfo->SetAttribute(attrName, temp);
}

void FileInfoDialog::WriteEditBoxes()
{
	wchar_t *temp=0;
	int thisSize=0;
	int size=0;

	WriteEditBoxHelper(g_wszWMTitle, IDC_EDIT_TITLE, temp, size);
	WriteEditBoxHelper(g_wszWMAuthor, IDC_EDIT_ARTIST, temp, size);
	WriteEditBoxHelper(g_wszWMAlbumTitle, IDC_EDIT_ALBUM, temp, size);
	WriteEditBoxHelper(g_wszWMDescription, IDC_EDIT_COMMENTS, temp, size);
	WriteEditBoxHelper(g_wszWMGenre, IDC_EDIT_GENRE, temp, size);
	WriteEditBoxHelper(g_wszWMYear, IDC_EDIT_YEAR, temp, size);
	WriteEditBoxHelper(g_wszWMTrackNumber, IDC_EDIT_TRACK, temp, size);
	WriteEditBoxHelper(g_wszWMPublisher, IDC_EDIT_PUBLISHER, temp, size);
	WriteEditBoxHelper(g_wszWMAlbumArtist, IDC_EDIT_ALBUMARTIST, temp, size);
}

void FileInfoDialog::WriteAttributeListA()
{
	int attributeTextLength=0, valueTextLength=0;
	char *attribute=0, *value=0;
	size_t numAttrs = attributeList.GetCount();
	for (size_t i=0;i!=numAttrs;i++)
	{
		int textLength;
		textLength = attributeList.GetTextLength(i, 0);
		if (textLength>attributeTextLength)
		{
			if (attribute)
				delete[] attribute;
			attribute = new char[textLength];
			attributeTextLength=textLength;
		}
		attributeList.GetText(i, 0, attribute, attributeTextLength);

		textLength = attributeList.GetTextLength(i, 0);
		if (textLength>valueTextLength)
		{
			if (value)
				delete[] value;
			value = new char[textLength];
			valueTextLength=textLength;
		}
		attributeList.GetText(i, 0, value, valueTextLength);
	}
}

void FileInfoDialog::WriteAttributeList()
{
	int attributeTextLength=0, valueTextLength=0;
	wchar_t *attribute=0, *value=0;
	size_t numAttrs = attributeList.GetCount();
	for (size_t i=0;i!=numAttrs;i++)
	{
		int textLength;
		textLength = attributeList.GetTextLength(i, 0);
		if (textLength>attributeTextLength)
		{
			if (attribute)
				delete[] attribute;
			attribute = new wchar_t[textLength];
			attributeTextLength=textLength;
		}
		attributeList.GetText(i, 0, attribute, attributeTextLength);

		textLength = attributeList.GetTextLength(i, 0);
		if (textLength>valueTextLength)
		{
			if (value)
				delete[] value;
			value = new wchar_t[textLength];
			valueTextLength=textLength;
		}
		attributeList.GetText(i, 0, value, valueTextLength);
	}
}

bool FileInfoDialog::Apply()
{
  edited=true;
	if (!wmInfo->MakeWritable(fileName))
	{
		wchar_t title[64] = {0};
		if (activePlaylist.IsMe(fileName) && mod.playing)
		{
			// TODO: this is a race condition.  we might have stopped in between the above if () and now... 
			int outTime = mod.GetOutputTime();
			winamp.PressStop();
			wmInfo->MakeWritable(fileName);	
			WriteEditBoxes();
			wmInfo->Flush();
			wmInfo->MakeReadOnly(fileName);
			mod.startAtMilliseconds=outTime;
			winamp.PressPlay();
			return true;
		}
		else if (!wmInfo->MakeReadOnly(fileName))
			MessageBox(fileInfoHWND, WASABI_API_LNGSTRINGW(IDS_UNABLE_TO_WRITE_TO_FILE_DOES_FILE_EXIST),
					   WASABI_API_LNGSTRINGW_BUF(IDS_UNABLE_TO_WRITE_TO_FILE,title,64), MB_OK);
		else
			MessageBox(fileInfoHWND, WASABI_API_LNGSTRINGW(IDS_UNABLE_TO_WRITE_TO_FILE_MAY_NOT_HAVE_ACCESS),
					   WASABI_API_LNGSTRINGW_BUF(IDS_UNABLE_TO_WRITE_TO_FILE,title,64), MB_OK);

		return false;
	}

	WriteEditBoxes();
	if (!wmInfo->Flush())
	{
		wchar_t* title = WASABI_API_LNGSTRINGW(IDS_SAVE_FAILED);
		MessageBox(NULL, title, title, MB_OK);
	}

	wmInfo->MakeReadOnly(fileName);
	return true;
}

BOOL FileInfoDialog::OnOk()
{
	if (Apply())
		CLOSEDIALOGBOX(fileInfoHWND);
	return 0;
}

BOOL FileInfoDialog::OnCancel()
{
	CLOSEDIALOGBOX(fileInfoHWND);
	return 0;
}

INT_PTR WINAPI FileInfoDialog::FileInfoProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	FileInfoDialog *fileInfoDialog = (FileInfoDialog *)GetWindowLongPtr(wnd, GWLP_USERDATA);
	switch (msg)
	{
	case WM_NOTIFYFORMAT:
		return NFR_UNICODE;

	case WM_NOTIFY:
		{
			LPNMHDR l = (LPNMHDR)lp;

			if (l->hwndFrom == GetDlgItem(wnd, IDC_METADATALIST))
				return fileInfoDialog->MetadataList_Notify(l);
		}
		break;

	case WM_INITDIALOG:
		SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)lp);
		fileInfoDialog = (FileInfoDialog *)lp;
		fileInfoDialog->Init(wnd);
		return FALSE;
	case WM_DESTROY:
		if (fileInfoDialog)
		{
			//delete fileInfoDialog;
			//fileInfoDialog=0;
			SetWindowLongPtr(wnd, GWLP_USERDATA, 0);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wp))
		{
		case IDC_REVERT:
			fileInfoDialog->Revert();
			break;
		case IDCANCEL:
			return fileInfoDialog->OnCancel();
			break;
		case IDOK:
			return fileInfoDialog->OnOk();
			break;
		case IDC_APPLY:
			fileInfoDialog->Apply();
			break;
		}
		break;
	}

	return 0;
}

FileInfoDialog::FileInfoDialog(HINSTANCE _hInstance, HWND parent,const wchar_t *_fileName)
: wmInfo(0),hInstance(_hInstance),fileName(0),fileNameToShow(0), edited(false)
{
	if (activePlaylist.IsMe(_fileName))
	{
		fileName = _wcsdup(activePlaylist.GetFileName());
		fileNameToShow = _wcsdup(_fileName);
	}
	else
		fileName = _wcsdup(_fileName);
	
	wmInfo = new WMInformation(fileName);
	CREATEDIALOGBOX(hInstance,   MAKEINTRESOURCE(IDD_FILEINFO),   parent,   FileInfoProc,   (LPARAM)this);
}

FileInfoDialog::~FileInfoDialog()
{
	delete wmInfo;
	wmInfo=0;
	free(fileName);
	free(fileNameToShow);
}

bool FileInfoDialog::WasEdited()
{
  return edited;
}
*/