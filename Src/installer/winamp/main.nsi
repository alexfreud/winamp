Unicode true
!ifdef _DEBUG
	SetCompress off
	FileBufSize 64
	SetDatablockOptimize off
!else
	SetDatablockOptimize on
	!ifdef lzma
		SetCompressor /SOLID lzma
		SetCompressorDictSize 16
		FileBufSize 64
	!else
		SetCompressor /SOLID bzip2 ; actually doesnt seem to help :(
	!endif
!endif

ReserveFile /plugin "LangDLL.dll"
ReserveFile /plugin "nsDialogs.dll"
ReserveFile /plugin "System.dll"

; temp hack 2022 (hard-coded to 32-bit release build - might need e.g. a Winamp_x64_Release version later)
!ifndef FILES_PATH
	!define FILES_PATH ..\..\..\Build\Winamp_x86_Release
; change to this when copied to Winamp-Desktop\Src\Build\Winamp_x86_Release\installer\winamp
;	!define FILES_PATH ..\..\..\Winamp_x86_Release
!endif

!define MUI_LANGDLL_REGISTRY_ROOT HKLM
!define MUI_LANGDLL_REGISTRY_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "LangId"

!define MUI_LANGDLL_WINDOWTITLE $(LANGUAGE_DLL_TITLE)
!define MUI_LANGDLL_INFO $(LANGUAGE_DLL_INFO)

!addincludedir "..\shared\scripts"

!include "MUI2.nsh"
!include "WordFunc.nsh"
!include "WinMessages.nsh"
!include "LogicLib.nsh"
!include ".\parameters.nsh"
!include ".\branding.nsh"
!include "verInfo.nsh"
!include "WinVer.nsh"

Var SETTINGSDIR
Var INSTINI
Var WINAMPINI
Var WINAMPM3U
Var M3UBASEDIR
Var PREVINSTINI
Var RESTARTAGENT
Var FIRSTINSTALL
Var needplaystart
Var IDX_INSTTYPE_STANDARD
Var IDX_INSTTYPE_PREVIOUS
Var IDX_INSTTYPE_FULL
Var /GLOBAL VERSION_REGISTRY
Var /GLOBAL VERSION_MINIMUM

!include ".\utils\wafuncs.nsh"

!ifndef _DEBUG
;	BrandingText "Winamp ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MINOR_SECOND} ${InstallType} -- $(BuiltOn) ${__DATE__} $(at) ${__TIME__}"
	BrandingText "Winamp ${VERSION_MAJOR}.${VERSION_MINOR} ${InstallType} -- $(BuiltOn) ${__DATE__} $(at) ${__TIME__}"
!else
	BrandingText "Winamp Debug -- internal use only"
!endif

RequestExecutionLevel admin
Caption $(IDS_CAPTION)
Name "${WINAMP}"
InstallDir "$PROGRAMFILES\${WINAMPFOLDER}"
InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${WINAMP}" "UninstallString"

XPStyle on

!define OVERWRITEMODE on

!ifdef _DEBUG
	ShowInstDetails show
	AutoCloseWindow false
!else
	ShowInstDetails nevershow
	AutoCloseWindow true
!endif

SetDateSave on

!define SECTIONIN_FULL "SectionIn 1"

!ifdef FULL
	!define SECTIONIN_STD "SectionIn 1 2"
	!define SECTIONIN_LITE "SectionIn 1 2 3"
!else
	!define SECTIONIN_STD "SectionIn 1"
	!define SECTIONIN_LITE "SectionIn 1"
!endif

!ifndef INSTALL_NAME
	!define INSTALL_NAME "winamp${VERSION_MAJOR}${VERSION_MINOR}${VERSION_MINOR_SECOND}"
!endif

!ifdef _DEBUG
	OutFile "wasetup_dbg${LANG_FILESPEC}.exe"
!else
	!ifdef LITE
		OutFile "$%INSTALL_NAME%_lite${LANG_FILESPEC}.exe"
	!else ifdef full
		OutFile "$%INSTALL_NAME%_full${LANG_FILESPEC}.exe"
	!else
		OutFile 'winamp_unknown${LANG_FILESPEC}.exe'
	!endif
!endif

!include ".\ui.nsh"
!include ".\sectionsHelper.nsh"
!include ".\wasections.nsh"
!include ".\uninstall\uninstall.nsh"
!include ".\languages.nsh"

Section -LastSection IDX_SECTION_LAST ; keep last section after languages
SectionEnd

!include ".\uiEvents.nsh"
!include ".\descriptionTable.nsh"

Function .onInit
	!ifdef WINAMP64
	ReadEnvStr $0 PROGRAMW6432
	StrCpy $INSTDIR "$0\${WINAMPFOLDER}"
	!endif

	InitPluginsDir

	System::Call "User32::SetProcessDPIAware()"

	StrCpy $PREVINSTINI  ""
	StrCpy $INSTINI  "$PLUGINSDIR\install.ini"

	!ifdef LANGID
		WriteRegStr "${MUI_LANGDLL_REGISTRY_ROOT}" "${MUI_LANGDLL_REGISTRY_KEY}" "${MUI_LANGDLL_REGISTRY_VALUENAME}" ${LANGID}
	!endif ; LANGID

	!insertmacro MUI_LANGDLL_DISPLAY

	; check Windows version
	; this will need testing...
	${If} ${AtMostWin7}
		${If} ${IsWin7}
		${AndIf} ${AtLeastServicePack} 1
		${Else}
			MessageBox MB_OK|MB_ICONEXCLAMATION "$(IDS_MSG_WINDOWS_TOO_OLD)" /SD IDOK
			Quit
		${EndIf}
	${EndIf}

; VC142 runtime required for all installations - pt.1: external
/*	ReadRegStr $0 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86" "Version"
	${If} $0 == "v14.29.30139.00"
	${OrIf} $0 == "v14.30.30423.00"
	${OrIf} $0 == "v14.30.30528.00"
	${OrIf} $0 == "v14.30.30704.00"
	${OrIf} $0 == "v14.30.*"
	${OrIf} $0 == "v14.31.30818.00"
	${OrIf} $0 == "v14.31.30919.00"
	${OrIf} $0 == "v14.31.31005.00"
	${OrIf} $0 == "v14.31.31103.00"
	${OrIf} $0 == "v14.31.*"
	${OrIf} $0 == "v14.32.31302.00"
	${OrIf} $0 == "v14.32.31326.00"
	${OrIf} $0 == "v14.32.31332.00"
	${OrIf} $0 == "v14.32.*"
	${OrIf} $0 == "v14.33.31424.00"
	${OrIf} $0 == "v14.33.*"
	${OrIf} $0 == "v14.34.*"
	Goto continue
*/
	; Get Visual Studio Version
	DetailPrint "Getting Visual Studio Version string from registry..."
	ReadRegStr $0 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86" "Version"
	DetailPrint "Visual Studio Version String: $0"
	DetailPrint "Done!"
	DetailPrint ""

	; Replace v in Version String
	DetailPrint "Replacing v substring in version string..."
	${WordReplace} "$0" "v" "" "+" $0
	DetailPrint "Visual Studio Version: $0"
	DetailPrint "Done!"
	DetailPrint ""

	; Compare Versions
	DetailPrint "Comparing Visual Studio Registry and Minimal Versions..."
	; ${VersionCompare} "[Version1]" "[Version2]" $var
	; "[Version1]"      ; REGISTRY VERSION
	; "[Version2]"      ; MINIMUM VERSION
	; $var              ; Result:
						;    $var=0  Versions are equal
						;    $var=1  Version1 is newer
						;    $var=2  Version2 is newer


	; Visual Studio Registry Version
	StrCpy $VERSION_REGISTRY $0
	DetailPrint "	Registry Version: $VERSION_REGISTRY"

	; Visual Studio Minimal Version
	StrCpy $VERSION_MINIMUM "14.29.30139.00" ; MINIMUM VERSION!!!
	; ===========================================================
	; --- DEBUG ---
	;StrCpy $VERSION_MINIMUM "14.29.30139.00" ; REGISTRY > MINIMUM
	;StrCpy $VERSION_MINIMUM "14.32.31332.00" ; REGISTRY = MINIMUM
	;StrCpy $VERSION_MINIMUM "14.33.31424.00" ; REGISTRY < MINIMUM -> INSTALLATION REQUIRED!
	; --- DEBUG ---
	; ===========================================================
	DetailPrint "	Minimum Version: $VERSION_MINIMUM"

	${VersionCompare} "$VERSION_REGISTRY" "$VERSION_MINIMUM" $R0
	DetailPrint "Version Compare Result: $R0"

	${If} $R0 == 0
	${OrIf} $R0 == 1
		; Visual C++ Redistributable for Visual Studio 2015 Installation NOT Required!
		${If} $R0 == 0
			DetailPrint "VERSIONS ARE EQUAL ($VERSION_REGISTRY = $VERSION_MINIMUM)"
			DetailPrint "Visual C++ Redistributable for Visual Studio 2015 Installation NOT Required!"
		${Else}
			DetailPrint "REGISTRY VERSION ($VERSION_REGISTRY) > MINIMUM VERSION ($VERSION_MINIMUM)"
			DetailPrint "Visual C++ Redistributable for Visual Studio 2015 Installation NOT Required!"
		${EndIf}
	${Else}
		; Visual C++ Redistributable for Visual Studio 2015 Installation Required!
		DetailPrint "REGISTRY VERSION ($VERSION_REGISTRY) < MINIMUM VERSION ($VERSION_MINIMUM)"
		DetailPrint "Visual C++ Redistributable for Visual Studio 2015 Installation Required!"
		
		; Code for Download and Install vc_redist.x86.exe...

		MessageBox MB_YESNO|MB_ICONQUESTION "$(IDS_MSG_VC_REDIST_LINK_TO_MSDOWNLOAD)$\r$\n$\r$\n$(IDS_VC_REDIST_MSDOWNLOAD_URL)$\r$\n$\r$\n$(IDS_VC_DOWNLOAD)$\r$\n$(IDS_VC_DOWNLOAD_OR_ABORT)" /SD IDYES IDYES download IDNO abortInstaller

			download:
				SetOutPath "$TEMP"
				NSISdl::download "https://aka.ms/vs/16/release/vc_redist.x86.exe" "$TEMP\vc_redist.x86.exe"
				ExecWait "$TEMP\vc_redist.x86.exe"
				Delete "$TEMP\vc_redist.x86.exe"
				Goto continue

			abortInstaller:
				Quit

			continue:

	${EndIf}

	${FrenchRadio_OnInit}
	${If} ${AtLeastWin7}
		${ExpressMode_Initialize}
	${EndIf}
FunctionEnd

Function .onInstSuccess
	Delete $PREVINSTINI
	Call SaveSelection
	DeleteINISec "$INSTINI" "Bundle"
	CopyFiles /SILENT "$INSTINI" "$INSTDIR\install.ini"
FunctionEnd

Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
	StrCpy $winamp.uninstall.checkFolder "false"
FunctionEnd

${InitializeGetSectionName}

Function GetLastInstTypeIndex
  Push $R0
  Push $R1
  StrCpy $R0 0
loop:
  InstTypeGetText $R0 $R1
  StrCmp $R1 '' +3
    IntOp $R0 $R0 + 1
    goto loop
  Pop $R1
  IntOp $R0 $R0 - 1
  Exch $R0
FunctionEnd

Function ReadSections
  Push $R0
  Push $R1
  Push $R2
  Push $R3
  Push $R4
  Push $R5
  Push $R6
  Push $R7
  Push $R8
  StrCpy $R1 ${IDX_SECTION_LAST}
  Call GetLastInstTypeIndex
  Pop $R5
  StrCpy $R0 1
  IntOp $R6 $R0 << $R5
  IntOp $R6 $R6 ~

  StrCpy $R0 0
  
  ReadINIStr $R8 "$INSTINI" "installer" "sectionsVer"
  
  loop:
  
	!ifdef IDX_SEC_ML
		StrCmp $R0 ${IDX_SEC_ML} skipRead
	!endif
	
	!ifdef IDX_SEC_ML_PMP
		StrCmp $R0 ${IDX_SEC_ML_PMP} skipRead
	!endif	
		   
	${If} $R8 >= 2
		${GetSectionName} $R0 $R2
	${Else}
		SectionGetText $R0 $R2
	${EndIf}
	
    StrCmp $R2 "-" skipRead
    StrCmp $R2 "" skipRead

    SectionGetFlags $R0 $R3
    IntOp $R3 $R3 & 0x00000017
    IntCmp $R3 1 gotoRead gotoRead
    IntOp $R7 $R3 & 0x00000002  ; check if it is section
    IntCmp $R7 0 +2
    StrCpy $R7 $R0  ;remember section index (we can use it later to make it bold)
    goto skipRead ; skip sections, section end and readonly
gotoRead:
    ReadINIStr $R4 "$INSTINI" "sections" $R2
  ;  MessageBox MB_OK "$R2 - $R4"
    StrCmp $R4 "" 0 normalRead ;new feature or not?
    IntOp $R4 0x00000000 | 0x0000009 ; new feature  make it selected and bold
    SectionGetFlags $R7 $0
    IntOp $0 $0 | 0x0000008
    SectionSetFlags $R7 $0
normalRead:

	IntOp $R3 $R3 & 0xfffffffe
	IntOp $R3 $R3 | $R4
	SectionSetFlags $R0 $R3

	SectionGetInstTypes $R0 $R3
	IntOp $R3 $R3 & $R6
	IntOp $R4 $R4 << $R5
	IntOp $R3 $R3 | $R4
	SectionSetInstTypes $R0 $R3

skipRead:
	IntOp $R0 $R0 + 1
	IntCmp $R0 $R1 "" loop
	Pop $R8
	Pop $R7
	Pop $R6
	Pop $R5
	Pop $R4
	Pop $R3
	Pop $R2
	Pop $R1
	Pop $R0
FunctionEnd

Function SaveSelection
  Push $R0
  Push $R1
  Push $R2
  Push $R3
  StrCpy $R1 ${IDX_SECTION_LAST}
  StrCpy $R0 0
  
  WriteINIStr "$INSTINI" "installer" "sectionsVer" "2"
  DeleteINISec "$INSTINI" "sections"

	loop:
    !ifdef FULL
     	StrCmp $R0 ${IDX_SEC_ML} 0 ignore_1
    	StrCmp $R0 ${IDX_SEC_ML_PMP} 0 ignore_1
    !endif ; FULL
  	${GetSectionName} $R0 $R2
   	SectionGetFlags $R0 $R3
    IntOp $R3 $R3 & 0x0001
    goto force_write
!ifdef FULL
 ignore_1:
!endif ; FULL
    ${GetSectionName} $R0 $R2
    StrCmp $R2 "-" skipWrite
    StrCmp $R2 "" skipWrite
   
    SectionGetFlags $R0 $R3
    IntOp $R3 $R3 & 0x00000017
    IntCmp $R3 1 0 0 skipWrite ; skip sections, section end and readonly
 force_write:
    WriteINIStr "$INSTINI" "sections" $R2 $R3
skipWrite:
    IntOp $R0 $R0 + 1
    IntCmp $R0 $R1 "" loop
  Pop $R3
  Pop $R2
  Pop $R1
  Pop $R0
FunctionEnd

!ifdef FULL
Function UpdateAutoSelectSection

	Exch $1    ; section id
	Exch
	Exch $0    ; group id
	Exch
	Push $2
	Push $3
	Push $4
	Push $5

	StrCpy $2 "1" ; - nesting counter
	StrCpy $3 "0" ; - sel counter
	IntOp $0  $0 + 1

	${Do}
		SectionGetFlags $0 $4
		IntOp $5 $4 & 0x0002  ; test if group start
		${If} $5 <> 0
			IntOp $2  $2 + 1
		${EndIf}
    
		IntOp $5 $4 & 0x0004  ; test if group end
		${If} $5 <> 0
			IntOp $2  $2 - 1
			${If} $2 == 0
				${ExitDo}
			${EndIf}
		${Else}
			IntOp $5 $4 & 0x0041  ; test if selected or partial selected
			${If} $5 <> 0
			${AndIf} $0 <> $1
				StrCpy $3  "1"
				${ExitDo}
			${EndIf}
		${EndIf}

		IntOp $0  $0 + 1
	${Loop}
  
	SectionGetFlags $1 $4
	IntOp $4 $4 & 0xFFEF
	${If} $3 <> 0
		IntOp $4 $4 | 0x0011
	${EndIf}
	SectionSetFlags $1 $4

	Pop $5
	Pop $4
	Pop $3
	Exch $2
	Exch
	Exch $1
	Exch 2
	Exch $0
FunctionEnd

!macro UpdateAutoSelectSection __groupId __sectionId
	Push ${__groupId}
	Push ${__sectionId}
	Call UpdateAutoSelectSection
!macroend
!define UpdateAutoSelectSection "!insertmacro 'UpdateAutoSelectSection'"
!endif

Function .onSelChange
	!ifdef FULL
	${UpdateAutoSelectSection} ${IDX_GRP_WALIB_PORTABLE} ${IDX_SEC_ML_PMP}
	${UpdateAutoSelectSection} ${IDX_GRP_WALIB} ${IDX_SEC_ML}
	!endif
FunctionEnd
