!ifndef WINAMP_SECTIONS_HELPER_HEADER
!define WINAMP_SECTIONS_HELPER_HEADER

; use WinampSection/WinampSectionEnd instead of Section/SectionEnd if you want section
; to store invariant name in install.ini
; Example:
; WinampSection "winampSection1" "$(IDS_WINAMPSECTION1_TITLE)" IDX_WINAMPSECTION1
;	....
;	....
; WinampSectionEnd
;
;

!macro WinampSection __sectionName __sectionTitle __sectionIndex
	!ifdef WINAMP_SECTION_ACTIVE_ID | WINAMP_SECTION_ACTIVE_NAME
		!error "Missing WinampSectionEnd"
	!endif
	!define WINAMP_SECTION_ACTIVE_ID		"${__sectionIndex}"
	!define WINAMP_SECTION_ACTIVE_NAME		"${__sectionName}"
	Section "${__sectionTitle}" "${__sectionIndex}"
!macroend

!define WinampSection "!insertmacro 'WinampSection'"

!macro SECTIONHEADER_LINE __line
	!appendfile "${WINAMP_SECTION_HEADER}" "${__line}$\r$\n"
!macroend

!define SECTIONHEADER_LINE	"!insertmacro 'SECTIONHEADER_LINE'"

!macro InitializeGetSectionName
	!ifdef WINAMP_SECTION_HEADER
		!include "${WINAMP_SECTION_HEADER}"
		!delfile "${WINAMP_SECTION_HEADER}"
		SectionGetText $0 $0
		GetSectionName_FunctionEnd:
			Exch $0
		FunctionEnd
	!else
		!define GetSectionName SectionGetText
	!endif
!macroend

!define InitializeGetSectionName	"!insertmacro 'InitializeGetSectionName'"

!macro WinampSectionEnd
	SectionEnd
!ifndef WINAMP_SECTION_ACTIVE_ID | WINAMP_SECTION_ACTIVE_NAME
	!error "Missing WinampSection"
!endif

!ifndef WINAMP_SECTION_HEADER
	!tempfile WINAMP_SECTION_HEADER
	!delfile "${WINAMP_SECTION_HEADER}"

	${SECTIONHEADER_LINE} "!define GetSectionName $\"!insertmacro 'GetSectionName'$\""
	${SECTIONHEADER_LINE} "!macro GetSectionName __sectionIndex __outputVar"
	${SECTIONHEADER_LINE} "$\tPush ${__sectionIndex}"
	${SECTIONHEADER_LINE} "$\tCall GetSectionName"
	${SECTIONHEADER_LINE} "$\tPop ${__outputVar}"
		${SECTIONHEADER_LINE} "!macroend"
	${SECTIONHEADER_LINE} "Function GetSectionName"
	${SECTIONHEADER_LINE} "$\tExch $0"
!endif

	${SECTIONHEADER_LINE} "$\tIntCmp $0	${${WINAMP_SECTION_ACTIVE_ID}} 0 +3 +3"
	${SECTIONHEADER_LINE} "$\t$\tStrCpy $0	${WINAMP_SECTION_ACTIVE_NAME}"
	${SECTIONHEADER_LINE} "$\t$\tGoto GetSectionName_FunctionEnd"

	!undef WINAMP_SECTION_ACTIVE_ID
	!undef WINAMP_SECTION_ACTIVE_NAME
!macroend

!define WinampSectionEnd "!insertmacro 'WinampSectionEnd'"
!endif