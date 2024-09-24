!define WMFDIST  "Windows Media Format"

; ToDo
; Win7 package (wmfdist11): http://www.microsoft.com/en-us/download/details.aspx?id=16546
; Win8 package (wmfdist12): http://www.microsoft.com/en-us/download/details.aspx?id=30685

Function WMF_Download

  Call GetWMFLink
  Exch $0
  ${If} $0 != ""
    DetailPrint "$(IDS_RUN_DOWNLOAD) ${WMFDIST}..."
    NSISdl::download /TRANSLATE2 "$(IDS_RUN_DOWNLOAD) ${WMFDIST}..." "$(IDS_CONNECTING)" "$(IDS_SECOND)" "$(IDS_MINUTE)" "$(IDS_HOUR)" "$(IDS_SECONDS)" "$(IDS_MINUTES)" "$(IDS_HOURS)" "$(IDS_PROGRESS)" /TIMEOUT=30000 "$0" "$PLUGINSDIR\wmfdist.exe"
    Pop $0  ; Get the return value
    ${If} $0 == "success"
      DetailPrint "$(IDS_RUN_DOWNLOADSUCCESS)"
    ${ElseIf} $0 == "cancel"
      DetailPrint "$(IDS_RUN_DOWNLOADCANCELLED)"
    ${Else}
      DetailPrint "$(IDS_RUN_DOWNLOADFAILED) $R0"
    ${EndIf}
  ${EndIf}
  Pop $0

FunctionEnd

Function WMF_Install
  ClearErrors
  IfFileExists "$PLUGINSDIR\wmfdist.exe" 0 wmf_install_end

  DetailPrint "$(IDS_RUN_INSTALL) ${WMFDIST}..."
  SetDetailsPrint none
  ExecWait '$PLUGINSDIR\wmfdist.exe /Q:A' $0
  SetDetailsPrint lastused

  ${If} ${Errors}
    StrCpy $1 "1"
  ${Else}
    ReadRegDWORD $0 HKCU "Software\Microsoft\MediaPlayer\Setup" InstallResult
    IntOp $1 $0 & 0x80000000
    ${If} $0 = 0xd2af9
      SetRebootFlag true
    ${EndIf}
  ${EndIf}
  
  ${If} $1 = "1"
    DetailPrint "$(IDS_RUN_INSTALLFIALED)"
  ${Else}
    DetailPrint "$(IDS_RUN_DONE)"
  ${EndIf}
 wmf_install_end:
FunctionEnd


Function WMF_NeedDownload
	
	DetailPrint "$(IDS_RUN_EXTRACT) ${WMFDIST}..."
	SetDetailsPrint none
	
	${FileIfExist} "$PLUGINSDIR\wmfdist.exe" "..\..\resources\bundles\wmfdist\wmfdist.exe"
	
	SetDetailsPrint lastused

	${Unless} ${FileExists} "$PLUGINSDIR\${WMFDIST}"
		DetailPrint "$(IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD)"
		
		Call GetWMFLink
		Exch $0
		${If} $0 != ""
			StrCpy $0 "1"
		${Else}
			StrCpy $0 "0"
		${EndIF}
		Exch $0
    ${Else}
		DetailPrint "$(IDS_RUN_DONE)"
		Push "0"
    ${EndUnless}
FunctionEnd

;------------------------
; return link to the wmf distributives or '' (empty string) if you don't need to download
;------------------------

Function GetWMFLink

	Push $0
	Push $R0
	Push $R1
	Push $R2
	Push $R3
	Push $R4
	Push $R5

	; check if wma/wmv selected
	${IfNot} ${SectionIsSelected} ${IDX_SEC_WMA_DEC}
  !ifdef IDX_SEC_WMV_DEC
	${AndIfNot} ${SectionIsSelected} ${IDX_SEC_WMV_DEC}
  !endif
  !ifdef IDX_SEC_WMA_ENC
	${AndIfNot} ${SectionIsSelected} ${IDX_SEC_WMA_ENC}
  !endif	
		Goto no_download
	${EndIf}

	GetDllVersion "$SYSDIR\wmvcore.dll" $R0 $R1
	IntOp $R2 $R0 / 0x00010000
	IntOp $R3 $R0 & 0x0000FFFF
	IntOp $R4 $R1 / 0x00010000
	IntOp $R5 $R1 & 0x0000FFFF
	; StrCpy $1 "$R2.$R3.$R4.$R5"

	; StrCmp $WinVer "ME" wmf90_link
	; TODO check this out!
	/*StrCmp $WinVer "NT" wmf90_link
	StrCmp $WinVer "2000" wmf90_link
	StrCmp $WinVer "XP" wmf95_link
	StrCmp $WinVer "2003" wmf95_link*/
	Goto no_download

;wmf95_link:
	IntCmp $R2 10 +1 copy_link1 no_download
	IntCmp $R3 0 +1 copy_link1 no_download
	IntCmp $R4 0 +1 copy_link1 no_download
	IntCmp $R5 2802 no_download copy_link1 no_download
copy_link1:
	StrCpy $0  https://download.nullsoft.com/redist/wm/wmfdist95.exe
	Goto function_end
;wmf90_link:
	IntCmp $R2 9 +1 copy_link2 no_download
	IntCmp $R3 0 +1 copy_link2 no_download
	IntCmp $R4 0 +1 copy_link2 no_download
	IntCmp $R5 3287 no_download copy_link2 no_download
copy_link2:
	StrCpy $0  https://download.nullsoft.com/redist/wm/wmfdist9.exe
	Goto function_end

no_download:
    StrCpy $0 ''
    goto function_end
    
function_end:
    Pop $R5
    Pop $R4
    Pop $R3
    Pop $R2
    Pop $R1
    Pop $R0
    Exch $0

FunctionEnd
