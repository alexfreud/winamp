!ifndef NULLOSFT_WINAMP_INSTALLER_SUPPORTED_LANGUAGES_HEADER
!define NULLOSFT_WINAMP_INSTALLER_SUPPORTED_LANGUAGES_HEADER

!macro WALANG_INCLUDE LANGID NSIS_LANGID

	!ifdef "LANG_USE_${LANGID}" | LANG_USE_ALL
		!define WALANG_INCLUDE_OKTOINCLUDE
	!endif

	!ifdef WALANG_INCLUDE_OKTOINCLUDE
		!echo "Including language support for: ${LANGID}"
		!verbose push
		!verbose 2

		!ifndef MUI_LANGDLL_ALLLANGUAGES
			!define MUI_LANGDLL_ALLLANGUAGES
		!endif ; MUI_LANGDLL_ALLLANGUAGES

		!ifndef MUI_LANGDLL_ALWAYSSHOW
			!ifdef WALANG_ATLEASTONE
				!define MUI_LANGDLL_ALWAYSSHOW
			!endif ; WALANG_ATLEASTONE
		!endif ; MUI_LANGDLL_ALWAYSSHOW

		!ifndef WALANG_ATLEASTONE
			!define WALANG_ATLEASTONE
		!endif ; WALANG_ATLEASTONE

		!insertmacro MUI_INSERT
		!ifndef "NSIS_NLF_${NSIS_LANGID}_LOADED"
			LoadLanguageFile "${NSISDIR}\Contrib\Language files\${NSIS_LANGID}.nlf"
			!define "NSIS_NLF_${NSIS_LANGID}_LOADED"
		!endif
    	
		!ifndef LANGFILE_DEFAULT
			!define LANGFILE_DEFAULT "${NSISDIR}\Contrib\Language files\English.nsh"
		!endif
		
		!insertmacro LANGFILE_INCLUDE "${NSISDIR}\Contrib\Language files\${NSIS_LANGID}.nsh"
		
		!ifdef LANGFILE_DEFAULT
			!undef LANGFILE_DEFAULT
		!endif
		
	;	!define LANGFILE_DEFAULT ".\languages\installer_en-us.nsh"
		
		!insertmacro LANGFILE_INCLUDE_WITHDEFAULT ".\languages\installer_${LANGID}.nsh" ".\languages\installer_en-us.nsh"
		
	;	!undef LANGFILE_DEFAULT
		!define LANGFILE_DEFAULT "${NSISDIR}\Contrib\Language files\English.nsh"

		!ifndef MUI_LANGDLL_LANGUAGES
			!ifdef MUI_LANGDLL_ALLLANGUAGES
				!define MUI_LANGDLL_LANGUAGES "'${LANGFILE_${NSIS_LANGID}_NAME}' '${LANG_${NSIS_LANGID}}' "
			!else
				!define MUI_LANGDLL_LANGUAGES "'${LANGFILE_${NSIS_LANGID}_NAME}' '${LANG_${NSIS_LANGID}}' '${LANG_${NSIS_LANGID}_CP}' "
			!endif
		!else
			!ifdef MUI_LANGDLL_LANGUAGES_TEMP
				!undef MUI_LANGDLL_LANGUAGES_TEMP
			!endif
			!define MUI_LANGDLL_LANGUAGES_TEMP "${MUI_LANGDLL_LANGUAGES}"
			!undef MUI_LANGDLL_LANGUAGES
			!ifdef MUI_LANGDLL_ALLLANGUAGES
				!define MUI_LANGDLL_LANGUAGES "'${LANGFILE_${NSIS_LANGID}_NAME}' '${LANG_${NSIS_LANGID}}' ${MUI_LANGDLL_LANGUAGES_TEMP}"
			!else
				!define MUI_LANGDLL_LANGUAGES "'${LANGFILE_${NSIS_LANGID}_NAME}' '${LANG_${NSIS_LANGID}}' '${LANG_${NSIS_LANGID}_CP}' ${MUI_LANGDLL_LANGUAGES_TEMP}"
			!endif
		!endif
		!undef WALANG_INCLUDE_OKTOINCLUDE
		!verbose pop
	!endif
!macroend

!insertmacro WALANG_INCLUDE "EN-US" "English" 
;!insertmacro WALANG_INCLUDE "DE-DE" "German"
!insertmacro WALANG_INCLUDE "ES-US" "SpanishInternational"
!insertmacro WALANG_INCLUDE "FR-FR" "French"
;!insertmacro WALANG_INCLUDE "IT-IT" "Italian"
;!insertmacro WALANG_INCLUDE "NL-NL" "Dutch"
!insertmacro WALANG_INCLUDE "PL-PL" "Polish"
;!insertmacro WALANG_INCLUDE "SV-SE" "Swedish"
!insertmacro WALANG_INCLUDE "RU-RU" "Russian"
;!insertmacro WALANG_INCLUDE "ZH-CN" "SimpChinese"
;!insertmacro WALANG_INCLUDE "ZH-TW" "TradChinese"
!insertmacro WALANG_INCLUDE "JA-JP" "Japanese"
;!insertmacro WALANG_INCLUDE "KO-KR" "Korean"
!insertmacro WALANG_INCLUDE "TR-TR" "Turkish"
!insertmacro WALANG_INCLUDE "PT-BR" "PortugueseBR"
!insertmacro WALANG_INCLUDE "RO-RO" "Romanian"
!insertmacro WALANG_INCLUDE "HU-HU" "Hungarian"
;!insertmacro WALANG_INCLUDE "ID-ID" "Indonesian"

!include ".\sections\languages.nsh"

!endif ;NULLOSFT_WINAMP_INSTALLER_SUPPORTED_LANGUAGES_HEADER