!ifndef NULLSOFT_WINAMP_INSTALLER_SECTION_DESCRIPTION_HEADER
!define NULLSOFT_WINAMP_INSTALLER_SECTION_DESCRIPTION_HEADER

!macro GetSectionDescription __sectionIndex __outputVar
	Push "${__sectionIndex}"
	!ifndef __UNINSTALL__
		Call GetSectionDescription
	!else
		Call un.GetSectionDescription
	!endif
	Pop ${__outputVar}
!macroend

!define GetSectionDescription "!insertmacro 'GetSectionDescription'"

!macro InstallSectionDescriptionBegin
	Function GetSectionDescription
		Exch $0
!macroend

!macro UninstallSectionDescriptionBegin
	Function un.GetSectionDescription
		Exch $0
!macroend

!macro SectionDescription __sectionIndex __sectionDescription
	!verbose push
	!verbose 4
	StrCmp $0 ${__sectionIndex} 0 +3
		StrCpy $0 "${__sectionDescription}"
		!ifndef __UNINSTALL__
			Goto getsectiondescription_end
		!else
			Goto ungetsectiondescription_end
		!endif
	!verbose pop
!macroend

!macro SectionDescriptionEnd
		StrCpy $0 ""
		!ifndef __UNINSTALL__
			Goto getsectiondescription_end
		!else
			Goto ungetsectiondescription_end
		!endif
		!ifndef __UNINSTALL__
			getsectiondescription_end:
		!else
			ungetsectiondescription_end:
		!endif
		Exch $0
	FunctionEnd
!macroend

!macro WALANG_DESCRIPTION LANGID NSIS_LANGID
	!ifdef "LANG_USE_${LANGID}" | LANG_USE_ALL
		!insertmacro SectionDescription ${IDX_SEC_${LANGID}} "${LANGFILE_${NSIS_LANGID}_NAME}"
	!endif
!macroend

!macro DESCRIPTION __sectionIndex __descriptionText
	!ifdef ${__sectionIndex}
		!verbose push
		!verbose 2
		!insertmacro SectionDescription ${${__sectionIndex}} "${__descriptionText}"
		!verbose pop
	!endif
!macroend

!define DESCRIPTION "!insertmacro 'DESCRIPTION'"
!define WALANG_DESCRIPTION "!insertmacro 'WALANG_DESCRIPTION'"
!define INSTALL_DESCRIPTION_TABLE_BEGIN "!insertmacro 'InstallSectionDescriptionBegin'"
!define UNINSTALL_DESCRIPTION_TABLE_BEGIN "!insertmacro 'UninstallSectionDescriptionBegin'"
!define DESCRIPTION_TABLE_END "!insertmacro 'SectionDescriptionEnd'"

!endif ;NULLSOFT_WINAMP_INSTALLER_SECTION_DESCRIPTION_HEADER