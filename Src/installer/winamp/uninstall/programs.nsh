/*!ifndef NULLSOFT_WINAMP_UNINSTALL_BUNDLE_HEADER
!define NULLSOFT_WINAMP_UNINSTALL_BUNDLE_HEADER

!include "fileFunc.nsh"
!include ".\utils\wafuncs.nsh"
!include ".\utils\uninstallInfo.nsh"
!include ".\utils\sectionDescription.nsh"

!macro UninstallBundle_IsGroupEmpty __outputVar
	Call ${INSTALL_TYPE_PREFIX}UninstallBundle_IsGroupEmpty
	Pop "${__outputVar}"
!macroend

!define UninstallBundle_IsGroupEmpty  "!insertmacro 'UninstallBundle_IsGroupEmpty'"

!macro UninstallBundle_GetDescriptionProvider __outputVar
	GetFunctionAddress ${__outputVar} ${INSTALL_TYPE_PREFIX}UninstallBundle_DescriptionProvider
!macroend

!define UninstallBundle_GetDescriptionProvider  "!insertmacro 'UninstallBundle_GetDescriptionProvider'"

!macro UninstallBundle_UninstallProgram __displayName __programName __uninstallParam __outputResult
	Push "${__displayName}"
	Push "${__uninstallParam}"
	Push "${__programName}"
	Call ${INSTALL_TYPE_PREFIX}UninstallBundle_UninstallProgram
	Pop ${__outputResult}
!macroend

!define UninstallBundle_UninstallProgram  "!insertmacro 'UninstallBundle_UninstallProgram'"

SectionGroup "${INSTALL_TYPE_PREFIX}$(MUI_UNTEXT_CONFIRM_TITLE)" IDX_UNINSTALL_BUNDLES_GROUP
	Section "${INSTALL_TYPE_PREFIX}$(IDS_BUNDLE1_NAME)" IDX_UNINSTALL_WINAMP_REMOTE
		${UninstallBundle_UninstallProgram} "${__SECTION__}" "Orb" "/S _?=$$INSTDIR" $0
	SectionEnd

	Section "${INSTALL_TYPE_PREFIX}$(IDS_BUNDLE2_NAME)" IDX_UNINSTALL_WINAMP_TOOLBAR
		${UninstallBundle_UninstallProgram} "${__SECTION__}" "Winamp Toolbar" "/S _?=$$INSTDIR" $0
	SectionEnd

	Section "${INSTALL_TYPE_PREFIX}Winamp Detector Plug-in" IDX_UNINSTALL_BROWSER_PLUGIN
		${UninstallBundle_UninstallProgram} "${__SECTION__}" "Winamp Detect" "/S _?=$$INSTDIR" $0
	SectionEnd

	Section "${INSTALL_TYPE_PREFIX}$(IDS_BUNDLE3_NAME)" IDX_UNINSTALL_EMUSIC
		${UninstallBundle_UninstallProgram} "${__SECTION__}" "eMusic Promotion" "/S _?=$$INSTDIR" $0
	SectionEnd
SectionGroupEnd

Function ${INSTALL_TYPE_PREFIX}UninstallBundle_InitializeSection
	Exch $0
	Exch 1
	Exch $1
	Push $2
	Push $3

	${UninstallInfo_FindProgramKey} $0 $2
	${If} $2 == ""
		SectionSetText $1 ""
		SectionGetFlags $1 $2
		IntOp $3 ${SF_SELECTED} ~
		IntOp $2 $2 & $3
		SectionSetFlags $1 $2
	${Else}
		${UninstallInfo_GetStringValue} $2 "DisplayName" $3
		${If} $3 != ""
			SectionSetText $1 $3
		${EndIf}
		SectionGetFlags $1 $2
		IntOp $2 $2 | ${SF_SELECTED}
		SectionSetFlags $1 $2
	${EndIf}

	Pop $3
	Pop $2
	Pop $1
	Pop $0
FunctionEnd

!macro UninstallBundle_InitializeSection __sectionIndex __bundleName
	Push "${__sectionIndex}"
	Push "${__bundleName}"
	Call ${INSTALL_TYPE_PREFIX}UninstallBundle_InitializeSection
!macroend

!define UninstallBundle_InitializeSection "!insertmacro 'UninstallBundle_InitializeSection'"

Function ${INSTALL_TYPE_PREFIX}UninstallBundle_InitializeGroup
	${UninstallBundle_InitializeSection} ${IDX_UNINSTALL_WINAMP_TOOLBAR} "Winamp Toolbar"
	${UninstallBundle_InitializeSection} ${IDX_UNINSTALL_BROWSER_PLUGIN} "Winamp Detect"
	${UninstallBundle_InitializeSection} ${IDX_UNINSTALL_WINAMP_REMOTE} "Orb"
	${UninstallBundle_InitializeSection} ${IDX_UNINSTALL_EMUSIC} "eMusic Promotion"
FunctionEnd

!define UninstallBundle_InitializeGroup "Call ${INSTALL_TYPE_PREFIX}UninstallBundle_InitializeGroup"

Function ${INSTALL_TYPE_PREFIX}UninstallBundle_IsGroupEmpty
	Push $0
	Push $1
	Push $2

	StrCpy $1 ${IDX_UNINSTALL_BUNDLES_GROUP}
	${Do}
		IntOp $1 $1 + 1
		ClearErrors
		SectionGetFlags $1 $0
		${IfNot} ${Errors}
			IntOp $2 $0 & ${SF_SECGRPEND}
			${If} 0 != $2
				StrCpy $0 "true"
				${Break}
			${Else}
				IntOp $2 $0 & ${SF_SELECTED}
				${If} 0 != $2
					Push $3
					SectionGetText $1 $3
					Pop $3
					StrCpy $0 "false"
					${Break}
				${EndIf}
			${EndIf}
		${Else}
			StrCpy $0 "true"
			${Break}
		${EndIf}
	${Loop}
	Pop $2
	Pop $1
	Exch $0
FunctionEnd

Function  ${INSTALL_TYPE_PREFIX}UninstallBundle_DescriptionProvider
	Exch $0
	Push $1
	${Select} $0
		${Case} ${IDX_UNINSTALL_WINAMP_TOOLBAR}
			StrCpy $1 "Winamp Toolbar"
		${Case} ${IDX_UNINSTALL_BROWSER_PLUGIN}
			StrCpy $1 "Winamp Detect"
		${Case} ${IDX_UNINSTALL_WINAMP_REMOTE}
			StrCpy $1 "Orb"
		${Case} ${IDX_UNINSTALL_EMUSIC}
			StrCpy $1 "eMusic Promotion"
		${CaseElse}
			StrCpy $1 ""
	${EndSelect}

	${If} $1 != ""
		Push $2
		${UninstallInfo_FindProgramKey} $1 $2
		${If} $2 != ""
			${UninstallInfo_GetStringValue} $2 "winampDescription" $1
			${If} $1 == ""
				${UninstallInfo_GetStringValue} $2 "Comments" $1
			${EndIf}
		${Else}
			StrCpy $1 ""
		${EndIf}
		Pop $2
	${EndIf}

	${If} $1 == ""
		Push $0
		Call un.GetSectionDescription
		Pop $1
	${EndIf}

	Exch $1
	Exch 1
	Pop $0
FunctionEnd

Function ${INSTALL_TYPE_PREFIX}UninstallBundle_UninstallProgram
	Exch $0
	Exch 1
	Exch $1
	Exch 2
	Exch $2
	Push $3
	Push $4

	StrCpy $4 "$(IDS_UNINSTALL_BUNDLE_TEMPLATE)"

	${UninstallInfo_FindProgramKey} $0 $3
	${If} $3 != ""
		${UninstallInfo_GetExpandableStringValue} $3 "winampUninstall" $0
		${If} $0 == ""
			${UninstallInfo_GetExpandableStringValue} $3 "UninstallString" $0
			${If} $0 != ""
			${AndIf} $1 != ""

				${Path_UnquoteSpaces} $0 $0
				${Path_RemoveBlanks} $0 $0

				StrLen $2 "_?=$$INSTDIR"
				StrLen $3 $1
				${If} $3 >= $2
					Push $4
					StrCpy $4 $1 "" -$2
					${If} $4 == "_?=$$INSTDIR"
						IntOp $2 $3 - $2
						StrCpy $1 $1 $2 0
						${GetParent} $0 $2
						${Path_UnquoteSpaces} $2 $2
						StrCpy $1 "$1 _?=$2"
						StrCpy $3 "deleteUninstaller"
					${Else}
						StrCpy $3 ""
					${EndIf}
					Pop $4
				${Else}
					StrCpy $3 ""
				${EndIf}
				StrCpy $0 "$\"$0$\" $1"
			${EndIf}
		${EndIf}
	${Else}
		StrCpy $0 ""
	${EndIf}

	${If} $0 != ""
		ClearErrors
		DetailPrint $4
		SetDetailsPrint none
		ExecWait '$0' $2
		${IfNot} ${Errors}
		${AndIf} $2 == 0
			${If} $3 == "deleteUninstaller"
				${Path_RemoveArgs} $0 $1
				${Path_UnquoteSpaces} $1 $1
				${Path_RemoveBlanks} $1 $1
				Delete "$1"
				${IfNot} ${Errors}
					${GetParent} $1 $3
					RMDir "$3"
				${EndIf}
			${EndIf}
		${EndIf}
		SetDetailsPrint lastused
	${Else}
		StrCpy $2 ""
	${EndIf}

	Pop $4
	Pop $3
	Exch $2
	Exch 2
	Pop $1
	Pop $0
FunctionEnd

!endif ;NULLSOFT_WINAMP_UNINSTALL_BUNDLE_HEADER*/