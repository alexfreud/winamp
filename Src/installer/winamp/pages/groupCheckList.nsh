!ifndef NULLSOFT_WINAMP_PAGE_SECTIONGROUPLIST_HEADER
!define NULLSOFT_WINAMP_PAGE_SECTIONGROUPLIST_HEADER

!include "util.nsh"
!include "nsDialogs.nsh"
!include "logicLib.nsh"
!include ".\utils\sectionDescription.nsh"

!define CLS_VCENTER			0x00000001
!define CLS_BOLDNAME		0x00000002
!define CLS_RADIOBUTTON		0x00000004
!define CLS_COMPACT			0x00000008
!define CLS_READONLY		0x00000010

!define CLIS_DISABLED		0x00000001
!define CLIS_CHECKED		0x00000002

!ifndef SWP_NOSIZE
	!define SWP_NOSIZE		0x0001
!endif

!ifndef SWP_NOZORDER
	!define SWP_NOZORDER   	0x0004
!endif

!ifndef SWP_NOMOVE
	!define SWP_NOMOVE 		0x0002
!endif

!ifndef SWP_NOACTIVATE
	!define SWP_NOACTIVATE 	0x0010
!endif

!macro GroupCheckList_InsertSectionHelper __index
	SectionGetFlags ${__index} ${GCL_FLAGS}
	${IfNot} ${Errors}
		IntOp ${GCL_TEMP} ${GCL_FLAGS} & ${SF_SECGRPEND}
		${If} 0 != ${GCL_TEMP}
			SetErrors
		${Else}
			StrCpy ${GCL_STYLE} 0
			IntOp ${GCL_TEMP} ${GCL_FLAGS} & ${SF_RO}
			${If} 0 != ${GCL_TEMP}
				IntOp ${GCL_STYLE} ${GCL_STYLE} | ${CLIS_DISABLED}
			${EndIf}

			IntOp ${GCL_TEMP} ${GCL_FLAGS} & ${SF_SELECTED}
			${If} 0 != ${GCL_TEMP}
				IntOp ${GCL_STYLE} ${GCL_STYLE} | ${CLIS_CHECKED}
			${EndIf}

			SectionGetText ${__index} ${CGL_HEADER}
			${IfNot} ${Errors}
			${AndIf} "${CGL_HEADER}" != ""
				StrCpy ${GCL_LISTTOP} ""
				${If} ${GCL_DESCFUNC} != ""
					Push ${__index}
					Call "${GCL_DESCFUNC}"
					Pop ${GCL_LISTTOP}
				${EndIf}

				StrCpy ${GCL_TEMP} ""
				${If} ${GCL_NOTEFUNC} != ""
					Push ${__index}
					Call "${GCL_NOTEFUNC}"
					Pop ${GCL_TEMP}
				${EndIf}

				nsis_chklist::InsertItem /NOUNLOAD ${GCL_CTLHWND} 200000 ${CGL_HEADER} "${GCL_TEMP}" "${GCL_LISTTOP}" ${GCL_STYLE} ${__index}
				Pop ${GCL_TEMP}
				ClearErrors
			${EndIf}
		${EndIf}
	${EndIf}
!macroend

!macro GroupCheckList_CreatePageInternal
	!ifndef __UNINSTALL__
		!define GCL_SUFFIX	inst
	!else
		!define GCL_SUFFIX	uninst
	!endif

	!define GCL_DIALOG		$0
	!define GCL_INDEX		$1
	!define CGL_HEADER		$2
	!define CGL_FOOTER		$3
	!define GCL_STYLE		$4
	!define GCL_CTLHWND 	$5
	!define GCL_TEMP		$6
	!define GCL_FLAGS		$7
	!define GCL_DESCFUNC	$8
	!define GCL_NOTEFUNC	$9

	nsDialogs::Create  /NOUNLOAD 1018
	Exch ${GCL_DIALOG}

	${If} ${GCL_DIALOG} == error
		Pop ${GCL_DIALOG}
		Abort
	${EndIf}

	Exch 1
	Exch ${GCL_INDEX}
	Exch 2
	Exch ${CGL_HEADER}
	Exch 3
	Exch ${CGL_FOOTER}
	Exch 4
	Exch ${GCL_STYLE}
	Exch 5
	Exch ${GCL_DESCFUNC}
	Exch 6
	Exch ${GCL_NOTEFUNC}

	Push ${GCL_CTLHWND}
	Push ${GCL_TEMP}

	${If} ${CGL_HEADER} == "[groupName]"
		ClearErrors
		SectionGetText ${GCL_INDEX} ${CGL_HEADER}
		${If} ${Errors}
			StrCpy ${CGL_HEADER} ""
		${EndIf}
	${EndIf}

	${If} ${CGL_FOOTER} == "[groupName]"
		ClearErrors
		SectionGetText ${GCL_INDEX} ${CGL_FOOTER}
		${If} ${Errors}
			StrCpy ${CGL_FOOTER} ""
		${EndIf}
	${EndIf}

	${If} "${GCL_DESCFUNC}" == "default"
		GetLabelAddress ${GCL_DESCFUNC} ".GroupCheckList_DefaultDescriptionProvider${GCL_SUFFIX}"
	${EndIf}

	${If} "${GCL_NOTEFUNC}" == "default"
		GetLabelAddress ${GCL_NOTEFUNC} ".GroupCheckList_DefaultNoteProvider${GCL_SUFFIX}"
	${EndIf}

	!define GCL_LISTTOP		$R0
	!define GCL_LISTHEIGHT	$R1
	Push ${GCL_LISTTOP}
	Push ${GCL_LISTHEIGHT}
	StrCpy ${GCL_LISTTOP} 0
	StrCpy ${GCL_LISTHEIGHT} "100%"

	${If} ${CGL_HEADER} != ""
		${NSD_CreateLabel} 0 0 100% 8u ${CGL_HEADER}
		Pop ${GCL_CTLHWND}
		${If} ${GCL_CTLHWND} != error
			nsis_chklist::AdjustStaticHeight ${GCL_CTLHWND} 1 3
			Pop ${GCL_TEMP}
			IntOp ${GCL_LISTTOP} ${GCL_TEMP} + 13
		${EndIf}
	${EndIf}

	${If} ${CGL_FOOTER} != ""
		${NSD_CreateLabel} 0 0 100% 8u ${CGL_FOOTER}
		Pop ${GCL_CTLHWND}
		${If} ${GCL_CTLHWND} != error
			nsis_chklist::AdjustStaticHeight ${GCL_CTLHWND} 1 3
			Pop ${GCL_TEMP}

			Push $R2
			Push $R3
			Push $R4
			Push $R5
			Push $R6
			System::Call "*${stRECT} .R3"
			System::Call "User32::GetClientRect(i, i) i (${GCL_DIALOG}, R3) .R2"
			System::Call "*$R3${stRECT} (.R5, .R2, .R6, .R4)"
			System::Free $R3
			IntOp $R2 $R4 - $R2
			Pop $R6
			Pop $R5
			Pop $R4
			Pop $R3

			IntOp $R2 $R2 - ${GCL_TEMP}
			System::Call "User32::SetWindowPos(i, i, i, i, i, i, i) b (${GCL_CTLHWND}, 0, 0, $R2, 0, 0, ${SWP_NOACTIVATE}|${SWP_NOZORDER}|${SWP_NOSIZE})"
			IntOp $R2 $R2 - 4
			IntOp ${GCL_LISTHEIGHT} $R2 - ${GCL_LISTTOP}
			Pop $R2
		${EndIf}
	${EndIf}

	${If} ${GCL_STYLE} == ""
		IntOp ${GCL_STYLE} ${CLS_BOLDNAME} | ${CLS_VCENTER}
	${EndIf}

	nsis_chklist::CreateControl /NOUNLOAD ${GCL_DIALOG} ${GCL_STYLE} 0u ${GCL_LISTTOP} 100% ${GCL_LISTHEIGHT}
	Pop ${GCL_CTLHWND}

	${If} ${GCL_CTLHWND} != "error"
		GetLabelAddress ${GCL_TEMP} ".GroupCheckList_OnCheckChanged${GCL_SUFFIX}"
		nsis_chklist::RegisterCallback /NOUNLOAD ${GCL_CTLHWND} ${GCL_TEMP}

		SendMessage ${GCL_CTLHWND} ${WM_SETREDRAW} 0 0

		CreateFont ${GCL_TEMP} "Tahoma" "8" "400" 
		SendMessage ${GCL_CTLHWND} ${WM_SETFONT} ${GCL_TEMP} 0

		Push ${GCL_FLAGS}
		SectionGetFlags ${GCL_INDEX} ${GCL_FLAGS}
		IntOp ${GCL_TEMP} ${GCL_FLAGS} & 6;${SF_SECGRP}|${SF_SECGRPEND}
		${If} ${SF_SECGRP} == ${GCL_TEMP}
			${Do}
				IntOp ${GCL_INDEX} ${GCL_INDEX} + 1
				ClearErrors
				!insertmacro 'GroupCheckList_InsertSectionHelper' ${GCL_INDEX}
				${If} ${Errors}
					${Break}
				${EndIf}
			${Loop}
		${Else}
			!insertmacro 'GroupCheckList_InsertSectionHelper' ${GCL_INDEX}
		${EndIf}
		Pop ${GCL_FLAGS}
		!undef GCL_FLAGS

		SendMessage ${GCL_CTLHWND} ${WM_SETREDRAW} 1 0
	${EndIf}

	Pop ${GCL_LISTTOP} 
	!undef GCL_LISTTOP
	!undef GCL_LISTHEIGHT

	nsDialogs::Show ; not return until the user clicks Next, Back or Cancel.

	${If} ${GCL_CTLHWND} != ""
		SendMessage ${GCL_CTLHWND} ${WM_GETFONT} 0 0 ${GCL_TEMP}
		System::Call 'gdi32::DeleteObject(i ${GCL_TEMP}) i.s'
		Pop ${GCL_TEMP}
	${EndIf}

	Pop ${GCL_TEMP}
	Pop ${GCL_CTLHWND}

	Pop ${GCL_NOTEFUNC}
	Pop ${GCL_DIALOG}
	Pop ${GCL_INDEX}
	Pop ${CGL_HEADER}
	Pop ${CGL_FOOTER}
	Pop ${GCL_STYLE}
	Pop ${GCL_DESCFUNC}

	!undef GCL_DIALOG
	!undef GCL_INDEX
	!undef CGL_HEADER
	!undef CGL_FOOTER
	!undef GCL_STYLE
	!undef GCL_DESCFUNC
	!undef GCL_NOTEFUNC
	!undef GCL_CTLHWND
	!undef GCL_TEMP
	Return

	!ifndef GROUPCHECKLIST_ONCHECKCHNAGED_${GCL_SUFFIX}_DEFINED
		!define GROUPCHECKLIST_ONCHECKCHNAGED_${GCL_SUFFIX}_DEFINED
		.GroupCheckList_OnCheckChanged${GCL_SUFFIX}:
			!insertmacro 'GroupCheckList_OnCheckChanged'
			Return
	!endif

	!ifndef GROUPCHECKLIST_DEFAULTDESCRIPTIONPROVIDER_${GCL_SUFFIX}_DEFINED
		!define GROUPCHECKLIST_DEFAULTDESCRIPTIONPROVIDER_${GCL_SUFFIX}_DEFINED
		.GroupCheckList_DefaultDescriptionProvider${GCL_SUFFIX}:
			!insertmacro 'GroupCheckList_DefaultDescriptionProvider'
			Return
	!endif

	!ifndef GROUPCHECKLIST_DEFAULTNOTEPROVIDER_${GCL_SUFFIX}_DEFINED
		!define GROUPCHECKLIST_DEFAULTNOTEPROVIDER_${GCL_SUFFIX}_DEFINED
		.GroupCheckList_DefaultNoteProvider${GCL_SUFFIX}:
			!insertmacro 'GroupCheckList_DefaultNoteProvider'
			Return
	!endif
	!undef GCL_SUFFIX
!macroend

!macro GroupCheckList_OnCheckChanged
	Exch $0		; hwnd
	Exch 1	
	Exch $1		; item index
	Exch 2
	Exch $2		; checked
	Exch 3
	Exch $3		; item param
	Push $4

	SectionGetFlags $3 $4
	${If} $2 != 1
		IntOp $2 ${SF_SELECTED} ~
		IntOp $4 $4 & $2
		StrCpy $2 0
	${Else}
		IntOp $4 $4  | ${SF_SELECTED}
	${EndIf}
	SectionSetFlags $3 $4

	Pop $4
	Pop $3
	Pop $0
	Pop $1
	Pop $2
!macroend

!macro GroupCheckList_DefaultDescriptionProvider
	!ifndef __UNINSTALL__
		Call GetSectionDescription
	!else
		Call un.GetSectionDescription
	!endif
!macroend

!macro GroupCheckList_DefaultNoteProvider
	Exch $0
	Push $1
	ClearErrors
	SectionGetSize $0 $1
	${If} ${Errors}
		StrCpy $1 ""
	${Else}
		StrCpy $1 "($1)"
	${EndIf}
	Exch $1
	Exch 1
	Pop $0
!macroend

; if header or footer == [groupName] then it will be replaced with group name.
; if style == "" default style used
; if provider == "default" - default provider used
!macro GroupCheckList_CreatePage __groupIndex __header __footer __style __descriptionProvider __noteProvider
	Push "${__noteProvider}"
	Push "${__descriptionProvider}"
	Push "${__style}"
	Push "${__footer}"
	Push "${__header}"
	Push "${__groupIndex}"
	${CallArtificialFunction} GroupCheckList_CreatePageInternal
!macroend

!define GroupCheckList_CreatePage `!insertmacro GroupCheckList_CreatePage`

!endif ;NULLSOFT_WINAMP_PAGE_SECTIONGROUPLIST_HEADER