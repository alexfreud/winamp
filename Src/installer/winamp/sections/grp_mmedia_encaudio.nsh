!ifndef LITE
SectionGroup $(IDS_GRP_MMEDIA_AUDIO_ENC) IDX_GRP_MMEDIA_AUDIO_ENC ; Audio Encoders
!ifndef WINAMP64
!ifdef FULL
	${WinampSection} "winampWmaEncoder" $(secWMAE) IDX_SEC_WMA_ENC
		${SECTIONIN_FULL}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\enc_wma.dll
	${WinampSectionEnd}
!endif ; FULL
!endif ; WINAMP64

!ifndef WINAMP64
!ifdef FULL
	${WinampSection} "winampWavEncoder" $(IDS_SEC_WAV_ENC) IDX_SEC_WAV_ENC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\enc_wav.dll
	${WinampSectionEnd}
!endif ; FULL
!endif ; WINAMP64

!ifndef WINAMP64
!ifdef STD | FULL
	${WinampSection} "winampMp3Encoder" $(secMP3E) IDX_SEC_MP3_ENC
		${SECTIONIN_STD}
;		SetOutPath $INSTDIR\Shared
;		File ..\..\resources\libraries\lame_enc.dll  ; 5.9 - lame_enc now statically-linked, not a shared dynamic dll
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\enc_lame.dll
	${WinampSectionEnd}
!endif ;  FULL
!endif ; WINAMP64

; need to fix this section at some point, will probably need to remove it....
!if 0
!ifndef WINAMP64
!ifdef FULL
	${WinampSection} "encoderAac" $(secAACE) IDX_SEC_AAC_ENC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		Delete $INSTDIR\Plugins\enc_aac.dll
		Delete $INSTDIR\Plugins\enc_mp4.dll
		File /nonfatal ${FILES_PATH}\Plugins\enc_fhgaac.dll ; missing, as this can no longer be shipped
		SetOutPath $INSTDIR\Shared
		File ${FILES_PATH}\Shared\libmp4v2.dll
	${WinampSectionEnd}
!endif ; FULL
!endif ; WINAMP64
!endif

!ifndef WINAMP64
!ifdef FULL
	${WinampSection} "encoderFlac" $(IDS_SEC_FLAC_ENC) IDX_SEC_FLAC_ENC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\enc_flac.dll
;		SetOutPath $INSTDIR\Shared
;		File ${FILES_PATH}\Shared\libFLAC.dll ; 5.9 - libflac now statically-linked, not a shared dynamic dll
	${WinampSectionEnd}
!endif ; FULL
!endif ; WINAMP64

!ifndef WINAMP64
!ifdef FULL
	${WinampSection} "encoderOgg" $(secOGGEnc) IDX_SEC_OGG_ENC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\enc_vorbis.dll
	${WinampSectionEnd}
!endif ; FULL
!endif ; WINAMP64

SectionGroupEnd ;  Audio Encoders
!endif ; NOT LITE