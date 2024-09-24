!ifdef std | full
SectionGroup $(IDS_GRP_MMEDIA_VIDEO_DEC)  IDX_GRP_MMEDIA_VIDEO_DEC ; Video Playback
	${WinampSection} "decoderWmv" $(secWMV) IDX_SEC_WMV_DEC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\in_wm.dll
		ClearErrors
		ReadINIStr $0 "$WINAMPINI" "in_dshow" "extlist"
		${IfNot} ${Errors}
			${If} $0 <> ""
				extstrip::remove "WMV" $0
				Pop $0
				extstrip::remove "ASF" $0
				Pop $0
				WriteINIStr "$WINAMPINI" "in_dshow" "extlist" $0
			${EndIf}
		${EndIf}
	${WinampSectionEnd}
   
!ifndef WINAMP64
	${WinampSection} "decoderNsv" $(secNSV) IDX_SEC_NSV_DEC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\in_nsv.dll
;		File /nonfatal ${FILES_PATH}\Plugins\nsvdec_vp3.dll ; this line will need to be commented out if we can't get nsvdec_vp3 to build (or we'll need to include a pre-built dll instead) - for now, set as: /nonfatal
		File ${FILES_PATH}\Plugins\nsvdec_vp5.dll
		Delete $INSTDIR\Plugins\nsvdec_vp6.dll ; delete old VP6 plugin
		SetOutPath $INSTDIR\System
		File ${FILES_PATH}\System\vp6.w5s
;		File /nonfatal ${FILES_PATH}\System\vp6.wbm
		File ${FILES_PATH}\System\vp8.w5s
;		File /nonfatal ${FILES_PATH}\System\vp8.wbm
		WriteRegStr HKEY_CLASSES_ROOT "UNSV" "" "URL: Ultravox Protocol"
		WriteRegStr HKEY_CLASSES_ROOT "UNSV" "URL Protocol" ""
		WriteRegStr HKEY_CLASSES_ROOT "UNSV\shell\open\command" "" "$INSTDIR\${WINAMPEXE} %1"
	${WinampSectionEnd}
!endif ; Winamp64

!ifndef WINAMP64
	${WinampSection} "decoderDirectShow" $(secDSHOW) IDX_SEC_DSHOW_DEC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\in_dshow.dll
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayVideoFilesOnArrival" "${WINAMP}PlayMediaOnArrival" ""
	${WinampSectionEnd}
!endif ; WINAMP64
   
	${WinampSection} "decoderAvi" $(secAVI) IDX_SEC_AVI_DEC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\in_avi.dll

		; remove AVI from in_dshow's extension list
		ClearErrors
		ReadINIStr $0 "$WINAMPINI" "in_dshow" "extlist"
		${IfNot} ${Errors}
			${If} $0 <> ""
				extstrip::remove "AVI" $0
				Pop $0
				WriteINIStr "$WINAMPINI" "in_dshow" "extlist" $0
			${EndIf}
		${EndIf}

		SetOutPath $INSTDIR\System
      
		; AVI video codecs
		File ${FILES_PATH}\System\h264.w5s
		; File /nonfatal ${FILES_PATH}\System\h264.wbm ; h264.wbm is currently created as 0 byte file, so pointless to include it.
		File ${FILES_PATH}\System\vp6.w5s
;		File /nonfatal ${FILES_PATH}\System\vp6.wbm
		File ${FILES_PATH}\System\mp4v.w5s
;		File /nonfatal ${FILES_PATH}\System\mp4v.wbm

		; AVI audio codecs
		; aacdec.w5s is installed by default (see winamp.nsh)
		; File ${FILES_PATH}\System\aacdec.w5s
		; File /nonfatal ${FILES_PATH}\System\aacdec.wbm
		File ${FILES_PATH}\System\adpcm.w5s
;		File /nonfatal ${FILES_PATH}\System\adpcm.wbm
		File ${FILES_PATH}\System\pcm.w5s
;		File /nonfatal ${FILES_PATH}\System\pcm.wbm
	${WinampSectionEnd}

	${WinampSection} "decoderFlv" $(secFLV) IDX_SEC_FLV_DEC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\in_flv.dll
		SetOutPath $INSTDIR\System

		; FLV video codecs
		File ${FILES_PATH}\System\h264.w5s
		; File /nonfatal ${FILES_PATH}\System\h264.wbm ; h264.wbm is currently created as 0 byte file, so pointless to include it.
		File ${FILES_PATH}\System\vp6.w5s
;		File /nonfatal ${FILES_PATH}\System\vp6.wbm

		; FLV audio codecs
		; aacdec.w5s is installed by default (see winamp.nsh)
		; File ${FILES_PATH}\System\aacdec.w5s
		; File /nonfatal ${FILES_PATH}\System\aacdec.wbm
		File ${FILES_PATH}\System\adpcm.w5s
;		File /nonfatal ${FILES_PATH}\System\adpcm.wbm

		File ${FILES_PATH}\System\f263.w5s
;		File /nonfatal ${FILES_PATH}\System\f263.wbm
	${WinampSectionEnd}
    
	${WinampSection} "decoderMkv" $(secMKV) IDX_SEC_MKV_DEC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\in_mkv.dll
		SetOutPath $INSTDIR\System

		; MKV video codecs
		File ${FILES_PATH}\System\h264.w5s
		; File /nonfatal ${FILES_PATH}\System\h264.wbm ; h264.wbm is currently created as 0 byte file, so pointless to include it.
		File ${FILES_PATH}\System\vp8.w5s
;		File /nonfatal ${FILES_PATH}\System\vp8.wbm
		File ${FILES_PATH}\System\theora.w5s
;		File /nonfatal ${FILES_PATH}\System\theora.wbm

		; MKV audio codecs
		; aacdec.w5s is installed by default (see winamp.nsh)
		; File ${FILES_PATH}\System\aacdec.w5s
		; File /nonfatal ${FILES_PATH}\System\aacdec.wbm

		File ${FILES_PATH}\System\f263.w5s
;		File /nonfatal ${FILES_PATH}\System\f263.wbm
	${WinampSectionEnd}

	${WinampSection} "decoderM4v" $(secM4V) IDX_SEC_M4V_DEC
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\in_mp4.dll
		SetOutPath $INSTDIR\Shared
		File ${FILES_PATH}\Shared\libmp4v2.dll
		SetOutPath $INSTDIR\System

		; MP4 video codecs
		File ${FILES_PATH}\System\h264.w5s
		; File /nonfatal ${FILES_PATH}\System\h264.wbm ; h264.wbm is currently created as 0 byte file, so pointless to include it.
		File ${FILES_PATH}\System\mp4v.w5s
;		File /nonfatal ${FILES_PATH}\System\mp4v.wbm

		; MP4 audio codecs
		; aacdec.w5s is installed by default (see winamp.nsh)
		; File ${FILES_PATH}\System\aacdec.w5s
		; File /nonfatal "${FILES_PATH}\System\aacdec.wbm"
		File ${FILES_PATH}\System\pcm.w5s
;		File /nonfatal ${FILES_PATH}\System\pcm.wbm
      
		SetOutPath $INSTDIR\System
		File ${FILES_PATH}\System\alac.w5s
		; File /nonfatal ${FILES_PATH}\System\alac.wbm
		; File ${FILES_PATH}\System\a52.w5s ; need legal team to determine whether Dolby AC3 patents have expired so AC3 decoder can be included
		; File /nonfatal ${FILES_PATH}\System\a52.wbm
	${WinampSectionEnd}
    
!ifndef WINAMP64
	${WinampSection} "decoderSwf" $(secSWF) IDX_SEC_SWF_DEC
		${SECTIONIN_FULL}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\in_swf.dll
		File ..\..\resources\data\winampFLV.swf
	${WinampSectionEnd}
!endif ; WINAMP64
SectionGroupEnd ; Video Playback
!endif ; std | full