; IRC tell example component
SetOutPath $INSTDIR\irctell

; project files
File ${SDKPlugins}\irctell\irctell.sln
File ${SDKPlugins}\irctell\irctell.vcxproj
File ${SDKPlugins}\irctell\irctell.vcxproj.filters

; source
File ${SDKPlugins}\irctell\dde.cpp
File ${SDKPlugins}\irctell\dde.h
File ${SDKPlugins}\irctell\irctell.cpp
File ${SDKPlugins}\irctell\irctell.h
File ${SDKPlugins}\irctell\api.h

; resources
File ${SDKPlugins}\irctell\irctell.rc
File ${SDKPlugins}\irctell\resource.h
File ${SDKPlugins}\irctell\version.rc2

