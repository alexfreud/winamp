!ifndef NULLSOFT_WINAMP_INSALLER_FRENCHRADIO_HEADER
!define NULLSOFT_WINAMP_INSALLER_FRENCHRADIO_HEADER

!define FRENCHRADIO_PLUGIN_ENABLED

!ifdef FRENCHRADIO_PLUGIN_ENABLED
!ifdef "LANG_USE_FR-FR" | "LANG_USE_FR-FR-CMTY" ;"LANG_USE_ALL"
!ifdef FULL

	!define FRENCHRADIO_PLUGIN

	!macro FrenchRadio_OnInit
		SectionSetSize ${IDX_SEC_GEN_FRENCHRADIO_DEPENDENCIES} 0
	!macroend
	!define FrenchRadio_OnInit "!insertmacro 'FrenchRadio_OnInit'"

	!macro FrenchRadio_InsertInstallSections

		!define FRENCH_RADIO_SOURCE_BASE		"..\..\resources\plugins\French Radio"

		!ifndef FRENCH_RADIO_SOURCE
			!define FRENCH_RADIO_SOURCE				""
		!endif

		Section "-FrenchRadioDependencies" IDX_SEC_GEN_FRENCHRADIO_DEPENDENCIES
			DetailPrint "$(IDS_FRENCHRADIO_INSTALLING)"
			SetDetailsPrint none

			Call FrenchRadio_IsSectionSelected
			Exch $0

			${if} $0 != ""
				SetOutPath "$INSTDIR\Microsoft.VC90.CRT"
				File ..\..\resources\libraries\msvcr90.dll
				File ..\..\resources\libraries\msvcp90.dll
				File ..\..\resources\libraries\Microsoft.VC90.CRT.manifest
				SetOutPath "$INSTDIR\Plugins\Microsoft.VC90.ATL"
				File ..\..\resources\libraries\atl90.dll
				File ..\..\resources\libraries\Microsoft.VC90.ATL.manifest
				SetOutPath "$INSTDIR"
			${Endif}
			Pop $0
			SetDetailsPrint lastused
		SectionEnd

		${WinampSection} "FrenchRadio" $(IDS_SEC_GEN_FRENCHRADIO)  IDX_SEC_GEN_FRENCHRADIO  ; >>> [French Radio plugin]

			${SECTIONIN_FULL}
			DetailPrint "$(IDS_FRENCHRADIO_INSTALLING)"
			SetDetailsPrint none

			SetOverwrite try

			;artwork
			SetOutPath "$INSTDIR\Plugins\Todae\LMPwa\img"
			!undef FRENCH_RADIO_SOURCE
			!define FRENCH_RADIO_SOURCE		"${FRENCH_RADIO_SOURCE_BASE}\Todae\LMPwa\img"
			File "${FRENCH_RADIO_SOURCE}\icon_add.png"
			File "${FRENCH_RADIO_SOURCE}\icon_edit.png"
			File "${FRENCH_RADIO_SOURCE}\icon_fav.png"
			File "${FRENCH_RADIO_SOURCE}\icon_fb.png"
			File "${FRENCH_RADIO_SOURCE}\icon_info.png"
			File "${FRENCH_RADIO_SOURCE}\icon_thumb.png"
			File "${FRENCH_RADIO_SOURCE}\play.png"
			File "${FRENCH_RADIO_SOURCE}\play_on.png"
			File "${FRENCH_RADIO_SOURCE}\play_on_small.png"
			File "${FRENCH_RADIO_SOURCE}\star.png"
			File "${FRENCH_RADIO_SOURCE}\starblack.png"
			File "${FRENCH_RADIO_SOURCE}\starno.png"
			File "${FRENCH_RADIO_SOURCE}\starno2.png"
			File "${FRENCH_RADIO_SOURCE}\starover.png"
			File "${FRENCH_RADIO_SOURCE}\stop.png"
			File "${FRENCH_RADIO_SOURCE}\stop_on.png"

			;languages
			SetOutPath "$INSTDIR\Plugins\Todae\LMPwa\lang"
			!undef FRENCH_RADIO_SOURCE
			!define FRENCH_RADIO_SOURCE "${FRENCH_RADIO_SOURCE_BASE}\Todae\LMPwa\lang"
			File "${FRENCH_RADIO_SOURCE}\Arabe.ini"
			File "${FRENCH_RADIO_SOURCE}\Arabe.readme.txt"
			File "${FRENCH_RADIO_SOURCE}\Deutsch.ini"
			File "${FRENCH_RADIO_SOURCE}\Deutsch.readme.txt"
			File "${FRENCH_RADIO_SOURCE}\English.ini"
			File "${FRENCH_RADIO_SOURCE}\English.readme.txt"
			File "${FRENCH_RADIO_SOURCE}\Español.ini"
			File "${FRENCH_RADIO_SOURCE}\Español.readme.txt"
			File "${FRENCH_RADIO_SOURCE}\Français.ini"
			File "${FRENCH_RADIO_SOURCE}\Français.readme.txt"
			File "${FRENCH_RADIO_SOURCE}\Italiano.ini"
			File "${FRENCH_RADIO_SOURCE}\Italiano.readme.txt"

			; plugin
			SetOutPath "$INSTDIR\Plugins"
			!undef FRENCH_RADIO_SOURCE
			!define FRENCH_RADIO_SOURCE "${FRENCH_RADIO_SOURCE_BASE}"
			File "${FRENCH_RADIO_SOURCE}\gen_LMPwa.dll"

			; user settings
			SetOutPath "$APPDATA\Todae\LMPwa"
			!undef FRENCH_RADIO_SOURCE
			!define FRENCH_RADIO_SOURCE "${FRENCH_RADIO_SOURCE_BASE}\UserData"
			File "${FRENCH_RADIO_SOURCE}\lmpv3.xml"
			File "${FRENCH_RADIO_SOURCE}\lmpv3_categories.xml"
			File "${FRENCH_RADIO_SOURCE}\LMP_config.ini"
			File "${FRENCH_RADIO_SOURCE}\LMP_default.ini"
			File "${FRENCH_RADIO_SOURCE}\LMP_default.rtv"
			File "${FRENCH_RADIO_SOURCE}\LMP_default_tmp.ini"

			SetOutPath "$APPDATA\Todae\LMPwa\256x256"
			!undef FRENCH_RADIO_SOURCE
			!define FRENCH_RADIO_SOURCE "${FRENCH_RADIO_SOURCE_BASE}\UserData\256x256"
			File "${FRENCH_RADIO_SOURCE}\*.png"

			SetOverwrite lastused

			SetDetailsPrint lastused
		${WinampSectionEnd}                                                  			; <<< [French Radio plugin]

		Function FrenchRadio_IsSectionSelected
			${if} ${SectionIsSelected} ${IDX_SEC_GEN_FRENCHRADIO}
				Push "true"
			${Else}
				Push ""
			${EndIf}
		FunctionEnd

		!undef FRENCH_RADIO_SOURCE
	!macroend
	!define FrenchRadio_InsertInstallSections "!insertmacro 'FrenchRadio_InsertInstallSections'"

	!macro FrenchRadio_UninstallPlugin
		RMDir /r "$INSTDIR\Plugins\Todae"
		Delete "$INSTDIR\Plugins\Todae\gen_LMPwa.dll"
		RMDir /r "$APPDATA\Todae"
	!macroend
	!define FrenchRadio_UninstallPlugin "!insertmacro 'FrenchRadio_UninstallPlugin'"

	!macro FrenchRadio_UninstallUserData
		RMDir /r "$APPDATA\Todae"
	!macroend
	!define FrenchRadio_UninstallUserData "!insertmacro 'FrenchRadio_UninstallUserData'"

!endif ; FULL
!endif ;lang scope
!endif ; FRENCHRADIO_PLUGIN_ENABLED

!ifndef FRENCHRADIO_PLUGIN
	!define FrenchRadio_OnInit
	!define FrenchRadio_InsertInstallSections
	!define FrenchRadio_UninstallPlugin
	!define FrenchRadio_UninstallUserData
!endif

!endif ;NULLSOFT_WINAMP_INSALLER_FRENCHRADIO_HEADER