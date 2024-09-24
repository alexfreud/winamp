!include "FileFunc.nsh"
!include ".\sectionsHelper.nsh"

Section -SetWinampFolders
	${If} ${Silent} 
		Call SetupWinampDirectories
	${EndIf}
SectionEnd

Section -KillWA
	${If} ${FileExists} "$INSTDIR\${WINAMPEXE}"
		StrCpy $FIRSTINSTALL  "0"
	${Else}
		StrCpy $FIRSTINSTALL  "1"
	${EndIf}

	${If} ${FileExists} "$WINAMPM3U"
		StrCpy $needplaystart "0"
	${Else}
		StrCpy $needplaystart "1"
	${EndIf}

	DetailPrint $(IDS_RUN_CHECK_PROCESS)
	SetDetailsPrint none
	${Do}
		ClearErrors
		Delete "$INSTDIR\${WINAMPEXE}"
		${If} ${Errors} 
			SetErrors
			IfSilent done
			MessageBox MB_DEFBUTTON2|MB_ABORTRETRYIGNORE $(msgCloseWinamp) IDABORT done IDRETRY +2
			ClearErrors
		${EndIf}
	${LoopWhile} ${Errors}

	StrCpy $1 0
	${Do}
		ClearErrors
		Delete "$INSTDIR\winampa.exe"
		${If} ${Errors}
			SetErrors
			FindWindow $0 "WinampAgentMain"
			${If} $0 = 0  ; something bad or other session
				IfSilent done
				MessageBox MB_DEFBUTTON2|MB_ABORTRETRYIGNORE $(IDS_MSG_AGENTONOTHERSESSION) IDABORT done IDRETRY +2
				ClearErrors
			${Else}
				IntOp $1 $1 + 1
				IntCmp $1 40 done
				SendMessage $0 ${WM_CLOSE} 0 0 /TIMEOUT=2000
				Sleep 250
				IsWindow $0 0 agentwnd_killed
				SendMessage $0 ${WM_DESTROY} 0 0 /TIMEOUT=2000
				Sleep 250
				agentwnd_killed:
			${EndIf}
		${EndIf}
	${LoopWhile} ${Errors}

done:  
	SetDetailsPrint lastused
	${If} ${Errors}
		Abort $(msgInstallAborted)
	${EndIf}

	ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "WinampAgent"
	${If} $0 == "$INSTDIR\winampa.exe"
		StrCpy $RESTARTAGENT "1"
		DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "WinampAgent"
	${Else}
		StrCpy $RESTARTAGENT "0"
	${EndIf}
SectionEnd

Section -HiddenPathINI
	CreateDirectory $INSTDIR
	${If} $SETTINGSDIR != $INSTDIR
		CreateDirectory $SETTINGSDIR
		Call GetWinampFolder
		Pop $0
		SetShellVarContext current
		StrCpy $1 "$APPDATA\$0"
		${If} $SETTINGSDIR == $1
			WriteINIStr "$INSTDIR\paths.ini" "Winamp" "inidir"  "{26}\$0"
		${Else}
			WriteINIStr "$INSTDIR\paths.ini" "Winamp" "inidir"  "$SETTINGSDIR"
		${EndIf}
	${EndIf}
SectionEnd

!include ".\sections\winamp.nsh"

!ifndef WINAMP64
Section -HiddenFeedback
	SetOutPath "$INSTDIR\Plugins"
	Delete "$INSTDIR\Plugins\gen_talkback.dll"
	File /nonfatal "${FILES_PATH}\Plugins\gen_crasher.dll"
	Delete "$INSTDIR\Plugins\reporter.exe"
	Delete "$INSTDIR\Plugins\tbsetup.exe"

	SetOutPath "$INSTDIR"
	File /nonfatal "${FILES_PATH}\reporter.exe"
SectionEnd
!endif ; WINAMP64

!ifndef WINAMP64
${WinampSection} "winampAgent" $(compAgent) IDX_SEC_AGENT
	${SECTIONIN_LITE}
	SetOutPath $INSTDIR
	File "${FILES_PATH}\winampa.exe"
	${If} $RESTARTAGENT == "1"
		WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "WinampAgent" "$INSTDIR\winampa.exe"
	${EndIf}
	Var /GLOBAL WINAMP_SECTION_NAME_4
	StrCpy $WINAMP_SECTION_NAME_4 "winampAgent"
${WinampSectionEnd}

!endif ; WINAMP64
!include ".\sections\frenchRadio.nsh"
!include ".\sections\grp_mmedia.nsh"
!include ".\sections\grp_walib.nsh"
!include ".\sections\grp_uiext.nsh"
!include ".\sections\grp_vis.nsh"
!include ".\sections\extra.nsh"
!include ".\sections\wmfdist.nsh"
!include ".\sections\directX.nsh"

${DIRECTXINSTALL_INSERT_SECTION}

!insertmacro GetTime

Section "-end" IDX_SECTION_END
	SetDetailsPrint textonly
	${If} ${FileExists} "$INSTDIR\Plugins\gen_peercast.dll"
		GetDLLVersion "$INSTDIR\Plugins\gen_peercast.dll" $0 $1
		${If} $0 == ""
			MessageBox MB_YESNO "Peercast plug-in detected. The old version of this plug-in$\r$\nmay cause problems. Hit Yes to remove this plug-in (recommended)." /SD IDYES IDNO +2
			Delete "$INSTDIR\Plugins\gen_peercast.dll"
		${EndIf}
	${EndIf}

	Delete "$INSTDIR\Plugins\gen_nomad.dll" ; ha ha fuck off creative
	Delete "$INSTDIR\Plugins\in_mp3pro.dll" ; mp3 pro plugin is BUGGGGGY
	Delete "$INSTDIR\Plugins\gen_diebrowserdie.dll" ; DrO's deprecated StationInfo window killer
	Delete "$INSTDIR\Plugins\ml_lite_n.dll" ; deprecated essential pack plugin
	Delete "$INSTDIR\Plugins\gen_nopro.dll" ; deprecated essential pack plugin
	Delete "$INSTDIR\Plugins\gen_killpredixis.dll" ; deprecated essential pack plugin
	Delete "$INSTDIR\Plugins\gen_b4s2m3u.dll" ; DrO's deprecated PlaylistLoader plugin
;	Delete "$INSTDIR\Plugins\gen_skinmanager.dll" ; placeholder for old essential pack plugin
;	Delete "$INSTDIR\Plugins\gen_timerestore.dll" ; placeholder for old essential pack plugin
;	Delete "$INSTDIR\Plugins\gen_undo.dll" ; placeholder for old essential pack plugin
	DeleteRegKey HKCR .wpz
	DeleteRegKey HKCR aip_file

	DeleteINIStr "$WINAMPINI" "Winamp" "mb_open"
	DeleteINIStr "$WINAMPINI" "Winamp" "last_shortdesc"
	DeleteINIStr "$WINAMPINI" "Winamp" "last_shorturl"
	WriteINIStr "$WINAMPINI" "Winamp" "mw_open" "1"

	ReadINIStr $R0 "$WINAMPINI" "Winamp" "outname"
	${If} $R0 == ""
		${If} ${FileExists} "$INSTDIR\Plugins\out_ds.dll"
			WriteINIStr "$WINAMPINI" "Winamp" "outname" "out_ds.dll"
		${Else}
			${If} ${FileExists} "$INSTDIR\Plugins\out_wave.dll"
				WriteINIStr "$WINAMPINI" "Winamp" "outname" "out_wave.dll"
			${EndIf}
		${EndIf}
	${EndIf}

	Call StartMenu_WriteData

	WriteUninstaller "$INSTDIR\UninstWA.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "DisplayName" "${WINAMP}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "DisplayIcon" "$INSTDIR\${WINAMPEXE},0"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "UninstallString" '"$INSTDIR\UninstWA.exe"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "HelpLink" "http://forums.winamp.com"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "URLInfoAbout" "http://www.winamp.com/"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "URLUpdateInfo" "http://www.winamp.com/"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "Publisher" "Winamp SA"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "DisplayVersion" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MINOR_SECOND}"
	; WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "DisplayVersion" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MINOR_SECOND} ${InstallType}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "NoRepair" 1

	WriteRegStr HKCU "Software\${WINAMP}" "" "$INSTDIR"

	; Add to App Paths for: Start -> Run -> Winamp
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${WINAMPEXE}" "" "$INSTDIR\${WINAMPEXE}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${WINAMPEXE}" "Path" "$INSTDIR"

	WriteINIStr "$INSTINI" "installer" "build" "${VERSION_SUFFIX}"
	${GetTime} "" "LS" $0 $1 $2 $3 $4 $5 $6
	WriteINIStr "$INSTINI" "installer" "date" "$1/$0/$2 $4:$5:$6"

	WriteINIStr "$WINAMPINI" "WinampReg" "IsFirstInst" $FIRSTINSTALL
	WriteINIStr "$WINAMPINI" "WinampReg" "NeedReg" "1"

	${If} $needplaystart <> 0 
		WriteINIStr "$WINAMPINI" "SETUP" "WAParam" "$WINAMPM3U"
	${Else}
		DeleteINIStr "$WINAMPINI" "SETUP" "WAParam"
	${EndIf}
SectionEnd