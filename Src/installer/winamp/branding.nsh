!ifndef NULLSOFT_WINAMP_INSTALLER_BRANDING_HEADER
!define NULLSOFT_WINAMP_INSTALLER_BRANDING_HEADER

!define WINAMP			"Winamp"
!ifdef WINAMP64
	!define WINAMPFOLDER	"${WINAMP}64"
!else
	!define WINAMPFOLDER	"${WINAMP}"
!endif
!define WINAMPEXE			"winamp.exe"
!define WINAMPLINK			"${WINAMP}.lnk"
!define MODERNSKINNAME		"Winamp Modern"

; Header image path
!define HEADER_IMAGE_PATH	".\res\wabanner.bmp"

; WelcomeFinish image path
!ifdef EXPRESS_MODE
	!define WELCOMEFINISH_IMAGE_PATH				".\res\welcome55_short.bmp"
!else
	!define WELCOMEFINISH_IMAGE_PATH				".\res\welcome55.bmp"
!endif

!define UNINSTALLER_WELCOMEFINISH_IMAGE_PATH 	"${WELCOMEFINISH_IMAGE_PATH}"

; License path
!ifndef LICENSE_PATH 
	!define LICENSE_PATH "..\..\resources\license\license.rtf"
!endif

; Privacy policy
!define PRIVACY_POLICY_PATH "..\..\resources\license\privacyPolicy.rtf"

!ifdef FULL
	!define INSTALLER_TYPE_DESCRIPTION 	"$(installerContainsFull)"
!else ifdef LITE
	!define INSTALLER_TYPE_DESCRIPTION 	"$(installerContainsLite)"
!else
	!define INSTALLER_TYPE_DESCRIPTION	""
!endif

!endif ;NULLSOFT_WINAMP_INSTALLER_BRANDING_HEADER