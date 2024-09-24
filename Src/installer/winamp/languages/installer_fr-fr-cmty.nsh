; Languge-Culture:	FR-FR
; LangId:			1036
; CodePage:			1252
; Revision:			1.2
; Last updated:			26.10.2007
; Author:			Todae
; Email:			veekee@todae.fr

!insertmacro LANGFILE_EXT "French"

${LangFileString} installFull "Complète"
${LangFileString} installStandard "Standard"
${LangFileString} installLite "Légère"
${LangFileString} installMinimal "Minimale"
${LangFileString} installPrevious "Installation précédente"

${LangFileString} installWinampTop "Ceci installera la version ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}.  "
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
${LangFileString} secAACE "Encodage AAC/aacPlus"
${LangFileString} secMP3E "Encodage MP3"
${LangFileString} secMP4E "Support du MP4"
${LangFileString} secWMAE "Encodage WMA"
${LangFileString} msgWMAError "Un problème est survenu lors de l'installation des composants. L'encodage WMA ne sera pas installé. Veuillez visiter http://www.microsoft.com/windows/windowsmedia/forpros/encoder/default.mspx , téléchargez l'encodeur et réessayez."
${LangFileString} secCDDA "Lecture et extraction de CD" 
${LangFileString} msgCDError "Un problème est survenu lors de l'installation des composants. L'extraction/gravure CD peut ne pas fonctionner correctement. "
${LangFileString} secCDDB "CDDB pour la reconnaissance des CDs"
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
${LangFileString} secDSHOW "AVI/MPG"


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

${LangFileString} sec_ML_LOCAL "Média local"
${LangFileString} sec_ML_PLAYLISTS "Listes de lecture"
${LangFileString} sec_ML_DISC "Extraction & Gravure CD"
${LangFileString} sec_ML_BOOKMARKS "Signets"
${LangFileString} sec_ML_HISTORY "Historique"
${LangFileString} sec_ML_NOWPLAYING "Lecture en cours"
${LangFileString} sec_ML_RG "Outil d'analyse Replay Gain"
${LangFileString} sec_ML_TRANSCODE "Outil de conversion"

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


${LangFileString} IDS_SEC_ML_AUTOTAG		"Catégorisation automatique"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Configuration des services Internet..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Vérification de la présence d'une autre instance de Winamp en cours d'exécution..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Ouverture d'une connexion vers l'Internet..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Validation de l'accessibilité à l'Internet..."
${LangFileString} IDS_RUN_NOINET				"Aucune connexion à l'Internet disponible"
${LangFileString} IDS_RUN_EXTRACT				"Extraction"
${LangFileString} IDS_RUN_DOWNLOAD				"Téléchargement"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Téléchargement terminé."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Echec du téléchargement. Raison :"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Téléchargement abandonné."
${LangFileString} IDS_RUN_INSTALL				"Installation"
${LangFileString} IDS_RUN_INSTALLFIALED			"Echec de l'installation."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Le fichier n'a pas été trouvé. Le téléchargement est replanifié."
${LangFileString} IDS_RUN_DONE					"Terminé."

${LangFileString} IDS_DSP_PRESETS 	"Définitions DSP"
${LangFileString} IDS_DEFAULT_SKIN	"Skins par défaut"
${LangFileString} IDS_AVS_PRESETS	"Définitions AVS"
${LangFileString} IDS_MILK_PRESETS	"Définitions MilkDrop"

; download
${LangFileString} IDS_DOWNLOADING	"Téléchargement de %s"
${LangFileString} IDS_CONNECTING	"Connexion..."
${LangFileString} IDS_SECOND		" (1 seconde restante)"
${LangFileString} IDS_MINUTE		" (1 minute restante)"
${LangFileString} IDS_HOUR			" (1 heure restante)"
${LangFileString} IDS_SECONDS		" (%u secondes restantes)"
${LangFileString} IDS_MINUTES		" (%u minutes restantes)"
${LangFileString} IDS_HOURS			" (%u heures restantes)"
${LangFileString} IDS_PROGRESS		"%sko (%d%%) sur %sko @ %u.%01uko/s"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Installation terminée"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) a correctement été installé sur votre ordinateur.$\r$\n$\r$\n\
													Cliquez sur Terminer pour fermer cet assistant."
${LangFileString} IDS_PAGE_FINISH_RUN		"Exécuter $(^NameDA)"
${LangFileString} IDS_PAGE_FINISH_LINK		"Cliquez ici pour visiter Winamp.com"

; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE		"Bienvenue dans le programme d'installation de $(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT		'Soyez prêt à essayer le gestionnaire multimédia ultime !$\r$\n$\r$\n\
													Savourez vos musiques, vidéos et podcasts préférés. Récupérez et gravez vos CD, créez vos listes de lecture, \
                          synchronisez votre lecteur multimédia portable, et partagez vos coups de coeur avec vos amis. Découvrez de nouveaux univers musicaux à travers les milliers de stations radio disponibles, \
                          les vidéos et les critiques d$\'artistes. $\r$\n  \
													•  Une toute nouvelle interface, le skin Bento$\r$\n  \
													•  Le support de vos périphériques multimédia, iPod® inclu$\r$\n  \
													•  Le support des couvertures des albums$\r$\n  \
													•  Les meilleures musiques disponibles sur l$\'Internet$\r$\n  \
													•  Des recommandations musicales$\r$\n  \
													•  Le support du son MP3 Surround$\r$\n  \
													•  L$\'accès à distance à votre musique et à vos vidéos$\r$\n  \
													•  La catégorisation automatiquement de la musique'

; components
${LangFileString} IDS_PAGE_COMPONENTS_INSTTYPE		'Type d$\'installation :'
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		'NOTE : Pour profiter de toutes ces fonctionnalités ainsi que \
															de l$\'interface du nouveau skin “Bento” (recommandé), tous les \
															composants doivent être cochés.'

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Sélection des paramètres d'accessibilité"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Choisissez les différents raccourcis vers Winamp à créer."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Veuillez sélectionner, parmi les possibilités suivantes, les différents raccourcis vers Winamp à créer."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Raccourci dans le menu démarrer"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Icône dans la barre de Lancement Rapide"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Icône sur votre Bureau"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Impossible de fermer l'Agent Winamp.$\r$\n\
                                                   Vérifiez qu'un autre utilisateur n'est pas connecté sur Windows.\
                                                   $\r$\n$\r$\n	Après avoir fermé l'Agent Winamp, sélectionnez Réessayer.\
                                                   $\r$\n$\r$\n	Si vous voulez tout de même continuer l'installation, sélectionnez Ignorer.\
                                                   $\r$\n$\r$\n	Si vous souhaitez abandonner l'installation, sélectionnez Abandonner."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Noyau Winamp (requis)"
${LangFileString} IDS_SEC_AGENT_DESC			"Ajoute une icône Winamp près de l'horloge, et maintient l'association entre Winamp et les fichiers multimédia"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Moteur multimédia (système Entrée/Sortie)"
${LangFileString} IDS_SEC_CDDB_DESC				"Permet la récupération automatiquement des titres de vos CD à partir de la base de données en ligne Gracenote"
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
${LangFileString} IDS_SEC_DSHOW_DEC_DESC			"Support de la lectyre des formats vidéo AVI && MPEG"
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
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Plugin Contrôleur Système Nullsoft, pour l'ajout d'icônes de contrôle de la lecture dans la barre système."
${LangFileString} IDS_SEC_FREEFORM_DESC			"Support des skins modernes, requis pour l'utilisation des skins avancés comme Winamp Modern et Winamp Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Plugins de visualisation"
${LangFileString} IDS_SEC_NSFS_DESC				"Plugin de visualisation Simple Plein-Écran"
${LangFileString} IDS_SEC_AVS_DESC				"Plugin Studio de Visualisation avancé (plugin par défaut)"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Plugin de visualisation Milkdrop"
${LangFileString} IDS_SEL_LINEIN_DESC			"Support de l'Entrée Ligne à travers la commande linein:// (permet l'écoute de l'entrée micro/ligne)"
${LangFileString} IDS_GRP_WALIB_DESC			"Médiathèque Winamp"
${LangFileString} IDS_SEC_ML_DESC				"Médiathèque Winamp (requise)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Outil de conversion, utilisé pour convertir un média d'un format vers un autre"
${LangFileString} IDS_SEC_ML_RG_DESC			"Outil d'analyse Replay Gain, utilisé pour le nivellement du volume"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Propulsé par Gracenote. Permet de remplir automatiquement les métadonnées manquantes"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Dossier Podcast, pour l'abonnement et le téléchargement de podcasts" 
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Services Internet, incluant les SHOUTcast Radio && TV, In2TV, AOL Videos et les flux XM Radio"
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
