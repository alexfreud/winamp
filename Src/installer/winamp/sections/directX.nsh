!include "logicLib.nsh"

!define DIRECTXINSTALL_URL_FULL		"http://download.nullsoft.com/redist/dx/dxwebsetup.exe"	
!define DIRECTXINSTALL_URL_EMBED	"http://download.nullsoft.com/redist/dx/d3dx9_31_42_x86_embed.exe"

!define DIRECTX_MSDOWNLOAD_URL		"https://www.microsoft.com/download/details.aspx?id=35"
;!define DIRECTX_MSDOWNLOAD_URL		"https://support.microsoft.com/kb/179113"

!define DIRECTXINSTAL_DIRECTXNAME		"Microsoft DirectX®"
!define DIRECTXINSTAL_WINDOWSNAME		"Microsoft Windows®"
!define DIRECTXINSTAL_WINAMPNAME		"Winamp"
!define DIRECTXINSTAL_WINVER_LO			"${DIRECTXINSTAL_WINDOWSNAME} Vista"
!define DIRECTXINSTAL_WINVER_HI			"${DIRECTXINSTAL_WINDOWSNAME} 7"
!define DIRECTXINSTALL_DIRECTXMINVER	"9.0"

!macro DirextXInstall_CheckLibrary __libraryName __outputVar
	StrCpy ${__outputVar} ""
	
	Push $0
	StrCpy $0 ${__libraryName}
	DetailPrint "$(IDS_DIRECTX_CHECKING_D3DX_COMPONENT)..."
	Pop $0
	
	${If} ${FileExists} "$SYSDIR\${__libraryName}"
		DetailPrint "  $(IDS_DIRECTX_FOUND)."
	${Else}
		DetailPrint "  $(IDS_DIRECTX_MISSING)."
		StrCpy ${__outputVar} "${DIRECTXINSTALL_URL_EMBED}"
	${EndIf}
!macroend

!define DirextXInstall_CheckLibrary "!insertmacro 'DirextXInstall_CheckLibrary'"

!macro DirectXInstall_GetRequiredDownloadUrl __outputVar
	StrCpy ${__outputVar} ""

	${If} ${AtMostWin2003}
	${OrIf} ${AtMostWinVista}
		DetailPrint "$(IDS_DIRECTX_DETECTED_WINVER_OR_LOWER)"

		DetailPrint "$(IDS_DIRECTX_CHECKING_DIRECTX_VER)..."
		DetailPrint "  $(IDS_DIRECTX_REQUIRED_DIRECTX_MINVER): ${DIRECTXINSTALL_DIRECTXMINVER}"
		; check directX version

		Push $0
		ClearErrors
		ReadRegStr $0 HKLM "Software\Microsoft\DirectX" "Version"
		${If} $0 == ""
			DetailPrint "  $(IDS_DIRECTX_UNABLE_DETECT_DIRECTX)"
			StrCpy $0 "0"
		${Else}
			Push $1
			StrCpy $1 $0 2 5    ; get the minor version
			StrCpy $0 $0 2 2    ; get the major version
			IntOp $0 $0 * 100   ; $0 = major * 100 + minor
			IntOp $0 $0 + $1
			Pop $1
			DetailPrint "  $(IDS_DIRECTX_DETECTED_DIRECTX_VER): $0"
		${EndIf}

		${If} $0 < 900
			Pop $0
			DetailPrint "  $(IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER)."
			StrCpy ${__outputVar} "${DIRECTXINSTALL_URL_FULL}"
		${Else}
			Pop $0
		${EndIf}
	${Else}
		DetailPrint "$(IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER)"
		${DirextXInstall_CheckLibrary} "d3dx9_42.dll" ${__outputVar}
	${EndIF}

	${If} ${__outputVar} == ""
		${DirextXInstall_CheckLibrary} "d3dx9_31.dll" ${__outputVar}
	${EndIf}
!macroend 

!define DirectXInstall_GetRequiredDownloadUrl "!insertmacro 'DirectXInstall_GetRequiredDownloadUrl'"

!macro DirectXInstall_CheckConnection	__outputVar
	StrCpy ${__outputVar} ""
	ClearErrors
	Dialer::AttemptConnect
	${IfNot} ${Errors}
		nsExec::Exec "ping -n 1 -w 400 www.google.com"
		Pop ${__outputVar}
		${If} ${__outputVar} <> 0
			nsExec::Exec "ping -n 1 -w 400 www.yahoo.com"
			Pop ${__outputVar} 
		${EndIf}

		${If} ${__outputVar} == 0
			StrCpy ${__outputVar} "success"
		${EndIf}
	${EndIf}
!macroend

!define DirectXInstall_CheckConnection "!insertmacro 'DirectXInstall_CheckConnection'"

!macro DirectXInstall_ShowErrorMessage __installerUrl __messageFull __messageEmbed
	Push $2
	${If} ${__installerUrl} == "${DIRECTXINSTALL_URL_FULL}"
		StrCpy $2 "${__messageFull}."
	${Else}
		StrCpy $2 "${__messageEmbed}."
	${EndIf}
	StrCpy $2 "$2$\r$\n$\r$\n$(IDS_DIRECTX_LINK_TO_MSDOWNLOAD)$\r$\n${DIRECTX_MSDOWNLOAD_URL}"
	MessageBox MB_OK|MB_ICONEXCLAMATION $2 /SD IDOK
	Pop $2
!macroend

!define DirectXInstall_ShowErrorMessage "!insertmacro 'DirectXInstall_ShowErrorMessage'"

!macro DIRECTXINSTALL_INSERT_SECTION
!ifdef FULL
	Section -DirectXInstall IDX_DIRECTX_INSTALL

		; check if at least one dependent section selected
		${IfNot} ${SectionIsSelected} ${IDX_GRP_MMEDIA_VIDEO_DEC}
		${AndIfNot} ${SectionIsPartiallySelected} ${IDX_GRP_MMEDIA_VIDEO_DEC}
		${AndIfNot} ${SectionIsSelected} ${IDX_SEC_MILKDROP2}
			Goto DirectXInstall_SectionEnd
		${EndIf}

		; get url to download or empty string if download not required
		${DirectXInstall_GetRequiredDownloadUrl} $0
		${If} $0 == ""
			Goto DirectXInstall_SectionEnd
		${EndIf}

		; check internet connection
		DetailPrint "$(IDS_DIRECTX_DOWNLOAD_REQUIRED)"
		DetailPrint "$(IDS_DIRECTX_CHECKING_INTERNET)..."
		${DirectXInstall_CheckConnection} $1
		${If} $1 == "success"
			DetailPrint "  $(IDS_DIRECTX_SUCCESS)."
		${Else}
			DetailPrint "  $(IDS_DIRECTX_FAILED)."
			${DirectXInstall_ShowErrorMessage} "$0" "$(IDS_DIRECTX_FULL_CONNECT_FAILED)""$(IDS_DIRECTX_EMBED_CONNECT_FAILED)"
			Goto DirectXInstall_SectionEnd
		${EndIf}

		; download installer

		DetailPrint "$(IDS_DIRECTX_DOWNLOADING_SETUP)..."
		NSISdl::download /TRANSLATE2 "$(IDS_DIRECTX_DOWNLOADING_SETUP)..." "$(IDS_CONNECTING)" "$(IDS_SECOND)" "$(IDS_MINUTE)" "$(IDS_HOUR)" "$(IDS_SECONDS)" "$(IDS_MINUTES)" "$(IDS_HOURS)" "$(IDS_PROGRESS)" /TIMEOUT=30000 "$0" "$PLUGINSDIR\wadxsetup.exe"
		Pop $1  
		${If} $1 != "success"
			${If} $1 == "cancel"
				DetailPrint "  $(IDS_DIRECTX_ABORTED)."
			${Else}
				DetailPrint "  $(IDS_DIRECTX_FAILED)."
			${EndIf}
			${DirectXInstall_ShowErrorMessage} "$0" "$(IDS_DIRECTX_FULL_DOWNLOAD_FAILED)""$(IDS_DIRECTX_EMBED_DOWNLOAD_FAILED)"
			Goto DirectXInstall_SectionEnd
		${Else}
			DetailPrint "  $(IDS_DIRECTX_DONE)."
		${EndIf}

		; run setup
		DetailPrint "$(IDS_DIRECTX_RUNNING_SETUP)..."
		ClearErrors
		${If} $0 == "${DIRECTXINSTALL_URL_FULL}"
			StrCpy $2 "$(IDS_DIRECTX_FULL_INSTALL_APPROVAL)"
			${IfNot} ${Cmd} 'MessageBox MB_YESNO|MB_ICONEXCLAMATION $2 /SD IDYES IDYES'
				DetailPrint "  $(IDS_DIRECTX_ABORTED)."
				Goto DirectXInstall_SectionEnd
			${EndIf}
			Exec '$PLUGINSDIR\wadxsetup.exe'
			${If} ${Errors}
				StrCpy $1 "-9"
			${EndIf}
		${Else}
			ExecWait "$PLUGINSDIR\wadxsetup.exe" $1
			${If} $1 != 0
				SetErrors
			${EndIf}
		${EndIf}

		${If} ${Errors}
			DetailPrint "  $(IDS_DIRECTX_FAILED)."
			${DirectXInstall_ShowErrorMessage} "$0" "$(IDS_DIRECTX_FULL_INSTALL_FAILED)""$(IDS_DIRECTX_EMBED_INSTALL_FAILED)"
		${EndIf}

	DirectXInstall_SectionEnd:
	SectionEnd
!endif
!macroend

!define DIRECTXINSTALL_INSERT_SECTION "!insertmacro 'DIRECTXINSTALL_INSERT_SECTION'"