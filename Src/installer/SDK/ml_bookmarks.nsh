; Nullsoft Bookmarks / Media Library example
SetOutPath $INSTDIR\ml_bookmarks

; project files
File ${LibPlugins}\ml_bookmarks\ml_bookmarks.sln
File ${LibPlugins}\ml_bookmarks\ml_bookmarks.vcxproj
File ${LibPlugins}\ml_bookmarks\ml_bookmarks.vcxproj.filters

; source
File ${LibPlugins}\ml_bookmarks\main.h
File ${LibPlugins}\ml_bookmarks\main.cpp
File ${LibPlugins}\ml_bookmarks\api.h
File ${LibPlugins}\ml_bookmarks\bookmark.h
File ${LibPlugins}\ml_bookmarks\bookmark.cpp
File ${LibPlugins}\ml_bookmarks\listview.h
File ${LibPlugins}\ml_bookmarks\listview.cpp
File ${LibPlugins}\ml_bookmarks\view.cpp

; resource files
File ${LibPlugins}\ml_bookmarks\resource.h
File ${LibPlugins}\ml_bookmarks\ml_bookmarks.rc
File ${LibPlugins}\ml_bookmarks\version.rc2

SetOutPath $INSTDIR\ml_bookmarks\resources
File ${LibPlugins}\ml_bookmarks\resources\ti_bookmarks_16x16x16.bmp
