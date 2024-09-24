!ifndef NULLSOFT_NX_SYSTEM_NSIS_HEADER
!define NULLSOFT_NX_SYSTEM_NSIS_HEADER

;   LONG left; 
;   LONG top; 
;   LONG right; 
;   LONG bottom; 
; } RECT, *PRECT; 
!ifndef stRECT
!define stRECT '(i, i, i, i) i'
!endif

;typedef struct tagSIZE
;{
;    LONG        cx;
;    LONG        cy;
;} SIZE, *PSIZE, *LPSIZE;
!ifndef stSIZE
!define stSIZE '(i, i) i'
!endif

; typedef struct tagBITMAP {
;   LONG   bmType; 
;   LONG   bmWidth; 
;   LONG   bmHeight; 
;   LONG   bmWidthBytes; 
;   WORD   bmPlanes; 
;   WORD   bmBitsPixel; 
;   LPVOID bmBits; 
; } BITMAP, *PBITMAP; 
!ifndef stBITMAP
!define stBITMAP '(i, i, i, i, &i2, &i2, i) i'
!endif

;typedef struct _ICONINFO {
;  BOOL    fIcon;
;  DWORD   xHotspot;
;  DWORD   yHotspot;
;  HBITMAP hbmMask;
;  HBITMAP hbmColor;
;} ICONINFO, *PICONINFO;
!ifndef stICONINFO
!define stICONINFO '(i, i, i, i, i) i'
!endif

;typedef struct tagTEXTMETRICW
;{
;    LONG        tmHeight;
;    LONG        tmAscent;
;    LONG        tmDescent;
;    LONG        tmInternalLeading;
;    LONG        tmExternalLeading;
;    LONG        tmAveCharWidth;
;    LONG        tmMaxCharWidth;
;    LONG        tmWeight;
;    LONG        tmOverhang;
;    LONG        tmDigitizedAspectX;
;    LONG        tmDigitizedAspectY;
;    WCHAR       tmFirstChar;
;    WCHAR       tmLastChar;
;    WCHAR       tmDefaultChar;
;    WCHAR       tmBreakChar;
;    BYTE        tmItalic;
;   BYTE        tmUnderlined;
;    BYTE        tmStruckOut;
;    BYTE        tmPitchAndFamily;
;    BYTE        tmCharSet;
;} TEXTMETRICW
!ifndef stTEXTMETRIC
!define stTEXTMETRIC '(i, i, i, i, i, i, i, i, i, i, i, &i2, &i2, &i2, &i2, &i1, &i1, &i1, &i1, &i1) i'
!endif

;typedef struct tagLOGFONTW
;{
;    LONG      lfHeight;
;    LONG      lfWidth;
;    LONG      lfEscapement;
;    LONG      lfOrientation;
;    LONG      lfWeight;
;    BYTE      lfItalic;
;    BYTE      lfUnderline;
;    BYTE      lfStrikeOut;
;    BYTE      lfCharSet;
;    BYTE      lfOutPrecision;
;    BYTE      lfClipPrecision;
;    BYTE      lfQuality;
;    BYTE      lfPitchAndFamily;
;    WCHAR     lfFaceName[LF_FACESIZE];
;} LOGFONTW
!ifndef stLOGFONT
!define stLOGFONT '(i, i, i, i, i, &i1, &i1, &i1, &i1, &i1, &i1, &i1, &i1, &w32) i'
!endif

;typedef struct tagNMHDR {
;  HWND     hwndFrom;
;  UINT_PTR idFrom;
;  UINT     code;
;} NMHDR;
!ifndef stNMHDR
!define stNMHDR '(i, i, i) i'
!endif

!ifndef NM_FIRST
!define NM_FIRST			0
!endif

!ifndef NM_CLICK
!define /math NM_CLICK		${NM_FIRST} - 2
!endif

!ifndef NM_RETURN
!define /math NM_RETURN		${NM_FIRST} - 4
!endif

; BOOL GetWindowRect(HWND hWnd, 
;					 LPRECT lpRect)
!define fnGetWindowRect \
		'User32::GetWindowRect(i, i) i'

; int MapWindowPoints(HWND hWndFrom, 
;					  HWND hWndTo, 
;					  LPPOINT lpPoints, 
;					  UINT cPoints)
!define fnMapWindowPoints \
		'User32::MapWindowPoints(i, i, i, i) i'

		
; SetWindowPos() uFlags
!define SWP_NOSIZE          0x0001
!define SWP_NOMOVE          0x0002
!define SWP_NOZORDER        0x0004
!define SWP_NOREDRAW        0x0008
!define SWP_NOACTIVATE      0x0010
!define SWP_FRAMECHANGED    0x0020  
!define SWP_SHOWWINDOW      0x0040
!define SWP_HIDEWINDOW      0x0080
!define SWP_NOCOPYBITS      0x0100
!define SWP_NOOWNERZORDER   0x0200  
!define SWP_NOSENDCHANGING  0x0400  
!define SWP_DRAWFRAME       ${SWP_FRAMECHANGED}
!define SWP_NOREPOSITION    ${SWP_NOOWNERZORDER}
!define SWP_DEFERERASE      0x2000
!define SWP_ASYNCWINDOWPOS  0x4000 

; BOOL SetWindowPos(HWND hWnd, 
;					HWND hWndInsertAfter, 
;					int X, 
;					int Y, 
;					int cx, 
;					int cy, 
;					UINT uFlags)
!define fnSetWindowPos \
		'User32::SetWindowPos(i, i, i, i, i, i, i) i'
		
		
; GetAncestor() gaFlags
!define GA_PARENT 1
		
; HWND GetAncestor(HWND hwnd,
;				   UINT gaFlags)
!define fnGetAncestor \
		'User32::GetAncestor(i, i) i'

; BOOL MapDialogRect(HWND hDlg,
;					 LPRECT lpRect)
!define fnMapDialogRect \
		'User32::MapDialogRect(i, i) i'

; GetDCEx() flags		
!define DCX_WINDOW			0x00000001
!define DCX_CACHE			0x00000002
!define DCX_NORESETATTRS	0x00000004

; HDC GetDCEx(HWND hWnd,
;			  HRGN hrgnClip,
;  			  DWORD flags)
!define fnGetDCEx \
		'User32::GetDCEx(i, i, i ) i'

; DrawText uFormat		
!define DT_CALCRECT			0x00000400
!define DT_LEFT				0x00000000
!define DT_TOP				0x00000000
!define DT_EDITCONTROL		0x00002000
!define DT_NOPREFIX			0x00000800
!define DT_SINGLELINE		0x00000020
!define DT_WORDBREAK		0x00000010
	
; int DrawText(HDC hDC,
;			   LPCTSTR lpchText,
;			   int nCount,
;			   LPRECT lpRect,
;  			   UINT uFormat)
!define fnDrawText \
		'User32::DrawText(i, t, i, i, i) i'

; int ReleaseDC(HWND hWnd,
;				HDC hDC)
!define fnReleaseDC \
		'User32::ReleaseDC(i, i) i'

; GetWindowLong()/SetWindowLong() nIndex
!define GWL_WNDPROC         -4
!define GWL_HINSTANCE       -6
!define GWL_HWNDPARENT      -8
!ifndef GWL_STYLE ; defined in nsDialogs
!define GWL_STYLE           -16  
!endif
!ifndef GWL_EXSTYLE ; defined in nsDialogs
!define GWL_EXSTYLE           -16  
!endif
!define GWL_USERDATA        -21
!define GWL_ID              -12

; LONG GetWindowLong(HWND hWnd,
;  					 int nIndex)
!define fnGetWindowLong \		
		'User32::GetWindowLong(i, i) i'

; LONG SetWindowLong(HWND hWnd,
;  					 int nIndex
;					 LONG dwNewLong)
!define fnSetWindowLong \		
		'User32::SetWindowLong(i, i, i) i'

; int GetWindowText(HWND hWnd,
;					LPTSTR lpString,
;					int nMaxCount)	
!define fnGetWindowText \		
		'User32::GetWindowText(i, t, i) i'
		
; BOOL SetWindowText(HWND hWnd,
;					 LPCTSTR lpString)
!define fnSetWindowText \		
		'User32::SetWindowText(i, t) i'

;HGDIOBJ SelectObject(HDC hdc,
;					  HGDIOBJ hgdiobj)
!define fnSelectObject \
		'GDI32::SelectObject(i, i) i'

;BOOL GetTextMetrics(HDC hdc,
;					 LPTEXTMETRIC lptm)
!define fnGetTextMetrics \
		'GDI32::GetTextMetricsW(i, i) i'

; GetSystemMetrics()
!define SM_CXSCREEN             0
!define SM_CYSCREEN             1
!define SM_CXVSCROLL            2
!define SM_CYHSCROLL            3
!define SM_CYCAPTION            4
!define SM_CXBORDER             5
!define SM_CYBORDER             6
!define SM_CXDLGFRAME           7
!define SM_CYDLGFRAME           8
!define SM_CYVTHUMB             9
!define SM_CXHTHUMB             10
!define SM_CXICON               11
!define SM_CYICON               12
!define SM_CXCURSOR             13
!define SM_CYCURSOR             14
!define SM_CYMENU               15
!define SM_CXFULLSCREEN         16
!define SM_CYFULLSCREEN         17
!define SM_CYKANJIWINDOW        18
!define SM_MOUSEPRESENT         19
!define SM_CYVSCROLL            20
!define SM_CXHSCROLL            21
!define SM_DEBUG                22
!define SM_SWAPBUTTON           23
!define SM_RESERVED1            24
!define SM_RESERVED2            25
!define SM_RESERVED3            26
!define SM_RESERVED4            27
!define SM_CXMIN                28
!define SM_CYMIN                29
!define SM_CXSIZE               30
!define SM_CYSIZE               31
!define SM_CXFRAME              32
!define SM_CYFRAME              33
!define SM_CXMINTRACK           34
!define SM_CYMINTRACK           35
!define SM_CXDOUBLECLK          36
!define SM_CYDOUBLECLK          37
!define SM_CXICONSPACING        38
!define SM_CYICONSPACING        39
!define SM_MENUDROPALIGNMENT    40
!define SM_PENWINDOWS           41
!define SM_DBCSENABLED          42
!define SM_CMOUSEBUTTONS        43
!define SM_CXFIXEDFRAME         ${SM_CXDLGFRAME} 
!define SM_CYFIXEDFRAME         ${SM_CYDLGFRAME} 
!define SM_CXSIZEFRAME          ${SM_CXFRAME}    
!define SM_CYSIZEFRAME          ${SM_CYFRAME}    
!define SM_SECURE               44
!define SM_CXEDGE               45
!define SM_CYEDGE               46
!define SM_CXMINSPACING         47
!define SM_CYMINSPACING         48
!define SM_CXSMICON             49
!define SM_CYSMICON             50
!define SM_CYSMCAPTION          51
!define SM_CXSMSIZE             52
!define SM_CYSMSIZE             53
!define SM_CXMENUSIZE           54
!define SM_CYMENUSIZE           55
!define SM_ARRANGE              56
!define SM_CXMINIMIZED          57
!define SM_CYMINIMIZED          58
!define SM_CXMAXTRACK           59
!define SM_CYMAXTRACK           60
!define SM_CXMAXIMIZED          61
!define SM_CYMAXIMIZED          62
!define SM_NETWORK              63
!define SM_CLEANBOOT            67
!define SM_CXDRAG               68
!define SM_CYDRAG               69
!define SM_SHOWSOUNDS           70
!define SM_CXMENUCHECK          71 
!define SM_CYMENUCHECK          72
!define SM_SLOWMACHINE          73
!define SM_MIDEASTENABLED       74
!define SM_MOUSEWHEELPRESENT    75
!define SM_XVIRTUALSCREEN       76
!define SM_YVIRTUALSCREEN       77
!define SM_CXVIRTUALSCREEN      78
!define SM_CYVIRTUALSCREEN      79
!define SM_CMONITORS            80
!define SM_SAMEDISPLAYFORMAT    81
!define SM_IMMENABLED           82
!define SM_CXFOCUSBORDER        83
;!define SM_TABLETPC             86
;!define SM_MEDIACENTER          87
;!define SM_STARTER              88
;!define SM_SERVERR2             89
!define SM_MOUSEHORIZONTALWHEELPRESENT    91
!define SM_CXPADDEDBORDER       92
!define SM_DIGITIZER            94
!define SM_MAXIMUMTOUCHES       95
!define SM_REMOTESESSION        0x1000
!define SM_SHUTTINGDOWN         0x2000
!define SM_REMOTECONTROL        0x2001
!define SM_CARETBLINKINGENABLED 0x2002
!define SM_CONVERTIBLESLATEMODE 0x2003
!define SM_SYSTEMDOCKED         0x2004

;int GetSystemMetrics(int nIndex)
!define fnGetSystemMetrics \
		'User32::GetSystemMetrics(i) i'

; Image types
!ifndef IMAGE_BITMAP
!define IMAGE_BITMAP        0
!endif
!ifndef IMAGE_ICON
!define IMAGE_ICON          1
!endif
!ifndef IMAGE_CURSOR
!define IMAGE_CURSOR        2
!endif
!ifndef IMAGE_ENHMETAFILE
!define IMAGE_ENHMETAFILE   3
!endif

; Load Image Flags
!ifndef LR_DEFAULTCOLOR
!define LR_DEFAULTCOLOR     0x00000000
!define LR_MONOCHROME       0x00000001
!define LR_COLOR            0x00000002
!define LR_COPYRETURNORG    0x00000004
!define LR_COPYDELETEORG    0x00000008
!define LR_LOADFROMFILE     0x00000010
!define LR_LOADTRANSPARENT  0x00000020
!define LR_DEFAULTSIZE      0x00000040
!define LR_VGACOLOR         0x00000080
!define LR_LOADMAP3DCOLORS  0x00001000
!define LR_CREATEDIBSECTION 0x00002000
!define LR_COPYFROMRESOURCE 0x00004000
!define LR_SHARED           0x00008000
!endif ; defined (LR_DEFAULTCOLOR)

; HANDLE LoadImage(HINSTANCE hinst,
;				   LPCTSTR lpszName,
;				   UINT uType,
;				   int cxDesired,
;				   int cyDesired,
;				   UINT fuLoad)
!define fnLoadImage \
		'User32::LoadImage(i, t, i, i, i, i) i'

;BOOL DeleteObject(HGDIOBJ hObject);
!define fnDeleteObject \
		'GDI32::DeleteObject(i) i'

;int GetObject(HGDIOBJ hgdiobj,
;			   int cbBuffer,
;			   LPVOID lpvObject);
!define fnGetObject \
		'GDI32::GetObject(i, i, i) i'

;BOOL GetIconInfo(HICON hIcon,
;				  PICONINFO piconinfo)
!define fnGetIconInfo \
		'User32::GetIconInfo(i, i) i'

;BOOL DestroyIcon(HICON hIcon)
!define fnDestroyIcon \
		'User32::DestroyIcon(i) i'

;BOOL DestroyWindow(HWND hWnd)
!define fnDestroyWindow \
		'User32::DestroyWindow(i) i'

;HWND WINAPI CreateWindowEx(DWORD dwExStyle,
;						    LPCTSTR lpClassName,
;							LPCTSTR lpWindowName,
;							DWORD dwStyle,
;							int x,
;							int y,
;							int nWidth,
;							int nHeight,
;							HWND hWndParent,
;							HMENU hMenu,
;							HINSTANCE hInstance,
;							LPVOID lpParam)
!define fnCreateWindowEx \
		'User32::CreateWindowEx(i, t, t, i, i, i, i, i, i, i, i, i) i'

!endif ; defined(NULLSOFT_NX_SYSTEM_NSIS_HEADER)

