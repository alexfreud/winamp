SectionGroup $(IDS_GRP_MMEDIA_OUTPUT)  IDX_GRP_MMEDIA_OUTPUT ; Output
	${WinampSection} "outputDirectSound" $(secDirectSound) IDX_SEC_OUT_DS
		SectionIn 1 2 3 4 5 6 7 8 RO
		SetOutPath $INSTDIR\Plugins
		File "${FILES_PATH}\Plugins\out_ds.dll"
	${WinampSectionEnd}
	
	${WinampSection} "outputWasapi" $(secWasapi) IDX_SEC_OUT_WASAPI
		${SECTIONIN_LITE}
		SetOutPath $INSTDIR\Plugins
		File "${FILES_PATH}\Plugins\out_wasapi.dll"
	${WinampSectionEnd}
  
!ifndef WINAMP64
	${WinampSection} "outputDisk" $(secWriteWAV) IDX_SEC_OUT_DISK
		${SECTIONIN_LITE}
		SetOutPath $INSTDIR\Plugins
		File "${FILES_PATH}\Plugins\out_disk.dll"
	${WinampSectionEnd}
!endif ; WINAMP64

	${WinampSection} "outputWav" $(IDS_SEC_OUT_WAV) IDX_SEC_OUT_WAV
		${SECTIONIN_LITE}
		SetOutPath $INSTDIR\Plugins
		File "${FILES_PATH}\Plugins\out_wave.dll"
	${WinampSectionEnd}
SectionGroupEnd ; Output