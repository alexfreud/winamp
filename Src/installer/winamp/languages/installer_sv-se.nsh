; Languge-Culture:	SV-SE
; LangId:			1053
; CodePage:			1252
; Revision:			1.1
; Last udpdated #1:	03.10.2007 (by official translator)
; Last udpdated #2:	13.12.2009 (by DJ Egg - added missing items, and attempted to (badly) translate some)
; Author:
; Email:

!insertmacro LANGFILE_EXT "Swedish"
 
${LangFileString} installFull "Fullständig"
${LangFileString} installStandard "Standard"
${LangFileString} installLite "Begränsad"
${LangFileString} installMinimal "Minimal"
${LangFileString} installPrevious "Tidigare installation"

; BrandingText
${LangFileString} BuiltOn "built on"
${LangFileString} at "at"

${LangFileString} installWinampTop "Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType} kommer att installeras.  " 
${LangFileString} installerContainsFull "Installationsprogrammet innehåller den fullständiga installationen."
${LangFileString} installerContainsLite "Installationsprogrammet innehåller den begränsade installationen."
${LangFileString} licenseTop "Läs igenom och godkänn installationsvillkoren nedan innan du installerar."
${LangFileString} directoryTop "Installationsprogrammet har valt den bästa platsen för $(^NameDA). Om du vill byta mapp ska du göra det nu."

${LangFileString} uninstallPrompt "Winamp kommer att avinstalleras. Vill du fortsätta?"

${LangFileString} msgCancelInstall "Avbryt installationen?"
${LangFileString} msgReboot "Du måste starta om datorn för att slutföra installationen.$\r$\nVill du starta om nu? (Välj Nej om du vill starta om senare)"
${LangFileString} msgCloseWinamp "Du måste stänga Winamp innan du kan fortsätta.$\r $\n $\r $\n \tab När du har stängt Winamp väljer du Försök igen.$\r $\n $\r $\n \tab Om du vill försöka installera ändå, väljer du Ignorera.$\r $\n $\r $\n \tab Välj Avbryt om du vill avbryta installationen."
${LangFileString} msgInstallAborted "Installationen avbröts av användaren."

${LangFileString} secWinamp "Winamp (krävs)"
${LangFileString} compAgent "Winamp-agent"
${LangFileString} compModernSkin "Stöd för moderna skal"
${LangFileString} uninstallWinamp "Avinstallera Winamp"

${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Hämta och installera Windows Media Format"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "OGG Vorbis-uppspelning"
${LangFileString} secOGGEnc "OGG Vorbis-kodning"
${LangFileString} secAACE "HE-AAC-kodning"
${LangFileString} secMP3E "MP3-kodning"
${LangFileString} secMP4E "MP4-stöd"
${LangFileString} secWMAE "WMA-kodning"
${LangFileString} msgWMAError "Det inträffade ett fel när komponenter installerades. WMA Encoder installeras inte. Gå till http://www.microsoft.com/windows/windowsmedia/9series/encoder/, hämta kodaren och försök igen."
${LangFileString} secCDDA "Uppspelning och extrahering av CD-skivor" 
${LangFileString} msgCDError "Det inträffade ett fel när komponenter installerades. CD Rip  Burn kanske inte fungerar korrekt."
${LangFileString} secCDDB "CDDB för identifiering av CD-skivor"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"

${LangFileString} secDSP "Plugin-programmet Signal Processor Studio"
${LangFileString} secWriteWAV "Klassisk WAV Writer"
${LangFileString} secLineInput "Stöd för linjeingång"
${LangFileString} secDirectSound "Stöd för DirectSound-utdata"

${LangFileString} secHotKey "Globalt stöd för snabbtangenter"
${LangFileString} secJmp "Utökat stöd för Hoppa till fil"
${LangFileString} secTray "Nullsoft Tray Control"

${LangFileString} msgRemoveMJUICE "Vill du ta bort stöd för MJuice från systemet?$\r $\n $\r $\nV älj JA om du inte använder MJF-filer i andra program än Winamp."
${LangFileString} msgNotAllFiles "Alla filer togs inte bort.$\r $\nDu kan ta bort dem själv om du vill."


${LangFileString} secNSV "Nullsoft Video (NSV)"
${LangFileString} secDSHOW "DirectShow Formats (MPG, M2V)"
${LangFileString} secAVI "AVI Video"
${LangFileString} secFLV "Flash Video (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "MPEG-4 Video (MP4, M4V)"

${LangFileString} secSWF "Flash Media Protocol (SWF, RTMP)"

${LangFileString} secTiny "Nullsoft Tiny Fullscreen"
${LangFileString} secAVS "Advanced Visualization Studio"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Winamp-mediebiblioteket"
${LangFileString} secOM "Media online"
${LangFileString} secWire "Podcast-katalog"
${LangFileString} secPmp "Bärbara mediespelare"
${LangFileString} secPmpIpod "Stöd för iPod®"
${LangFileString} secPmpCreative "Stöd för Creative®-spelare"
${LangFileString} secPmpP4S "Stöd för Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "Stöd för USB-enheter"
${LangFileString} secPmpActiveSync "Stöd för Microsoft® ActiveSync®"

${LangFileString} sec_ML_LOCAL "Lokala media"
${LangFileString} sec_ML_PLAYLISTS "Spellistor"
${LangFileString} sec_ML_DISC "CD Rip & Burn"
${LangFileString} sec_ML_BOOKMARKS "Bokmärken"
${LangFileString} sec_ML_HISTORY "Historik"
${LangFileString} sec_ML_NOWPLAYING "Spelas nu"
${LangFileString} sec_ML_RG "Analysverktyg för Replay Gain"
${LangFileString} sec_ML_TRANSCODE "Omkodningsverktyg"
${LangFileString} sec_ML_PLG "Playlist Generator"
${LangFileString} sec_ML_IMPEX "Database Import/Export Tool"

;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "Installationsprogram för $(^NameDA)"
${LangFileString} IDS_SELECT_LANGUAGE  "Välj språk för installationsprogrammet"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Multimediemotor"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Utdata"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Ljuduppspelning"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Ljudkodare"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Videouppspelning"
${LangFileString} IDS_GRP_VISUALIZATION		"Visualisering"
${LangFileString} IDS_GRP_UIEXTENSION		"Tillägg för användargränssnitt"
${LangFileString} IDS_GRP_WALIB				"Winamp-biblioteket"
${LangFileString} IDS_GRP_WALIB_CORE		"Kärnkomponenter i mediebiblioteket"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Stöd för bärbara mediespelare"
${LangFileString} IDS_GRP_LANGUAGES 	    "Språk"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"WaveOut/MME-utdata"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC-kodning"

${LangFileString} IDS_SEC_ML_AUTOTAG		"Automatisk taggning"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Konfigurerar onlinetjänster..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Kontrollerar om en annan instans av Winamp är igång..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Öppnar internetanslutning..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Kontrollerar om internet är tillgängligt..."
${LangFileString} IDS_RUN_NOINET				"Ingen internetanslutning"
${LangFileString} IDS_RUN_EXTRACT				"Extraherar"
${LangFileString} IDS_RUN_DOWNLOAD				"Hämtar"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Hämtningen är klar."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Hämtningen misslyckades. Orsak:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Hämtningen avbröts."
${LangFileString} IDS_RUN_INSTALL				"Installerar"
${LangFileString} IDS_RUN_INSTALLFIALED			"Installationen misslyckades."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Det gick inte att hitta filen. Planerar hämtning."
${LangFileString} IDS_RUN_DONE					"Klar."

${LangFileString} IDS_DSP_PRESETS 	"Förval för SPS"
${LangFileString} IDS_DEFAULT_SKIN	"standardskal"
${LangFileString} IDS_AVS_PRESETS	"Förval för AVS"
${LangFileString} IDS_MILK_PRESETS	"Förval för MilkDrop"
${LangFileString} IDS_MILK2_PRESETS	"Förval för MilkDrop2"

; download
${LangFileString} IDS_DOWNLOADING	"Hämtar %s"
${LangFileString} IDS_CONNECTING	"Ansluter ..."
${LangFileString} IDS_SECOND		" (1 sekund återstår)"
${LangFileString} IDS_MINUTE		" (1 minut återstår)"
${LangFileString} IDS_HOUR			" (1 timme återstår)"
${LangFileString} IDS_SECONDS		" (%u sekunder återstår)"
${LangFileString} IDS_MINUTES		" (%u minuter återstår)"
${LangFileString} IDS_HOURS			" (%u timmar återstår)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%) av %skB @ %u.%01ukB/s"

; AutoplayHandler
${LangFileString} AutoplayHandler	"Play"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Installationen är klar"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) har installerats på datorn.$\r$\n$\r$\n\
													Stäng guiden genom att klicka på Slutför."
${LangFileString} IDS_PAGE_FINISH_RUN		"Starta $(^NameDA) när installationsprogrammet stängs"
${LangFileString} IDS_PAGE_FINISH_LINK		"Klicka här för att gå till Winamp.com"

; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE		"Välkommen till installationsprogrammet för $(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"Nu kan du uppleva den optimala mediehanteraren och mycket mer.$\r$\n$\r$\n\
													Spela upp dina favoritlåtar eller filmer eller lyssna på poddradio. Rippa och bränn CD-skivor, skapa spellistor, \
                          synkronisera med en bärbar musikspelare och dela filer med dina vänner. Upptäck nya radiostationer med ny musik, \
                          videofilmer, recensioner och mycket mer. $\r$\n  \
													•  Helt ny design i Bento-skalet$\r$\n  \
													•  Stöd för många typer av musikenheter, t.ex. iPod®$\r$\n  \
													•  Stöd för skivomslag - Winamp hittar det åt dig!$\r$\n  \
													•  Håll koll på den bästa musiken på webben med medieövervakaren$\r$\n  \
													•  Dynamiska låtrekommendationer$\r$\n  \
													•  Stöd för MP3-surroundljud$\r$\n  \
													•  Åtkomst till musik och videor via fjärranslutning, fildelning$\r$\n  \
													•  Automatisk taggning för kategorisering av musik"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"Obs! Om du vill utnyttja alla nya funktioner och\
															Bento-skalets design (rekommenderas), \
															måste du markera alla komponenter."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Välj startalternativ"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Välj bland följande startalternativ."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Välj bland följande alternativ för att konfigurera startalternativen för Winamp."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Skapa post på Startmenyn"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Skapa snabbstartsikon"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Skapa skrivbordsikon"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Det går inte att stänga Winamp-agenten.$\r$\n\
                                                   Kontrollera att inte någon annan är inloggad i Windows.\
                                                   $\r$\n$\r$\n	Välj Försök igen när du har stängt Winamp-agenten.\
                                                   $\r$\n$\r$\n	Välj Ignorera om du vill försöka ändå.\
                                                   $\r$\n$\r$\n	Välj Avbryt om du vill avbryta installationen."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"Detected ${DIRECTXINSTAL_WINVER_LO} or lower"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "Detected ${DIRECTXINSTAL_WINVER_HI} or higher"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Checking ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Required minimal ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Unable to detect ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"Detected ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Unsupported ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Checking if $0 present"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Download required"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Checking internet connection"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"Latest version of ${DIRECTXINSTAL_DIRECTXNAME} available at:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Downloading ${DIRECTXINSTAL_DIRECTXNAME} installer"
${LangFileString} IDS_DIRECTX_FOUND						"Found"
${LangFileString} IDS_DIRECTX_MISSING					"Missing"
${LangFileString} IDS_DIRECTX_SUCCESS					"Success"
${LangFileString} IDS_DIRECTX_ABORTED					"Aborted"
${LangFileString} IDS_DIRECTX_FAILED					"Failed"
${LangFileString} IDS_DIRECTX_DONE						"Done"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Running ${DIRECTXINSTAL_DIRECTXNAME} installer"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"${DIRECTXINSTAL_WINAMPNAME} requires at least ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} to operate properly.$\r$\nInstall it right now?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"${DIRECTXINSTAL_WINAMPNAME} requires at least ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} to operate properly"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Unable to download ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Unable to install ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"Your computer is missing a ${DIRECTXINSTAL_DIRECTXNAME} component required by ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Unable to download missing ${DIRECTXINSTAL_DIRECTXNAME} component"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Unable to install missing ${DIRECTXINSTAL_DIRECTXNAME} component"

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Winamp-kärnan (nödvändigt)"
${LangFileString} IDS_SEC_AGENT_DESC			"Winamp-agenten förenklar tillgången till systemfältet och underhåller filtypsassociationer"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Multimediemotor (indata-/utdatasystem)"
${LangFileString} IDS_SEC_CDDB_DESC				"CDDB-stöd för automatisk hämtning av CD-titlar från Gracenotes onlinedatabas"
${LangFileString} IDS_SEC_DSP_DESC				"DSP-plugin-program för extra effekter som kör, tempo och pitch-kontroll"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Stöd för uppspelning av ljud (plugin-program för indata: Audio Decoders)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Stöd för uppspelning av formaten MP3, MP2, MP1 och AAC (nödvändigt)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Stöd för uppspelning av WMA-format (inklusive DRM-stöd)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Stöd för uppspelning av MIDI-format (MID, RMI, KAR, MUS, CMF med flera)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Stöd för uppspelning av Module-format (MOD, XM, IT, S3M, ULT med flera)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Stöd för uppspelning av Ogg Vorbis-format (OGG)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Stöd för uppspelning av MPEG-4-ljudformat (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Stöd för uppspelning av FLAC-format"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Stöd för uppspelning av ljud-CD-skivor"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Stöd för uppspelning av Waveform-format (WAV, VOC, AU, AIFF med flera)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Stöd för videouppspelning (plugin-program för indata: Video Decoders)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Stöd för uppspelning av Windows Media-videoformat (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Stöd för uppspelning av Nullsoft Video-format (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Stöd för uppspelning av MPEG-videoformat"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Stöd för uppspelning av AVI Video"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Stöd för uppspelning av Flash Video (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Stöd för uppspelning av Matroska Video (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Stöd för uppspelning av MPEG-4 Video (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Stöd för uppspelning av Adobe Flash-format (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Stöd för kodning och omkodning (nödvändigt för CD-rippning och konvertering mellan olika filformat)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Stöd för rippning och omkodning till WMA-format"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Stöd för rippning och omkodning till WAV-format"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Stöd för rippning och omkodning till M4A- och AAC-format"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Stöd för rippning och omkodning till FLAC-format"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Stöd för rippning och omkodning till Ogg Vorbis-format"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Plugin-program för utdata (styr hur ljudet behandlas och skickas till ljudkortet)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"WAV/MME Writer med klassiska funktioner (rekommenderas inte, men en del användare föredrar fortfarande detta framför kodningsplugin-program)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"DirectSound Output (nödvändigt/standard plugin-program för utdata)"	
${LangFileString} IDS_SEC_OUT_WAV_DESC			"WaveOut Output med klassiska funktioner (valfritt, varken nödvändigt eller rekommenderat)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Tillägg för användargränssnitt"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Plugin-programmet Globala snabbtangenter för styrning av Winamp från tangentbordet när andra program är aktiva"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Utökat stöd för att hoppa till fil, köa låtar i spellistan och mycket, mycket mer"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Plugin-programmet Nullsoft Tray Control lägger till ikoner i systemfältet för hantering av uppspelning"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Stöd för moderna skal, krävs för frihandsskal som Winamp Modern och Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Plugin-program för visualisering"
${LangFileString} IDS_SEC_NSFS_DESC				"Plugin-programmet Nullsoft Tiny Fullscreen för visualisering"
${LangFileString} IDS_SEC_AVS_DESC				"Plugin-programmet Advanced Visualization Studio (standardplugin för vis)"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Plugin-program för Milkdrop-visualisering"
${LangFileString} IDS_SEL_LINEIN_DESC			"Stöd för linjeingång med kommandot linein:// (tillämpar visualisering på mikrofon/linjeingång)"
${LangFileString} IDS_GRP_WALIB_DESC			"Winamp-biblioteket"
${LangFileString} IDS_SEC_ML_DESC				"Winamp-mediebibliotek (nödvändigt)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Omkodningsverktyg för konvertering mellan olika filformat"
${LangFileString} IDS_SEC_ML_RG_DESC			"Analysverktyget Replay Gain för volymanpassning"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Winamps funktion för automatisk taggning (utformad av Gracenote) kompletterar saknade metadata"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Katalog för poddsändningar, används för prenumeration och hämtning av poddsändningar" 
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Onlinetjänster som SHOUTcast Radio && TV, In2TV, AOL-videor och  XM Radio-dataströmmar"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Spellistegeneratorn (utformad av Gracenote) används för att skapa dynamiska spellistor med akustiska effekter"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Kärnkomponenter i mediebiblioteket"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Lokal mediedatabas med kraftfullt söksystem och anpassningsbara smarta vyer"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Spellistehanterareren där du skapar, redigerar och lagrar egna spellistor"
${LangFileString} IDS_SEC_ML_DISC_DESC			"Rippa och bränn CD, ett mediebibliotek där du kan rippa och bränna ljud-CD-skivor"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Hantering av bokmärken, där du kan märka upp dataströmmar, filer och mappar som favoriter"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Historik som ger snabb tillgång till alla lokala filer, fjärrfiler och strömmar som spelats nyligen"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Spelas nu, används för att visa information om det aktuella spåret"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Stöd för bärbara mediespelare" 
${LangFileString} IDS_SEC_ML_PMP_DESC			"Plugin-program för stöd för bärbara mediespelare (Core) (nödvändigt)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"Stöd för iPod®"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Stöd för bärbara enheter från Creative® (för hantering av spelare av typen Nomad™, Zen™ och MuVo™)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Stöd för Microsoft® PlaysForSure® (för hantering av alla P4S-kompatibla spelare)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"Stöd för USB-enheter (för generell hantering av USB-enheter och spelare)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Stöd för Microsoft® ActiveSync® (för hantering av Windows Mobile®-, Smartphone- och Pocket PC-enheter)"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"iTunes-compatible Media Library database import/export plugin"
