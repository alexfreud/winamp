SetOutPath $INSTDIR\plLoadEx

; project files
File ${SDKPlugins}\plLoadEx\plLoadEx.sln
File ${SDKPlugins}\plLoadEx\plLoadEx.vcxproj
File ${SDKPlugins}\plLoadEx\plLoadEx.vcxproj.filters

; source files
File ${SDKPlugins}\plLoadEx\ExComponent.cpp
File ${SDKPlugins}\plLoadEx\ExComponent.h
File ${SDKPlugins}\plLoadEx\SimpleHandler.cpp
File ${SDKPlugins}\plLoadEx\SimpleHandler.h
File ${SDKPlugins}\plLoadEx\SimpleHandlerFactory.cpp
File ${SDKPlugins}\plLoadEx\SimpleHandlerFactory.h
File ${SDKPlugins}\plLoadEx\SimpleLoader.cpp
File ${SDKPlugins}\plLoadEx\SimpleLoader.h
File ${SDKPlugins}\plLoadEx\w5s.cpp
File ${SDKPlugins}\plLoadEx\api.h

; resources
File ${SDKPlugins}\plLoadEx\plLoadEx.rc
File ${SDKPlugins}\plLoadEx\resource.h
File ${SDKPlugins}\plLoadEx\version.rc2
File ${SDKPlugins}\plLoadEx\example.simple