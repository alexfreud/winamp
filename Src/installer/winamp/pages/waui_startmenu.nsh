!ifndef WAUI_STARTMENU_HEADER
!define WAUI_STARTMENU_HEADER

!include ".\wordFunc.nsh"
!include ".\utils\wafuncs.nsh"
!include ".\express_mode.nsh"

var waui.startmenu.text.start
var waui.startmenu.chk.start
var waui.startmenu.chk.quicklaunch
var waui.startmenu.chk.desktop

Function nsPageWAStartMenu_Create

!ifdef STARTMENUPAGE_CHECK_NEXT_BUTTON
	Push $0
	${ExpressMode_IsEnabled} $0
	${If} $0 == "yes"
		Pop $0
		Abort
	${EndIf}
	Pop $0
!endif

	SetPluginUnload alwaysoff
	!insertmacro MUI_HEADER_TEXT $(IDS_PAGE_STARTMENU_TITLE)  $(IDS_PAGE_STARTMENU_SUBTITLE)

	nsDialogs::Create 1018
	Pop $waui.dialog

	${NSD_CreateLabel} 0 0 100% 30u $(IDS_PAGE_STARTMENU_CAPTION)

	ReadIniStr $0 "$INSTINI" "Startmenu" "Name"
	${If} $0 == ""
		StrCpy $0 $(^NameDA)
		WriteIniStr $INSTINI "StartMenu" "Name" $0
	${EndIf}
  
	${NSD_CreateText} 0 30u 100% 12u $0
	Pop $waui.startmenu.text.start
	GetFunctionAddress $0 nsPageWAStartMenu__OnTextChange
	nsDialogs::OnChange $waui.startmenu.text.start $0

	${NSD_CreateCheckBox} 0 44u 100% 12u $(IDS_PAGE_STARTMENU_CHK_START)
	Pop $waui.startmenu.chk.start
	GetFunctionAddress $0 nsPageWAStartMenu__OnStartClick
	nsDialogs::OnClick $waui.startmenu.chk.start $0

	${NSD_CreateCheckBox} 0 56u 100% 12u $(IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH)
	Pop $waui.startmenu.chk.quicklaunch
	GetFunctionAddress $0 nsPageWAStartMenu__OnQuickLaunchClick
	nsDialogs::OnClick $waui.startmenu.chk.quicklaunch $0

	${NSD_CreateCheckBox} 0 68u 100% 12u $(IDS_PAGE_STARTMENU_CHK_DESKTOP)
	Pop $waui.startmenu.chk.desktop
	GetFunctionAddress $0 nsPageWAStartMenu__OnDesktopClick
	nsDialogs::OnClick $waui.startmenu.chk.desktop $0

	ReadIniStr $0 "$INSTINI" "Startmenu" "NoMenu"
	IntOp $0 $0 !
	SendMessage $waui.startmenu.chk.start ${BM_SETCHECK} $0 0

	ReadIniStr $0 "$INSTINI" "Startmenu" "NoQuickLaunch"
	IntOp $0 $0 !
	SendMessage $waui.startmenu.chk.quicklaunch ${BM_SETCHECK} $0 0

	ReadIniStr $0 "$INSTINI" "Startmenu" "NoDesktop"
	IntOp $0 $0 !
	SendMessage $waui.startmenu.chk.desktop ${BM_SETCHECK} $0 0

	Call nsPageWAStartMenu__OnStartClick

	nsDialogs::Show
	SetPluginUnload manual
FunctionEnd

Function nsPageWAStartMenu__OnTextChange
	System::Call "user32::GetWindowText(i$waui.startmenu.text.start, w.r0, i${NSIS_MAX_STRLEN})"
	WriteIniStr $INSTINI "StartMenu" "Name" $0
FunctionEnd

Function nsPageWAStartMenu__OnStartClick
	SendMessage $waui.startmenu.chk.start ${BM_GETCHECK} 0 0 $0
	EnableWindow $waui.startmenu.text.start $0
	${If} $0 = 0
		WriteIniStr $INSTINI "StartMenu" "NoMenu" "1"
	${Else}
		DeleteINIStr $INSTINI "StartMenu" "NoMenu"
	${EndIf}
FunctionEnd

Function nsPageWAStartMenu__OnQuickLaunchClick
	SendMessage $waui.startmenu.chk.quicklaunch ${BM_GETCHECK} 0 0 $0
	${If} $0 = 0
		WriteIniStr $INSTINI "StartMenu" "NoQuickLaunch" "1"
	${Else}
		DeleteINIStr $INSTINI "StartMenu" "NoQuickLaunch"
	${EndIf}
FunctionEnd

Function nsPageWAStartMenu__OnDesktopClick
	SendMessage $waui.startmenu.chk.desktop ${BM_GETCHECK} 0 0 $0
	${If} $0 = 0
		WriteIniStr $INSTINI "StartMenu" "NoDesktop" "1"
	${Else}
		DeleteINIStr $INSTINI "StartMenu" "NoDesktop"
	${EndIf}
FunctionEnd

Function StartMenu_WriteData
	Push $0
	Push $1
	Push $2

	StrCpy $2	"$OUTDIR"
	SetOutPath "$INSTDIR" 
	SetShellVarContext all

	ReadIniStr $1 "$INSTINI" "Startmenu" "Name"
	${If} $1 != ""
		ReadIniStr $0 "$INSTINI" "Startmenu" "NoMenu"
		${If} $0 == ""
			CreateDirectory "$SMPROGRAMS\$1"
			CreateShortcut "$SMPROGRAMS\$1\${WINAMPLINK}" "$INSTDIR\${WINAMPEXE}"
			CreateShortcut "$SMPROGRAMS\$1\$(safeMode).lnk" "$INSTDIR\${WINAMPEXE}" "/SAFE=1"
			CreateShortcut "$SMPROGRAMS\$1\$(uninstallWinamp).lnk" "$INSTDIR\uninstwa.exe"
		${EndIf}
	${EndIf}
  
	ReadIniStr $0 "$INSTINI" "Startmenu" "NoQuickLaunch"
	${If} $0 == ""
		CreateShortcut "$QUICKLAUNCH\${WINAMPLINK}" "$INSTDIR\${WINAMPEXE}"
	${EndIf}
	ReadIniStr $0 "$INSTINI" "Startmenu" "NoDesktop"
	${If} $0 == ""
		CreateShortcut "$DESKTOP\${WINAMPLINK}" "$INSTDIR\${WINAMPEXE}"
	${EndIf}

	SetShellVarContext current
	SetOutPath "$2" 

	Pop $2
	Pop $1
	Pop $0
FunctionEnd

Function un.StartMenu_CleanData
	Push $0
	Push $1
	Push $2

	SetShellVarContext all

	StrCpy $0 $INSTINI
	${If} ${FileExists} "$0"
		ReadIniStr $1 "$0" "Startmenu" "Name"
		${If} $1 != ""
			ReadIniStr $2 "$0" "Startmenu" "NoMenu"
			${If} $2 != "1"
				Delete "$SMPROGRAMS\$1\*.lnk"
				RMDir /r "$SMPROGRAMS\$1"
			${EndIf}
		${EndIf}
		ReadIniStr $2 "$0" "Startmenu" "NoQuickLaunch"
		${If} $2 != "1"
			Delete "$QUICKLAUNCH\${WINAMPLINK}"
		${EndIf}
		ReadIniStr $2 "$0" "Startmenu" "NoDesktop"
		${If} $2 != "1"
			Delete "$DESKTOP\${WINAMPLINK}"
		${EndIf}
	${EndIf}

	Pop $2
	Pop $1
	Pop $0
FunctionEnd
!endif ;WAUI_STARTMENU_HEADER