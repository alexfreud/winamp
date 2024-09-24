!macro DllUnregisterAndDelete __dllPath
	${If} ${FileExists} "${__dllPath}"
		UnRegdll "${__dllPath}"
		Delete /REBOOTOK "${__dllPath}"
	${EndIf}
!macroend

!define DllUnregisterAndDelete "!insertmacro 'DllUnregisterAndDelete'"

${WinampSection} "winampApplication" $(secWinamp) IDX_SEC_WINAMP           ; <<< [Winamp]
	SectionIn 1 2 3 4 5 6 7 8 RO
  
	DetailPrint "$(IDS_CLEANUP_PLUGINS)"
	SetDetailsPrint none

	; cleanup old shit
	UnRegDLL "$INSTDIR\Plugins\in_asfs.dll"
	Delete /REBOOTOK "$INSTDIR\Plugins\in_asfs.dll"

	; Gracenote doesn't need installing, remove any existing files
	${DllUnregisterAndDelete} "$INSTDIR\Plugins\cddbcontrolwinamp.dll"
	${DllUnregisterAndDelete} "$INSTDIR\Plugins\cddbuiwinamp.dll"
	${DllUnregisterAndDelete} "$INSTDIR\Plugins\Gracenote\cddbuiwinamp.dll"
	${DllUnregisterAndDelete} "$INSTDIR\Plugins\Gracenote\CddbMusicIDWinamp.dll"
	${DllUnregisterAndDelete} "$INSTDIR\Plugins\Gracenote\CddbPlaylist2Winamp.dll"
	${DllUnregisterAndDelete} "$INSTDIR\Plugins\Gracenote\cddbcontrolwinamp.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbWOManagerWinamp.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbFPX1.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbAFX3.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbCMSig_1_2.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbCMSig_1_3.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbFEX.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbS12T.dll"
	Delete "$INSTDIR\Plugins\Gracenote\Cddbx*.dll"
	Delete "$INSTDIR\System\gracenote.w5s"
	Delete "$INSTDIR\System\gracenote.wbm"
	RMDir "$INSTDIR\Plugins\Gracenote"
	; Gracenote end

	; Remove old VS2008 runtime on Win8.1 and higher
	${if} ${IsWin7}
	goto skipthis1
	${Else}
	Delete "$INSTDIR\Microsoft.VC90.CRT\msvcr90.dll"
	Delete "$INSTDIR\Microsoft.VC90.CRT\msvcp90.dll"
	Delete "$INSTDIR\Microsoft.VC90.CRT\Microsoft.VC90.CRT.manifest"
	RMDir /r "$INSTDIR\Microsoft.VC90.CRT"
	skipthis1:
	${EndIf}

	Delete "$INSTDIR\System\vp8x.w5s"
	Delete "$INSTDIR\System\vp8x.wbm"
	Delete "$INSTDIR\System\a52.w5s"
	Delete "$INSTDIR\System\a52.wbm"
	Delete "$INSTDIR\System\dca.w5s"
	Delete "$INSTDIR\System\dca.wbm"

	Delete "$INSTDIR\Plugins\ml_orb.dll"
	Delete "$INSTDIR\Plugins\gen_dropbox.dll"

	Delete "$INSTDIR\System\watcher.w5s" ; watcher is unused now that Music Now is gone  
	Delete "$INSTDIR\System\watcher.wbm"
	Delete "$INSTDIR\System\db.w5s"
	Delete "$INSTDIR\System\db.wbm"
	Delete "$INSTDIR\Plugins\gen_ff.dll"
	Delete "$INSTDIR\Plugins\Freeform\wacs\freetype\freetype.wac"
	Delete "$INSTDIR\System\timer.w5s"
	Delete "$INSTDIR\System\UnicodeTaskbarFix.w5s"
	Delete "$INSTDIR\System\timer.wbm"
	Delete "$INSTDIR\System\UnicodeTaskbarFix.wbm"
	Delete "$INSTDIR\System\filereader.w5s"
	Delete "$INSTDIR\System\filereader.wbm"
	Delete "$INSTDIR\System\jnetlib.w5s"
	Delete "$INSTDIR\System\jnetlib.wbm"
	Delete "$INSTDIR\System\aacPlusDecoder.w5s"
	Delete "$INSTDIR\System\aacPlusDecoder.wbm"
	Delete "$INSTDIR\System\aacdec.w5s"
	Delete "$INSTDIR\System\aacdec.wbm"
	Delete "$INSTDIR\System\adpcm.w5s"
	Delete "$INSTDIR\System\adpcm.wbm"
	Delete "$INSTDIR\System\pcm.w5s"
	Delete "$INSTDIR\System\pcm.wbm"
	Delete "$INSTDIR\System\mp3.w5s"
	Delete "$INSTDIR\System\mp3.wbm"
	Delete "$INSTDIR\System\alac.w5s"
	Delete "$INSTDIR\System\alac.wbm"
	Delete "$INSTDIR\System\theora.w5s"
	Delete "$INSTDIR\System\theora.wbm"
	Delete "$INSTDIR\System\f263.w5s"
	Delete "$INSTDIR\System\f263.wbm"
	Delete "$INSTDIR\System\vlb.w5s"
	Delete "$INSTDIR\System\vlb.wbm"
	Delete "$INSTDIR\System\mp4v.w5s"
	Delete "$INSTDIR\System\mp4v.wbm"
	Delete "$INSTDIR\System\vp6.w5s"
	Delete "$INSTDIR\System\vp6.wbm"
	Delete "$INSTDIR\System\vp8.w5s"
	Delete "$INSTDIR\System\vp8.wbm"
	Delete "$INSTDIR\System\h264.w5s"
	Delete "$INSTDIR\System\h264.wbm"
	Delete "$INSTDIR\System\dlmgr.w5s"
	Delete "$INSTDIR\System\dlmgr.wbm"
	Delete "$INSTDIR\System\ClassicPro.wbm"
	Delete "$INSTDIR\System\wac_network.w5s"
	Delete "$INSTDIR\System\wac_downloadManager.w5s"
	Delete "$INSTDIR\Plugins\gen_ml.dll" ; make sure gen_ff and gen_ml are not kept if unchecked

	; delete ML plug-ins so that unselecting them does not keep the old versions around
	Delete "$INSTDIR\Plugins\ml_xpdxs.dll"
	Delete "$INSTDIR\Plugins\ml_local.dll"
	Delete "$INSTDIR\Plugins\ml_playlists.dll"
	Delete "$INSTDIR\Plugins\ml_disc.dll"
	Delete "$INSTDIR\Plugins\ml_bookmarks.dll"
	Delete "$INSTDIR\Plugins\ml_history.dll"
	Delete "$INSTDIR\Plugins\ml_impex.dll"
	Delete "$INSTDIR\Plugins\ml_nowplaying.dll"
	Delete "$INSTDIR\Plugins\ml_rg.dll"
	Delete "$INSTDIR\Plugins\ml_plg.dll"
	Delete "$INSTDIR\Plugins\ml_online.dll"
	Delete "$INSTDIR\Plugins\ml_dash.dll"
	Delete "$INSTDIR\Plugins\ml_wire.dll"
	Delete "$INSTDIR\Plugins\ml_transcode.dll"
	Delete "$INSTDIR\Plugins\ml_autotag.dll"
	Delete "$INSTDIR\Plugins\ml_addons.dll"
	Delete "$INSTDIR\Plugins\ml_downloads.dll"
	Delete "$INSTDIR\Plugins\ml_cloud.dll"
	Delete "$INSTDIR\Plugins\ml_nft.dll"
	Delete "$INSTDIR\Plugins\ml_fanzone.dll"
	Delete "$INSTDIR\Plugins\ml_hotmixradio.dll"
	Delete "$INSTDIR\Plugins\gen_orgler.dll"
	Delete "$INSTDIR\System\auth.w5s"
	Delete "$INSTDIR\System\wasabi2.w5s"
	Delete "$INSTDIR\Components\cloud.w6c"
	Delete "$INSTDIR\Components\ssdp.w6c"
	Delete "$INSTDIR\Components\web.w6c"

	Delete "$INSTDIR\Plugins\ml_pmp.dll"
	Delete "$INSTDIR\Plugins\ml_devices.dll"
	Delete "$INSTDIR\Plugins\pmp_p4s.dll"
	Delete "$INSTDIR\Plugins\pmp_ipod.dll"
	Delete "$INSTDIR\Plugins\pmp_wifi.dll"
	Delete "$INSTDIR\Plugins\pmp_njb.dll"
	Delete "$INSTDIR\Plugins\pmp_usb.dll"
	Delete "$INSTDIR\Plugins\pmp_usb2.dll"
	Delete "$INSTDIR\Plugins\pmp_android.dll"
	Delete "$INSTDIR\Plugins\pmp_activesync.dll"
	Delete "$INSTDIR\Plugins\pmp_cloud.dll"
	Delete "$INSTDIR\System\devices.w5s"
	Delete "$INSTDIR\System\devices.wbm"

	Delete "$INSTDIR\Plugins\gen_b4s2m3u.dll"
	Delete "$INSTDIR\pxsdkpls.dll"
	Delete "$INSTDIR\primosdk.dll"
	Delete "$INSTDIR\burnlib.dll"
	Delete "$INSTDIR\System\primo.w5s"
	Delete "$INSTDIR\pconfig.dcf"
	Delete "$INSTDIR\demoedit.aac"

	Delete "$INSTDIR\jnetlib.dll"
	Delete "$INSTDIR\libFLAC.dll"
	Delete "$INSTDIR\libmp4v2.dll"
	Delete "$INSTDIR\Shared\libmpg123.dll"
	Delete "$INSTDIR\libsndfile.dll"
	Delete "$INSTDIR\nde.dll"
	Delete "$INSTDIR\nsutil.dll"
	Delete "$INSTDIR\nxlite.dll"
	Delete "$INSTDIR\Plugins\ReplayGainAnalysis.dll"
	Delete "$INSTDIR\tataki.dll"
	Delete "$INSTDIR\Plugins\tataki.dll"
	Delete "$INSTDIR\zlib.dll"

	Delete "$INSTDIR\Shared\jnetlib.dll"
	Delete "$INSTDIR\Shared\libFLAC.dll"
	Delete "$INSTDIR\Shared\libalac.dll"
	Delete "$INSTDIR\Shared\libFLAC_dynamic.dll"
	Delete "$INSTDIR\Shared\libmp4v2.dll"
	Delete "$INSTDIR\Shared\libsndfile.dll"
	Delete "$INSTDIR\Shared\nde.dll"
	Delete "$INSTDIR\Shared\nsutil.dll"
	Delete "$INSTDIR\Shared\nxlite.dll"
	Delete "$INSTDIR\Shared\ReplayGainAnalysis.dll"
	Delete "$INSTDIR\Shared\tataki.dll"
	Delete "$INSTDIR\Shared\zlib.dll"
	Delete "$INSTDIR\Shared\read_file.dll"

	Delete "$INSTDIR\Winamp.exe.manifest"
	Delete "$SETTINGSDIR\winamp.pic" ; deprecated plug-in cache file

	; delete the Qt5 engine files
	Delete "$INSTDIR\Qt5Core.dll"
	Delete "$INSTDIR\Qt5Network.dll"
	Delete "$INSTDIR\platforms\qwindows.dll"
	RMDir "$INSTDIR\platforms"
	Delete "$INSTDIR\printsupport\windowsprintersupport.dll"
	RMDir "$INSTDIR\printsupport"
	Delete "$INSTDIR\QtPositioning\declarative_positioning.dll"
	RMDir "$INSTDIR\QtPositioning"

	; delete the Cloud settings and associated folders
	; will leave playlists behind incase people made
	; cloud ones or used the restore on loss feature
	RMDir /r "$SETTINGSDIR\Cloud\views"
	RMDir "$SETTINGSDIR\Cloud\playlists"
	RMDir /r "$SETTINGSDIR\Cloud\logs"
	Delete "$SETTINGSDIR\Cloud\*.db"
	Delete "$SETTINGSDIR\Cloud\*.db-shm"
	Delete "$SETTINGSDIR\Cloud\*.db-wal"
	RMDir "$SETTINGSDIR\Cloud" ; if there's still something left then leave e.g. playlists

	; delete any other plug-ins so that unselecting them does not keep the old versions around
	Delete "$INSTDIR\Plugins\gen_crasher.dll"
	Delete "$INSTDIR\Plugins\gen_tray.dll"
	Delete "$INSTDIR\Plugins\gen_hotkeys.dll"
	Delete "$INSTDIR\Plugins\in_wm.dll"
	Delete "$INSTDIR\Plugins\in_mp4.dll"
	Delete "$INSTDIR\Plugins\in_cdda.dll"
	Delete "$INSTDIR\Plugins\in_midi.dll"
	Delete "$INSTDIR\Plugins\in_mod.dll"
	Delete "$INSTDIR\Plugins\read_file.dll"
	Delete "$INSTDIR\Plugins\in_vorbis.dll"
	Delete "$INSTDIR\Plugins\in_flac.dll"
	Delete "$INSTDIR\Plugins\in_wave.dll"
	Delete "$INSTDIR\Plugins\in_nsv.dll"
	Delete "$INSTDIR\Plugins\in_flv.dll"
	Delete "$INSTDIR\Plugins\in_swf.dll"
	Delete "$INSTDIR\Plugins\in_dshow.dll"
	Delete "$INSTDIR\Plugins\in_avi.dll"
	Delete "$INSTDIR\Plugins\in_mkv.dll"
	Delete "$INSTDIR\Plugins\in_linein.dll"
	Delete "$INSTDIR\Plugins\enc_lame.dll"
	Delete "$INSTDIR\Plugins\enc_wma.dll"
	Delete "$INSTDIR\Plugins\enc_wav.dll"
	Delete "$INSTDIR\Plugins\enc_flac.dll"
	Delete "$INSTDIR\Plugins\enc_vorbis.dll"
	Delete "$INSTDIR\Plugins\enc_aacplus.dll"
	Delete "$INSTDIR\Plugins\out_wave.dll"
	Delete "$INSTDIR\Plugins\out_disk.dll"
	Delete "$INSTDIR\Plugins\vis_nsfs.dll"
	Delete "$INSTDIR\Plugins\vis_avs.dll"
	Delete "$INSTDIR\Plugins\vis_milk2.dll"
	Delete "$INSTDIR\Plugins\dsp_sps.dll"
	Delete "$INSTDIR\Plugins\nsvdec_vp3.dll"
	Delete "$INSTDIR\Plugins\nsvdec_vp5.dll"
	Delete "$INSTDIR\System\ombrowser.w5s"
	Delete "$INSTDIR\System\ombrowser.wbm"
	; Delete "$INSTDIR\Plugins\gen_jumpex.dll" ; delete JTFE as no longer supported (not)

	; 5.8: keep enc_fhgaac.dll & lame_enc.dll (if present on upgrades) | 5.9: delete lame_enc.dll only
	; Delete "$INSTDIR\Plugins\enc_fhgaac.dll"
	Delete "$INSTDIR\Plugins\lame_enc.dll"

	; Delete DrO's ml_enqplay plugin because it crashes Winamp 5.9
	Delete "$INSTDIR\Plugins\ml_enqplay.dll"
  
	; Delete DrO's gen_ipc_stopplaying_blocker.dll which was a workaround for 5.57x only
	Delete "$INSTDIR\Plugins\gen_ipc_stopplaying_blocker.dll"

	; Delete DrO's gen_find_on_disk.dll which is now fully integrated into Winamp in 5.64
	; The plug-ins core had been present as api_explorerfindfile.h for a while, just not
	; done fully as a native menu item and global hotkey (does preseve any prior hotkey)
	Delete "$INSTDIR\Plugins\gen_find_on_disk.dll"

	; Delete DrO's gen_nunzio.dll as now replaced by koopa's info tool
	Delete "$INSTDIR\Plugins\gen_nunzio.dll"

	; Delete DrO's gen_cd_menu.dll which is now fully integrated into Winamp in 5.64
	; The functionality was in winamp agent (showing cd drive volume name in menus)
	Delete "$INSTDIR\Plugins\gen_cd_menu.dll"

	; Delete DrO's gen_wolfgang.dll which is now fully integrated into gen_hotkeys in 5.64
	Delete "$INSTDIR\Plugins\gen_wolfgang.dll"

	; Delete DrO's gen_os_diag.dll which is now partially integrated (saving main dialog positions) in 5.64
	; and is causing issues with the Alt+3 dialog and the combobox so better safe than sorry, it now dies.
	Delete "$INSTDIR\Plugins\gen_os_diag.dll"

	; Delete DrO's gen_mwblock.dll which is now fully integrated into Winamp in 5.66
	Delete "$INSTDIR\Plugins\gen_mwblock.dll"

	; Delete Joonas' old mass tagger plugin, as is now natively implemented in 5.66 pledit
	Delete "$INSTDIR\Plugins\gen_tagger.dll"

	; Delete Thinktink's bpembededart plugin (uncomment these lines if dro's embed albumart feature is reimplemeted)
	; Delete "$INSTDIR\Plugins\gen_bpembededart.dll"
	; Delete "$INSTDIR\System\bpembededart.w5s"
	; Delete "$SETTINGSDIR\Plugins\BPEmbededArt.ini"

	; Disable buggy 3rd-party gen_msn7 plugin (causes v5.57+ to crash on load)
	${If} ${FileExists} "$INSTDIR\Plugins\gen_msn7.dll"
	MessageBox MB_OK "$(IDS_MSN7_PLUGIN_DISABLE)" /SD IDOK
	Rename "$INSTDIR\Plugins\gen_msn7.dll" "$INSTDIR\Plugins\gen_msn7.dll.off"
	${EndIf}

	; Warn about 3rd-party gen_lyrics plugin (old version causes v5.59+ to crash on load)
	${If} ${FileExists} "$INSTDIR\Plugins\gen_lyrics.dll"
	MessageBox MB_OK "$(IDS_LYRICS_PLUGIN_WARNING)" /SD IDOK
	; Rename "$INSTDIR\Plugins\gen_lyrics.dll" "$INSTDIR\Plugins\gen_lyrics.dll.off"
	${EndIf}

	; Disable 3rd-party gen_lyrics_ie plugin (causes Winamp to crash)
	${If} ${FileExists} "$INSTDIR\Plugins\gen_lyrics_ie.dll"
	MessageBox MB_OK "$(IDS_LYRICS_IE_PLUGIN_DISABLE)" /SD IDOK
	Rename "$INSTDIR\Plugins\gen_lyrics_ie.dll" "$INSTDIR\Plugins\gen_lyrics_ie.off"
	${EndIf}

	DeleteINIStr "$WINAMPINI" "Winamp" "mbdefloc"
	SetDetailsPrint lastused

;------------------------------------------------------------------------------------
;install section

; VC142 runtime required for Win7 & 8 installations - pt.2: embedded
; Do we need this any more? Let's leave it for now, but look at removing it later (for 5.9.1)...
!ifndef WINAMP64
	${If} ${AtLeastWin7}
	${AndIf} ${AtMostWin8.1}
	SetOutPath "$INSTDIR\Microsoft.VC142.CRT"
	File "${FILES_PATH}\resources\libraries\msvcp140.dll"
	File "${FILES_PATH}\resources\libraries\vcruntime140.dll"
	File "${FILES_PATH}\resources\libraries\msvcp140_1.dll"
	File "${FILES_PATH}\resources\libraries\msvcp140_2.dll"
	File "${FILES_PATH}\resources\libraries\msvcp140_atomic_wait.dll"
	File "${FILES_PATH}\resources\libraries\msvcp140_codecvt_ids.dll"
	File "${FILES_PATH}\resources\libraries\vccorlib140.dll"
	File "${FILES_PATH}\resources\libraries\concrt140.dll"
	${EndIf}
!endif

; VC90 runtime section (for Win7 only)
!ifndef WINAMP64
	${If} ${IsWin7}
	${AndIfNot} ${FileExists} "$INSTDIR\Microsoft.VC90.CRT\msvcr90.dll"
	SetOutPath "$INSTDIR\Microsoft.VC90.CRT"
	File "${FILES_PATH}\resources\libraries\msvcr90.dll"
	File "${FILES_PATH}\resources\libraries\msvcp90.dll"
	File "${FILES_PATH}\resources\libraries\Microsoft.VC90.CRT.manifest"
	${EndIf}
	SetOutPath "$INSTDIR"
!endif

	; TODO alter this to show a message (here or later in install)
;!ifdef PRO
;	WriteINIStr "$WINAMPINI" "Winamp" "PromptForRegKey" "1"
;!endif ; PRO
	SetOutPath "$INSTDIR"

	SetOverwrite off
	File "/oname=$WINAMPM3U" "..\..\resources\media\winamp.m3u"
	SetOverwrite ${OVERWRITEMODE}

	File "/oname=${WINAMPEXE}" "${FILES_PATH}\winamp.exe"

	${If} ${AtLeastWin7}
		${If} ${FileExists} "$INSTDIR\elevator.exe"
			KillProcDLL::KillProc "elevator.exe"
			Sleep 1000
		${EndIf}

		${If} ${FileExists} "$INSTDIR\elevatorps.dll"
			UnRegDLL "$INSTDIR\elevatorps.dll"
			Sleep 1000
			Delete "$INSTDIR\elevatorps.dll"
		${EndIf}

		${If} ${FileExists} "$INSTDIR\Shared\elevatorps.dll"
			UnRegDLL "$INSTDIR\Shared\elevatorps.dll"
			Sleep 1000
		${EndIf}

		ExecWait '"$INSTDIR\elevator.exe" /UnregServer'
		Sleep 1000

		${If} ${FileExists} "$INSTDIR\elevatorps.dll"
			UnRegDLL "$INSTDIR\elevatorps.dll"
			Sleep 1000
			Delete "$INSTDIR\elevatorps.dll"
		${EndIf}

		${If} ${FileExists} "$INSTDIR\Shared\elevatorps.dll"
			UnRegDLL "$INSTDIR\Shared\elevatorps.dll"
			Sleep 1000
		${EndIf}

		SetOutPath "$INSTDIR"
		File ${FILES_PATH}\elevator.exe
		ExecWait '"$INSTDIR\elevator.exe" /RegServer'

		SetOutPath "$INSTDIR\Shared"
		File ${FILES_PATH}\Shared\elevatorps.dll
		RegDll "$INSTDIR\Shared\elevatorps.dll"
	${EndIf}

	SetOutPath "$M3UBASEDIR"
	File "..\..\resources\media\demo.mp3"
	SetOutPath "$INSTDIR"

	File "/oname=whatsnew.txt" "..\..\resources\data\whatsnew.txt"

	Call GetSkinDir
	Pop $R0
	SetOutPath $R0

	DetailPrint "$(IDS_REMOVE_SKINS)"
	SetDetailsPrint none
	Delete "$R0\${MODERNSKINNAME}.*"
	RMDir /r "$R0\${MODERNSKINNAME}"

	RMDir /r "$R0\Winamp Bento"
	Delete "$R0\Winamp Bento.*"

	RMDir /r "$R0\Bento"
	RMDir /r "$R0\Big Bento"
	SetDetailsPrint lastused

	SetOutPath $INSTDIR\Shared
	File "${FILES_PATH}\Shared\nsutil.dll"
	File "${FILES_PATH}\Shared\tataki.dll"
	; File "${FILES_PATH}\Shared\zlib.dll" ; as from 5.9, zlib is now statically-linked, not a shared dynamic .dll

	SetOutPath "$INSTDIR\System"
	; 2022 - determine whether jnetlib.w5s is still required. If not, these 2 lines need disabling:
	; File "${FILES_PATH}\System\jnetlib.w5s"
	; File /nonfatal "${FILES_PATH}\System\jnetlib.wbm"
	; Jul 2022 - jnetlib.w5s is required by ml_wire & ml_downloads,
	; so for now, we will only install it with those two plugins (see: grp_walib.nsh)

	File "${FILES_PATH}\System\aacdec.w5s"
;!ifndef WINAMP64
;	File /nonfatal "${FILES_PATH}\System\aacdec.wbm"
;!endif

	; File "${FILES_PATH}\System\dlmgr.w5s"
	; File /nonfatal "${FILES_PATH}\System\dlmgr.wbm"
	File "${FILES_PATH}\System\wac_downloadManager.w5s"
	File "${FILES_PATH}\System\tagz.w5s"
	File "${FILES_PATH}\System\albumart.w5s"
	File "${FILES_PATH}\System\playlist.w5s"
	File "${FILES_PATH}\System\xspf.w5s"
	File "${FILES_PATH}\System\xml.w5s"
	File "${FILES_PATH}\System\jpeg.w5s"
	File "${FILES_PATH}\System\png.w5s"
	File "${FILES_PATH}\System\bmp.w5s"
	File "${FILES_PATH}\System\gif.w5s"

	; File "..\..\resources\Plugins\UnicodeTaskbarFix.w5s" ; this might need to be disabled...

	; uncomment these lines if dro's albumart retrieval service is reimplemented
	; File "${FILES_PATH}\System\wasabi2.w5s"
	; SetOutPath $INSTDIR\Components
	; File "${FILES_PATH}\Components\web.w6c"

	SetOutPath "$INSTDIR"
	File "${FILES_PATH}\fmt.dll"

	Push $0
	StrCpy $0 "$INSTDIR\${WINAMPEXE}"
	WriteRegStr HKCR "UVOX" "" "URL: Ultravox Protocol"
	WriteRegStr HKCR "UVOX" "URL Protocol" ""
	WriteRegStr HKCR "UVOX\shell\open\command" "" "$0 %1"

	WriteRegStr HKCR "SC" "" "URL: SHOUTcast Protocol"
	WriteRegStr HKCR "SC" "URL Protocol" ""
	WriteRegStr HKCR "SC\shell\open\command" "" "$0 %1"
	WriteRegStr HKCR "ICY" "" "URL: SHOUTcast Protocol"
	WriteRegStr HKCR "ICY" "URL Protocol" ""
	WriteRegStr HKCR "ICY\shell\open\command" "" "$0 %1"
	WriteRegStr HKCR "SHOUT" "" "URL: SHOUTcast Protocol"
	WriteRegStr HKCR "SHOUT" "URL Protocol" ""
	WriteRegStr HKCR "SHOUT\shell\open\command" "" "$0 %1"
	Pop $0

	; bugfix?
	; this might come back due to WinampURIHandler reg code in Winamp\handler.cpp
	; which can cause filetypes associated with Winamp to be erroneously registered as "URL:Winamp Command Handler"
	DeleteRegKey HKCR "winamp"

	; Enable Firewall
	DetailPrint "$(IDS_FIREWALL)"
	Push $0
	Push $1

	StrCpy $0 "$INSTDIR\${WINAMPEXE}"

	; TODO possibly add a command-line switch to allow for this part to be skipped over?
	; TCP
	; try to update existing rule
	ExecDos::exec /NOUNLOAD /TIMEOUT=5000 'netsh advfirewall firewall set rule name="${WINAMP}" dir=in program="$0" profile=private,public protocol=TCP new action=allow enable=yes' "" ""
	Pop $1 ;return value
	${If} $1 != 0 ; failed
		; attempt to add application using advanced method (Vista+) 
		ExecDos::exec /NOUNLOAD /TIMEOUT=5000 'netsh advfirewall firewall add rule name="${WINAMP}" dir=in action=allow program="$0" enable=yes profile=private,public protocol=TCP' "" ""
		Pop $1
		${If} $1 != 0 
			ExecDos::exec /NOUNLOAD /TIMEOUT=2000 'netsh firewall add allowedprogram program="$0" name="${WINAMP}" mode=ENABLE scope=ALL profile=ALL' "" ""
		${EndIf}
	${EndIf}

	; UDP
	; try to update existing rule
	ExecDos::exec /NOUNLOAD /TIMEOUT=5000 'netsh advfirewall firewall set rule name="${WINAMP}" dir=in program="$0" profile=private,public protocol=UDP new action=allow enable=yes' "" ""
	Pop $1 ;return value
	${If} $1 != 0 ; failed
		; attempt to add application using advanced method (Vista+) 
		ExecDos::exec /NOUNLOAD /TIMEOUT=5000 'netsh advfirewall firewall add rule name="${WINAMP}" dir=in action=allow program="$0" enable=yes profile=private,public protocol=UDP' "" ""
	${EndIf}

	Pop $1
	Pop $0
	;SetDetailsPrint lastused
${WinampSectionEnd} ;                                                                   <<< [Winamp]
