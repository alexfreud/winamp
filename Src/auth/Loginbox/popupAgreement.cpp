#include "./popupAgreement.h"
#include "./loginNotifier.h"
#include "./loginProvider.h"

#include "./common.h"
#include "../resource.h"
#include "../api.h"

#include "../../winamp/commandLink.h"

#include <windows.h>
#include <commctrl.h>
#include <strsafe.h>

#define LINK_TOS		0
#define STATIC_AND		1
#define LINK_PRIVACY	2

#define IDC_AOL					11000
#define IDC_AOL_TOS				(IDC_AOL + LINK_TOS)
#define IDC_AOL_AND				(IDC_AOL + STATIC_AND)
#define IDC_AOL_PRIVACY			(IDC_AOL + LINK_PRIVACY)

#define IDC_3DPARTY				11010
#define IDC_3DPARTY_TOS			(IDC_3DPARTY + LINK_TOS)
#define IDC_3DPARTY_AND			(IDC_3DPARTY + STATIC_AND)
#define IDC_3DPARTY_PRIVACY		(IDC_3DPARTY + LINK_PRIVACY)


#define LINK_TARGET		L"LinkTargetProp"

typedef struct __PARTSIZE
{
	HWND hwnd;
	LONG cx;
	LONG cy;
} PARTSIZE;

typedef struct __LINKSIZEINFO
{
	PARTSIZE tos;
	PARTSIZE and;
	PARTSIZE privacy;
	LONG	spaceWidth;
	RECT	linkMargins;
} LINKSIZEINFO;

static HRESULT CALLBACK LoginPopupAgreement_CreateInstance(HWND hwnd, LPARAM param, LoginPopup **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hwnd) return E_INVALIDARG;

	*instance = new LoginPopupAgreement(hwnd);
	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

LoginPopupAgreement::LoginPopupAgreement(HWND hwnd)
	: LoginPopup(hwnd, -1, MAKEINTRESOURCE(IDS_POPUP_AGREEMENT_TITLE))
{
}

LoginPopupAgreement::~LoginPopupAgreement()
{
	
}

HWND LoginPopupAgreement::CreatePopup(HWND hParent, LoginProvider *provider)
{
	if (NULL == provider) 
		return NULL;

	return LoginPopup::CreatePopup(MAKEINTRESOURCE(IDD_POPUP_AGREEMENT), hParent, (LPARAM)provider, LoginPopupAgreement_CreateInstance);
}

void LoginPopupAgreement::UpdateLayout(BOOL fRedraw)
{
	LoginPopup::UpdateLayout(fRedraw);

	RECT rect;
	if (FALSE == GetInfoRect(&rect)) return;
		
	const INT szButtons[] = { IDOK, IDCANCEL, };
	
	
	UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
	if (FALSE == fRedraw) flags |= SWP_NOREDRAW;

	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(szButtons) + 1 + 2*3);
	if (NULL == hdwp) return;
	
	hdwp = LayoutButtons(hdwp, szButtons, ARRAYSIZE(szButtons), fRedraw, NULL);

	LONG top = rect.top;
	SIZE partSize;
	HWND hText = GetDlgItem(hwnd, IDC_TEXT);
	if (NULL != hText && FALSE != GetTextSize(hText, rect.right - rect.left, &partSize))
	{
		hdwp = DeferWindowPos(hdwp, hText, NULL, rect.left, top, partSize.cx, partSize.cy, flags);
		if (NULL == hdwp) return;
		top += partSize.cy;
	}

	top += marginLinkFirst;
	hdwp = LayoutProviderLinks(hdwp, IDC_AOL, NULL, rect.left + marginLinkLeft, top, flags, &partSize);
	if (NULL == hdwp) return;
	
	if (0 != partSize.cy)
		top += partSize.cy + marginLinkNext;
	hdwp = LayoutProviderLinks(hdwp, IDC_3DPARTY, NULL, rect.left + marginLinkLeft, top, flags, &partSize);
	if (NULL == hdwp) return;

	EndDeferWindowPos(hdwp);

	if (FALSE != fRedraw)
	{
		HWND hControl = GetDlgItem(hwnd, IDC_TEXT);
		if (NULL != hControl) InvalidateRect(hControl, NULL, FALSE);
	}
}

void LoginPopupAgreement::EndDialog(INT_PTR code)
{
	NLPNRESULT result;
	result.exitCode = code;
	SendNotification(NLPN_RESULT, (NMHDR*)&result);

	LoginPopup::EndDialog(code);
}

void LoginPopupAgreement::UpdateMargins()
{
	RECT rect;
	SetRect(&rect, 8, 8, 0, 2);
	MapDialogRect(hwnd, &rect);

	marginLinkLeft = rect.left;
	marginLinkFirst = rect.top;
	marginLinkNext = rect.bottom;

	LoginPopup::UpdateMargins();
}

static BOOL ProviderLinks_GetSizeInfo(HWND hwnd, INT groupId, LINKSIZEINFO *sizeInfo)
{
	if(NULL == sizeInfo)
		return FALSE;

	HWND hControl;
	SIZE partSize;

	SetRectEmpty(&sizeInfo->linkMargins);

	hControl = GetDlgItem(hwnd, groupId + LINK_TOS);
	if (NULL == hControl || 
		0 == (WS_VISIBLE & GetWindowStyle(hControl)) || 
		FALSE == CommandLink_GetIdealSize(hControl, &partSize))
	{
		ZeroMemory(&sizeInfo->tos, sizeof(PARTSIZE));
	}
	else
	{
		sizeInfo->tos.hwnd = hControl;
		sizeInfo->tos.cx = partSize.cx;
		sizeInfo->tos.cy = partSize.cy;

		CommandLink_GetMargins(hControl, &sizeInfo->linkMargins);
	}

	hControl = GetDlgItem(hwnd, groupId + LINK_PRIVACY);
	if (NULL == hControl || 
		0 == (WS_VISIBLE & GetWindowStyle(hControl)) || 
		FALSE == CommandLink_GetIdealSize(hControl, &partSize))
	{
		ZeroMemory(&sizeInfo->privacy, sizeof(PARTSIZE));
	}
	else
	{
		sizeInfo->privacy.hwnd = hControl;
		sizeInfo->privacy.cx = partSize.cx;
		sizeInfo->privacy.cy = partSize.cy;

		if (IsRectEmpty(&sizeInfo->linkMargins))
			CommandLink_GetMargins(hControl, &sizeInfo->linkMargins);
	}
	
	if (NULL == sizeInfo->tos.hwnd && NULL == sizeInfo->privacy.hwnd)
		return FALSE;

	ZeroMemory(&sizeInfo->and, sizeof(PARTSIZE));
	sizeInfo->spaceWidth = 0;
	
	if (NULL != sizeInfo->tos.hwnd && NULL != sizeInfo->privacy.hwnd)
	{
		hControl = GetDlgItem(hwnd, groupId + STATIC_AND);
		if (NULL != hControl)
		{
			WCHAR szBuffer[64] = {0};
			INT cchLen = (INT)SendMessage(hControl, WM_GETTEXT, ARRAYSIZE(szBuffer), (LPARAM)szBuffer);
			if (cchLen > 0)
			{
				HDC hdc = GetDCEx(hControl, NULL, DCX_CACHE | DCX_WINDOW | DCX_NORESETATTRS);
				if (NULL != hdc)
				{
					HFONT font = (HFONT)SendMessage(hControl, WM_GETFONT, 0, 0L);
					HFONT fontOrig = (HFONT)SelectObject(hdc, font);
						
					if (FALSE != GetTextExtentPoint32W(hdc, szBuffer, cchLen, &partSize))
					{
						sizeInfo->and.hwnd = hControl;
						sizeInfo->and.cx = partSize.cx;
						sizeInfo->and.cy = partSize.cy;
					}

					if (FALSE != GetTextExtentPoint32W(hdc, L" ", 1, &partSize))
					{
						sizeInfo->spaceWidth = partSize.cx;
					}

					SelectObject(hdc, fontOrig);
					ReleaseDC(hControl, hdc);
				}
			}
		}
	}
	return TRUE;
}

HDWP LoginPopupAgreement::LayoutProviderLinks(HDWP hdwp, INT groupId, HWND hwndInsertAfter, INT x, INT y, UINT flags, SIZE *size)
{	
	if (NULL == hdwp)
	{
		if (NULL == size)
			return NULL;
		x = 0;
		y = 0;
	}

	LONG ox = x, cy = 0;
	
	LINKSIZEINFO sizeInfo;
	if (FALSE == ProviderLinks_GetSizeInfo(hwnd, groupId, &sizeInfo))
		return hdwp;
		
	if (NULL != sizeInfo.tos.hwnd)
	{
		if (NULL != hdwp)
		{
			hdwp = DeferWindowPos(hdwp, sizeInfo.tos.hwnd, hwndInsertAfter, x, y, sizeInfo.tos.cx, sizeInfo.tos.cy, flags);
			if (NULL == hdwp) return NULL;
		}

		x += sizeInfo.tos.cx;
		if (cy < sizeInfo.tos.cy) cy = sizeInfo.tos.cy;
	}

	if (NULL != sizeInfo.and.hwnd)
	{		
		LONG top = y + ((sizeInfo.tos.cy - (sizeInfo.linkMargins.bottom + sizeInfo.linkMargins.top)) - sizeInfo.and.cy)/2;
		LONG space = (sizeInfo.spaceWidth - sizeInfo.linkMargins.right);
		if (space < 1) space = 1;
		x += space;
		if (NULL != hdwp)
		{
			hdwp = DeferWindowPos(hdwp, sizeInfo.and.hwnd, hwndInsertAfter, x, top, sizeInfo.and.cx, sizeInfo.and.cy, flags);
			if (NULL == hdwp) return NULL;
		}

		x += sizeInfo.and.cx;
		if (cy < sizeInfo.and.cy) cy = sizeInfo.and.cy;
	}
	
	if (NULL != sizeInfo.privacy.hwnd)
	{	
		if (NULL != sizeInfo.and.hwnd)
		{
			LONG space = (sizeInfo.spaceWidth - sizeInfo.linkMargins.left);
			if (space < 1) space = 1;
			x += space;
		}

		if (NULL != hdwp)
		{
			hdwp = DeferWindowPos(hdwp, sizeInfo.privacy.hwnd, hwndInsertAfter, x, y, sizeInfo.privacy.cx, sizeInfo.privacy.cy, flags);
			if (NULL == hdwp) return NULL;
		}

		x += sizeInfo.privacy.cx;
		if (cy < sizeInfo.privacy.cy) cy = sizeInfo.privacy.cy;
		
	}
	
	if (NULL != size)
	{
		size->cx = (x - ox);
		size->cy = cy;
	}
	
	return (NULL == hdwp) ? (HDWP)(TRUE) : hdwp;
}

BOOL LoginPopupAgreement::CreateProviderLinks(LPCWSTR pszProvider, LPCWSTR pszTos, LPCWSTR pszPrivacy, INT groupId, HWND hwndInsertAfter)
{
	WCHAR szTemplate[256] = {0}, szBuffer[256] = {0};
	UINT linkStyle = WS_CHILD | WS_TABSTOP | WS_GROUP | WS_VISIBLE |
					CLS_DEFAULTCOLORS | CLS_HOTTRACK /*| CLS_ALWAYSUNDERLINE*/;

	HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);

	LPWSTR pszUrl;
	HWND hControl;
	INT createdCount = 0;
	INT failedCount = 0;
	if (NULL != pszTos && L'\0' != *pszTos)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_TOSLINK_TEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
		StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szTemplate, pszProvider);
		hControl = CommandLink_CreateWindow(0, szBuffer, linkStyle, 0, 0, 0, 0, hwnd, groupId + LINK_TOS);
		if (NULL != hControl)
		{
			pszUrl = LoginBox_CopyString(pszTos);
			if (NULL == pszUrl || FALSE == SetProp(hControl, LINK_TARGET, pszUrl))
			{
				LoginBox_FreeString(pszUrl);
				DestroyWindow(hControl);
				hControl = NULL;
			}
		}

		if (NULL != hControl)
		{	
			SendMessage(hControl, WM_SETFONT, (WPARAM)font, 0L);
			SetWindowPos(hControl, hwndInsertAfter, 0, 0,0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);
			hwndInsertAfter = hControl;
			createdCount++;
		}
		else failedCount++;
	}

	if (0 == failedCount && NULL != pszPrivacy && L'\0' != *pszPrivacy)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_PRIVACYLINK_TEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
		StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szTemplate, pszProvider);
		hControl = CommandLink_CreateWindow(0, szBuffer, linkStyle, 0, 0, 0, 0, hwnd, groupId + LINK_PRIVACY);
		if (NULL != hControl)
		{
			pszUrl = LoginBox_CopyString(pszPrivacy);
			if (NULL == pszUrl || FALSE == SetProp(hControl, LINK_TARGET, pszUrl))
			{
				LoginBox_FreeString(pszUrl);
				DestroyWindow(hControl);
				hControl = NULL;
			}
		}
		if (NULL != hControl)
		{
			SendMessage(hControl, WM_SETFONT, (WPARAM)font, 0L);
			SetWindowPos(hControl, hwndInsertAfter, 0, 0,0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);
			hwndInsertAfter = hControl;
			createdCount++;
		}
		else failedCount++;
	}

	if (0 == failedCount && createdCount > 1)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_AND, szBuffer, ARRAYSIZE(szBuffer));
		hControl = CreateWindowEx(WS_EX_NOPARENTNOTIFY, L"Static", szBuffer, WS_CHILD | WS_VISIBLE | SS_LEFT, 
			0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)(groupId + STATIC_AND), NULL, 0L);
		if (NULL != hControl)
		{
			SendMessage(hControl, WM_SETFONT, (WPARAM)font, 0L);
			SetWindowPos(hControl, hwndInsertAfter, 0, 0,0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);
			hwndInsertAfter = hControl;
			createdCount++;
		}
		else
			failedCount++;
	}
	
	if (0 != failedCount)
	{
		hControl = GetDlgItem(hwnd, (groupId + LINK_TOS));
		if (NULL != hControl) DestroyWindow(hControl);
		hControl = GetDlgItem(hwnd, (groupId + LINK_PRIVACY));
		if (NULL != hControl) DestroyWindow(hControl);
		hControl = GetDlgItem(hwnd, (groupId + STATIC_AND));
		if (NULL != hControl) DestroyWindow(hControl);
	}

	return (0 == failedCount);
}

BOOL LoginPopupAgreement::OnInitDialog(HWND hFocus, LPARAM param)
{
	LoginProvider *provider = (LoginProvider*)param;
	if (NULL != provider)
	{
		WCHAR szName[128] = {0}, szTos[4096] = {0}, szPrivacy[4096] = {0};
		if (FAILED(provider->GetTosLink(szTos, ARRAYSIZE(szTos))))
			szTos[0] = L'\0';
		if (FAILED(provider->GetPrivacyLink(szPrivacy, ARRAYSIZE(szPrivacy))))
			szPrivacy[0] = L'\0';
		
		if((L'\0' != szTos[0] || L'\0' != szPrivacy[0]) &&
			SUCCEEDED(provider->GetName(szName, ARRAYSIZE(szName))))
		{
			CreateProviderLinks(szName, szTos, szPrivacy, IDC_3DPARTY, NULL);
		}
	}

	CreateProviderLinks(L"AOL", L"https://new.aol.com/freeaolweb/resources/jsp/mem_tos.jsp", 
		L"http://about.aol.com/aolnetwork/mem_policy", IDC_AOL, NULL);
	
	LoginPopup::OnInitDialog(hFocus, param);

	HWND hAgree = GetDlgItem(hwnd, IDOK);
	if (NULL != hAgree && (WS_VISIBLE == ((WS_VISIBLE | WS_DISABLED) & GetWindowStyle(hAgree))))
	{
		PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hAgree, TRUE);
		return TRUE;
	}

	return FALSE;
}
void LoginPopupAgreement::OnDestroy()
{
	INT szLinks[] = {IDC_AOL_TOS, IDC_AOL_PRIVACY, IDC_3DPARTY_TOS, IDC_3DPARTY_PRIVACY, };
	for (INT i = 0; i < ARRAYSIZE(szLinks); i++)
	{
		HWND hLink = GetDlgItem(hwnd, szLinks[i]);
		if (NULL != hLink) DestroyWindow(hLink);
	}
}

BOOL LoginPopupAgreement::OnUpdateWindowPos(const RECT* clientRect, RECT *rectOut)
{
	if (NULL == clientRect || NULL == rectOut)
		return FALSE;

	SIZE maxSize;
	SIZE partSize;
	
	maxSize.cx = 0;
	maxSize.cy = 0;

	if (((HDWP)TRUE) == LayoutProviderLinks(NULL, IDC_AOL, NULL, 0, 0, 0, &partSize))
	{
		if (maxSize.cx < partSize.cx) maxSize.cx = partSize.cx;
		maxSize.cy += (partSize.cy + marginLinkFirst);
	}

	if (((HDWP)TRUE) == LayoutProviderLinks(NULL, IDC_3DPARTY, NULL, 0, 0, 0, &partSize))
	{
		if (maxSize.cx < partSize.cx) maxSize.cx = partSize.cx;
		maxSize.cy += (partSize.cy + ((0 == maxSize.cy) ? marginLinkFirst : marginLinkNext));
	}

	if (0 != maxSize.cx)
		maxSize.cx += marginLinkLeft;

	if (0 != maxSize.cy)
		maxSize.cy += marginLinkFirst;

	LONG maxWidth = clientRect->right - clientRect->left - 
					(clientMargins.right + clientMargins.left) -
					(infoMargins.right + infoMargins.left);

	if (maxSize.cx > maxWidth)
		maxSize.cx = maxWidth;

	if (FALSE != GetTextSize(GetDlgItem(hwnd, IDC_TEXT), maxWidth, &partSize))
	{
		if (maxSize.cx < partSize.cx) maxSize.cx = partSize.cx;
		maxSize.cy += partSize.cy;
	}
	
	if (FALSE == CalculateWindowRect(maxSize.cx, maxSize.cy, NULL, 0, TRUE, rectOut))
		return FALSE;
	

	LONG ox = clientRect->left + ((clientRect->right - clientRect->left) - (rectOut->right - rectOut->left))/2;
	LONG oy = clientRect->top + ((clientRect->bottom - clientRect->top) - (rectOut->bottom - rectOut->top))/2;

	if (ox < clientRect->left) ox = clientRect->left;
	if (oy < clientRect->top) oy = clientRect->top;
	
	OffsetRect(rectOut, ox, oy);
	return TRUE;
}
HBRUSH LoginPopupAgreement::OnGetStaticColor(HDC hdc, HWND hControl)
{
	HBRUSH hb = LoginPopup::OnGetStaticColor(hdc, hControl);
	INT controlId = (NULL != hControl) ? (INT)GetWindowLongPtr(hControl, GWLP_ID) : 0;
	switch(controlId)
	{
		case IDC_AOL_AND:
		case IDC_3DPARTY_AND:
			{
				HWND hLink = GetDlgItem(hwnd, (controlId - STATIC_AND));
				if (NULL != hLink)
				{				
			//		SetTextColor(hdc, CommandLink_GetTextColor(hLink));
				}
			}
			break;
	}

	return hb;
}

void LoginPopupAgreement::OnParentNotify(UINT eventId, UINT wParam, LPARAM lParam)
{
	switch(eventId)
	{
		case WM_DESTROY:
			switch(wParam)
			{
				case IDC_AOL_TOS:
				case IDC_AOL_PRIVACY:
				case IDC_3DPARTY_TOS:
				case IDC_3DPARTY_PRIVACY:
					{
						LPWSTR url = (LPWSTR)GetProp((HWND)lParam, LINK_TARGET);
						RemoveProp((HWND)lParam, LINK_TARGET);
						LoginBox_FreeString(url);
					}
					break;
			}
			break;

	}
	LoginPopup::OnParentNotify(eventId, wParam, lParam);
}


void LoginPopupAgreement::OnLinkClicked(HWND hLink)
{
	if (NULL == hLink)
		return;

	LPCWSTR pszTarget = (LPCWSTR)GetProp(hLink, LINK_TARGET);
	if (NULL != pszTarget && L'\0' != *pszTarget)
	{
		LoginBox_OpenUrl(hwnd, pszTarget, TRUE);
	}
}

LRESULT LoginPopupAgreement::OnNotify(UINT controlId, const NMHDR *pnmh)
{
	switch(controlId)
	{
		case IDC_AOL_TOS:
		case IDC_AOL_PRIVACY:
		case IDC_3DPARTY_TOS:
		case IDC_3DPARTY_PRIVACY:
			switch(pnmh->code)
			{
				case NM_CLICK:
					OnLinkClicked(pnmh->hwndFrom);
					return 0;
			}
		break;
	}
	return LoginPopup::OnNotify(controlId, pnmh);
}
