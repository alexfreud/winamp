Name "CDDB2 test, beta 1"

; The file to write
OutFile "cddb2.exe"

InstallDir $PROGRAMFILES\Winamp
InstallDirRegKey HKLM \
                 "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" \
                 "UninstallString"

; The text to prompt the user to enter a directory
DirText "Please select your Winamp path below (you will be able to proceed when Winamp is detected):"
DirShow hide

; automatically close the installer when done.
AutoCloseWindow true
; hide the "show details" box
ShowInstDetails nevershow

BGGradient 000000 308030 FFFFFF
InstallColors FF8080 000000
InstProgressFlags smooth colored

Function .onInit
  MessageBox MB_YESNO|MB_ICONQUESTION "Install CDDB2 update test?" IDYES update
   MessageBox MB_OK|MB_ICONINFORMATION "Install aborted."
   Abort
  update:
FunctionEnd

Function .onVerifyInstDir
  IfFileExists $INSTDIR\Winamp.exe Good
    Abort
  Good:
FunctionEnd

Function CloseWinamp
  Push $0
  loop:
    FindWindow $0 "Winamp v1.x"
    IntCmp $0 0 done
     SendMessage $0 16 0 0
     StrCpy $9 "yes"
     Sleep 100
     Goto loop
  done:
  Pop $0
FunctionEnd


Section "ThisNameIsIgnoredSoWhyBother?"
  StrCpy $9 "no"
  Call CloseWinamp
  SetOutPath $INSTDIR
  File "C:\program files\winamp\winamp.exe"
  SetOutPath $INSTDIR\Plugins

  UnRegDll $OUTDIR\cddbcontrolwinamp.dll
  UnRegDll $OUTDIR\cddbuiwinamp.dll
  File "C:\program files\winamp\plugins\in_cdda.dll"
  File "C:\program files\winamp\plugins\in_mp3.dll"
  File "cddbcontrolwinamp.dll"
  File "cddbuiwinamp.dll"
  RegDll $OUTDIR\cddbcontrolwinamp.dll
  RegDll $OUTDIR\cddbuiwinamp.dll

  DetailPrint Completed.
SectionEnd


Function .onInstSuccess
  MessageBox MB_OK|MB_ICONINFORMATION "Update installed."
  StrCmp $9 "no" nope
    Exec '"$INSTDIR\Winamp.exe"'
  nope:
FunctionEnd

; eof
