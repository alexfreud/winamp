SectionGroup $(IDS_GRP_WALIB_PORTABLE) IDX_GRP_WALIB_PORTABLE  ;  Portable Media Player Support
	${WinampSection} "mediaLibraryPortable" $(secPmp) IDX_SEC_ML_PMP                                    ; >>> [pmp main]
		${SECTIONIN_STD}
		SectionGetFlags ${IDX_GRP_WALIB_PORTABLE} $1
		IntOp $1 $1 & 0x0041
		StrCmp $1 "0" done
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\ml_pmp.dll
		File ${FILES_PATH}\Plugins\ml_devices.dll
		SetOutPath $INSTDIR\System
		File ${FILES_PATH}\System\devices.w5s
		; File /nonfatal ${FILES_PATH}\System\devices.wbm
		SetOutPath $INSTDIR\Shared
		File ${FILES_PATH}\Shared\nde.dll
		File ${FILES_PATH}\Shared\nxlite.dll
		File ${FILES_PATH}\Shared\jnetlib.dll
		done:
	${WinampSectionEnd}                                                           ; <<< [pmp main]

	${WinampSection} "portableDeviceIPod" $(secPmpiPod) IDX_SEC_PMP_IPOD                            ; >>> [iPod support]
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\pmp_ipod.dll
		Delete $INSTDIR\Plugins\ml_ipod.dll
	${WinampSectionEnd}                                                          ; <<< [iPod support]

!ifdef full
	${WinampSection} "portableDeviceCreative" $(secPmpCreative) IDX_SEC_PMP_CREATIVE                    ; >>> [Creative]
		${SECTIONIN_FULL}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\pmp_njb.dll
	${WinampSectionEnd}                                                          ; <<< [Creative]
!endif

!ifndef WINAMP64
!ifdef full
	${WinampSection} "portableDeviceP4S" $(secPmpP4S) IDX_SEC_PMP_P4S                              ; >>> [PlayForSure]
		${SECTIONIN_FULL}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\pmp_p4s.dll
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\EventHandlers\MTPMediaPlayerArrival" "${WINAMP}MTPHandler" ""
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "Action" "Open with ${WINAMP}"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "DefaultIcon" "$INSTDIR\${WINAMPEXE},0"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "InitCmdLine" "$INSTDIR\${WINAMPEXE}"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "ProgID" "Shell.HWEventHandlerShellExecute"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "Provider" "${WINAMP}"
		; required on Win7+
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\${WINAMP}MTPHandler" "CLSIDForCancel" "{7DE5C6C7-DAF2-42F9-9324-C8CF4E7E8AC5}"
		WriteRegStr HKEY_CLASSES_ROOT "AppID\{7DE5C6C7-DAF2-42F9-9324-C8CF4E7E8AC5}" "" "${WINAMP}"
		WriteRegStr HKEY_CLASSES_ROOT "AppID\{7DE5C6C7-DAF2-42F9-9324-C8CF4E7E8AC5}" "RunAs" "Interactive User"
		; If no "AccessPermission" & "LaunchPermission" RegStr (REG_BINARY) values exist, then Windows will use default values instead ~ Ref: http://msdn.microsoft.com/en-gb/library/windows/desktop/ms688679%28v%3Dvs.85%29.aspx
	${WinampSectionEnd}                                                      ; <<< [PlayForSure]
!endif
!endif ; WINAMP64

!ifdef full
	${WinampSection} "portableDeviceUsb" $(secPmpUSB) IDX_SEC_PMP_USB                         ; >>> [USB]
		${SECTIONIN_FULL}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\pmp_usb.dll
	${WinampSectionEnd}                                                           ; <<< [USB]
!endif
  
!ifdef full
	${WinampSection} "portableDeviceAndroid" $(secPmpAndroid) IDX_SEC_PMP_ANDROID              ; >>> [Android]
		${SECTIONIN_FULL}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\pmp_android.dll
	${WinampSectionEnd}                                                           ; <<< [Android]
!endif

	${WinampSection} "portableDeviceWifi" $(secPmpWifi) IDX_SEC_PMP_WIFI                            ; >>> [Wifi support]
		${SECTIONIN_STD}
		SetOutPath $INSTDIR\Shared
		File ${FILES_PATH}\Shared\jnetlib.dll
		SetOutPath $INSTDIR\System
		File ${FILES_PATH}\System\wasabi2.w5s
		SetOutPath $INSTDIR\Components
		File ${FILES_PATH}\Components\ssdp.w6c
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\pmp_wifi.dll
		${If} ${FileExists} "$SETTINGSDIR\pmp_wifi.ini"
			Rename "$SETTINGSDIR\pmp_wifi.ini" "$SETTINGSDIR\Plugins\ml\pmp_wifi.ini"
		${EndIf}
	${WinampSectionEnd}                                                          ; <<< [Wifi support]

!ifndef WINAMP64
!ifdef full
	${WinampSection} "portableDeviceActiveSync" $(secPmpActiveSync) IDX_SEC_PMP_ACTIVESYNC                ; >>> [ActiveSync]
		${SECTIONIN_FULL}
		SetOutPath $INSTDIR\Plugins
		File ${FILES_PATH}\Plugins\pmp_activesync.dll
	${WinampSectionEnd}                                                ; <<< [ActiveSync]
!endif
!endif ; WINAMP64
SectionGroupEnd