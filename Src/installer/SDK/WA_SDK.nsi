; WA_SDK.nsi

; This script will collect the files in Winamp SDK and create an installer for them

;------------------------

!ifndef VERSION
  !define VERSION "5.9"
!endif

!define PRODUCT_NAME "Winamp ${VERSION} SDK"

; This is where all projects live.  Ensure this is the correct relative path.  
!ifndef PROJECTS
  !define PROJECTS "..\.."
!endif

; Hinterland repo
!ifndef Hinterland
  !define Hinterland "..\..\..\..\Hinterland"
!endif

; Path to SDK Plugins
!ifndef SDKPlugins
  !define SDKPlugins "..\..\Plugins\SDK"
!endif

; Path to Gen Plugins
!ifndef GenPlugins
  !define GenPlugins "..\..\Plugins\General"
!endif

; Path to Input Plugins
!ifndef InPlugins
  !define InPlugins "..\..\Plugins\Input"
!endif

; Path to Enc Plugins
!ifndef EncPlugins
  !define EncPlugins "..\..\Plugins\Encoder"
!endif

; Path to Library Plugins
!ifndef LibPlugins
  !define LibPlugins "..\..\Plugins\Library"
!endif

; Path to Output Plugins
!ifndef OutPlugins
  !define OutPlugins "..\..\Plugins\Output"
!endif

; Path to DSP Plugins
!ifndef DSPPlugins
  !define DSPPlugins "..\..\Plugins\DSP"
!endif

; Path to Portable Plugins
!ifndef PortablePlugins
  !define PortablePlugins "..\..\Plugins\Portable"
!endif

; Path to Vis Plugins
!ifndef VisPlugins
  !define VisPlugins "..\..\Plugins\Visualization"
!endif

!define old_stuff_for_reference

!define WINAMP "Winamp"
!define UNINSTALLER "WA${VERSION}_SDK_Uninstaller.exe"

;Set Compression
SetCompress force
SetCompressor /solid lzma

!include "sections.nsh"
!include "LogicLib.nsh"
!include "WordFunc.nsh"

XPStyle on

;Request Administrator Privileges
RequestExecutionLevel user

;Version information for Windows Explorer
VIProductVersion "5.9.0.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "Comments" "Winamp SDK Installer"
VIAddVersionKey "LegalCopyright" "Copyright © 1997-2022 Winamp SA"
VIAddVersionKey "CompanyName" "Winamp SA"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} Installer"
VIAddVersionKey "FileVersion" "5.9.0.0"
VIAddVersionKey "ProductVersion" "5.9.0.0"

;------------------------

Name "Winamp ${VERSION} SDK"
OutFile "WA${VERSION}_SDK.exe"
InstallDir "$PROGRAMFILES\Winamp SDK"

Page Directory
Page instfiles

Section ""

; APIs
!include "winamp_api.nsh"
!include "ml_api.nsh"
!include "wasabi.nsh"
!include "bfc.nsh"
!include "xml.nsh"
!include "playlist.nsh"
!include "nu.nsh"
!include "Agave.nsh"
!include "nsv.nsh"
!include "burner.nsh"

; examples
!include "gen_tray.nsh"
!include "enc_flac.nsh"
!include "in_flac.nsh"
!include "ml_bookmarks.nsh"
!include "xspf.nsh"

; Previously moved to Hinterland. Now updated / migrated to VS2019...
!include "ml_xmlex.nsh"
!include "plLoadEx.nsh"
!include "dsp_test.nsh"
!include "in_tone.nsh"
!include "coverdirectory.nsh"
!include "irctell.nsh"
!include "ml_iso.nsh"
!include "out_null.nsh"
!include "gen_classicart.nsh"

; These old plugins were moved to Hinterland and do not compile under VS2019, so let's remove them from the SDK
;!include "in_chain.nsh"
;!include "in_raw.nsh"
;!include "ml_http.nsh"
;!include "mlExplorer.nsh"

; skinning
!include "maki.nsh"

; open source
!include "ReplayGainAnalysis.nsh"
!include "nde.nsh"

; TODO
; example using api_tagz
; example using hotkeys
; example using api_decodefile
; vis_avs
; jnetlib (but which one? dll or w5s?)
; file reader API
; example using api_random (maybe by adding noise generator to dsp_test)

!ifdef old_stuff_for_reference

SetOutPath $INSTDIR\gen_ml
File ${GenPlugins}\gen_ml\gaystring.h ; this needs replacing
File ${GenPlugins}\gen_ml\gaystring.cpp ; this needs replacing
File ${GenPlugins}\gen_ml\itemlist.cpp
File ${GenPlugins}\gen_ml\itemlist.h
File ${GenPlugins}\gen_ml\listview.cpp
File ${GenPlugins}\gen_ml\listview.h
File ${GenPlugins}\gen_ml\ml_ipc.h
File ${GenPlugins}\gen_ml\ml_lib.cpp

SetOutPath $INSTDIR\vis
File ${PROJECTS}\resources\SDK\wa5vis.txt
SetOutPath $INSTDIR\vis\vis_avs\apesdk
File /x CVS ${VisPlugins}\vis_avs\apesdk\*.*
SetOutPath $INSTDIR\vis\vis_avs\ns-eel2
File /x CVS ${PROJECTS}\ns-eel2\*.*
SetOutPath $INSTDIR\vis\vis_test
File ${Hinterland}\vis_milkdrop\svis.mak

SetOutPath $INSTDIR\Winamp
File ${GenPlugins}\gen_hotkeys\wa_hotkeys.h
File ${PROJECTS}\Winamp\api_random.h
File ${PROJECTS}\Winamp\api_decodefile.h
File ${PROJECTS}\Winamp\api_audiostream.h

SetOutPath $INSTDIR\tagz
File ${PROJECTS}\tagz\api_tagz.h
File ${PROJECTS}\tagz\ifc_tagprovider.h
File ${PROJECTS}\tagz\ifc_tagparams.h

File /oname=$INSTDIR\readme.txt ${PROJECTS}\Resources\SDK\sdkreadme.txt
!endif

WriteUninstaller "$INSTDIR\${UNINSTALLER}"

SectionEnd

Section "Uninstall"
Delete $INSTDIR\*.*
RMDir $INSTDIR
SectionEnd
