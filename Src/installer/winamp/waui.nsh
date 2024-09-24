;!include "nsDialogs.nsh"

Var waui.dialog

!macro WAUI_PAGE_STARTMENU
	!include pages\waui_startmenu.nsh
	PageEx custom
		PageCallbacks nsPageWAStartMenu_Create
	PageExEnd
!macroend