#include "main.h"
#include <time.h>
#include <winuser.h>
#include <assert.h>
#include "./ml.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"
#include "./resource.h"
#include "./comboskin.h"
#include "./childwnd.h"
#include "./sendto.h"
#include "./navigation.h"
#include "../nu/AutoWide.h"
#include "../nu/ns_wc.h"
#include "api__gen_ml.h"

#include "./ml_ipc_0313.h"
#include "./ml_imageloader.h"
#include "./ml_imagefilter.h"
#include "./ml_imagelist.h"
#include "./ml_rating.h"
#include "./ml_ratingcolumn.h"
#include "./ml_cloud.h"
#include "./ml_cloudcolumn.h"
#include "./skinning.h"
#include "./fileview.h"


extern HWND m_curview_hwnd;
extern "C" HWND g_ownerwnd;
int queryEditOther(HWND hwnd, char *query, char *viewname, int mode);

extern C_ItemList m_plugins;
extern HNAVCTRL hNavigation;
extern HMLIMGFLTRMNGR hmlifMngr;
extern HMLIMGLST hmlilRating;
extern HMLIMGLST hmlilCloud;

static wchar_t playBuf[64], enqueueBuf[64];

#define WEBINFO_PREFFERED_HEIGHT		100

#define MAX_MENU_DEPTH		10

typedef struct _NAVMENUBUILDER
{
	HMENU		hMenu[MAX_MENU_DEPTH];
	HNAVITEM	hParent[MAX_MENU_DEPTH];
	INT			level;
	NAVITEM_I	nis;
	INT			maxNum;
	INT			offset;
	INT			counter;
} NAVMENUBUILDER;

static INT GetNavImageIndexFromTag(HMLIMGLST hmlil, INT_PTR tag)
{
	if (!hmlil) return -1;
	
	switch(tag)
	{
		case MLTREEIMAGE_NONE:				
		case MLTREEIMAGE_DEFAULT:			
		case MLTREEIMAGE_BRANCH:				
			return -1;
		case MLTREEIMAGE_BRANCH_EXPANDED:	
		case MLTREEIMAGE_BRANCH_COLLAPSED:
		case MLTREEIMAGE_BRANCH_NOCHILD:		
			break;
	}
	return MLImageListI_GetIndexFromTag(hmlil, tag);
}

static BOOL CALLBACK EnumerateNavItemsCB(HNAVITEM hItem, LPARAM lParam)
{
	NAVMENUBUILDER *pnmb;
	HNAVITEM hParent;
	if (!lParam || !hItem) return FALSE;

	pnmb = (NAVMENUBUILDER*)lParam;
	pnmb->counter++;
	if (pnmb->maxNum > 0  && pnmb->counter == pnmb->maxNum) return FALSE;
	if (!NavItemI_GetIndirect(hItem, &pnmb->nis)) return FALSE;
	if (pnmb->nis.pszText) FixAmpsW(pnmb->nis.pszText, pnmb->nis.cchTextMax);
	
	hParent = NavItemI_GetParent(hItem);
	while (hParent != pnmb->hParent[pnmb->level])
	{
		pnmb->level--;
		if (pnmb->level < 0) return FALSE;
	}

	if (NIS_HASCHILDREN_I & pnmb->nis.style) 
	{
		pnmb->level++;
		if (pnmb->level == MAX_MENU_DEPTH) return FALSE;

		pnmb->hMenu[pnmb->level] = CreatePopupMenu();
		pnmb->hParent[pnmb->level] = hItem;

		// this will add a separator into the parent menu if we have a more than 1 child menu (ml_pmp can do this)
		if(pnmb->level > 1 && GetMenuItemCount(pnmb->hMenu[pnmb->level-1]) == 1) AppendMenuW(pnmb->hMenu[pnmb->level-1], MF_SEPARATOR,  0, 0);
		AppendMenuW(pnmb->hMenu[pnmb->level-1], MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)pnmb->hMenu[pnmb->level], pnmb->nis.pszText);
		AppendMenuW(pnmb->hMenu[pnmb->level], MF_ENABLED | MF_STRING,  pnmb->nis.id + pnmb->offset, pnmb->nis.pszText);
	}
	else
	{
		// if we've got a 'root' item for the current level then add a separator
		if(pnmb->level && GetMenuItemCount(pnmb->hMenu[pnmb->level]) == 1) AppendMenuW(pnmb->hMenu[pnmb->level], MF_SEPARATOR,  0, 0);
		AppendMenuW(pnmb->hMenu[pnmb->level], MF_ENABLED | MF_STRING, pnmb->nis.id + pnmb->offset, pnmb->nis.pszText);
	}

	return TRUE;
}

static INT_PTR getPlRating(HWND hwndML, INT_PTR plindex)
{
	const wchar_t *filename = (const wchar_t *)SendMessage(plugin.hwndParent, WM_WA_IPC, plindex, IPC_GETPLAYLISTFILEW);
	if (!filename) return 0;
	return SendMessage(hwndML, WM_ML_IPC, (WPARAM)filename, ML_IPC_GET_FILE_RATINGW);
}

static INT_PTR SetPlRating(HWND hwndML, int plindex, int rating)
{
	const wchar_t *filename = (const wchar_t *)SendMessage(plugin.hwndParent, WM_WA_IPC, plindex, IPC_GETPLAYLISTFILEW);
	if (!filename) return 0;
	file_set_ratingW data;
	data.fileName = filename;
	data.newRating = rating;
	return (INT_PTR)SendMessage(hwndML, WM_ML_IPC, (WPARAM)&data, ML_IPC_SET_FILE_RATINGW);
}

INT_PTR IPC_PL_SetRating(HWND hwndML, INT_PTR param)
{
	pl_set_rating *psr = (pl_set_rating *)param;
	if (psr) return SetPlRating(hwndML, psr->plentry, psr->rating);
	return 0;
}

static INT_PTR IPC_InsertView(UINT msg, INT_PTR param)
{
	INT insertAfterId = 0, parentId = 0;
	BOOL converted = FALSE;
	HNAVITEM hItem = 0, hParentItem = 0, hInsertAfter = 0;
	NAVITEM_I nis = {0};

	switch(msg)
	{
		case ML_IPC_TREEITEM_ADD:
		case ML_IPC_TREEITEM_INSERT:
			if (((MLTREEITEM*)param)->title)
			{
				nis.pszText = AutoWideDup(((MLTREEITEM*)param)->title);
				converted = TRUE;
			}
		case ML_IPC_TREEITEM_INSERTW:
		case ML_IPC_TREEITEM_ADDW:
			if (ML_IPC_TREEITEM_ADD == msg || ML_IPC_TREEITEM_ADDW == msg)
				nis.id = (INT)((MLTREEITEMW*)param)->id;
			else
				insertAfterId= (INT)((MLTREEITEMW*)param)->id;

			if (!converted)
				nis.pszText = ((MLTREEITEMW*)param)->title;

			parentId    = (INT)((MLTREEITEMW*)param)->parentId;
			nis.style  |= (((MLTREEITEMW*)param)->hasChildren) ? NIS_HASCHILDREN_I : 0;
			nis.iImage  = ((MLTREEITEMW*)param)->imageIndex;
			break;
		case ML_IPC_ADDTREEITEM_EX:
			nis.iImage = ((mlAddTreeItemStructEx*)param)->imageIndex;
		case ML_IPC_ADDTREEITEM:
			if (((mlAddTreeItemStruct*)param)->title)
			{
				nis.pszText = AutoWideDup(((mlAddTreeItemStruct*)param)->title);
				converted = TRUE;
			}
			parentId= (INT)((mlAddTreeItemStruct*)param)->parent_id;
			nis.style |= (((mlAddTreeItemStruct*)param)->has_children) ? NIS_HASCHILDREN_I : 0;
			break;
	}

	nis.iImage = GetNavImageIndexFromTag(NavCtrlI_GetImageList(hNavigation), nis.iImage);
	nis.iSelectedImage = nis.iImage;
	nis.styleMask |= NIS_HASCHILDREN_I;

	if (nis.id) nis.mask |= NIMF_ITEMID_I;
	if (nis.iImage) nis.mask |= NIMF_IMAGE_I; 
	if (nis.iSelectedImage) nis.mask |= NIMF_IMAGESEL_I; 
	if (nis.style) nis.mask |= NIMF_STYLE_I;
	if (nis.pszText) nis.mask |= NIMF_TEXT_I;
	if (nis.state) nis.mask |= NIMF_STATE_I;

	hInsertAfter = NavCtrlI_FindItem(hNavigation, insertAfterId);
	hParentItem = NavCtrlI_FindItem(hNavigation, parentId);

	hItem = NavCtrlI_InsertItem(hNavigation, hInsertAfter, hParentItem, &nis);

	if (converted && nis.pszText) free(nis.pszText);
	if (hItem)
	{
		if (ML_IPC_ADDTREEITEM ==msg || ML_IPC_ADDTREEITEM_EX == msg)
			((mlAddTreeItemStruct*)param)->this_id = NavItemI_GetId(hItem);
		else
			((MLTREEITEMW*)param)->id = NavItemI_GetId(hItem);
	}
	
	return (INT_PTR)hItem;
}

static BOOL IPC_SetView(int msg, INT_PTR param)
{
	HNAVITEM hItem;
	switch(msg)
	{
		case ML_IPC_SETTREEITEM:
		case ML_IPC_SETTREEITEM_EX:
			hItem = NavCtrlI_FindItem(hNavigation, (INT)((mlAddTreeItemStruct*)param)->this_id);
			if (!hItem) return FALSE;
			if (!NavItemI_SetText(hItem, AutoWide(((mlAddTreeItemStruct*)param)->title)) ||
				!NavItemI_SetStyle(hItem, (((mlAddTreeItemStruct*)param)->has_children) ? NIS_HASCHILDREN_I : 0, NIS_HASCHILDREN_I))
				return FALSE;
			if (ML_IPC_SETTREEITEM_EX == msg) 
			{
				INT mlilIndex = GetNavImageIndexFromTag(NavCtrlI_GetImageList(hNavigation), ((mlAddTreeItemStructEx*)param)->imageIndex);
				if (!NavItemI_SetImageIndex(hItem, mlilIndex, IMAGE_NORMAL_I) || !NavItemI_SetImageIndex(hItem, mlilIndex, IMAGE_SELECTED_I)) return FALSE;
			}
			return TRUE;
		case ML_IPC_TREEITEM_SETINFO:
		case ML_IPC_TREEITEM_SETINFOW:
			hItem = (((MLTREEITEMINFO*)param)->handle) ? 
					(HNAVITEM)((MLTREEITEMINFO*)param)->handle :
					NavCtrlI_FindItem(hNavigation, (INT)((MLTREEITEMINFO*)param)->item.id);
			if (!hItem) return FALSE;
			if (MLTI_TEXT & ((MLTREEITEMINFO*)param)->mask) 
			{
				if (!NavItemI_SetText(hItem, (ML_IPC_TREEITEM_SETINFOW == msg) ? ((MLTREEITEMINFOW*)param)->item.title :
																			AutoWide(((MLTREEITEMINFO*)param)->item.title))) return FALSE;
			}
			if (MLTI_ID & ((MLTREEITEMINFO*)param)->mask &&
				! NavItemI_SetId(hItem, (INT)((MLTREEITEMINFO*)param)->item.id)) return FALSE;
			if (MLTI_CHILDREN & ((MLTREEITEMINFO*)param)->mask &&
				!NavItemI_SetStyle(hItem, (((MLTREEITEMINFO*)param)->item.hasChildren) ? NIS_HASCHILDREN_I : 0, NIS_HASCHILDREN_I)) return FALSE;
			if (MLTI_IMAGE & ((MLTREEITEMINFO*)param)->mask)
			{
				INT mlilIndex = GetNavImageIndexFromTag(NavCtrlI_GetImageList(hNavigation), ((MLTREEITEMINFO*)param)->item.imageIndex);
				if (!NavItemI_SetImageIndex(hItem, mlilIndex, IMAGE_NORMAL_I) || !NavItemI_SetImageIndex(hItem, mlilIndex, IMAGE_SELECTED_I)) return FALSE;
			}
	}
	return FALSE;
}

static BOOL IPC_GetView(INT msg, INT_PTR param)
{
	HNAVITEM hItem;

	hItem = (((MLTREEITEMINFO*)param)->handle) ? 
					(HNAVITEM)((MLTREEITEMINFO*)param)->handle : 
					NavCtrlI_FindItem(hNavigation, (INT)((MLTREEITEMINFO*)param)->item.id);
	if (!hItem) return FALSE;
	
	if (MLTI_ID & ((MLTREEITEMINFO*)param)->mask) ((MLTREEITEMINFO*)param)->item.id  = NavItemI_GetId(hItem);
	if (MLTI_CHILDREN & ((MLTREEITEMINFO*)param)->mask) ((MLTREEITEMINFO*)param)->item.hasChildren  = NavItemI_HasChildren(hItem);
	/// TODO: index <--> tag
	if (MLTI_IMAGE & ((MLTREEITEMINFO*)param)->mask) 
	{
		INT_PTR tag;

		((MLTREEITEMINFO*)param)->item.id =  (MLImageListI_GetTagFromIndex(NavCtrlI_GetImageList(hNavigation),
																			NavItemI_GetImageIndex(hItem, IMAGE_NORMAL_I),
																			&tag)) ? (INT)tag : -1;
	}
	if (MLTI_TEXT & ((MLTREEITEMINFO*)param)->mask) 
	{
		if (ML_IPC_TREEITEM_GETINFO == msg) 
		{
			wchar_t buffer[4096] = {0};
			if (!NavItemI_GetText(hItem, buffer, 4096)) return FALSE;
			if (!WideCharToMultiByte(CP_ACP, 0, buffer, -1, 
										((MLTREEITEMINFO*)param)->item.title, 
										(INT)((MLTREEITEMINFO*)param)->item.titleLen, NULL, NULL)) return FALSE;
		}
		else
		{
			if (!NavItemI_GetText(hItem, ((MLTREEITEMINFOW*)param)->item.title, (INT)((MLTREEITEMINFOW*)param)->item.titleLen)) return FALSE;
		}
	}
	
	return TRUE;
}

static INT_PTR IPC_SendToWinamp(INT_PTR param)
{
	mlSendToWinampStruct *p = (mlSendToWinampStruct*)param;
	if (p->data)
	{
		int enq = p->enqueue & 1;
		if (!(p->enqueue & 2) && g_config->ReadInt(L"enqueuedef", 0)) enq ^= 1;

		switch (p->type)
		{
			case ML_TYPE_ITEMRECORDLIST:
			case ML_TYPE_CDTRACKS:
				main_playItemRecordList((itemRecordList*)p->data, enq);
				break;
			case ML_TYPE_ITEMRECORDLISTW:
				main_playItemRecordListW((itemRecordListW *)p->data, enq);
				break;
			case ML_TYPE_STREAMNAMES:
			case ML_TYPE_FILENAMES:
			{
				char *list = (char*)p->data;
				int cnt = 0;
				if (list) while (*list)
					{
						if (!cnt && !enq) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_DELETE);
						cnt++;

						COPYDATASTRUCT cds;
						cds.dwData = IPC_PLAYFILE;
						cds.lpData = (void *) list;
						cds.cbData =(INT)strlen((char *) cds.lpData) + 1; // include space for null char
						SendMessage(plugin.hwndParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);

						list += strlen(list) + 1;
					}
				if (cnt && !enq) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
			}
			break;
			case ML_TYPE_STREAMNAMESW:
			case ML_TYPE_FILENAMESW:
			{
				wchar_t *list = (wchar_t*)p->data;
				int cnt = 0;
				if ( list )
				{
					while ( *list )
					{
						if ( !cnt && !enq ) SendMessage( plugin.hwndParent, WM_WA_IPC, 0, IPC_DELETE );
						cnt++;

						COPYDATASTRUCT cds;
						cds.dwData = IPC_PLAYFILEW;
						cds.lpData = (void *)list;
						cds.cbData = (INT)( sizeof( wchar_t ) * wcslen( (wchar_t *)cds.lpData ) + sizeof( wchar_t ) ); // include space for null char
						SendMessage( plugin.hwndParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds );

						list += wcslen( list ) + 1;
					}
				}

				if (cnt && !enq)
					SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
			}
			break;
		}
	}
	return 0;
}

typedef struct __DRAGTARGET
{
	HWND hwnd;
	UINT state;
	INT testResult;
} DRAGTARGET;

typedef enum 
{
	DRAGSTATE_NOTSUPPORTED = -2,
	DRAGSTATE_UNKNOWN = -1,
	DRAGSTATE_REJECTED = 0,
	DRAGSTATE_ACCEPTED = 1,
} DRAGSTATE;

static DRAGTARGET dragTarget;
static UINT WAML_NOTIFY_DRAGDROP = 0;

static INT_PTR IPC_HandleDrag(HWND hwndML, mlDropItemStruct *pdip)
{
	if (NULL == pdip)
	{
		SetCursor(LoadCursor(NULL, IDC_NO));
		return 0;
	}

	HWND hTarget = (NULL != pdip) ? WindowFromPoint(pdip->p) : NULL;

	if (0 == WAML_NOTIFY_DRAGDROP)
	{
		WAML_NOTIFY_DRAGDROP = RegisterWindowMessageW(WAMLM_DRAGDROP);
		if (0 == WAML_NOTIFY_DRAGDROP)
		{
			if (0 == (ML_HANDLEDRAG_FLAG_NOCURSOR & pdip->flags))
				SetCursor(LoadCursor(NULL, IDC_NO));
			return 0;
		}
	}

	if (NULL != dragTarget.hwnd  && hTarget != dragTarget.hwnd)
	{
		if (DRAGSTATE_NOTSUPPORTED != dragTarget.state && 
			DRAGSTATE_UNKNOWN != dragTarget.state)
			SendMessageW(dragTarget.hwnd, WAML_NOTIFY_DRAGDROP, DRAGDROP_DRAGLEAVE, 0L);

		dragTarget.hwnd = NULL;
		dragTarget.state = DRAGSTATE_UNKNOWN;
		dragTarget.testResult = 0;
	}

	if (NULL == dragTarget.hwnd && NULL != hTarget)
	{
		DWORD targetPid;
		GetWindowThreadProcessId(hTarget, &targetPid);
		if (GetCurrentProcessId() != targetPid) 
		{
			if (0 == (ML_HANDLEDRAG_FLAG_NOCURSOR & pdip->flags))
				SetCursor(LoadCursor(NULL, IDC_NO));
			return 0;
		}
		dragTarget.hwnd = hTarget;
	}

	if (NULL != dragTarget.hwnd)
	{
		switch(dragTarget.state)
		{
			case DRAGSTATE_UNKNOWN:
				pdip->result = 0;
				if (SendMessageW(dragTarget.hwnd, WAML_NOTIFY_DRAGDROP, DRAGDROP_DRAGENTER, (LPARAM)pdip))
				{
					dragTarget.testResult = pdip->result;
					dragTarget.state = (dragTarget.testResult > 0) ? DRAGSTATE_ACCEPTED : DRAGSTATE_REJECTED;
					if (0 == (ML_HANDLEDRAG_FLAG_NOCURSOR & pdip->flags))
						SetCursor((dragTarget.testResult > 0) ? hDragNDropCursor : LoadCursor(NULL, IDC_NO));
					return 0;
				}
				else 
					dragTarget.state = DRAGSTATE_NOTSUPPORTED;
				break;

			case DRAGSTATE_REJECTED:
				pdip->result = dragTarget.testResult;
                return 0;

			case DRAGSTATE_ACCEPTED:
				SendMessageW(dragTarget.hwnd, WAML_NOTIFY_DRAGDROP, DRAGDROP_DRAGOVER, (LPARAM)pdip);
				dragTarget.testResult = pdip->result;
				if (0 == (ML_HANDLEDRAG_FLAG_NOCURSOR & pdip->flags))
						SetCursor((dragTarget.testResult > 0) ? hDragNDropCursor : LoadCursor(NULL, IDC_NO));
				return 0;
		}
	}
	pdip->result = handleDragDropMove(hwndML, pdip->type, pdip->p, !(pdip->flags & ML_HANDLEDRAG_FLAG_NOCURSOR)) > 0 ? 1 : -1;
	return 0;
}

static INT_PTR IPC_HandleDrop(INT_PTR param)
{
	if (param)
	{
		extern HNAVITEM g_treedrag_lastSel;
		if (g_treedrag_lastSel)
		{
			NavItemI_SetState(g_treedrag_lastSel, 0, NIS_DROPHILITED_I);
			g_treedrag_lastSel = 0;
		}

		mlDropItemStruct *m = (mlDropItemStruct *)param;
		if (!m->data) return 0;

		HWND h = WindowFromPoint(m->p);
		if (!h) return 0;;

		if (NULL != dragTarget.hwnd)
		{
			BOOL bProcessed = FALSE;
			if (DRAGSTATE_NOTSUPPORTED != dragTarget.state && 
				DRAGSTATE_UNKNOWN != dragTarget.state)
			{
				if (h != dragTarget.hwnd)
					SendMessageW(dragTarget.hwnd, WAML_NOTIFY_DRAGDROP, DRAGDROP_DRAGLEAVE, 0L);
				else
				{	
					bProcessed = TRUE;
					if (DRAGSTATE_ACCEPTED == dragTarget.state)
						SendMessageW(dragTarget.hwnd, WAML_NOTIFY_DRAGDROP, DRAGDROP_DROP, (LPARAM)m);
				}
			}
			

			dragTarget.hwnd = NULL;
			dragTarget.state = DRAGSTATE_UNKNOWN;
			dragTarget.testResult = 0;

			if (bProcessed)
				return 0;
		}

		extern HWND wa3wnd_fixwnd(HWND);
		h = wa3wnd_fixwnd(h);

		itemRecordList *obj = (itemRecordList *)m->data; // may not be valid heh

		if (IsChild(plugin.hwndParent, h)) h = plugin.hwndParent;
		else if (g_PEWindow && IsChild(g_PEWindow, h)) h = g_PEWindow;
		else
		{
			HWND vid = (HWND)SendMessage(plugin.hwndParent, WM_WA_IPC, IPC_GETWND_VIDEO, IPC_GETWND);
			if (vid)
			{
				if (h == vid || IsChild(vid, h)) h = plugin.hwndParent;
			}
		}

		if (h == plugin.hwndParent)
		{
			int enqueue = (!!g_config->ReadInt(L"enqueuedef", 0)) ^(!!(GetAsyncKeyState(VK_SHIFT) & 0x8000));
			if (m->type == ML_TYPE_ITEMRECORDLIST || m->type == ML_TYPE_CDTRACKS)
				main_playItemRecordList((itemRecordList *)m->data, enqueue);
			else if (m->type == ML_TYPE_ITEMRECORDLISTW)
				main_playItemRecordListW((itemRecordListW *)m->data, enqueue);
			else if (m->type == ML_TYPE_FILENAMES || m->type == ML_TYPE_STREAMNAMES)
			{
				if (!enqueue) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_DELETE);
				char *p = (char*)m->data;

				while (p && *p)
				{
					COPYDATASTRUCT cds;
					cds.dwData = IPC_PLAYFILE;
					cds.lpData = (void *) p;
					cds.cbData =(INT) strlen((char *) cds.lpData) + 1; // include space for null char
					SendMessage(plugin.hwndParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
					p += strlen(p) + 1;
				}
				if (!enqueue) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
			}
			else if (m->type == ML_TYPE_FILENAMESW || m->type == ML_TYPE_STREAMNAMESW)
			{
				if (!enqueue) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_DELETE);
				wchar_t *p = (wchar_t*)m->data;

				while (p && *p)
				{
					COPYDATASTRUCT cds;
					cds.dwData = IPC_PLAYFILEW;
					cds.lpData = (void *) p;
					cds.cbData =(INT)sizeof(wchar_t) * wcslen((wchar_t *) cds.lpData) + sizeof(wchar_t); // include space for null char
					SendMessage(plugin.hwndParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
					p += wcslen(p) + 1;
				}
				if (!enqueue) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
			}
			else if (m->type == ML_TYPE_PLAYLIST)
			{
				if (!enqueue) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_DELETE);
				mlPlaylist *p = (mlPlaylist *)m->data;

					COPYDATASTRUCT cds;
					cds.dwData = IPC_PLAYFILEW;
					cds.lpData = (void *) p->filename;
					cds.cbData = sizeof(wchar_t)*(wcslen(p->filename) + 1);// include space for null char
					SendMessage(plugin.hwndParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);

				if (!enqueue) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
			}
			else if (m->type == ML_TYPE_PLAYLISTS)
			{
				if (!enqueue) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_DELETE);
				mlPlaylist **playlists = (mlPlaylist **)m->data;
				while (playlists && *playlists)
				{
					mlPlaylist *p = *playlists;
					COPYDATASTRUCT cds;
					cds.dwData = IPC_PLAYFILEW;
					cds.lpData = (void *) p->filename;
					cds.cbData = sizeof(wchar_t)*(wcslen(p->filename) + 1);// include space for null char
					SendMessage(plugin.hwndParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
					playlists++;
				}

				if (!enqueue) SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_STARTPLAY);
			}
		}
		else if (h == g_PEWindow)
		{
			POINT p = m->p;
			ScreenToClient(g_PEWindow, &p);
			LRESULT idx = SendMessage(g_PEWindow, WM_USER, IPC_PE_GETIDXFROMPOINT, (LPARAM) & p);
			SendMessage(g_PEWindow, WM_USER, IPC_PE_SAVEEND, (LPARAM)idx);
			if (m->type == ML_TYPE_ITEMRECORDLIST || m->type == ML_TYPE_CDTRACKS)
				main_playItemRecordList((itemRecordList *)m->data, 1);
			else if (m->type == ML_TYPE_ITEMRECORDLISTW)
				main_playItemRecordListW((itemRecordListW *)m->data, 1);
			else if (m->type == ML_TYPE_FILENAMES || m->type == ML_TYPE_STREAMNAMES)
			{
				char *fileName = (char*)m->data;
				while (fileName && *fileName)
				{
					COPYDATASTRUCT cds;
					cds.dwData = IPC_PLAYFILE;
					cds.lpData = (void *)fileName;
					cds.cbData = (INT)strlen((char *) cds.lpData) + 1; // include space for null char
					SendMessage(plugin.hwndParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
					fileName += strlen(fileName) + 1;
				}
			}
			else if (m->type == ML_TYPE_FILENAMESW || m->type == ML_TYPE_STREAMNAMESW)
			{
				wchar_t *fileName = (wchar_t*)m->data;
				while (fileName && *fileName)
				{
					COPYDATASTRUCT cds;
					cds.dwData = IPC_PLAYFILEW;
					cds.lpData = (void *)fileName;
					cds.cbData = (INT)sizeof(wchar_t) * wcslen((wchar_t *) cds.lpData) + sizeof(wchar_t); // include space for null char
					SendMessage(plugin.hwndParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
					fileName += wcslen(fileName) + 1;
				}
			}
			else if (m->type == ML_TYPE_PLAYLIST)
			{
				mlPlaylist *pl = (mlPlaylist *)m->data;

				COPYDATASTRUCT cds;
				cds.dwData = IPC_PLAYFILEW;
				cds.lpData = (void *) pl->filename;
				cds.cbData = sizeof(wchar_t)*(wcslen(pl->filename) + 1);// include space for null char
				SendMessage(plugin.hwndParent, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
			}
			SendMessage(g_PEWindow, WM_USER, IPC_PE_RESTOREEND, 0);
		}
		else if (h == NavCtrlI_GetHWND(hNavigation))
		{
			HNAVITEM hItemHit, hItemSel;
			POINT pt = m->p;
			MapWindowPoints(HWND_DESKTOP, h, &pt, 1);
			
			hItemHit = NavCtrlI_HitTest(hNavigation, &pt, NULL);
			hItemSel = NavCtrlI_GetSelection(hNavigation);
			
			if (hItemHit == hItemSel) m->result = -1;
			else
			{
				INT itemId = NavItemI_GetId(hItemHit);
				if (itemId && plugin_SendMessage(ML_MSG_TREE_ONDROPTARGET, itemId, m->type, NULL) > 0) 
					plugin_SendMessage(ML_MSG_TREE_ONDROPTARGET, itemId, m->type, (INT_PTR)m->data);
			}
		}
		else{
			// TODO: verify this is correct against NAV api usage
			// try to send to the current view if all else failed
			// e.g. dragging a playlist view onto bookmarks page
			HNAVITEM hItemSel = NavCtrlI_GetSelection(hNavigation);
			INT itemId = NavItemI_GetId(hItemSel);
			if(itemId && plugin_SendMessage(ML_MSG_TREE_ONDROPTARGET, itemId, m->type, NULL) > 0)
				plugin_SendMessage(ML_MSG_TREE_ONDROPTARGET, itemId, m->type, (INT_PTR)m->data);
		}
	}
	return 0;
}

static INT_PTR IPC_GetTree(INT_PTR param)
{
	mlGetTreeStruct *mgts;
	HNAVITEM hStartItem = NULL, hStartItem2 = NULL;
	NAVMENUBUILDER nmb;
	wchar_t buffer[1024] = {0};

	if (!param) return NULL;

	mgts = (mlGetTreeStruct *)param;

	hStartItem = NavCtrlI_FindItem(hNavigation, mgts->item_start);

	if (hStartItem) hStartItem2 = NavItemI_GetChild(hStartItem);

	nmb.level = 0;
	nmb.hMenu[0] = CreatePopupMenu();
	nmb.hParent[0] = NavItemI_GetParent(hStartItem2);
	nmb.maxNum = mgts->max_numitems;
	nmb.offset = mgts->cmd_offset;
	nmb.counter	= 0;
	nmb.nis.mask = NIMF_TEXT_I | NIMF_STYLE_I | NIMF_ITEMID_I;
	nmb.nis.styleMask = NIS_HASCHILDREN_I;
	nmb.nis.cchTextMax = sizeof(buffer)/sizeof(wchar_t);
	nmb.nis.pszText = buffer;

	if(hStartItem2 || !mgts->item_start) NavCtrlI_EnumItems(hNavigation, EnumerateNavItemsCB, hStartItem2, (LPARAM)&nmb);

	return (INT_PTR)nmb.hMenu[0];
}

static INT_PTR IPC_GetRating(HWND hwndML, INT_PTR param)
{
	LRESULT plindex = SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
	return getPlRating(hwndML, plindex);
}

static INT_PTR IPC_SetRating(HWND hwndML, INT_PTR param)
{
	INT_PTR rating = min(5, max(0, param));
	LRESULT plindex = SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
	SetPlRating(hwndML, (INT)plindex, (INT)rating);
	SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_UPDTITLE);
	return 1;
}

static INT IPC_RegisterNavigationImage(INT_PTR param)
{
	HMLIMGLST hmlilNavigation;
	MLIMAGESOURCE_I is;
	INT_PTR tag;
	INT mlilIndex;

	if (!param) return -1;

	hmlilNavigation = NavCtrlI_GetImageList(hNavigation);
	if (!hmlilNavigation) return -1;

	ZeroMemory(&is, sizeof(MLIMAGESOURCE_I));
	is.hInst	= ((MLTREEIMAGE*)param)->hinst;
	is.bpp		= 24;
	is.lpszName = MAKEINTRESOURCEW(((MLTREEIMAGE*)param)->resourceId);
	is.type		= SRC_TYPE_BMP_I;
	is.flags	= ISF_FORCE_BPP_I;

	tag = ((MLTREEIMAGE*)param)->imageIndex;
	if (-1 != tag)
	{
		mlilIndex = MLImageListI_GetIndexFromTag(hmlilNavigation, tag);
		if (-1 != mlilIndex) 
		{
			if (!MLImageListI_Replace(hmlilNavigation, mlilIndex, &is, 
				GUID_NULL, tag)) tag = -1;
		}
	}
	else mlilIndex = -1; 

	if (-1 == mlilIndex)
	{
		if ( -1 == tag)
		{
			static INT_PTR tagCounter = 8000;
			tag = tagCounter++;
		}
		mlilIndex = MLImageListI_Add(hmlilNavigation, &is, MLIF_FILTER1_UID, tag);
		if (-1 == mlilIndex) tag = -1;
	}

	return (INT)tag;
}

INT_PTR OnGetCurrentView(void)
{
	return (INT_PTR)m_curview_hwnd;
}

static BOOL OnFlickerFixIPC(FLICKERFIX * pff)
{
	return (pff) ? FlickerFixWindow(pff->hwnd, pff->mode) : FALSE;
}

static HBITMAP OnIPC_MLImageLoader_LoadDib(MLIMAGESOURCE *pmlis)
{
	return (pmlis && pmlis->cbSize == sizeof(MLIMAGESOURCE)) ?
			MLImageLoaderI_LoadDib((MLIMAGESOURCE_I*)&pmlis->hInst) : 
			NULL;
}

static BOOL OnIPC_MLImageLoader_CopyStruct(MLIMAGESOURCECOPY* pmlisCopy)
{
	return (pmlisCopy && pmlisCopy->dest && pmlisCopy->src &&
			pmlisCopy->dest->cbSize == sizeof(MLIMAGESOURCE) &&
			pmlisCopy->dest->cbSize == sizeof(MLIMAGESOURCE)) ? 
				MLImageLoaderI_CopyData((MLIMAGESOURCE_I*)&pmlisCopy->dest->hInst, (MLIMAGESOURCE_I*)&pmlisCopy->src->hInst) :
				FALSE;
}

static BOOL OnIPC_MLImageLoader_FreeStruct(MLIMAGESOURCE *pmlis)
{
	return (pmlis && pmlis->cbSize == sizeof(MLIMAGESOURCE)) ?
			MLImageLoaderI_FreeData((MLIMAGESOURCE_I*)&pmlis->hInst) : 
			FALSE;
}

static BOOL OnIPC_MLImageLoader_CheckExist(MLIMAGESOURCE *pmlis)
{
	return (NULL != pmlis && pmlis->cbSize == sizeof(MLIMAGESOURCE)) ?
			MLImageLoaderI_CheckExist((MLIMAGESOURCE_I*)&pmlis->hInst) : 
			FALSE;
}

static HMLIMGLST OnIPC_MLImageList_Create(MLIMAGELISTCREATE *pmlilCreate)
{
	return (pmlilCreate) ? MLImageListI_Create(pmlilCreate->cx, pmlilCreate->cy, pmlilCreate->flags, pmlilCreate->cInitial, pmlilCreate->cGrow, pmlilCreate->cCacheSize, hmlifMngr) : NULL;
}

static BOOL OnIPC_MLImageList_Destroy(HMLIMGLST hmlil)
{
	return MLImageListI_Destroy(hmlil);
}

static HIMAGELIST OnIPC_MLImageList_GetRealList(HMLIMGLST hmlil)
{
	return MLImageListI_GetRealList(hmlil);
}

static INT OnIPC_MLImageList_GetRealIndex(MLIMAGELISTREALINDEX *pmlilRealIdx)
{
	return (pmlilRealIdx && pmlilRealIdx->cbSize == sizeof(MLIMAGELISTREALINDEX)) ? 
				MLImageListI_GetRealIndex(pmlilRealIdx->hmlil, pmlilRealIdx->mlilIndex, pmlilRealIdx->rgbBk, pmlilRealIdx->rgbFg) : -1;
}

static INT OnIPC_MLImageList_Add(MLIMAGELISTITEM *pmlilItem)
{
	MLIMAGESOURCE_I *pis;

	if (!pmlilItem || pmlilItem->cbSize != sizeof(MLIMAGELISTITEM)) return -1;

	if (pmlilItem->pmlImgSource)
	{
		if (pmlilItem->pmlImgSource->cbSize != sizeof(MLIMAGESOURCE)) return -1;
		pis = (MLIMAGESOURCE_I*)(&pmlilItem->pmlImgSource->hInst);
	} 
	else pis = NULL;
	pmlilItem->mlilIndex = MLImageListI_Add(pmlilItem->hmlil, pis, pmlilItem->filterUID, pmlilItem->nTag);
	return pmlilItem->mlilIndex;
}

static BOOL OnIPC_MLImageList_Replace(MLIMAGELISTITEM *pmlilItem)
{
	MLIMAGESOURCE_I *pis;

	if (!pmlilItem || pmlilItem->cbSize != sizeof(MLIMAGELISTITEM)) return -1;

	if (pmlilItem->pmlImgSource)
	{
		if (pmlilItem->pmlImgSource->cbSize != sizeof(MLIMAGESOURCE)) return -1;
		pis = (MLIMAGESOURCE_I*)((INT*)pmlilItem->pmlImgSource + 1);
	} 
	else pis = NULL;
	return MLImageListI_Replace(pmlilItem->hmlil, pmlilItem->mlilIndex, pis, pmlilItem->filterUID, pmlilItem->nTag);
}

static BOOL OnIPC_MLImageList_Remove(MLIMAGELISTITEM *pmlilItem)
{
	return (pmlilItem) ? MLImageListI_Remove(pmlilItem->hmlil, pmlilItem->mlilIndex) : FALSE;
}

static BOOL OnIPC_MLImageList_GetImageSize(MLIMAGELISTIMAGESIZE *pmlilImageSize)
{
	return (pmlilImageSize) ? MLImageListI_GetImageSize(pmlilImageSize->hmlil, &pmlilImageSize->cx, &pmlilImageSize->cy) : FALSE;
}

static INT OnIPC_MLImageList_GetImageCount(HMLIMGLST hmlil)
{
	return MLImageListI_GetImageCount(hmlil);
}

static BOOL OnIPC_MLImageList_GetIndexFromTag(MLIMAGELISTTAG *pmlilTag)
{
	if (!pmlilTag) return FALSE;
	pmlilTag->mlilIndex = MLImageListI_GetIndexFromTag(pmlilTag->hmlil, pmlilTag->nTag);
	return (-1 != pmlilTag->mlilIndex);
}

static BOOL OnIPC_MLImageList_GetTagFromIndex(MLIMAGELISTTAG *pmlilTag)
{
	return (pmlilTag) ? MLImageListI_GetTagFromIndex(pmlilTag->hmlil, pmlilTag->mlilIndex, &pmlilTag->nTag) : FALSE;
}

static BOOL OnIPC_MLImageList_CheckItemExist(MLIMAGELISTITEM *pmlilItem)
{
	return (pmlilItem) ? MLImageListI_CheckItemExist(pmlilItem->hmlil, pmlilItem->mlilIndex) : FALSE;
}

static BOOL OnIPC_MLImageFilter_Register(MLIMAGEFILTERINFO* pmlif)
{
	return (pmlif && pmlif->cbSize == sizeof(MLIMAGEFILTERINFO)) ?
		MLImageFilterI_Register(hmlifMngr, (MLIMAGEFILTERINFO_I*)((INT*)pmlif +1)) : FALSE;
}

static BOOL OnIPC_MLImageFilter_Unregister(const GUID* pUID)
{
	return (pUID) ? MLImageFilterI_Unregister(hmlifMngr, *pUID) : FALSE;
}

static BOOL OnIPC_MLImageFilter_GetInfo(MLIMAGEFILTERINFO* pmlif)
{
	return (pmlif && pmlif->cbSize == sizeof(MLIMAGEFILTERINFO)) ?
		MLImageFilterI_GetInfo(hmlifMngr, (MLIMAGEFILTERINFO_I*)((INT*)pmlif +1)) : FALSE;
}

static BOOL OnIPC_MLImageFilter_ApplyEx(MLIMAGEFILTERAPPLYEX* pmlifApplyEx)
{
	if (!pmlifApplyEx || pmlifApplyEx->cbSize != sizeof(MLIMAGEFILTERAPPLYEX)) return FALSE;
	return MLImageFilterI_ApplyEx(hmlifMngr, &pmlifApplyEx->filterUID, pmlifApplyEx->pData, pmlifApplyEx->cx,
									pmlifApplyEx->cy, pmlifApplyEx->bpp, pmlifApplyEx->rgbBk, pmlifApplyEx->rgbFg, pmlifApplyEx->imageTag);
}

static INT OnIPC_MLNavCtrl_BeginUpdate(UINT fRememberPos)
{
	return NavCtrlI_BeginUpdate(hNavigation, fRememberPos);
}

static BOOL OnIPC_MLNavCtrl_DeleteItem(HNAVITEM hItem)
{
	return NavCtrlI_DeleteItem(hNavigation, hItem);
}

static INT OnIPC_MLNavCtrl_EndUpdate(void)
{
	return NavCtrlI_EndUpdate(hNavigation);
}

static BOOL OnIPC_MLNavCtrl_EnumItems(NAVCTRLENUMPARAMS *pnavEnumParams)
{
	return (pnavEnumParams) ? NavCtrlI_EnumItems(hNavigation, (NAVENUMPROC_I)pnavEnumParams->enumProc, 
									pnavEnumParams->hItemStart, pnavEnumParams->lParam) : FALSE;
}

HNAVITEM OnIPC_MLNavCtrl_FindItemById(INT itemId)
{
	return NavCtrlI_FindItem(hNavigation, itemId);
}

static HNAVITEM OnIPC_MLNavCtrl_FindItemByName(NAVCTRLFINDPARAMS *pnavFindParams)
{
	if (!pnavFindParams) return NULL;
	return  (!pnavFindParams->fFullNameSearch) ? 
		NavCtrlI_FindItemByName(hNavigation, pnavFindParams->Locale, pnavFindParams->compFlags, pnavFindParams->pszName, pnavFindParams->cchLength) :
		NavCtrlI_FindItemByFullName(hNavigation, pnavFindParams->Locale, pnavFindParams->compFlags, pnavFindParams->pszName, pnavFindParams->cchLength, pnavFindParams->fAncestorOk);
}

static HMLIMGLST OnIPC_MLNavCtrl_GetImageList(void)
{
	return NavCtrlI_GetImageList(hNavigation);
}

static HNAVITEM OnIPC_MLNavCtrl_GetFirst(void)
{
	return NavCtrlI_GetRoot(hNavigation);
}

static INT OnIPC_MLNavCtrl_GetIndent(void)
{
	return NavCtrlI_GetIndent(hNavigation);
}

static DWORD OnIPC_MLNavCtrl_GetStyle(void)
{
	return NavCtrlI_GetStyle(hNavigation);
}

static HNAVITEM OnIPC_MLNavCtrl_GetSelection(void)
{
	return NavCtrlI_GetSelection(hNavigation);
}

static HWND OnIPC_MLNavCtrl_GetHWND(void)
{
	return NavCtrlI_GetHWND(hNavigation);
}

static HNAVITEM OnIPC_MLNavCtrl_HitTest(NAVHITTEST *pnavHit)
{
	if (!pnavHit) return NULL;
	pnavHit->hItem = NavCtrlI_HitTest(hNavigation, &pnavHit->pt, &pnavHit->flags);
	return pnavHit->hItem;
}

static HNAVITEM OnIPC_MLNavCtrl_InsertItem(NAVINSERTSTRUCT *pnavInsert)
{
	if (!hNavigation || !pnavInsert || pnavInsert->item.cbSize != sizeof(NAVITEM)) return NULL;
	return NavCtrlI_InsertItem(hNavigation, pnavInsert->hInsertAfter, pnavInsert->hParent, (NAVITEM_I*)&pnavInsert->item.mask);
}

static BOOL OnIPC_MLNavItem_EnsureVisible(HNAVITEM hItem)
{
	return NavItemI_EnsureVisible(hItem);
}

static BOOL OnIPC_MLNavItem_Expand(NAVITEMEXPAND *pnavExpand)
{
	return (pnavExpand) ? NavItemI_Expand(pnavExpand->hItem, pnavExpand->fCmdExpand) : FALSE;
}

static HNAVITEM OnIPC_MLNavItem_GetChild(HNAVITEM hItem)
{
	return NavItemI_GetChild(hItem);
}

static INT OnIPC_MLNavItem_GetChildrenCount(HNAVITEM hItem)
{
	return NavItemI_GetChildrenCount(hItem);
}

static INT OnIPC_MLNavItem_GetFullName(NAVITEMFULLNAME* pnavFullName)
{
	return (pnavFullName) ? NavItemI_GetFullName(pnavFullName->hItem, pnavFullName->pszText, pnavFullName->cchTextMax) : 0;
}

static BOOL OnIPC_MLNavItem_GetInfo(NAVITEM *pnavItem)
{
	return (pnavItem && pnavItem->cbSize == sizeof(NAVITEM)) ? NavItemI_GetIndirect(pnavItem->hItem, (NAVITEM_I*)&pnavItem->mask) : FALSE;
}

static HNAVITEM OnIPC_MLNavItem_GetNext(HNAVITEM hItem)
{
	return NavItemI_GetNext(hItem);
}

static WORD OnIPC_MLNavItem_GetOrder(HNAVITEM hItem)
{
	return NavItemI_GetOrder(hItem);
}

static HNAVITEM OnIPC_MLNavItem_GetParent(HNAVITEM hItem)
{
	return NavItemI_GetParent(hItem);
}

static HNAVITEM OnIPC_MLNavItem_GetPrevious(HNAVITEM hItem)
{
	return NavItemI_GetPrevious(hItem);
}

static BOOL OnIPC_MLNavItem_GetRect(NAVITEMGETRECT *pnavRect)
{
	return (pnavRect) ? NavItemI_GetRect(pnavRect->hItem, &pnavRect->rc, pnavRect->fItem) : FALSE;
}

static UINT OnIPC_MLNavItem_GetState(HNAVITEM hItem)
{
	return NavItemI_GetState(hItem, 0xFFFFFFFF);
}

static BOOL OnIPC_MLNavItem_HasChildrenReal(HNAVITEM hItem)
{
	return NavItemI_HasChildrenReal(hItem);
}

static BOOL OnIPC_MLNavItem_Invalidate(NAVITEMINAVLIDATE *pnavInvalidate)
{
	return (pnavInvalidate) ? NavItemI_Invalidate(pnavInvalidate->hItem, pnavInvalidate->prc, pnavInvalidate->fErase) : FALSE;
}

static BOOL OnIPC_MLNavItem_Move(NAVITEMMOVE *pnavMove)
{
	return (pnavMove) ? NavItemI_Move(pnavMove->hItem, pnavMove->hItemDest, pnavMove->fAfter) : FALSE;
}

static BOOL OnIPC_MLNavItem_Select(HNAVITEM hItem)
{
	return NavItemI_Select(hItem);
}

static BOOL OnIPC_MLNavItem_SetInfo(NAVITEM *pnavItem)
{
	return (pnavItem && pnavItem->cbSize == sizeof(NAVITEM)) ? NavItemI_SetIndirect(pnavItem->hItem, (NAVITEM_I*)&pnavItem->mask) : FALSE;
}

static WORD OnIPC_MLNavItem_SetOrder(NAVITEMORDER *pnavOrder)
{
	return (pnavOrder) ? NavItemI_SetOrder(pnavOrder->hItem, pnavOrder->order, pnavOrder->flags) : 0xFFFF;
}

static BOOL OnIPC_MLNavItem_EditTitle(HNAVITEM hItem)
{
	return NavItemI_EditTitle(hItem);
}

static BOOL OnIPC_MLNavCtrl_EndEditTitle(BOOL fCancel)
{
	return NavCtrlI_EndEditTitle(hNavigation, fCancel);
}

static BOOL OnIPC_MLRating_Draw(RATINGDRAWPARAMS *prdp)
{
	if (!prdp) return FALSE;
	return MLRatingI_Draw(prdp->hdcDst, prdp->maxValue, prdp->value, prdp->trackingValue, 
							(prdp->hMLIL) ? prdp->hMLIL : hmlilRating, 
							(prdp->hMLIL) ? prdp->index : 0, 
							&prdp->rc, prdp->fStyle);
}

static INT OnIPC_MLRating_Hittest(RATINGHITTESTPARAMS *prhtp)
{
	LONG result;
	if (!prhtp) return FALSE;
	result = MLRatingI_HitTest(prhtp->pt, prhtp->maxValue,  
							(prhtp->hMLIL) ? prhtp->hMLIL : hmlilRating, 
							&prhtp->rc, prhtp->fStyle);
	prhtp->hitFlags = HIWORD(result);
	prhtp->value = LOWORD(result);
	return prhtp->value;
}

static BOOL OnIPC_MLRating_CalcRect(RECT *prc)
{
	return (prc) ? MLRatingI_CalcMinRect(prc->top, (prc->left) ? (HMLIMGLST)(LONG_PTR)prc->left : hmlilRating, prc) : FALSE;
}

static BOOL OnIPC_MLCloud_Draw(CLOUDDRAWPARAMS *prdp)
{
	if (!prdp) return FALSE;
	return MLCloudI_Draw(prdp->hdcDst, prdp->value, (prdp->hMLIL) ? prdp->hMLIL : hmlilCloud, 
						 (prdp->hMLIL) ? prdp->index : 0, &prdp->rc);
}

static BOOL OnIPC_MLCloud_CalcRect(RECT *prc)
{
	return (prc) ? MLCloudI_CalcMinRect((prc->left) ? (HMLIMGLST)(LONG_PTR)prc->left : hmlilCloud, prc) : FALSE;
}

static BOOL OnIPC_SkinWindow(MLSKINWINDOW* pmlsw)
{
	return (pmlsw) ? SkinWindowEx(pmlsw->hwndToSkin, pmlsw->skinType, pmlsw->style) : FALSE;
}

static BOOL OnIPC_UnskinWindow(HWND hwndToUnskin)
{
	return UnskinWindow(hwndToUnskin);
}

static BOOL OnIPC_TrackSkinnedPopup(MLSKINNEDPOPUP *pmlsp)
{
	if (pmlsp && pmlsp->cbSize == sizeof(MLSKINNEDPOPUP))
	{
		return MediaLibrary_TrackPopupEx(pmlsp->hmenu, pmlsp->fuFlags, pmlsp->x, pmlsp->y, pmlsp->hwnd, pmlsp->lptpm,
				pmlsp->hmlil, pmlsp->width, pmlsp->skinStyle, pmlsp->customProc, pmlsp->customParam);
	}
	else
		return FALSE;
}

static HANDLE OnIPC_InitSkinnedPopupHook(MLSKINNEDPOPUP *pmlsp)
{
	if (pmlsp && pmlsp->cbSize == sizeof(MLSKINNEDPOPUP))
	{
		return MediaLibrary_InitSkinnedPopupHook(pmlsp->hwnd, pmlsp->hmlil, pmlsp->width, pmlsp->skinStyle, 
					pmlsp->customProc, pmlsp->customParam);
	}
	
	return NULL;
}

static BOOL OnIPC_RemoveSkinnedPopupHook(HANDLE hPopupHook)
{
	RemoveSkinnedPopupHook(hPopupHook);
	return TRUE;
}

static BOOL OnIPC_RatingColumn_Paint(RATINGCOLUMNPAINT *prcp)
{
	return (prcp) ? MLRatingColumnI_Paint((RATINGCOLUMNPAINT_I*)prcp) : FALSE;
}

static BOOL OnIPC_RatingColumn_OnClick(RATINGCOLUMN *pRating)
{
	return (pRating) ? MLRatingColumnI_Click((RATINGCOLUMN_I*)pRating) : FALSE;
}

static BOOL OnIPC_RatingColumn_OnTrack(RATINGCOLUMN *pRating)
{
	if (!pRating) return FALSE;
	MLRatingColumnI_Track((RATINGCOLUMN_I*)pRating);
	return TRUE;
}

static BOOL OnIPC_RatingColumn_CancelTracking(BOOL bRedrawNow)
{
	MLRatingColumnI_CancelTracking(bRedrawNow);
	return TRUE;
}

static BOOL OnIPC_RatingColumn_BeginDrag(RATINGCOLUMN *pRating)
{
	return (pRating) ? MLRatingColumnI_BeginDrag((RATINGCOLUMN_I*)pRating) : FALSE;
}

static BOOL OnIPC_RatingColumn_OnDrag(POINT *ppt)
{
	return (ppt) ? MLRatingColumnI_Drag(*ppt) : FALSE;
}

static BOOL OnIPC_RatingColumn_OnEndDrag(RATINGCOLUMN *pRating)
{
	return (pRating) ? MLRatingColumnI_EndDrag((RATINGCOLUMN_I*)pRating) : FALSE;
}

static BOOL OnIPC_RatingColumn_Animate(RATINGCOLUMN *pRating)
{
	if (!pRating) return FALSE;
	MLRatingColumnI_Animate(pRating->hwndList, pRating->iItem, pRating->iSubItem);
	return TRUE;
}

static INT OnIPC_RatingColumn_GetMinWidth()
{
	return MLRatingColumnI_GetMinWidth();
}

static BOOL OnIPC_CloudColumn_GetWidth(INT *width)
{
	if (!width) return FALSE;
	*width = MLCloudColumnI_GetWidth(*width);
	return TRUE;
}

static BOOL OnIPC_CloudColumn_Paint(RATINGCOLUMNPAINT *prcp)
{
	return (prcp) ? MLCloudColumnI_Paint((CLOUDCOLUMNPAINT_I*)prcp) : FALSE;
}

static INT OnIPC_CloudColumn_GetMinWidth()
{
	return MLCloudColumnI_GetMinWidth();
}

static BOOL OnIPC_RatingColumn_FillBackString(RATINGBACKTEXT *prbt)
{
	if (!prbt) return FALSE;
	MLRatingColumnI_FillBackString(prbt->pszText, prbt->cchTextMax, prbt->nColumnWidth, prbt->fStyle);
	return TRUE;
}

static BOOL OnIPC_RatingColumn_GetWidth(RATINGWIDTH *prw)
{
	if (!prw) return FALSE;
	prw->width = MLRatingColumnI_GetWidth(prw->width, prw->fStyle);
	return TRUE;
}

static HWND OnIPC_CreateWebInfo(WEBINFOCREATE *pWebInfoCreate)
{

	if (!pWebInfoCreate) return NULL;
	if (-1 == pWebInfoCreate->cy) pWebInfoCreate->cy = WEBINFO_PREFFERED_HEIGHT;

	return CreateWebInfoWindow(pWebInfoCreate->hwndParent, pWebInfoCreate->uMsgQuery, 
					pWebInfoCreate->x, pWebInfoCreate->y, pWebInfoCreate->cx, pWebInfoCreate->cy, pWebInfoCreate->ctrlId);
}

static HWND OnIPC_CreateFileView(MLFILEVIEWCREATESTRUCT *pfvcs)
{
	if (!pfvcs) return NULL;
	return FileView_CreateDialog(pfvcs->hwndParent, pfvcs->fStyle, pfvcs->hwndInsertAfter, pfvcs->x, pfvcs->y, pfvcs->cx, pfvcs->cy); 
}

INT_PTR pluginHandleIpcMessage(HWND hwndML, int msg, INT_PTR param)
{
	switch (msg)
	{
		case ML_IPC_PL_GETRATING:		return getPlRating(hwndML, param);
		case ML_IPC_PL_SETRATING:		return IPC_PL_SetRating(hwndML, param);
		case ML_IPC_GETRATING:			return IPC_GetRating(hwndML, param);
		case ML_IPC_SETRATING:			return IPC_SetRating(hwndML, param);
		case ML_IPC_OPENPREFS:
			{
				extern prefsDlgRecW myPrefsItem;
				SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&myPrefsItem, IPC_OPENPREFSTOPAGE);
			}
			return 1;

		case ML_IPC_GETTREE:				return IPC_GetTree(param);
		case ML_IPC_TREEITEM_INSERT:
		case ML_IPC_TREEITEM_INSERTW:
		case ML_IPC_ADDTREEITEM_EX:
		case ML_IPC_ADDTREEITEM:
		case ML_IPC_TREEITEM_ADD:
		case ML_IPC_TREEITEM_ADDW:			return IPC_InsertView(msg, param);
		case ML_IPC_SETTREEITEM_EX:
		case ML_IPC_SETTREEITEM:
		case ML_IPC_TREEITEM_SETINFO:
		case ML_IPC_TREEITEM_SETINFOW:		return (INT_PTR)IPC_SetView(msg, param);
		case ML_IPC_TREEITEM_GETINFO:
		case ML_IPC_TREEITEM_GETINFOW:		return (INT_PTR)IPC_GetView(msg, param);
		case ML_IPC_GETCURTREEITEM:			return (INT_PTR)NavItemI_GetId(NavCtrlI_GetSelection(hNavigation));
		case ML_IPC_TREEITEM_GETSELECTED:	return (INT_PTR)NavCtrlI_GetSelection(hNavigation);	
		case ML_IPC_SETCURTREEITEM:			return (INT_PTR)NavItemI_Select(NavCtrlI_FindItem(hNavigation, (INT)param));
		case ML_IPC_TREEITEM_SELECT:		return (INT_PTR)NavItemI_Select((HNAVITEM)param);
		case ML_IPC_DELTREEITEM:			return (INT_PTR)NavCtrlI_DeleteItem(hNavigation, NavCtrlI_FindItem(hNavigation, (INT)param));
		case ML_IPC_TREEITEM_DELETE:		return (INT_PTR)NavCtrlI_DeleteItem(hNavigation, (HNAVITEM)param);
		case ML_IPC_TREEITEM_GETHANDLE:		return (INT_PTR)NavCtrlI_FindItem(hNavigation, (INT)param);
		case ML_IPC_TREEITEM_GETCHILD:		return (INT_PTR)((MLTI_ROOT == param) ? NavCtrlI_GetRoot(hNavigation) : NavItemI_GetChild((HNAVITEM)param));
		case ML_IPC_TREEITEM_GETNEXT:		return (INT_PTR)NavItemI_GetNext((HNAVITEM)param);
		case ML_IPC_TREEITEM_GETCHILD_ID:	return (INT_PTR)NavItemI_GetId(NavItemI_GetChild(NavCtrlI_FindItem(hNavigation, (INT)param)));
		case ML_IPC_TREEITEM_GETNEXT_ID:	return (INT_PTR)NavItemI_GetId(NavItemI_GetNext(NavCtrlI_FindItem(hNavigation, (INT)param)));
		case ML_IPC_TREEITEM_GETROOT:		return (INT_PTR)NavCtrlI_GetRoot(hNavigation);
		case ML_IPC_ADDTREEIMAGE:
		case ML_IPC_TREEIMAGE_ADD:			return (INT_PTR)IPC_RegisterNavigationImage(param);
		case ML_IPC_GET_VIEW_BUTTON_TEXT:
		{
			if (!playBuf[0]) WASABI_API_LNGSTRINGW_BUF(IDS_PLAY, playBuf, ARRAYSIZE(playBuf));
			if (!enqueueBuf[0]) WASABI_API_LNGSTRINGW_BUF(IDS_ENQUEUE, enqueueBuf, ARRAYSIZE(enqueueBuf));

			viewButtons *view = (viewButtons*)param;
			if (view)
			{
				view->play = playBuf;
				view->enqueue = enqueueBuf;
			}
			break;
		}

		case ML_IPC_HANDLEDRAG:				return IPC_HandleDrag(hwndML, (mlDropItemStruct*)param);
		case ML_IPC_HANDLEDROP: // hmm, not sure bout this one =)
			return IPC_HandleDrop(param);

		case ML_IPC_SKIN_COMBOBOX:			return (param) ? (INT_PTR)(new ComboSkin((HWND)param)) : NULL;
		case ML_IPC_UNSKIN_COMBOBOX:		if (param) delete reinterpret_cast<ComboSkin*>(param); return (NULL != param);
		case ML_IPC_SKIN_LISTVIEW:			return (SkinWindowEx((HWND)param, SKINNEDWND_TYPE_LISTVIEW, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER)) ? param : NULL;
		case ML_IPC_UNSKIN_LISTVIEW:		return UnskinWindow((HWND)param);
		case ML_IPC_LISTVIEW_UPDATE:		return MLSkinnedScrollWnd_UpdateBars((HWND)param, TRUE);
		case ML_IPC_LISTVIEW_DISABLEHSCROLL:return MLSkinnedScrollWnd_ShowHorzBar((HWND)param, FALSE);
		case ML_IPC_LISTVIEW_DISABLEVSCROLL:return MLSkinnedScrollWnd_ShowVertBar((HWND)param, FALSE);
	
		case ML_IPC_LISTVIEW_SHOWSORT:
			if (param && ((LV_SKIN_SHOWSORT*)param)->listView)
			{
				if (((LV_SKIN_SHOWSORT*)param)->showSort) return TRUE;
				return SendMessageW((HWND)((LV_SKIN_SHOWSORT*)param)->listView, WM_ML_IPC, 
										MAKEWPARAM((((LV_SKIN_SHOWSORT*)param)->showSort) ? 0 : -1, TRUE),
										ML_IPC_SKINNEDLISTVIEW_DISPLAYSORT);
			}
			return 0;
		case ML_IPC_LISTVIEW_SORT:			
			if (param && ((LV_SKIN_SORT*)param)->listView)
			{
				return SendMessageW((HWND)((LV_SKIN_SHOWSORT*)param)->listView, WM_ML_IPC, 
										MAKEWPARAM(((LV_SKIN_SORT*)param)->columnIndex, ((LV_SKIN_SORT*)param)->ascending),
										ML_IPC_SKINNEDLISTVIEW_DISPLAYSORT);
			}
			return 0;

		case ML_IPC_SENDTOWINAMP:
			return IPC_SendToWinamp(param);

		case ML_IPC_SKIN_WADLG_GETFUNC:
			switch (param)
			{
				case 1: return (INT_PTR)WADlg_getColor;
				case 2: return (INT_PTR)WADlg_handleDialogMsgs;
				case 3: return (INT_PTR)WADlg_DrawChildWindowBorders;
				case 4: return (INT_PTR)WADlg_getBitmap;
				case 32: return (INT_PTR)childresize_init;
				case 33: return (INT_PTR)childresize_resize;
				case 66:
					if (g_config->ReadInt(L"plfont_everywhere", 1))
					{
						INT height = (INT)SendMessageW(plugin.hwndParent, WM_WA_IPC, 3, IPC_GET_GENSKINBITMAP);
						DWORD charset = (DWORD)SendMessageW(plugin.hwndParent, WM_WA_IPC, 2, IPC_GET_GENSKINBITMAP);
						char *fontname = (char*)SendMessageW(plugin.hwndParent, WM_WA_IPC, 1, IPC_GET_GENSKINBITMAP);
						return (INT_PTR)CreateFontA(-height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontname);
					}
					return 0;

			}
			break;
		case ML_IPC_ADDTOSENDTO:
			if (param)
			{
				mlAddToSendToStruct *p = (mlAddToSendToStruct *)param;
				if (p->context)
				{
					SendToMenu *a = (SendToMenu*)p->context;
					a->onAddItem(p);
				}
			}
			break;
		case ML_IPC_ADDTOSENDTOW:
			if (param)
			{
				mlAddToSendToStructW *p = (mlAddToSendToStructW *)param;
				if (p->context)
				{
					SendToMenu *a = (SendToMenu*)p->context;
					a->onAddItem(p);
				}
			}
			break;
		case ML_IPC_BRANCHSENDTO:
			if (param)
			{
				mlAddToSendToStructW *p = (mlAddToSendToStructW *)param;
				if (p->context)
				{
					SendToMenu *a = (SendToMenu*)p->context;
					if (p->desc)
						a->endBranch(p->desc);
					else
						a->startBranch();
				}
			}
			break;
		case ML_IPC_ADDTOBRANCH:
			if (param)
			{
				mlAddToSendToStructW *p = (mlAddToSendToStructW *)param;
				if (p->context)
				{
					SendToMenu *a = (SendToMenu*)p->context;
					a->addItemToBranch(p);
				}
			}
			break;

		///////////// plugin adding/removing
		case ML_IPC_ADD_PLUGIN:
		case ML_IPC_REMOVE_PLUGIN:
			if (param)
			{
				winampMediaLibraryPlugin *p = (winampMediaLibraryPlugin *)param;
				int x, l = m_plugins.GetSize();
				for (x = 0; x < l && m_plugins.Get(x) != (void *)param; x ++);
				if (x < l)
				{
					if (msg == ML_IPC_ADD_PLUGIN) return -1;
					m_plugins.Del(x); // remove this plugin
					return 1;
				}
				if (msg == ML_IPC_REMOVE_PLUGIN) return -1; //plugin not found to delete

				if (p->version > MLHDR_VER && p->version < MLHDR_VER_OLD /*|| p->hDllInstance*/)  return -1; //benski> cut the hDllInstance == 0 requirement, BUT you have to do a LoadLibrary to fake it out

				if (msg == ML_IPC_ADD_PLUGIN && p->hDllInstance)
				{
					// if they set the hDllInstance, do a LoadLibrary to increment the reference count
					// that way we can FreeLibrary it safely later
					TCHAR filename[MAX_PATH] = {0};
					GetModuleFileName(p->hDllInstance, filename, MAX_PATH);
					HMODULE dummyLoad = LoadLibraryW(filename);
					assert(dummyLoad == p->hDllInstance);
				}
				p->hwndLibraryParent = hwndML;
				p->hwndWinampParent = plugin.hwndParent;

				m_plugins.Add(p);
				return 1;
			}
			return -1;
		case ML_IPC_SEND_PLUGIN_MESSAGE:
			if (param)
			{
				pluginMessage *m = (pluginMessage*)param;
				return plugin_SendMessage(m->messageType, m->param1, m->param2, m->param3);
			}
			return -1;
			/*
		case ML_IPC_GRACENOTE:
			switch (param)
			{
				case GRACENOTE_TUID:			return(INT_PTR)gracenoteGetTuid();
				case GRACENOTE_IS_WORKING:  		return(INT_PTR)gracenoteIsWorking();
				case GRACENOTE_DO_TIMER_STUFF:	return(INT_PTR)gracenoteDoTimerStuff();
				case GRACENOTE_CANCEL_REQUEST:	gracenoteCancelRequest(); return 1;
			}
			break;
			*/

		case ML_IPC_REFRESH_PREFS:
			refreshPrefs(param);
			return 1;

		case ML_IPC_GETCURRENTVIEW:
			return OnGetCurrentView();

		case ML_IPC_ENSURE_VISIBLE:
			if (!IsVisible())
				toggleVisible();
			return 1;

		case ML_IPC_IS_VISIBLE:
			return IsVisible() ? 1 : 0;

		case ML_IPC_GET_PARENTAL_RATING: // TODO: store this in api_config
			return g_config->ReadInt(L"tvrating", 7);

		case ML_IPC_TOGGLE_VISIBLE:
			toggleVisible();
			return 1;

		case ML_IPC_FOCUS_TREE:
			SetFocus(NavCtrlI_GetHWND(hNavigation));
			return 1;

		case ML_IPC_FLICKERFIX:				return (param) ? OnFlickerFixIPC((FLICKERFIX*)param) : FALSE;

		case ML_IPC_GETVERSION:				return PLUGIN_VERSION;

		case ML_IPC_SHOW_HELP:				return MediaLibrary_OpenHelpUrl((const wchar_t*)param);

		case ML_IPC_IMAGELOADER_LOADDIB:	return (INT_PTR)OnIPC_MLImageLoader_LoadDib((MLIMAGESOURCE*)param);
		case ML_IPC_IMAGELOADER_COPYSTRUCT:	return (INT_PTR)OnIPC_MLImageLoader_CopyStruct((MLIMAGESOURCECOPY*)param);
		case ML_IPC_IMAGELOADER_FREESTRUCT:	return (INT_PTR)OnIPC_MLImageLoader_FreeStruct((MLIMAGESOURCE*)param);
		case ML_IPC_IMAGELOADER_CHECKEXIST: return (INT_PTR)OnIPC_MLImageLoader_CheckExist((MLIMAGESOURCE*)param);

		case ML_IPC_IMAGELIST_CREATE:		return (INT_PTR)OnIPC_MLImageList_Create((MLIMAGELISTCREATE*)param);
		case ML_IPC_IMAGELIST_DESTROY:		return (INT_PTR)OnIPC_MLImageList_Destroy((HMLIMGLST)param);
		case ML_IPC_IMAGELIST_GETREALLIST:	return (INT_PTR)OnIPC_MLImageList_GetRealList((HMLIMGLST)param);
		case ML_IPC_IMAGELIST_GETREALINDEX:	return (INT_PTR)OnIPC_MLImageList_GetRealIndex((MLIMAGELISTREALINDEX*)param);
		case ML_IPC_IMAGELIST_ADD:			return (INT_PTR)OnIPC_MLImageList_Add((MLIMAGELISTITEM*)param);
		case ML_IPC_IMAGELIST_REPLACE:		return (INT_PTR)OnIPC_MLImageList_Replace((MLIMAGELISTITEM*)param);
		case ML_IPC_IMAGELIST_REMOVE:		return (INT_PTR)OnIPC_MLImageList_Remove((MLIMAGELISTITEM*)param);
		case ML_IPC_IMAGELIST_GETIMAGESIZE:	return (INT_PTR)OnIPC_MLImageList_GetImageSize((MLIMAGELISTIMAGESIZE*)param);
		case ML_IPC_IMAGELIST_GETIMAGECOUNT:	return (INT_PTR)OnIPC_MLImageList_GetImageCount((HMLIMGLST)param);
		case ML_IPC_IMAGELIST_GETINDEXFROMTAG:	return (INT_PTR)OnIPC_MLImageList_GetIndexFromTag((MLIMAGELISTTAG*)param);
		case ML_IPC_IMAGELIST_GETTAGFROMINDEX:	return (INT_PTR)OnIPC_MLImageList_GetTagFromIndex((MLIMAGELISTTAG*)param);
		case ML_IPC_IMAGELIST_CHECKEXIST:	return (INT_PTR)OnIPC_MLImageList_CheckItemExist((MLIMAGELISTITEM*)param);
		case ML_IPC_IMAGEFILTER_REGISTER:	return (INT_PTR)OnIPC_MLImageFilter_Register((MLIMAGEFILTERINFO*)param);
		case ML_IPC_IMAGEFILTER_UNREGISTER:	return (INT_PTR)OnIPC_MLImageFilter_Unregister((const GUID*)param);
		case ML_IPC_IMAGEFILTER_GETINFO:	return (INT_PTR)OnIPC_MLImageFilter_GetInfo((MLIMAGEFILTERINFO*)param);
		case ML_IPC_IMAGEFILTER_APPLYEX:	return (INT_PTR)OnIPC_MLImageFilter_ApplyEx((MLIMAGEFILTERAPPLYEX*)param);

		case ML_IPC_NAVCTRL_BEGINUPDATE:	return (INT_PTR)OnIPC_MLNavCtrl_BeginUpdate((UINT)param);
		case ML_IPC_NAVCTRL_DELETEITEM:		return (INT_PTR)OnIPC_MLNavCtrl_DeleteItem((HNAVITEM)param);
		case ML_IPC_NAVCTRL_ENDUPDATE:		return (INT_PTR)OnIPC_MLNavCtrl_EndUpdate();
		case ML_IPC_NAVCTRL_ENUMITEMS:		return (INT_PTR)OnIPC_MLNavCtrl_EnumItems((NAVCTRLENUMPARAMS*)param);
		case ML_IPC_NAVCTRL_FINDITEMBYID:	return (INT_PTR)OnIPC_MLNavCtrl_FindItemById((INT)param);
		case ML_IPC_NAVCTRL_FINDITEMBYNAME:	return (INT_PTR)OnIPC_MLNavCtrl_FindItemByName((NAVCTRLFINDPARAMS*)param);
		case ML_IPC_NAVCTRL_GETIMAGELIST:	return (INT_PTR)OnIPC_MLNavCtrl_GetImageList();
		case ML_IPC_NAVCTRL_GETFIRST:		return (INT_PTR)OnIPC_MLNavCtrl_GetFirst();
		case ML_IPC_NAVCTRL_GETINDENT:		return (INT_PTR)OnIPC_MLNavCtrl_GetIndent();
		case ML_IPC_NAVCTRL_GETSELECTION:	return (INT_PTR)OnIPC_MLNavCtrl_GetSelection();
		case ML_IPC_NAVCTRL_GETHWND:		return (INT_PTR)OnIPC_MLNavCtrl_GetHWND();
		case ML_IPC_NAVCTRL_HITTEST:		return (INT_PTR)OnIPC_MLNavCtrl_HitTest((NAVHITTEST*)param);
		case ML_IPC_NAVCTRL_INSERTITEM:		return (INT_PTR)OnIPC_MLNavCtrl_InsertItem((NAVINSERTSTRUCT*)param);
		case ML_IPC_NAVCTRL_ENDEDITTITLE:	return (INT_PTR)OnIPC_MLNavCtrl_EndEditTitle((BOOL)param);
		case ML_IPC_NAVCTRL_GETSTYLE:		return (INT_PTR)OnIPC_MLNavCtrl_GetStyle();

		case ML_IPC_NAVITEM_EDITTITLE:		return (INT_PTR)OnIPC_MLNavItem_EditTitle((HNAVITEM)param);
		case ML_IPC_NAVITEM_ENSUREVISIBLE:	return (INT_PTR)OnIPC_MLNavItem_EnsureVisible((HNAVITEM)param);
		case ML_IPC_NAVITEM_EXPAND:			return (INT_PTR)OnIPC_MLNavItem_Expand((NAVITEMEXPAND*)param);
		case ML_IPC_NAVITEM_GETCHILD:		return (INT_PTR)OnIPC_MLNavItem_GetChild((HNAVITEM)param);
		case ML_IPC_NAVITEM_GETCHILDRENCOUNT:	return (INT_PTR)OnIPC_MLNavItem_GetChildrenCount((HNAVITEM)param);
		case ML_IPC_NAVITEM_GETFULLNAME:	return (INT_PTR)OnIPC_MLNavItem_GetFullName((NAVITEMFULLNAME*)param);
		case ML_IPC_NAVITEM_GETINFO:		return (INT_PTR)OnIPC_MLNavItem_GetInfo((NAVITEM*)param);
		case ML_IPC_NAVITEM_GETNEXT:		return (INT_PTR)OnIPC_MLNavItem_GetNext((HNAVITEM)param);
		case ML_IPC_NAVITEM_GETORDER:		return (INT_PTR)OnIPC_MLNavItem_GetOrder((HNAVITEM)param);
		case ML_IPC_NAVITEM_GETPARENT:		return (INT_PTR)OnIPC_MLNavItem_GetParent((HNAVITEM)param);
		case ML_IPC_NAVITEM_GETPREVIOUS:	return (INT_PTR)OnIPC_MLNavItem_GetPrevious((HNAVITEM)param);
		case ML_IPC_NAVITEM_GETRECT:		return (INT_PTR)OnIPC_MLNavItem_GetRect((NAVITEMGETRECT*)param);
		case ML_IPC_NAVITEM_GETSTATE:		return (INT_PTR)OnIPC_MLNavItem_GetState((HNAVITEM)param);
		case ML_IPC_NAVITEM_HASCHILDRENREAL:return (INT_PTR)OnIPC_MLNavItem_HasChildrenReal((HNAVITEM)param);
		case ML_IPC_NAVITEM_INVALIDATE:		return (INT_PTR)OnIPC_MLNavItem_Invalidate((NAVITEMINAVLIDATE*)param);
		case ML_IPC_NAVITEM_MOVE:			return (INT_PTR)OnIPC_MLNavItem_Move((NAVITEMMOVE*)param);
		case ML_IPC_NAVITEM_SELECT:			return (INT_PTR)OnIPC_MLNavItem_Select((HNAVITEM)param);
		case ML_IPC_NAVITEM_SETINFO:		return (INT_PTR)OnIPC_MLNavItem_SetInfo((NAVITEM*)param);
		case ML_IPC_NAVITEM_SETORDER:		return (INT_PTR)OnIPC_MLNavItem_SetOrder((NAVITEMORDER*)param);

		case ML_IPC_RATING_DRAW:			return (INT_PTR)OnIPC_MLRating_Draw((RATINGDRAWPARAMS*)param);
		case ML_IPC_RATING_HITTEST:			return (INT_PTR)OnIPC_MLRating_Hittest((RATINGHITTESTPARAMS*)param);
		case ML_IPC_RATING_CALCRECT:		return (INT_PTR)OnIPC_MLRating_CalcRect((RECT*)param);

		case ML_IPC_CLOUD_DRAW:				return (INT_PTR)OnIPC_MLCloud_Draw((CLOUDDRAWPARAMS*)param);
		case ML_IPC_CLOUD_CALCRECT:			return (INT_PTR)OnIPC_MLCloud_CalcRect((RECT*)param);

		case ML_IPC_SKINWINDOW:				return (INT_PTR)OnIPC_SkinWindow((MLSKINWINDOW*)param);
		case ML_IPC_UNSKINWINDOW:			return (INT_PTR)OnIPC_UnskinWindow((HWND)param);

		case ML_IPC_TRACKSKINNEDPOPUPEX:	return (INT_PTR)OnIPC_TrackSkinnedPopup((MLSKINNEDPOPUP*)param);
		case ML_IPC_INITSKINNEDPOPUPHOOK:	return (INT_PTR)OnIPC_InitSkinnedPopupHook((MLSKINNEDPOPUP*)param);
		case ML_IPC_REMOVESKINNEDPOPUPHOOK:	return (INT_PTR)OnIPC_RemoveSkinnedPopupHook((HANDLE)param);

		case ML_IPC_RATINGCOLUMN_PAINT:		return (INT_PTR)OnIPC_RatingColumn_Paint((RATINGCOLUMNPAINT*)param);
		case ML_IPC_RATINGCOLUMN_CLICK:		return (INT_PTR)OnIPC_RatingColumn_OnClick((RATINGCOLUMN*)param);
		case ML_IPC_RATINGCOLUMN_TRACK:		return (INT_PTR)OnIPC_RatingColumn_OnTrack((RATINGCOLUMN*)param);
		case ML_IPC_RATINGCOLUMN_CANCELTRACKING:	return (INT_PTR)OnIPC_RatingColumn_CancelTracking((BOOL)param);
		case ML_IPC_RATINGCOLUMN_BEGINDRAG:	return (INT_PTR)OnIPC_RatingColumn_BeginDrag((RATINGCOLUMN*)param);
		case ML_IPC_RATINGCOLUMN_DRAG:		return (INT_PTR)OnIPC_RatingColumn_OnDrag((POINT*)param);
		case ML_IPC_RATINGCOLUMN_ENDDRAG:	return (INT_PTR)OnIPC_RatingColumn_OnEndDrag((RATINGCOLUMN*)param);
		case ML_IPC_RATINGCOLUMN_ANIMATE:	return (INT_PTR)OnIPC_RatingColumn_Animate((RATINGCOLUMN*)param);
		case ML_IPC_RATINGCOLUMN_GETMINWIDTH:	return (INT_PTR)OnIPC_RatingColumn_GetMinWidth();
		case ML_IPC_RATINGCOLUMN_FILLBACKSTRING:	return (INT_PTR)OnIPC_RatingColumn_FillBackString((RATINGBACKTEXT*)param);
		case ML_IPC_RATINGCOLUMN_GETWIDTH:	return  (INT_PTR)OnIPC_RatingColumn_GetWidth((RATINGWIDTH*)param);

		case ML_IPC_CLOUDCOLUMN_PAINT:		return (INT_PTR)OnIPC_CloudColumn_Paint((RATINGCOLUMNPAINT*)param);
		case ML_IPC_CLOUDCOLUMN_GETMINWIDTH:	return (INT_PTR)OnIPC_CloudColumn_GetMinWidth();
		case ML_IPC_CLOUDCOLUMN_GETWIDTH:	return  (INT_PTR)OnIPC_CloudColumn_GetWidth((INT*)param);

		case ML_IPC_CREATEWEBINFO:			return (INT_PTR)OnIPC_CreateWebInfo((WEBINFOCREATE*)param);
		case ML_IPC_CREATEFILEVIEW:			return (INT_PTR)OnIPC_CreateFileView((MLFILEVIEWCREATESTRUCT*)param);
	}
	return 0;
}