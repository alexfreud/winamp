SectionGroup $(IDS_GRP_VISUALIZATION) IDX_GRP_VISUALIZATION ;  Visualization
!if 0
!ifndef WINAMP64
	${WinampSection} "visTiny" $(secTiny) IDX_SEC_NSFS        ; >>> [Nullsoft Tiny Fullscreen]
		${SECTIONIN_LITE}
		Call SetVisPluginDir
		File ${FILES_PATH}\Plugins\vis_nsfs.dll
	${WinampSectionEnd}                                       ; <<< [Nullsoft Tiny Fullscreen]
!endif ; WINAMP64
!endif

!ifdef full
!ifndef WINAMP64
	${WinampSection} "visAVS" $(secAVS) IDX_SEC_AVS          ; >>> [Advanced Visualization Studio]
		${SECTIONIN_FULL}
		Call SetVisPluginDir

		; ${If} ${IsWinXP}
		;	File ..\..\resources\Plugins\vis_avs.dll
		; ${Else}
		;	File /oname=vis_avs.dll "..\..\resources\Plugins\vis_avs2.dll"
		; ${EndIf}
		File ${FILES_PATH}\Plugins\vis_avs.dll
		SetOverwrite off
		File ..\..\resources\data\vis_avs.dat
		SetOverwrite ${OVERWRITEMODE}

		DetailPrint "$(IDS_RUN_EXTRACT) $(IDS_AVS_PRESETS)..."
		SetDetailsPrint none

		SetOutPath $OUTDIR\AVS
		File ..\..\resources\data\avs\*.ape
		File ..\..\resources\data\avs\*.bmp

		SetOutPath "$OUTDIR\Winamp 5 Picks"
		Delete "$OUTDIR\fçk - checkers with metaballs (skupers remix).avs" ;fix for fucked up char
		File "..\..\resources\data\avs\Winamp 5 Picks\*.avs"

		SetOutPath "$OUTDIR\..\Community Picks"
		File "..\..\resources\data\avs\Community Picks\*.avs"

		SetDetailsPrint lastused

		SetOutPath $INSTDIR\Plugins
	${WinampSectionEnd}                                   ; <<< [Advanced Visualization Studio]
!endif ; WINAMP64
!endif ; full

!ifdef std | full
!ifndef WINAMP64
	${WinampSection} "secMilk2" $(IDS_SEC_MILKDROP2) IDX_SEC_MILKDROP2      ; >>> [Milkdrop2]
		${SECTIONIN_STD}
		Call SetVisPluginDir
		File "${FILES_PATH}\Plugins\vis_milk2.dll"
		;File ..\..\resources\Plugins\vis_milk2.dll
		SetOutPath $SETTINGSDIR\Plugins\Milkdrop2
		File ..\..\resources\data\milk2_img.ini
		File ..\..\resources\data\milk2_msg.ini

		; TODO if we make any other changes to presets, amend this as needed
		${IfNot} ${FileExists} "$INSTDIR\Plugins\Milkdrop2\installed.ini"
			DetailPrint "$(IDS_RUN_EXTRACT) $(IDS_MILK2_PRESETS)..."
			SetDetailsPrint none

			SetOutPath $INSTDIR\Plugins\Milkdrop2
			File /nonfatal /a /r /x CVS "..\..\resources\data\Milkdrop2\*.*"
			SetDetailsPrint lastused

			; using this to prevent overwriting existing presets (i.e. those which are rated)
			; and from re-install if this hasn't changed when we next update things as it'll
			; only install the presets the once and so speeds up the whole install process
			WriteIniStr "$INSTDIR\Plugins\Milkdrop2\installed.ini" "Install" "ver" "1"
		${Endif}

		; Start Temp 5.9.0 section
		; Force "suppress all warnings" settings on for v5.9.0 - this can be removed once issue is fixed
		${If} ${FileExists} "$SETTINGSDIR\Plugins\Milkdrop2\milk2.ini"
		WriteIniStr "$SETTINGSDIR\Plugins\Milkdrop2\milk2.ini" "settings" "bWarningsDisabled2" "1"
		${Endif}

		; New presets for 5.9
		${if} ${FileExists} "$InstDir\Plugins\Milkdrop2\Presets\martin - cascade.milk"
		${AndIf} ${FileExists} "$InstDir\Plugins\Milkdrop2\Presets\Serge - MilkDrop2077.R004.milk"
		goto skippy
		${Else}
		CopyFiles "${FILES_PATH}\resources\data\Milkdrop2\Presets\new\martin - cascade.milk" "$InstDir\Plugins\Milkdrop2\Presets\martin - cascade.milk"
		CopyFiles "${FILES_PATH}\resources\data\Milkdrop2\Presets\new\martin - colorwall.milk" "$InstDir\Plugins\Milkdrop2\Presets\martin - colorwall.milk"
		CopyFiles "${FILES_PATH}\resources\data\Milkdrop2\Presets\new\Serge - MilkDrop2077.R004.milk" "$InstDir\Plugins\Milkdrop2\Presets\Serge - MilkDrop2077.R004.milk"
		CopyFiles "${FILES_PATH}\resources\data\Milkdrop2\Presets\new\Serge - MilkDrop2077.R015.milk" "$InstDir\Plugins\Milkdrop2\Presets\Serge - MilkDrop2077.R015.milk"
		CopyFiles "${FILES_PATH}\resources\data\Milkdrop2\Presets\new\Serge - MilkDrop2077.R027.milk" "$InstDir\Plugins\Milkdrop2\Presets\Serge - MilkDrop2077.R027.milk"
		CopyFiles "${FILES_PATH}\resources\data\Milkdrop2\Presets\new\Serge - MilkDrop2077.R266.milk" "$InstDir\Plugins\Milkdrop2\Presets\Serge - MilkDrop2077.R266.milk"
		skippy:
		${EndIf}

		; Rename existing affected files with .off extension for 5.9.0 - this can be reversed once issue is fixed
		${If} ${FileExists} "$INSTDIR\Plugins\Milkdrop2\presets\martin - satellite view.milk"
		${AndIf} ${FileExists} "$INSTDIR\Plugins\Milkdrop2\presets\yin - 300 - Daydreamer.milk"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\martin - satellite view.milk" "$INSTDIR\Plugins\Milkdrop2\presets\martin - satellite view.milk.off"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\yin - 300 - Daydreamer.milk" "$INSTDIR\Plugins\Milkdrop2\presets\yin - 300 - Daydreamer.milk.off"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\shifter + geiss - neon pulse (glow mix).milk" "$INSTDIR\Plugins\Milkdrop2\presets\shifter + geiss - neon pulse (glow mix).milk.off"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\Redi Jedi - i dont think those were portabello mushrooms.milk" "$INSTDIR\Plugins\Milkdrop2\presets\Redi Jedi - i dont think those were portabello mushrooms.milk.off"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\yin - 140 - Ohm to the stars.milk" "$INSTDIR\Plugins\Milkdrop2\presets\yin - 140 - Ohm to the stars.milk.off"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\yin - 191 - Temporal singularities.milk" "$INSTDIR\Plugins\Milkdrop2\presets\yin - 191 - Temporal singularities.milk.off"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\shifter - brain coral.milk" "$INSTDIR\Plugins\Milkdrop2\presets\shifter - brain coral.milk.off"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\shifter - brain coral (left brained).milk" "$INSTDIR\Plugins\Milkdrop2\presets\shifter - brain coral (left brained).milk.off"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\martin - electric pool.milk" "$INSTDIR\Plugins\Milkdrop2\presets\martin - electric pool.milk.off"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\yin - 315 - Ocean of Light (yo im peakin yo Eo.S.-Phat).milk" "$INSTDIR\Plugins\Milkdrop2\presets\yin - 315 - Ocean of Light (yo im peakin yo Eo.S.-Phat).milk.off"
		Rename "$INSTDIR\Plugins\Milkdrop2\presets\Rovastar & Zylot - Crystal Ball (Many Visions Mix).milk" "$INSTDIR\Plugins\Milkdrop2\presets\Rovastar & Zylot - Crystal Ball (Many Visions Mix).milk.off"
		; Template
		; Rename "$INSTDIR\Plugins\Milkdrop2\presets\.milk" "$INSTDIR\Plugins\Milkdrop2\presets\.milk.off"
		${Endif}
		; End Temp 5.9.0 section

		; Remove "New" subfolder on clean installs (we already copy the new presets to root dir)
		${if} ${FileExists} "$InstDir\Plugins\Milkdrop2\Presets\new\martin - cascade.milk"
		${AndIf} ${FileExists} "$InstDir\Plugins\Milkdrop2\Presets\new\Serge - MilkDrop2077.R004.milk"
		Delete "$INSTDIR\Plugins\Milkdrop2\presets\new\martin - cascade.milk"
		Delete "$INSTDIR\Plugins\Milkdrop2\presets\new\martin - colorwall.milk"
		Delete "$INSTDIR\Plugins\Milkdrop2\presets\new\Serge - MilkDrop2077.R004.milk"
		Delete "$INSTDIR\Plugins\Milkdrop2\presets\new\Serge - MilkDrop2077.R015.milk"
		Delete "$INSTDIR\Plugins\Milkdrop2\presets\new\Serge - MilkDrop2077.R027.milk"
		Delete "$INSTDIR\Plugins\Milkdrop2\presets\new\Serge - MilkDrop2077.R266.milk"
		RmDir "$INSTDIR\Plugins\Milkdrop2\presets\new"
		${Endif}

		ClearErrors
		ReadINIStr $0 "$WINAMPINI" "Winamp" "visplugin_name"
		IfErrors 0 +3
		WriteINIStr "$WINAMPINI" "Winamp" "visplugin_name" vis_milk2.dll
		WriteINIStr "$WINAMPINI" "Winamp" "visplugin_num" 0
		SetOutPath $INSTDIR\Plugins
	${WinampSectionEnd}                                                  ; <<< [Milkdrop2]
!endif ; WINAMP64
!endif ; full

!ifdef full
!ifndef WINAMP64
	${WinampSection} "visLine" $(secLineInput) IDX_SEL_LINEIN           ; >>> [Line Input Support]
		${SECTIONIN_FULL}
		SetOutPath $INSTDIR\Plugins
		File "${FILES_PATH}\Plugins\in_linein.dll"
	${WinampSectionEnd}                                                  ; <<< [Line Input Support]
!endif ; WINAMP64
!endif ; std | full
SectionGroupEnd ;  Visualization