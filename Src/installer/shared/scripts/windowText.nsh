!ifndef NULLSOFT_NX_WINDOW_TEXT_NSIS_HEADER
!define NULLSOFT_NX_WINDOW_TEXT_NSIS_HEADER

!include "system.nsh"

!macro NX_SetWindowText __hwnd __text
	System::Call "${fnSetWindowText}(${__hwnd}, '${__text}')"
!macroend

!define NX_SetWindowText "!insertmacro 'NX_SetWindowText'"

!macro NX_GetWindowText __hwnd __textOut
	System::Call "${fnGetWindowText}(${__hwnd}, ${__textOut}, ${NSIS_MAX_STRLEN})"
!macroend

!define NX_GetWindowText "!insertmacro 'NX_GetWindowText'"

!macro NX_AppendWindowText __hwnd __text
	Push ${__hwnd}
	Push ${__text}
	Exch $R0 ; text
	Exch
	Exch $R1 ; hwnd

	Push $0
	StrCpy $0 ""
	${NX_GetWindowText} $R1 $0
	StrCpy $0 "$0$R0"
	${NX_SetWindowText} $R1 $0
	Pop $0
	Pop $R1
	Pop $R0
!macroend 

!define NX_AppendWindowText "!insertmacro 'NX_AppendWindowText'"

!endif ;NULLSOFT_NX_WINDOW_TEXT_NSIS_HEADER