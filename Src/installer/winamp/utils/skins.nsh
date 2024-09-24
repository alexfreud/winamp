!ifndef NULLSOFT_WINAMP_SKINS_HEADER
!define NULLSOFT_WINAMP_SKINS_HEADER

Function ${INSTALL_TYPE_PREFIX}Skins_GetPath
	Push $0
	ReadINIStr $0 "$WINAMPINI" "Winamp" "SkinDir"
	${If} $0 != ""
		Push $INSTDIR
		Push $0
		nsis_winamp::GetFullPath
		Pop $0
	${EndIf}
	Exch $0
FunctionEnd

!macro Skins_GetPath __pathOut
	Call ${INSTALL_TYPE_PREFIX}Skins_GetPath
	Pop "${__pathOut}"
!macroend

!define Skins_GetPath "!insertmacro 'Skins_GetPath'"

Function ${INSTALL_TYPE_PREFIX}Skins_GetDefaultPath
	Push $0
	StrCpy  $0 "$INSTDIR\Skins"
	Exch $0
FunctionEnd

!macro Skins_GetDefaultPath __pathOut
	Call ${INSTALL_TYPE_PREFIX}Skins_GetDefaultPath
	Pop "${__pathOut}"
!macroend

!define Skins_GetDefaultPath "!insertmacro 'Skins_GetDefaultPath'"

Function ${INSTALL_TYPE_PREFIX}Skins_DeleteFolder
	Exch $0

	Delete "$0\${MODERNSKINNAME}.wal"
	Delete "$0\${MODERNSKINNAME}.wsz"
	Delete "$0\${MODERNSKINNAME}.zip"
	RMDir /r "$0\${MODERNSKINNAME}"
	RMDir /r "$0\Bento"
	RMDir /r "$0\Big Bento"
	RMDir "$0" ; don't try to delete, prompt user later
	Pop $0
FunctionEnd

!macro Skins_DeleteFolder __skinsDir
	Push "${__skinsDir}"
	Call ${INSTALL_TYPE_PREFIX}Skins_DeleteFolder
!macroend

!define Skins_DeleteFolder "!insertmacro 'Skins_DeleteFolder'"

!endif ;NULLSOFT_WINAMP_SKINS_HEADER