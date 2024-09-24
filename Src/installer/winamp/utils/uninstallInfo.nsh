!ifndef NULLSOFT_WINAMP_UNINSTALL_INFO_LIB_HEADER
!define NULLSOFT_WINAMP_UNINSTALL_INFO_LIB_HEADER

!include "util.nsh"

!define SOFTWARE_UNINSTALL_KEY "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall"

!macro UninstallInfo_FindProgramKeyHelper __regView __subKey __value __outputVar
	SetRegView ${__regView}
	ClearErrors
	ReadRegStr ${__outputVar} HKCU "${__subkey}" "${__value}"
	${IfNot} ${Errors}
		StrCpy ${__outputVar} "${__regView},HKCU,${__subkey}"
	${Else}	
		ClearErrors
		ReadRegStr ${__outputVar} HKLM "${__subkey}" "${__value}"
		${IfNot} ${Errors}
			StrCpy ${__outputVar} "${__regView},HKLM,${__subkey}"
		${Else}
			StrCpy ${__outputVar} ""
		${EndIf}
	${EndIf}
	SetRegView lastused
!macroend

!define UninstallInfo_FindProgramKeyHelper "!insertmacro 'UninstallInfo_FindProgramKeyHelper'"

!macro UninstallInfo_FindProgramKeyInternal
	Exch $0
	Push $1

	StrCpy $1 "${SOFTWARE_UNINSTALL_KEY}\$0"
	${UninstallInfo_FindProgramKeyHelper} 32 $1 "UninstallString" $0
	${If} $0 == ""
		${UninstallInfo_FindProgramKeyHelper} 64 $1 "UninstallString" $0
		${If} $0 == ""
			SetErrors
		${EndIf}
	${EndIf}

	Pop $1
	Exch $0
!macroend

!macro UninstallInfo_FindProgramKey __programName __outputVar
	Push "${__programName}"
	${CallArtificialFunction} UninstallInfo_FindProgramKeyInternal
	Pop  "${__outputVar}"
!macroend

!define UninstallInfo_FindProgramKey `!insertmacro UninstallInfo_FindProgramKey`

!macro UninstallInfo_GetRegistryValueHelper __regView __readFunc __unparsedPath __value __outputVar
	SetRegView ${__regView}
	StrCpy ${__outputVar} "${__unparsedPath}" 4 3
	${If} ${__outputVar} == "HKCU"
		StrCpy ${__outputVar} "${__unparsedPath}" "" 8
		${__readFunc} ${__outputVar} HKCU "${__outputVar}" "${__value}"
	${ElseIf} ${__outputVar} == "HKLM"
		StrCpy ${__outputVar} "${__unparsedPath}" "" 8
		${__readFunc} ${__outputVar} HKLM "${__outputVar}" "${__value}"
	${Else}
		StrCpy ${__outputVar} ""
		SetErrors
	${EndIf}
	SetRegView lastused
!macroend

!define UninstallInfo_GetRegistryValueHelper "!insertmacro 'UninstallInfo_GetRegistryValueHelper'"

!macro UninstallInfo_GetRegistryValueInternal __readFunc
	Exch $0
	Exch 1
	Exch $1 ; path in format VV,KKKK,PATH where VV - registry view (32|64), KKKK - rootkey (HKCU|HKLM), PATH - subkey path
	Push $2 

	StrCpy $2 $1 2 0
	${If} $2 == 32
		${UninstallInfo_GetRegistryValueHelper} 32 "${__readFunc}" $1 $0 $2
	${ElseIf} $2 == 64
		${UninstallInfo_GetRegistryValueHelper} 64 "${__readFunc}" $1 $0 $2
	${Else}
		StrCpy $2 ""
		SetErrors
	${EndIf}

	Exch $2
	Exch 2
	Pop $0
	Pop $1
!macroend

!macro UninstallInfo_GetStringValueInternal
	!insertmacro 'UninstallInfo_GetRegistryValueInternal' "ReadRegStr"
!macroend

!macro UninstallInfo_GetDwordValueInternal
	!insertmacro 'UninstallInfo_GetRegistryValueInternal' "ReadRegDWORD"
!macroend

!macro UninstallInfo_GetStringValue __programRegPath __valueName __outputVar
	Push "${__programRegPath}"
	Push "${__valueName}"
	${CallArtificialFunction} UninstallInfo_GetStringValueInternal
	Pop  "${__outputVar}"
!macroend
!define UninstallInfo_GetStringValue "!insertmacro 'UninstallInfo_GetStringValue'"

!macro UninstallInfo_GetExpandableStringValue __programRegPath __valueName __outputVar
	${UninstallInfo_GetStringValue} "${__programRegPath}" "${__valueName}" "${__outputVar}"
	ExpandEnvStrings ${__outputVar} "${__outputVar}"
!macroend

!define UninstallInfo_GetExpandableStringValue "!insertmacro 'UninstallInfo_GetExpandableStringValue'"

!macro UninstallInfo_GetDwordValue __programRegPath __valueName __outputVar
	Push "${__programRegPath}"
	Push "${__valueName}"
	${CallArtificialFunction} UninstallInfo_GetDwordValueInternal
	Pop  "${__outputVar}"
!macroend

!define UninstallInfo_GetDwordValue "!insertmacro 'UninstallInfo_GetDwordValue`"

!endif ;NULLSOFT_WINAMP_UNINSTALL_INFO_LIB_HEADER