; Cover Directory example component
SetOutPath $INSTDIR\coverdirectory

; project files
File ${SDKPlugins}\coverdirectory\coverdirectory.sln
File ${SDKPlugins}\coverdirectory\cover_directory.vcxproj
File ${SDKPlugins}\coverdirectory\cover_directory.vcxproj.filters

; source
File ${SDKPlugins}\coverdirectory\CoverDirectory.cpp
File ${SDKPlugins}\coverdirectory\CoverDirectory.h
File ${SDKPlugins}\coverdirectory\main.cpp
File ${SDKPlugins}\coverdirectory\api.h

; resources
File ${SDKPlugins}\coverdirectory\coverdirectory.rc
File ${SDKPlugins}\coverdirectory\resource.h
File ${SDKPlugins}\coverdirectory\version.rc2