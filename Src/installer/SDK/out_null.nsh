; Null output plugin example
SetOutPath $INSTDIR\out_null

; project files
File ${SDKPlugins}\out_null\Out_null.dsw
File ${SDKPlugins}\out_null\Out_null.dsp
File ${SDKPlugins}\out_null\Out_null.sln
File ${SDKPlugins}\out_null\Out_null.vcxproj
File ${SDKPlugins}\out_null\Out_null.vcxproj.filters

; source
File ${SDKPlugins}\out_null\main.c

; resources
File ${SDKPlugins}\out_null\out_null.rc
File ${SDKPlugins}\out_null\resource.h
File ${SDKPlugins}\out_null\version.rc2

