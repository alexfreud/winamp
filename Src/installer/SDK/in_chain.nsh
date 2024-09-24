; Chained input plugin example
SetOutPath $INSTDIR\in_chain

; project files
File ${Hinterland}\in_chain\in_chain.sln
File ${Hinterland}\in_chain\in_chain.vcproj

; source
File ${Hinterland}\in_chain\main.cpp
