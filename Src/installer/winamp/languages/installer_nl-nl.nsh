; Winamp Installer Translation file.
;
; Languge-Culture:	NL-NL
; LangId:		1043
; CodePage:		1252
; Revision:		6
; Last updated:		06.03.2011
; Author:		Updates by Paul van Garderen
; Email:		paul@winampnederlands.nl
; Community:		www.winampnederlands.nl
;
; History:
; 27.10.07	* Added milkdrop2 strings,
;		  IDS_SEC_MILKDROP2 & IDS_MILK2_PRESETS & IDS_SEC_MILKDROP2_DESC
;
; 05.11.07	* Added FLV strings,
;		  secFLV & IDS_SEC_FLV_DEC_DESC
;		* Various changes...
;
; 18.11.07	* Added IDS_MSG_WINDOWS_TOO_OLD message
;
; 22.03.08	* Added IDS_BUNDLE21_NAME, _DESCRIPTION & IDS_WINAMP_SEARCH strings
;		* Added suffix 360 to Xbox
;		* Various small changes
;
; 26.03.08	* Edited IDS_WINAMP_SEARCH; removed Google®
;
; 18.05.08	* "(standaard visualisatie plug-in)" verplaatst van IDS_SEC_AVS_DESC naar IDS_SEC_MILKDROP2_DESC
; 
; 07.07.08	* Added secSWF & IDS_SEC_SWF_DEC_DESC
;		* Added IDS_SEC_GEN_DROPBOX & IDS_SEC_GEN_DROPBOX_DESC
;		* Added SEC_ML_PLG
;		* Changed emusic bundle text items
;
; 14.01.09	* Added BrandingText section
;		* Added Winmp3 installation strings
;
; 08.02.09	* Added AutoplayHandler
;
; 30.11.09	* Up-to-date with Nov. 20th EN-US version
;
; 13.12.09	* Added DirectX section
;
; 06.03.11	* Up-todate until Dec. 4th 2010 EN-US
;

!insertmacro LANGFILE_EXT "Dutch"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE	"Installatietaal"
${LangFileString} LANGUAGE_DLL_INFO	"Selecteer een taal."

${LangFileString} installFull 		"Volledig"
${LangFileString} installStandard 	"Standaard"
${LangFileString} installLite 		"Lite"
${LangFileString} installMinimal 	"Minimaal"
${LangFileString} installPrevious 	"Vorige Installatie"

; BrandingText
${LangFileString} BuiltOn		"gebouwd op"
${LangFileString} at			"om"

${LangFileString} installWinampTop 	"Hiermee installeert u Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}.  "
${LangFileString} installerContainsFull "Dit installatieprogramma omvat de volledige versie."
${LangFileString} installerContainsLite "Dit installatieprogramma omvat de lite-versie."
${LangFileString} licenseTop 		"Lees vóór installatie de licentievoorwaarden zorgvuldig door en accepteer deze indien u hiermee instemt."
${LangFileString} directoryTop 		"Het installatieprogramma heeft de optimale locatie voor $(^NameDA) bepaald. Als u het programma in een andere map wilt installeren, kunt u deze nu opgeven."

${LangFileString} uninstallPrompt 	"Hierdoor wordt Winamp gedeïnstalleerd. Doorgaan?"

${LangFileString} msgCancelInstall 	"Installatie annuleren?"
${LangFileString} msgReboot 		"U moet de computer opnieuw opstarten om de installatie te voltooien.$\r$\nNu opnieuw opstarten? (Selecteer Nee als u op een later moment opnieuw wilt opstarten)"
${LangFileString} msgCloseWinamp 	"Voordat u verder kunt, moet u eerst Winamp afsluiten.$\r$\n$\r$\n	Nadat u Winamp hebt afgesloten, selecteert u Opnieuw.$\r$\n$\r$\n	Als u de installatie wilt voortzetten zonder Winamp af te sluiten, selecteert u Negeren.$\r$\n$\r$\n	Als u de installatie wilt annuleren, selecteert u Afbreken."
${LangFileString} msgInstallAborted 	"Installatie afgebroken door gebruiker"

${LangFileString} secWinamp 		"Winamp (vereist)"
${LangFileString} compAgent 		"Winamp Agent"
${LangFileString} compModernSkin 	"Ondersteuning voor Moderne Skins"
${LangFileString} uninstallWinamp 	"Winamp Verwijderen"

${LangFileString} secWMA 		"Windows Media Audio (WMA)"
${LangFileString} secWMV 		"Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist 		"Windows Media-formaat downloaden en installeren"

${LangFileString} secMIDI 		"MIDI"
${LangFileString} secMOD 		"MOD/XM/S3M/IT"
${LangFileString} secOGGPlay 		"OGG Vorbis ondersteuning"
${LangFileString} secOGGEnc 		"OGG Vorbis-codering"
${LangFileString} secAACE 		"HE-AAC-codering"
${LangFileString} secMP3E 		"MP3-codering"
${LangFileString} secMP4E 		"MP4-ondersteuning"
${LangFileString} secWMAE 		"WMA-codering"
${LangFileString} msgWMAError 		"Er is een probleem opgetreden bij de installatie van bepaalde componenten. De WMA-encoder zal niet worden geïnstalleerd. Ga naar http://www.microsoft.com/windows/windowsmedia/9series/encoder/ , download de encoder en probeer het opnieuw."
${LangFileString} secCDDA 		"Afspelen en extraheren van cd's" 
${LangFileString} msgCDError 		"Er is een probleem opgetreden bij de installatie van bepaalde componenten. Het rippen en branden van cd's zal mogelijk niet correct functioneren. "
${LangFileString} secCDDB 		"CDDB voor herkenning van cd's"
${LangFileString} secWAV 		"WAV/VOC/AU/AIFF"

${LangFileString} secDSP 		"Signal Processor Studio Plug-in"
${LangFileString} secWriteWAV 		"Oude versie van WAV-writer"
${LangFileString} secLineInput 		"Ondersteuning voor Line-input"
${LangFileString} secDirectSound 	"Ondersteuning voor DirectSound-output"

${LangFileString} secHotKey 		"Ondersteuning voor globale sneltoetsen"
${LangFileString} secJmp 		"Ondersteuning voor uitgebreide mogelijkheid om naar bestanden te gaan"
${LangFileString} secTray 		"Nullsoft Besturingselement in systeemvak"

${LangFileString} msgRemoveMJUICE 	"Wilt u de MJuice-ondersteuning van uw systeem verwijderen?$\r$\n$\r$\nSelecteer Nee als u de MJF-bestanden in andere programma dan Winamp wilt gebruiken."
${LangFileString} msgNotAllFiles 	"Niet alle bestanden zijn verwijderd.$\r$\nDesgewenst kunt u de bestanden handmatig verwijderen."

${LangFileString} secNSV 		"Nullsoft Video (NSV)"
${LangFileString} secDSHOW 		"DirectShow Formeten (MPG, M2V)"
${LangFileString} secAVI 		"AVI Video"
${LangFileString} secFLV 		"Flash Video (FLV)"

${LangFileString} secMKV		"Matroska (MKV, MKA)"
${LangFileString} secM4V		"MPEG-4 Video (MP4, M4V)"

${LangFileString} secSWF 		"Flash Media Protocol (SWF, RTMP)"

${LangFileString} secTiny 		"Nullsoft Klein Volledig Scherm"
${LangFileString} secAVS 		"Geavanceerde Visualisatie Studio"
${LangFileString} secMilkDrop 		"Milkdrop"

${LangFileString} secML 		"Winamp Mediabibliotheek"
${LangFileString} secOM 		"Online Media"
${LangFileString} secWire 		"Podcast-Directory"
${LangFileString} secPmp 		"Draagbare Mediaspelers"
${LangFileString} secPmpIpod 		"iPod® Ondersteuning"
${LangFileString} secPmpCreative 	"Ondersteuning voor Creative®-spelers"
${LangFileString} secPmpP4S 		"Ondersteuning voor Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB 		"Ondersteuning voor USB-apparaten"
${LangFileString} secPmpActiveSync 	"Ondersteuning voor Microsoft® ActiveSync®"

${LangFileString} sec_ML_LOCAL 		"Lokale Media"
${LangFileString} sec_ML_PLAYLISTS 	"Speellijsten"
${LangFileString} sec_ML_DISC 		"Cd Rippen & Branden"
${LangFileString} sec_ML_BOOKMARKS 	"Favorieten"
${LangFileString} sec_ML_HISTORY 	"Geschiedenis"
${LangFileString} sec_ML_NOWPLAYING 	"Nu Speelt"
${LangFileString} sec_ML_RG 		"Hulpmiddel Gelijkmatig Afspeelvolume"
${LangFileString} sec_ML_TRANSCODE 	"Hulpmiddel voor transcodering"
${LangFileString} sec_ML_PLG 		"Speellijst Generator"
${LangFileString} sec_ML_IMPEX		"Database Import/Export Tool"

;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          		"Installatieprogramma van $(^NameDA)"
${LangFileString} IDS_SELECT_LANGUAGE  		"Selecteer de taal voor het installatieprogramma"

; Groups
${LangFileString} IDS_GRP_MMEDIA		"Multimedia-engine"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Output"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Audio afspelen"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Audio-encoders"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Video afspelen"
${LangFileString} IDS_GRP_VISUALIZATION		"Visualisatie"
${LangFileString} IDS_GRP_UIEXTENSION		"Uitbreidingen voor de gebruikersinterface"
${LangFileString} IDS_GRP_WALIB			"Winamp Bibliotheek"
${LangFileString} IDS_GRP_WALIB_CORE		"Kerncomponenten voor mediabibliotheek"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Ondersteuning voor draagbare mediaspelers"
${LangFileString} IDS_GRP_LANGUAGES 	    	"Talen"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"WaveOut/MME-output"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC-codering"
${LangFileString} IDS_SEC_MILKDROP2             "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG		"Auto-Tagger"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE				"Configureert online services..."
${LangFileString} IDS_RUN_CHECK_PROCESS				"Controleert of er een andere sessie van Winamp actief is..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED			"Opent Internetverbinding..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Controleert of er een Internetverbinding is..."
${LangFileString} IDS_RUN_NOINET				"Geen Internetverbinding"
${LangFileString} IDS_RUN_EXTRACT				"Uitpakken"
${LangFileString} IDS_RUN_DOWNLOAD				"Downloaden"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS			"Downloaden voltooid."
${LangFileString} IDS_RUN_DOWNLOADFAILED			"Downloaden mislukt. Reden:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED			"Downloaden geannuleerd."
${LangFileString} IDS_RUN_INSTALL				"Installeert"
${LangFileString} IDS_RUN_INSTALLFIALED				"Installatie mislukt."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Bestand niet gevonden. Download wordt ingepland."
${LangFileString} IDS_RUN_DONE					"Gereed."

${LangFileString} IDS_DSP_PRESETS 	"SPS voorinstellingen"
${LangFileString} IDS_DEFAULT_SKIN	"standaard skins"
${LangFileString} IDS_AVS_PRESETS	"AVS voorinstellingen"
${LangFileString} IDS_MILK_PRESETS	"MilkDrop voorinstellingen"
${LangFileString} IDS_MILK2_PRESETS	"MilkDrop2 voorinstellingen"

${LangFileString} IDS_CLEANUP_PLUGINS	"Plugins opschonen..."
${LangFileString} IDS_REMOVE_SKINS	"Standard skins verwijderen skins..."

; download
${LangFileString} IDS_DOWNLOADING	"%s Downloaden"
${LangFileString} IDS_CONNECTING	"Verbinden..."
${LangFileString} IDS_SECOND		" (1 seconde resterend)"
${LangFileString} IDS_MINUTE		" (1 minuut resterend)"
${LangFileString} IDS_HOUR		" (1 uur resterend)"
${LangFileString} IDS_SECONDS		" (%u seconden resterend)"
${LangFileString} IDS_MINUTES		" (%u minuten resterend)"
${LangFileString} IDS_HOURS		" (%u uur resterend)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%) van %skB @ %u.%01ukB/s"

; AutoplayHandler
${LangFileString} AutoplayHandler	"Spelen"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Installatie Voltooid"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) is op uw computer geïnstalleerd.$\r$\n$\r$\n\
							Klik op Voltooien om deze wizard te sluiten."
${LangFileString} IDS_PAGE_FINISH_RUN		"$(^NameDA) opstarten nadat de installatie is afgesloten"
${LangFileString} IDS_PAGE_FINISH_LINK		"Klik hier om Winamp.com te bezoeken"

; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"Welkom bij het installatieprogramma van $(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"Bereid u voor op de ultieme mediamanager, en nog veel meer.$\r$\n$\r$\n\
							Geniet van uw favoriete muziek, video's en podcasts. Rip en brand cd's, maak speellijsten, \
                          				Synchroniseer uw draagbare speler met uw pc en deel muziek met vrienden. Ontdek nieuwe muziek via duizenden radiozenders, \
                          				video's, artiestrecenties en meer. $\r$\n  \
							•  Geheel vernieuwde Bento-skin$\r$\n  \
							•  Ondersteuning voor allerlei muziekspelers, waaronder de iPod®$\r$\n  \
							•  Ondersteuning voor albumhoezen die Winamp heeft gevonden!$\r$\n  \
							•  Media Monitor waarmee u van de beste muziek op het web geniet$\r$\n  \
							•  Dynamische songaanbevelingen$\r$\n  \
							•  Ondersteuning voor mp3's met surround sound$\r$\n  \
							•  Externe toegang tot muziek en video$\r$\n  \
							•  Autocoderingsfunctie voor categorisering van uw muziek"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"N.B.: om van de nieuwe functies en het \
								ontwerp van de (aanbevolen) Bento-skin te genieten, \
								moeten alle componenten worden geselecteerd."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE		"Startopties kiezen"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Maak een keuze uit de volgende startopties."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Configureer de startopties van Winamp met de volgende opties."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Optie toevoegen aan menu Start"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Icoon toevoegen aan werkbalk Snel starten"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Icoon aan bureaublad toevoegen"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Kan Winamp Agent niet sluiten.$\r$\n\
                                                   Zorg dat er geen andere gebruikers bij Windows zijn aangemeld.\
                                                   $\r$\n$\r$\n	Selecteer Opnieuw nadat u WinampAgent hebt afgesloten.\
                                                   $\r$\n$\r$\n	Selecteer Negeren als u wilt proberen om de installatie toch uit te voeren.\
                                                   $\r$\n$\r$\n	Selecteer Afbreken als u de installatie wilt annuleren."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Deze versie van Windows wordt niet ondersteund.$\r$\n\
                                                 $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} vereist minimaal Windows 2000 of nieuwer."


; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE	"Niet ondersteunde versie van gen_lyrics.dll plugin gevonden!!$\r$\n$\r$\nDeze plugin maakt Winamp 5.59 en nieuwe instabiel.$\r$\nDe plugin wordt uitgeschakeld. Klik op OK om door te gaan."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "gen_lyrics plugin van derden gevonden!$\r$\n$\r$\nOude versies van deze plugin zijn niet compatibel met Winamp 5.6 en nieuwe. Zorg er voor dat u de laatste versie hebt van http://lyricsplugin.com alvorens u doorgaat."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER		"${DIRECTXINSTAL_WINVER_LO} of lager is gedetecteerd"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER 	"${DIRECTXINSTAL_WINVER_HI} of hoger is gedetecteerd"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"${DIRECTXINSTAL_DIRECTXNAME} versie controleren"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 		"Vereiste minimum ${DIRECTXINSTAL_DIRECTXNAME} versie"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Kan ${DIRECTXINSTAL_DIRECTXNAME} versie niet vinden"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"${DIRECTXINSTAL_DIRECTXNAME} versie is gedetecteerd"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER		"Niet-ondersteunde ${DIRECTXINSTAL_DIRECTXNAME} versie"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT		"Controleren of $0 aanwezig is"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Download vereist"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Internet verbinding controleren"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"Laatste versie van ${DIRECTXINSTAL_DIRECTXNAME} is beschikbaar op:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"${DIRECTXINSTAL_DIRECTXNAME} installatie downloaden"
${LangFileString} IDS_DIRECTX_FOUND				"Gevonden"
${LangFileString} IDS_DIRECTX_MISSING				"Ontbreekt"
${LangFileString} IDS_DIRECTX_SUCCESS				"Succes"
${LangFileString} IDS_DIRECTX_ABORTED				"Afgebroken"
${LangFileString} IDS_DIRECTX_FAILED				"Mislukt"
${LangFileString} IDS_DIRECTX_DONE				"Klaar"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP			"Start ${DIRECTXINSTAL_DIRECTXNAME} installatie"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"${DIRECTXINSTAL_WINAMPNAME} vereist minimaal ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} om te kunnen draaien.$\r$\nWilt u het nu installeren?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"${DIRECTXINSTAL_WINAMPNAME} veresit minumaal ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} om te kunnen draaien"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Kan ${DIRECTXINSTAL_DIRECTXNAME} niet downloaden"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Kan ${DIRECTXINSTAL_DIRECTXNAME} niet installeren"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"Uw computer mist een ${DIRECTXINSTAL_DIRECTXNAME} component vereist voor ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Kan ontbrekend ${DIRECTXINSTAL_DIRECTXNAME} component niet downloaden"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Kan ontbrekend ${DIRECTXINSTAL_DIRECTXNAME} component niet installeren"

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Kerncomponenten van Winamp (vereist)"
${LangFileString} IDS_SEC_AGENT_DESC			"Winamp Agent, voor handig systeemvakicoon en beheer van bestandskoppelingen"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Multimedia-engine (invoer/uitvoersysteem)"
${LangFileString} IDS_SEC_CDDB_DESC			"CDDB-ondersteuning, voor automatisch ophalen van cd-titels uit online Gracenote-database"
${LangFileString} IDS_SEC_DSP_DESC			"DSP-plug-in, voor extra effecten zoals chorus-, conversie-, tempo- en toonhoogteregeling"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC		"Ondersteuning voor afspelen van audio (invoerplug-ins: audiodecoders)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Ondersteuning voor afspelen van MP3-, MP2-, MP1- en AAC-formaten (vereist)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Ondersteuning voor afspelen van WMA-formaten (biedt DRM-ondersteuning)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Ondersteuning voor afspelen van MIDI-formaten (MID, RMI, KAR, MUS, CMF en andere)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Ondersteuning voor afspelen van moduleformaten (MOD, XM, IT, S3M, ULT en meer)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Ondersteuning voor afspelen van Ogg Vorbis-formaat (OGG)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Ondersteuning voor afspelen van MPEG-4-audioformaten (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Ondersteuning voor afspelen van FLAC-formaat"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Ondersteuning voor afspelen van audio-cd's"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Ondersteuning voor afspelen van Waveform-formaten (WAV, VOC, AU, AIFF en meer)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC		"Ondersteuning voor afspelen van video (invoerplug-ins: videodecoders)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Ondersteuning voor afspelen van Windows Media-videoformaten (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Ondersteuning voor afspelen van Nullsoft Video-formaat (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Ondersteuning voor afspelen van MPEG-1/2 en andere videoformaten"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Ondersteuning voor afspelen van AVI Video"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Ondersteuning voor afspelen van Flash Video (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Ondersteuning voor afspelen van Matroska Video (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Ondersteuning voor afspelen van MPEG-4 Video (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Ondersteuning voor afspelen van Adobe Flash streaming format (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC		"Ondersteuning voor coderen en transcoderen (vereist voor cd rippen en conversies tussen bestandsformaten)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Ondersteuning voor rippen en transcoderen naar WMA-formaat"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Ondersteuning voor rippen en transcoderen naar WAV-formaat"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Ondersteuning voor rippen en transcoderen naar M4A- en AAC-formaat"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Ondersteuning voor rippen en transcoderen naar FLAC-formaat"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Ondersteuning voor rippen en transcoderen naar Ogg Vorbis-formaat"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC		"Uitvoerplug-ins (bepalen hoe audio wordt verwerkt en naar uw geluidskaart wordt gezonden)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Oude versie van WAV/MME-writer (standaard niet meer geïnstalleerd, maar door sommigen geprefereerd boven de coderingsplug-ins)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"DirectSound-uitvoer (vereist / standaarduitvoerplug-in)"	
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Oude versie van WaveOut-uitvoer (optioneel en niet langer aanbevolen of vereist)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Uitbreidingen voor de gebruikersinterface"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Plug-in voor globale sneltoetsen, voor toetsenbordbediening van Winamp als andere toepassingen de actieve toepassing zijn"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Uitgebreide Spring naar bestand-ondersteuning, voor het in de wachtrij plaatsen van songs uit speellijsten, en meer"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Nullsoft besturingselement in systeemvak, voor het toevoegen van afspeelknoppen aan het systeemvak"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Ondersteuning voor moderne skins, vereist voor skins met een vrije vorm, zoals Winamp Modern en Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC		"Visualisatie plug-ins"
${LangFileString} IDS_SEC_NSFS_DESC			"Plug-in Nullsoft Klein volledig scherm"
${LangFileString} IDS_SEC_AVS_DESC			"Plug-in Geadvanceerde Visualisatie Studio"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Milkdrop visualisatie plug-in"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Milkdrop2 visualisatie plug-in (standaard visualisatie plug-in)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Ondersteuning voor line input via opdracht linein:// (past visualisatie toe op mic/lijninvoer)"
${LangFileString} IDS_GRP_WALIB_DESC			"Winamp Bibliotheek"
${LangFileString} IDS_SEC_ML_DESC			"Winamp Mediabibliotheek (vereist)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Hulpmiddel voor transcodering, voor conversies tussen bestandsformaten"
${LangFileString} IDS_SEC_ML_RG_DESC			"Hulpmiddel Gelijkmatig Afspeelvolume, speelt nummers even hard af."
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Winamp Auto-Tagger (aangeboden door Gracenote), voor het invullen van ontbrekende metagegevens"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Podcast Directory, voor het abonneren op en downloaden van podcasts" 
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Online Services, waaronder streams van SHOUTcast Radio && TV, In2TV, AOL Video en XM Radio"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Winamp Speellijstgenerator (aangeboden door Gracenote), voor het opstellen van akoestisch dynamische speellijsten"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Kerncomponenten voor mediabibliotheek"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Database met lokale media, met krachtige queryfunctie en aangepaste weergaven"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Afspeellijstbeheer, voor het maken, bewerken en opslaan van al uw speellijsten"
${LangFileString} IDS_SEC_ML_DISC_DESC			"Cd rippen && branden, de interface voor het rippen && branden van audio-cd's vanuit de mediabibliotheek"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Favorietenbeheer, voor het toevoegen van uw favoriete streams, bestanden of mappen aan uw favorieten"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Geschiedenis, voor directe toegang tot alle recent afgespeelde lokale of externe bestanden en streams"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC		"Nu Speelt, voor het weergeven van informatie over de track die momenteel wordt afgespeeld"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC		"Ondersteuning voor draagbare mediaspelers" 
${LangFileString} IDS_SEC_ML_PMP_DESC			"Plug-in Ondersteuning voor draagbare mediaspelers (vereist)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"iPod® Ondersteuning"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Ondersteuning voor draagbare Creative®-spelers (voor beheer Nomad™-.  Zen™- en MuVo™-spelers)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Ondersteuning voor Microsoft® PlaysForSure® (voor beheer alle P4S-compatibele spelers)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"Ondersteuning voor USB-apparaten (voor beheer van generieke USB-sticks en -spelers)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC		"Ondersteuning voor Microsoft® ActiveSync® (voor beheer van Windows Mobile®-, Smartphone- && Pocket PC-apparaten)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Ondersteuning voor Android apparaten"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Android Wifi ondersteuning"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"iTunes-compatible Media Bibliotheek database import/export plugin"

;========================================================================================
; uninstall section

${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC		"Verwijder $(^NameDA) van uw systeem."

${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Deinstallatie pad:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Media Player"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Verwijder alle $(^NameDA) Media Player onderdelen inclusief bijbehorende plugins van derden."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Voorkeuren van de gebruiker"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 		"Verwijder alle $(^NameDA) voorkeuren en plugins."

${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT		"Ondersteun $(^NameDA) door feedback te sturen"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT		"Open $(^NameDA) map"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nNB: Niet alle bestanden zijn verwijderd. Om deze te bekijken opent u de Winamp-map."