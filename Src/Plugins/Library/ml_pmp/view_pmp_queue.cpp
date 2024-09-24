/* almost the same as view_pmp_devices, but only one device*/
#include "main.h"
#include <windows.h> 
#include <windowsx.h>
#include <stdio.h>
#include <shlobj.h>
#include "..\..\General\gen_ml/ml.h"
#include "..\..\General\gen_ml/itemlist.h"
#include "../nu/listview.h"
#include "..\..\General\gen_ml/childwnd.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/wa_dlg.h"
#include "resource1.h"
#include "SkinnedListView.h"
#include "DeviceView.h"
#include "api__ml_pmp.h"
#include "./graphics.h"
#include <strsafe.h>
#include "../nu/AutoWide.h"

static HRGN g_rgnUpdate = NULL;
static int offsetX, offsetY, customAllowed;
static DeviceView *device;
static SkinnedListView * listTransfers=NULL;
std::map<DeviceView *, bool> device_update_map;

typedef struct TransferView
{
	HFONT font;
	SIZE unitSize;
	HRGN updateRegion;
	POINT updateOffset;
} TransferView;

#define TRANSFERVIEW(_hwnd) ((TransferView*)GetViewData(_hwnd))
#define TRANSFERVIEW_RET_VOID(_self, _hwnd) { (_self) = TRANSFERVIEW((_hwnd)); if (NULL == (_self)) return; }
#define TRANSFERVIEW_RET_VAL(_self, _hwnd, _error) { (_self) = TRANSFERVIEW((_hwnd)); if (NULL == (_self)) return (_error); }

#define TRANSFERVIEW_DLU_TO_HORZ_PX(_self, _dlu) MulDiv((_dlu), (_self)->unitSize.cx, 4)
#define TRANSFERVIEW_DLU_TO_VERT_PX(_self, _dlu) MulDiv((_dlu), (_self)->unitSize.cy, 8)

void handleContextMenuResult(int r, C_ItemList * items=NULL, DeviceView * dev=NULL);
int showContextMenu(int context,HWND hwndDlg, Device * dev, POINT pt);

class TransferItemShadowShort
{
public:
	CopyInst * c;
	wchar_t * status, * type, * track, * sourceDevice, * destDevice, * lastChanged, * sourceFile, * clientType;
	bool changed;
	TransferItemShadowShort(CopyInst * c)
	{
		changed = false;
		this->c = c;
		status = _wcsdup(c->statusCaption);
		type = _wcsdup(c->typeCaption);
		track = _wcsdup(c->trackCaption);
		lastChanged = _wcsdup(c->lastChanged);
		sourceDevice = _wcsdup(c->sourceDevice);
		destDevice = _wcsdup(c->destDevice);
		sourceFile = _wcsdup(c->sourceFile);
		clientType = AutoWideDup(c->dev->GetConnection());
	}
	~TransferItemShadowShort()
	{
		if (status) free(status);
		if (type) free(type);
		if (track) free(track);
		if (lastChanged) free(lastChanged);
		if (sourceDevice) free(sourceDevice);
		if (destDevice) free(destDevice);
		if (sourceFile) free(sourceFile);
		if (clientType) free(clientType);
	}
	bool Equals(TransferItemShadowShort * a)
	{
		if (!a) return false;
		return (c == a->c) && !wcscmp(track,a->track) && !wcscmp(status,a->status) && !wcscmp(type,a->type);
	}
};

LinkedQueue *getTransferQueue(DeviceView *deviceView)
{
	if (deviceView)
	{
		return (deviceView->isCloudDevice ? &cloudTransferQueue : &deviceView->transferQueue);
	}
	else if (device)
	{
		return (device->isCloudDevice ? &cloudTransferQueue : &device->transferQueue);
	}
	return NULL;
}

LinkedQueue *getFinishedTransferQueue(DeviceView *deviceView)
{
	if (deviceView)
	{
		return (deviceView->isCloudDevice ? &cloudFinishedTransfers : &deviceView->finishedTransfers);
	}
	else if (device)
	{
		return (device->isCloudDevice ? &cloudFinishedTransfers : &device->finishedTransfers);
	}
	return NULL;
}

int getTransferProgress(DeviceView *deviceView)
{
	if(deviceView)
	{
		return (deviceView->isCloudDevice ? cloudTransferProgress : deviceView->currentTransferProgress);
	}
	else if(device)
	{
		return (device->isCloudDevice ? cloudTransferProgress : device->currentTransferProgress);
	}
	return 0;
}

// TODO hook up to a config (would need to work after a reset to be 100% safe...?)
int sharedQueue = 1;
static C_ItemList *getTransferListShadow()
{
	C_ItemList * list = new C_ItemList;
	if (!sharedQueue)
	{
		LinkedQueue * txQueue = getTransferQueue();
		if (txQueue)
		{
			txQueue->lock();
			for (int j = 0; j < txQueue->GetSize(); j++)
				list->Add(new TransferItemShadowShort((CopyInst*)txQueue->Get(j)));
			txQueue->unlock();
		}

		LinkedQueue * finishedTX = getFinishedTransferQueue();
		if (finishedTX)
		{
			finishedTX->lock();
			for (int j = 0; j < finishedTX->GetSize(); j++)
				list->Add(new TransferItemShadowShort((CopyInst*)finishedTX->Get(j)));
			finishedTX->unlock();
		}
	}
	else
	{
		// TODO should probably review this when we're more intelligent on multi-handling
		// we do the cloud transfer queue specifically so as not to duplicate by devices
		LinkedQueue * txQueue = &cloudTransferQueue;
		if (txQueue)
		{
			txQueue->lock();
			for (int j = 0; j < txQueue->GetSize(); j++)
				list->Add(new TransferItemShadowShort((CopyInst*)txQueue->Get(j)));
			txQueue->unlock();
		}

		LinkedQueue * finishedTX = &cloudFinishedTransfers;
		if (finishedTX)
		{
			finishedTX->lock();
			for (int j = 0; j < finishedTX->GetSize(); j++)
				list->Add(new TransferItemShadowShort((CopyInst*)finishedTX->Get(j)));
			finishedTX->unlock();
		}

		for (int i = 0; i < devices.GetSize(); i++)
		{
			DeviceView * d = (DeviceView *)devices.Get(i);
			if (!d || d->isCloudDevice) continue;

			LinkedQueue * txQueue = getTransferQueue(d);
			if (txQueue)
			{
				txQueue->lock();
				for (int j = 0; j < txQueue->GetSize(); j++)
					list->Add(new TransferItemShadowShort((CopyInst*)txQueue->Get(j)));
				txQueue->unlock();
			}

			LinkedQueue * finishedTX = getFinishedTransferQueue(d);
			if (finishedTX)
			{
				finishedTX->lock();
				for (int j = 0; j < finishedTX->GetSize(); j++)
					list->Add(new TransferItemShadowShort((CopyInst*)finishedTX->Get(j)));
				finishedTX->unlock();
			}
		}
	}
	return list;
}

class TransferContents : public ListContents
{
public:
	TransferContents()
	{
		oldSize = 0;
		listShadow = 0;
		InitializeCriticalSection(&cs);
	}

	void Init()
	{
		lock();

		if (!listShadow)
			listShadow = getTransferListShadow();

		unlock();
	}

	virtual ~TransferContents()
	{
		DeleteCriticalSection(&cs); 
		delete listShadow;
	}

	virtual int GetNumColumns()
	{
		// TODO check the number of columns are ok, etc
		return 7;//(device->isCloudDevice ? 7 : 3);
	}

	virtual int GetNumRows()
	{
		return (listShadow ? listShadow->GetSize() : 0);
	}

	virtual wchar_t * GetColumnTitle(int num)
	{
		// TODO need to clean this up as needed
		switch (num + 1)
		{
			case 0: return WASABI_API_LNGSTRINGW(IDS_TYPE);
			case 1: return WASABI_API_LNGSTRINGW(IDS_TRACK);
			case 2: return WASABI_API_LNGSTRINGW(IDS_STATUS);
			case 3: return WASABI_API_LNGSTRINGW(IDS_LAST_CHANGED);
			case 4: return WASABI_API_LNGSTRINGW(IDS_SOURCE);
			case 5: return WASABI_API_LNGSTRINGW(IDS_DESTINATION);
			case 6: return WASABI_API_LNGSTRINGW(IDS_SOURCE_FILE);
			case 7: return WASABI_API_LNGSTRINGW(IDS_CLIENT_TYPE);
		}
		return L"";
	}

	virtual int GetColumnWidth(int num)
	{
		switch (num)
		{
			case 0: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col0_width" : L"cloud_col0_width"), 300);
			case 1: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col1_width" : L"cloud_col1_width"), 150);
			case 2: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col2_width" : L"cloud_col2_width"), 100);
			case 3: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col3_width" : L"cloud_col3_width"), 100);
			case 4: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col4_width" : L"cloud_col4_width"), 100);
			case 5: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col5_width" : L"cloud_col5_width"), 100);
			case 6: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col6_width" : L"cloud_col6_width"), 300);
			case 7: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col7_width" : L"cloud_col7_width"), 100);
			default: return 0;
		}
	}

	virtual void ColumnResize(int col, int newWidth)
	{
		if (NULL != global_config &&
			col >= 0 && 
			col < GetNumColumns())
		{
			wchar_t buffer[64] = {0};

			if (FAILED(StringCchPrintf(buffer, ARRAYSIZE(buffer), (!device->isCloudDevice ? L"transfers_col%d_width" : L"cloud_col%d_width"), col)))
				return;

			global_config->WriteInt(buffer, newWidth);
		}
	}

	void lock()
	{
		EnterCriticalSection(&cs);
	}

	void unlock()
	{
		LeaveCriticalSection(&cs);
	}

	virtual void GetCellText(int row, int col, wchar_t * buf, int buflen)
	{
		if (NULL == buf)
			return;

		buf[0] = L'\0';

		lock();

		if (row < listShadow->GetSize())
		{
			TransferItemShadowShort * t = (TransferItemShadowShort *)listShadow->Get(row);
			if (NULL != t)
			{
				// TODO need to clean this up as needed
				switch (col + 1)
				{
					case 0: /*StringCchCopy(buf, buflen, t->type);*/ break;
					case 1: StringCchCopy(buf, buflen, t->track); break;
					case 2: StringCchCopy(buf, buflen, t->status); break;
					case 3: StringCchCopy(buf, buflen, t->lastChanged); break;
					case 4: StringCchCopy(buf, buflen, t->sourceDevice); break;
					case 5: StringCchCopy(buf, buflen, t->destDevice); break;
					case 6: StringCchCopy(buf, buflen, t->sourceFile); break;
					case 7: StringCchCopy(buf, buflen, t->clientType); break;
				}
			}
		}
		unlock();
	}

	void PushPopItem(CopyInst *c)
	{
		lock();
		int size = listShadow->GetSize();
		for (int i=0; i<size; i++)
		{
			TransferItemShadowShort * t = (TransferItemShadowShort *)listShadow->Get(i);
			if (c == t->c)
			{
				t->changed=true;
				listShadow->Del(i);
				listShadow->Add(t);
				if (listTransfers)
				{
					HWND hwnd = listTransfers->listview.getwnd();
					PostMessage(hwnd, LVM_REDRAWITEMS, i, size);
				}
				unlock();
				return;
			}
		}
		unlock();
	}

	void ItemUpdate(CopyInst * c)
	{
		lock();
		int size = listShadow->GetSize();
		for (int i=0; i<size; i++)
		{
			TransferItemShadowShort * t = (TransferItemShadowShort *)listShadow->Get(i);
			if (c == t->c)
			{
				TransferItemShadowShort * n = new TransferItemShadowShort(c);
				n->changed=true;
				listShadow->Set(i,n);
				delete t;
				if (listTransfers)
				{
					HWND hwnd = listTransfers->listview.getwnd();
					PostMessage(hwnd,LVM_REDRAWITEMS,i,i);
				}
				unlock();
				return;
			}
		}
		unlock();
	}

	void FullUpdate()
	{
		C_ItemList * newListShadow = getTransferListShadow();
		if (newListShadow)
		{
			int newSize = newListShadow->GetSize();
			lock();
			oldSize = listShadow->GetSize();
			for (int i = 0; i < newSize; i++)
			{
				TransferItemShadowShort * newt = (TransferItemShadowShort *)newListShadow->Get(i);
				TransferItemShadowShort * oldt = i < oldSize ? (TransferItemShadowShort *)listShadow->Get(i) : NULL;
				newt->changed = !newt->Equals(oldt);
			}

			C_ItemList * oldListShadow = listShadow;
			listShadow = newListShadow;
			for (int i = 0; i < oldListShadow->GetSize(); i++) delete(TransferItemShadowShort *)oldListShadow->Get(i);
			delete oldListShadow;
			if (listTransfers)
			{
				HWND hwnd = listTransfers->listview.getwnd();
				if (newSize != oldSize) PostMessage(hwnd,LVM_SETITEMCOUNT,newSize, 0);
				for (int i=0; i<newSize; i++)
				{
					TransferItemShadowShort * t = (TransferItemShadowShort *)listShadow->Get(i);
					if (t->changed) PostMessage(hwnd,LVM_REDRAWITEMS,i,i);
				}
			}
			unlock();
		}
	}

	virtual songid_t GetTrack(int pos) { return 0; }

private:
	CRITICAL_SECTION cs;
	C_ItemList * listShadow;
	int oldSize;
};

static TransferContents transferListContents;

static void updateStatus(HWND hwnd)
{	
	HWND statusWindow = GetDlgItem(hwnd, IDC_STATUS);
	if (NULL == statusWindow)
		return;

	int txProgress = getTransferProgress(device);
	LinkedQueue * txQueue = getTransferQueue(device);
	LinkedQueue * finishedTX = getFinishedTransferQueue(device);
	int size = (txQueue ? txQueue->GetSize() : 0);
	if (size > 0)
	{
		wchar_t buffer[256] = {0}, format[256] = {0};
		int pcnum, time, pc, total;

		pcnum = (size * 100) - txProgress;
		total = size * 100;
		total += 100 * (finishedTX ? finishedTX->GetSize() : 0);
	
		time = (int)(device->transferRate * (((double)pcnum)/100.0));
		pc = ((total-pcnum)*100)/total;

		WASABI_API_LNGSTRINGW_BUF((time > 0 ? IDS_TRANFERS_PERCENT_REMAINING : IDS_TRANFERS_PERCENT_REMAINING_NOT_TIME), format, ARRAYSIZE(format));
		if (SUCCEEDED(StringCchPrintf(buffer, ARRAYSIZE(buffer), format, size, pc, time/60, time%60)))
		{
			if (0 == SendMessage(statusWindow, WM_GETTEXT, (WPARAM)ARRAYSIZE(format), (LPARAM)format) ||
				CSTR_EQUAL != CompareString(LOCALE_USER_DEFAULT, 0, format, -1, buffer, -1))
			{		
				SendMessage(statusWindow, WM_SETTEXT, 0, (LPARAM)buffer);
			}
		}
	}
	else 
	{
		int length = (int)SendMessage(statusWindow, WM_GETTEXTLENGTH, 0, 0L);
		if (0 != length)
			SendMessage(statusWindow, WM_SETTEXT, 0, 0L);
	}
}

void TransfersListUpdateItem(CopyInst * item, DeviceView *view)
{
	if (view == device)
		transferListContents.ItemUpdate(item);
}

void TransfersListPushPopItem(CopyInst * item, DeviceView *view)
{
	if (view == device)
		transferListContents.PushPopItem(item);
}

static bool AddSelectedItems(C_ItemList *items, W_ListView *listview, LinkedQueue *transfer_queue, int &row, DeviceView *&dev)
{
	transfer_queue->lock();
	int l = transfer_queue->GetSize();
	for (int j=0; j<l; j++)
	{
		if (listview->GetSelected(row++))
		{
			CopyInst * c = (CopyInst *)transfer_queue->Get(j);
			if (c->songid)
			{
				if (!dev && c->dev) 
					dev = c->dev;
				if (dev)
				{
					if (c->dev != dev)
					{
						transfer_queue->unlock();
						return false;
					}
					else
						items->Add((void*)c->songid);
				}
			}
		}
	}
	transfer_queue->unlock();
	return true;
}

static void RemoveSelectedItems(DeviceView *device, W_ListView *listview, LinkedQueue *transfer_queue, int &row, bool finished_queue)
{
	transfer_queue->lock();
	int j = transfer_queue->GetSize();
	while (j-- > 0)
	{
		if (listview->GetSelected(--row))
		{
			if (j == 0 && !finished_queue)
			{
				CopyInst * d = (CopyInst *)transfer_queue->Get(j);
				if (d && (d->status == STATUS_WAITING || d->status == STATUS_CANCELLED) && device->transferContext.IsPaused())
				{
					transfer_queue->Del(j);
					d->Cancelled();
					delete d;
				} // otherwise don't bother
			}
			else
			{
				CopyInst * d = (CopyInst*)transfer_queue->Del(j);
				if (d)
				{
					if ((d->status == STATUS_WAITING || d->status == STATUS_CANCELLED) && !finished_queue)
						d->Cancelled();
					delete d;
				}
			}
		}
	}
	transfer_queue->unlock();
}

static void CancelSelectedItems(DeviceView *device, W_ListView *listview, LinkedQueue *transfer_queue, int &row)
{
	transfer_queue->lock();
	int j = transfer_queue->GetSize();
	int sel = listview->GetSelectedCount();

	for (int i = 0, q = 0; i <= j; i++)
	{
		if (listview->GetSelected(i) || !sel)
		{
			CopyInst * d = (CopyInst *)transfer_queue->Get(q);
			if (d && d->status == STATUS_WAITING)
			{
				transfer_queue->Del(q);
				d->Cancelled();
				delete d;
			}
			else if (d && d->status == STATUS_TRANSFERRING)
			{
				d->Cancelled();
			}
			else
			{
				q++;
			}
		}
	}

	transfer_queue->unlock();
}

static void RetrySelectedItems(DeviceView *device, W_ListView *listview, LinkedQueue *transfer_queue, LinkedQueue *finished_transfer_queue, int &row)
{
	transfer_queue->lock();
	finished_transfer_queue->lock();
	int j = finished_transfer_queue->GetSize();
	int sel = listview->GetSelectedCount();

	LinkedQueue retryTransferQueue;
	int i = 0;
	for (int q = 0; i <= j; i++)
	{
		if (listview->GetSelected(i) || !sel)
		{
			CopyInst * d = (CopyInst *)finished_transfer_queue->Get(q);
			if (d && (d->status == STATUS_DONE || d->status == STATUS_CANCELLED || d->status == STATUS_ERROR))
			{
				// due to STATUS_DONE being applied in most cases, have to look at the
				// status message and use as the basis on how to proceed with the item
				if (lstrcmpi(d->statusCaption, WASABI_API_LNGSTRINGW(IDS_UPLOADED)))
				{
					retryTransferQueue.Offer(d);
					finished_transfer_queue->Del(q);
				}
				else
				{
					q++;
				}
			}
			else
			{
				q++;
			}
		}
	}

	i = 0;
	for (; i <= retryTransferQueue.GetSize(); i++)
	{
		CopyInst * d = (CopyInst *)retryTransferQueue.Get(i);
		if (d)
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_WAITING, d->statusCaption, sizeof(d->statusCaption)/sizeof(wchar_t));

			SYSTEMTIME system_time;
			GetLocalTime(&system_time);
			GetTimeFormat(LOCALE_INVARIANT, NULL, &system_time, NULL, d->lastChanged, sizeof(d->lastChanged)/sizeof(wchar_t));

			d->dev->AddTrackToTransferQueue(d);
		}
	}

	transfer_queue->unlock();
	finished_transfer_queue->unlock();
}

typedef struct _LAYOUT
{
	INT		id;
	HWND		hwnd;
	INT		x;
	INT		y;
	INT		cx;
	INT		cy;
	DWORD	flags;
	HRGN	rgn;
}
LAYOUT, PLAYOUT;

#define SETLAYOUTPOS(_layout, _x, _y, _cx, _cy) { _layout->x=_x; _layout->y=_y;_layout->cx=_cx;_layout->cy=_cy;_layout->rgn=NULL; }
#define SETLAYOUTFLAGS(_layout, _r)																						\
	{																													\
		BOOL fVis;																										\
		fVis = (WS_VISIBLE & (LONG)GetWindowLongPtr(_layout->hwnd, GWL_STYLE));											\
		if (_layout->x == _r.left && _layout->y == _r.top) _layout->flags |= SWP_NOMOVE;									\
		if (_layout->cx == (_r.right - _r.left) && _layout->cy == (_r.bottom - _r.top)) _layout->flags |= SWP_NOSIZE;	\
		if ((SWP_HIDEWINDOW & _layout->flags) && !fVis) _layout->flags &= ~SWP_HIDEWINDOW;								\
		if ((SWP_SHOWWINDOW & _layout->flags) && fVis) _layout->flags &= ~SWP_SHOWWINDOW;									\
	}

#define LAYOUTNEEEDUPDATE(_layout) ((SWP_NOMOVE | SWP_NOSIZE) != ((SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_SHOWWINDOW) & _layout->flags))

#define GROUP_MIN			0x1
#define GROUP_MAX			0x2
#define GROUP_STATUSBAR		0x1
#define GROUP_MAIN			0x2

/*
IDC_BUTTON_PAUSETRANSFERS,
IDC_BUTTONCANCELSELECTED,
IDC_BUTTON_CLEARFINISHED,
IDC_BUTTON_REMOVESELECTED,
IDC_BUTTON_RETRYSELECTED,
IDC_STATUS,
IDC_LIST_TRANSFERS,
*/

static void TransferView_UpdateLayout(HWND hwnd, BOOL fRedraw, BOOL fUpdateAll = FALSE)
{
	static INT controls[] =
	{
		GROUP_STATUSBAR, IDC_BUTTON_PAUSETRANSFERS, IDC_BUTTON_CLEARFINISHED, IDC_BUTTON_REMOVESELECTED, IDC_BUTTON_RETRYSELECTED, IDC_STATUS,
		GROUP_MAIN, IDC_LIST_TRANSFERS
	};

	INT index;
	RECT rc, rg, ri;
	LAYOUT layout[sizeof(controls)/sizeof(controls[0])], *pl;
	BOOL skipgroup;
	HRGN rgn = NULL;

	GetClientRect(hwnd, &rc);
	if (rc.bottom == rc.top || rc.right == rc.left) return;

	SetRect(&rg, rc.left, rc.top, rc.right, rc.bottom);

	pl = layout;
	skipgroup = FALSE;

	InvalidateRect(hwnd, NULL, TRUE);

	for (index = 0; index < sizeof(controls) / sizeof(*controls); index++)
	{
		if (controls[index] >= GROUP_MIN && controls[index] <= GROUP_MAX) // group id
		{
			skipgroup = FALSE;
			switch (controls[index])
			{
			case GROUP_STATUSBAR:
				{
					wchar_t buffer[128] = {0};
					HWND ctrl = GetDlgItem(hwnd, IDC_BUTTON_PAUSETRANSFERS);
					GetWindowTextW(ctrl, buffer, ARRAYSIZE(buffer));
					LRESULT idealSize = MLSkinnedButton_GetIdealSize(ctrl, buffer);

					SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1),
							rc.bottom - WASABI_API_APP->getScaleY(HIWORD(idealSize)),
							rc.right, rc.bottom);
					rc.bottom = rg.top - WASABI_API_APP->getScaleY(3);
				}
				break;
			case GROUP_MAIN:
				SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1), rc.top, rc.right, rc.bottom);
				break;
			}
			continue;
		}
		if (skipgroup) continue;

		pl->id = controls[index];
		pl->hwnd = GetDlgItem(hwnd, pl->id);
		if (!pl->hwnd) continue;

		GetWindowRect(pl->hwnd, &ri);
		MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&ri, 2);
		pl->flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW  | SWP_NOCOPYBITS;

		switch (pl->id)
		{
			case IDC_BUTTON_PAUSETRANSFERS:
			case IDC_BUTTON_CLEARFINISHED:
			case IDC_BUTTON_REMOVESELECTED:
			case IDC_BUTTON_RETRYSELECTED:
		{
				wchar_t buffer[128] = {0};
				GetWindowTextW(pl->hwnd, buffer, ARRAYSIZE(buffer));
				LRESULT idealSize = MLSkinnedButton_GetIdealSize(pl->hwnd, buffer);
				LONG width = LOWORD(idealSize) + WASABI_API_APP->getScaleX(6);
				SETLAYOUTPOS(pl, rg.left, rg.bottom - WASABI_API_APP->getScaleY(HIWORD(idealSize)),
							 width, WASABI_API_APP->getScaleY(HIWORD(idealSize)));
				pl->flags |= ((rg.right - rg.left) > width) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				if (SWP_SHOWWINDOW & pl->flags) rg.left += (pl->cx + WASABI_API_APP->getScaleX(4));
				break;
			}
			case IDC_STATUS:
				SETLAYOUTPOS(pl, rg.left, rg.top, rg.right - rg.left, (rg.bottom - rg.top));
				pl->flags |= (pl->cx > WASABI_API_APP->getScaleX(16)) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				break;
			case IDC_LIST_TRANSFERS:
				SETLAYOUTPOS(pl, rg.left, rg.top + 1, (rg.right - rg.left) - WASABI_API_APP->getScaleX(2), (rg.bottom - rg.top) - WASABI_API_APP->getScaleY(1));
				break;
		}

		SETLAYOUTFLAGS(pl, ri);
		if (LAYOUTNEEEDUPDATE(pl))
		{
			if (SWP_NOSIZE == ((SWP_HIDEWINDOW | SWP_SHOWWINDOW | SWP_NOSIZE) & pl->flags) &&
			    ri.left == (pl->x + offsetX) && ri.top == (pl->y + offsetY) && !fUpdateAll && IsWindowVisible(pl->hwnd))
			{
				SetRect(&ri, pl->x, pl->y, pl->cx + pl->x, pl->y + pl->cy);
				ValidateRect(hwnd, &ri);
			}

			pl++;
		}
		else if (!fUpdateAll && (fRedraw || (!offsetX && !offsetY)) && IsWindowVisible(pl->hwnd))
		{
			ValidateRect(hwnd, &ri);
			if (GetUpdateRect(pl->hwnd, NULL, FALSE))
			{
				if (!rgn) rgn = CreateRectRgn(0,0,0,0);
				GetUpdateRgn(pl->hwnd, rgn, FALSE);
				OffsetRgn(rgn, pl->x, pl->y);
				InvalidateRgn(hwnd, rgn, FALSE);
			}
		}
	}

	if (pl != layout)
	{
		LAYOUT *pc;
		HDWP hdwp = BeginDeferWindowPos((INT)(pl - layout));
		for (pc = layout; pc < pl && hdwp; pc++)
		{
			hdwp = DeferWindowPos(hdwp, pc->hwnd, NULL, pc->x, pc->y, pc->cx, pc->cy, pc->flags);
		}
		if (hdwp) EndDeferWindowPos(hdwp);

		if (!rgn) rgn = CreateRectRgn(0, 0, 0, 0);

		if (fRedraw)
		{
			GetUpdateRgn(hwnd, rgn, FALSE);
			for (pc = layout; pc < pl && hdwp; pc++)
			{
				if (pc->rgn)
				{
					OffsetRgn(pc->rgn, pc->x, pc->y);
					CombineRgn(rgn, rgn, pc->rgn, RGN_OR);
				}
			}
			RedrawWindow(hwnd, NULL, rgn, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
		}

		if (g_rgnUpdate)
		{
			GetUpdateRgn(hwnd, g_rgnUpdate, FALSE);
			for (pc = layout; pc < pl && hdwp; pc++)
			{
				if (pc->rgn)
				{
					OffsetRgn(pc->rgn, pc->x, pc->y);
					CombineRgn(g_rgnUpdate, g_rgnUpdate, pc->rgn, RGN_OR);
				}
			}
		}

		for (pc = layout; pc < pl && hdwp; pc++)
			if (pc->rgn) DeleteObject(pc->rgn);
	}
	if (rgn) DeleteObject(rgn);
	ValidateRgn(hwnd, NULL);
}

static void
TransferView_UpdateFont(HWND hwnd, BOOL redraw)
{
	TransferView *self;
	HWND elementWindow;
	HDWP hdwp;

	const int buttonList[] = 
	{
		IDC_BUTTON_PAUSETRANSFERS,
		IDC_BUTTONCANCELSELECTED,
		IDC_BUTTON_CLEARFINISHED,
		IDC_BUTTON_REMOVESELECTED,
		IDC_BUTTON_RETRYSELECTED
	};

	TRANSFERVIEW_RET_VOID(self, hwnd);

	if (FALSE == Graphics_GetWindowBaseUnits(hwnd, &self->unitSize.cx, &self->unitSize.cy))
	{
		self->unitSize.cx = 6;
		self->unitSize.cy = 13;
	}	

	elementWindow = GetDlgItem(hwnd, IDC_LIST_TRANSFERS);
	if (NULL != elementWindow)
	{
		elementWindow = (HWND)SendMessage(elementWindow, LVM_GETHEADER, 0, 0L);
		if (NULL != elementWindow)
			MLSkinnedHeader_SetHeight(elementWindow, -1);
	}

	hdwp = BeginDeferWindowPos(ARRAYSIZE(buttonList) + 1);
	if (NULL != hdwp)
	{
		LRESULT idealSize;
		SIZE buttonSize;

		elementWindow = GetDlgItem(hwnd, IDC_STATUS);
		if (NULL != elementWindow)
		{
			hdwp = DeferWindowPos(hdwp, elementWindow, NULL, 0, 0, 100, self->unitSize.cy,
							SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
		}

		for(size_t index = 0; index < ARRAYSIZE(buttonList) && NULL != hdwp; index++)
		{
			elementWindow = GetDlgItem(hwnd, buttonList[index]);
			if (NULL == elementWindow)
				continue;

			if (IDC_BUTTON_PAUSETRANSFERS == buttonList[index])
			{
				wchar_t buffer[128] = {0};

				WASABI_API_LNGSTRINGW_BUF(IDS_RESUME, buffer, ARRAYSIZE(buffer));
				idealSize = MLSkinnedButton_GetIdealSize(elementWindow, buffer);
				buttonSize.cx = LOWORD(idealSize);
				buttonSize.cy = HIWORD(idealSize);

				WASABI_API_LNGSTRINGW_BUF(IDS_PAUSE, buffer, ARRAYSIZE(buffer));
				idealSize = MLSkinnedButton_GetIdealSize(elementWindow, buffer);

				if (buttonSize.cx < LOWORD(idealSize))
					buttonSize.cx = LOWORD(idealSize);

				if (buttonSize.cy < HIWORD(idealSize))
					buttonSize.cy = HIWORD(idealSize);
			}
			else
			{
				idealSize = MLSkinnedButton_GetIdealSize(elementWindow, NULL);
				buttonSize.cx = LOWORD(idealSize);
				buttonSize.cy = HIWORD(idealSize);
			}

			hdwp = DeferWindowPos(hdwp, elementWindow, NULL, 0, 0, buttonSize.cx, buttonSize.cy,
							SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
		}

		if (NULL != hdwp)
			EndDeferWindowPos(hdwp);
	}

	TransferView_UpdateLayout(hwnd, redraw);
}

static int 
TransferView_OnInitDialog(HWND hwnd, HWND focusWindow, LPARAM param)
{
	HWND controlWindow;

	const int skinList[] = 
	{
		IDC_BUTTON_PAUSETRANSFERS,
		IDC_BUTTONCANCELSELECTED,
		IDC_BUTTON_CLEARFINISHED,
		IDC_BUTTON_REMOVESELECTED,
		IDC_BUTTON_RETRYSELECTED,
		IDC_STATUS,
	};

	TransferView *self = (TransferView*)calloc(1, sizeof(TransferView));
	if (NULL != self) 
	{
		if (FALSE == SetViewData(hwnd, self))
		{
			free(self);
			self = NULL;
		}
	}

	if (NULL == self)
	{
		DestroyWindow(hwnd);
		return 0;
	}

	device = (DeviceView *)param;

	transferListContents.Init();
	transferListContents.lock();

	transferListContents.FullUpdate();

	listTransfers = new SkinnedListView(&transferListContents,IDC_LIST_TRANSFERS,plugin.hwndLibraryParent, hwnd);
	listTransfers->DialogProc(hwnd,WM_INITDIALOG, (WPARAM)focusWindow, param);

	transferListContents.unlock();
	SetDlgItemText(hwnd,IDC_BUTTON_PAUSETRANSFERS,
				   WASABI_API_LNGSTRINGW((device->transferContext.IsPaused()?IDS_RESUME:IDS_PAUSE)));

	MLSkinWindow2(plugin.hwndLibraryParent, hwnd, SKINNEDWND_TYPE_DIALOG,
				  SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	for(size_t index = 0; index < ARRAYSIZE(skinList); index++)
	{
		controlWindow = GetDlgItem(hwnd, skinList[index]);
		if (NULL != controlWindow)
		{
			MLSkinWindow2(plugin.hwndLibraryParent, controlWindow,
						  (skinList[index] != IDC_STATUS ? SKINNEDWND_TYPE_BUTTON : SKINNEDWND_TYPE_STATIC),
						  SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);
		}
	}

	TransferView_UpdateFont(hwnd, FALSE);

	SetTimer(hwnd,1,500,NULL);
	updateStatus(hwnd);

	return 0;
}

static void
TransferView_OnDestroy(HWND hwnd)
{
	TransferView *self;

	self = (TransferView *)RemoveViewData(hwnd);
	if (NULL == self)
		return;

	KillTimer(hwnd, 1);
	device = 0;

	SkinnedListView * lt = listTransfers;
	if (NULL != lt)
	{
		transferListContents.lock();
		listTransfers=NULL;
		transferListContents.unlock();
		delete lt;
	}

	free(self);
}

static void
TransferView_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	if (NULL == windowPos)
		return;

	if (SWP_NOSIZE != ((SWP_NOSIZE | SWP_FRAMECHANGED) & windowPos->flags)) 
	{
		TransferView_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & windowPos->flags));
	}
}

static void
TransferView_OnDisplayChanged(HWND hwnd, INT bpp, INT dpi_x, INT dpi_y)
{
	UpdateWindow(hwnd);
	TransferView_UpdateFont(hwnd, TRUE);
}

static void
TransferView_OnSetFont(HWND hwnd, HFONT font, BOOL redraw)
{
	TransferView *self;
	TRANSFERVIEW_RET_VOID(self, hwnd);

	self->font = font;
}

static HFONT
TransferView_OnGetFont(HWND hwnd)
{
	TransferView *self;
	TRANSFERVIEW_RET_VAL(self, hwnd, NULL);

	return self->font;
}

static void 
TransferView_OnSetUpdateRegion(HWND  hwnd, HRGN updateRegion, POINTS regionOffset)
{
	TransferView *self;
	TRANSFERVIEW_RET_VOID(self, hwnd);

	self->updateRegion = updateRegion;
	self->updateOffset.x = regionOffset.x;
	self->updateOffset.y = regionOffset.y;
}

INT_PTR CALLBACK pmp_queue_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (NULL != listTransfers)
	{
		INT_PTR processed = listTransfers->DialogProc(hwndDlg,uMsg,wParam,lParam);
		if (0 != processed) 
			return processed;
	}

	switch (uMsg)
	{
		case WM_INITDIALOG:			return TransferView_OnInitDialog(hwndDlg, (HWND)wParam, lParam);
		case WM_DESTROY:			TransferView_OnDestroy(hwndDlg); break;
		case WM_WINDOWPOSCHANGED:	TransferView_OnWindowPosChanged(hwndDlg, (WINDOWPOS*)lParam); return TRUE;
		case WM_DISPLAYCHANGE:		TransferView_OnDisplayChanged(hwndDlg, (INT)wParam, LOWORD(lParam), HIWORD(lParam)); return TRUE;
		case WM_GETFONT:			DIALOG_RESULT(hwndDlg, TransferView_OnGetFont(hwndDlg));
		case WM_SETFONT:			TransferView_OnSetFont(hwndDlg, (HFONT)wParam, LOWORD(lParam)); return TRUE;
		case WM_TIMER:
			if (wParam == 1)
			{
				updateStatus(hwndDlg);
				/*if (device_update_map[device] == true)
				{
					device_update_map[device] = false;*/
					transferListContents.FullUpdate();
				/*}*/
			}
			break;
		case WM_NOTIFY:
			{
				LPNMHDR l=(LPNMHDR)lParam;
				if (l->idFrom==IDC_LIST_TRANSFERS)
				{
					switch (l->code)
					{
						case NM_RETURN: // enter!
						//case NM_RCLICK: // right click!
						case LVN_KEYDOWN:
						{
							int row = 0;
							C_ItemList items;
							if (!AddSelectedItems(&items, &listTransfers->listview, getTransferQueue(device), row, device))
								return 0;
							if (!AddSelectedItems(&items, &listTransfers->listview, getFinishedTransferQueue(device), row, device))
								return 0;

							if (items.GetSize())
							{
								// TODO need to check the handling of this...
								/*if (l->code == NM_RCLICK)
								{
									LPNMITEMACTIVATE lva=(LPNMITEMACTIVATE)lParam;
									handleContextMenuResult(showContextMenu(7,l->hwndFrom,device->dev,lva->ptAction),&items,device);
								}
								else*/ if (l->code == NM_RETURN)
								{
									handleContextMenuResult((!GetAsyncKeyState(VK_SHIFT)?ID_TRACKSLIST_PLAYSELECTION:ID_TRACKSLIST_ENQUEUESELECTION),&items,device);
								}
								else if (l->code == LVN_KEYDOWN)
								{
									switch (((LPNMLVKEYDOWN)lParam)->wVKey)
									{
									case VK_DELETE:
										{
											if (!(GetAsyncKeyState(VK_CONTROL)&0x8000) && !(GetAsyncKeyState(VK_SHIFT)&0x8000))
											{
												handleContextMenuResult(ID_TRACKSLIST_DELETE,&items,device);
											}
										}
										break;
									case 0x45: //E
										if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && !(GetAsyncKeyState(VK_SHIFT)&0x8000))
										{
											handleContextMenuResult(ID_TRACKSLIST_EDITSELECTEDITEMS,&items,device);
										}
										break;
									}
								}
							}
							break;
						}
					}
				}
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON_CLEARFINISHED:
				{
					LinkedQueue * finishedTX = getFinishedTransferQueue(device);
					if (finishedTX)
					{
						finishedTX->lock();
						int j=finishedTX->GetSize();
						while (j-- > 0) delete(CopyInst*)finishedTX->Del(j);
						finishedTX->unlock();
					}
					transferListContents.FullUpdate();
				}
				break;
				case IDC_BUTTON_REMOVESELECTED:
				{
					int row = transferListContents.GetNumRows();
					RemoveSelectedItems(device, &listTransfers->listview, getTransferQueue(device), row, false);
					RemoveSelectedItems(device, &listTransfers->listview, getFinishedTransferQueue(device), row, true);
					transferListContents.FullUpdate();
				}
				break;
				case IDC_BUTTON_PAUSETRANSFERS:
				{
					if (false != device->transferContext.IsPaused())
						device->transferContext.Resume();
					else
						device->transferContext.Pause();

					SetDlgItemText(hwndDlg,IDC_BUTTON_PAUSETRANSFERS,WASABI_API_LNGSTRINGW((device->transferContext.IsPaused()?IDS_RESUME:IDS_PAUSE)));
					TransferView_UpdateLayout(hwndDlg, TRUE);
				}
				break;
				case IDC_BUTTONCANCELSELECTED:
				{
					int row = transferListContents.GetNumRows();
					CancelSelectedItems(device, &listTransfers->listview, &cloudTransferQueue, row);
					transferListContents.FullUpdate();
				}
				break;
				case IDC_BUTTON_RETRYSELECTED:
				{
					int row = transferListContents.GetNumRows();
					RetrySelectedItems(device, &listTransfers->listview, &cloudTransferQueue, &cloudFinishedTransfers, row);
					transferListContents.FullUpdate();
				}
				break;
			}
			break;

		// gen_ml flickerless drawing
		case WM_USER + 0x200: DIALOG_RESULT(hwndDlg, 1);
		case WM_USER + 0x201: TransferView_OnSetUpdateRegion(hwndDlg, (HRGN)lParam, MAKEPOINTS(wParam)); return TRUE;
	}

	return 0;
}