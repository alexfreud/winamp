!macro CustomCodePreInstall
	IfFileExists "$INSTDIR\Data\settings\7zip_portable.reg" "" CustomCodePreInstallEnd
		ReadINIStr $0 "$INSTDIR\Data\settings\7zip_portable.reg" "HKEY_CURRENT_USER\Software\7-zip\FM" '"FolderShortcuts"'
		StrCmp $0 "" "" CustomCodePreInstallEnd
			RMDir /r "$INSTDIR\Data"

	CustomCodePreInstallEnd:
!macroend