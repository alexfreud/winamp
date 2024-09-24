SectionGroup $(IDS_GRP_MMEDIA) IDX_GRP_MMEDIA ; Multimedia Engine
	!include ".\sections\grp_mmedia_decaudio.nsh"
	!include ".\sections\grp_mmedia_decvideo.nsh"
	!include ".\sections\grp_mmedia_encaudio.nsh"
	!include ".\sections\grp_mmedia_output.nsh"

/* To be replaced by MusicBrainz
!ifndef WINAMP64
	${WinampSection} "cddb" $(secCDDB) IDX_SEC_CDDB                                          ; >>> [CDDB for recognizing CDs]
		${SECTIONIN_LITE}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\in_cdda.dll
	${WinampSectionEnd}                                                   ; <<< [CDDB for recognizing CDs]
!endif ; WINAMP64
*/
  
!ifndef WINAMP64
!ifdef FULL
	${WinampSection} "digitalSignalProcessing" $(secDSP) IDX_SEC_DSP                                       ; >>> [Signal Processor Studio Plug-in]
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		; File "..\..\resources\Plugins\dsp_sps.dll"
		File ${FILES_PATH}\Plugins\dsp_sps.dll
		DetailPrint "$(IDS_RUN_EXTRACT) $(IDS_DSP_PRESETS)..." ; Extracting presets...
		SetDetailsPrint none
		SetOutPath $INSTDIR\Plugins\DSP_SPS
		File "..\..\resources\data\dsp_sps\*.sps"
		SetOutPath $INSTDIR\Plugins
		SetDetailsPrint lastused
	${WinampSectionEnd}                                                  ; <<< [Signal Processor Studio Plug-in]
!endif ; full
!endif ; WINAMP64
SectionGroupEnd ; Multimedia Engine