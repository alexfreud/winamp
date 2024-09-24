#pragma once

#include "./main.h"
#include <commctrl.h>

#include "./playlist.h"

#define WM_BURNER				((WM_USER) + 0x400)

#define WM_BURNGETSTATUS			((WM_BURNER) + 0x001)  
#define WM_BURNGETITEMSTATUS		((WM_BURNER) + 0x002)  
#define WM_BURNUPDATEOWNER		((WM_BURNER) + 0x003)   // wParam = 0; lParam = ownerWnd
#define WM_BURNCONFIGCHANGED		((WM_BURNER) + 0x004)   // wParam = changed item; lParam = new value

#define WM_BURNNOTIFY			((WM_BURNER) + 0x100)   // wParam = Notify code, lParam notify  data

// Notification types
#define BURN_READY				0xFFF  // lParam = hwnd
#define BURN_DESTROYED			0x001
#define BURN_WORKING				0x002  
#define BURN_FINISHED			0x003
#define BURN_STATECHANGED		0x004
#define BURN_CONFIGCHANGED		0x005


#define BURN_ITEMSTATECHANGED	0x010
#define BURN_ITEMDECODEPROGRESS	0x011
#define BURN_ITEMBURNPROGRESS	0x012

// status types
#define BURNSTATUS_DRIVE		0x0000
#define BURNSTATUS_ELAPSED		0x0001
#define BURNSTATUS_ESTIMATED		0x0002
#define BURNSTATUS_PROGRESS		0x0003
#define BURNSTATUS_STATE		0x0004
#define BURNSTATUS_ERROR		0x0005


#define BURNPLAYLISTUI_SUCCESS			0x0000
#define BURNPLAYLISTUI_PRIMOSDKNOTSET	0x0105


//stages
#define PLSTAGE_READY		0x00
#define PLSTAGE_LICENSED	0x01
#define PLSTAGE_DECODED		0x02
#define PLSTAGE_BURNED		0x03

// config items
#define BURNCFG_AUTOCLOSE		0x01
#define BURNCFG_AUTOEJECT		0x02
#define BURNCFG_ADDTODB			0x03
#define BURNCFG_HIDEVIEW			0x04

class BurnPlaylistUI
{
public:
	BURNLIB_API BurnPlaylistUI(void);
	BURNLIB_API ~BurnPlaylistUI(void);

public:
	BURNLIB_API DWORD Burn(obj_primo *primoSDK, DWORD drive, DWORD maxspeed, DWORD burnFlags,
							BurnerPlaylist *playlist, const wchar_t* tempPath, HWND ownerWnd);

	
protected:
	static DWORD CALLBACK OnLicensingPlaylist(void *sender, void *userparam, DWORD notifyCode, DWORD errorCode, ULONG_PTR param);
	static DWORD CALLBACK OnDecodePlaylist(void *sender, void *userparam, DWORD notifyCode, DWORD errorCode, ULONG_PTR param);
	static DWORD CALLBACK OnBurnPlaylist(void *sender, void *userparam, DWORD notifyCode, DWORD errorCode, ULONG_PTR param);
	static LRESULT CALLBACK WndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnLicense(void);
	void OnDecode(void);
	void OnBurn(void);
	void OnInitDialog(HWND hwndDlg);
	void OnCancel(void);
	void OnDestroy(void);
	void SetExtendedView(BOOL extView);
	void SetColumns(void);
	void FillList(void);
	void SetProgress(int position);
	void UpdateTime(BOOL recalcEstimates);
	void ReportError(unsigned int stringCode, BOOL allowContinue);
	void ReportError(const wchar_t *errorString, BOOL allowContinue);
	DWORD DrawList(NMLVCUSTOMDRAW* cd);
	HBITMAP CreateStripBmp(HDC compDC);
	void SetReadyClose(BOOL ready);
	void UpdateItemStatus(int index);
	void SetItemStatusText(int index, unsigned int stringCode, BOOL redraw);
	void SetCurrentOperation(unsigned int stringCode);
	int MessageBox(unsigned int messageCode, unsigned int captionCode, unsigned int uType);
protected:

	struct aproxtime
	{
		DWORD license;
		DWORD convert;
		DWORD transition;
		DWORD chkdisc;
		DWORD init;
		DWORD leadin;
		DWORD burn;
		DWORD leadout;
		DWORD finish;
	};

protected:
	HWND				hwnd;
	HWND				ownerWnd;
	DWORD			drive;
	DWORD			maxspeed;
	DWORD			burnFlags;
	BOOL			extendedView;
	DWORD			errCode;
	obj_primo			*primoSDK;
	BurnerPlaylist	*playlist;
	unsigned int	startedTime;
	unsigned int	estimatedTime;
	wchar_t			*tmpfilename;	
	HANDLE			hTmpFile;
	int				currentPercent;
	DWORD			prevRefresh;
	HBITMAP			stripBmp;
	BOOL			cancelOp;
	HANDLE			workDone;
	aproxtime		estimated;
	BOOL			readyClose;
	DWORD			controlTime;
	DWORD			realSpeed;
	DWORD			stage;
	DWORD			count; // count of items to process (actual)
	DWORD			processed; // count of actually processed items

};