SetOutPath $INSTDIR\xspf

; project files
File ${PROJECTS}\xspf\xspf.sln
File ${PROJECTS}\xspf\xspf.vcxproj
File ${PROJECTS}\xspf\xspf.vcxproj.filters

; source files
File ${PROJECTS}\xspf\main.cpp
File ${PROJECTS}\xspf\api.h
File ${PROJECTS}\xspf\XSPFHandler.cpp
File ${PROJECTS}\xspf\XSPFHandler.h
File ${PROJECTS}\xspf\XSPFHandlerFactory.cpp
File ${PROJECTS}\xspf\XSPFHandlerFactory.h
File ${PROJECTS}\xspf\XSPFLoader.cpp
File ${PROJECTS}\xspf\XSPFLoader.h

; resource files
File ${PROJECTS}\xspf\xspf.rc
File ${PROJECTS}\xspf\resource.h
File ${PROJECTS}\xspf\version.rc2
