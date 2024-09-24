Var LANG_DIR

!macro WALANG_SECTION LANGID NSIS_LANGID

!ifdef "LANG_USE_${LANGID}" | LANG_USE_ALL | "LANG_USE_${LANGID}-CMTY"
!ifdef WALANG_SHOWSECTIONS
	${WinampSection} "language${NSIS_LANGID}" "${LANGFILE_${NSIS_LANGID}_NAME}" IDX_SEC_${LANGID}
		${SECTIONIN_STD}
!else
	${WinampSection} "language${NSIS_LANGID}" "-${LANGFILE_${NSIS_LANGID}_NAME}" IDX_SEC_${LANGID}
		SectionIn 1 2 3 4 5 6 7 8 RO
!endif ; WALANG_SHOWSECTIONS
		SetOutPath "$LANG_DIR"
; TODO decide if we ship as wlz or folders (smaller download installer but longer to install a full install)
!if 1
		File "..\..\resources\languages\Winamp-${LANGID}.wlz"
		${If} '${LANG_${NSIS_LANGID}}' == $LANGUAGE
			WriteINIStr "$WINAMPINI" "Winamp" "langpack" "Winamp-${LANGID}.wlz"
		${EndIf}
!else
		File /r "..\..\resources\languages\Winamp-${LANGID}"
		${If} '${LANG_${NSIS_LANGID}}' == $LANGUAGE
			WriteINIStr "$WINAMPINI" "Winamp" "langpack" "Winamp-${LANGID}"
		${EndIf}
!endif
	${WinampSectionEnd}
!endif
!macroend

!ifdef WALANG_SHOWSECTIONS
SectionGroup $(IDS_GRP_LANGUAGES) IDX_GRP_LANGUAGES
!endif

!ifdef WALANG_SHOWSECTIONS
	${WinampSection} "languageEnglish" "${LANGFILE_ENGLISH_NAME}" IDX_SEC_EN-US
!else
	${WinampSection} "languageEnglish" "-${LANGFILE_ENGLISH_NAME}" IDX_SEC_EN-US
!endif ; WALANG_SHOWSECTIONS
		SectionIn 1 2 3 4 5 6 7 8 RO
		ReadINIStr $LANG_DIR "$WINAMPINI" "Winamp" "LangDir"
		${If} $LANG_DIR == ""
			StrCpy $LANG_DIR "Lang"
		${EndIf}

		Push $INSTDIR
		Push $LANG_DIR
		nsis_winamp::GetFullPath
		Pop $LANG_DIR

		${If} '1033' == $LANGUAGE
			DeleteINIStr "$WINAMPINI" "Winamp" "langpack"
		${EndIf}
	${WinampSectionEnd} ; IDX_SEC_EN-US

	;!insertmacro WALANG_SECTION "DE-DE" "German"
	!insertmacro WALANG_SECTION "ES-US" "SpanishInternational"
	!insertmacro WALANG_SECTION "FR-FR" "French"
	;!insertmacro WALANG_SECTION "IT-IT" "Italian"
	;!insertmacro WALANG_SECTION "NL-NL" "Dutch"
	!insertmacro WALANG_SECTION "PL-PL" "Polish"
	;!insertmacro WALANG_SECTION "SV-SE" "Swedish"
	!insertmacro WALANG_SECTION "RU-RU" "Russian"
	;!insertmacro WALANG_SECTION "ZH-TW" "TradChinese"
	;!insertmacro WALANG_SECTION "ZH-CN" "SimpChinese"
	!insertmacro WALANG_SECTION "JA-JP" "Japanese"
	;!insertmacro WALANG_SECTION "KO-KR" "Korean"
	!insertmacro WALANG_SECTION "TR-TR" "Turkish"
	!insertmacro WALANG_SECTION "PT-BR" "PortugueseBR"
	!insertmacro WALANG_SECTION "RO-RO" "Romanian"
	!insertmacro WALANG_SECTION "HU-HU" "Hungarian"
	;!insertmacro WALANG_SECTION "ID-ID" "Indonesian"

!ifdef WALANG_SHOWSECTIONS
SectionGroupEnd
!endif