!ifndef NULLSOFT_NX_WINDOW_RECT_NSIS_HEADER
!define NULLSOFT_NX_WINDOW_RECT_NSIS_HEADER

!include "util.nsh"
!include "logicLib.nsh"
!include "system.nsh"

!macro NX_GetWindowRectInternal
	Exch $0

	Push $2
	Push $3
	Push $4
	Push $5

	Push $1
	System::Call "*${stRECT} .r1"

	System::Call "${fnGetWindowRect}($0, $1)"
	System::Call "*$1${stRECT} (.r2, .r3, .r4, .r5)"
	System::Free $1
	Pop $1

	Exch 4
	Pop $0

	Exch $5
	Exch 3
	Exch $2
	Exch 
	Exch $4
	Exch 2
	Exch $3
	Exch
!macroend

!macro NX_GetWindowRect __hwnd __left __top __right __bottom
	Push "${__hwnd}"
	${CallArtificialFunction} NX_GetWindowRectInternal
	Pop "${__left}"
	Pop "${__top}"
	Pop "${__right}"
	Pop "${__bottom}"
!macroend

!define NX_GetWindowRect `!insertmacro NX_GetWindowRect`

!macro NX_GetMappedWindowRectInternal
	Exch $0
	Exch 
	Exch $2

	Push $3
	Push $4
	Push $5

	Push $1
	System::Call "*${stRECT} .r1"

	System::Call "${fnGetWindowRect}(r0, r1)"
	System::Call '${fnMapWindowPoints}(0, r2, r1, 2)'
	System::Call "*$1${stRECT} (.r5, .r2, .r3, .r4)"
	System::Free $1
	Pop $1

	Exch 4
	Pop $0

	Exch $4
	Exch 3
	Exch $5
	Exch 
	Exch $3
	Exch 2
	Exch $2
	Exch
!macroend

!macro NX_GetMappedWindowRect __hwnd __hwndParent __left __top __right __bottom
	Push "${__hwndParent}"
	Push "${__hwnd}"
	${CallArtificialFunction} NX_GetMappedWindowRectInternal
	Pop "${__left}"
	Pop "${__top}"
	Pop "${__right}"
	Pop "${__bottom}"
!macroend

!define NX_GetMappedWindowRect `!insertmacro NX_GetMappedWindowRect`

!macro NX_GetWindowSizeInternal
	Exch $0

	Push $2
	Push $3
	Push $4
	Push $5

	Push $1
	System::Call "*${stRECT} .r1"

	System::Call "${fnGetWindowRect} ($0, $1)"
	System::Call "*$1${stRECT} (.r2, .r3, .r4, .r5)"
	System::Free $1
	Pop $1

	IntOp $0 $4 - $2
	IntOp $2 $5 - $3

	Pop $5
	Pop $4
	Pop $3
	Exch $2
	Exch
	Exch $0
!macroend

!macro NX_GetWindowSize __hwnd __width __height
	Push "${__hwnd}"
	${CallArtificialFunction} NX_GetWindowSizeInternal
	Pop "${__width}"
	Pop "${__height}"
!macroend

!define NX_GetWindowSize `!insertmacro NX_GetWindowSize`

!macro NX_GetWindowPosInternal
	Exch $0

	Push $2
	Push $3
	Push $4
	Push $5

	Push $1
	System::Call "*${stRECT} .r1"

	System::Call "${fnGetWindowRect}($0, $1)"
	System::Call "*$1${stRECT} (.r2, .r3, .r4, .r5)"
	System::Free $1
	Pop $1

	StrCpy $0 $3

	Pop $5
	Pop $4
	Pop $3
	Exch $2
	Exch
	Exch $0
	Exch
!macroend

!macro NX_GetWindowPos __hwnd __left __top
	Push "${__hwnd}"
	${CallArtificialFunction} NX_GetWindowPosInternal
	Pop "${__left}"
	Pop "${__top}"
!macroend
!define NX_GetWindowPos `!insertmacro NX_GetWindowPos`

!macro NX_GetMappedWindowPosInternal
	Exch $0
	Exch 
	Exch $2

	Push $3
	Push $4
	Push $5

	Push $1
	System::Call "*${stRECT} .r1"

	System::Call "${fnGetWindowRect}($0, $1)"
	System::Call "${fnMapWindowPoints}(0, $2, r1, 1)"
	System::Call "*$1${stRECT} (.r2, .r3, .r4, .r5)"
	System::Free $1
	Pop $1

	StrCpy $0 $3

	Pop $5
	Pop $4
	Pop $3
	Exch $2
	Exch
	Exch $0
	Exch
!macroend

!macro NX_GetMappedWindowPos __hwnd __hwndParent __left __top
	Push "${__hwndParent}"
	Push "${__hwnd}"
	${CallArtificialFunction} NX_GetMappedWindowPosInternal
	Pop "${__left}"
	Pop "${__top}"
!macroend

!define NX_GetMappedWindowPos `!insertmacro NX_GetMappedWindowPos`

!macro NX_SetWindowPos __hwnd __left __top
	System::Call "${fnSetWindowPos} \
				 (${__hwnd}, 0, ${__left}, ${__top}, 0, 0, \
				 ${SWP_NOACTIVATE}|${SWP_NOZORDER}|${SWP_NOSIZE})"
!macroend

!define NX_SetWindowPos `!insertmacro NX_SetWindowPos`

!macro NX_SetWindowSize __hwnd __width __height
	System::Call "${fnSetWindowPos} \
				 (${__hwnd}, 0, 0, 0, ${__width}, ${__height}, \
				 ${SWP_NOACTIVATE}|${SWP_NOZORDER}|${SWP_NOMOVE})"
!macroend

!define NX_SetWindowSize `!insertmacro NX_SetWindowSize`

!macro NX_SetWindowPosAndSize __hwnd __left __top __width __height
	System::Call "User32::SetWindowPos(i, i, i, i, i, i, i) b \
				 (${__hwnd}, 0, ${__left}, ${__top}, ${__width}, ${__height},\
				 ${SWP_NOACTIVATE}|${SWP_NOZORDER})"
!macroend

!define NX_SetWindowPosAndSize `!insertmacro NX_SetWindowPosAndSize`

!macro NX_SetWindowOrder __hwnd __insertAfterWindow
	System::Call "${fnSetWindowPos} \
				 (${__hwnd}, ${__insertAfterWindow}, 0, 0, 0, 0, \
				 ${SWP_NOACTIVATE}|${SWP_NOSIZE}|${SWP_NOMOVE})"
!macroend

!define NX_SetWindowOrder `!insertmacro NX_SetWindowOrder`

!macro NX_OffsetWindowPosInternal
	!define hwnd_ $R0
	!define offsetX_ $R1
	!define offsetY_ $R2

	Exch ${offsetY_}
	Exch 
	Exch ${offsetX_}
	Exch 2
	Exch ${hwnd_}

	Push $0
	Push $1
	Push $2
	Push $3
	Push $4

	System::Call "*${stRECT} .r4"
	System::Call "${fnGetWindowRect} (${hwnd_}, r4)"
	System::Call "${fnGetAncestor} (${hwnd_}, ${GA_PARENT}).r0"
	${If} $R0 != 0
		System::Call "${fnMapWindowPoints} (0, r0, r4, 2)"
	${EndIf}
	System::Call "*$4${stRECT} (.r0, .r1, .r2, .r3)"
	System::Free $4

	IntOp $0 $0 + ${offsetX_}
	IntOp $1 $1 + ${offsetY_}

	System::Call "${fnSetWindowPos} \
				(${hwnd_}, 0,  $0, $1, 0, 0, \
				${SWP_NOACTIVATE}|${SWP_NOZORDER}|${SWP_NOSIZE})"
	Pop $4
	Pop $3
	Pop $2
	Pop $1
	Pop $0

	Pop ${hwnd_}
	Pop ${offsetY_}
	Pop ${offsetX_}

	!undef hwnd_
	!undef offsetX_
	!undef offsetY_
!macroend

!macro NX_OffsetWindowPos __hwnd __offset_x __offset_y
	Push "${__hwnd}"
	Push "${__offset_x}"
	Push "${__offset_y}"
	${CallArtificialFunction} NX_OffsetWindowPosInternal
!macroend

!define NX_OffsetWindowPos "!insertmacro 'NX_OffsetWindowPos'"

!macro NX_IncreaseWindowSizeInternal
	!define hwnd_ $R0
	!define deltaCX_ $R1
	!define deltaCY_ $R2

	Exch ${deltaCY_}
	Exch 
	Exch ${deltaCX_}
	Exch 2
	Exch ${hwnd_}

	Push $0
	Push $1
	Push $2
	Push $3
	Push $4

	System::Call "*${stRECT} .r4"
	System::Call "${fnGetWindowRect}(${hwnd_}, r4)"
	System::Call "*$4${stRECT} (.r0, .r1, .r2, .r3)"
	System::Free $4

	IntOp $0 $2 - $0
	IntOp $1 $3 - $1
	IntOp $0 $0 + ${deltaCX_}
	IntOp $1 $1 + ${deltaCY_}

	System::Call "${fnSetWindowPos}\
				(${__hwnd}, 0, 0, 0, $0, $1, \
				${SWP_NOACTIVATE}|${SWP_NOZORDER}|${SWP_NOMOVE})"
	Pop $4
	Pop $3
	Pop $2
	Pop $1
	Pop $0

	Pop ${hwnd_}
	Pop ${deltaCY_}
	Pop ${deltaCX_}

	!undef hwnd_
	!undef deltaCX_
	!undef deltaCY_
!macroend

!macro NX_IncreaseWindowSize __hwnd __delta_cx __delta_cy
	Push "${__hwnd}"
	Push "${__delta_cx}"
	Push "${__delta_cy}"
	${CallArtificialFunction} NX_IncreaseWindowSizeInternal
!macroend

!define NX_IncreaseWindowSize "!insertmacro 'NX_IncreaseWindowSize'"

!macro NX_ConvertHorzDLUInternal
	Exch $1 ; dlu
	Exch
	Exch $0 ; hwnd
	Push $2

	System::Call "*${stRECT}($1, 0, 0, 0) .r2"
	System::Call "${fnMapDialogRect}($0, $2).s"
	Pop $1
	${If} $1 != 0
		System::Call "*$2${stRECT} (.r1, _)"
	${EndIf}

	System::Free $2

	Pop $2
	Pop $0
	Exch $1
!macroend

!macro NX_ConvertHorzDLU __hwnd __dlu __px
	Push "${__hwnd}"
	Push "${__dlu}"
	${CallArtificialFunction} NX_ConvertHorzDLUInternal
	Pop "${__px}"
!macroend

!define NX_ConvertHorzDLU `!insertmacro NX_ConvertHorzDLU`

!macro NX_ConvertVertDLUInternal
	Exch $1 ; dlu
	Exch
	Exch $0 ; hwnd
	Push $2

	System::Call "*${stRECT}(0, $1, 0, 0) .r2"
	System::Call "${fnMapDialogRect}($0, $2).s"
	Pop $1
	${If} $1 != 0
		System::Call "*$2${stRECT} (., .r1, _)"
	${EndIf}

	System::Free $2

	Pop $2
	Pop $0
	Exch $1
!macroend

!macro NX_ConvertVertDLU __hwnd __dlu __px
	Push "${__hwnd}"
	Push "${__dlu}"
	${CallArtificialFunction} NX_ConvertVertDLUInternal
	Pop "${__px}"
!macroend

!define NX_ConvertVertDLU `!insertmacro NX_ConvertVertDLU`

!endif ; defined(NULLSOFT_NX_WINDOW_RECT_NSIS_HEADER)