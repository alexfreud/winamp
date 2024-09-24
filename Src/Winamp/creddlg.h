#ifndef NULLSOFT_CREDAENTIAL_DIALOG_HEADER
#define NULLSOFT_CREDAENTIAL_DIALOG_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#include <windows.h>

#define CDS_USEUSERNAME		0x0001	// szUser will be used to prepopulate user name field
#define CDS_USEPASSWORD		0x0002	// szPassword will be used to prepopulate user name field
#define CDS_SKINDIALOG		0x0004	// Dialog will be skinned (requires hwndWA)
#define CDS_APPMODAL			0x0008	// Dialog will be application(thread) modal.

typedef struct _WACREDDLG
{
	int		size;			// sizeof(WACREDDLG)
	HWND		hwndParent;		// parents HWND  (passing NULL can be bad idea especially if not CDS_APPMODAL)
	LPWSTR	szUser;			// pointer to the user name buffer 
	INT		cchUser;		// size of the user name buffer in characters	
	LPWSTR	szPassword;		// pointer to the password buffer
	INT		cchPassword;		// size of the password buffer in characters
	DWORD	flags;			// any combination of CDS_XXX
	LPCWSTR	title;			// title of the dialog
	HBITMAP	hbmp;			// bitmap to display (can be NULL - this will make dialog smaller)
	LPCWSTR greating;		// text to display on top of user name filed
	HWND		hwndWA;			// only if you want skinning handle to the Winamp main window

} WACREDDLG, *PWACREDDLG; 

#ifdef __cplusplus
extern "C" {
#endif

INT ShowCredentialDialog(const WACREDDLG *pcd); // displays dialog. Returns: error(-1), canceled(0), ok(1)

#ifdef __cplusplus
}
#endif

//Expample
	//wchar_t usr[64], pwd[64];
	//WACREDDLG dlg;
	//ZeroMemory(&dlg, sizeof(WACREDDLG));
	//dlg.size = sizeof(WACREDDLG);
	//dlg.hwndWA = plugin.hwndParent;
	//dlg.hwndParent = g_hwnd;
	//dlg.flags = CDS_APPMODAL | CDS_USEPASSWORD | CDS_USEUSERNAME | CDS_SKINDIALOG;
	//dlg.title = L"User Credentials";
	//dlg.greating = L"Resource that you trying to access requires authentification.\nPlease enter credentials.";
	//dlg.szUser = usr;
	//dlg.cchUser = 64;
	//dlg.szPassword = pwd;
	//dlg.cchPassword = 64;
	//dlg.hbmp = (HBITMAP)LoadImage(NULL, "C:\\cred_banner.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	//StringCchCopyW(usr, 64, L"testuser");
	//StringCchCopyW(pwd, 64, L"12345");

	//
	//
	//wchar_t buffer[256];

	//switch(ShowCredentialDialog(&dlg))
	//{
	//	case -1:	StringCchCopyW(buffer, 256, L"Error duaring initialization."); break;
	//	case 0:		StringCchCopyW(buffer, 256, L"Canceled by user."); break;
	//	default:	StringCchPrintfW(buffer, 256, L"Userdata:\nUser name:\t\t%s\nPassword:\t\t%s", usr, pwd); 	break;
	//	
	//}

	//if (dlg.hbmp) DeleteObject(dlg.hbmp);
// Example end



#endif /*NULLSOFT_CREDAENTIAL_DIALOG_HEADER*/