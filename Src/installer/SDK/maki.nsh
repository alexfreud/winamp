; media library API
SetOutPath $INSTDIR\maki
File ${PROJECTS}\Wasabi\mc.exe
File ${PROJECTS}\Wasabi\lib\std.mi
File ${PROJECTS}\Wasabi\lib\winampconfig.mi
File ${PROJECTS}\Wasabi\lib\pldir.mi
File ${PROJECTS}\Wasabi\lib\config.mi
File ${PROJECTS}\Wasabi\lib\application.mi

; mc runtime dependency
File ${PROJECTS}\Wasabi\nscrt.dll