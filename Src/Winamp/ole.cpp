/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoWideFn.h"
extern "C"
{
	extern int g_has_deleted_current;
};
#include "shlobj.h"
#include "../Plugins/General/gen_ff/ff_ipc.h"

 bool refuseDrops = false;
class DropTarget: public IDropTarget
{
public:
	DropTarget(int enqueue, int isshell)
	{ m_enqueue = enqueue; m_isshell = isshell; }

	STDMETHODIMP QueryInterface (REFIID riid, LPVOID * ppvObj)
	{
		if (riid == IID_IDropTarget || riid == IID_IUnknown)
		{
			*ppvObj = this;
			return S_OK;
		}
		*ppvObj = NULL;
		return E_NOINTERFACE;
	}
	STDMETHODIMP_(ULONG) AddRef ()
	{ return 0; }
	STDMETHODIMP_(ULONG) Release ()
	{ return 0; }

	STDMETHODIMP DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		if (pdwEffect)
		{
			if (refuseDrops && !m_isshell)
				*pdwEffect = DROPEFFECT_NONE;
			else
				*pdwEffect = DROPEFFECT_COPY;
		}
			
		return S_OK;
	}
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		
		if (pdwEffect)
		{
			if (refuseDrops && !m_isshell)
				*pdwEffect = DROPEFFECT_NONE;
		}

		return S_OK;
	}
	STDMETHODIMP DragLeave()
	{
		return 0;
	}

	STDMETHODIMP Drop(IDataObject * pDataObj, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		POINT p = {pt.x, pt.y};
		HWND wnd;
		int pl_forceappend = 0;
		int is_main = 0;
		int is_pe = 0;
		int pe_insert = 0;
		if (m_isshell)
		{
			is_main = 1;
			wnd = hMainWindow;
			if (m_enqueue == 2)
			{
				pe_insert = !SendMessageW(hMainWindow, WM_WA_IPC, PlayList_getlength(), IPC_SHELL_ACTION_START);
			}
		}
		else
		{
			wnd = WindowFromPoint(p);

			HWND child=0;
			do
			{
				RECT r;
				GetWindowRect(wnd, &r);
				POINT offset = p;
				offset.x -= r.left;
				offset.y -= r.top;
				child = ChildWindowFromPoint(wnd, offset);
				if (child == wnd)
					child = 0;
				else if (child)
					wnd = child;
			} while (child);

			HWND par = wnd;
			HWND pledit_poopie = NULL;
			while (par && !(GetWindowLongW(par, GWL_EXSTYLE) & WS_EX_ACCEPTFILES))
			{ 
				par = GetParent(par);
			}
			if (par) wnd = par;

			if (wnd == hMainWindow || IsChild(hMainWindow, wnd) 
				|| (HWND)SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)wnd, IPC_FF_GETCONTENTWND) == hMainWindow)
			{
				is_main = 1;
			}
			else if (wnd == hPLWindow || IsChild(hPLWindow, wnd) || GetParent(hPLWindow) && IsChild(wnd, hPLWindow) ||
			         (pledit_poopie = (HWND)SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)wnd, IPC_FF_GETCONTENTWND)) == hPLWindow)
			{
				is_pe = 1;
				if (pledit_poopie) pl_forceappend = 1; // if we got here because we're hosting the pledit, but not actually
				// parenting it (meaning we are windowshaded playlist)
			}
			else
			{
				EnterCriticalSection(&embedcs);
				{
					embedWindowState *p = embedwndlist;
					while (p)
					{
						if (IsChild(p->me, wnd) || (GetParent(p->me) && IsChild(wnd, p->me))) break;
						p = p->link;
					}
					if (p)
					{
						if (wnd == p->me || IsChild(wnd, p->me) || !(GetWindowLongW(wnd, GWL_EXSTYLE) & WS_EX_ACCEPTFILES)) // if this window accepts files, dont fuck it up
						{
							HWND wnd2 = FindWindowExW(p->me, NULL, NULL, NULL);
							if (!(GetWindowLongW(wnd2, GWL_EXSTYLE)&WS_EX_ACCEPTFILES)) is_main = 1;
							else
							{
								HWND h = GetParent(p->me);
								char buf[128] = {0};
								// see if we are (AVS) docked to the main (modern) window
								if (h && (h = GetParent(h)) && GetWindowTextA(h, buf, sizeof(buf)) && !_stricmp(buf, "Player Window"))
									is_main = 1;
								else wnd = wnd2;
							}
						}
					}
					else is_main = 1;
				}
				LeaveCriticalSection(&embedcs);
			}

			if (is_main) wnd = hMainWindow;
			if (is_pe) wnd = hPLWindow;
		}

		HRESULT hr = S_OK;
		if (pDataObj)
		{
			// Important: these strings need to be non-Unicode (don't compile UNICODE)
			unsigned short cp_format_url = (unsigned short)RegisterClipboardFormat(CFSTR_SHELLURL);
			//Set up format structure for the descriptor and contents
			FORMATETC format_url =
			    {cp_format_url, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
			FORMATETC format_file =
			    {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
			// Check for URL
			hr = pDataObj->QueryGetData(&format_url);
			if (hr == S_OK)
			{
				// Get the descriptor information
				STGMEDIUM storage = {0, 0, 0};
				hr = pDataObj->GetData(&format_url, &storage);
				char* url = (char *)GlobalLock(storage.hGlobal);
				if (url)
				{
					if (is_pe || is_main || m_isshell)
					{
						int t, lp, a;
						int del = is_main && !(GetAsyncKeyState(VK_SHIFT) & (1 << 15));
						if (del) PlayList_delete();
						else if (!is_main && !m_isshell) // playlist
						{
							RECT r;
							POINT dp;
							GetWindowRect(wnd, &r);
							dp.x = pt.x - r.left;
							dp.y = pt.y - r.top;
							if (config_pe_height == 14 || pl_forceappend) t = PlayList_getlength();
							else
							{
								t = (dp.y - 20) / pe_fontheight;
								if (t < 0) t = 0;
								else if (t > (config_pe_height - 38 - 20 - 2) / pe_fontheight) t = PlayList_getlength();
								else t += pledit_disp_offs;
							}
							a = PlayList_getlength();
							PlayList_saveend(t);
							lp = PlayList_getPosition();
						}

						if(!PathIsURLA(url))
						{
							// if it's not reported as an url, try to treat it like
							// a filepath which has been url-encoded and insert it
							// though if it fails to be converted then revert to it
							// just being added to the playlist without processing.
							char url2[FILENAME_SIZE] = {"file:///"};
							strncat(url2, url, FILENAME_SIZE);
							DWORD len = lstrlenA(url2);
							if(SUCCEEDED(PathCreateFromUrlA(url2, url2, &len, 0)))
							{
								PlayList_appendthing(AutoWideFn(url2), 0, 0);
							}
							else
							{
								PlayList_appendthing(AutoWideFn(url), 0, 0);
							}
						}
						else
						{
							PlayList_appendthing(AutoWideFn(url), 0, 0);
						}

						if (del) StartPlaying();
						else if (!is_main && !m_isshell) // playlist
						{
							PlayList_restoreend();
							if (t <= PlayList_getPosition())
							{
								PlayList_setposition(lp + PlayList_getlength() - a);
								if (!g_has_deleted_current)
								{
									PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
									draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
								}
							}
							plEditRefresh();
						}
					}
					else // fucko: pass url to 'wnd' somehow?
					{}
				}
				GlobalUnlock(url);
				GlobalFree(url);
			}
			else
			{
				// check for file
				hr = pDataObj->QueryGetData(&format_file);
				if (hr == S_OK)
				{
					STGMEDIUM medium;
					pDataObj->GetData (&format_file, &medium);
					wchar_t temp[FILENAME_SIZE] = {0};
					if (m_enqueue)
					{
						HDROP hdrop = (HDROP)medium.hGlobal;
						int y = DragQueryFileW(hdrop, 0xffffffff, temp, FILENAME_SIZE);
						int current = PlayList_getPosition();
						for (int x = 0; x < y; x ++)
						{
							DragQueryFileW(hdrop, x, temp, FILENAME_SIZE);
							if (!pe_insert)
								PlayList_appendthing(temp, 0, 0);
							else
							{
								PlayList_insert(++current, temp);
							}
						}
						DragFinish(hdrop);

						plEditRefresh();
					}
					else
					{
						int skinLoad = -2;
						int langLoad = -2;
						RECT r;
						GetWindowRect(wnd, &r);
						LPDROPFILES d = (LPDROPFILES)GlobalLock(medium.hGlobal);
						d->pt.x = pt.x - r.left;
						if (pl_forceappend)
							d->pt.y = r.bottom - r.top;
						else d->pt.y = pt.y - r.top;
						d->fNC = FALSE;

						GlobalUnlock(d);
						
						// check for the file being a skin or language pack
						// being dropped so we can intercept and not add it
						// into the pledit but can instead prompt for install
						DragQueryFileW((HDROP)medium.hGlobal, 0, temp, FILENAME_SIZE);
						CheckSkin(temp, hMainWindow, &skinLoad);
						CheckLang(temp, hMainWindow, &langLoad);

						if(!skinLoad && !langLoad)
						{
							PostMessageW(wnd, WM_DROPFILES, (WPARAM)medium.hGlobal, 0);
						}
					}
				}
			}
		}

		if (m_isshell && (m_enqueue == 2))
		{
			SendMessageW(hMainWindow, WM_WA_IPC, PlayList_getlength(), IPC_SHELL_ACTION_END);
		}
		return S_OK;
	}
public:
	int m_enqueue;
	int m_isshell;
};

static DropTarget m_target(0, 0);

class ClassFactory : public IClassFactory
{
public:
	ClassFactory(int enqueue) { m_drop = new DropTarget(enqueue, 1); }
	~ClassFactory() { delete(m_drop); }

	STDMETHODIMP QueryInterface (REFIID riid, LPVOID * ppvObj)
	{
		if (riid == IID_IClassFactory || riid == IID_IUnknown)
		{
			*ppvObj = this;
			return S_OK;
		}
		*ppvObj = NULL;
		return E_NOINTERFACE;
	}
	STDMETHODIMP_(ULONG) AddRef ()
	{ return 0; }
	STDMETHODIMP_(ULONG) Release ()
	{ return 0; }

	STDMETHODIMP CreateInstance(IUnknown * pUnkOuter, REFIID riid, void ** ppvObject)
	{
		if (pUnkOuter != NULL)
		{
			return CLASS_E_NOAGGREGATION;
		}
		if (riid == IID_IDropTarget || riid == IID_IUnknown)
		{
			*ppvObject = m_drop;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHODIMP LockServer(BOOL fLock)
	{
		return S_OK;
	}
private:
	DropTarget *m_drop;
};

static ClassFactory m_playCF(0);
static ClassFactory m_enqueueCF(1);
static ClassFactory m_enqueuePlayCF(2);

static DWORD m_exposePlayHandle = NULL, m_exposeEnqueueHandle = NULL, m_exposeEnqueuePlayHandle = NULL;

extern "C"
{

	void InitDragDrops()
	{
		SetWindowLong(hMainWindow, GWL_EXSTYLE, GetWindowLongW(hMainWindow, GWL_EXSTYLE)|(WS_EX_ACCEPTFILES));

		refuseDrops = false;
	}

	void Ole_initDragDrop()
	{
		OleInitialize(NULL);
		RegisterDragDrop(hMainWindow, &m_target);
		RegisterDragDrop(hPLWindow, &m_target);
		if (hVideoWindow) RegisterDragDrop(hVideoWindow, &m_target);

		// {46986115-84D6-459c-8F95-52DD653E532E}
		static const GUID playGuid =
		    { 0x46986115, 0x84D6, 0x459c, { 0x8f, 0x95, 0x52, 0xdd, 0x65, 0x3e, 0x53, 0x2e } };
		if (CoRegisterClassObject(playGuid, &m_playCF, CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &m_exposePlayHandle) != S_OK)
			m_exposePlayHandle = NULL;

		// {77A366BA-2BE4-4a1e-9263-7734AA3E99A2}
		static const GUID enqueueGuid =
		    { 0x77A366BA, 0x2BE4, 0x4a1e, { 0x92, 0x63, 0x77, 0x34, 0xAA, 0x3e, 0x99, 0xa2 } };
		if (CoRegisterClassObject(enqueueGuid, &m_enqueueCF, CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &m_exposeEnqueueHandle) != S_OK)
			m_exposeEnqueueHandle = NULL;

		// {ED50B649-42B0-4153-9E99-54C45F6BD708}
		static const GUID enqueuePlayGuid = 
			{ 0xed50b649, 0x42b0, 0x4153, { 0x9e, 0x99, 0x54, 0xc4, 0x5f, 0x6b, 0xd7, 0x8 } };
		if (CoRegisterClassObject(enqueuePlayGuid, &m_enqueuePlayCF, CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &m_exposeEnqueuePlayHandle) != S_OK)
			m_exposeEnqueuePlayHandle = NULL;
	}

	void UninitDragDrops()
	{
		SetWindowLong(hMainWindow, GWL_EXSTYLE, GetWindowLongW(hMainWindow, GWL_EXSTYLE)&~(WS_EX_ACCEPTFILES));
		refuseDrops = true;
	}

	void Ole_uninitDragDrop()
	{
		if (hVideoWindow) RevokeDragDrop(hVideoWindow);
		RevokeDragDrop(hPLWindow);
		RevokeDragDrop(hMainWindow);

		if (m_exposePlayHandle)
			CoRevokeClassObject(m_exposePlayHandle);
		if (m_exposeEnqueueHandle)
			CoRevokeClassObject(m_exposeEnqueueHandle);
		if (m_exposeEnqueuePlayHandle)
			CoRevokeClassObject(m_exposeEnqueuePlayHandle);
		OleUninitialize();
	}

	void *Ole_getDropTarget()
	{
		return (void *)&m_target;
	}
};