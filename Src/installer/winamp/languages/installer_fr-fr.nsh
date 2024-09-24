; Languge-Culture:		FR-FR
; LangId:			1036
; CodePage:			1252
; Revision:			6
; Last updated:			25.10.2013
; Author:			Veekee
; Email:			veekee@todae.fr

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
; nov 4 2009 > barabanger: added IDS_SEC_ML_ADDONS & IDS_SEC_ML_ADDONS_DESC, lines 217 & 418 (nov 6 2009 > ychen: modified description)
; nov 13 2009 > barabanger: added IDS_SEC_MUSIC_BUNDLE & DESC, lines 218 & 419. Edited "Downloading" string
; nov 22 2009 > barabanger: updated music bundle related text (see prev rec)
; nov 24 2009 > djegg: updated IDS_SEC_ML_ONLINE_DESC
; nov 30 2009 > smontgo: added IDS_DXDIST for download of DirectX9 web installer (for d3dx9 libs for osd)
; dec 01 2009 > smontgo: added IDS_DIRECTX_INSTALL_ERR to report directx download or install error
; dec 04 2009 > barabanger: removed IDS_DXDIST and IDS_DIRECTX_ISNTALL_ERR
; dec 04 2009 > barabanger: added DirectX Section: IDS_DIRECTX_*
; dec 11 2009 > smontgo: edited IDS_DIRECTX_EMBED_CONNECT_FAILED string (Your computer is missing a)
; jan 22 2010 > djegg: added IDS_CLEANUP_PLUGINS & IDS_REMOVE_SKINS to 'installation strings' (lines 247-248)
; jan 22 2010 > djegg: added IDS_MSN7_PLUGIN_DISABLE to end of 'Messages' section (line 342)
; mar 15 2010 > djegg: removed msgRemoveSettings for uninstaller (line 347)
; mar 15 2010 > barabanger: new uninstaller strings (lines 463-479)
; apr 22 2010 > barabanger: updated uninstaller strings (lines 465,467,470,472,474,476,481)
; may 11 2010 > barabanger: changed text for IDS_UNINSTALL_FEEDBACK_LINK_TEXT (line 485)
; may 26 2010 > djegg: added pmp_android (lines 184 & 461)
; may 27 2010 > barabanger: changed/added uninstaller strings again (w.i.p. - liable to change) (lines 472, 473, 486 / new: 487, 488)
; sep 29 2010 > benski: added pmp_wifi (lines 182 & 457)
; nov 08 2010 > barabanger: updated IDS_PAGE_WELCOME_TEXT // nov 12 2010 > added extra line inbetween welcome text and bullet points // nov 19 2010 > updated welcome text
; nov 12 2010 > barabanger: Commented-out Winamp Remote from bundle page (line 321)
; dec 04 2010 > djegg: added IDS_LYRICS_PLUGIN_DISABLE for disabling incompatible gen_lyrics plugin (line 349)
; dec 04 2010 > djegg: added IDS_LYRICS_PLUGIN_WARNING for warning about incompatible gen_lyrics plugin (line 351)
; jun 23 2011 > djegg: changed AAC/aacPlus to HE-AAC (secAACE, line 123)
; jun 27 2011 > barabanger: added Live Media plugin for French installer only (lines 234, 389 & 471)
; jul 26 2013 > djegg: added IDS_FIREWALL (line 497)
; aug 26 2013 > barabanger: updated welcome page text (lines 297-307)
; aug 26 2013 > barabanger: added AVG promo (lines 520-530 = replacement for OpenCandy & Emusic/AOL Toolbar/Search)
; sep 11 2013 > dro: added safe mode start menu item

!insertmacro LANGFILE_EXT "French"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Langue de l'installation"
${LangFileString} LANGUAGE_DLL_INFO "Veuillez sélectionner la langue utilisée par le programme d'installation."
 
${LangFileString} installFull "Complète"
${LangFileString} installStandard "Standard"
${LangFileString} installLite "Légère"
${LangFileString} installMinimal "Minimale"
${LangFileString} installPrevious "Installation précédente"

; BrandingText
${LangFileString} BuiltOn "généré le"
${LangFileString} at "à"

${LangFileString} installWinampTop "Ceci installera la version ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}. "
${LangFileString} installerContainsFull "Cette installation contient la version complète."
${LangFileString} installerContainsLite "Cette installation contient la version légère."
${LangFileString} licenseTop "Veuillez lire et accepter les termes de la licence ci-dessous, avant de continuer l'installation."
${LangFileString} directoryTop "L'installation a déterminée l'emplacement optimal pour l'installation. Si vous souhaitez modifier le répertoire d'installation, c'est le moment."

${LangFileString} uninstallPrompt "Ceci désinstallera Winamp. Voulez-vous continuer ?"

${LangFileString} msgCancelInstall "Voulez-vous annuler l'installation ?"
${LangFileString} msgReboot "Un redémarrage est nécessaire afin de terminer l'installation.$\r$\nVoulez-vous redémarrer dès maintenant ? (Si vous voulez redémarrer plus tard, sélectionnez Non)"
${LangFileString} msgCloseWinamp "Veuillez préalablement fermer Winamp avant de continuer.$\r$\n$\r$\nAprès avoir fermé Winamp, veuillez sélectionner Réessayer.$\r$\n$\r$\nSi vous souhaitez tout de même essayer de continuer, sélectionnez Ignorer.$\r$\n$\r$\nSi vous souhaitez annuler l'installation, sélectionnez Abandonner."
${LangFileString} msgInstallAborted "Installation abandonnée par l'utilisation"

${LangFileString} secWinamp "Winamp (requis)"
${LangFileString} compAgent "Agent Winamp"
${LangFileString} compModernSkin "Support des skins modernes"
${LangFileString} uninstallWinamp "Désinstaller Winamp"

${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Télécharger et installer le format Windows Media"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "Lecture OGG Vorbis"
${LangFileString} secOGGEnc "Encodage OGG Vorbis"
${LangFileString} secAACE "Encodage HE-AAC"
${LangFileString} secMP3E "Encodage MP3"
${LangFileString} secMP4E "Support du MP4"
${LangFileString} secWMAE "Encodage WMA"
${LangFileString} msgWMAError "Un problème est survenu lors de l'installation des composants. L'encodage WMA ne sera pas installé. Veuillez visiter http://www.microsoft.com/windows/windowsmedia/forpros/encoder/default.mspx , téléchargez l'encodeur et réessayez."
${LangFileString} secCDDA "Lecture et extraction de CD" 
${LangFileString} msgCDError "Un problème est survenu lors de l'installation des composants. L'extraction/gravure CD peut ne pas fonctionner correctement. "
${LangFileString} secCDDB "CDDB pour la reconnaissance des CD"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"

${LangFileString} secDSP "Studio de Traitement du Signal"
${LangFileString} secWriteWAV "Enregistreur WAV classique"
${LangFileString} secLineInput "Support Entrée Ligne"
${LangFileString} secDirectSound "Support Sortie DirectSound"

${LangFileString} secHotKey "Support des raccourcis clavier"
${LangFileString} secJmp "Support Lire Tout De Suite"
${LangFileString} secTray "Contrôleur Système Nullsoft"

${LangFileString} msgRemoveMJUICE "Voulez-vous vraiment enlever le support MJuice de votre système ?$\r$\n$\r$\nSélectionnez OUI à moins que vous n'utilisiez des fichiers MJF dans des programmes autre que Winamp."
${LangFileString} msgNotAllFiles "Tous les fichiers n'ont pu être enlevés.$\r$\nSi vous souhaitez enlever les fichiers restant par vous-même, allez-y !"


${LangFileString} secNSV "Nullsoft Video (NSV)"
${LangFileString} secDSHOW "Formats DirectShow (MPG, M2V)"
${LangFileString} secAVI "Vidéo AVI"
${LangFileString} secFLV "Vidéo Flash (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "Vidéo MPEG-4 (MP4, MP4V)"

${LangFileString} secSWF "Protocole Flash Media (SWF, RTMP)"

${LangFileString} secTiny "Simple Plein-Écran"
${LangFileString} secAVS "Studio de Visualisation Avancé"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Médiathèque Winamp"
${LangFileString} secOM "Média Internet"
${LangFileString} secWire "Dossier Podcast"
${LangFileString} secPmp "Lecteurs multimédia portables"
${LangFileString} secPmpIpod "iPod®"
${LangFileString} secPmpCreative "Creative®"
${LangFileString} secPmpP4S "Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "Périphériques USB"
${LangFileString} secPmpActiveSync "Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid "Périphériques Android"
${LangFileString} secPmpWifi "Support du Wifi Android"

${LangFileString} sec_ML_LOCAL "Média local"
${LangFileString} sec_ML_PLAYLISTS "Listes de lecture"
${LangFileString} sec_ML_DISC "Extraction & Gravure CD"
${LangFileString} sec_ML_BOOKMARKS "Signets"
${LangFileString} sec_ML_HISTORY "Historique"
${LangFileString} sec_ML_NOWPLAYING "Lecture en cours"
${LangFileString} sec_ML_RG "Outil d'analyse Replay Gain"
${LangFileString} sec_ML_TRANSCODE "Outil de conversion"
${LangFileString} sec_ML_PLG "Générateur de liste"
${LangFileString} sec_ML_IMPEX "Outil d'importation/exportation"


;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "Installation de $(^NameDA)"
${LangFileString} IDS_SELECT_LANGUAGE  "Veuillez sélectionner la langue utilisée par le programme d'installation"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Moteur multimédia"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Sortie"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Lecture audio"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Encodage audio"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Lecture vidéo"
${LangFileString} IDS_GRP_VISUALIZATION		"Visualisation"
${LangFileString} IDS_GRP_UIEXTENSION		"Extension de l'interface utilisateur"
${LangFileString} IDS_GRP_WALIB				"Médiathèque Winamp"
${LangFileString} IDS_GRP_WALIB_CORE		"Composants principaux"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Support des périphériques portables"
${LangFileString} IDS_GRP_LANGUAGES 	    "Langues"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"Sortie WaveOut/MME"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"Encodage FLAC"
${LangFileString} IDS_SEC_MILKDROP2     "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG		"Tag automatique"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO "Radios Françaises"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Configuration des services Internet..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Vérification de la présence d'une autre instance de Winamp en cours d'exécution..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Ouverture d'une connexion vers l'Internet..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Validation de l'accessibilité à l'Internet..."
${LangFileString} IDS_RUN_NOINET				"Aucune connexion à l'Internet disponible"
${LangFileString} IDS_RUN_EXTRACT				"Extraction de"
${LangFileString} IDS_RUN_DOWNLOAD				"Téléchargement de"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Téléchargement terminé."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Echec du téléchargement. Raison :"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Téléchargement abandonné."
${LangFileString} IDS_RUN_INSTALL				"Installation de"
${LangFileString} IDS_RUN_INSTALLFIALED			"Echec de l'installation."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Le fichier n'a pas été trouvé. Le téléchargement est replanifié."
${LangFileString} IDS_RUN_DONE					"Terminé."

${LangFileString} IDS_DSP_PRESETS 	"Définitions DSP"
${LangFileString} IDS_DEFAULT_SKIN	"Skins par défaut"
${LangFileString} IDS_AVS_PRESETS	"Définitions AVS"
${LangFileString} IDS_MILK_PRESETS	"Définitions MilkDrop"
${LangFileString} IDS_MILK2_PRESETS	"Définitions MilkDrop2"

${LangFileString} IDS_CLEANUP_PLUGINS	"Nettoyage des plugins..."
${LangFileString} IDS_REMOVE_SKINS		"Suppression des skins par défaut..."


; download
${LangFileString} IDS_DOWNLOADING	"Téléchargement de '%s'"
${LangFileString} IDS_CONNECTING	"Connexion..."
${LangFileString} IDS_SECOND		" (1 seconde restante)"
${LangFileString} IDS_MINUTE		" (1 minute restante)"
${LangFileString} IDS_HOUR			" (1 heure restante)"
${LangFileString} IDS_SECONDS		" (%u secondes restantes)"
${LangFileString} IDS_MINUTES		" (%u minutes restantes)"
${LangFileString} IDS_HOURS			" (%u heures restantes)"
${LangFileString} IDS_PROGRESS		"%sko (%d%%) sur %sko @ %u.%01uko/s"


; AutoplayHandler
${LangFileString} AutoplayHandler	"Lecture"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Installation terminée"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) a correctement été installé sur votre ordinateur.$\r$\n$\r$\n\
													Cliquez sur Fermer pour fermer cet assistant."
${LangFileString} IDS_PAGE_FINISH_RUN		"Démarrer $(^NameDA)"
${LangFileString} IDS_PAGE_FINISH_LINK		"Cliquez ici pour visiter Winamp.com"


; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE		"Bienvenue dans le programme d'installation de $(^NameDA)"

!ifdef EXPRESS_MODE
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) vous permet d'écouter, de regarder et de gérer \
											la musique, les vidéos les podcasts et les radios Internet, \
											depuis chez vous, votre travail ou votre voiture.\
											$\r$\n$\r$\n\
											Il comprend :$\r$\n$\r$\n  \
											•  la synchronisation sans fil de vos médias vers l$\'application $(^NameDA) pour Android$\r$\n$\r$\n  \
											•  le nettoyage de vos métadonnées avec Auto-Tag$\r$\n$\r$\n  \
											•  un générateur automatique de listes de lecture personnalisées$\r$\n$\r$\n  \
											•  l$\'accès à plus de 50 000 stations SHOUTcast dans le monde$\r$\n$\r$\n  \
											•  l$\'accès à plus de 30 000 podcasts"
${LangFileString} IDS_PAGE_WELCOME_LEGAL	"En cliquant sur “Suivant”, vous acceptez les <a id=$\"winamp_eula$\">Accords de licence</a> ainsi que la <a id=$\"winamp_privacy_policy$\">politique de confidentialité</a> de $(^NameDA)."
!else
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA)vous permet de gérer votre médiathèque et d'écouter les radios Internet.$\r$\n$\r$\n\
											Il comprend :$\r$\n$\r$\n  \
												•  la synchronisation sans fil de vos médias vers l$\'application $(^NameDA) pour Android$\r$\n$\r$\n  \
												•  un générateur automatique de listes de lecture personnalisées$\r$\n$\r$\n  \
												• l$\'accès à plus de 30 000 podcasts"

!endif ; defined (EXPRESS_MODE)

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"NOTE : Pour profiter de toutes ces fonctionnalités ainsi que \
															de l$\'interface du nouveau skin “Bento” (recommandé), tous les \
															composants doivent être cochés."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Sélection des paramètres d'accessibilité"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Choisissez les différents raccourcis vers Winamp à créer."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Veuillez sélectionner, parmis les possibilités suivantes, les différents raccourcis vers Winamp à créer."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Raccourci dans le menu démarrer"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Icône dans la barre de Lancement Rapide"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Icône sur votre Bureau"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Impossible de fermer l'Agent Winamp.$\r$\n\
                                                   Vérifiez qu'un autre utilisateur n'est pas connecté sur Windows.\
                                                   $\r$\n$\r$\n	Après avoir fermé l'Agent Winamp, sélectionnez Réessayer.\
                                                   $\r$\n$\r$\n	Si vous voulez tout de même continuer l'installation, sélectionnez Ignorer.\
                                                   $\r$\n$\r$\n	Si vous souhaitez abandonner l'installation, sélectionnez Abandonner."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Cette version de Windows n'est plus supportée.$\r$\n\
                                                 $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} nécessite le système d'exploitation Windows 2000 ou supérieur."

; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"Le plugin gen_msn7.dll a été détecté !$\r$\n$\r$\nCe plugin entraine le non fonctionnement des versions Winamp 5.57 et supérieures.$\r$\nCe plugin va être désactivé. Cliquez sur OK pour continuer."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"Le plugin tiers gen_lyrics.dll a été détecté !$\r$\n$\r$\nCe plugin entraine le non fonctionnement des versions Winamp 5.59 et supérieures.$\r$\nCe plugin va être désactivé. Cliquez sur OK pour continuer."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "Le plugin tiers gen_lyrics.dll a été détecté !$\r$\n$\r$\nLes anciennes versions de ce plugin sont incompatibles avec Winamp 5.6 et supérieur. Veuillez vérifier que vous utilisez la version la plus récente sur http://lyricsplugin.com avant de continuer."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"${DIRECTXINSTAL_WINVER_LO} ou inférieur détecté"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "${DIRECTXINSTAL_WINVER_HI} ou supérieur détecté"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Validation de la version de ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Version minimum requise de ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Impossible de détecter votre version de ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"Version détectée de ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Version de ${DIRECTXINSTAL_DIRECTXNAME} non supportée"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Validation de la présence de $0"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Téléchargement requis"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Validation de la connectivité à l'Internet"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"La dernière version de ${DIRECTXINSTAL_DIRECTXNAME} est disponible sur"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Téléchargement de l'installation de ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FOUND						"Trouvé"
${LangFileString} IDS_DIRECTX_MISSING					"Manquant"
${LangFileString} IDS_DIRECTX_SUCCESS					"Succès"
${LangFileString} IDS_DIRECTX_ABORTED					"Annulé"
${LangFileString} IDS_DIRECTX_FAILED					"Echec"
${LangFileString} IDS_DIRECTX_DONE						"Terminé"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Lancement de l'installation de ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"${DIRECTXINSTAL_WINAMPNAME} requiert au moins ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} pour fonctionner correctement.$\r$\nVoulez-vous l'installer dès maintenant ?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"${DIRECTXINSTAL_WINAMPNAME} requiert au moins ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} pour fonctionner correctement"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Impossible de télécharger ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Impossible d'installer ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"Votre ordinateur ne possède pas un composant de ${DIRECTXINSTAL_DIRECTXNAME} requis par ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Impossible de télécharger le composant de ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Impossible d'installer le composant manquant de ${DIRECTXINSTAL_DIRECTXNAME}"

;French Radio Section
${LangFileString} IDS_FRENCHRADIO_INSTALLING			"Installation de $(IDS_SEC_GEN_FRENCHRADIO)..."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Noyau Winamp (requis)"
${LangFileString} IDS_SEC_AGENT_DESC			"Ajoute une icône Winamp près de l'horloge, et maintient l'association entre Winamp et les fichiers multimédia"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Moteur multimédia (système Entrée/Sortie)"
${LangFileString} IDS_SEC_CDDB_DESC				"Permet la récupération automatique des titres de vos CD à partir de la base de données en ligne Gracenote"
${LangFileString} IDS_SEC_DSP_DESC				"Ajout d'effets supplémentaires comme le contrôle du chœur, de la modulation, du tempo et de la tonalité"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Support de la lecture audio (plugins d'entrée : décodeurs audio)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Support de la lecture des formats MP3, MP2, MP1 et AAC (requis)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Support de la lecture du format WMA (incluant le support des DRM)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Support de la lecture des formats MIDI (MID, RMI, KAR, MUS, CMF, etc.)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Support de la lecture des formats Module (MOD, XM, IT, S3M, ULT, etc.)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Support de la lecture du format Ogg Vorbis (OGG)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Support de la lecture des formats audio MPEG-4 (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Support de la lecture du format FLAC"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Support de la lecture des CD Audio"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Support de la lecture des formats Waveform (WAV, VOC, AU, AIFF, etc.)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Support de la lecture vidéo (plugins d'entrée : décodeurs vidéo)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Support de la lecture des formats vidéo Windows Media (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Support de la lecture du format Nullsoft Video (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Support de la lecture des MPEG-1/2 et autres formats vidéo"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Support de la lecture du format vidéo AVI"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Support de la lecture des formats vidéo Flash vp6 (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Support de la lecture du format vidéo Matroska (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Support de la lecture du format vidéo MPEG-4 (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Support de la lecture du format des flux Adobe Flash (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Support de l'encodage et de la conversion de fichiers d'un format à un autre"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Support de l'extraction et de la conversion vers le format WMA"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Support de l'extraction et de la conversion vers le format WAV"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Support de l'extraction et de la conversion vers les formats M4A et AAC"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Support de l'extraction et de la conversion vers le format FLAC"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Support de l'extraction et de la conversion vers le format Ogg Vorbis"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Plugins de Sortie (qui contrôlent comment le signal audio est traité et envoyé à votre carte son)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Enregistreur WAV/MME classique (déprécié, mais certains utilisateurs le préfère aux plugins de conversion)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"Sortie DirectSound (requis / plugin de Sortie par défaut)"	
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Sortie WaveOut classique (optionnel, ni requis ni recommandé)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Extensions de l'Interface Utilisateur"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Permet le contrôle de Winamp depuis votre clavier alors que vous travaillez avec d'autres applications"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Ajoute l'insertion de morceaux dans la liste d'attente et de nombreuses autres fonctionnalités"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Plugin Contrôleur Système Nullsoft, pour l'ajout d'icônes de contrôle de la lecture dans la barre système"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Support des skins modernes, requis pour l'utilisation des skins avancés comme Winamp Modern et Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Plugins de visualisation"
${LangFileString} IDS_SEC_NSFS_DESC				"Plugin de visualisation Simple Plein-Écran"
${LangFileString} IDS_SEC_AVS_DESC				"Plugin Studio de Visualisation avancé"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Plugin de visualisation Milkdrop"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Plugin de visualisation Milkdrop 2 (plugin par défaut)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Support de l'Entrée Ligne à travers la commande linein:// (permet l'écoute de l'entrée micro/ligne)"
${LangFileString} IDS_GRP_WALIB_DESC			"Médiathèque Winamp"
${LangFileString} IDS_SEC_ML_DESC				"Médiathèque Winamp (requise)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Outil de conversion, utilisé pour convertir un média d'un format vers un autre"
${LangFileString} IDS_SEC_ML_RG_DESC			"Outil d'analyse Replay Gain, utilisé pour le nivellement du volume"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Propulsé par Gracenote. Permet de remplir automatiquement les métadonnées manquantes"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Dossier Podcast, pour l'abonnement et le téléchargement de podcasts" 
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Services Internet, incluant SHOUTcast Radio && TV, In2TV, AOL Radio avec CBS Radio, les Charts Winamp et d'autres encore"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Propulsé par Gracenote. Permet la création de listes de lecture acoustiquement dynamiques"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Composants principaux de la médiathèque"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Base de données locale de vos média, incluant un moteur de recherche puissant ainsi que des vues intelligentes"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Gère la création, la modification et la centralisation de toutes vos listes de lecture"
${LangFileString} IDS_SEC_ML_DISC_DESC			"Extraction && Gravure de CD, l'interface de la médiathèque pour l'extraction && la gravure de CD Audio"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Gestionnaire de signets, pour garder en mémoire vos flux Internet, fichiers ou dossiers préférés"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Historique, pour un accès instantanné à tous les flux et fichiers locaux ou distants joués récemment"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Lecture en cours, pour l'affichage des informations sur le morceau en cours d'écoute"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Support des lecteurs multimédia portables" 
${LangFileString} IDS_SEC_ML_PMP_DESC			"Plugin noyau de support des lecteur smultimédia portables (requis)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"Support de l'iPod®"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Support des périphériques Creative® (pour la gestion des lecteurs Nomad™, Zen™ et MuVo™)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Support de Microsoft® PlaysForSure® (pour les gestion de tous les lecteurs compatibles P4S)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"Support des périphériques USB (pour la gestion des clés ou lecteurs USB génériques)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Support de Microsoft® ActiveSync® (périphériques Windows Mobile®, Smartphone && Pocket PC)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Support des périphériques Android"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Support au Wifi Android"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"Plugin d'importation/explortation d'une médiathèque compatible iTunes"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO_DESC	"Ecoutez les radios francophones, directement dans $(^NameDA) (Virgin radio, NRJ, RTL, Skyrock, RMC...)"

${LangFileString} IDS_FIREWALL					"Ajouter les règles dans les firewall"

${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Désinstalle $(^NameDA) de votre ordinateur."

${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Dossier de $(^NameDA) :$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Lecteur multimédia"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Désinstalle tous les composants du lecteur multimédia $(^NameDA), ainsi que les plugins tiers.."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Préférences utilisateur"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Supprime toutes les préférences et les plugins de $(^NameDA)."

${LangFileString} IDS_UNINSTALL_FEEDBACK_LINK_TEXT		"Aidez-nous à améliorer $(^NameDA) en nous apportant vos retours."
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"Ouvrir le dossier de $(^NameDA)"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nNote : Tous les fichiers n'ont pas pu être supprimés pendant la désinstallation. Pour les afficher, ouvrez le dossier de Winamp."

!ifdef EXPRESS_MODE
${LangFileString} IDS_EXPRESS_MODE_HEADER "Type d'installation de $(^NameDA)"
${LangFileString} IDS_EXPRESS_MODE_SUBHEADER "Veuillez sélectionner le type d'installation"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_INSTALL_RADIO "Installation &standard"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_INSTALL_TEXT "Cela installera $(^NameDA), avec tous les composants sélectionnés, dans$\r$\n\
                                                          '$INSTDIR'"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_REINSTALL_TEXT "Cela installera $(^NameDA), avec les composants précédement sélectionnés, dans$\r$\n\
                                                          '$INSTDIR'"
${LangFileString} IDS_EXPRESS_MODE_CUSTOM_INSTALL_RADIO "Installation &personnalisée"
${LangFileString} IDS_EXPRESS_MODE_CUSTOM_INSTALL_TEXT "L'installation personnalisée vous permet de construire un $(^NameDA) selon vos goûts$\r$\n\
                                                        en sélectionnant manuellement les composants à intégrer."
!endif ; defined (EXPRESS_MODE) 