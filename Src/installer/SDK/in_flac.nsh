; Nullsoft FLAC Decoder plugin
SetOutPath $INSTDIR\in_flac

; project files
File ${InPlugins}\in_flac\in_flac.sln
File ${InPlugins}\in_flac\in_flac.vcxproj
File ${InPlugins}\in_flac\in_flac.vcxproj.filters

; source
File ${InPlugins}\in_flac\main.h
File ${InPlugins}\in_flac\main.cpp
File ${InPlugins}\in_flac\api.h
File ${InPlugins}\in_flac\AlbumArt.h
File ${InPlugins}\in_flac\AlbumArt.cpp
File ${InPlugins}\in_flac\DecodeThread.cpp
File ${InPlugins}\in_flac\ExtendedFileInfo.cpp
File ${InPlugins}\in_flac\ExtendedRead.cpp
File ${InPlugins}\in_flac\FileInfo.cpp
File ${InPlugins}\in_flac\FLACFileCallbacks.h
File ${InPlugins}\in_flac\FLACFileCallbacks.cpp
File ${InPlugins}\in_flac\Metadata.h
File ${InPlugins}\in_flac\Metadata.cpp
File ${InPlugins}\in_flac\mkv_flac_decoder.h
File ${InPlugins}\in_flac\mkv_flac_decoder.cpp
File ${InPlugins}\in_flac\Preferences.cpp
File ${InPlugins}\in_flac\QuickBuf.h
File ${InPlugins}\in_flac\RawReader.h
File ${InPlugins}\in_flac\RawReader.cpp
File ${InPlugins}\in_flac\Stopper.h
File ${InPlugins}\in_flac\Stopper.cpp
File ${InPlugins}\in_flac\StreamFileWin32.h
File ${InPlugins}\in_flac\StreamFileWin32.cpp

; resources
File ${InPlugins}\in_flac\in_flac.rc
File ${InPlugins}\in_flac\resource.h
File ${InPlugins}\in_flac\version.rc2
