; Wasabi

; Service Manager API
SetOutPath $INSTDIR\Wasabi\api\service
File ${PROJECTS}\Wasabi\api\service\api_service.h
File ${PROJECTS}\Wasabi\api\service\waServiceFactory.h
SetOutPath $INSTDIR\Wasabi\api\syscb\callbacks
File ${PROJECTS}\Wasabi\api\syscb\callbacks\svccb.h

; services
SetOutPath $INSTDIR\Wasabi\api\service
File ${PROJECTS}\Wasabi\api\service\services.h
SetOutPath $INSTDIR\Wasabi\api\service\svcs
File ${PROJECTS}\Wasabi\api\service\svcs\svc_imgload.h

; Application API
SetOutPath $INSTDIR\Wasabi\api\application
File ${PROJECTS}\Wasabi\api\application\api_application.h
File ${PROJECTS}\Wasabi\api\application\ifc_messageprocessor.h

; Memory Manager
SetOutPath $INSTDIR\Wasabi\api\memmgr
File ${PROJECTS}\Wasabi\api\memmgr\api_memmgr.h

; System Callbacks
SetOutPath $INSTDIR\Wasabi\api\syscb
File ${PROJECTS}\Wasabi\api\syscb\api_syscb.h
SetOutPath $INSTDIR\Wasabi\api\syscb\callbacks
File ${PROJECTS}\Wasabi\api\syscb\callbacks\syscb.h

; Replicant/nswasabi
SetOutPath $INSTDIR\Replicant\nswasabi
File ${PROJECTS}\Replicant\nswasabi\ReferenceCounted.h

; Playback Core (Modern skins only!)
SetOutPath $INSTDIR\Wasabi\api\core
File ${PROJECTS}\Wasabi\api\core\api_core.h
SetOutPath $INSTDIR\Wasabi\api\syscb\callbacks
File ${PROJECTS}\Wasabi\api\syscb\callbacks\corecb.h

; Maki (Script) API
SetOutPath $INSTDIR\Wasabi\api\script
File ${PROJECTS}\Wasabi\api\script\api_maki.h
File ${PROJECTS}\Wasabi\api\script\scriptvar.h
File ${PROJECTS}\Wasabi\api\script\vcputypes.h
File ${PROJECTS}\Wasabi\api\script\scriptguid.h
File ${PROJECTS}\Wasabi\api\script\scriptobj.h
File ${PROJECTS}\Wasabi\api\script\objects\rootobjcb.h
SetOutPath $INSTDIR\Wasabi\api\service\svcs
File ${PROJECTS}\Wasabi\api\service\svcs\svc_scriptobj.h