!ifndef NULLSOFT_NX_TEXT_RECT_NSIS_HEADER
!define NULLSOFT_NX_TEXT_RECT_NSIS_HEADER

!include "util.nsh"
!include "logicLib.nsh"
!include "system.nsh"

!macro NX_CalculateTextRectInternal
	!define hwnd_ $R0
	!define text_ $R1
	!define style_ $R2
	!define maxWidth_ $R3
	!define hdc_ $R4
	!define font_ $R5
	!define previousFont_ $R6
	!define width_ $R7
	!define height_ $R8
	!define averageCharWidth_ $R9

	Exch ${hwnd_}
	Exch 1
	Exch ${text_}
	Exch 2
	Exch ${style_}
	Exch 3
	Exch ${maxWidth_}

	Push ${hdc_}
	Push ${font_}
	Push ${previousFont_}
	Push ${width_}
	Push ${height_}
	Push ${averageCharWidth_}

	StrCpy ${width_} 0
	StrCpy ${height_} 0
	StrCpy ${averageCharWidth_} 0

	System::Call "${fnGetDCEx}(${hwnd_}, 0, ${DCX_CACHE}|${DCX_NORESETATTRS}).s"
	Pop ${hdc_}

	SendMessage ${hwnd_} ${WM_GETFONT} 0 0 ${font_}
	System::Call "${fnSelectObject}(${hdc_}, ${font_}).s"
	Pop ${previousFont_}

	; Get Text Metrics (in case of empty line)
	Push $0
	Push $1
	System::Call "*${stTEXTMETRIC} .r1"
	System::Call "${fnGetTextMetrics}(${hdc_}, $1).r0"
	${If} $0 != 0
		Push $2
		System::Call "*$1${stTEXTMETRIC}(.r0, ., ., ., ., .r2, _)"
		StrCpy ${height_} $0
		StrCpy ${averageCharWidth_} $2
		Pop $2
	${EndIf}

	System::Free $1
	Pop $1
	Pop $0

	; Calculate Text Rect
	Push $0
	StrLen $0 ${text_}
	${If} $0 != 0
		Push $1 ; rect
		Push $2 ; text flags

		; build text flags
		StrCpy $2 ${DT_CALCRECT}|${DT_LEFT}|${DT_TOP}
		IntOp $2 $2 | ${style_}

		System::Call "*${stRECT}(0, 0, ${maxWidth_}, $0) .r1"
		System::Call "${fnDrawText}(${hdc_}, '${text_}', -1, $1, $2).r0"
		${if} $0 != 0
			Push $3
			Push $4
			Push $5

			System::Call "*$1${stRECT}(.r2, .r3, .r4, .r5)"
			IntOp ${width_} $4 - $2
			IntOp $3 $5 - $3
			${if} $3 > ${height_}
				StrCpy ${height_} $3
			${EndIf}

			IntOp $4 ${averageCharWidth_} / 2
			IntOp ${width_} ${width_} + $4

			Pop $5
			Pop $4
			Pop $3
		${EndIf}

		System::Free $1
		Pop $2 ; text flags
		Pop $1 ; rect
	${EndIf}
	Pop $0

	${If} ${previousFont_} != 0
		System::Call "${fnSelectObject}(${hdc_}, ${previousFont_})"
	${EndIf}

	System::Call "${fnReleaseDC}(${hwnd_}, ${hdc_})"

	StrCpy ${style_} ${width_}
	StrCpy ${text_} ${height_}

	Pop ${averageCharWidth_}
	Pop ${height_}
	Pop ${width_}
	Pop ${previousFont_}
	Pop ${font_}
	Pop ${hdc_}
	Pop ${maxWidth_}
	Pop ${hwnd_}
	Exch ${text_}
	Exch
	Exch ${style_}

	!undef hwnd_
	!undef text_
	!undef style_
	!undef maxWidth_
	!undef hdc_
	!undef font_
	!undef previousFont_
	!undef width_
	!undef height_
	!undef averageCharWidth_
!macroend

!macro NX_CalculateTextRect __hwnd __text __style __maxWidth __width __height
	Push "${__maxWidth}"
	Push "${__style}"
	Push "${__text}"
	Push "${__hwnd}"
	${CallArtificialFunction} NX_CalculateTextRectInternal
	Pop "${__width}"
	Pop "${__height}"
!macroend

!define NX_CalculateTextRect `!insertmacro NX_CalculateTextRect`

!macro NX_GetLabelIdealSize __hwnd __maxWidth __width __height
	Push "${__maxWidth}"

	System::Call "${fnGetWindowLong}(${__hwnd}, ${GWL_STYLE}).s"
	Exch $0	
	Push $1
	Push $2

	StrCpy $2 0
	IntOp $1 $0 & ${SS_EDITCONTROL}
	${If} $1 != 0
		IntOp $2 $2 | ${DT_EDITCONTROL}
	${EndIf}

	IntOp $1 $0 & ${SS_NOPREFIX}
	${If} $1 != 0
		IntOp $2 $2 | ${DT_NOPREFIX}
	${EndIf}

	IntOp $1 $0 & ${SS_SIMPLE}
	${If} $1 != 0
		IntOp $2 $2 | ${DT_SINGLELINE}
	${Else}
		IntOp $2 $2 | ${DT_WORDBREAK}
	${EndIf}

	StrCpy $0 $2
	Pop $2
	Pop $1
	Exch $0

	System::Call "${fnGetWindowText}(${__hwnd}, .s, ${NSIS_MAX_STRLEN})"
	Push "${__hwnd}"
	${CallArtificialFunction} NX_CalculateTextRectInternal
	Pop "${__width}"
	Pop "${__height}"
!macroend
!define NX_GetLabelIdealSize `!insertmacro NX_GetLabelIdealSize`

!endif ; defined(NULLSOFT_NX_TEXT_RECT_NSIS_HEADER)