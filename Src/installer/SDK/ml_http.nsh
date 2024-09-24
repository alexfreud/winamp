; Sample HTTP Plugin
SetOutPath $INSTDIR\ml_http

; project files
File ${Hinterland}\ml_http\ml_http.sln
File ${Hinterland}\ml_http\ml_http.vcproj

; source
File ${Hinterland}\ml_http\main.cpp
File ${Hinterland}\ml_http\main.h
File ${Hinterland}\ml_http\HTMLControl.cpp
File ${Hinterland}\ml_http\HTMLControl.h
File ${Hinterland}\ml_http\SampleHttp.cpp

; resources
File ${Hinterland}\ml_http\resource.h
File ${Hinterland}\ml_http\ml_http.rc

SetOutPath $INSTDIR\ml_http\resources
File ${Hinterland}\ml_http\resources\ti_now_playing_16x16x16.bmp