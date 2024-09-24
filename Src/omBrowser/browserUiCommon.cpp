#include "main.h"
#include "./browserUiCommon.h"
#include "./browserUiInternal.h"

#include "./toolbar.h"
#include "./statusbar.h"
#include "./curtain.h"
#include "./browserHost.h"

#include "./obj_ombrowser.h"
#include "./ifc_omservice.h"
#include "./ifc_omservicecommand.h"
#include "./ifc_omtoolbarconfig.h"
#include "./ifc_omstatusbarconfig.h"

#include "./resource.h"

#define IDC_OPERATIONWIDGET	0x1010

static HACCEL commonTable = NULL;
static HACCEL popupTable = NULL;
static BOOL regiterUnload = TRUE;

static void CALLBACK BrowserControl_OnPluginUnload()
{
	if (NULL != commonTable)
	{
		DestroyAcceleratorTable(commonTable);
		commonTable = NULL;
	}

	if (NULL != popupTable)
	{
		DestroyAcceleratorTable(popupTable);
		popupTable = NULL;
	}
}

static HACCEL BrowserControl_LoadCommonAccel()
{
	if (NULL != commonTable)
		return commonTable;

	commonTable = Plugin_LoadAccelerators(MAKEINTRESOURCE(IDR_BROWSERACCEL));

	if (FALSE != regiterUnload && NULL != commonTable)
	{
		Plugin_RegisterUnloadCallback(BrowserControl_OnPluginUnload);
		regiterUnload = FALSE;
	}

	return commonTable;

}

static HACCEL BrowserControl_LoadPoppupAccel()
{
	if (NULL != popupTable)
		return popupTable;
	
	UINT cCommon, cPopup, cTotal = 0;
	
	HACCEL hCommon = Plugin_LoadAccelerators(MAKEINTRESOURCE(IDR_BROWSERACCEL));
	cCommon = (NULL != hCommon) ? CopyAcceleratorTable(hCommon, NULL, 0) : 0;
	
	HACCEL hPopup = Plugin_LoadAccelerators(MAKEINTRESOURCE(IDR_BROWSERPOPUPACCEL));
	cPopup = (NULL != hPopup) ? CopyAcceleratorTable(hPopup, NULL, 0) : 0;
	
	cTotal += (cCommon + cPopup);
	if (0 != cTotal)
	{
		ACCEL *pAccel = (ACCEL*)calloc(cTotal, sizeof(ACCEL));
		if (NULL != pAccel)
		{
			UINT copied = 0;
			if (NULL != hCommon)
				copied += CopyAcceleratorTable(hCommon, pAccel + copied, cTotal - copied);
			
			if (NULL != hPopup)
				copied += CopyAcceleratorTable(hPopup, pAccel + copied, cTotal - copied);
			
			if (0 != copied)
				popupTable = CreateAcceleratorTable(pAccel, copied);

				free(pAccel);
		}
	}

	if (NULL != hCommon)
		DestroyAcceleratorTable(hCommon);
	
	if (NULL != hPopup)
		DestroyAcceleratorTable(hPopup);
	
	if (FALSE != regiterUnload && NULL != popupTable)
	{
		Plugin_RegisterUnloadCallback(BrowserControl_OnPluginUnload);
		regiterUnload = FALSE;
	}

	return popupTable;
	
}

HACCEL BrowserControl_GetAccelTable(UINT tableType)
{
	switch(tableType)
	{
		case ACCELTABLE_VIEW:
			return BrowserControl_LoadCommonAccel();
		case ACCELTABLE_POPUP:
			return BrowserControl_LoadPoppupAccel();
	}
	return NULL;
}

static void BrowserControl_ShowHistoryPopup(HWND hControl)
{
	HWND hBrowser = BrowserControl_GetHost(hControl);
	if (NULL == hBrowser) return;

	UINT flags = TPM_LEFTALIGN;
	POINT pt = {0, 0};

	BOOL fUseHost = TRUE;;
	HWND hToolbar = BrowserControl_GetToolbar(hControl);
	DWORD toolbarStyle = (NULL != hToolbar) ? GetWindowStyle(hToolbar) : 0;

	if (NULL != hToolbar && 0 != (WS_VISIBLE & toolbarStyle))
	{
		fUseHost = FALSE;
		RECT toolbarRect;
		GetWindowRect(hToolbar, &toolbarRect);
		pt.x = toolbarRect.left + 2;
		if (0 != (TBS_BOTTOMDOCK & toolbarStyle))
		{
			pt.y = toolbarRect.top + 1;
			flags |= TPM_VERNEGANIMATION | TPM_BOTTOMALIGN;
		}
		else
		{
			pt.y = toolbarRect.bottom - 1;
			flags |= TPM_VERPOSANIMATION | TPM_TOPALIGN;
		}
	}

	if (FALSE != fUseHost)
	{
		RECT windowRect;
		GetClientRect(hControl, &windowRect);
		pt.x = windowRect.left;
		pt.y = windowRect.top;
		MapWindowPoints(hControl, HWND_DESKTOP, &pt, 1);
		flags |= TPM_VERPOSANIMATION | TPM_TOPALIGN;
	}

	if (NULL != hToolbar && 0 != (TBS_AUTOHIDE & toolbarStyle))
		SetWindowLongPtr(hToolbar, GWL_STYLE, toolbarStyle & ~TBS_AUTOHIDE);

	HWND hStatusbar = BrowserControl_GetStatusbar(hControl);
	DWORD statusStyle = (NULL != hStatusbar) ? GetWindowStyle(hStatusbar) : 0;
	if (NULL != hStatusbar && 0 == (WS_DISABLED & statusStyle))
		SetWindowLongPtr(hStatusbar, GWL_STYLE, WS_DISABLED | statusStyle);

	SendMessage(hBrowser, NBHM_SHOWHISTORYPOPUP, (WPARAM)flags, MAKELPARAM(pt.x, pt.y));

	if (NULL != hToolbar && 0 != (TBS_AUTOHIDE & toolbarStyle))
	{
		SetWindowLongPtr(hToolbar, GWL_STYLE, toolbarStyle);
		TRACKMOUSEEVENT tm;
		tm.cbSize = sizeof(TRACKMOUSEEVENT);
		tm.dwFlags = TME_LEAVE;
		tm.hwndTrack = hToolbar;
		TrackMouseEvent(&tm);
	}

	if (NULL != hStatusbar && 0 == (WS_DISABLED & statusStyle))
		SetWindowLongPtr(hStatusbar, GWL_STYLE, statusStyle);
}

static BOOL BrowserControl_ExecServiceCommand(HWND hControl, ifc_omservicecommand *command, const GUID *group, UINT id, ULONG_PTR arg)
{
	if (NULL == command) return FALSE;

	HRESULT state = command->QueryState(hControl, group, id);
	if (CMDSTATE_ENABLED == state)
	{
		command->Exec(hControl, group, id, arg);
		return TRUE;
	}
	return (CMDSTATE_DISABLED == state) ? TRUE : FALSE;
}

BOOL BrowserControl_ProcessCommonCommand(HWND hControl, INT commandId)
{
	if (NULL == hControl) 
		return FALSE;
	
	UINT controlStyle = GetWindowStyle(hControl);
	ifc_omservice *service;
	if (0 != (NBCS_NOSERVICECOMMANDS & controlStyle) || 
		FALSE == BrowserControl_GetService(hControl, &service))
	{
		service = NULL;
	}
	
	ifc_omservicecommand *serviceCommand;
	if (NULL == service || 
		FAILED(service->QueryInterface(IFC_OmServiceCommand, (void**)&serviceCommand)))
	{
		serviceCommand = NULL;
	}
			
	BOOL fProcessed = FALSE;

	switch(commandId)
	{
		case ID_NAVIGATION_HOME:	
			if (FALSE == BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_HOME, 0))
				BrowserControl_Navigate(hControl, NAVIGATE_HOME, 0L);
			fProcessed = TRUE;
			break;

		case ID_NAVIGATION_BACK:
			if (FALSE == BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_BACKFORWARD, FALSE))
				BrowserControl_Navigate(hControl, NAVIGATE_BACK, 0L);
			fProcessed = TRUE;
			break;

		case ID_NAVIGATION_FORWARD: 
			if (FALSE == BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_BACKFORWARD, TRUE))
				BrowserControl_Navigate(hControl, NAVIGATE_FORWARD, 0L);
			fProcessed = TRUE;
			break;

		case ID_NAVIGATION_REFRESH: 
			if (FALSE == BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_REFRESH, FALSE))
				BrowserControl_Navigate(hControl, NAVIGATE_REFRESH, 0L);
			fProcessed = TRUE;
			break;

		case ID_NAVIGATION_REFRESH_COMPLETELY:
			if (FALSE == BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_REFRESH, TRUE))
				BrowserControl_Navigate(hControl, NAVIGATE_REFRESH_COMPLETELY, 0L);
			fProcessed = TRUE;
			break;
		
		case ID_NAVIGATION_STOP:
			if (FALSE == BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_STOP, 0L))
				BrowserControl_Navigate(hControl, NAVIGATE_STOP, 0L);
			fProcessed = TRUE;
			break;

		case ID_NAVIGATION_HISTORY:	
			if (FALSE == BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_HISTORY, 0L))
				BrowserControl_ShowHistoryPopup(hControl);
			fProcessed = TRUE;
			break;

		case ID_SERVICE_GETINFO:
			BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_SHOWINFO, 0);
			fProcessed = TRUE;
			break;
	
		case ID_SERVICE_REPORT:
			BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_REPORT, 0);
			fProcessed = TRUE;
			break;

		case ID_SERVICE_UNSUBSCRIBE:
			BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_UNSUBSCRIBE, 0);
			fProcessed = TRUE;
			break;

		case ID_RATING_VALUE_1:
			BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_RATE, 1);
			fProcessed = TRUE;
			break;

		case ID_RATING_VALUE_2:
			BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_RATE, 2);
			fProcessed = TRUE;
			break;

		case ID_RATING_VALUE_3:
			BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_RATE, 3);
			fProcessed = TRUE;
			break;

		case ID_RATING_VALUE_4:
			BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_RATE, 4);
			fProcessed = TRUE;
			break;

		case ID_RATING_VALUE_5:
			BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_RATE, 5);
				fProcessed = TRUE;
			break;

		case ID_WINDOW_CLOSE:
			if (FALSE != BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_WINDOW, WNDCOMMAND_CLOSE, 0))
				fProcessed = TRUE;
			break;

		case ID_WINDOW_FULLSCREEN:
			if (FALSE != BrowserControl_ExecServiceCommand(hControl, serviceCommand, &CMDGROUP_WINDOW, WNDCOMMAND_FULLSCREEN, 0))
				fProcessed = TRUE;
			break;
		case ID_ADDRESSBAR_ACTIVATE:
			{
				HWND hToolbar = BrowserControl_GetToolbar(hControl);
				if(NULL != hToolbar)
				{
					Toolbar_NextItem(hToolbar, TOOLITEM_ADDRESSBAR, TRUE);
				}
			}
			fProcessed = TRUE;
			break;
		case ID_ADDRESSBAR_CHANGED:
			{
				HWND hToolbar = BrowserControl_GetToolbar(hControl);
				if(NULL != hToolbar)
				{
					INT itemId = Toolbar_FindItem(hToolbar, TOOLITEM_ADDRESSBAR);
					if (ITEM_ERR != itemId)
					{
						size_t cchBufferMax;
						if (FALSE == Toolbar_GetTextLength(hToolbar, MAKEINTRESOURCE(itemId), &cchBufferMax))
							cchBufferMax = 8192;
						
						cchBufferMax++;
						LPWSTR pszBuffer = Plugin_MallocString(cchBufferMax);
						if (NULL != pszBuffer)
						{
							TBITEMINFO itemInfo;
							ZeroMemory(&itemInfo, sizeof(itemInfo));

							
							itemInfo.pszText = pszBuffer;
							itemInfo.cchText = (INT)cchBufferMax;
							if (FALSE != Toolbar_GetItemInfo(hToolbar, MAKEINTRESOURCE(itemId), &itemInfo))
							{
								if (FALSE == BrowserControl_ExecServiceCommand(hControl, serviceCommand, 
												&CMDGROUP_ADDRESSBAR, ADDRESSCOMMAND_EXECUTE, (ULONG_PTR)itemInfo.pszText))
								{
									BrowserControl_Navigate(hControl, NAVIGATE_STOP, 0L);
									BrowserControl_Navigate(hControl, itemInfo.pszText, 0L);
								}
							}

							Plugin_FreeString(pszBuffer);
						}
					}
				}
			}
			fProcessed = TRUE;
			break;
	}

	if (NULL != serviceCommand)
		serviceCommand->Release();

	if (NULL != service)
		service->Release();

	return fProcessed;
}

BOOL BrowserControl_ProcessStatusbarCommand(HWND hControl, INT commandId)
{
	HWND hStatusbar = BrowserControl_GetStatusbar(hControl);
	if (NULL == hStatusbar) return FALSE;

	BOOL fProcessed = FALSE;
	obj_ombrowser *browserManager;
	if (FALSE != BrowserControl_GetBrowserObject(hControl, &browserManager) && NULL != browserManager)
	{
		ifc_omstatusbarconfig *statusbarConfig;

		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmStatusbarConfig, (void**)&statusbarConfig)))
		{
			UINT statusbarStyle = GetWindowStyle(hStatusbar);
			switch(commandId)
			{
				case SBN_ENABLECHANGED:
					statusbarConfig->EnableStatusbar(0 == (WS_DISABLED & statusbarStyle));
					fProcessed = TRUE;
					break;
			}
			statusbarConfig->Release();
		}
		browserManager->Release();
	}
	return fProcessed;
}

BOOL BrowserControl_ProcessToolbarCommand(HWND hControl, INT commandId)
{
	HWND hToolbar = BrowserControl_GetToolbar(hControl);
	if (NULL == hToolbar) return FALSE;

	BOOL fProcessed = FALSE;
	obj_ombrowser *browserManager;
	if (FALSE != BrowserControl_GetBrowserObject(hControl, &browserManager) && NULL != browserManager)
	{
		ifc_omtoolbarconfig *toolbarConfig;

		if (SUCCEEDED(browserManager->GetConfig(&IFC_OmToolbarConfig, (void**)&toolbarConfig)))
		{
			UINT toolbarStyle = GetWindowStyle(hToolbar);
			switch(commandId)
			{
				case TBN_DOCKCHANGED:
					toolbarConfig->EnableBottomDock(0 != (TBS_BOTTOMDOCK & toolbarStyle));
					fProcessed = TRUE;
					break;
				case TBN_AUTOHIDECHANGED:
					toolbarConfig->EnableAutoHide(0 != (TBS_AUTOHIDE & toolbarStyle));
					fProcessed = TRUE;
					break;
				case TBN_TABSTOPCHANGED:
					toolbarConfig->EnableTabStop(0 != (TBS_TABSTOP & toolbarStyle));
					fProcessed = TRUE;
					break;
			}
			toolbarConfig->Release();
		}
		browserManager->Release();
	}
	return fProcessed;
}

BOOL BrowserControl_ProcessAppCommand(HWND hControl, INT commandId)
{
	if (NULL == hControl)
		return FALSE;

	switch(commandId)
	{
		case APPCOMMAND_BROWSER_BACKWARD:	BrowserControl_ProcessCommonCommand(hControl, ID_NAVIGATION_BACK); return TRUE;
		case APPCOMMAND_BROWSER_FORWARD:	BrowserControl_ProcessCommonCommand(hControl, ID_NAVIGATION_FORWARD); return TRUE;
		case APPCOMMAND_BROWSER_HOME:		BrowserControl_ProcessCommonCommand(hControl, ID_NAVIGATION_HOME); return TRUE;
		case APPCOMMAND_BROWSER_REFRESH:	BrowserControl_ProcessCommonCommand(hControl, ID_NAVIGATION_REFRESH); return TRUE;
		case APPCOMMAND_BROWSER_STOP:		BrowserControl_ProcessCommonCommand(hControl, ID_NAVIGATION_STOP); return TRUE;
		case APPCOMMAND_BROWSER_FAVORITES:	return TRUE;
		case APPCOMMAND_BROWSER_SEARCH:		return TRUE;
	}

	return FALSE;
}

HWND BrowserControl_GetOperationWidget(HWND hControl)
{
	return GetDlgItem(hControl, IDC_OPERATIONWIDGET);
}

HWND BrowserControl_CreateOperationWidget(HWND hControl)
{
	if (FALSE == Curtain_RegisterClass(Plugin_GetInstance()))
		return NULL;
			
	RECT rc;
	HWND hTarget = BrowserControl_GetHost(hControl);
	if (NULL == hTarget) hTarget = hControl;

	if (FALSE == GetWindowRect(hTarget, &rc))
		return NULL;
	
	MapWindowPoints(HWND_DESKTOP, hControl, (POINT*)&rc, 2);
	
	
	HWND hWidget = CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CONTROLPARENT, 
				NWC_ONLINEMEDIACURTAIN, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS /*| WS_VISIBLE*/, 
				rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hControl, 
				(HMENU)IDC_OPERATIONWIDGET, Plugin_GetInstance(), NULL);


	return hWidget;

}

BOOL BrowserControl_OnShowOperation(HWND hControl, OPERATIONINFO *poi)
{
	if (NULL == poi) return FALSE;
	if (poi->cbSize != sizeof(OPERATIONINFO))
		return FALSE;

	BOOL updateSkin = FALSE;
	HWND hWidget = BrowserControl_GetOperationWidget(hControl);

	if (0 != (NBCOM_FLAGS & poi->mask))
	{
		if (0 != (NBCOF_SHOWWIDGET & poi->flags))
		{
			if (NULL == hWidget) 
			{
				hWidget = BrowserControl_CreateOperationWidget(hControl);
				if (NULL != hWidget)
					updateSkin = TRUE;
			}
		}

		if (0 != (NBCOF_HIDEWIDGET & poi->flags))
		{
			if (NULL != hWidget)
			{
				DestroyWindow(hWidget);
				hWidget = NULL;

				SetWindowPos(hControl, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | 
							SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS | SWP_DEFERERASE);
			}
		}
	}


	if (0 != (NBCOM_TITLE & poi->mask))
	{
		if (NULL != hWidget)
			SetWindowText(hWidget, poi->title);
	}

	if (0 != (NBCOM_TEXT & poi->mask))
	{
		if (NULL != hWidget)
			Curtain_SetOperationText(hWidget, poi->text);
	}

	if (NULL != hWidget && 0 != (NBCOM_FLAGS & poi->mask) && 0 != (NBCOF_SHOWWIDGET & poi->flags))
	{
		if (FALSE != updateSkin)
		{
			PostMessage(hWidget, CWM_UPDATESKIN, 0, 0L);
			SetWindowPos(hControl, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | 
					SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS | SWP_DEFERERASE);
		}

		ShowWindowAsync(hWidget, SW_SHOWNA);
	}
		
	return TRUE;
}


BOOL BrowserControl_UpdateLayout(HWND hControl, BOOL fRedraw, BOOL fFrame, HRGN updateRegion, const POINT *updateOffset)
{
	RECT clientRect;
	if (!GetClientRect(hControl, &clientRect))
		return FALSE;

	UINT uFlags = SWP_NOZORDER | SWP_NOACTIVATE;
	if (FALSE == fRedraw) uFlags |= SWP_NOREDRAW;
	if (FALSE != fFrame) uFlags |= SWP_FRAMECHANGED;

	HWND hToolbar, hBrowser, hStatusbar, hWidget;
	
	BOOL toolbarSet = FALSE;
	TOOLBARLAYOUT layout;
	layout.prcParent = &clientRect;
	
	HDWP hdwp = BeginDeferWindowPos(2);
	if (NULL == hdwp) return FALSE;

	if (NULL != (hToolbar = BrowserControl_GetToolbar(hControl)))
	{
		if (Toolbar_Layout(hToolbar, &layout))
		{
			toolbarSet = TRUE;
			hdwp = DeferWindowPos(hdwp, hToolbar, layout.insertAfter, layout.toolbarRect.left, layout.toolbarRect.top, 
					layout.toolbarRect.right - layout.toolbarRect.left,
					layout.toolbarRect.bottom - layout.toolbarRect.top, uFlags & ~SWP_NOZORDER);
			if (NULL == hdwp) return FALSE;
		}
	}

	if (FALSE == toolbarSet)
		CopyRect(&layout.clientRect, layout.prcParent);

	if (NULL != (hBrowser = BrowserControl_GetHost(hControl)))
	{	
		if (0 != (NBHS_BROWSERREADY & GetWindowStyle(hBrowser)))
		{
			SetWindowPos(hBrowser, NULL, layout.clientRect.left, layout.clientRect.top, 
					layout.clientRect.right - layout.clientRect.left, layout.clientRect.bottom - layout.clientRect.top,
					uFlags | SWP_ASYNCWINDOWPOS |  SWP_DEFERERASE);
		}
	}	

	if (NULL != (hWidget = BrowserControl_GetOperationWidget(hControl)))
	{
		HWND hInsertAfter;
		if (NULL != hBrowser) hInsertAfter = GetWindow(hBrowser, GW_HWNDPREV);
		else if( NULL != hToolbar) hInsertAfter = GetWindow(hBrowser, GW_HWNDPREV);
		else hInsertAfter = HWND_TOP;

		UINT flags;
		flags = uFlags;
		if (hInsertAfter != hWidget)
			flags &= ~SWP_NOZORDER;

		hdwp = DeferWindowPos(hdwp, hWidget, hInsertAfter, layout.clientRect.left, layout.clientRect.top, 
			layout.clientRect.right - layout.clientRect.left, layout.clientRect.bottom - layout.clientRect.top, flags);

		if (NULL == hdwp) return FALSE;
	}

	EndDeferWindowPos(hdwp);


	if (NULL != (hStatusbar = BrowserControl_GetStatusbar(hControl)))
		Statusbar_SetParentRect(hStatusbar, &layout.clientRect);
	
////////////////////////////////////

	if (NULL != updateRegion)
	{
		RECT rect;
		HRGN rgnValid = NULL;
	
		if (NULL != hBrowser && 0 != (WS_VISIBLE & GetWindowStyle(hBrowser)) && 
			FALSE != GetWindowRect(hBrowser, &rect))
		{
			MapWindowPoints(HWND_DESKTOP, hControl, (POINT*)&rect, 2);
			rgnValid = CreateRectRgnIndirect(&rect);
		}

		if (NULL != rgnValid)
		{
			HRGN rgn = NULL;
			HWND szControls[] = {hToolbar, hStatusbar, hWidget};
			for (INT i = 0; i < ARRAYSIZE(szControls); i++)
			{
				if (NULL != szControls[i] && 0 != (WS_VISIBLE & GetWindowStyle(szControls[i])) && 
					FALSE != GetWindowRect(szControls[i], &rect))
				{
					MapWindowPoints(HWND_DESKTOP, hControl, (POINT*)&rect, 2);
					if (NULL == rgn) rgn = CreateRectRgnIndirect(&rect);
					else SetRectRgn(rgn, rect.left, rect.top, rect.right, rect.bottom);

					if (NULL != rgn) 
						CombineRgn(rgnValid, rgnValid, rgn, RGN_DIFF);
				}
			}
			
			if (NULL != updateOffset)
				OffsetRgn(rgnValid, -updateOffset->x, -updateOffset->y);

			CombineRgn(updateRegion, updateRegion, rgnValid, RGN_DIFF);
			if (NULL != rgn) DeleteObject(rgn);
			DeleteObject(rgnValid);
		}
	}

	return TRUE;
}

BOOL BrowserControl_EnableChildren(HWND hControl, BOOL fEnable)
{
	HWND hChild;
	if (NULL != (hChild = BrowserControl_GetHost(hControl)))
		PostMessage(hChild, NBHM_ENABLEWINDOW, 0, fEnable);

	if (NULL != (hChild = BrowserControl_GetToolbar(hControl)))
		EnableWindow(hChild, fEnable);

	if (NULL != (hChild = BrowserControl_GetStatusbar(hControl)))
		EnableWindow(hChild, fEnable);

	return TRUE;
}

BOOL BrowserControl_SetBlockedState(HWND hControl, BOOL fBlocked)
{
	if (NULL == hControl) 
		return FALSE;

	UINT prevStyle = BrowserControl_SetExtendedStyle(hControl, NBCS_EX_BLOCKNAVIGATION, (FALSE != fBlocked) ? NBCS_EX_BLOCKNAVIGATION : 0);	
	if (FALSE != fBlocked)
	{		
		if (0 == (NBCS_EX_BLOCKNAVIGATION & prevStyle))
		{
			BrowserControl_EnableChildren(hControl, FALSE);
		}
	}
	else
	{
		if (0 != (NBCS_EX_BLOCKNAVIGATION & prevStyle))
		{
			BrowserControl_EnableChildren(hControl, TRUE);
		}
		BrowserControl_NavigateStoredUrl(hControl);
	}
	return TRUE;
}