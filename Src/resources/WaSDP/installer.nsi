; Winamp Skin Development Pack Installer

;--------------------------------
;Include Modern UI

!include "MUI.nsh"

;--------------------------------

; The name of the installer
Name "Winamp Skin Development Pack v5.9"

; The file to write
OutFile "WinampSDP_59.exe"

; The default installation directory
InstallDir $PROFILE\WinampSDP

; The text to prompt the user to enter a directory
DirText "Select the installation folder for the Winamp Skin Development Pack:"

; automatically close the installer when done.
AutoCloseWindow false

; hide the "show details" box
ShowInstDetails show

SetCompressor /SOLID lzma

;--------------------------------
;Interface Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_RIGHT
  !define MUI_HEADERIMAGE_BITMAP "modern-header.BMP"
  !define MUI_ABORTWARNING
  !define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\classic-install.ico"
  !define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\classic-uninstall.ico"

;--------------------------------

Function .onInit
        # the plugins dir is automatically deleted when the installer exits
        InitPluginsDir
        File /oname=$PLUGINSDIR\splash.bmp "splash.BMP"
        advsplash::show 1000 600 400 0x04025C $PLUGINSDIR\splash
        Pop $0 

        Delete $PLUGINSDIR\splash.bmp
FunctionEnd

;--------------------------------

;Pages

  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install

Section "Maki Compiler" SecCompiler

  DetailPrint "Installing Maki Compiler..."
  SetOutPath "$INSTDIR\"
  File "..\..\Wasabi\mc.exe"
  File "..\..\Wasabi\nscrt.dll"

SectionEnd

Section "Maki Standard Libraries" SecLibs

  DetailPrint "Installing Maki Standard Libraries..."
  SetOutPath "$INSTDIR\lib"
  File /x "private.mi" "..\..\Wasabi\lib\*.m*"

SectionEnd

Section "Maki Community Scripts" SecLibsCom

  DetailPrint "Installing Maki Community Scripts..."
  SetOutPath "$INSTDIR\lib\com"
  File /x "private.mi" "..\..\Wasabi\lib\com\*.m*"

SectionEnd

Section "Winamp Bento Source" SecSkinBento

  DetailPrint "Installing Winamp Bento Source Code..."
  SetOutPath "$INSTDIR\Skins\Big Bento"
  File /r /x "about.m" /x "nibbles.m" "..\skins\Big Bento\*.m"
  SetOutPath "$INSTDIR\Skins\Bento"
  File /r "..\skins\Bento\*.m"

SectionEnd

Section "Winamp Modern Source" SecSkinModern

  DetailPrint "Installing Winamp Modern Source Code..."
  SetOutPath "$INSTDIR\Skins\Winamp Modern"
  File /r "..\skins\Winamp Modern\*.m"

SectionEnd

# Where is the source code for ConsoleFile.w5s? This old version does not work with 5.9 :-(
/* Section "Wasabi Debugger" SecDebugger

  DetailPrint "Installing Wasabi Debugger..."
  SetOutPath "$INSTDIR\system"
  File "ConsoleFile.w5s"

SectionEnd */

Section "Edit Plus Syntax Libs" SecEditplus

  DetailPrint "Installing Edit Plus Syntax..."
  SetOutPath "$INSTDIR"
  File "Maki.*"

SectionEnd

Section ""

  SetOutPath "$INSTDIR"
  File "wasdp_readme.txt"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall_WaSDP.exe"

  ExecShell "open" "$INSTDIR\wasdp_readme.txt"
 
SectionEnd
;--------------------------------

;Descriptions

  ;Language strings

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCompiler} "This tool (mc.exe) is needed to compile *.m files to *.maki files."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLibs} "Standard Maki Libraries."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLibsCom} "Some maki scripts done by the Winamp community."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSkinModern} "Install Winamp Modern Skin Maki source code."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSkinBento} "Install Winamp Bento Maki source code."
  ;!insertmacro MUI_DESCRIPTION_TEXT ${SecDebugger} "Wasabi Debugger will print debug strings to c:\wasabi.log"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecEditplus} "This will install Edit Plus Syntax Libs. For more info see readme.txt"
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\mc.exe"
  Delete "$INSTDIR\Maki.*"
  Delete "$INSTDIR\wasdp_readme.txt"
  Delete "$INSTDIR\system\ConsoleFile.w5s"
  Delete "$INSTDIR\Skins\Winamp Modern\scripts\*.m"
  Delete "$INSTDIR\Skins\Big Bento\scripts\*.m"
  Delete "$INSTDIR\Skins\Bento\scripts\*.m"
  Delete "$INSTDIR\Skins\Big Bento\about\*.m"
  RMDir /r "$INSTDIR\lib"
  RMDir /r "$INSTDIR\Skins\Bento\scripts\mcvcore"
  RMDir /r "$INSTDIR\Skins\Big Bento\scripts\mcvcore"
  RMDir /r "$INSTDIR\Skins\Big Bento\scripts\lib"
  RMDir /r "$INSTDIR\Skins\Big Bento\scripts\suicore"
  RMDir /r "$INSTDIR\Skins\Big Bento\scripts\attribs"
  Delete "$INSTDIR\Uninstall_WaSDP.exe"

SectionEnd
