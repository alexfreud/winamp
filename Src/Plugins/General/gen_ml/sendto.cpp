#include "main.h"
#include <windowsx.h>
#include "resource.h"
#include "config.h"
#include "sendto.h"
#include "api__gen_ml.h"
#include "../nu/AutoWideFn.h"
#include "../Winamp/strutil.h"

SendToMenu::SendToMenu()
{
	activePlaylist=0;
	m_addtolibrary=0;
	_hm=0;
	branch=0;
	branch_pos=0;
	_pos=0;
	_len=0;
	_start=0;
	m_start=0;
	m_len=0;
	plugin_start=0;
	plugin_len=0;
}

SendToMenu::~SendToMenu()
{
}

void SendToMenu::onAddItem(mlAddToSendToStruct *ptr)
{
	if (--_len < 0)
		return;

	MENUITEMINFOA mii= {sizeof(MENUITEMINFOA),};

	if (ptr->desc && *ptr->desc == '-')
	{
		// cannot insert a seperator at the top
		if (_pos <= 2) return;
		mii.fMask = MIIM_TYPE;
		mii.fType = MFT_SEPARATOR;
	}
	else
	{
		mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_DATA | MIIM_STATE;
		mii.fType = MFT_STRING;
		if (ptr->desc && *ptr->desc == '#')
		{
			mii.fState = MFS_GRAYED;
			mii.dwTypeData = ptr->desc+1;
		}
		else
		{
			mii.fState = MFS_ENABLED;
			mii.dwTypeData = ptr->desc;
		}
		mii.wID = _start++;
		mii.dwItemData = ptr->user32;
		mii.cch = (UINT)strlen(mii.dwTypeData);
	}
	InsertMenuItemA(_hm,_pos++,TRUE,&mii);
}

void SendToMenu::onAddItem(mlAddToSendToStructW *ptr)
{
	if (--_len < 0)
		return;

	MENUITEMINFOW mii= {sizeof(MENUITEMINFOW),};

	if (ptr->desc && *ptr->desc == L'-')
	{
		// cannot insert a seperator at the top
		if (_pos <= 2) return;
		mii.fMask = MIIM_TYPE;
		mii.fType = MFT_SEPARATOR;
	}
	else
	{
		mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_DATA | MIIM_STATE;
		mii.fType = MFT_STRING;
		if (ptr->desc && *ptr->desc == L'#')
		{
			mii.fState = MFS_GRAYED;
			mii.dwTypeData = ptr->desc+1;
		}
		else
		{
			mii.fState = MFS_ENABLED;
			mii.dwTypeData = ptr->desc;
		}
		mii.wID = _start++;
		mii.dwItemData = ptr->user32;
		mii.cch = (UINT)wcslen(mii.dwTypeData);
	}
	InsertMenuItemW(_hm,_pos++,TRUE,&mii);
}

void SendToMenu::addItemToBranch(mlAddToSendToStructW *ptr)
{
	if (!branch)
		return;

	if (--_len < 0)
		return;

	MENUITEMINFOW mii= {sizeof(MENUITEMINFOW),};

	if (ptr->desc && *ptr->desc == L'-')
	{
		// cannot insert a seperator at the top
		if (_pos <= 2) return;
		mii.fMask = MIIM_TYPE;
		mii.fType = MFT_SEPARATOR;
	}
	else
	{
		mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_DATA | MIIM_STATE;
		mii.fType = MFT_STRING;
		if (ptr->desc && *ptr->desc == L'#')
		{
			mii.fState = MFS_GRAYED;
			mii.dwTypeData = ptr->desc+1;
		}
		else
		{
			mii.fState = MFS_ENABLED;
			mii.dwTypeData = ptr->desc;
		}
		mii.wID = _start++;
		mii.dwItemData = ptr->user32;
		mii.cch = (UINT)wcslen(mii.dwTypeData);
	}
	InsertMenuItemW(branch,branch_pos++,TRUE,&mii);
}

void SendToMenu::startBranch()
{
	branch=CreateMenu();
	branch_pos=0;
}

void SendToMenu::endBranch(const wchar_t *name)
{
	MENUITEMINFOW mii=
	{
		sizeof(MENUITEMINFOW),
		MIIM_TYPE|MIIM_DATA|MIIM_SUBMENU,
		MFT_STRING,
		MFS_ENABLED,
		0,
		branch,
		NULL,
		NULL,
		0,
		(LPWSTR)name,
	};
	mii.cch= (UINT)wcslen(mii.dwTypeData);
	InsertMenuItemW(_hm,_pos++,TRUE,&mii);
	branch=0;
}

void SendToMenu::buildmenu(HMENU hMenu, int type, int simple, int true_type, int start, int len)
{
	_start=start;
	_len=len;
	_hm=hMenu;
	_pos=0;
	m_start=_start;
	plugin_start=0;

	while (DeleteMenu(hMenu,0,MF_BYPOSITION));

	if (type == ML_TYPE_ITEMRECORDLIST || type == ML_TYPE_FILENAMES ||
		type == ML_TYPE_ITEMRECORDLISTW || type == ML_TYPE_FILENAMESW ||
		type == ML_TYPE_STREAMNAMES || type == ML_TYPE_CDTRACKS)
	{
		activePlaylist=_start++; _len--;

		// hardcode playlists :)
		MENUITEMINFOW mii=
		{
			sizeof(MENUITEMINFO),
			MIIM_TYPE|MIIM_ID,
			MFT_STRING,
			MFS_ENABLED,
			(UINT)activePlaylist,
			NULL,
			NULL,
			NULL,
			0,
		};

		if(simple == FALSE)
		{
			mii.dwTypeData = WASABI_API_LNGSTRINGW(IDS_ENQUEUE_IN_WINAMP);
			mii.cch = (UINT)wcslen(mii.dwTypeData);
			InsertMenuItemW(hMenu,_pos++,TRUE,&mii);

			mii.fType = MFT_SEPARATOR;
			mii.wID=0;
			InsertMenuItemW(hMenu,_pos++,TRUE,&mii);
		}
	}

	plugin_start=_start;
	plugin_SendMessage(ML_MSG_ONSENDTOBUILD,type,reinterpret_cast<INT_PTR>(this),(true_type-1));
	plugin_len=_start - plugin_start;
	m_len=_start-m_start;
}

int SendToMenu::isourcmd(int id)
{
	return id >= m_start && id < m_start+m_len;
}

void TAG_FMT_EXT(const wchar_t *filename, void *f, void *ff, void *p, wchar_t *out, int out_len, int extended);
wchar_t *itemrecordTagFunc(wchar_t *tag, void * p);
wchar_t *itemrecordWTagFunc(wchar_t *tag, void * p);
void fieldTagFuncFree(wchar_t * tag, void * p);

int SendToMenu::handlecmd(HWND hwndParent, int id, int type, void *data)
{
	if (!isourcmd(id)) 
		return 0;

	if (plugin_start && id >= plugin_start && id < plugin_start+plugin_len)
	{
		MENUITEMINFO i={sizeof(i),MIIM_DATA,};
		GetMenuItemInfo(_hm,id,FALSE,&i);
		return (INT)plugin_SendMessage(ML_MSG_ONSENDTOSELECT,type,reinterpret_cast<INT_PTR>(data),i.dwItemData);
	}
	else if (activePlaylist && id == activePlaylist)
	{
		if (type == ML_TYPE_FILENAMES || type == ML_TYPE_STREAMNAMES)
		{
			char *ptr=(char*)data;

			while (ptr && *ptr)
			{
				COPYDATASTRUCT cds;
				cds.dwData = IPC_PLAYFILE;
				cds.lpData = (void *) ptr;
				cds.cbData = (DWORD)strlen((char *)cds.lpData)+1; // include space for null char
				SendMessage(plugin.hwndParent,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
				ptr+=strlen(ptr)+1;
			}
			return 1;
		}
		else if (type == ML_TYPE_FILENAMESW || type == ML_TYPE_STREAMNAMESW)
		{
			wchar_t *ptr=(wchar_t*)data;

			while (ptr && *ptr)
			{
				COPYDATASTRUCT cds;
				cds.dwData = IPC_PLAYFILEW;
				cds.lpData = (void *) ptr;
				cds.cbData = (DWORD)sizeof(wchar_t) * wcslen((wchar_t *)cds.lpData) + sizeof(wchar_t); // include space for null char
				SendMessage(plugin.hwndParent,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
				ptr+=wcslen(ptr)+1;
			}
			return 1;
		}
		else if (type == ML_TYPE_ITEMRECORDLIST)
		{
			wchar_t title[FILETITLE_SIZE] = {0};

			itemRecordList *obj = (itemRecordList *)data;
			for (int i=0;i < obj->Size;i++)
			{
				AutoWideFn wfn( obj->Items[ i ].filename );

				TAG_FMT_EXT(wfn, itemrecordTagFunc, fieldTagFuncFree, (void*)&obj->Items[i], title, FILETITLE_SIZE, 1);

				enqueueFileWithMetaStructW enqueueFile;
				enqueueFile.filename = wfn;
				enqueueFile.title    = title;
				enqueueFile.ext      = NULL;
				enqueueFile.length   = obj->Items[i].length;
				SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&enqueueFile, IPC_PLAYFILEW);
			}
			return 1;
		}
		else if (type == ML_TYPE_ITEMRECORDLISTW)
		{
			wchar_t title[FILETITLE_SIZE] = {0};

			itemRecordListW *obj = (itemRecordListW *)data;
			for (int i=0;i < obj->Size;i++)
			{
				TAG_FMT_EXT(obj->Items[i].filename, itemrecordWTagFunc, fieldTagFuncFree, (void*)&obj->Items[i], title, FILETITLE_SIZE, 1);
				enqueueFileWithMetaStructW enqueueFile;
				enqueueFile.filename = obj->Items[i].filename;
				enqueueFile.title    = title;
				enqueueFile.ext      = NULL;
				enqueueFile.length   = obj->Items[i].length;
				SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&enqueueFile, IPC_PLAYFILEW);
			}
			return 1;
		}
	}
	return 0;
}