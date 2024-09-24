!ifndef NULLSOFT_WINAMP_UNINSTALLER_PLAYER_HEADER
!define NULLSOFT_WINAMP_UNINSTALLER_PLAYER_HEADER

!macro WALANG_UNINSTALL LNGPATH LANGID
	Delete "${LNGPATH}\Winamp-${LANGID}.wlz"
!macroend

!macro Uninstaller_KillWinamp

	DetailPrint "$(IDS_RUN_CHECK_PROCESS)"
	SetDetailsPrint none
	${Do}
		ClearErrors
		Delete "$INSTDIR\${WINAMPEXE}"
		${If} ${Errors}
			SetErrors
			IfSilent killWinamp_done
			MessageBox MB_DEFBUTTON2|MB_ABORTRETRYIGNORE "$(msgCloseWinamp)" IDABORT killWinamp_done IDRETRY +2
			ClearErrors
		${EndIf}
	${LoopWhile} ${Errors}

	Push $0
	Push $1

	StrCpy $1 0
	${Do}
		ClearErrors
		Delete "$INSTDIR\winampa.exe"
		${If} ${Errors}
			SetErrors
			FindWindow $0 "WinampAgentMain"
			${If} $0 = 0  ; something bad or other session
				IfSilent killWinamp_done
				MessageBox MB_DEFBUTTON2|MB_ABORTRETRYIGNORE "$(IDS_MSG_AGENTONOTHERSESSION)" IDABORT killWinamp_done IDRETRY +2
				ClearErrors
			${Else}
				IntOp $1 $1 + 1
				IntCmp $1 40 killWinamp_done
				SendMessage $0 ${WM_CLOSE} 0 0 /TIMEOUT=2000
				Sleep 250
				IsWindow $0 0 killWinamp_agentKilled
					SendMessage $0 ${WM_DESTROY} 0 0 /TIMEOUT=2000
					Sleep 250
				killWinamp_agentKilled:
			${EndIf}
		${EndIf}
	${LoopWhile} ${Errors}

	Pop $1
	Pop $0

	SetDetailsPrint lastused

 killWinamp_done:
	${If} ${Errors}
		Abort "$(msgInstallAborted)"
	${EndIf}
!macroend
    
Section "un.$(IDS_UNINSTALL_MEDIA_PLAYER)" IDX_UNINSTALL_MEDIA_PLAYER
	ExecWait '"$INSTDIR\${WINAMPEXE}" /UNREG'
	!insertmacro 'Uninstaller_KillWinamp'

	ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "WinampAgent"
	${If} $0 == "$INSTDIR\winampa.exe"
		DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "WinampAgent"
	${EndIf}

	DeleteRegKey HKEY_CURRENT_USER 'Software\${WINAMP}'

	DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.File"
	DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.Playlist"
	DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.SkinZip"
	DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.LangZip"
	DeleteRegKey HKEY_CLASSES_ROOT "${WINAMP}.File.*"

	DeleteRegKey HKEY_CLASSES_ROOT "Directory\shell\${WINAMP}.Play"
	DeleteRegKey HKEY_CLASSES_ROOT "Directory\shell\${WINAMP}.Enqueue"
	DeleteRegKey HKEY_CLASSES_ROOT "Directory\shell\${WINAMP}.Bookmark"
	DeleteRegKey HKEY_CLASSES_ROOT "Directory\shell\${WINAMP}.WinampLibrary"
	DeleteRegKey HKEY_CLASSES_ROOT "Directory\shell\${WINAMP}.EnqueueAndPlay"
	DeleteRegKey HKLM "Software\Classes\Directory\shell\${WINAMP}.Play"
	DeleteRegKey HKLM "Software\Classes\Directory\shell\${WINAMP}.Enqueue"
	DeleteRegKey HKLM "Software\Classes\Directory\shell\${WINAMP}.Bookmark"
	DeleteRegKey HKLM "Software\Classes\Directory\shell\${WINAMP}.WinampLibrary"
	DeleteRegKey HKLM "Software\Classes\Directory\shell\${WINAMP}.EnqueueAndPlay"

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${WINAMPEXE}"

	DeleteRegKey HKEY_CLASSES_ROOT "UNSV"
	DeleteRegKey HKEY_CLASSES_ROOT "UVOX"
	DeleteRegKey HKEY_CLASSES_ROOT "SHOUT"
	DeleteRegKey HKEY_CLASSES_ROOT "SC"
	DeleteRegKey HKEY_CLASSES_ROOT "ICY"
	DeleteRegKey HKEY_CLASSES_ROOT "winamp"
	; need to get "winamp.exe /unreg" to do this instead...
	; DeleteRegValue HKEY_CLASSES_ROOT "pcast\shell\open\command" "" "$INSTDIR\${WINAMPEXE} /HANDLE %1"
	; DeleteRegValue HKEY_CLASSES_ROOT "feed\shell\open\command" "" "$INSTDIR\${WINAMPEXE} /HANDLE %1"

	DeleteRegKey HKEY_CLASSES_ROOT "CLSID\{46986115-84D6-459c-8F95-52DD653E532E}"
	DeleteRegKey HKEY_CLASSES_ROOT "CLSID\{77A366BA-2BE4-4a1e-9263-7734AA3E99A2}"
	DeleteRegKey HKEY_CLASSES_ROOT "AppID\{7DE5C6C7-DAF2-42F9-9324-C8CF4E7E8AC5}"

	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\MTPMediaPlayerArrival" "${WINAMP}MTPHandler"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler"
	DeleteRegKey HKEY_CLASSES_ROOT "Software\shell\${WINAMP}MTPHandler"

	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayMusicFilesOnArrival" "${WINAMP}PlayMediaOnArrival"
	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayCDAudioOnArrival" "${WINAMP}PlayMediaOnArrival"
	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\PlayVideoFilesOnArrival" "${WINAMP}PlayMediaOnArrival"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}PlayMediaOnArrival"

	DeleteRegKey HKLM "Software\Clients\Media\Winamp"
	DeleteRegValue HKLM "Software\RegisteredApplications" "Winamp"

!ifdef no_old_aod
	ExecWait '"$INSTDIR\AOD\AolAod.exe" -uninstall'
	Delete $INSTDIR\AOD\SOAF\*.*
	RMDIR $INSTDIR\AOD\SOAF
	Delete $INSTDIR\AOD\AOL\*.*
	RMDIR $INSTDIR\AOD\AOL
!endif

	SetShellVarContext current
	Call un.StartMenu_CleanData
	SetShellVarContext all
	Call un.StartMenu_CleanData

	${FrenchRadio_UninstallPlugin}

	; Files
	ClearErrors
	Delete "$INSTDIR\${WINAMPEXE}"
	Delete "$INSTDIR\Winampa.exe"

	Delete "$INSTDIR\Winamp.exe.manifest"
	Delete "$INSTDIR\Winampmb.htm"
	Delete "$INSTDIR\whatsnew.txt"
	Delete "$INSTDIR\fmt.dll"
	Delete "$INSTDIR\winamp.log"
	Delete "$INSTDIR\logs\winamp.log"
	RMDir "$INSTDIR\logs"
	Delete "$INSTDIR\Winamp.lks"
	Delete "$INSTDIR\winamp.pic"
	Delete "$INSTDIR\Winamp.q1"
	Delete "$INSTDIR\demo.aac"
	Delete "$INSTDIR\demo.mp3"
	Delete "$INSTDIR\demoedit.aac"
	Delete "$INSTDIR\pxsdkpls.dll"
	Delete "$INSTDIR\primosdk.dll"
	Delete "$INSTDIR\pconfig.dcf"
	Delete "$INSTDIR\System\primo.w5s"
	Delete "$INSTDIR\UninstWA.exe"
	Delete "$INSTDIR\unicows.dll"
	Delete "$INSTDIR\Databurner.ini"
	Delete "$INSTDIR\libsndfile.dll"
	Delete "$INSTDIR\Shared\libsndfile.dll"
	Delete "$INSTDIR\libFLAC.dll"
	Delete "$INSTDIR\Shared\libFLAC.dll"
	Delete "$INSTDIR\Shared\libFLAC_dynamic.dll"
	Delete "$INSTDIR\burnlib.dll"
	Delete "$INSTDIR\nde.dll"
	Delete "$INSTDIR\Shared\nde.dll"
	Delete "$INSTDIR\libmp4v2.dll"
	Delete "$INSTDIR\Shared\libmp4v2.dll"
	Delete "$INSTDIR\Shared\libalac.dll"
	Delete "$INSTDIR\tataki.dll"
	Delete "$INSTDIR\Shared\tataki.dll"
	Delete "$INSTDIR\zlib.dll"
	Delete "$INSTDIR\Shared\zlib.dll"
	Delete "$INSTDIR\nxlite.dll"
	Delete "$INSTDIR\Shared\nxlite.dll"
	Delete "$INSTDIR\jnetlib.dll"
	Delete "$INSTDIR\Shared\jnetlib.dll"
	Delete "$SETTINGSDIR\winamp.pic" ; deprecated

	;  - even if we do not ship it anymore - still need to remove
	SetShellVarContext all
	; Music Now
	Delete "$INSTDIR\AOL Music Now.ico"
	Delete "$SMPROGRAMS\Winamp\AOL Music Now - 30 Days Free!.lnk"
	Delete "$DESKTOP\30 Days Free! AOL MusicNow.lnk"
	; Active Virus Shield
	Delete "$INSTDIR\avs.ico"
	Delete "$SMPROGRAMS\Winamp\Free Active Virus Shield!.lnk"
	Delete "$DESKTOP\Free Active Virus Shield!.lnk"
	; ASM Bundle
	Delete "$INSTDIR\asm.ico"
	Delete "$SMPROGRAMS\Winamp\Free security diagnostic!.lnk"
	Delete "$DESKTOP\Free security diagnostic!.lnk"

	; MessageBox MB_YESNO|MB_ICONQUESTION $(msgRemoveSettings) /SD IDYES IDNO skip_removeSettings

	${If} ${FileExists} "$INSTDIR\Plugins\in_mjf.dll"
	${AndIfNot} ${Cmd} 'MessageBox MB_YESNO|MB_ICONQUESTION "$(msgRemoveMJUICE)" /SD IDYES IDNO'
		UnRegDLL $SYSDIR\audioexctl.dll ; 1
		Delete $SYSDIR\audioexctl.dll ; 2
		;   DeleteNSPlug npaxdlpi.dll ; 3
		Delete "$PROGRAMFILES\Mjuice Media Player\MJAgent.exe" ; 4
		Delete "$PROGRAMFILES\Mjuice Media Player\MJSecurity.exe" ; 5
		Delete "$PROGRAMFILES\Mjuice Media Player\MJSecurityClient.dll" ; 6
		Delete "$PROGRAMFILES\Mjuice Media Player\MJUnInst.exe" ; 7
		RMDir "$PROGRAMFILES\Mjuice Media Player\Users\MJuiceUser\Config Files" ; 8
		RMDir "$PROGRAMFILES\Mjuice Media Player\Users\MJuiceUser" ; 9
		RMDir "$PROGRAMFILES\Mjuice Media Player\Users" ; 10
		RMDir "$PROGRAMFILES\Mjuice Media Player" ; 11
		DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MJuiceWinamp" ; 12
		DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\MJAgent.exe" ; 13
		DeleteRegKey HKLM "Software\Audio Explosion\MJuice" ; 14
		DeleteRegKey HKLM "Software\Audio Explosion\MJuice" ; 15
	${EndIf}

	; Plugins dir
	UnRegDLL "$INSTDIR\Plugins\in_asfs.dll"
	Delete "$INSTDIR\Plugins\in_asfs.dll"
	Delete "$INSTDIR\Plugins\in_cdda.dll"
	Delete "$INSTDIR\Plugins\in_cdda.cdb"
	Delete "$INSTDIR\Plugins\cdinfo.db3"
	Delete "$INSTDIR\Plugins\in_vorbis.dll"
	Delete "$INSTDIR\Plugins\in_midi.dll"
	Delete "$INSTDIR\Plugins\in_mod.dll"
	Delete "$INSTDIR\Plugins\in_mjf.dll"
	Delete "$INSTDIR\Plugins\in_mp3.dll"
	Delete "$INSTDIR\Plugins\in_wave.dll"
	Delete "$INSTDIR\Plugins\in_wm.dll"
	Delete "$INSTDIR\Plugins\in_flac.dll"
	Delete "$INSTDIR\Plugins\in_flv.dll"
	Delete "$INSTDIR\Plugins\in_swf.dll"
	Delete "$INSTDIR\Plugins\winampFLV.swf"
	Delete "$INSTDIR\Plugins\in_avi.dll"
	Delete "$INSTDIR\Plugins\in_mkv.dll"
	Delete "$INSTDIR\Plugins\enc_flac.dll"
	Delete "$INSTDIR\Plugins\enc_wav.dll"
	Delete "$INSTDIR\Plugins\dsp_sps.dll"
	Delete "$INSTDIR\Plugins\dsp_sps\justin -*.sps"
	Delete "$INSTDIR\Plugins\dsp_sps\cockos -*.sps"
	RMDir "$INSTDIR\Plugins\dsp_sps"
	Delete "$INSTDIR\Plugins\out_disk.dll"
	Delete "$INSTDIR\Plugins\out_ds.dll"
	Delete "$INSTDIR\Plugins\out_wave.dll"
	Delete "$INSTDIR\Plugins\out_wm.dll"
	Delete "$INSTDIR\Plugins\out_wasapi.dll"
	Delete "$INSTDIR\Plugins\read_file.dll"
	Delete "$INSTDIR\Shared\read_file.dll"
	Delete "$INSTDIR\Plugins\vis_avs.dat"
	Delete "$INSTDIR\Plugins\vis_avs.dll"
	Delete "$INSTDIR\Plugins\vis_nsfs.dll"
	Delete "$INSTDIR\Plugins\vis_milk.dll"
	Delete "$INSTDIR\Plugins\vis_milk2.dll"
	Delete "$INSTDIR\Plugins\milkdrop.html"
	Delete "$INSTDIR\Plugins\q_and_t_vars.gif" 
	Delete "$INSTDIR\Plugins\vms_desktop.dll"
	Delete "$INSTDIR\Plugins\milk_config.ini"
	Delete "$INSTDIR\Plugins\milk_img.ini"
	Delete "$INSTDIR\Plugins\milk_msg.ini"
	Delete "$INSTDIR\Plugins\vx*.*"
	Delete "$INSTDIR\Plugins\gen_vx*.d*"
	Delete "$INSTDIR\Plugins\readme_vx*.txt"

	Delete "$INSTDIR\Plugins\avs\*.ape"
	Delete "$INSTDIR\Plugins\avs\*.bmp"
	RMdir /r "$INSTDIR\Plugins\avs\newpicks"
	RMdir /r "$INSTDIR\Plugins\avs\NullsoftPicks"
	RMdir /r "$INSTDIR\Plugins\avs\Winamp 5 Picks"
	RMdir /r "$INSTDIR\Plugins\avs\Community Picks"

	Delete "$INSTDIR\Plugins\gen_ml.dll"
	Delete "$INSTDIR\Plugins\gen_ff.dll"
	Delete "$INSTDIR\Plugins\Freeform\wacs\freetype\freetype.wac"
	Delete "$INSTDIR\Plugins\gen_orgler.dll"
	Delete "$INSTDIR\Plugins\gen_tray.dll"
	Delete "$INSTDIR\Plugins\gen_hotkeys.dll"
	Delete "$INSTDIR\Plugins\in_nsv.dll"
	Delete "$INSTDIR\Plugins\in_dshow.dll"
	Delete "$INSTDIR\Plugins\nsvdec_vp3.dll"
	Delete "$INSTDIR\Plugins\nsvdec_vp5.dll"
	Delete "$INSTDIR\Plugins\nsvdec_vp6.dll"
	Delete "$INSTDIR\Plugins\nsvdec_aac.dll"
	Delete "$INSTDIR\Plugins\enc_aac.dll"
	Delete "$INSTDIR\Plugins\enc_fhgaac.dll"
	Delete "$INSTDIR\Plugins\enc_aacplus.dll"
	Delete "$INSTDIR\Plugins\enc_wma.dll"
	Delete "$INSTDIR\Plugins\*.prx"
	Delete "$INSTDIR\Plugins\enc_vorbis.dll"
	Delete "$INSTDIR\Plugins\enc_lame.dll"
	Delete "$INSTDIR\Shared\lame_enc.dll"
	Delete "$INSTDIR\Plugins\lame_enc.dll"
	Delete "$INSTDIR\Plugins\gen_jumpex.dll"
	Delete "$INSTDIR\Plugins\gen_orgler.dll"
	Delete "$INSTDIR\Plugins\in_mp4.dll"
	Delete "$INSTDIR\Plugins\enc_mp4.dll"
	Delete "$INSTDIR\Plugins\libmp4v2.dll"
	Delete "$INSTDIR\Shared\libmp4v2.dll"
	Delete "$INSTDIR\Shared\libmpg123.dll"
	Delete "$INSTDIR\Plugins\ml_wire.dll"
	Delete "$INSTDIR\Plugins\ml_online.dll"
	Delete "$INSTDIR\Plugins\ml_bookmarks.dll"
	Delete "$INSTDIR\Plugins\ml_history.dll"
	Delete "$INSTDIR\Plugins\ml_local.dll"
	Delete "$INSTDIR\Plugins\ml_nowplaying.dll"
	Delete "$INSTDIR\Plugins\ml_playlists.dll"
	Delete "$INSTDIR\Plugins\ml_rg.dll"
	Delete "$INSTDIR\Plugins\ReplayGainAnalysis.dll"
	Delete "$INSTDIR\Shared\ReplayGainAnalysis.dll"
	Delete "$INSTDIR\Plugins\ml_disc.dll"
	Delete "$INSTDIR\Plugins\ml_xpdxs.dll"
	Delete "$INSTDIR\Plugins\ml_dash.dll"
	Delete "$INSTDIR\Plugins\ml_pmp.dll"
	Delete "$INSTDIR\Plugins\ml_autotag.dll"
	Delete "$INSTDIR\Plugins\ml_orb.dll"
	Delete "$INSTDIR\Plugins\ml_plg.dll"
	Delete "$INSTDIR\Plugins\ml_transcode.dll"
	Delete "$INSTDIR\Plugins\ml_impex.dll"
	Delete "$INSTDIR\Plugins\ml_addons.dll"
	Delete "$INSTDIR\Plugins\ml_downloads.dll"
	Delete "$INSTDIR\Plugins\ml_cloud.dll"
	Delete "$INSTDIR\Plugins\ml_enqplay.dll"
	Delete "$INSTDIR\Plugins\ml_nft.dll"
	Delete "$INSTDIR\Plugins\ml_fanzone.dll"
	Delete "$INSTDIR\Plugins\ml_hotmixradio.dll"
	Delete "$INSTDIR\Plugins\pmp_ipod.dll"
	Delete "$INSTDIR\Plugins\pmp_wifi.dll"
	Delete "$INSTDIR\Plugins\pmp_njb.dll"
	Delete "$INSTDIR\Plugins\pmp_p4s.dll"
	Delete "$INSTDIR\Plugins\pmp_usb.dll"
	Delete "$INSTDIR\Plugins\pmp_usb2.dll"
	Delete "$INSTDIR\Plugins\pmp_android.dll"
	Delete "$INSTDIR\Plugins\pmp_activesync.dll"
	Delete "$INSTDIR\Plugins\pmp_cloud.dll"
	Delete "$INSTDIR\Plugins\in_linein.dll"
	Delete "$INSTDIR\Plugins\Predixis MusicMagic\images\*.*"
	Delete "$INSTDIR\Plugins\Predixis MusicMagic\*.*"
	Delete "$INSTDIR\Plugins\milkdrop_preset_authoring.html"
	Delete "$INSTDIR\Plugins\ml_devices.dll"

	Delete "$INSTDIR\Plugins\gen_talkback.dll"

	Delete "$INSTDIR\Plugins\gen_crasher.dll" 
	Delete "$INSTDIR\Plugins\reporter.exe"
	Delete "$INSTDIR\reporter.exe"
	Delete "$INSTDIR\Plugins\nscrt.dll" 
	Delete "$INSTDIR\Plugins\tataki.dll"
	Delete "$INSTDIR\Plugins\gen_dropbox.dll"

	UnRegDLL "$INSTDIR\Plugins\Gracenote\cddbcontrolwinamp.dll"
	UnRegDLL "$INSTDIR\Plugins\Gracenote\cddbuiwinamp.dll"
	UnRegDLL "$INSTDIR\Plugins\Gracenote\CddbMusicIDWinamp.dll"
	UnRegDLL "$INSTDIR\Plugins\Gracenote\CddbPlaylist2Winamp.dll"
	Delete "$INSTDIR\Plugins\Gracenote\cddbcontrolwinamp.dll"
	Delete "$INSTDIR\Plugins\Gracenote\cddbuiwinamp.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbWOManagerWinamp.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbFPX1.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbAFX3.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbCMSig_1_2.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbCMSig_1_3.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbFEX.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbS12T.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbMusicIDWinamp.dll"
	Delete "$INSTDIR\Plugins\Gracenote\CddbPlaylist2Winamp.dll"
	Delete "$INSTDIR\Plugins\Gracenote\Cddbx*.dll"
	RMDir  "$INSTDIR\Plugins\Gracenote"

	RMDir /r "$INSTDIR\Plugins\Freeform"
	RMDir /r "$INSTDIR\Plugins\milkdrop"
	RMDir /r "$INSTDIR\Plugins\Milkdrop2"
	RMDir /r "$INSTDIR\Plugins\Predixis MusicMagic\images"
	RMDir /r "$INSTDIR\Plugins\Predixis MusicMagic"
	RMDir "$INSTDIR\Plugins\avs"
	RMDir "$INSTDIR\Plugins"

	; System directory
	Delete "$INSTDIR\System\aacPlusDecoder.w5s"
	Delete "$INSTDIR\System\aacPlusDecoder.wbm"
	Delete "$INSTDIR\System\aacdec.w5s"
	Delete "$INSTDIR\System\aacdec.wbm"
	Delete "$INSTDIR\System\mp3.w5s"
	Delete "$INSTDIR\System\mp3.wbm"
	Delete "$INSTDIR\System\h264.w5s"
	Delete "$INSTDIR\System\h264.wbm"
	Delete "$INSTDIR\System\mp4v.w5s"
	Delete "$INSTDIR\System\mp4v.wbm"
	Delete "$INSTDIR\System\a52.w5s"
	Delete "$INSTDIR\System\a52.wbm"
	Delete "$INSTDIR\System\vlb.w5s"
	Delete "$INSTDIR\System\vlb.wbm"
	Delete "$INSTDIR\System\vp6.w5s"
	Delete "$INSTDIR\System\vp6.wbm"
	Delete "$INSTDIR\System\adpcm.w5s"
	Delete "$INSTDIR\System\adpcm.wbm"
	Delete "$INSTDIR\System\pcm.w5s"
	Delete "$INSTDIR\System\pcm.wbm"
	Delete "$INSTDIR\System\jnetlib.w5s"
	Delete "$INSTDIR\System\jnetlib.wbm"
	Delete "$INSTDIR\System\tagz.w5s"
	Delete "$INSTDIR\System\albumart.w5s"
	Delete "$INSTDIR\System\png.w5s"
	Delete "$INSTDIR\System\jpeg.w5s"
	Delete "$INSTDIR\System\bmp.w5s"
	Delete "$INSTDIR\System\gif.w5s"
	Delete "$INSTDIR\System\xml.w5s"
	Delete "$INSTDIR\System\alac.w5s"
	Delete "$INSTDIR\System\alac.wbm"
	Delete "$INSTDIR\System\f263.w5s"
	Delete "$INSTDIR\System\f263.wbm"
	Delete "$INSTDIR\System\playlist.w5s"
	Delete "$INSTDIR\System\watcher.w5s"
	Delete "$INSTDIR\System\filereader.w5s"
	Delete "$INSTDIR\System\filereader.wbm"
	Delete "$INSTDIR\System\db.w5s"
	Delete "$INSTDIR\System\gracenote.w5s"
	Delete "$INSTDIR\System\dlmgr.w5s"
	Delete "$INSTDIR\System\dlmgr.wbm"
	Delete "$INSTDIR\System\timer.w5s"
	Delete "$INSTDIR\System\omBrowser.w5s"
	Delete "$INSTDIR\System\omBrowser.wbm"
	Delete "$INSTDIR\System\auth.w5s"
	Delete "$INSTDIR\System\vp8x.w5s"
	Delete "$INSTDIR\System\vp8x.wbm"
	Delete "$INSTDIR\System\vp8.w5s"
	Delete "$INSTDIR\System\vp8.wbm"
	Delete "$INSTDIR\System\theora.w5s"
	Delete "$INSTDIR\System\theora.wbm"
	Delete "$INSTDIR\System\dca.w5s"
	Delete "$INSTDIR\System\dca.wbm"
	Delete "$INSTDIR\System\devices.w5s"
	Delete "$INSTDIR\System\devices.wbm"
	Delete "$INSTDIR\System\wasabi2.w5s"
	Delete "$INSTDIR\System\xspf.w5s"
	Delete "$INSTDIR\System\UnicodeTaskbarFix.w5s"
	Delete "$INSTDIR\System\wac_network.w5s"
	Delete "$INSTDIR\System\wac_downloadManager.w5s"
	RMDir "$INSTDIR\System"

	; components directory
	Delete "$INSTDIR\Components\cloud.w6c"
	Delete "$INSTDIR\Components\ssdp.w6c"
	RMDir "$INSTDIR\Components"

	; Qt Components
	Delete "$INSTDIR\Qt5Core.dll"
	Delete "$INSTDIR\Qt5Network.dll"
	Delete "$INSTDIR\platforms\qwindows.dll"
	RMDir "$INSTDIR\platforms"
	Delete "$INSTDIR\printsupport\windowsprintersupport.dll"
	RMDir "$INSTDIR\printsupport"
	Delete "$INSTDIR\QtPositioning\declarative_positioning.dll"
	RMDir "$INSTDIR\QtPositioning"

	; Skins
	${Skins_GetPath} $0
	${If} $0 != ""
		${Skins_DeleteFolder} "$0"
	${EndIf}

	${Skins_GetDefaultPath} $0
	${If} $0 != ""
		${Skins_DeleteFolder} "$0"
	${EndIf}

	; Languages
	ReadINIStr $0 "$WINAMPINI" "Winamp" "LangDir"
	${If} $0 == ""
		StrCpy $0 "Lang"
	${EndIf}
	Push $INSTDIR
	Push $0
	nsis_winamp::GetFullPath
	Pop $0

	!insertmacro WALANG_UNINSTALL $0 "ES-US"
	!insertmacro WALANG_UNINSTALL $0 "DE-DE"
	!insertmacro WALANG_UNINSTALL $0 "ES-US"
	!insertmacro WALANG_UNINSTALL $0 "FR-FR"
	;!insertmacro WALANG_UNINSTALL $0 "IT-IT"
	;!insertmacro WALANG_UNINSTALL $0 "NL-NL"
	!insertmacro WALANG_UNINSTALL $0 "PL-PL"
	;!insertmacro WALANG_UNINSTALL $0 "SV-SE"
	!insertmacro WALANG_UNINSTALL $0 "RU-RU"
	;!insertmacro WALANG_UNINSTALL $0 "ZH-CN"
	;!insertmacro WALANG_UNINSTALL $0 "ZH-TW"
	!insertmacro WALANG_UNINSTALL $0 "JA-JP"
	;!insertmacro WALANG_UNINSTALL $0 "KO-KR"
	!insertmacro WALANG_UNINSTALL $0 "TR-TR"
	!insertmacro WALANG_UNINSTALL $0 "PT-BR"
	!insertmacro WALANG_UNINSTALL $0 "RO-RO"
	!insertmacro WALANG_UNINSTALL $0 "HU-HU"
	;!insertmacro WALANG_UNINSTALL $0 "ID-ID"
	RMDir "$0" ; don't try to delete, prompt user later

	Delete $INSTDIR\AOD\*.*
	RMDir $INSTDIR\AOD

	Delete "$INSTDIR\nsutil.dll"
	Delete "$INSTDIR\Shared\nsutil.dll"

	${If} ${AtLeastWinVista}
		ExecWait '"$INSTDIR\elevator.exe" /UnregServer'
		Sleep 1000

		${If} ${FileExists} "$INSTDIR\elevatorps.dll"
			UnRegDLL "$INSTDIR\elevatorps.dll"
			Sleep 1000
		${EndIf}

		${If} ${FileExists} "$INSTDIR\Shared\elevatorps.dll"
			UnRegDLL "$INSTDIR\Shared\elevatorps.dll"
			Sleep 1000
		${EndIf}

		KillProcDLL::KillProc "elevator.exe"
		Sleep 1000
		Delete "$INSTDIR\elevator.exe"
		Delete "$INSTDIR\elevatorps.dll"
		Delete "$INSTDIR\Shared\elevatorps.dll"

		${If} ${FileExists} "$INSTDIR\elevator.exe"
			Sleep 1000
			Delete /REBOOTOK "$INSTDIR\elevator.exe"
		${EndIf}

		${If} ${FileExists} "$INSTDIR\elevatorps.dll"
			Sleep 1000
			Delete /REBOOTOK "$INSTDIR\elevatorps.dll"
		${EndIf}

		${If} ${FileExists} "$INSTDIR\Shared\elevatorps.dll"
			Sleep 1000
			Delete /REBOOTOK "$INSTDIR\Shared\elevatorps.dll"
		${EndIf}
	${EndIf}

	RMDir "$INSTDIR\Shared"

	;Sleep 1000
	Delete /REBOOTOK "$INSTDIR\nscrt.dll"
	RMDir /r /REBOOTOK "$INSTDIR\Microsoft.VC90.CRT"
	RMDir /r /REBOOTOK "$INSTDIR\Microsoft.VC142.CRT"
	Delete "$INSTDIR\msvcr90.dll" ; this will exist on win2k installs

	Delete "$INSTDIR\OpenCandy\*.*"
	RMDir /r "$INSTDIR\OpenCandy"

	;Sleep 1000
	RMDir "$INSTDIR"

	; Deleting Firewall Rules
	Push $0
	Push $1

	StrCpy $0 "$INSTDIR\${WINAMPEXE}"
	ExecDos::exec /NOUNLOAD /TIMEOUT=5000 'netsh advfirewall firewall delete rule name="${WINAMP}" program="$0"' "" ""
	Pop $1
	${If} $1 != 0
		ExecDos::exec /NOUNLOAD /TIMEOUT=5000 'netsh firewall delete allowedprogram program="$0"' "" ""
	${EndIf}

	Pop $1
	Pop $0

	StrCpy $winamp.uninstall.checkFolder "true"

SectionEnd

!endif ;NULLSOFT_WINAMP_UNINSTALLER_PLAYER_HEADER