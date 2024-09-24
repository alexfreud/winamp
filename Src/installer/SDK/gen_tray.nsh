; System Tray plugin
SetOutPath $INSTDIR\gen_tray

; project files
File ${GenPlugins}\gen_tray\GEN_TRAY.sln
File ${GenPlugins}\gen_tray\GEN_TRAY.vcxproj
File ${GenPlugins}\gen_tray\GEN_TRAY.vcxproj.filters

; source
File ${GenPlugins}\gen_tray\TRAYCTL.C
File ${GenPlugins}\gen_tray\WINAMPCMD.H
File ${GenPlugins}\gen_tray\api.h

; icons
SetOutPath $INSTDIR\gen_tray\icons
File ${GenPlugins}\gen_tray\icons\compact.bmp
File ${GenPlugins}\gen_tray\icons\icon1.ico
File ${GenPlugins}\gen_tray\icons\icon2.ico
File ${GenPlugins}\gen_tray\icons\icon3.ico
File ${GenPlugins}\gen_tray\icons\icon4.ico
File ${GenPlugins}\gen_tray\icons\icon5.ico
File ${GenPlugins}\gen_tray\icons\icon7.ico
File ${GenPlugins}\gen_tray\icons\icon8.ico
File ${GenPlugins}\gen_tray\icons\icon9.ico

SetOutPath $INSTDIR\gen_tray

; resources
File ${GenPlugins}\gen_tray\RESOURCE.H
File ${GenPlugins}\gen_tray\gen_tray.rc
File ${GenPlugins}\gen_tray\version.rc2
