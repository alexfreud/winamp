!ifndef NULLSOFT_WINAMP_INSTALLER_EXPRESS_MODE_PAGE_HEADER
!define NULLSOFT_WINAMP_INSTALLER_EXPRESS_MODE_PAGE_HEADER

!include "LogicLib.nsh"
!include "express_mode.nsh"
!include "nx.nsh"
!include "fileFunc.nsh"

!ifdef EXPRESS_MODE

Var expressMode.page.radiobuttonFont
Var expressMode.page.dialog
Var expressMode.page.descriptionText
Var expressMode.page.standardRadiobutton
Var expressMode.page.standardText
Var expressMode.page.customRadiobutton
Var expressMode.page.customText

!macro ExpressMode_InitializePage
	StrCpy $expressMode.page.radiobuttonFont 0

	StrCpy $expressMode.page.dialog 0
	StrCpy $expressMode.page.descriptionText 0
	StrCpy $expressMode.page.standardRadiobutton 0
	StrCpy $expressMode.page.standardText 0
	StrCpy $expressMode.page.customRadiobutton 0
	StrCpy $expressMode.page.customText 0
!macroend

!define ExpressMode_InitializePage "!insertmacro 'ExpressMode_InitializePage'"

!macro ExpressMode_InsertPage
	PageEx custom
		PageCallbacks ExpressMode_OnPageCreate ExpressMode_OnPageLeave
	PageExEnd
!macroend

!define ExpressMode_InsertPage "!insertmacro 'ExpressMode_InsertPage'"

Function ExpressMode_OnPageCreate
	${If} ${AtLeastWinVista}
		!insertmacro MUI_HEADER_TEXT $(IDS_EXPRESS_MODE_HEADER) $(IDS_EXPRESS_MODE_SUBHEADER)

		nsDialogs::Create  /NOUNLOAD 1018
		Pop $expressMode.page.dialog

		${If} $expressMode.page.dialog == error
			Abort
			Return
		${EndIf}

		nsDialogs::SetRTL $(^RTL)

		Push $R0
		Push $R1

		StrCpy $R0 0 ;next avaliable top position

		; Create Radio Button Font
		${If} $expressMode.page.radiobuttonFont = 0
			IntOp $0 $(^FontSize) + 0
			CreateFont $expressMode.page.radiobuttonFont $(^Font) $0 600
		${EndIf}

		; Calculate Radio Button Text Offset
		${NX_ConvertHorzDLU} $expressMode.page.dialog 12 $R2
		IntOp $R2 $R2 + 1

		; Description Text
		StrCpy $0 ""
		${If} $0 != ""
			${NSD_CreateLabel} 0u $R0 100% 20u $0
			Pop $expressMode.page.descriptionText
			${NX_GetWindowSize} $expressMode.page.descriptionText $0 $1
			${NX_GetLabelIdealSize} $expressMode.page.descriptionText $0 $2 $3
			${If} $3 <= $1
				${NX_SetWindowSize} $expressMode.page.descriptionText $2 $3
				IntOp $R0 $R0 + $3
			${Else}
				IntOp $R0 $R0 + $1
			${EndIf}
		${EndIf}

		StrCpy $R1 $R0

		; Standard Installation Radio Button
		${NSD_CreateRadioButton} 0u $R0 100% 10u "$(IDS_EXPRESS_MODE_STANDARD_INSTALL_RADIO)"
		Pop $expressMode.page.standardRadiobutton
		${If} $expressMode.page.standardRadiobutton <> 0
			${NSD_AddStyle} $expressMode.page.standardRadiobutton ${WS_GROUP}
			${NSD_OnClick} $expressMode.page.standardRadiobutton \
						   ExpressMode_OnStandardRadiobuttonClick
			${If} $expressMode.page.radiobuttonFont <> 0
				SendMessage $expressMode.page.standardRadiobutton \
							${WM_SETFONT} \
							$expressMode.page.radiobuttonFont 0 
			${EndIf}
			${NX_GetWindowSize} $expressMode.page.standardRadiobutton $0 $1
			IntOp $0 $0 - $R2
			${NSD_GetText} $expressMode.page.standardRadiobutton $4
			${NX_CalculateTextRect} $expressMode.page.standardRadiobutton \
									$4 ${DT_WORDBREAK} \
									$0 $2 $3
			${If} $3 < $1
					StrCpy $3 $1
			${EndIf}
			IntOp $2 $2 + $R2
			${NX_SetWindowSize} $expressMode.page.standardRadiobutton $2 $3
			IntOp $R0 $R0 + $3

			; Standard Installation Text
			${NX_ConvertVertDLU} $expressMode.page.dialog 3 $3
			IntOp $R0 $R0 + $3

			${IfNot} ${FileExists} "$INSTDIR\install.ini"
				; we don't have previous selection
				StrCpy $0 "$(IDS_EXPRESS_MODE_STANDARD_INSTALL_TEXT)"
			${Else}
				StrCpy $0 "$(IDS_EXPRESS_MODE_STANDARD_REINSTALL_TEXT)"
			${EndIf}

			${NSD_CreateLabel} 11u $R0 289u 30u $0
			Pop $expressMode.page.standardText
			${If} $expressMode.page.standardText <> 0
				${NSD_AddStyle} $expressMode.page.standardText ${WS_GROUP}
				${NX_GetWindowSize} $expressMode.page.standardText $0 $1
				${NX_GetLabelIdealSize} $expressMode.page.standardText $0 $2 $3
				${If} $3 > $1
					StrCpy $3 $1
				${EndIf}
				${NX_SetWindowSize} $expressMode.page.standardText $2 $3
				IntOp $R0 $R0 + $3
			${Else}
				IntOp $R0 $R0 - $3
			${EndIf}

			${NX_ConvertVertDLU} $expressMode.page.dialog 12 $0
			IntOp $R0 $R0 + $0
		${EndIf}

		; Custom Installation Radio Button
		${NSD_CreateRadioButton} 0u $R0 100% 10u "$(IDS_EXPRESS_MODE_CUSTOM_INSTALL_RADIO)"
		Pop $expressMode.page.customRadiobutton
		${If} $expressMode.page.customRadiobutton <> 0
			${NSD_OnClick} $expressMode.page.customRadiobutton \
						   ExpressMode_OnCustomRadiobuttonClick

			${If} $expressMode.page.radiobuttonFont <> 0
				SendMessage $expressMode.page.customRadiobutton \
							${WM_SETFONT} \
							$expressMode.page.radiobuttonFont 0 
			${EndIf}

			${NX_GetWindowSize} $expressMode.page.customRadiobutton $0 $1
			IntOp $0 $0 - $R2
			${NSD_GetText} $expressMode.page.customRadiobutton $4
			${NX_CalculateTextRect} $expressMode.page.customRadiobutton \
									$4 ${DT_WORDBREAK} \
									$0 $2 $3
			${If} $3 < $1
				StrCpy $3 $1
			${EndIf}
			IntOp $2 $2 + $R2
			${NX_SetWindowSize} $expressMode.page.customRadiobutton $2 $3
			IntOp $R0 $R0 + $3

			${If} $expressMode.page.standardText <> 0
				${NX_SetWindowOrder} $expressMode.page.standardText \
									 $expressMode.page.customRadiobutton
			${EndIf}

			; Custom Installation Text
			${NX_ConvertVertDLU} $expressMode.page.dialog 3 $3
			IntOp $R0 $R0 + $3

			${NSD_CreateLabel} 11u $R0 289u 30u "$(IDS_EXPRESS_MODE_CUSTOM_INSTALL_TEXT)"
			Pop $expressMode.page.customText
			${If} $expressMode.page.customText <> 0
				${NX_GetWindowSize} $expressMode.page.customText $0 $1
				${NX_GetLabelIdealSize} $expressMode.page.customText $0 $2 $3
				${If} $3 > $1
					StrCpy $3 $1
				${EndIf}
				${NX_SetWindowSize} $expressMode.page.customText $2 $3
				IntOp $R0 $R0 + $3
			${Else}
				IntOp $R0 $R0 - $3
			${EndIf}
		${EndIf}

		${NX_GetWindowSize} $expressMode.page.dialog $1 $0
		IntOp $0 $0 - $R1
		IntOp $1 $R0 - $R1
		${If} $0 > $1
			IntOp $0 $0 - $1
			IntOp $0 $0 / 3
			${If} $expressMode.page.standardRadiobutton <> 0
				${NX_OffsetWindowPos} $expressMode.page.standardRadiobutton 0 $0
			${EndIf}
			${If} $expressMode.page.standardText <> 0
				${NX_OffsetWindowPos} $expressMode.page.standardText 0 $0
			${EndIf}
			${If} $expressMode.page.customRadiobutton <> 0
				${NX_OffsetWindowPos} $expressMode.page.customRadiobutton 0 $0
			${EndIf}
			${If} $expressMode.page.customText <> 0
				${NX_OffsetWindowPos} $expressMode.page.customText 0 $0
			${EndIf}
		${EndIf}

		Pop $R1
		Pop $R0

		Call ExpressMode_UpdatePage

		; Show Dialog
		nsDialogs::Show ; not return until the user clicks Next, Back or Cancel.
	${EndIf}
	Return
FunctionEnd

Function ExpressMode_UpdatePage
	Push $0
	${ExpressMode_IsEnabled} $0
	${If} $0 == "yes"
		${NSD_Check} $expressMode.page.standardRadiobutton
		${NSD_Uncheck} $expressMode.page.customRadiobutton
		${NextButton_SetNextPageMode} $0
	${Else}
		${NSD_Uncheck} $expressMode.page.standardRadiobutton
		${NSD_Check} $expressMode.page.customRadiobutton
		${NextButton_SetLastPageMode} "no"
	${EndIf}
	Pop $0
FunctionEnd

!define ExpressMode_UpdatePage 'call ExpressMode_UpdatePage'

Function ExpressMode_OnStandardRadiobuttonClick
	Exch $0
	${NSD_GetState} $expressMode.page.standardRadiobutton $0
	${If} $0 = ${BST_CHECKED}
		${ExpressMode_IsEnabled} $0
		${If} $0 != "yes"
			${ExpressMode_Enable}
			${ExpressMode_UpdatePage}
		${EndIf}
	${Else}
		${ExpressMode_IsEnabled} $0
		${If} $0 == "yes"
			${ExpressMode_Disable}
			${ExpressMode_UpdatePage}
		${EndIf}
	${EndIf}
	Pop $0
FunctionEnd

Function ExpressMode_OnCustomRadiobuttonClick
	Exch $0
	${NSD_GetState} $expressMode.page.customRadiobutton $0
	${If} $0 = ${BST_CHECKED}
		${ExpressMode_IsEnabled} $0
		${If} $0 == "yes"
			${ExpressMode_Disable}
			${ExpressMode_UpdatePage}
		${EndIf}
	${Else}
		${ExpressMode_IsEnabled} $0
		${If} $0 != "yes"
			${ExpressMode_Enable}
			${ExpressMode_UpdatePage}
		${EndIf}
	${EndIf}
	Pop $0
FunctionEnd

Function ExpressMode_OnPageLeave
	${If} $expressMode.page.radiobuttonFont <> 0
		System::Call "${fnDeleteObject}($expressMode.page.radiobuttonFont)"
		StrCpy $expressMode.page.radiobuttonFont 0
	${EndIf}

	StrCpy $expressMode.page.dialog 0
	StrCpy $expressMode.page.descriptionText 0
	StrCpy $expressMode.page.standardRadiobutton 0
	StrCpy $expressMode.page.standardText 0
	StrCpy $expressMode.page.customRadiobutton 0
	StrCpy $expressMode.page.customText 0
FunctionEnd
!else ; defined(EXPRESS_MODE)
!define ExpressMode_InitializePage ""
!define ExpressMode_InsertPage ""
!endif ; defined(EXPRESS_MODE)
!endif ; defined(NULLSOFT_WINAMP_INSTALLER_EXPRESS_MODE_PAGE_HEADER)