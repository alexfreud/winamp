; XML Parser / Media Library example
SetOutPath $INSTDIR\ml_iso

; project files
File ${SDKPlugins}\ml_iso\ml_iso.sln
File ${SDKPlugins}\ml_iso\ml_iso.vcxproj
File ${SDKPlugins}\ml_iso\ml_iso.vcxproj.filters

; source
File ${SDKPlugins}\ml_iso\main.cpp
File ${SDKPlugins}\ml_iso\main.h
File ${SDKPlugins}\ml_iso\ToISO.cpp
File ${SDKPlugins}\ml_iso\api.h

; resource files
File ${SDKPlugins}\ml_iso\resource.h
File ${SDKPlugins}\ml_iso\ml_iso.rc
File ${SDKPlugins}\ml_iso\version.rc2