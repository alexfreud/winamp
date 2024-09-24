; Tone input plugin
SetOutPath $INSTDIR\in_tone

; project files
File ${SDKPlugins}\in_tone\IN_TONE.sln
File ${SDKPlugins}\in_tone\IN_TONE.vcxproj
File ${SDKPlugins}\in_tone\IN_TONE.vcxproj.filters

; source
File ${SDKPlugins}\in_tone\MAIN.C

; resources
File ${SDKPlugins}\in_tone\in_tone.rc
File ${SDKPlugins}\in_tone\resource.h
File ${SDKPlugins}\in_tone\version.rc2
