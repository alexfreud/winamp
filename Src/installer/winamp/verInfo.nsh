!ifndef _DEBUG

!include "buildConstants.nsh"

VIProductVersion "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MINOR_SECOND_SHORT}.${BUILD_NUM}"
VIAddVersionKey "ProductName" "${WINAMP} Installer"
VIAddVersionKey "Comments" "Visit http://www.winamp.com/ for updates."
VIAddVersionKey "CompanyName" "Winamp SA"
VIAddVersionKey "LegalTrademarks" "Nullsoft and Winamp are trademarks of Winamp SA"
VIAddVersionKey "LegalCopyright" "Copyright © 1997-2023 Winamp SA"
VIAddVersionKey "FileDescription" "${WINAMP} Installer"
VIAddVersionKey "FileVersion" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MINOR_SECOND_SHORT}.${BUILD_NUM}"
VIAddVersionKey "ProductVersion" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MINOR_SECOND_SHORT} Build ${BUILD_NUM}"
VIAddVersionKey "SpecialBuild" "${VERSION_ADDITIONALINFO}"

!ifndef InstallType
!define InstallType "Final"
!endif

!else ; _DEBUG

!define VERSION_MAJOR                "Debug"
!define VERSION_MINOR                ""
!define VERSION_MINOR_SECOND         ""
!define VERSION_MINOR_SECOND_SHORT   ""
!define BUILD_NUM                    ""
!define InstallType                  "Internal"

!endif ; _DEBUG