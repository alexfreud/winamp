; Language-Country:	RO-RO
; LangId:			1048
; CodePage:			1250 - UTF8
; Revision:			5.61.1
; Last udpdated:	12.03.2011 22:28
; Author:			Cătălin ZAMFIRESCU - x10
; Email:			x10@mail.com

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

!insertmacro LANGFILE_EXT "Romanian"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE	"Limbă instalare"
${LangFileString} LANGUAGE_DLL_INFO		"Selectați limba în care se va instala programul:"

${LangFileString} installFull		"Completă"
${LangFileString} installStandard	"Standard"
${LangFileString} installLite		"Redusă"
${LangFileString} installMinimal	"Minimală"
${LangFileString} installPrevious	"Configurația precedentă"

; BrandingText
${LangFileString} BuiltOn	"compilat în"
${LangFileString} at 		"la"

${LangFileString} installWinampTop 			"Se va instala Winamp, ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}."
${LangFileString} installerContainsFull		" Acest pachet de instalare conține versiunea completă."
${LangFileString} installerContainsLite		" Acest pachet de instalare conține versiunea redusă."
${LangFileString} licenseTop				"Vă rugăm, citiți și acceptați termenii licenței de mai jos înaintea începerii instalării."
${LangFileString} directoryTop				"Asistentul de instalare a determinat locația optimă pentru $(^NameDA). Dacă doriți să schimbați acest director, puteți să o faceți acum."

${LangFileString} uninstallPrompt	"Winamp se va dezinstala. Continuați?"

${LangFileString} msgCancelInstall	"Abandonați instalarea?"
${LangFileString} msgReboot 		"Este necesară o repornire a calculatorului pentru finalizarea instalării.$\r$\nÎl reporniți acum? (Dacă doriți să-l reporniți mai târziu, selectați Nu)"
${LangFileString} msgCloseWinamp	"Trebuie să închideți Winamp înainte de a continua.$\r$\n$\r$\n	După ce ați închis Winamp, selectați Reîncercare.$\r$\n$\r$\n	Dacă totuși doriți să continuați instalarea oricum, selectați Ignorare.$\r$\n$\r$\n	Dacă doriți să abandonați instalarea, selectați Abandonare."
${LangFileString} msgInstallAborted	"Instalare întreruptă de utilizator"

${LangFileString} secWinamp			"Winamp (necesar)"
${LangFileString} compAgent			"Agent Winamp"
${LangFileString} compModernSkin	"Fațete moderne"
${LangFileString} uninstallWinamp	"Dezinstalare Winamp"

${LangFileString} secWMA			"WMA (Windows Media Audio)"
${LangFileString} secWMV			"WMV/ASF (Windows Media Video)"
${LangFileString} secWMFDist		"Descărcare și instalare Windows Media Format"

${LangFileString} secMIDI			"MIDI"
${LangFileString} secMOD			"MOD/XM/S3M/IT"
${LangFileString} secOGGPlay		"OGG Vorbis"
${LangFileString} secOGGEnc			"OGG Vorbis"
${LangFileString} secAACE			"HE-AAC"
${LangFileString} secMP3E			"MP3"
${LangFileString} secMP4E			"MP4"
${LangFileString} secWMAE			"WMA"
${LangFileString} msgWMAError		"A survenit o problemă în timpul instalării componentelor. Codorul WMA nu va fi instalat. Vizitați http://www.microsoft.com/windows/windowsmedia/9series/encoder/, descărcați codorul și încercați din nou."
${LangFileString} secCDDA			"CDA (CD audio)"
${LangFileString} msgCDError		"A survenit o problemă în timpul instalării componentelor. E posibil ca extragerea/inscripționarea CD-urilor să nu funcționeze corespunzător."
${LangFileString} secCDDB			"Recunoaștere CD-uri"
${LangFileString} secWAV			"WAV/VOC/AU/AIFF"

${LangFileString} secDSP			"Studio de procesare a sunetului"
${LangFileString} secWriteWAV		"Convertor WAV clasic"
${LangFileString} secLineInput		"Suport intrare LineIn"
${LangFileString} secDirectSound	"DirectSound"

${LangFileString} secHotKey			"Comenzi rapide globale"
${LangFileString} secJmp			"Direct la piesă"
${LangFileString} secTray			"Control din zona de notificare"

${LangFileString} msgRemoveMJUICE	"Eliminați suportul MJuice din sistemul Dvs.?$\r$\n$\r$\nSelectați Da numai dacă nu folosiți fișiere MJF și în alte programe decât Winamp."
${LangFileString} msgNotAllFiles	"Nu s-au putut șterge toate fișierele.$\r$\nDacă doriți, le puteți șterge Dvs."


${LangFileString} secNSV			"Nullsoft Video (NSV)"
${LangFileString} secDSHOW			"DirectShow (MPG, M2V)"
${LangFileString} secAVI			"AVI Video (AVI)"
${LangFileString} secFLV			"Flash Video (FLV)"

${LangFileString} secMKV			"Matroska (MKV, MKA)"
${LangFileString} secM4V			"MPEG-4 Video (MP4, M4V)"

${LangFileString} secSWF			"Protocol Flash Media (SWF/RTMP)"

${LangFileString} secTiny			"Mini-ecran complet Nullsoft"
${LangFileString} secAVS			"Studio pentru efecte grafice avansate"
${LangFileString} secMilkDrop		"Milkdrop"

${LangFileString} secML				"Mediatecă Winamp"
${LangFileString} secOM				"Mediatecă internet"
${LangFileString} secWire			"Podcast"
${LangFileString} secPmp			"Gestionar unități portabile"
${LangFileString} secPmpIpod		"iPod®"
${LangFileString} secPmpCreative	"Creative®"
${LangFileString} secPmpP4S			"Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB			"Dispozitive USB"
${LangFileString} secPmpActiveSync	"Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid		"Android"
${LangFileString} secPmpWifi		"Android Wifi"

${LangFileString} sec_ML_LOCAL		"Mediatecă locală"
${LangFileString} sec_ML_PLAYLISTS	"Liste de redare"
${LangFileString} sec_ML_DISC		"Extragere și inscripționare CD"
${LangFileString} sec_ML_BOOKMARKS	"Semne de carte"
${LangFileString} sec_ML_HISTORY	"Istoric"
${LangFileString} sec_ML_NOWPLAYING	"În curs de redare"
${LangFileString} sec_ML_RG			"Instrument de analiză Replay Gain"
${LangFileString} sec_ML_TRANSCODE	"Instrument de transcodare"
${LangFileString} sec_ML_PLG		"Generator de liste"
${LangFileString} sec_ML_IMPEX		"Instrument de import/export bază de date"


;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION				"Instalare $(^NameDA)"
${LangFileString} IDS_SELECT_LANGUAGE		"Selectați o limbă ce va fi utilizată de asistentul de instalare"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Nucleu de procesare multimedia"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Ieșire"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Redare audio"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Codare audio"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Redare video"
${LangFileString} IDS_GRP_VISUALIZATION		"Efecte grafice"
${LangFileString} IDS_GRP_UIEXTENSION		"Extensii interfață utilizator"
${LangFileString} IDS_GRP_WALIB				"Mediatecă Winamp"
${LangFileString} IDS_GRP_WALIB_CORE		"Componente principale"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Unități portabile"
${LangFileString} IDS_GRP_LANGUAGES 		"Pachete lingvistice"

; Sections
${LangFileString} IDS_SEC_OUT_WAV			"WaveOut/MME"
${LangFileString} IDS_SEC_WAV_ENC			"WAV"
${LangFileString} IDS_SEC_MP3_DEC			"MP3"
${LangFileString} IDS_SEC_FLAC_DEC			"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC			"FLAC"
${LangFileString} IDS_SEC_MILKDROP2			"Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG		"Etichetare automată"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE				"Configurare servicii internet..."
${LangFileString} IDS_RUN_CHECK_PROCESS				"Verificare prezență a altei instanțe în curs de execuție..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED			"Deschidere conexiune internet..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE		"Verificare disponibilitate internet..."
${LangFileString} IDS_RUN_NOINET					"Nu există nici o conexiune internet"
${LangFileString} IDS_RUN_EXTRACT					"Extragere"
${LangFileString} IDS_RUN_DOWNLOAD					"Descărcare"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS			"Descărcare completă."
${LangFileString} IDS_RUN_DOWNLOADFAILED			"Descărcare eșuată. Motiv:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED			"Descărcare abandonată."
${LangFileString} IDS_RUN_INSTALL					"Instalare"
${LangFileString} IDS_RUN_INSTALLFIALED				"Instalare eșuată."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Fișierul nu a fost găsit. Descărcarea a fost replanificată."
${LangFileString} IDS_RUN_DONE						"Finalizat."

${LangFileString} IDS_DSP_PRESETS 	"configurații SPS"
${LangFileString} IDS_DEFAULT_SKIN	"fațete predefinite"
${LangFileString} IDS_AVS_PRESETS	"configurații AVS"
${LangFileString} IDS_MILK_PRESETS	"configurații MilkDrop"
${LangFileString} IDS_MILK2_PRESETS	"configurații MilkDrop2"

${LangFileString} IDS_CLEANUP_PLUGINS	"Curățare extensii..."
${LangFileString} IDS_REMOVE_SKINS		"Eliminare fațete predefinite..."


; download
${LangFileString} IDS_DOWNLOADING	"Descărcare"
${LangFileString} IDS_CONNECTING	"Conectare..."
${LangFileString} IDS_SECOND		" (1 secundă rămasă)"
${LangFileString} IDS_MINUTE		" (1 minut rămas)"
${LangFileString} IDS_HOUR			" (1 oră rămasă)"
${LangFileString} IDS_SECONDS		" (%u secunde rămase)"
${LangFileString} IDS_MINUTES		" (%u minute rămase)"
${LangFileString} IDS_HOURS			" (%u ore rămase)"
${LangFileString} IDS_PROGRESS		"%skO (%d%%) din %skO @ %u,%01ukO/s"


; AutoplayHandler
${LangFileString} AutoplayHandler	"Redare"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE				"Instalare completă"
${LangFileString} IDS_PAGE_FINISH_TEXT				"$(^NameDA) a fost instalat cu succes pe acest calculator.$\r$\n$\r$\n\
														Apăsați Terminare pentru a închide asistentul de instalare."
${LangFileString} IDS_PAGE_FINISH_RUN				"Lansare $(^NameDA) după finalizarea instalării"
${LangFileString} IDS_PAGE_FINISH_LINK				"Vizitați Winamp.com"


; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"Bun venit în asistentul de instalare $(^NameDA)"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) vă permite să ascultați, să vizionați și să gestionați muzică, filme, podcast-uri și posturi radio pe internet.$\r$\n$\r$\nDintre caracteristicile $(^NameDA) fac parte:$\r$\n  \
													•  Sincronizarea fără fir a mediatecii locale cu cea a aplicației$\r$\n      \
													   $(^NameDA) pentru Android™$\r$\n  \
													•  Controlul redării în navigatorul web prin$\r$\n      \
													   Bara de instrumnte Winamp$\r$\n  \
													•  Gestionarea etichetelor și a informațiilor despre piese cu$\r$\n      \
													   „Etichetare automată”$\r$\n  \
													•  Generarea listelor de redare cu$\r$\n      \
													   Generatorul automat de liste$\r$\n  \
													•  Abonarea și audierea a peste 30.000 de podcast-uri"
; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"NOTĂ:$\r$\n\
															Pentru a beneficia de noile caracteristici și \
															de interfața Bento (recomandată), \
															trebuie să fie selectate toate componentele."
; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE				"Opțiuni de lansare Winamp"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE			"Selectare opțiuni de lansare Winamp."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION			"Alegeți oricare dintre următoarele opțiuni pentru a crea modalitățile de lansare Winamp."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START			"Creare înregistrare în meniul Pornire"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Creare pictogramă în bara de instrumente Lansare rapidă"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP		"Creare pictogramă pe desktop"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Agentul Winamp nu poate fi închis.$\r$\n\
													Asigurați-vă că nici un alt utilizator nu a deschis o sesiune Windows.$\r$\n$\r$\n \
													După ce ați închis Agentul Winamp, selectați Reîncercare.$\r$\n$\r$\n	\
													Dacă totuși doriți să continuați instalarea oricum, selectați Ignorare.$\r$\n$\r$\n \
													Dacă doriți să abandonați instalarea, selectați Abandonare."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Această versiune de Windows nu mai este suportată.$\r$\n\
												$(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} necesită cel puțin Winsows 2000."

; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"A fost detectată extensia incompatibilă „gen_msn7.dll”!$\r$\n$\r$\nAceastă extensie duce la nefuncționarea Winamp începând cu versiunea 5.57.$\r$\nExtensia va fi dezactivată. Apăsați OK pentru a continua."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"A fost detectată extensia incompatibilă „gen_lyrics.dll”!$\r$\n$\r$\nAceastă extensie duce la nefuncționarea Winamp începând cu versiunea 5.59.$\r$\nExtensia va fi dezactivată. Apăsați OK pentru a continua."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "A fost detectată extensia „gen_lyrics”!$\r$\n$\r$\nVersiunile vechi ale acestei extensii sunt incompatibile cu Winamp 5.6  sau mai recent. Înainte de a continua, asigurați-vă că aveți cea mai recentă versiune de la http://lyricsplugin.com before proceeding."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"S-a detectat ${DIRECTXINSTAL_WINVER_LO} sau o versiune mai mică."
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "S-a detectat ${DIRECTXINSTAL_WINVER_HI} sau o versiune mai mare."
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Verificare versiune ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Este necesară o versiune minimală a ${DIRECTXINSTAL_DIRECTXNAME}."
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Nu se poate detecta versiunea ${DIRECTXINSTAL_DIRECTXNAME}."
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"S-a detectat versiunea ${DIRECTXINSTAL_DIRECTXNAME}."
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Versiune ${DIRECTXINSTAL_DIRECTXNAME} nesuportată."
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Verificare prezență a $0"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Este necesară descărcarea"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Verificare conexiune internet"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"Ultima versiune a ${DIRECTXINSTAL_DIRECTXNAME} este disponibilă la:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Descărcare asistent de instalare ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FOUND						"Găsit"
${LangFileString} IDS_DIRECTX_MISSING					"Lipsește"
${LangFileString} IDS_DIRECTX_SUCCESS					"Succes"
${LangFileString} IDS_DIRECTX_ABORTED					"Abandon"
${LangFileString} IDS_DIRECTX_FAILED					"Eșuare"
${LangFileString} IDS_DIRECTX_DONE						"Finalizare"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Lansare asistent instalare ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"${DIRECTXINSTAL_WINAMPNAME} necesită cel puțin ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} pentru a funcționa corect.$\r$\nÎl instalați acum?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"${DIRECTXINSTAL_WINAMPNAME} necesită cel puțin ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} pentru a funcționa corect."
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"${DIRECTXINSTAL_DIRECTXNAME} nu se poate descărca."
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"${DIRECTXINSTAL_DIRECTXNAME} nu se poate instala."
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"De pe computer lipsește o componentă ${DIRECTXINSTAL_DIRECTXNAME} necesară pentru ${DIRECTXINSTAL_WINAMPNAME}."
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Componenta ${DIRECTXINSTAL_DIRECTXNAME} nu se poate descărca."
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Componenta ${DIRECTXINSTAL_DIRECTXNAME} nu se poate instala."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Nucleu Winamp (necesar)"
${LangFileString} IDS_SEC_AGENT_DESC			"Componentă opțională - permite controlul Winamp din zona de notificare și menține asocierile tipurilor de fișiere"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Nucleu de procesare a fișierelor multimedia (sistem de intrare/ieșire)"
${LangFileString} IDS_SEC_CDDB_DESC				"Suport CDDB - facilitează obținerea automată a titlurilor pieselor din baza de date Gracenote"
${LangFileString} IDS_SEC_DSP_DESC				"Extensie DSP - permite aplicarea unor efecte audio cum ar fi cor și flanger, sau controlul ritmului și al frecvenței"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Suport pentru redare audio (extensii de intrare și decodoare audio)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Suport pentru redarea formatelor MP3, MP2, MP1, AAC (necesar)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Suport pentru redarea formatului WMA (inclusiv suport DRM)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Suport pentru redarea formatelor MIDI (MID, RMI, KAR, MUS, CMF și altele)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Suport pentru redarea formatelor Module (MOD, XM, IT, S3M, ULT și altele)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Suport pentru redarea formatului Ogg Vorbis (OGG)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Suport pentru redarea formatelor MPEG-4 Audio (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Suport pentru redarea formatului FLAC"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Suport pentru redarea CD-urilor audio"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Suport pentru redarea formatelor Waveform (WAV, VOC, AU, AIFF și altele)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Suport pentru redare video (extensii de intrare, decodoare video)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Suport pentru redarea formatelor Windows Media video (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Suport pentru redarea formatelor Nullsoft Video (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Suport pentru redarea formatelor MPEG-1/2 și a altor formate video"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Suport pentru redarea formatelor AVI Video"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Suport pentru redarea formatelor VP6 Flash Video (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Suport pentru redarea formatelor Matroska Video (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Suport pentru redarea formatelor MPEG-4 Video (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Suport pentru redarea fluxurilor Adobe Flash (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Suport pentru codare și transcodare (necesar pentru extragerea pistelor CD și conversia fișierelor)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Suport pentru extragerea și transcodarea în formatul WMA"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Suport pentru extragerea și transcodarea în formatul WAV"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Suport pentru extragerea și transcodarea în formatul M4A și AAC"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Suport pentru extragerea și transcodarea în formatul FLAC"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Suport pentru extragerea și transcodarea în formatul Ogg Vorbis"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Extensii de ieșire - controlează modul de procesare a semnalului audio și trimiterea lui la cartela de sunet"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Convertor WAV/MME clasic (depășit, dar preferat de unii utilizatori în locul extensiilor de codare)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"Ieșire DirectSound (necesar, extensie de ieșire implicită)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Ieșire WaveOut clasică (opțional, nu mai este nici necesar nici recomandat)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Extensii pentru interfața utilizator"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Extensie pentru comenzile rapide globale - controlează Winamp de la tastatură când alte aplicații sunt active"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Suport pentru extensia Direct la piesă - extinde controlul listei de redare și multe altele"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Extensie pentru controlul redării din zona de notificare Windows"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Suport pentru interfețele moderne - permite utilizarea interfețelor avansate cum ar fi Winamp Modern și Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Extensie pentru efecte grafice"
${LangFileString} IDS_SEC_NSFS_DESC				"Extensie pentru efectele grafice în mini-ecran complet"
${LangFileString} IDS_SEC_AVS_DESC				"Extensie pentru studioul pentru efecte grafice avansate"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Extensie pentru efectele grafice Milkdrop"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Extensie pentru efectele grafice Milkdrop2 (extensie implicită pentru efectele grafice)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Suport pentru intrarea LineIn, folosind comanda „linein://” (aplică efecte grafice pentru intrarea de microfon/linie)"
${LangFileString} IDS_GRP_WALIB_DESC			"Colecție de instrumente și extensii Winamp"
${LangFileString} IDS_SEC_ML_DESC				"Mediatecă Winamp (necesar)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Extensie pentru conversia fișierelor dintr-un format în altul"
${LangFileString} IDS_SEC_ML_RG_DESC			"Extensie pentru analiză Replay Gain - analizează amplificarea și egalizează volumul pieselor redate"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Instrument de etichetare automată Winamp (dezvoltat de Gracenote) - completează automat datele extinse"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Director Podcast - permite abonarea și descărcarea podcast-urilor"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Servicii internet - include serviciile SHOUTcast Radio && TV, AOL Radio feat. CBS Radio, Winamp Charts și altele"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Instrument de generare liste (dezvoltat de Gracenote) - creează liste de redare acustic dinamice"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Componente principale ale mediatecii"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Bază de date multimedia locală - conține un puternic sistem de interogare și poate crea vizualizări inteligente"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Gestionar de liste - creează, editează și sortează toate listele de redare"
${LangFileString} IDS_SEC_ML_DISC_DESC			"Interfață pentru extragerea și inscripționarea CD-urilor audio"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Gestionar de semne de carte - gestionează semnele de carte pentru fișierele, dosarele și fluxurile favorite"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Istoric - permite accesarea imediată a tuturor pieselor sau fluxurilor recent redate"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Interfață pentru afișarea informațiilor despre piesa în curs de redare"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Extensii pentru unitățile de redare portabile"
${LangFileString} IDS_SEC_ML_PMP_DESC			"Extensie principală pentru controlul unităților de redare portabile (necesar)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"Suport pentru iPod®"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Suport pentru unitățile Creative® (gestionează unitățile Nomad™, Zen™ și MuVo™)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Suport pentru Microsoft® PlaysForSure® (gestionează toate unitățile compatibile P4S)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"Suport pentru dispozitivele USB (gestionează dispozitivele generice și unitățile de redare USB)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Suport pentru Microsoft® ActiveSync® (gestionează dispozitivele Windows Mobile®, Smartphone și Pocket PC)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Suport pentru dispozitive Android"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Suport pentru Android Wifi"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"Extensie pentru importul/exportul mediatecii compatibile iTunes"

${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Elimină $(^NameDA) din computer."

${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Cale pentru dezinstalare:$\r$\n $INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Winamp"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Dezinstalează toate componentele $(^NameDA), inclusiv extensiile de la terțe părți incluse în pachet."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Opțiuni utilizator"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Elimină toate opțiunile pentru $(^NameDA) și extensiile sale."

${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"Ajutați echipa $(^NameDA) trimițând părerile Dvs."
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"Deschidere dosar $(^NameDA)"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nNotă:  Nu toate fișierele au fost eliminate. Pentru a le vedea deschideți dosarul Winamp."
