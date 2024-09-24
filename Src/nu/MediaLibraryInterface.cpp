#include "MediaLibraryInterface.h"
#include <commctrl.h>
#include "../nu/ns_wc.h"
#include "../winamp/wa_ipc.h"
#include "../Agave/Language/api_language.h"
#define _ML_HEADER_IMPMLEMENT
#include "..\Plugins\General\gen_ml/ml_ipc_0313.h"
#undef _ML_HEADER_IMPMLEMENT
#include "..\Plugins\General\gen_ml/ml_imageloader.h"
#include <shlwapi.h>
#include <strsafe.h>
MediaLibraryInterface mediaLibrary;

void MediaLibraryInterface::AddTreeItem(MLTREEITEM &newItem)
{
	SendMessage(library, WM_ML_IPC, (WPARAM) &newItem, ML_IPC_TREEITEM_ADD);
}

void MediaLibraryInterface::AddTreeItem(MLTREEITEMW &newItem)
{
	SendMessage(library, WM_ML_IPC, (WPARAM) &newItem, ML_IPC_TREEITEM_ADDW);
}

int MediaLibraryInterface::AddTreeImage(int resourceId)
{
	return AddTreeImage(resourceId, -1, (BMPFILTERPROC)FILTER_DEFAULT1);
}

int MediaLibraryInterface::AddTreeImage(int resourceId, int imageIndex, BMPFILTERPROC filter)
{
	MLTREEIMAGE img = {instance, resourceId, imageIndex, filter, 0, 0};
	return (int)(INT_PTR)SendMessage(library, WM_ML_IPC, (WPARAM) &img, ML_IPC_TREEIMAGE_ADD);
}

int MediaLibraryInterface::AddTreeImageBmp(int resourceId)
{
	HMLIMGLST       hmlilNavigation = MLNavCtrl_GetImageList(library);
	MLIMAGESOURCE   mlis            = {sizeof(MLIMAGESOURCE),0};

	MLIMAGELISTITEM item            = {0};
	item.cbSize                     = sizeof( MLIMAGELISTITEM );
	item.hmlil                      = hmlilNavigation;
	item.filterUID                  = MLIF_FILTER1_UID;
	item.pmlImgSource               = &mlis;

	mlis.hInst                      = instance;
	mlis.bpp                        = 24;
	mlis.lpszName                   = MAKEINTRESOURCEW(resourceId);
	mlis.type                       = SRC_TYPE_BMP;
	mlis.flags                      = ISF_FORCE_BPP;

	return MLImageList_Add(library, &item);
}

void MediaLibraryInterface::SetTreeItem(MLTREEITEM &item)
{
	MLTREEITEMINFO ii = {0};
	ii.item           = item;
	ii.mask           = MLTI_IMAGE | MLTI_CHILDREN | MLTI_TEXT | MLTI_ID;

	SendMessage(library, WM_ML_IPC, (WPARAM) &ii, ML_IPC_TREEITEM_SETINFO);
}

void MediaLibraryInterface::SetTreeItem(MLTREEITEMW &item)
{
	MLTREEITEMINFOW ii = {0};
	ii.item            = item;
	ii.mask            = MLTI_IMAGE | MLTI_CHILDREN | MLTI_TEXT | MLTI_ID;

	SendMessage(library, WM_ML_IPC, (WPARAM) &ii, ML_IPC_TREEITEM_SETINFOW);
}

void MediaLibraryInterface::InsertTreeItem(MLTREEITEM &newItem)
{
	SendMessage(library, WM_ML_IPC, (WPARAM) &newItem, ML_IPC_TREEITEM_INSERT);
}

void MediaLibraryInterface::InsertTreeItem(MLTREEITEMW &newItem)
{
	SendMessage(library, WM_ML_IPC, (WPARAM) &newItem, ML_IPC_TREEITEM_INSERTW);
}

void MediaLibraryInterface::RemoveTreeItem(INT_PTR treeId)
{
	SendMessage(library, WM_ML_IPC, (WPARAM)treeId, ML_IPC_DELTREEITEM);
}

void MediaLibraryInterface::SelectTreeItem(INT_PTR treeId)
{
	SendMessage(library, WM_ML_IPC, (WPARAM)treeId, ML_IPC_SETCURTREEITEM);
}

INT_PTR MediaLibraryInterface::GetSelectedTreeItem(void)
{
	return (INT_PTR)SendMessage(library, WM_ML_IPC, 0, ML_IPC_GETCURTREEITEM);
}

void MediaLibraryInterface::UpdateTreeItem(MLTREEITEMINFO &newItem)
{
	SendMessage(library, WM_ML_IPC, (WPARAM) &newItem, ML_IPC_TREEITEM_SETINFO);
}

void MediaLibraryInterface::UpdateTreeItem(MLTREEITEMINFOW &newItem)
{
	SendMessage(library, WM_ML_IPC, (WPARAM) &newItem, ML_IPC_TREEITEM_SETINFOW);
}

int MediaLibraryInterface::SkinList(HWND list)
{
	return (int)(INT_PTR)SendMessage(library, WM_ML_IPC, (WPARAM)list, ML_IPC_SKIN_LISTVIEW);
}

void MediaLibraryInterface::UnskinList(int token)
{
	SendMessage(library, WM_ML_IPC, (WPARAM)token, ML_IPC_UNSKIN_LISTVIEW);
}

void MediaLibraryInterface::ListViewShowSort(int token, BOOL show)
{
	LV_SKIN_SHOWSORT lvs = {0};
	lvs.listView         = token;
	lvs.showSort         = show;

	SendMessage(library, WM_ML_IPC, (WPARAM)&lvs, ML_IPC_LISTVIEW_SHOWSORT);
}

void MediaLibraryInterface::ListViewSort(int token, int columnIndex, BOOL ascending)
{
	LV_SKIN_SORT lvs = {0};
	lvs.listView     = token;
	lvs.ascending    = ascending;
	lvs.columnIndex  = columnIndex;

	SendMessage(library, WM_ML_IPC, (WPARAM)&lvs, ML_IPC_LISTVIEW_SORT);
}

void MediaLibraryInterface::ListSkinUpdateView(int listSkin)
{
	SendMessage(library, WM_ML_IPC, listSkin, ML_IPC_LISTVIEW_UPDATE);
}

void *MediaLibraryInterface::GetWADLGFunc(int num)
{
	return (void *)SendMessage(library, WM_ML_IPC, (WPARAM)num, ML_IPC_SKIN_WADLG_GETFUNC);
}

void MediaLibraryInterface::PlayStream(wchar_t *url, bool force)
{
	wchar_t temp[ 2048 ] = { 0 };
	wchar_t *end = 0;
	StringCchCopyExW( temp, 2047, url, &end, 0, 0 );
	if ( end )
	{
		end[ 1 ] = 0; // double null terminate

		mlSendToWinampStruct send;
		send.type    = ML_TYPE_STREAMNAMESW;
		send.enqueue = force ? ( 0 | 2 ) : 0;
		send.data    = (void *)temp;

		SendMessage( library, WM_ML_IPC, (WPARAM)&send, ML_IPC_SENDTOWINAMP );
	}
}

void MediaLibraryInterface::PlayStreams(std::vector<const wchar_t*> &urls, bool force)
{
	size_t totalSize = 0;
	std::vector<const wchar_t*>::iterator itr;
	for (itr = urls.begin();itr != urls.end();itr++)
	{
		totalSize += lstrlenW(*itr) + 1;
	}
	totalSize++;
	wchar_t *sendTo = new wchar_t[totalSize];
	wchar_t *ptr = sendTo;
	for (itr = urls.begin();itr != urls.end();itr++)
	{
		//AutoChar narrow(*itr);
		StringCchCopyExW(ptr, totalSize, *itr, &ptr, &totalSize, 0);

		ptr++;
		if (totalSize)
			totalSize--;
		else
			break;
	}
	ptr[0] = 0;

	mlSendToWinampStruct send;
	send.type = ML_TYPE_STREAMNAMESW;
	send.enqueue = force?(0 | 2):0;
	send.data = sendTo;
	SendMessage(library, WM_ML_IPC, (WPARAM)&send, ML_IPC_SENDTOWINAMP);
	delete [] sendTo;
}

void MediaLibraryInterface::EnqueueStream(wchar_t *url, bool force)
{
	wchar_t temp[2048] = {0};
	wchar_t *end=0;
	StringCchCopyExW(temp, 2047, url, &end, 0, 0);
	if (end)
	{
		end[1]=0; // double null terminate

		mlSendToWinampStruct send;
		send.type    = ML_TYPE_STREAMNAMESW;
		send.enqueue = force?(1 | 2):1;
		send.data    = (void *)temp;

		SendMessage(library, WM_ML_IPC, (WPARAM)&send, ML_IPC_SENDTOWINAMP);
	}
}

int MediaLibraryInterface::SkinComboBox(HWND comboBox)
{
	return (int)(INT_PTR)SendMessage(library, WM_ML_IPC, (WPARAM)comboBox, ML_IPC_SKIN_COMBOBOX);
}

void MediaLibraryInterface::UnskinComboBox(int token)
{
	SendMessage(library, WM_ML_IPC, (WPARAM)token, ML_IPC_UNSKIN_COMBOBOX);
}

const char *MediaLibraryInterface::GetIniDirectory()
{
	if (!iniDirectory)
	{
		iniDirectory = (const char*)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETINIDIRECTORY);
		if (((unsigned int)(ULONG_PTR)iniDirectory) < 65536)
			iniDirectory=0;
	}
	return iniDirectory;
}

const wchar_t *MediaLibraryInterface::GetIniDirectoryW()
{
	if (!iniDirectoryW)
	{
		iniDirectoryW = (const wchar_t*)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETINIDIRECTORYW);
		if (((unsigned int)(ULONG_PTR)iniDirectoryW) < 65536)
			iniDirectoryW=0;
	}
	return iniDirectoryW;
}

void MediaLibraryInterface::BuildPath(const wchar_t *pathEnd, wchar_t *path, size_t numChars)
{
	PathCombineW(path, GetIniDirectoryW(), pathEnd);
}

void MediaLibraryInterface::AddToSendTo(char description[], INT_PTR context, INT_PTR unique)
{
	mlAddToSendToStruct s;
	s.context = context;
	s.desc    = description;
	s.user32  = unique;

	SendMessage(library, WM_ML_IPC, (WPARAM)&s, ML_IPC_ADDTOSENDTO);
}

void MediaLibraryInterface::AddToSendTo(wchar_t description[], INT_PTR context, INT_PTR unique)
{
	mlAddToSendToStructW s;
	s.context = context;
	s.desc    = description;
	s.user32  = unique;

	SendMessage(library, WM_ML_IPC, (WPARAM)&s, ML_IPC_ADDTOSENDTOW);
}

void MediaLibraryInterface::BranchSendTo(INT_PTR context)
{
	mlAddToSendToStructW s = {0};
	s.context = context;

	SendMessage(library, WM_ML_IPC, (WPARAM)&s, ML_IPC_BRANCHSENDTO);
}

void MediaLibraryInterface::AddToBranchSendTo(const wchar_t description[], INT_PTR context, INT_PTR unique)
{
	mlAddToSendToStructW s;
	s.context = context;
	s.desc    = const_cast<wchar_t *>(description);
	s.user32  = unique;

	SendMessage(library, WM_ML_IPC, (WPARAM)&s, ML_IPC_ADDTOBRANCH);
}

void MediaLibraryInterface::EndBranchSendTo(const wchar_t description[], INT_PTR context)
{
	mlAddToSendToStructW s = {0};
	s.context              = context;
	s.desc                 = const_cast<wchar_t *>(description);

	SendMessage(library, WM_ML_IPC, (WPARAM)&s, ML_IPC_BRANCHSENDTO);
}

void MediaLibraryInterface::PlayFile(const wchar_t *url)
{
	wchar_t temp[2048] = {0};
	wchar_t *end=0;
	StringCchCopyExW(temp, 2047, url, &end, 0, 0);
	if (end)
	{
		end[1]=0; // double null terminate

		mlSendToWinampStruct send;
		send.type    = ML_TYPE_FILENAMESW;
		send.enqueue = 0 | 2;
		send.data    = (void *)temp;

		SendMessage(library, WM_ML_IPC, (WPARAM)&send, ML_IPC_SENDTOWINAMP);
	}
}

void MediaLibraryInterface::EnqueueFile(const wchar_t *url)
{
	wchar_t temp[2048] = {0};
	wchar_t *end=0;
	StringCchCopyExW(temp, 2047, url, &end, 0, 0);
	if (end)
	{
		end[1]=0; // double null terminate

		mlSendToWinampStruct send;
		send.type    = ML_TYPE_FILENAMESW;
		send.enqueue = 1 | 2;
		send.data    = (void *)temp;

		SendMessage(library, WM_ML_IPC, (WPARAM)&send, ML_IPC_SENDTOWINAMP);
	}
}

void MediaLibraryInterface::BuildPluginPath(const TCHAR *filename, TCHAR *path, size_t pathSize)
{
	if (!pluginDirectory)
		pluginDirectory = (const char *)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORY);
#ifdef UNICODE
	StringCchPrintf(path, pathSize, L"%S\\%s", pluginDirectory, filename);
#else
	StringCchPrintf(path, pathSize, "%s\\%s", pluginDirectory, filename);
#endif
}

void MediaLibraryInterface::AddToMediaLibrary(const char *filename)
{
	LMDB_FILE_ADD_INFO fi = {const_cast<char *>(filename), -1, -1};
	SendMessage(library, WM_ML_IPC, (WPARAM)&fi, ML_IPC_DB_ADDORUPDATEFILE);
	PostMessage(library, WM_ML_IPC, 0, ML_IPC_DB_SYNCDB);
}

void MediaLibraryInterface::AddToMediaLibrary(const wchar_t *filename)
{
	LMDB_FILE_ADD_INFOW fi = {const_cast<wchar_t *>(filename), -1, -1};
	SendMessage(library, WM_ML_IPC, (WPARAM)&fi, ML_IPC_DB_ADDORUPDATEFILEW);
	PostMessage(library, WM_ML_IPC, 0, ML_IPC_DB_SYNCDB);
}

IDispatch *MediaLibraryInterface::GetDispatchObject()
{
	IDispatch *dispatch = (IDispatch *)SendMessage(winamp, WM_WA_IPC, 0, IPC_GET_DISPATCH_OBJECT);
	if (dispatch == (IDispatch *)1)
		return 0;
	else
		return dispatch;
}

int MediaLibraryInterface::GetUniqueDispatchId()
{
	return (int)(INT_PTR)SendMessage(winamp, WM_WA_IPC, 0, IPC_GET_UNIQUE_DISPATCH_ID);
}

void MediaLibraryInterface::ListSkinDisableHorizontalScrollbar(int listSkin)
{
	SendMessage(library, WM_ML_IPC, listSkin, ML_IPC_LISTVIEW_DISABLEHSCROLL);
}

DWORD MediaLibraryInterface::AddDispatch(wchar_t *name, IDispatch *object)
{
	DispatchInfo dispatchInfo;
	dispatchInfo.name     = name;
	dispatchInfo.dispatch = object;

	if (SendMessage(winamp, WM_WA_IPC, (WPARAM)&dispatchInfo, IPC_ADD_DISPATCH_OBJECT) == 0)
		return dispatchInfo.id;
	else
		return 0;
}

void MediaLibraryInterface::GetFileInfo(const char *filename, char *title, int titleCch, int *length)
{
	basicFileInfoStruct infoStruct = {0};
	infoStruct.filename = filename;
	infoStruct.title    = title;
	infoStruct.titlelen = titleCch;

	SendMessage(winamp, WM_WA_IPC, (WPARAM)&infoStruct, IPC_GET_BASIC_FILE_INFO);

	*length = infoStruct.length;
}

void MediaLibraryInterface::GetFileInfo(const wchar_t *filename, wchar_t *title, int titleCch, int *p_length)
{
	if (filename)
	{
		basicFileInfoStructW infoStruct = {0};
		infoStruct.filename = filename;
		infoStruct.title    = title;
		infoStruct.titlelen = titleCch;

		SendMessage(winamp, WM_WA_IPC, (WPARAM)&infoStruct, IPC_GET_BASIC_FILE_INFOW);

		if ( p_length != NULL )
			*p_length = infoStruct.length;
	}
	else
	{
		title[0] = 0;
		*p_length  = -1;
	}
}

const char *MediaLibraryInterface::GetWinampIni()
{
	if (winampIni && *winampIni)
		return winampIni;

	winampIni = (const char *)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETINIFILE);

	return winampIni;
}

const wchar_t *MediaLibraryInterface::GetWinampIniW()
{
	if (winampIniW && *winampIniW)
		return winampIniW;

	winampIniW = (const wchar_t *)SendMessage(winamp, WM_WA_IPC, 0, IPC_GETINIFILEW);

	return winampIniW;
}

INT_PTR MediaLibraryInterface::GetChildId(INT_PTR id)
{
	return SendMessage(library, WM_ML_IPC, id, ML_IPC_TREEITEM_GETCHILD_ID);
}

INT_PTR MediaLibraryInterface::GetNextId(INT_PTR id)
{
	return SendMessage(library, WM_ML_IPC, id, ML_IPC_TREEITEM_GETNEXT_ID);
}

void MediaLibraryInterface::RenameTreeId(INT_PTR treeId, const wchar_t *newName)
{
	MLTREEITEMINFOW info = {0};
	info.mask            = MLTI_TEXT;
	info.item.size       = sizeof(info.item);
	info.item.id         = treeId;
	info.item.title      = const_cast<wchar_t *>(newName);

	SendMessage(library, WM_ML_IPC, (WPARAM) &info, ML_IPC_TREEITEM_SETINFOW);
}