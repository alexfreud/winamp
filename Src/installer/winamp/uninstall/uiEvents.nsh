!ifndef NULLSOFT_WINAMP_UNINSTALLER_UI_EVENTS_HEADER
!define NULLSOFT_WINAMP_UNINSTALLER_UI_EVENTS_HEADER

!include ".\uninstall\ui.nsh"
!include ".\pages\groupCheckList.nsh"
!include ".\utils\wafuncs.nsh"
!include ".\utils\sectionDescription.nsh"
!include "windowText.nsh"

Function un.UI_OnComponentsPageCreate
	Push $0
	Push $1
	Push $2

	${NextButton_SetLastPageMode} $0

	StrCpy $0 ${IDX_UNINSTALL_COMPONENTS_GROUP}
	ClearErrors
	SectionGetText $0 $1
	${If} ${Errors}
		StrCpy $1 ""
	${EndIf}

	${GetSectionDescription} $0 $2
	!insertmacro MUI_HEADER_TEXT "$1"  "$2"

	${GroupCheckList_CreatePage} $0 "" "$(IDS_UNINSTALL_COMPONENTS_FOOTER)" "" "default" ""

	Pop $2
	Pop $1
	Pop $0
FunctionEnd

Function un.UI_OnFinishPageReadMe
	${If} ${IsWinXP}
		StrCpy $0 "XP"
	${ElseIf} ${IsWinVista}
		StrCpy $0 "Vista"
	${ElseIf} ${IsWin7}
		StrCpy $0 "7"
	${ElseIf} ${IsWin8}
		StrCpy $0 "8"
	${ElseIf} ${IsWin8.1}
		StrCpy $0 "8.1"
	${Else}
		StrCpy $0 "Unknown"
	${Endif}
	ExecShell "open" '"http://services.winamp.com/redirect/support?reason=uninstall&subject=Winamp Uninstall&product=Winamp Desktop&v=${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}&platform=Windows $0"'
FunctionEnd

Function un.UI_OnFinishPageRun
	ExecShell "open" '"$INSTDIR"'
FunctionEnd

Function un.UI_OnFinishPageShow

	!ifdef MUI_FINISHPAGE_RUN_VARIABLES
		${NX_OffsetWindowPos} $mui.FinishPage.Run 0 80
	!endif

	!ifdef MUI_FINISHPAGE_SHOREADME_VARAIBLES
		${NX_OffsetWindowPos} $mui.FinishPage.ShowReadme 0 70
	!endif

	${NX_IncreaseWindowSize} $mui.FinishPage.Text 0 70

	Push $0

	StrCpy $0 ""
	${If} $winamp.uninstall.checkFolder == "true"
		${DirState} "$INSTDIR" $0
		${If} $0 == 1
			StrCpy $0 "show_explorer"
		${Else}
			StrCpy $0 0
		${EndIf}
	${EndIf}

	${If} $0 == "show_explorer"
		${If} $mui.FinishPage.Text != 0
			${NX_SetWindowText} $mui.FinishPage.Text "$(IDS_UNINSTALL_SUBHEADER)$(IDS_UNINSTALL_FILES_NOT_REMOVED)"
		${EndIf}
	${Else}
		!ifdef MUI_FINISHPAGE_RUN_VARIABLES
			SendMessage $mui.FinishPage.Run ${BM_SETCHECK} 0 0
			ShowWindow $mui.FinishPage.Run ${SW_HIDE}
		!endif
	${EndIf}

	Pop $0

FunctionEnd

!endif ; NULLSOFT_WINAMP_UNINSTALLER_UI_EVENTS_HEADER