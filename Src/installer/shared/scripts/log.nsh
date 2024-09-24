!ifndef NULLSOFT_NX_LOG_NSIS_HEADER
!define NULLSOFT_NX_LOG_NSIS_HEADER

!macro NX_Log __string
	System::Call "Kernel32::OutputDebugStringW(t s)" `${__string}`
!macroend

!define NX_Log "!insertmacro NX_Log"

!macro NX_DLog __string
	!ifdef _DEBUG
		${NX_Log} `${__string}`
	!endif
!macroend

!define NX_DLog "!insertmacro NX_DLog"

!endif ;NULLSOFT_NX_LOG_NSIS_HEADER