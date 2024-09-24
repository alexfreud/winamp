!ifndef NULLSOFT_WINAMP_INSTALLER_EXPRESS_MODE_HEADER
!define NULLSOFT_WINAMP_INSTALLER_EXPRESS_MODE_HEADER

!include "logicLib.nsh"
!include "nx.nsh"

!ifdef EXPRESS_MODE

Var expressMode.isEnabled

; ExpressMode_Initialize
!macro ExpressMode_Initialize
	StrCpy $expressMode.IsEnabled "yes"
!macroend

!define ExpressMode_Initialize "!insertmacro 'ExpressMode_Initialize'"

; ExpressMode_IsEnabled
!macro ExpressMode_IsEnabled __isEnabled
	StrCpy "${__isEnabled}" $expressMode.isEnabled
!macroend

!define ExpressMode_IsEnabled "!insertmacro 'ExpressMode_IsEnabled'"

; ExpressMode_Enable
!macro ExpressMode_Enable
	StrCpy $expressMode.isEnabled "yes"
!macroend

!define ExpressMode_Enable "!insertmacro 'ExpressMode_Enable'"

; ExpressMode_Disable
!macro ExpressMode_Disable
	StrCpy $expressMode.isEnabled "no"
!macroend

!define ExpressMode_Disable "!insertmacro 'ExpressMode_Disable'"

!else ; defined(EXPRESS_MODE)

!define ExpressMode_Initialize ""

!macro ExpressMode_IsEnabled __isEnabled
	StrCpy ${__isEnabled} "no"
!macroend

!define ExpressMode_IsEnabled "!insertmacro 'ExpressMode_IsEnabled'"

!define ExpressMode_Enable ""
!define ExpressMode_Disable ""

!endif ; defined(EXPRESS_MODE)

!include "pages\express_mode_page.nsh"
!endif ; defined(NULLSOFT_WINAMP_INSTALLER_EXPRESS_MODE_HEADER)