; Agave (Winamp 5 Wasabi API)

; Config API
SetOutPath $INSTDIR\Agave\Config
File ${PROJECTS}\Agave\Config\api_config.h
File ${PROJECTS}\Agave\Config\ifc_configgroup.h
File ${PROJECTS}\Agave\Config\ifc_configitem.h

; Metadata API
SetOutPath $INSTDIR\Agave\Metadata
File ${PROJECTS}\Agave\Metadata\api_metadata.h
File ${PROJECTS}\Agave\Metadata\svc_metatag.h

; Language API
SetOutPath $INSTDIR\Agave\Language
File ${PROJECTS}\Agave\Language\api_language.h
File ${PROJECTS}\Agave\Language\lang.h

; Component API
SetOutPath $INSTDIR\Agave\Component
File ${PROJECTS}\Agave\Component\ifc_wa5component.h

; Album Art API
SetOutPath $INSTDIR\Agave\AlbumArt
File ${PROJECTS}\Agave\AlbumArt\svc_albumArtProvider.h
File ${PROJECTS}\Agave\AlbumArt\api_albumart.h
File ${PROJECTS}\Wasabi\api\service\svcs\svc_imgwrite.h

; Audio Decoder API
SetOutPath $INSTDIR\Agave\DecodeFile
File ${PROJECTS}\Agave\DecodeFile\api_decodefile.h
File ${PROJECTS}\Agave\DecodeFile\ifc_audiostream.h

; Queue API
SetOutPath $INSTDIR\Agave\Queue
File ${PROJECTS}\Agave\Queue\api_queue.h

; ExplorerFindFile API
SetOutPath $INSTDIR\Agave\ExplorerFindFile
File ${PROJECTS}\Agave\ExplorerFindFile\api_explorerfindfile.h