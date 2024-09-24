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
#include "metadata_utils.h"

static int (*wad_handleDialogMsgs)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void (*wad_DrawChildWindowBorders)(HWND hwndDlg, int *tab, int tabsize);
static void (*cr_init)(HWND hwndDlg, ChildWndResizeItem *list, int num);
static void (*cr_resize)(HWND hwndDlg, ChildWndResizeItem *list, int num);


static HWND m_hwnd;

static ChildWndResizeItem resize_rlist[]=
{
	{IDC_LIST_DEVICES, 0x0010},
	{IDC_LIST_TRANSFERS, 0x0011},
	{IDC_TQ_STATIC, 0x0000},
	{IDC_HDELIM, 0x0010},
	{IDC_BUTTON_PAUSETRANSFERS, 0x0101},
	{IDC_BUTTON_CLEARFINISHED, 0x0101},
	{IDC_BUTTON_REMOVESELECTED, 0x0101},
	{IDC_STATUS,0x0111},
};
static int m_nodrawtopborders=0,adiv_clickoffs,adivpos=-1;
SkinnedListView * listDevices=NULL;
static SkinnedListView * listTransfers=NULL;
//CRITICAL_SECTION listTransfersLock;

static INT_PTR CALLBACK adiv_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static void adiv_UpdPos(int xp);
static WNDPROC adiv_oldWndProc;

class DeviceContents : public ListContents
{
public:
	virtual int GetNumColumns()
	{
		return 2;
	}
	virtual int GetNumRows()
	{
		return devices.GetSize();
	}
	virtual wchar_t * GetColumnTitle(int num)
	{
		switch (num)
		{
			case 0: return WASABI_API_LNGSTRINGW(IDS_NAME);
			case 1: return WASABI_API_LNGSTRINGW(IDS_CAPACITY_FREE);
		}
		return L"";
	}
	virtual int GetColumnWidth(int num)
	{
		switch (num)
		{
			case 0: return global_config->ReadInt(L"devices_col0_width",150);
			case 1: return global_config->ReadInt(L"devices_col1_width",100);
			default: return 0;
		}
	}
	virtual void ColumnResize(int col, int newWidth)
	{
		if (col==0) global_config->WriteInt(L"devices_col0_width",newWidth);
		else if (col==1) global_config->WriteInt(L"devices_col1_width",newWidth);
	}
	virtual void GetCellText(int row, int col, wchar_t * buf, int buflen)
	{
		switch (col)
		{
			case 0: 
				((DeviceView *)devices.Get(row))->dev->getPlaylistName(0,buf,buflen);
				return;
			case 1:
			{
				wchar_t capacity[20]=L"";
				Device * dev = ((DeviceView *)devices.Get(row))->dev;
				WASABI_API_LNG->FormattedSizeString(capacity, ARRAYSIZE(capacity), dev->getDeviceCapacityTotal());
				__int64 cap = dev->getDeviceCapacityTotal();
				int pc;
				if (cap) pc = (int)((((__int64)100)*dev->getDeviceCapacityAvailable()) / cap);
				else pc = 0;
				wsprintf(buf,L"%s (%d%%)",capacity,pc);
			}
			return;
		}
	}
	virtual songid_t GetTrack(int pos) { return 0; }
};

static DeviceContents deviceListContents;

class TransferItemShadow
{
public:
	CopyInst * c;
	wchar_t * status, * type, * track;
	wchar_t device[128];
	bool changed;
	TransferItemShadow(CopyInst * c)
	{
		changed = false;
		this->c = c;
		status = _wcsdup(c->statusCaption);
		type = _wcsdup(c->typeCaption);
		track = _wcsdup(c->trackCaption);
		device[0] = 0;
		c->dev->dev->getPlaylistName(0,device,128);
	}
	~TransferItemShadow()
	{
		free(status);
		free(type);
		free(track);
	}
	bool Equals(TransferItemShadow * a)
	{
		if (!a) return false;
		return (c == a->c) && !wcscmp(track,a->track) && !wcscmp(status,a->status) && !wcscmp(device,a->device) && !wcscmp(type,a->type);
	}
};

static C_ItemList *getTransferListShadow()
{
	C_ItemList * list = new C_ItemList;

	for (int i=0; i < devices.GetSize(); i++)
	{
		DeviceView *device = (DeviceView*)devices.Get(i);
		LinkedQueue * txQueue = getTransferQueue(device);
		LinkedQueue * finishedTX = getFinishedTransferQueue(device);
		if (txQueue)
		{
			txQueue->lock();
			int l = txQueue->GetSize();
	
			for (int j=0; j<l; j++)
				list->Add(new TransferItemShadow((CopyInst*)txQueue->Get(j)));
			txQueue->unlock();
		}

		if (finishedTX)
		{
			finishedTX->lock();
			int l = finishedTX->GetSize();
	
			for (int j=0; j<l; j++)
				list->Add(new TransferItemShadow((CopyInst*)finishedTX->Get(j)));
			finishedTX->unlock();
		}
	}
	return list;
}

class TransferContents2 : public ListContents
{
public:
	CRITICAL_SECTION cs;
	C_ItemList * listShadow;
	TransferContents2() : listShadow(0)
	{
		oldSize = 0;
		listShadow = getTransferListShadow();
		InitializeCriticalSection(&cs);
	}
	virtual ~TransferContents2()
	{
		DeleteCriticalSection(&cs); delete listShadow;
	}
	virtual int GetNumColumns()
	{
		return 4;
	}
	virtual int GetNumRows()
	{
		return (listShadow ? listShadow->GetSize() : 0);
	}
	virtual wchar_t * GetColumnTitle(int num)
	{
		switch (num)
		{
			case 0: return WASABI_API_LNGSTRINGW(IDS_TYPE);
			case 1: return WASABI_API_LNGSTRINGW(IDS_TRACK);
			case 2: return WASABI_API_LNGSTRINGW(IDS_STATUS);
			case 3: return WASABI_API_LNGSTRINGW(IDS_DEVICE);
		}
		return L"";
	}
	virtual int GetColumnWidth(int num)
	{
		switch (num)
		{
			case 0: return global_config->ReadInt(L"transfers_col0_width",100);
			case 1: return global_config->ReadInt(L"transfers_col1_width",300);
			case 2: return global_config->ReadInt(L"transfers_col2_width",150);
			case 3: return global_config->ReadInt(L"transfers_col3_width",150);
			default: return 0;
		}
	}
	virtual void ColumnResize(int col, int newWidth)
	{
		if (col==0) global_config->WriteInt(L"transfers_col0_width",newWidth);
		else if (col==1) global_config->WriteInt(L"transfers_col1_width",newWidth);
		else if (col==2) global_config->WriteInt(L"transfers_col2_width",newWidth);
		else if (col==3) global_config->WriteInt(L"transfers_col3_width",newWidth);
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
		lock();
		if (row >= listShadow->GetSize())
		{
			unlock(); return;
		}
		TransferItemShadow * t = (TransferItemShadow *)listShadow->Get(row);
		switch (col)
		{
		case 0: lstrcpyn(buf,t->type,buflen); break;
		case 1: lstrcpyn(buf,t->track,buflen); break;
		case 2: lstrcpyn(buf,t->status,buflen); break;
		case 3: lstrcpyn(buf,t->device,buflen); break;
		}
		unlock();
	}

	void PushPopItem(CopyInst *c)
	{
		lock();
		int size = listShadow->GetSize();
		for (int i=0; i<size; i++)
		{
			TransferItemShadow * t = (TransferItemShadow *)listShadow->Get(i);
			if (c == t->c)
			{
				t->changed=true;
				listShadow->Del(i);
				listShadow->Add(t);
				if (listTransfers)
				{
					HWND hwnd = listTransfers->listview.getwnd();
					PostMessage(hwnd,LVM_DELETEITEM,i,0);
					PostMessage(hwnd,LVM_SETITEMCOUNT,size,LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
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
			TransferItemShadow * t = (TransferItemShadow *)listShadow->Get(i);
			if (c == t->c)
			{
				TransferItemShadow * n = new TransferItemShadow(c);
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
	int oldSize;
	void FullUpdate()
	{
		C_ItemList * newListShadow = getTransferListShadow();
		int newSize = newListShadow->GetSize();
		lock();
		oldSize = listShadow->GetSize();
		for (int i=0; i<newSize; i++)
		{
			TransferItemShadow * newt = (TransferItemShadow *)newListShadow->Get(i);
			TransferItemShadow * oldt = i<oldSize?(TransferItemShadow *)listShadow->Get(i):NULL;
			newt->changed = !newt->Equals(oldt);
		}

		C_ItemList * oldListShadow = listShadow;
		listShadow = newListShadow;
		for (int i=0; i<oldListShadow->GetSize(); i++) delete(TransferItemShadow *)oldListShadow->Get(i);
		delete oldListShadow;
		if (listTransfers)
		{
			HWND hwnd = listTransfers->listview.getwnd();
			if (newSize != oldSize) PostMessage(hwnd,LVM_SETITEMCOUNT,newSize, 0);
			for (int i=0; i<newSize; i++)
			{
				TransferItemShadow * t = (TransferItemShadow *)listShadow->Get(i);
				if (t->changed) PostMessage(hwnd,LVM_REDRAWITEMS,i,i);
			}
		}
		unlock();
	}
	virtual songid_t GetTrack(int pos) { return 0; }
};

static void updateStatus()
{
	int pcnum=0,num=0,time=0,total=0;

	for (int i=0; i<devices.GetSize(); i++)
	{
		DeviceView* device = (DeviceView*)devices.Get(i);
		LinkedQueue * txQueue = getTransferQueue(device);
		LinkedQueue * finishedTX = getFinishedTransferQueue(device);
		int txProgress = getTransferProgress(device);
		int s = (txQueue ? txQueue->GetSize() : 0);
		int n = (s * 100) - txProgress;
		total += s * 100;
		total += 100 * (finishedTX ? finishedTX->GetSize() : 0);
		pcnum += n;
		num += s;
		int t = (int)(device->transferRate * (((double)n) / 100.0));
		if (time < t) time = t;
	}
	if (total)
	{
		wchar_t caption[256] = {0};
		int pc = ((total-pcnum)*100)/total;
		wsprintf(caption,WASABI_API_LNGSTRINGW((time > 0 ? IDS_TRANFERS_PERCENT_REMAINING : IDS_TRANFERS_PERCENT_REMAINING_NOT_TIME)),num,pc,time/60,time%60);
		SetDlgItemText(m_hwnd,IDC_STATUS,caption);
	}
	else SetDlgItemText(m_hwnd,IDC_STATUS,L"");
}

static TransferContents2 transferListContents;

void TransfersListUpdateItem(CopyInst * item)
{
	transferListContents.ItemUpdate(item);
}

void TransfersListPushPopItem(CopyInst * item)
{
	transferListContents.PushPopItem(item);
}

void UpdateDevicesListView(bool softUpdate)
{
	if (listDevices) listDevices->UpdateList(softUpdate);
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
				if (!dev && c->dev) dev = c->dev;
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
				if (d && d->status == STATUS_WAITING && device->transferContext.IsPaused())
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
					if (d->status == STATUS_WAITING && !finished_queue)
						d->Cancelled();
					delete d;
				}
			}
		}
	}
	transfer_queue->unlock();
}

extern void handleContextMenuResult(int r, C_ItemList * items=NULL, DeviceView * dev=NULL);
extern int showContextMenu(int context,HWND hwndDlg, Device * dev, POINT pt);
extern int (*wad_getColor)(int idx);

INT_PTR CALLBACK pmp_devices_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (wad_handleDialogMsgs)
	{
		BOOL a=wad_handleDialogMsgs(hwndDlg,uMsg,wParam,lParam); if (a) return a;
	}
	if (listDevices)
	{
		BOOL a=listDevices->DialogProc(hwndDlg,uMsg,wParam,lParam); if (a) return a;
	}
	if (listTransfers)
	{
		BOOL a=listTransfers->DialogProc(hwndDlg,uMsg,wParam,lParam);  if (a) return a;
	}
	switch (uMsg)
	{
	case WM_INITDIALOG:
		m_hwnd=hwndDlg;
		*(void **)&wad_handleDialogMsgs=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,2,ML_IPC_SKIN_WADLG_GETFUNC);
		*(void **)&wad_DrawChildWindowBorders=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,3,ML_IPC_SKIN_WADLG_GETFUNC);
		*(void **)&cr_init=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,32,ML_IPC_SKIN_WADLG_GETFUNC);
		*(void **)&cr_resize=(void*)SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,33,ML_IPC_SKIN_WADLG_GETFUNC);
		if (cr_init) cr_init(hwndDlg,resize_rlist,sizeof(resize_rlist)/sizeof(resize_rlist[0]));
		listDevices = new SkinnedListView(&deviceListContents,IDC_LIST_DEVICES,plugin.hwndLibraryParent, hwndDlg);
		listDevices->DialogProc(hwndDlg,uMsg,0,0);
		transferListContents.lock();
		listTransfers = new SkinnedListView(&transferListContents,IDC_LIST_TRANSFERS,plugin.hwndLibraryParent, hwndDlg);
		listTransfers->DialogProc(hwndDlg,uMsg,0,0);
		transferListContents.unlock();

		adiv_oldWndProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_HDELIM),GWLP_WNDPROC,(LONG_PTR)adiv_newWndProc);

		SetDlgItemText(hwndDlg,IDC_BUTTON_PAUSETRANSFERS,WASABI_API_LNGSTRINGW((TransferContext::IsAllPaused()?IDS_RESUME:IDS_PAUSE)));
		SetTimer(hwndDlg,1,250,NULL);
		updateStatus();
		break;
	case WM_TIMER:
		if (wParam == 1)
		{
			updateStatus();
			if (device_update_map[0])
			{
				device_update_map[0]=0;
				transferListContents.FullUpdate();
			}
		}
		break;
	case WM_DISPLAYCHANGE:
		break;
	case WM_SETCURSOR: // set cursor when near the dividers
	{
		RECT r;
		POINT p;
		GetCursorPos(&p);
		GetWindowRect(GetDlgItem(hwndDlg,IDC_HDELIM),&r);
		r.top-=3;
		r.bottom+=3;
		if (PtInRect(&r,p))
		{
			SetCursor(LoadCursor(NULL,IDC_SIZENS));
			return uMsg == WM_SETCURSOR;
		}

		break;
	}
	case WM_LBUTTONDOWN:
	{
		// forward dialog clicks to the dividers if they get near them
		POINT p;
		RECT r3;
		GetWindowRect(GetDlgItem(hwndDlg,IDC_VDELIM),&r3);
		GetCursorPos(&p);
		GetWindowRect(GetDlgItem(hwndDlg,IDC_HDELIM),&r3);
		if (p.x >= r3.left && p.x <= r3.right)
		{
			int d=p.y-r3.bottom;
			int d2=p.y-r3.top;
			if (d<0)d=-d;
			if (d2<0)d2=-d2;
			if (d < 6 || d2 < 6) SendDlgItemMessage(hwndDlg,IDC_HDELIM,uMsg,0,0);
		}
	}
	break;
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			if (cr_resize) cr_resize(hwndDlg,resize_rlist,sizeof(resize_rlist)/sizeof(resize_rlist[0]));
		}
		break;
	case WM_PAINT:
	{
		if (wad_DrawChildWindowBorders)
		{
			int tab[] = {m_nodrawtopborders==1?0:(IDC_LIST_DEVICES|DCW_SUNKENBORDER),
						 m_nodrawtopborders==2?0:(IDC_LIST_TRANSFERS|DCW_SUNKENBORDER),
						 IDC_HDELIM|DCW_DIVIDER
						};
			wad_DrawChildWindowBorders(hwndDlg,tab,3);
		}
	}
	break;
	case WM_DESTROY:
	{
		KillTimer(hwndDlg, 1);
		m_hwnd=NULL;
		SkinnedListView * ld = listDevices; listDevices=NULL; delete ld;
		SkinnedListView * lt = listTransfers;
		transferListContents.lock();
		listTransfers=NULL;
		transferListContents.unlock();
		delete lt;
	}
	break;
	case WM_NOTIFY:
	{
		LPNMHDR l=(LPNMHDR)lParam;
		if (l->idFrom==IDC_LIST_TRANSFERS)
			switch (l->code)
			{
			case NM_RETURN: // enter!
			case NM_RCLICK: // right click!
			case LVN_KEYDOWN:
			{
				DeviceView * dev = NULL;
				C_ItemList items;
				int row = 0;
				for (int i=0; i<devices.GetSize(); i++)
				{
					DeviceView *device = (DeviceView*)devices.Get(i);
					if (!AddSelectedItems(&items, &listTransfers->listview, getTransferQueue(device), row, dev))
						return 0;
					if (!AddSelectedItems(&items, &listTransfers->listview, getFinishedTransferQueue(device), row, dev))
						return 0;
				}
				bool foundDev=false;
				for (int k=0; k<devices.GetSize(); k++) if (dev == (DeviceView*)devices.Get(k)) foundDev=true;
				if (dev && foundDev && items.GetSize())
				{
					if (l->code == NM_RCLICK)
					{
						LPNMITEMACTIVATE lva=(LPNMITEMACTIVATE)lParam;
						handleContextMenuResult(showContextMenu(7,l->hwndFrom,dev->dev,lva->ptAction),&items,dev);
					}
					else if (l->code == NM_RETURN)
					{
						handleContextMenuResult((!GetAsyncKeyState(VK_SHIFT)?ID_TRACKSLIST_PLAYSELECTION:ID_TRACKSLIST_ENQUEUESELECTION),&items,dev);
					}
					else if (l->code == LVN_KEYDOWN)
					{
						switch (((LPNMLVKEYDOWN)lParam)->wVKey)
						{
						case VK_DELETE:
						{
							if (!(GetAsyncKeyState(VK_CONTROL)&0x8000) && !(GetAsyncKeyState(VK_SHIFT)&0x8000))
							{
								handleContextMenuResult(ID_TRACKSLIST_DELETE,&items,dev);
							}
						}
						break;
						case 0x45: //E
							if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && !(GetAsyncKeyState(VK_SHIFT)&0x8000))
							{
								handleContextMenuResult(ID_TRACKSLIST_EDITSELECTEDITEMS,&items,dev);
							}
							break;
						}
					}
				}
				break;
			}
		}
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_CLEARFINISHED:
		{
			for (int i=0; i<devices.GetSize(); i++)
			{
				DeviceView *device = (DeviceView*)devices.Get(i);
				LinkedQueue * finishedTX = getFinishedTransferQueue(device);
				if (finishedTX)
				{
					finishedTX->lock();
					int j=finishedTX->GetSize();
					while (j-- > 0) delete(CopyInst*)finishedTX->Del(j);
					finishedTX->unlock();
				}
			}
			transferListContents.FullUpdate();
		}
		break;
		case IDC_BUTTON_REMOVESELECTED:
		{
			int row = transferListContents.GetNumRows();
			int i = devices.GetSize();
			while (i-- > 0)
			{
				DeviceView *device = (DeviceView *)devices.Get(i);
				RemoveSelectedItems(device, &listTransfers->listview, getTransferQueue(device), row, false);
				RemoveSelectedItems(device, &listTransfers->listview, getFinishedTransferQueue(device), row, true);
			}
			transferListContents.FullUpdate();
		}
		break;
		case IDC_BUTTON_PAUSETRANSFERS:

			if (false != TransferContext::IsAllPaused())
				TransferContext::ResumeAll();
			else
				TransferContext::PauseAll();
		
			for (int i=0;i<devices.GetSize();i++)
			{
				DeviceView *device_view = (DeviceView*)devices.Get(i);
				SetEvent(device_view->transferContext.notifier);
			}

			SetDlgItemText(hwndDlg,IDC_BUTTON_PAUSETRANSFERS,WASABI_API_LNGSTRINGW((TransferContext::IsAllPaused()?IDS_RESUME:IDS_PAUSE)));
			break;
		}
		break;
	case WM_MOUSEMOVE:
		if (wParam==MK_LBUTTON)
		{
			if (GetCapture() == hwndDlg) ReleaseCapture();
		}
		break;
	}
	return 0;
}

// deals with the horizontal (IDC_HDELIM) divider

static void adiv_UpdPos(int yp)
{
	RECT r,old_divider_rect;
	GetClientRect(m_hwnd,&r);

	GetWindowRect(GetDlgItem(m_hwnd,IDC_HDELIM),&old_divider_rect);
	ScreenToClient(m_hwnd,(LPPOINT)&old_divider_rect);
	ScreenToClient(m_hwnd,((LPPOINT)&old_divider_rect)+1);
	if (yp < 50)
	{
		m_nodrawtopborders=1;
		yp=20;
	}
	else m_nodrawtopborders=0;
	if (yp > r.bottom-42-30)
	{
		yp=r.bottom-42;
		m_nodrawtopborders=2;
	}

	int x;
	for (x = 0; x < 4; x ++)
	{
		RECT myoldr;
		GetWindowRect(GetDlgItem(m_hwnd,resize_rlist[x].id),&myoldr);

		ScreenToClient(m_hwnd,(LPPOINT)&myoldr);
		ScreenToClient(m_hwnd,((LPPOINT)&myoldr)+1);
		switch (x)
		{
			case 0:
				resize_rlist[x].rinfo.bottom=yp - old_divider_rect.top + myoldr.bottom;
				break;
			case 1:
				resize_rlist[x].rinfo.top=yp - old_divider_rect.top + myoldr.top;
				break;
			case 2:
			case 3:
			{
				int h=resize_rlist[x].rinfo.bottom - resize_rlist[x].rinfo.top;
				resize_rlist[x].rinfo.top = yp - old_divider_rect.top + resize_rlist[x].rinfo.top;
				resize_rlist[x].rinfo.bottom = resize_rlist[x].rinfo.top + h;
			}
			break;
		}
	}
	cr_resize(m_hwnd,resize_rlist,4);
}

static INT_PTR CALLBACK adiv_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_LBUTTONDOWN)
	{
		SetForegroundWindow(hwndDlg);
		SetCapture(hwndDlg);
		SetCursor(LoadCursor(NULL,IDC_SIZENS));
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwndDlg,&p);
		adiv_clickoffs=p.y;
	}
	else if (uMsg == WM_SETCURSOR)
	{
		SetCursor(LoadCursor(NULL,IDC_SIZENS));
		return TRUE;
	}
	else if (uMsg == WM_MOUSEMOVE && GetCapture()==hwndDlg)
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(GetParent(hwndDlg),&p);
		adiv_UpdPos(p.y-adiv_clickoffs);
		RECT r;
		GetClientRect(GetParent(hwndDlg),&r);
		if (r.bottom > r.top)
		{
			int percent = ((p.y-adiv_clickoffs) * 100000) / (r.bottom-r.top);
			adivpos = percent;
		}
	}
	else if (uMsg == WM_MOUSEMOVE)
	{
		SetCursor(LoadCursor(NULL,IDC_SIZENS));
	}
	else if (uMsg == WM_LBUTTONUP)
	{
		ReleaseCapture();
	}
	return CallWindowProc(adiv_oldWndProc,hwndDlg,uMsg,wParam,lParam);
}