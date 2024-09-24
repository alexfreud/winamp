SectionGroup $(IDS_GRP_UIEXTENSION) IDX_GRP_UIEXTENSION ;  User Interface Extensions
	${WinampSection} "GlobalHotkeys" $(secHotKey) IDX_SEC_HOTKEY      ; >>> [Global Hotkey Support]
		${SECTIONIN_LITE}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\gen_hotkeys.dll
	${WinampSectionEnd}                                               ; <<< [Global Hotkey Support]

	${WinampSection} "TrayControl" $(secTray) IDX_SEC_TRAYCTRL        ; >>> [Nullsoft Tray Control]
		${SECTIONIN_LITE}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\gen_tray.dll
	${WinampSectionEnd}                                               ; <<< [Nullsoft Tray Control]

!ifndef WINAMP64
!ifdef full | std
	${WinampSection} "FreeformSkins" $(compModernSkin) IDX_SEC_FREEFORM      ; >>> [Modern Skin Support]
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\System
		File "${FILES_PATH}\System\filereader.w5s"
		; File /nonfatal "${FILES_PATH}\System\filereader.wbm"
		; File "${FILES_PATH}\System\dlmgr.w5s"
		; File /nonfatal "${FILES_PATH}\System\dlmgr.wbm"
		File "${FILES_PATH}\System\wac_downloadManager.w5s"
		File "${FILES_PATH}\System\timer.w5s"

		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\gen_ff.dll

		DetailPrint "$(IDS_RUN_EXTRACT) $(IDS_DEFAULT_SKIN)..."
		SetDetailsPrint none

		File /r /x CVS /x *.psd "..\..\resources\data\Freeform"
		Delete $INSTDIR\Plugins\Freeform\wacs\jpgload\jpgload.wac
		SetOutPath $INSTDIR\Plugins\Freeform\wacs\freetype
		File ${FILES_PATH}\Plugins\Freeform\wacs\freetype\freetype.wac

		SetOutPath $SETTINGSDIR
		File "..\..\resources\data\links.xml"

		Call GetSkinDir
		Pop $R0

		SetOutPath "$R0\${MODERNSKINNAME}"
		File /r /x CVS /x *.mi /x *.bat /x *.m "..\..\resources\skins\${MODERNSKINNAME}\*.*"

		SetOutPath "$R0\Bento"
		File /r /x CVS /x *.mi /x *.m /x *.psd "..\..\resources\skins\Bento\*.*"

		SetOutPath "$R0\Big Bento"
		File /r /x CVS /x *.mi /x *.bat /x *.m /x *.psd "..\..\resources\skins\Big Bento\*.*"

		SetDetailsPrint lastused
		SetOutPath "$INSTDIR\Plugins"

		ReadIniStr $0 "$WINAMPINI" "Winamp" "skin"
		${If} "$0" == "Winamp Bento"
			WriteIniStr "$WINAMPINI" "Winamp" "skin" "Bento"
		${EndIf}
	${WinampSectionEnd}                                            ; <<< [Modern Skin Support]
!endif ; full | std
!endif ; WINAMP64

	${FrenchRadio_InsertInstallSections}
SectionGroupEnd ;  User Interface Extensions