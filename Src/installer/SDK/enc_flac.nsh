; FLAC encoder plugin
SetOutPath $INSTDIR\enc_flac

; project files
File ${EncPlugins}\enc_flac\enc_flac2.sln
File ${EncPlugins}\enc_flac\enc_flac2.vcxproj
File ${EncPlugins}\enc_flac\enc_flac2.vcxproj.filters

; source
File ${EncPlugins}\enc_flac\AudioCoderFlac.cpp
File ${EncPlugins}\enc_flac\AudioCoderFlac.h
File ${EncPlugins}\enc_flac\StreamFileWin32.cpp
File ${EncPlugins}\enc_flac\StreamFileWin32.h
File ${EncPlugins}\enc_flac\api.h
File ${EncPlugins}\enc_flac\main.cpp

; resources
File ${EncPlugins}\enc_flac\enc_flac2.rc
File ${EncPlugins}\enc_flac\resource.h
File ${EncPlugins}\enc_flac\version.rc2
