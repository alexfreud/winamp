; XML Parser / Media Library example
SetOutPath $INSTDIR\ml_xmlex

; project files
File ${SDKPlugins}\ml_xmlex\ml_xmlex.sln
File ${SDKPlugins}\ml_xmlex\ml_xmlex.vcxproj
File ${SDKPlugins}\ml_xmlex\ml_xmlex.vcxproj.filters

; source
File ${SDKPlugins}\ml_xmlex\main.cpp
File ${SDKPlugins}\ml_xmlex\main.h
File ${SDKPlugins}\ml_xmlex\api.h
File ${SDKPlugins}\ml_xmlex\xmlview.cpp

; resources
File ${SDKPlugins}\ml_xmlex\resource.h
File ${SDKPlugins}\ml_xmlex\ml_xmlex.rc
File ${SDKPlugins}\ml_xmlex\version.rc2

; documents and sample files
File ${SDKPlugins}\ml_xmlex\readme.txt
File ${SDKPlugins}\ml_xmlex\xmltest.xml
File ${SDKPlugins}\ml_xmlex\xmltest2.xml

