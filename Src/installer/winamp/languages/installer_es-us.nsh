; Languge-Culture:	ES-US
; LangId:			3082
; CodePage:			1252
; Revision:			8 UNICODE
; Last udpdated:	19.07.2013
; Author: Manuel Fernando Gutiérrez
; Email: manufco@gmail.com
; Base: Darwin Rodrígo Toledo Cáceres
; Email: niwrad777@niwradsoft.com
; Comments: Spanish Winamp is back!
;History:
;23-10-2007 >> Fixed typo and missing strings
;30-10-2007 >> Added New strings, Milkdrop2
;01-11-2007 >> Fixed some strings that are shown incorrectly... changed couple strings, ....i think so! :P
;03-11-2007 >> Added secFLV and Desc_FLV lines
;15-11-2007 >> Added old os message - IDS_MSG_WINDOWS_TOO_OLD like en-us page code :p
;16-01-2008 >> changed winamp remote bundle text (IDS_BUNDLE1_DESCRIPTION).
;18-01-2008 >> Fixed typos (plurals, etc)
;23-03-2008 >> added toolbar search (IDS_BUNDLE21_XXX) (note: subject to change)
;23-03-2008 >> barabanger: added winamp search (IDS_WINAMP_SEARCH).
;26-03-2008 >> djegg: removed "(enhanced by Google®)" from IDS_BUNDLE21_DESCRIPTION
;31-05-2008 >> Manuel: added translation for SecFLV and IDS_SEC_FLV_DEC_DESC section and description
;15-06-2008 >> Manuel: added translation for IDS_SEC_GEN_DROPBOX & IDS_SEC_GEN_DROPBOX_DESC (subject to change)
;02-07-2008 >> Manuel: added translation for: "djegg: added missing SEC_ML_PLG item (line 151) for Playlist Generator"
;03-07-2008 >> Manuel: translated the new string for emusic
;06-01-2009 >> Manuel: added Winamp3 section for upgrade messages, lines 287-292 & added localized "built on" and "at" strings for branding text (lines 48-50)
;12-11-2009 >> A lot of Actualizations :-)
;06-12-2009 >> added DirectX section
;13-12-2009 >> Updated some strings
;16-12-2010 >> Darwin: Added IDS_UNINSTALL_XXX, Translation for winamp 5.61+
;16-12-2010 >> Darwin: Removed IDS_DXDIST, IDS_DIRECTX_INSTALL_ERR, IDS_SEC_ML_ORB_DESC
;16-12-2010 >> Darwin: Disable IDS_BUNDLE1_DESCRIPTION
;16-12-2010 >> Darwin: Changed IDS_PAGE_WELCOME_TEXT
;23-12-2010 >> Darwin: Changes in IDS_PAGE_WELCOME_TEXT 
;08-03-2011 >> Darwin: Changed "Reproduciendo ahora" by "Reproducción en Curso" (More appropriate).
;14-03-2011 >> Darwin: Changes in IDS_PAGE_WELCOME_TEXT
;23-06-2011 >> Darwin: changed AAC/aacPlus to HE-AAC (secAACE, line 91)
;19-07-2013 >> Manuel: Yes I'm Back! - Updated ${LangFileString} IDS_PAGE_WELCOME_TEXT 
;05-08-2013 >> Manuel: IDS_FIREWALL line 443

!insertmacro LANGFILE_EXT "SpanishInternational"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Idioma de la Instalación"
${LangFileString} LANGUAGE_DLL_INFO "Por favor seleccione su idioma:"
 
${LangFileString} installFull "Completa"
${LangFileString} installStandard "Estándar"
${LangFileString} installLite "Sencilla"
${LangFileString} installMinimal "Mínima"
${LangFileString} installPrevious "Instalación previa"

; BrandingText
${LangFileString} BuiltOn "compilado el"
${LangFileString} at "a las"

${LangFileString} installWinampTop "Se instalará Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}."
${LangFileString} installerContainsFull "Este instalador contiene la instalación completa."
${LangFileString} installerContainsLite "Este instalador contiene la instalación Minima."
${LangFileString} licenseTop "Lea y acepte las condiciones de licencia que aparecen a continuación para proseguir con instalación."
${LangFileString} directoryTop "$(^NameDA) se instalará en la siguiente ubicación. Si desea modificar la ubicación, hágalo ahora."

${LangFileString} uninstallPrompt "Esta acción desinstalará Winamp. ¿Desea continuar?"

${LangFileString} msgCancelInstall "¿Desea cancelar la instalación?"
${LangFileString} msgReboot "Es necesario reiniciar el ordenador para completar la instalación.$\r$\n¿Desea reiniciar ahora? (Si desea reiniciar más tarde, seleccione No)"
${LangFileString} msgCloseWinamp "Debe cerrar Winamp antes de continuar.$\r$\n$\r$\n	Después de cerrar Winamp, seleccione Reintentar.$\r$\n$\r$\n	Si desea intentar la instalación de todas maneras, seleccione Omitir.$\r$\n$\r$\n	Si desea anular la instalación, seleccione Anular."
${LangFileString} msgInstallAborted "Instalación anulada por el usuario"

${LangFileString} secWinamp "Winamp (requerido)"
${LangFileString} compAgent "Agente de Winamp"
${LangFileString} compModernSkin "Compatibilidad con Carátulas Modernas"
${LangFileString} uninstallWinamp "Desinstalar Winamp"

${LangFileString} secWMA "Audio de Windows Media (WMA)"
${LangFileString} secWMV "Video de Windows Media (WMV, ASF)"
${LangFileString} secWMFDist "Descargar e instalar formato de Windows Media"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "Reproducción de OGG Vorbis"
${LangFileString} secOGGEnc "Codificación de OGG Vorbis"
${LangFileString} secAACE "Codificación de HE-AAC"
${LangFileString} secMP3E "Codificación MP3"
${LangFileString} secMP4E "Compatibilidad con MP4"
${LangFileString} secWMAE "Codificación WMA"
${LangFileString} msgWMAError "Se ha producido un problema al instalar los componentes. No se instalará el codificador de WMA. Visite la página http://www.microsoft.com/windows/windowsmedia/9series/encoder/ , descargue el codificador e inténtelo de nuevo."
${LangFileString} secCDDA "Extracción y reproducción de CD" 
${LangFileString} msgCDError "Se ha producido un problema en la instalación de los componentes. Es posible que la grabación de CD no funcione correctamente."
${LangFileString} secCDDB "CDDB para reconocer CDs"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"

${LangFileString} secDSP "Complemento de Estudio de Procesador de Señal"
${LangFileString} secWriteWAV "Grabadora clasica de WAV"
${LangFileString} secLineInput "Compatibilidad con entrada de línea"
${LangFileString} secDirectSound "Compatibilidad de salida de DirectSound"

${LangFileString} secHotKey "Compatibilidad con Teclas Abreviadas Globales"
${LangFileString} secJmp "Extensión del Dialogo Saltar a:"
${LangFileString} secTray "Control de Bandeja de Nullsoft"

${LangFileString} msgRemoveMJUICE "¿Desea eliminar la compatibilidad con MJuice del sistema?$\r$\n$\r$\nSeleccione SÍ a menos que utilice archivos MJF en otros programas aparte de Winamp."
${LangFileString} msgNotAllFiles "No se han eliminado todos los archivos.$\r$\nSi desea eliminar los archivos usted mismo, puede hacerlo."


${LangFileString} secNSV "Video de Nullsoft (NSV)"
${LangFileString} secDSHOW "DirectShow Formats (MPG, M2V)"
${LangFileString} secAVI "Video AVI"
${LangFileString} secFLV "Video de Flash (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "Video MPEG-4 (MP4, M4V)"

${LangFileString} secSWF "Protocolo de Medios Flash (SWF, RTMP)"

${LangFileString} secTiny "Pantalla completa Reducida de Nullsoft"
${LangFileString} secAVS "Estudio de Visualización Avanzada"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Biblioteca Multimedia de Winamp"
${LangFileString} secOM "Medios en línea"
${LangFileString} secWire "Directorio de Podcast"
${LangFileString} secPmp "Reproductores Multimedia portátiles"
${LangFileString} secPmpIpod "Compatibilidad con iPod®"
${LangFileString} secPmpCreative "Compatibilidad con reproductores Creative®"
${LangFileString} secPmpP4S "Compatibilidad con Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "Compatibilidad con dispositivos USB"
${LangFileString} secPmpActiveSync "Compatibilidad con Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid "Compatibilidad con dispositivos Android"
${LangFileString} secPmpWifi "Compatibilidad con Android Wifi"

${LangFileString} sec_ML_LOCAL "Medios locales"
${LangFileString} sec_ML_PLAYLISTS "Listas de Reproducción"
${LangFileString} sec_ML_DISC "Grabado y Extracción de CDs"
${LangFileString} sec_ML_BOOKMARKS "Marcadores"
${LangFileString} sec_ML_HISTORY "Historial"
${LangFileString} sec_ML_NOWPLAYING "Reproducción en curso"
${LangFileString} sec_ML_RG "Herramienta de Análisis de Ganancia"
${LangFileString} sec_ML_TRANSCODE "Herramienta de transcodificación"
${LangFileString} sec_ML_PLG "Generador de Listas"
${LangFileString} sec_ML_IMPEX "Herramienta de importación de iTunes"


;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "Instalador de $(^NameDA)"
${LangFileString} IDS_SELECT_LANGUAGE  "Seleccione el idioma del instalador"

; Groups
${LangFileString} IDS_GRP_MMEDIA		      "Motor Multimedia"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Salida"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Reproductor de audio"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Codificadores de audio"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Reproductor de video"
${LangFileString} IDS_GRP_VISUALIZATION		"Visualización"
${LangFileString} IDS_GRP_UIEXTENSION		"Extensiones de interfaz de usuario"
${LangFileString} IDS_GRP_WALIB				"Biblioteca de Winamp"
${LangFileString} IDS_GRP_WALIB_CORE		"Componentes Principales de la Biblioteca Multimedia"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Compatibilidad con Reproductores Multimedia portátiles"
${LangFileString} IDS_GRP_LANGUAGES 	    "Idiomas"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"Salida WaveOut/MME"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"Codificación FLAC"
${LangFileString} IDS_SEC_MILKDROP2             "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG		"Etiquetador automático"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO "Plugin de Radio Francesa"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Configurando los servicios en línea..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Comprobando si se está ejecutando otra instancia de Winamp..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Abriendo conexión a Internet"
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Comprobando la disponibilidad de Internet..."
${LangFileString} IDS_RUN_NOINET				"No hay conexión a Internet"
${LangFileString} IDS_RUN_EXTRACT				"Extrayendo"
${LangFileString} IDS_RUN_DOWNLOAD				"Descargando"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Descarga completa."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Error en la descarga. Motivo: "
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Descarga cancelada."
${LangFileString} IDS_RUN_INSTALL				"Instalando"
${LangFileString} IDS_RUN_INSTALLFIALED			"Error en la instalación"
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Archivo no encontrado. Descarga de programación."
${LangFileString} IDS_RUN_DONE					"Hecho."

${LangFileString} IDS_DSP_PRESETS 	"Ecualizaciones de SPS"
${LangFileString} IDS_DEFAULT_SKIN	"Carátulas predeterminadas"
${LangFileString} IDS_AVS_PRESETS	"Visualizaciones de AVS"
${LangFileString} IDS_MILK_PRESETS	"Visualizaciones de MilkDrop"
${LangFileString} IDS_MILK2_PRESETS	"Visualizaciones de MilkDrop2"

${LangFileString} IDS_CLEANUP_PLUGINS	"Limpiar plugins..."
${LangFileString} IDS_REMOVE_SKINS		"Quitar carátulas predeterminadas..."


; download
${LangFileString} IDS_DOWNLOADING	"Descargando"
${LangFileString} IDS_CONNECTING	"Conectando..."
${LangFileString} IDS_SECOND		" (1 segundo restante)"
${LangFileString} IDS_MINUTE		" (1 minuto restante)"
${LangFileString} IDS_HOUR		" (1 hora restante)"
${LangFileString} IDS_SECONDS		" (%u segundos restantes)"
${LangFileString} IDS_MINUTES		" (%u minutos restantes)"
${LangFileString} IDS_HOURS		" (%u horas restantes)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%) de %skB @ %u.%01ukB/s"


; AutoplayHandler
${LangFileString} AutoplayHandler	"Reproducir"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Instalación terminada"
${LangFileString} IDS_PAGE_FINISH_TEXT		"Se ha instalado $(^NameDA) en su ordenador .$\r$\n$\r$\n\
						Haga clic en Terminar para cerrar el asistente."
${LangFileString} IDS_PAGE_FINISH_RUN		"Ejecutar $(^NameDA) al cerrar el instalador"
${LangFileString} IDS_PAGE_FINISH_LINK		"Haga clic aquí para visitar Winamp.com"


; welcome page

!ifdef EXPRESS_MODE
${LangFileString} IDS_PAGE_WELCOME_TITLE		"Bienvenido al asistente de instalación de $(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT			"$(^NameDA) te permite escuchar, ver y administrar toda tu Biblioteca Musical.\
												$\r$\n$\r$\n\
												Las características incluyen:$\r$\n$\r$\n  \
												•  Sincronización inalámbrica de medios usando la app de Winamp para Android$\r$\n  \
												•  Agregue los metadatos a sus canciones usando la característica auto etiquetar$\r$\n  \
												•  Escuchar y suscribirse a más de 30,000 podcasts$\r$\n  \
												•  Cree listas de Reproducción de forma automática$\r$\n  \
												•  Escuche más de 50000 emisoras desde SHOUTcast Radio"

${LangFileString} IDS_PAGE_WELCOME_LEGAL	"Al hacer clic en “siguiente”, está aceptando la <a id=$\"winamp_eula$\">Acuerdo de Licencia</a> y <a id=$\"winamp_privacy_policy$\">los términos de privacidad de</a> $(^NameDA)."
!else
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) le permite manejar su Biblioteca Multimedia y escuchar radio por Internet.$\r$\n$\r$\n\
											Características incluidas:$\r$\n$\r$\n  \
												•  Sincronización inalámbrica de medios usando la app de Winamp para Android$\r$\n$\r$\n  \
												•  Cree listas de Reproducción de forma automática$\r$\n$\r$\n  \
												•  Escuchar y suscribirse a más de 30,000 podcasts"

!endif ; defined (EXPRESS_MODE)

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"NOTA: Para disfrutar de las nuevas características y del diseño de la carátula Bento (recomendada),debe seleccionar todos los componentes."



; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE		"Seleccione las opciones de inicio"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Seleccione entre las siguientes opciones de inicio."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Seleccione una de las siguientes opciones para configurar las preferencias de Winamp."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Crear grupo en el menú de inicio."
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Crear un icono de acceso rápido."
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Crear un icono de escritorio."

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"No es posible cerrar el agente de Winamp.$\r$\n\
                                                   Asegúrese de que no hay otro usuario conectado a Windows.\
                                                   $\r$\n$\r$\n	Después de cerrar el agente de Winamp, seleccione Reintentar.\
                                                   $\r$\n$\r$\n	Si desea instalarlo de cualquier forma, seleccione Ignorar.\
                                                   $\r$\n$\r$\n	Si desea cancelar la instalación, seleccione Abortar."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Lo sentimos, esta versión de Windows ya no es soportada.$\r$\n\
                                                 $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} requiere Windows XP o posterior."

; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"?Un plugin de terceros incompatible fue detectado gen_msn7.dll!$\r$\n$\r$\nEste plugin hace que Winamp 5.57 y posteriores sean inestables al cargar.$\r$\nEl plugin será deshabilitado ahora. Haga clic en Aceptar para continuar."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"¡Un plugin de terceros incompatible fue detectado gen_lyrics.dll!$\r$\n$\r$\nEste plugin hace que Winamp 5.59 y posteriores queden inestables al cargar.$\r$\nEl plugin será deshabilitado ahora. Haga clic en Aceptar para continuar."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING             "?Un plugin de terceros gen_lyrics.dll fue detectado!$\r$\n$\r$\nLas versiones antiguas de este plugin son incompatibles con Winamp 5.6 y posteriores. Asegúrese de que tiene la última versión desde http://lyricsplugin.com antes de continuar."
${LangFileString} IDS_LYRICS_IE_PLUGIN_DISABLE	"¡Se detectó Plugin de terceros incompatible 3rd-party gen_lyrics_ie.dll!$\r$\n$\r$\nEste plugin hace que Winamp falle.$\r$\nEl plugin se deshabilitará.Haga clic en Aceptar para continuar."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"Se detectó ${DIRECTXINSTAL_WINVER_LO} o inferior"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "Se detectó ${DIRECTXINSTAL_WINVER_HI} o superior"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Verificando la versión de ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Se requiere al menos ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Imposible detectar la versión de ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"Se detectó la versión ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"${DIRECTXINSTAL_DIRECTXNAME} versión no soportada"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Verificando si $0 está presente"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Descarga requerida"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Verificando conexión a internet"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"La versión de ${DIRECTXINSTAL_DIRECTXNAME} mas reciente disponible en:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Descargando la versión ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FOUND						"Encontrado"
${LangFileString} IDS_DIRECTX_MISSING					"Falta"
${LangFileString} IDS_DIRECTX_SUCCESS					"Terminado"
${LangFileString} IDS_DIRECTX_ABORTED					"Abortado"
${LangFileString} IDS_DIRECTX_FAILED					"Falló"
${LangFileString} IDS_DIRECTX_DONE						"Hecho"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Ejecutando el instalador de ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"${DIRECTXINSTAL_WINAMPNAME} requiere al menos ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} para funcionar correctamente.$\r$\nInstalar ahora?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"${DIRECTXINSTAL_WINAMPNAME} requiere al menos ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} para funcionar correctamente"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"No es posible descargar ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"No es posible instalar ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"Su computador no tiene instalado ${DIRECTXINSTAL_DIRECTXNAME} que es un componente requerido por ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"No es posible descargar el componente ${DIRECTXINSTAL_DIRECTXNAME} que falta"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"No es posible instalar el componente ${DIRECTXINSTAL_DIRECTXNAME} que falta"

;French Radio Section
${LangFileString} IDS_FRENCHRADIO_INSTALLING			"Instalando $(IDS_SEC_GEN_FRENCHRADIO)..."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Nucleo de Winamp (requerido)"
${LangFileString} IDS_SEC_AGENT_DESC			"El agente de Winamp permite un acceso rápido a la bandeja del sistema y mantiene las asociaciones."
${LangFileString} IDS_GRP_MMEDIA_DESC			"Motor Multimedia (sistema de Entrada/Salida)"
${LangFileString} IDS_SEC_CDDB_DESC				"Compatibilidad con CDDB, obtiene automáticamente los nombres de CDs y Pistas, Provistas por Gracenote"
${LangFileString} IDS_SEC_DSP_DESC				"Complemento de DSP, aplica efectos como coros, velocidad, ritmo y control de sonido."
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Compatibilidad para reproducción de audio (Complementos de entrada: decodificadores de audio)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Compatibilidad de reproducción de formatos MP3, MP2, MP1, AAC (requerido)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Compatibilidad de reproducción de formatos WMA (incluido el formato DRM)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Compatibilidad de reproducción de formatos MIDI (MID, RMI, KAR, MUS, CMF y muchos más)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Compatibilidad de reproducción de formatos de módulo (MOD, XM, IT, S3M, ULT y muchos más)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Compatibilidad de reproducción del formato Ogg Vorbis (OGG)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Compatibilidad de reproducción de formatos de Audio MPEG-4 (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Compatibilidad de reproducción de formato FLAC"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Compatibilidad de reproducción de CDs de Audio"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Compatibilidad de reproducción de formatos de video Waveform (WAV, VOC, AU, AIFF y muchos más)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Compatibilidad de reproducción de video (Complementos de entrada: decodificadores de video)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Compatibilidad de reproducción de formatos de video de Windows Media  (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Compatibilidad de reproducción de formatos de video Nullsoft (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Compatibilidad de reproducción de formatos de video 1/2 y otros"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Compatibilidad de reproducción de formatos de video AVI"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Compatibilidad de reproducción de formatos vp6 video de flash (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Compatibilidad de reproducción para Video Matroska (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Compatibilidad de reproducción para Video MPEG-4 (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Soporte para la reproducción del formato de transmisión Flash de Adobe (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Compatibilidad con des/codificación (necesaria para la grabación de CDs y conversión a otros formatos)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Compatibilidad de grabación y transcodificación a formato WMA"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Compatibilidad de grabación y transcodificación a formato WAV"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Compatibilidad de grabación y transcodificación a formato M4A y AAC"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Compatibilidad de grabación y transcodificación a formato FLAC"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Compatibilidad de grabación y transcodificación a formato Ogg Vorbis"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Complementos de salida (controlan el modo en el que se procesa el audio y se envía a la tarjeta de sonido)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Grabadora clásica de WAV/MME (obsoleta, aunque algunos usuarios aún prefieren utilizarla.)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"Salida de DirectSound (requerida) complemento predeterminado salida"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Salida de ondas (opcional), no es obligatoria ni recomendada"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Extensiones de interfaz de usuario"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Complemento de teclas de acceso rápido, controla Winamp con el teclado cuando otras aplicaciones están activas."
${LangFileString} IDS_SEC_JUMPEX_DESC			"Mejoras para el diálogo saltar a"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Complemento Nullsoft Tray Control, añade los iconos de reproducción a la bandeja del sistema"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Compatibilidad con carátulas modernas, necesario para usar carátulas innovadoras, como Winamp Modern y Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Complementos de visualización"
${LangFileString} IDS_SEC_NSFS_DESC				"Complemento de visualización Nullsoft Tiny Fullscreen"
${LangFileString} IDS_SEC_AVS_DESC				"Complemento Advanced Visualization Studio, Magnificas Visualizacines al compaz de la música"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Complemento de visualización Milkdrop"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Complemento de visualización Milkdrop v2 (predeterminado)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Compatibilidad con la línea de entrada mediante el uso de linein:// command"
${LangFileString} IDS_GRP_WALIB_DESC			"Biblioteca Winamp"
${LangFileString} IDS_SEC_ML_DESC				"Biblioteca Multimedia de Winamp (requerida)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Herramienta de transcodificación, cambia el formato del archivo"
${LangFileString} IDS_SEC_ML_RG_DESC			"Herramienta de análisis de ganancia de repetición, nivela el volumen de las pistas."
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Etiquetado automático (Provisto por Gracenote), Obtiene información faltante en los archivos (Artista, Album,etc)"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Directorio Podcast, informa donde suscribirse y descargar podcasts"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Servicios en línea que incluyen SHOUTCast Radio y TV.AOL RADIO, CBS RADIO, Winamp charts y más..."
${LangFileString} IDS_SEC_ML_PLG_DESC			"Generador de listas de reprodución Winamp (Provisto por Gracenote), crea listas de reproducción dinámicas"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Componentes de la Biblioteca Multimedia principal"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Base de datos local, con un potente sistema de consulta y vistas personalizadas"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Administrador de listas de reproducción, crea, modifica y guarda sus listas de reproducción"
${LangFileString} IDS_SEC_ML_DISC_DESC			"Grabadora de CD, interfaz de la biblioteca multimedia para grabar CDs de audio"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Administrador de marcadores, marca sus direcciones, archivos o carpetas favoritas"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Historial, muestra accesos directos a todos los archivos o direcciones que han sido reproducidos recientemente."
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"En reproducción, muestra información acerca de la pista que se reproduce en el momento"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Compatibilidad con Reproductores Multimedia portátiles como,iPod®,Creative® y otros."
${LangFileString} IDS_SEC_ML_PMP_DESC			"Complemento de compatibilidad Media Player (Requerido)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"Compatibilidad con iPod®"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Compatibilidad con dispositivos portátiles Creative® (reproductores Nomad™, Zen™ y MuVo™ )"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Compatibilidad con Microsoft® PlaysForSure® (administra todos los reproductores compatibles P4S)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"Compatibilidad con dispositivos de USB (el uso de unidades de almacenamiento en miniatura y reproductores)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Compatibilidad con Microsoft® ActiveSync® (controla los dispositivos de Windows Mobile®, Smartphone y Pocket PC )"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Compatibilidad con dispositivos Android"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Compatibilidad con Android Wifi"
${LangFileString} IDS_SEC_GEN_DROPBOX_DESC		"Versión alfa del proximo plugin Dropbox. Use ctrl+shift+d para activarlo"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"Plugin para importar/exportar la base de datos de la Biblioteca Multimedia a iTunes"
${LangFileString} IDS_SEC_ML_ADDONS_DESC		"Descubra y agruegue extensiones a su Winamp con agregados (addons)"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO_DESC	"Escuche más de 300 radios francesas, en vivo con $(^NameDA) (Virgin radio, NRJ, RTL, Skyrock, RMC...)"

${LangFileString} IDS_FIREWALL					"Agregando Registros al Firewall"
                                
${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Eliminar $(^NameDA) de su computadora."

${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Ruta de desinstalación:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Media Player"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Desinstalar todos los componentes de $(^NameDA) Media Player incluyendo paquetes de plug-ins  de terceros."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Preferencias del usuario"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Eliminar todas las preferencias de $(^NameDA) y plug-ins."

${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"Ayudar a $(^NameDA) enviando sus comentarios"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"Abrir carpeta de $(^NameDA)"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nNota: No todos los archivos fueron eliminados en esta desinstalación. Para visualizar, abra la carpeta de Winamp"
${LangFileString} IDS_UNINSTALL_SUBHEADER				"$(^NameDA) ha sido desinstalado de su computador.$\r$\n$\r$\nHaga clic en terminar para cerrar."

!ifdef EXPRESS_MODE
${LangFileString} IDS_EXPRESS_MODE_HEADER "$(^NameDA) Modo de instalación"
${LangFileString} IDS_EXPRESS_MODE_SUBHEADER "Seleccione el modo de instalación"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_INSTALL_RADIO "Instala&ción Estándar"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_INSTALL_TEXT "Instalará $(^NameDA) con los componentes recomendados en $\r$\n\
                                                          '$INSTDIR'"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_REINSTALL_TEXT "Instalará $(^NameDA) con los componentes seleccionados previamente en$\r$\n\
                                                          '$INSTDIR'"
${LangFileString} IDS_EXPRESS_MODE_CUSTOM_INSTALL_RADIO "Instala&ción Personalizada"
${LangFileString} IDS_EXPRESS_MODE_CUSTOM_INSTALL_TEXT "Permite personalizar que componentes de $(^NameDA) instalar $\r$\n\
                                                        seleccionandolos manualmente."
!endif ; defined (EXPRESS_MODE) 