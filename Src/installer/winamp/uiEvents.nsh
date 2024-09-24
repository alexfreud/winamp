!ifndef WINAMP_PAGES_INSTALL_HEADER
!define WINAMP_PAGES_INSTALL_HEADER

!include ".\ui.nsh"
!include "nx.nsh"

!ifdef EXPRESS_MODE
!define WINAMP_LICENSE_AGREEMENT	${LICENSE_PATH}
!define WINAMP_PRIVACY_POLICY		${PRIVACY_POLICY_PATH}
!endif ; defined(EXPRESS_MODE)

Var waui.button.advanced

!ifdef EXPRESS_MODE
Var waui.WelcomePage.PrivacyNote
!endif ; defined(EXPRESS_MODE)

Var waui.WelcomePage.IsLicensePresented

Function UI_OnInit
	Push $R0
	StrCpy $R0 0
	StrCpy $IDX_INSTTYPE_FULL $R0
	!ifdef FULL
		InstTypeSetText $R0 $(installFull)
		IntOp $R0 $R0 + 1
	!endif ; FULL

	StrCpy $IDX_INSTTYPE_STANDARD $R0
	!ifdef FULL
		InstTypeSetText $R0 $(installStandard)
		IntOp $R0 $R0 + 1
	!endif ; FULL
 
	InstTypeSetText $R0 $(installLite)
	IntOp $R0 $R0 + 1
	InstTypeSetText $R0 $(installMinimal)

	StrCpy $IDX_INSTTYPE_PREVIOUS $R0
	StrCpy $waui.WelcomePage.IsLicensePresented "no"

	${ExpressMode_InitializePage}
FunctionEnd

Function .onGUIEnd
FunctionEnd

!ifdef EXPRESS_MODE
Function onXPClickLink
	Pop $0 ; don't forget to pop HWND of the stack
	ExecShell open "http://www.winamp.com/legal/eula/pc"
FunctionEnd
!endif

Function UI_OnWelcomePageShow
!ifdef EXPRESS_MODE
	${If} ${AtLeastWinVista}
		${NX_GetMappedWindowRect} $mui.WelcomePage.Image $mui.WelcomePage $2 $3 $4 $5
		; Adjust Image Control Size
		IntOp $4 $4 - $2 
		IntOp $5 $R1 - $3
		${NX_SetWindowSize} $mui.WelcomePage.Image $4 $5

		; Clear previous image
		${NSD_ClearImage} $mui.WelcomePage.Image
		${NSD_FreeImage} $mui.WelcomePage.Image.Bitmap
		StrCpy $mui.WelcomePage.Image.Bitmap 0

		; Reload Banner Image
		${If} $PLUGINSDIR == ""
		${OrIfNot} ${FileExists} "$PLUGINSDIR"
			InitPluginsDir
		${EndIf}

		StrCpy $0 "$PLUGINSDIR\welcomeImage.bmp"
		${IfNot} ${FileExists} "$0"
			ReserveFile "${WELCOMEFINISH_IMAGE_PATH}"
			File /oname=$0 "${WELCOMEFINISH_IMAGE_PATH}"
		${EndIf}

		System::Call 'user32::LoadImageW(i 0, w "$0", i ${IMAGE_BITMAP}, i $4, i $5, i ${LR_LOADFROMFILE}) i.s'
		Pop $mui.WelcomePage.Image.Bitmap

		SendMessage $mui.WelcomePage.Image ${STM_SETIMAGE} \
					${IMAGE_BITMAP} \
					$mui.WelcomePage.Image.Bitmap

		; Get Dialog Height
		${NX_GetWindowSize} $mui.WelcomePage $2 $R2

		; Create License and Privacy Note
		IntOp $0 $R1 + 0
		IntOp $1 $R2 - $0
		IntOp $1 $1 - 4
		StrCpy $2 "$(IDS_PAGE_WELCOME_LEGAL)"

		; TODO really don't want to have to do this as it's pretty lame as reason is
		;      on XP, using a SysLink will crash when it's destroyed i.e. page change
		${If} ${IsWinXP}
			IntOp $R2 $R2 - 16
			${NSD_CreateLink} 8u $R2 316u 16 "By clicking “Next”, you agree to the Winamp License Agreement."
			Pop $waui.WelcomePage.PrivacyNote
			${NSD_OnClick} $waui.WelcomePage.PrivacyNote onXPClickLink
			SetCtlColors $waui.WelcomePage.PrivacyNote 0xFF0000 transparent 
		${Else}
			nsDialogs::CreateControl "SysLink" \
						${WS_CHILD}|${WS_TABSTOP}|${WS_VISIBLE}\
						|${WS_CLIPSIBLINGS}\
						|${LWS_TRANSPARENT}|${LWS_NOPREFIX} \
						${WS_EX_NOPARENTNOTIFY} \
						8u $0 316u $1 $2
			Pop $waui.WelcomePage.PrivacyNote
		${EndIf}

		${If} $waui.WelcomePage.PrivacyNote != 0
			${NSD_OnNotify} $waui.WelcomePage.PrivacyNote UI_OnWelcomePageNotify

			SendMessage $HWNDPARENT ${WM_GETFONT} 0 0 $0
			SendMessage $waui.WelcomePage.PrivacyNote ${WM_SETFONT} $0 0 

			${IfNot} ${IsWinXP}
				; Adjust Link Control Size & Position
				${NX_GetMappedWindowRect} $waui.WelcomePage.PrivacyNote $mui.WelcomePage $2 $3 $4 $5
				IntOp $4 $4 - $2 ; Link Control Max Width
				IntOp $5 $5 - $3 ; Link Control Max Height

				System::Call '*${stSIZE}($4, $5) .r1'
				SendMessage $waui.WelcomePage.PrivacyNote ${LM_GETIDEALSIZE} $4 $1 $0
				${If} $0 <= $5
					System::Call "*$1${stSIZE} (.r6, .r7)"

					IntOp $8 $4 - $6
					IntOp $2 $2 + $8

					IntOp $8 $5 - $0
					IntOp $3 $3 + $8

					${NX_SetWindowPosAndSize} $waui.WelcomePage.PrivacyNote $2 $3 $6 $0
				${EndIf}
				System::Free $1
			${EndIf}
			StrCpy $waui.WelcomePage.IsLicensePresented "yes"
		${EndIf}
	${EndIf}
!endif	; EXPRESS_MODE
FunctionEnd

!ifdef EXPRESS_MODE
Function UI_OnWelcomePageNotify
	Exch $R0
	Exch
	Exch $R1
	Exch 2
	Exch $R2

	${If} $R1 = ${NM_CLICK}
	${OrIf} $R1 = ${NM_RETURN}
		Push $R3
		Push $R4
		Push $0
		System::Call `*$R2(i, i, i, i, i, i, i, &w48 .R3)`

		${If} $R3 == "winamp_eula"
			StrCpy $R4 "http://www.winamp.com/legal/eula/pc"
		${ElseIf} $R3 == "winamp_privacy_policy"
			StrCpy $R4 "http://www.winamp.com/legal/privacy"
		${Else}
			StrCpy $R4 ""
		${EndIf}

		${If} $R4 != ""
			Call ConnectInternet
			Pop $0
			${If} $0 == "online"
				Call IsInternetAvailable
				Pop $0
			${EndIf}
		${Else}
			StrCpy $0 "offline"
		${EndIf}

		${If} $0 != "online"
			${If} $PLUGINSDIR == ""
			${OrIfNot} ${FileExists} "$PLUGINSDIR"
				InitPluginsDir
			${EndIf}

			${If} $R3 == "winamp_eula"
				StrCpy $R4 "${WINAMP_LICENSE_AGREEMENT}"
			${ElseIf} $R3 == "winamp_privacy_policy"
				StrCpy $R4 "${WINAMP_PRIVACY_POLICY}"
			${Else}
				StrCpy $R4 ""
			${EndIf}

			${If} $R4 != ""
				${GetFileName} $R4 $R4
				StrCpy $R4 "$PLUGINSDIR\$R4"
				${IfNot} ${FileExists} "$R4"
					${If} $R3 == "winamp_eula"
						ReserveFile "${WINAMP_LICENSE_AGREEMENT}"
						File /oname=$R4 "${WINAMP_LICENSE_AGREEMENT}"
					${ElseIf} $R3 == "winamp_privacy_policy"
						ReserveFile "${WINAMP_PRIVACY_POLICY}"
						File /oname=$R4 "${WINAMP_PRIVACY_POLICY}"
					${EndIf}
				${EndIf}
				ExecShell open $R4
			${EndIf}
		${Else}
			ExecShell open $R4
		${EndIf}
		Pop $0
		Pop $R4
		Pop $R3
	${EndIf}
  	Pop $R2
	Pop $R0
	Pop $R1
FunctionEnd
!endif ;defined(EXPRESS_MODE)

Function UI_OnDirectoryPagePre
	Push $0
	${ExpressMode_IsEnabled} $0
	${If} $0 == "yes"
		Pop $0
		Call UI_OnDirectoryPageLeave
		Abort
	${EndIf}
	Pop $0
FunctionEnd

Function UI_OnDirectoryPageShow
	FindWindow $1 "#32770" "" $HWNDPARENT
	GetDlgItem $0 $1 0x3FF
	ShowWindow $0 ${SW_HIDE}

	GetDlgItem $0 $HWNDPARENT 0x02
	${NX_GetMappedWindowRect} $0 $HWNDPARENT $2 $3 $4 $5

	IntOp $4 $4 - $2 
	IntOp $5 $5 - $3

	System::Call `user32::CreateWindowExW(i 0, w "BUTTON", w "Advanced...", i ${WS_TABSTOP}|${WS_CHILD}, i 12, i r3, i r4 , i r5, i $HWNDPARENT, i 0, i 0, i 0) i.s`
	Pop $waui.button.advanced

	SendMessage $HWNDPARENT ${WM_GETFONT} 0 0 $0
	SendMessage $waui.button.advanced ${WM_SETFONT} $0 0 
FunctionEnd

Function UI_OnDirectoryPageLeave
	${NX_Log} "Directory page leave!!!"
	${If} $waui.button.advanced <> 0
		System::Call "${fnDestroyWindow}($waui.button.advanced)"
		StrCpy $waui.button.advanced 0
	${EndIf}
	Call SetupWinampDirectories
FunctionEnd

Function UI_OnLicensePagePre
	${If} $waui.WelcomePage.IsLicensePresented == "yes"
		Call UI_OnLicensePageLeave
		Abort
	${EndIf}
FunctionEnd

Function UI_OnLicensePageShow
	${If} $waui.button.advanced <> 0
		System::Call "${fnDestroyWindow}($waui.button.advanced)"
		StrCpy $waui.button.advanced 0
	${EndIf}
FunctionEnd

Function UI_OnLicensePageLeave
FunctionEnd

Function UI_OnComponentsPagePre
	Push $0
	${ExpressMode_IsEnabled} $0
	${If} $0 == "yes"
		Pop $0
		Abort
	${EndIf}
	Pop $0
FunctionEnd

Function UI_OnComponentsPageShow
	Push $0
	Push $1
	Push $2
	Push $3
	Push $R1

	${If} $mui.ComponentsPage.DescriptionText <> 0
	${AndIf} $mui.ComponentsPage.DescriptionTitle <> 0
		${NX_GetMappedWindowRect} $mui.ComponentsPage.DescriptionTitle \
								  $mui.ComponentsPage \
								  $0 $1 $2 $3

	${NX_ConvertHorzDLU} $mui.ComponentsPage 5 $R1
	IntOp $0 $0 + $R1

	${NX_ConvertHorzDLU} $mui.ComponentsPage 1 $R1
	IntOp $3 $3 - $R1

	${NX_ConvertVertDLU} $mui.ComponentsPage 10 $R1
	IntOp $1 $1 + $R1

	${If} $R1 > 10
		IntOp $1 $1 - 1  ; rollback 1px
	${EndIf}

	${NX_ConvertVertDLU} $mui.ComponentsPage 1 $R1
	IntOp $2 $2 - $R1
	IntOp $2 $2 - $0
	IntOp $3 $3 - $1

	${NX_SetWindowPosAndSize} $mui.ComponentsPage.DescriptionText \
							  $0 $1 \
							  $2 $3
	${EndIf}

	Pop $R1
	Pop $3
	Pop $2
	Pop $1
	Pop $0
FunctionEnd

Function UI_OnMouseOverSection
	Push $0
	Call GetSectionDescription
	Exch $0
	EnableWindow $mui.ComponentsPage.DescriptionText 1
    SendMessage $mui.ComponentsPage.DescriptionText ${WM_SETTEXT} 0 "STR:$0"
	Pop $0
FunctionEnd

Function UI_OnFinsihPagePre
	Push $0

	!ifndef _DEBUG

	${ExpressMode_IsEnabled} $0
	${If} $0 == "yes"
		Pop $0
		!ifndef MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
			Call UI_OnFinishPageReadMe
		!endif
		Abort
	${EndIf}

	!endif ; defined(_DEBUG)
	Pop $0
FunctionEnd

Function UI_OnFinishPageReadMe
	SetPluginUnload alwaysoff
	; Find Window info for the window we're displaying
	System::Call "*${stRECT} .r1"
	System::Call 'User32::GetWindowRect(i, i) i ($HWNDPARENT, r1) .r2'
	; Get left/top/right/bottom
	System::Call "*$1${stRECT} (.r2, .r3, .r4, .r5)"
	System::Free $1
	WriteINIStr "$WINAMPINI" "SETUP" "left" $2
	WriteINIStr "$WINAMPINI" "SETUP" "top" $3
	WriteINIStr "$WINAMPINI" "SETUP" "right" $4
	WriteINIStr "$WINAMPINI" "SETUP" "bottom" $5

	;SetPluginUnload manual
	HideWindow
  
	Push $1
	StrCpy $1 1
	File "/oname=$PLUGINSDIR\ShellDispatch.dll" "${NSISDIR}\Plugins\x86-unicode\ShellDispatch.dll"
	${If} ${FileExists} "$PLUGINSDIR\ShellDispatch.dll"
	${AndIf} ${FileExists} "$INSTDIR\${WINAMPEXE}"
		Push $0
		StrCpy $0 ""
		ClearErrors
		GetFullPathName /SHORT $0 "$PLUGINSDIR\ShellDispatch.dll"
		${IfNot} ${Errors}
		${AndIf} $0 != ""
			ExecWait 'rundll32.exe $0,RunDll_ShellExecute "open" "$INSTDIR\${WINAMPEXE}" "/NEW /REG=S" "$INSTDIR" 1' $1
			${If} ${Errors}
				StrCpy $1 1
			${EndIf}
		${EndIf}
		Pop $0
	${EndIf}

	${If} $1 != 0
		Exec "$INSTDIR\${WINAMPEXE} /NEW /REG=S"
	${EndIf}
	Pop $1

	Sleep 500
FunctionEnd
!endif