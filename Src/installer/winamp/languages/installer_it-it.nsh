; Language-Country:	IT-IT
; LangId:			1040
; CodePage:			1252
; Revision:			6
; Last udpdated:		01.07.2012
; Author:			Riccardo Vianello
; Email:			etms51@gmail.com

; Notes:
; use ';' or '#' for comments
; strings must be in double quotes.
; only edit the strings in quotes:
# example: ${LangFileString} installFull "Edit This Value Only!"
# Make sure there's no trailing spaces at ends of lines
; To use double quote inside string - '$\'
; To force new line  - '$\r$\n'
; To insert tabulation  - '$\t'

; History
; 04.10 > barabanger:  added 360 after Microsoft Xbox.
; 05.10 > barabanger:  in IDS_SEC_FREEFORM_DESC Winamp Bento changed to  "Bento"
; 05.10 > djegg: fixed typos in header comments, added extra notes
; 06.10 > barabanger: milkdrop2 strings added
; 27.10 > djegg: removed some trailing spaces
; 01.11 > benski: added in_flv
; 02.11 > djegg: added description for in_flv
; 15.11 > barabanger: added old os message - IDS_MSG_WINDOWS_TOO_OLD
; 14.01 > barabanger: changed winamp remote bundle text (IDS_BUNDLE1_DESCRIPTION).
; 20.03 > barabanger: added toolbar search (IDS_BUNDLE21_XXX).
; 21.03 > barabanger: added winamp search (IDS_WINAMP_SEARCH).
; 26.03 > djegg: removed "(enhanced by Google®)" from IDS_BUNDLE21_DESCRIPTION
; 02.05 > koopa: moved text "(default vis plugin) " from IDS_SEC_AVS_DESC to IDS_SEC_MILKDROP2_DESC
; 20.05 > djegg: added secSWF and IDS_SEC_SWF_DEC_DESC (possibly subject to change)
; 13.06 > barabanger: added IDS_SEC_GEN_DROPBOX & IDS_SEC_GEN_DROPBOX_DESC (subject to change)
; 17.06 > djegg: added missing SEC_ML_PLG item for Playlist Generator
; 03.07 > barabanger: changed emusic bundle text
; 24.11 > djegg: added Winamp3 section for upgrade messages
; 01/01 > djegg: added localized "built on" and "at" strings for branding text
; 15/01 > djegg: added ml_impex entry & description
; 01/02 > djegg: added AutoplayHandler
; 09/02 > djegg: added Language Selection dialog section
; 20/02 > djegg: added MB & KB to Bundle page
; 17.06 > barabanger: added orgler section name and description (IDS_SEC_ML_ORGLER)
; aug 27 2009 > benski: added IDS_RUN_OPTIMIZING
; sep 08 2009 > smontgo: changed Welcome screen per US1145 and Powerpoint deck.
; sep 17 2009 > benski: added sections & descriptions for MP4V & MKV
; sep 21 2009 > djegg: changed sec_ml_impex description
; oct 30 2009 > benski: changed in_dshow desc to IDS_SEC_DSHOW_DEC_DESC, removed AVI from desc
; oct 30 2009 > benski: added IDS_SEC_AVI_DEC_DESC for in_avi
; oct 30 2009 > djegg: changed secDSHOW description (removed AVI)
; oct 30 2009 > djegg: added secAVI
; nov 4 2009 > barabanger: added IDS_SEC_ML_ADDONS & IDS_SEC_ML_ADDONS_DESC, lines 231 & 468 (nov 6 2009 > ychen: modified description)
; nov 13 2009 > barabanger: added IDS_SEC_MUSIC_BUNDLE & DESC, lines 232 & 469. Edited "Downloading" string
; nov 22 2009 > barabanger: updated music bundle related text (see prev rec)
; nov 24 2009 > djegg: updated IDS_SEC_ML_ONLINE_DESC
; nov 30 2009 > smontgo: added IDS_DXDIST for download of DirectX9 web installer (for d3dx9 libs for osd)
; dec 01 2009 > smontgo: added IDS_DIRECTX_INSTALL_ERR to report directx download or install error
; dec 04 2009 > barabanger: removed IDS_DXDIST and IDS_DIRECTX_ISNTALL_ERR
; dec 04 2009 > barabanger: added DirectX Section: IDS_DIRECTX_*
; dec 11 2009 > smontgo: edited IDS_DIRECTX_EMBED_CONNECT_FAILED string (Your computer is missing a)
; jan 22 2010 > djegg: added IDS_CLEANUP_PLUGINS & IDS_REMOVE_SKINS to 'installation strings' (lines 257-258)
; mar 15 2010 > barabanger: new uninstaller strings (lines 471-490) // oct 4 2010 > djegg: edited lines 472+474
; may 26 2010 > djegg: added pmp_android (lines 186 & 462)
; sep 29 2010 > benski: added pmp_wifi (lines 187 & 463)
; nov 08 2010 > barabanger: updated IDS_PAGE_WELCOME_TEXT // nov 12 2010 > added extra line inbetween welcome text and bullet points // nov 19 2010 > updated welcome text
; nov 12 2010 > barabanger: Commented-out Winamp Remote from bundle page (line 324)
; dec 04 2010 > djegg: added IDS_LYRICS_PLUGIN_DISABLE for disabling incompatible gen_lyrics plugin (line 352)
; dec 04 2010 > djegg: added IDS_LYRICS_PLUGIN_WARNING for warning about incompatible gen_lyrics plugin (line 353)
; jun 23 2011 > djegg: changed AAC/aacPlus to HE-AAC (secAACE, line 123)

!insertmacro LANGFILE_EXT "Italian"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Lingua installatore"
${LangFileString} LANGUAGE_DLL_INFO "Seleziona una lingua."
 
${LangFileString} installFull "Completo"
${LangFileString} installStandard "Standard"
${LangFileString} installLite "Lite"
${LangFileString} installMinimal "Minima"
${LangFileString} installPrevious "Installazione precedente"

; BrandingText
${LangFileString} BuiltOn "costruito il"
${LangFileString} at "alle"

${LangFileString} installWinampTop "Questo programma installerà Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}."
${LangFileString} installerContainsFull "Questo programma d'installazione contiene l'installazione completa."
${LangFileString} installerContainsLite "Questo programma d'installazione contiene l'installazione lite."
${LangFileString} licenseTop "Si prega di leggere e accettare i termini di licenza prima di installare."
${LangFileString} directoryTop "Questo programma d'installazione ha determinato la posizione ottimale per $(^NameDA). Ora è possibile modificare la cartella, se necessario."

${LangFileString} uninstallPrompt "Questo disinstallerà Winamp. Vuoi continuare?"

${LangFileString} msgCancelInstall "Annullare l'installazione?"
${LangFileString} msgReboot "Un riavvio è richiesto per completare l'installazione.$\r$\nRiavviare ora? (Se si desidera riavviare il sistema più tardi, selezionare No)"
${LangFileString} msgCloseWinamp "Prima di continuare è necessario chiudere Winamp.$\r$\n$\r$\n	Una volta chiuso Winamp, selezionare Riprova.$\r$\n$\r$\n	Se si desidera tentare di installare in ogni caso, selezionare Ignora.$\r$\n$\r$\n	Se si desidera diinterrompere l'installazione, seleziona Interrompi."
${LangFileString} msgInstallAborted "Installazione interrotta dall'utente"

${LangFileString} secWinamp "Winamp (richiesto)"
${LangFileString} compAgent "Agent di Winamp"
${LangFileString} compModernSkin "Supporto per l'interfaccia Modern"
${LangFileString} uninstallWinamp "Disinstalla Winamp"

${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Scarica e installa il formato Windows"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "Riproduzione OGG Vorbis"
${LangFileString} secOGGEnc "Codifica OGG Vorbis"
${LangFileString} secAACE "Codifica HE-AAC"
${LangFileString} secMP3E "Codifica MP3"
${LangFileString} secMP4E "Supporto MP4"
${LangFileString} secWMAE "Codifica WMA"
${LangFileString} msgWMAError "Si era verificato un problema durante l'installazione dei componenti. Il codificatore WMA non sarà installato. Si prega di visitare il sito http://www.microsoft.com/windows/windowsmedia/9series/encoder/ , scarica il codificatore e riprova di nuovo."
${LangFileString} secCDDA "Riproduzione ed estrazione di CD"
${LangFileString} msgCDError "Si è verificato un problema durante l'installazione dei componenti. L'estrazione/masterizzazione del CD potrebbe non funzionare correttamente. "
${LangFileString} secCDDB "CDDB per il riconoscimento dei CD"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"

${LangFileString} secDSP "Plugin Signal Processor Studio"
${LangFileString} secWriteWAV "WAV writer vecchia scuola"
${LangFileString} secLineInput "Supporto Input di linea"
${LangFileString} secDirectSound "Supporto per output DirectSound"

${LangFileString} secHotKey "Supporto tasti rapidi globali"
${LangFileString} secJmp "Supporto esteso di salto al file"
${LangFileString} secTray "Sistema di controllo di Nullsoft"

${LangFileString} msgRemoveMJUICE "Si vuole rimuovere il supporto MJuice dal sistema?$\r$\n$\r$\nSeleziona SÌ a meno che non si utilizzano file MJF in programmi diversi da Winamp."
${LangFileString} msgNotAllFiles "Non tutti i file sono stati rimossi.$\r$\nSe si desidera rimuovere i file, è possibile farlo."


${LangFileString} secNSV "Nullsoft Video (NSV)"
${LangFileString} secDSHOW "Formati DirectShow (MPG, M2V)"
${LangFileString} secAVI "AVI Video"
${LangFileString} secFLV "Flash Video (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "MPEG-4 Video (MP4, M4V)"

${LangFileString} secSWF "Protocollo Flash Media (SWF, RTMP)"

${LangFileString} secTiny "Nullsoft Tiny Fullscreen"
${LangFileString} secAVS "Advanced Visualization Studio"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Libreria multimediale di Winamp"
${LangFileString} secOM "Supporto online multimediale"
${LangFileString} secWire "Elenco Podcast"
${LangFileString} secPmp "Lettori multimediali Portable"
${LangFileString} secPmpIpod "Supporto iPod®"
${LangFileString} secPmpCreative "Supporto per i lettori Creative®"
${LangFileString} secPmpP4S "Supporto per Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "Supporto Dispositivi USB"
${LangFileString} secPmpActiveSync "Supporto per Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid "Supporto per dispositivi Android"
${LangFileString} secPmpWifi "Supporto Wifi Android"

${LangFileString} sec_ML_LOCAL "Multimedia locale"
${LangFileString} sec_ML_PLAYLISTS "Liste dei brani"
${LangFileString} sec_ML_DISC "Estrazione & masterizzazione di CD"
${LangFileString} sec_ML_BOOKMARKS "Segnalibri"
${LangFileString} sec_ML_HISTORY "Cronologia"
${LangFileString} sec_ML_NOWPLAYING "Ora in riproduzione"
${LangFileString} sec_ML_RG "Ripeti esecuzione strumento di analisi guadagno"
${LangFileString} sec_ML_TRANSCODE "Strumento di transcodifica"
${LangFileString} sec_ML_PLG "Generatore lista dei brani"
${LangFileString} sec_ML_IMPEX "Strumento di importazione/esportazione Database"


;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "Programma di installazione di $(^NameDA)"
${LangFileString} IDS_SELECT_LANGUAGE  "Seleziona la lingua del programma d'installazione"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Motore multimediale"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Output"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Riproduzione audio"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Codificatori audio"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Riproduzione video"
${LangFileString} IDS_GRP_VISUALIZATION		"Visualizzazione"
${LangFileString} IDS_GRP_UIEXTENSION		"Interfaccia utente delle estensioni"
${LangFileString} IDS_GRP_WALIB				"Libreria di Winamp"
${LangFileString} IDS_GRP_WALIB_CORE		"Componenti della libreria multimediale di base"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Supporto per riproduttore multimediale portatili"
${LangFileString} IDS_GRP_LANGUAGES 	    "Lingue"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"Output WaveOut/MME"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"Codifica FLAC"
${LangFileString} IDS_SEC_MILKDROP2 	"Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG	"Auto-Tagger"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Configurazione dei servizi online in corso..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Verifica per determinare se è in esecuzione un'altra istanza di Winamp..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Apertura connessione internet in corso..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Vefifica se è disponibile internet in corso..."
${LangFileString} IDS_RUN_NOINET				"Nessuna connessione internet"
${LangFileString} IDS_RUN_EXTRACT				"Estrazione in corso..."
${LangFileString} IDS_RUN_DOWNLOAD				"Scaricamento in corso"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Download compleato."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Download non riuscito. Motivo:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Download annullato."
${LangFileString} IDS_RUN_INSTALL				"Installazione in corso"
${LangFileString} IDS_RUN_INSTALLFIALED			"Installazione non riuscita."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"File non trovato. Programmazione dei download."
${LangFileString} IDS_RUN_DONE					"Completato."

${LangFileString} IDS_DSP_PRESETS 	"I preset di SPS"
${LangFileString} IDS_DEFAULT_SKIN	"interfaccie predefinite"
${LangFileString} IDS_AVS_PRESETS	"I preset di AVS"
${LangFileString} IDS_MILK_PRESETS	"I preset di MilkDrop"
${LangFileString} IDS_MILK2_PRESETS	"I preset di MilkDrop2"

${LangFileString} IDS_CLEANUP_PLUGINS	"Pulizia plugin in corso..."
${LangFileString} IDS_REMOVE_SKINS		"Rimuovere le interfaccie predefinite in corso..."


; download
${LangFileString} IDS_DOWNLOADING	"Scaricamento in corso"
${LangFileString} IDS_CONNECTING	"Connessione in corso..."
${LangFileString} IDS_SECOND		" (1 secondo rimanente)"
${LangFileString} IDS_MINUTE		" (1 minuto rimanente)"
${LangFileString} IDS_HOUR			" (1 ora rimanente)"
${LangFileString} IDS_SECONDS		" (%u secondi rimanenti)"
${LangFileString} IDS_MINUTES		" (%u minuti rimanenti)"
${LangFileString} IDS_HOURS			" (%u ore rimanenti)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%) di %skB @ %u.%01ukB/s"


; AutoplayHandler
${LangFileString} AutoplayHandler	"Riproduci"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Installazione Completata"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) è stato installato sul computer.$\r$\n$\r$\n\
													Fare clic su Fine per chiudere dalla procedura guidata."
${LangFileString} IDS_PAGE_FINISH_RUN		"Avvia $(^NameDA) dopo che il programma di installazione termina"
${LangFileString} IDS_PAGE_FINISH_LINK		"Clicca qui per visitare il sito Winamp.com"


; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"Benvenuto al programma di installazione di $(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) permette di ascoltare, guardare e gestire la musica, i video, i podcast e internet radio.$\r$\n$\r$\n$\r$\nCaratteristiche inclusi:$\r$\n$\r$\n  \
													•  Sincronizza in modalità wireless media di $(^NameDA) per Android $\r$\n$\r$\n  \
													•  Controlla la riproduzione nel browser con Winamp Toolbar$\r$\n$\r$\n  \
													•  Pulire i metadati multimediali con le caratteristiche Auto-Tag$\r$\n$\r$\n  \
													•  Costruire le riproduzioni usando il generatore automatico della lista dei brani$\r$\n$\r$\n  \
													•  Ascolta e sottoscrivi più di 30,000 podcast"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"NOTA: Per godere delle nuove funzionalità e \
															l'interfaccia Bento (consigliata), tutti i \
															componenti devono essere selezionati."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Scegli le opzioni d'avvio"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Seleziona una delle seguenti opzioni di avvio."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Scegli tra le seguenti opzioni per configurare le opzioni d'avvio di Winamp"
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Crea una voce nel menu Start"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Crea icona Avvio veloce"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Crea icona sul desktop"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Impossibile terminare Winamp Agent.$\r$\n\
                                                   Essere certi che un altro utente non sia loggatto su Windows.\
                                                   $\r$\n$\r$\n	Dopo aver chiuso WinampAgent, seleziona Riprova.\
                                                   $\r$\n$\r$\n	Se si desidera di tentare di installare comunque, seleziona Ignora.\
                                                   $\r$\n$\r$\n	Se si desidera di interrompere l'installazione, seleziona Interrompi."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Questa versione di Windows non è più supportata.$\r$\n\
                                                 $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} richiede un minimo di Windows 2000 o più recente."

; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"Il plugin Incompatibile di terze parti gen_msn7.dll è rilevato!$\r$\n$\r$\nQuesto plugin causa su Winamp 5.57 e quelli successivi di bloccarsi sul caricamento.$\r$\nIl plugin sarà disattivato. Fare clic su OK per procedere."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off) 
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"Il plugin incompatibile di terze parti gen_lyrics.dll è rilevato!$\r$\n$\r$\nQuesto plugin causa su Winamp 5.59 e quelli successivi di bloccarsi sul caricamento.$\r$\nIl plugin sarà disattivato. Fare clic su OK per procedere."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "Il plugin di terze parti gen_lyrics è stato rilevato!$\r$\n$\r$\nLe vecchie versioni di questo plugin sono incompatibili con Winamp 5.6 e le versioni successive. Essere certo di ottenere l'ultima versione da http://lyricsplugin.com prima di procedere."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"Rilevato ${DIRECTXINSTAL_WINVER_LO} o versione minore"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "Rilevato ${DIRECTXINSTAL_WINVER_HI} o versione superiore"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Controllo versione ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Richiesta versione minima ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Impossibile rilevare la versione ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"Rilevata versione ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Versione non supportata di ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Verifica se $0 è presente"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Download richiesto"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Verifica connessione internet"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"L'ultima versione di ${DIRECTXINSTAL_DIRECTXNAME} disponibile al:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Scaricamento programma d'installazione ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FOUND						"Trovato"
${LangFileString} IDS_DIRECTX_MISSING					"Mancante"
${LangFileString} IDS_DIRECTX_SUCCESS					"Riuscito"
${LangFileString} IDS_DIRECTX_ABORTED					"Interrotto"
${LangFileString} IDS_DIRECTX_FAILED					"Non riuscito"
${LangFileString} IDS_DIRECTX_DONE						"Fatto"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Esegui il programma d'installazione di ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"${DIRECTXINSTAL_WINAMPNAME} richiede almeno ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} per farlo funzionare correttamente.$\r$\nInstallare proprio adesso?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"${DIRECTXINSTAL_WINAMPNAME} richiede almeno ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} per farlo funzionare correttamente"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Impossibile scaricare ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Impossibile installare ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"Il computer è mancante di un componente ${DIRECTXINSTAL_DIRECTXNAME} richiesto da ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Impossibile scaricare il componente mancante ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Impossibile installare il componente mancante ${DIRECTXINSTAL_DIRECTXNAME}"

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Winamp core (richiesto)"
${LangFileString} IDS_SEC_AGENT_DESC			"Agent di Winamp, per un accesso rapido all'area di notifica e la gestione delle associazioni dei tipi di file"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Engine multimediale (Sistema Input/Output)"
${LangFileString} IDS_SEC_CDDB_DESC				"Supporto CDDB, per recuperare automaticamente i titoli del CD dal database in linea di Gracenote"
${LangFileString} IDS_SEC_DSP_DESC				"Plugin DSP, per applicare effetti aggiuntivi come coro, flanger, tempo e controllo del tono"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Supporto per la riproduzione audio (Plugin di input: decodificatori audio)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Supporto per la riproduzione dei formati MP3, MP2, MP1, AAC (richiesto)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Supporto per la riproduzione del formato WMA (incluso il supporto per DRM)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Supporto per la riproduzione dei formati MIDI (MID, RMI, KAR, MUS, CMF e altri)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Supporto per la riproduzione dei formati Modulo (MOD, XM, IT, S3M, ULT e altri)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Supporto per la riproduzione del formato Ogg Vorbis (OGG)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Supporto per la riproduzione dei formati Audio MPEG-4 (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Supporto per la riproduzione del formato FLAC"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Supporto per la riproduzione per CD audio"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Supporto per la riproduzione dei formati Waveform (WAV, VOC, AU, AIFF e altri)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Supporto per la riproduzione video (Plugin di Input: decodificatori video)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Supporto per la riproduzione dei formati video Windows Media (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Supporto per la riproduzione del formato video Nullsoft (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Supporto per la riproduzione dei formati video MPEG-1/2 e altri formati video"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Supporto per la riproduzione del formato AVI Video"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Supporto per la riproduzione del formato Flash Video (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Supporto per la riproduzione del formato Matroska Video (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Supporto per la riproduzione del formato MPEG-4 Video (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Supporto per la riproduzione del formato Adobe Flash in streaming (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Supporto per codifica e trascodifica (richiesto per estrazione da CD e conversione da un formato di file a un altro)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Supporto per l'estrazione e la trascodifica nel formato WMA"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Supporto per l'estrazione e la trascodifica nel formato WAV"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Supporto per l'estrazione e la trascodifica nei formato M4A e AAC"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Supporto per l'estrazione e la trascodifica nel formato FLAC"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Supporto per l'estrazione e la trascodifica nel formato Ogg Vorbis"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Plugin di output (controllano l'elaborazione dell'audio e l'invio alla scheda audio)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Writer WAV/MME vecchio stile (non consigliato, ma alcuni utenti lo preferiscono al posto dei plugin Encoder)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"Output DirectSound (plugin di output richiesto/predefinito)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Output WaveOut vecchio stile (facoltativo e non più consigliato o richiesto)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Estensioni dell'interfaccia utente"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Plugin tasti rapidi globali, per controllare Winamp con la tastiera quando sono attive altre applicazioni"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Passaggio esteso al supporto file, per l'accodamento dei brani nella playlist e altro ancora"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Plugin Nullsoft Tray Control, per aggiungere icone di controllo della riproduzione nell'area di notifica"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Supporto per interfaccia Modern, richiesto per l'utilizzo di interfacce a mano libera come Winamp Modern e Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"I plugin di visualizzazione"
${LangFileString} IDS_SEC_NSFS_DESC				"Plugin di visualizzazione Nullsoft Tiny Fullscreen"
${LangFileString} IDS_SEC_AVS_DESC				"Plugin Advanced Visualization Studio"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Plugin di visualizzazione Milkdrop"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Plugin di visualizzazione Milkdrop2 (predefiniti rispetto ai plugin)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Supporto Input linea utilizzando ingresso linea:// comando (applica il visualizzatore a input mic/linea)"
${LangFileString} IDS_GRP_WALIB_DESC			"Libreria di Winamp"
${LangFileString} IDS_SEC_ML_DESC				"Libreria multimediale di Winamp (richiesta)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Strumento di trascodifica, usato per la conversione da un formato di file a un altro"
${LangFileString} IDS_SEC_ML_RG_DESC			"Ripeti esecuzione strumento di analisi guadagno, utilizzato per il livellamento del volume"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Winamp Auto-Tagger (sviluppato da Gracenote), per l'inserimento di metadati mancanti"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Directory podcast, per la sottoscrizione e il download di podcast"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Servizi online, includendo SHOUTcast Radio && TV, AOL Radio feat. CBS Radio, Winamp Charts, e altro"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Generatore di playlist Winamp (sviluppato da Gracenote), per la creazione di playlist acusticamente dinamiche)"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Componenti della libreria multimediale di base"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Database locale dei file multimediali, con un potente sistema di interrogazione e visualizzazioni smart personalizzate"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Gestore Playlist, per la creazione, la modifica e l'archiviazione di tutte le playlist"
${LangFileString} IDS_SEC_ML_DISC_DESC			"Estrai e masterizza CD, l'interfaccia del catalogo multimediale per l'estrazione e la masterizzazione di CD audio"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Gestore segnalibri, per la gestione dei segnalibri di flussi, file o cartelle preferiti"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Cronologia, per un accesso immediato a tutti i file e flussi riprodotti recentemente in locale o in remoto"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Riproduci ora, per la visualizzazione delle informazioni sulla traccia al momento in riproduzione"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Supporto per riproduttore multimediale portatile"
${LangFileString} IDS_SEC_ML_PMP_DESC			"Plugin del supporto per riproduttore multimediale portatile (richiesto)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"Supporto iPod®"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Supporto per sistemi portatili Creative® (per la gestione dei lettori Nomad™, Zen™ and MuVo™)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Supporto per Microsoft® PlaysForSure® (per la gestione di lettori compatibili con P4S)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"Supporto per periferiche USB (per la gestione di thumbdrive e lettori USB generici)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Supporto per Microsoft® ActiveSync® (per la gestione di periferiche Windows Mobile®, Smartphone e Pocket PC)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Supporto per dispositivi Android"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Supporto Wifi di Android"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"Il plugin del database importazione/esportazione combatibile con iTunes e con la libreria multimediale"

${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Rimuovere $(^NameDA) dal suo computer."

${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"disinstallare percorso:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Lettore multimediale"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Disinstalla tutti i compomenti del lettore multimediale $(^NameDA) includendo i pacchetti di plugin di terze parti."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Preferenza dell'utente"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Rimuovere tutte le preferenze e i plugin di $(^NameDA)."

${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"Aiuto $(^NameDA) presentando un feedback"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"Apri $(^NameDA) in una cartella"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nNota:  Non tutti i file sono stati rimossi da questa disinstallazione. Per visualizzare, aprire la cartella Winamp."
