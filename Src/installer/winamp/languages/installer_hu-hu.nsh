; Language-Country:	HU-HU
; LangId:			1038
; CodePage:			1252
; Revision:			1.1
; Last udpdated:	2012-05-06
; Author:			László Gárdonyi aka gLes
; Email:			gles@pro.hu

; Notes:
; use ';' or '#' for comments
; strings must be in double quotes.
; only edit the strings in quotes:
# example: ${LangFileString} installFull "Edit This Value Only!"
# Make sure there's no trailing spaces at ends of lines
; To use double quote inside string - '$\'
; To force new line  - '$\r$\n'
; To insert tabulation  - '$\t'

!insertmacro LANGFILE_EXT "Hungarian"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Telepítő nyelve"
${LangFileString} LANGUAGE_DLL_INFO "Kérem válasszon nyelvet."

${LangFileString} installFull "Teljes"
${LangFileString} installStandard "Általános"
${LangFileString} installLite "Könnyű"
${LangFileString} installMinimal "Minimális"
${LangFileString} installPrevious "Előző telepítés"

; BrandingText
${LangFileString} BuiltOn "készült:"
${LangFileString} at " "

${LangFileString} installWinampTop "Jelenleg a Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType} verzióját telepíti. "
${LangFileString} installerContainsFull "Ez a telepítő a teljes telepítő csomagot tartalmazza."
${LangFileString} installerContainsLite "Ez a telepítő a csak könnyű csomagot tartalmazza."
${LangFileString} licenseTop "Kérem olvassa el és fogadja el a licencfeltételeket a telepítés előtt."
${LangFileString} directoryTop "A telepítő az alábbi útvonalat javasolja a $(^NameDA) telepítésére. Ha ezt meg szeretné változtatni, most megteheti."

${LangFileString} uninstallPrompt "Biztosan szeretné eltávolítani a Winampot?"

${LangFileString} msgCancelInstall "Telepítés megszakítása?"
${LangFileString} msgReboot "A telepítés befejezéséhez újra kell indítani a számítógépet.$\r$\nÚjraindítja most? (Ha szeretné később újraindítani, kattintson a Nem gombra.)"
${LangFileString} msgCloseWinamp "A folytatás előtt be kell zárnia a Winampot.$\r$\n$\r$\n	A Winamp bezárása után kattintson az Ismét gombra.$\r$\n$\r$\n	Ha a mellőzni szeretné a Winamp bezárását, de folytatná a telepítést, kattintson a Kihagyás gombra.$\r$\n$\r$\n	Ha szeretné megszakítani a telepítést, kattintson a Leállítás gombra."
${LangFileString} msgInstallAborted "A telepítést a felhasználó leállította"

${LangFileString} secWinamp "Winamp (kötelező)"
${LangFileString} compAgent "Winamp ügynök"
${LangFileString} compModernSkin "Modern téma támogatás"
${LangFileString} uninstallWinamp "Winamp eltávolítása"

${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Windows Media formátum letöltése és a telepítése"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "OGG Vorbis lejátszás"
${LangFileString} secOGGEnc "OGG Vorbis kódolás"
${LangFileString} secAACE "HE-AAC kódolás"
${LangFileString} secMP3E "MP3 kódolás"
${LangFileString} secMP4E "MP4 támogatás"
${LangFileString} secWMAE "WMA kódolás"
${LangFileString} msgWMAError "Hiba történt az összetevők telepítése során. A WMA kódoló nem kerül telepítésre. Kérem látogasson el a http://www.microsoft.com/expression/products/EncoderPro_Overview.aspx oldalra, töltse le a kódoló alkalmazást és próbálja újra."
${LangFileString} secCDDA "CD lejátszás és mentés"
${LangFileString} msgCDError "Hiba történt az összetevők telepítése során. CD másolás/írás működése nem garantált."
${LangFileString} secCDDB "CDDB lemezfelismerés"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"

${LangFileString} secDSP "Jelfeldolgozó stúdió bővítmény"
${LangFileString} secWriteWAV "Régimódi WAV író"
${LangFileString} secLineInput "Vonalbemenet támogatás"
${LangFileString} secDirectSound "DirectSound kimenet támogatása"

${LangFileString} secHotKey "Billentyűparancsok támogatása"
${LangFileString} secJmp "Kibővített ugrás fájlhoz támogatás"
${LangFileString} secTray "Értesítési ikon vezérlő"

${LangFileString} msgRemoveMJUICE "Eltávolítja az MJuice támogatást a rendszerből?$\r$\n$\r$\nHa nem használja az MJF fájlokat más alkalmazásokban, akkor kattintson az Igen gombra."
${LangFileString} msgNotAllFiles "Nem lett eltávolítva az összes fájl.$\r$\nHa szeretné eltávolítani a fájlokat, kérem tegye meg."


${LangFileString} secNSV "Nullsoft videó (NSV)"
${LangFileString} secDSHOW "DirectShow formátumok (MPG, M2V)"
${LangFileString} secAVI "AVI videó"
${LangFileString} secFLV "Flash videó (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "MPEG-4 videó (MP4, M4V)"

${LangFileString} secSWF "Flash Media protokoll (SWF, RTMP)"

${LangFileString} secTiny "Nullsoft Tiny Fullscreen"
${LangFileString} secAVS "Advanced Visualization Studio"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Winamp Médiatár"
${LangFileString} secOM "Online média"
${LangFileString} secWire "Podcast könyvtár"
${LangFileString} secPmp "Hordozható médialejátszók"
${LangFileString} secPmpIpod "iPod® támogatás"
${LangFileString} secPmpCreative "Creative® lejátszók támogatása"
${LangFileString} secPmpP4S "Microsoft® PlaysForSure® támogatása"
${LangFileString} secPmpUSB "USB eszközök támogatása"
${LangFileString} secPmpActiveSync "Microsoft® ActiveSync® támogatása"
${LangFileString} secPmpAndroid "Android eszközök támogatása"
${LangFileString} secPmpWifi "Android WiFi támogatás"

${LangFileString} sec_ML_LOCAL "Helyi média"
${LangFileString} sec_ML_PLAYLISTS "Listák"
${LangFileString} sec_ML_DISC "CD másolás és írás"
${LangFileString} sec_ML_BOOKMARKS "Könyvjelzők"
${LangFileString} sec_ML_HISTORY "Előzmények"
${LangFileString} sec_ML_NOWPLAYING "Most játszott"
${LangFileString} sec_ML_RG "Replay Gain elemző eszköz"
${LangFileString} sec_ML_TRANSCODE "Átkódoló eszköz"
${LangFileString} sec_ML_PLG "Lista generáló"
${LangFileString} sec_ML_IMPEX "Adatbázis importáló/exportáló eszköz"


;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "$(^NameDA) telepítő"
${LangFileString} IDS_SELECT_LANGUAGE  "Kérem válassza ki a telepítő nyelvét"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Multimédia támogatás"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Kimenet"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Hanglejászás"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Hangkódolás"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Videólejátszás"
${LangFileString} IDS_GRP_VISUALIZATION		"Látvány"
${LangFileString} IDS_GRP_UIEXTENSION		"Felhasználói felület"
${LangFileString} IDS_GRP_WALIB				"Winamp Médiatár"
${LangFileString} IDS_GRP_WALIB_CORE		"Fő médiatár összetevők"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Hordozható lejászók támogatása"
${LangFileString} IDS_GRP_LANGUAGES 	    "Nyelvek"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"WaveOut/MME kimenet"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC kódoló"
${LangFileString} IDS_SEC_MILKDROP2 	"Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG	"Auto-címkéző"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Online szolgáltatások konfigurálása..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"A Winamp futó példányainak keresése..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Kapcsolódás az internethez..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Internet-kapcsolat ellenőrzése..."
${LangFileString} IDS_RUN_NOINET				"Nincs internet-kapcsolat"
${LangFileString} IDS_RUN_EXTRACT				"Kibontás"
${LangFileString} IDS_RUN_DOWNLOAD				"Letöltés"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Letöltés kész."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Letöltés sikertelen. Ok:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Letöltés megszakítva."
${LangFileString} IDS_RUN_INSTALL				"Telepítés"
${LangFileString} IDS_RUN_INSTALLFIALED			"Telepítés sikertelen."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"A fájl nem található. Letöltés ütemezése."
${LangFileString} IDS_RUN_DONE					"Kész."

${LangFileString} IDS_DSP_PRESETS 	"SPS beállítások"
${LangFileString} IDS_DEFAULT_SKIN	"alapértelmezett témák"
${LangFileString} IDS_AVS_PRESETS	"AVS beállítások"
${LangFileString} IDS_MILK_PRESETS	"MilkDrop beállítások"
${LangFileString} IDS_MILK2_PRESETS	"MilkDrop2 beállítások"

${LangFileString} IDS_CLEANUP_PLUGINS	"Bővítmények kitakarítása..."
${LangFileString} IDS_REMOVE_SKINS		"Alapértelmezett téma eltávolítása..."


; download
${LangFileString} IDS_DOWNLOADING	"Letöltés"
${LangFileString} IDS_CONNECTING	"Kapcsolódás..."
${LangFileString} IDS_SECOND		" (1 másodperc van hátra)"
${LangFileString} IDS_MINUTE		" (1 perc van hátra)"
${LangFileString} IDS_HOUR			" (1 óra van hátra)"
${LangFileString} IDS_SECONDS		" (%u másodperc van hátra)"
${LangFileString} IDS_MINUTES		" (%u perc van hátra)"
${LangFileString} IDS_HOURS			" (%u óra van hátra)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%)/%skB @ %u.%01ukB/s"


; AutoplayHandler
${LangFileString} AutoplayHandler	"Lejátszás"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"A telepítés befejeződött"
${LangFileString} IDS_PAGE_FINISH_TEXT		"A $(^NameDA) sikeresen feltelepült a számítógépére.$\r$\n$\r$\n\
													Kattintson a Befejezés gombra a varázsló bezárásához."
${LangFileString} IDS_PAGE_FINISH_RUN		"$(^NameDA) indítása a telepítő bezárása után"
${LangFileString} IDS_PAGE_FINISH_LINK		"Kattintson ide a Winamp.com meglátogatásához"


; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"Üdvözli a $(^NameDA) telepítője"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"A $(^NameDA) segítségével zenéket, videókat, podcastokat és internetes rádiókat hallgathat, illetve nézhet.$\r$\n$\r$\n$\r$\nTovábbi lehetőségek:$\r$\n$\r$\n  \
													•  Vezetéknélküli szinkronizálás a $(^NameDA) for Android alkalmazással$\r$\n  \
													•  Vezérelje a lejátszást a böngészőből a Winamp eszköztárral$\r$\n  \
													•  Tisztítsa meg gyűjteménye címkéit az Auto-címkézővel$\r$\n  \
													•  Készítsen listákat az Automatikus lista generálóval$\r$\n  \
													•  Halgassa és iratkozzon fel a több mint 30,000 podcast bármelyikére"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"Megj.: Az új képességek és \
															az ajánlott Bento téma használatához az összes \
															összetevőt ki kell jelölnie."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Parancsikon lehetőségek"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Válasszon az alábbi parancsikon lehetőségek közül."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Válasszon az alábbi lehetőségek közül a Winamp parancsikonjainak testreszabásához."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Start menü bejegyzés létrehozása"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Ikon a Gyorsindítás eszköztáron"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Ikon az asztalon"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Nem sikerült a Winamp ügynök bezárása.$\r$\n\
                                                   Bizonyosodjon meg róla, hogy nincs más felhasználó bejelentkezve a Windowsba.\
                                                   $\r$\n$\r$\n	Miután bezárta a Winamp ügynököt, kattintson az Ismét gombra.\
                                                   $\r$\n$\r$\n	Ha így is meg szeretné próbálni a telepítést, kattintson a Kihagyás gombra.\
                                                   $\r$\n$\r$\n	Ha szeretné megszakítani a telepítést, kattintson a Leállítás gombra."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Ez a Windows verzió már nem támogatott.$\r$\n\
                                                 A $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} telepítéséhez Windows 2000 vagy újabb operációs rendszer szükséges."

; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"Inkompatibilis gen_msn7.dll bővítmény észlelve!$\r$\n$\r$\nEz a bővítmény a Winamp 5.57-es verziójától a Winamp lefagyását okozza indításkor.$\r$\nA bővítmény le lesz tiltva. Kattintson az OK-ra a folytatáshoz."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"Inkompatibilis gen_lyrics.dll bővítmény észlelve!$\r$\n$\r$\nnEz a bővítmény a Winamp 5.59-es verziójától a Winamp lefagyását okozza indításkor.$\r$\nA bővítmény le lesz tiltva. Kattintson az OK-ra a folytatáshoz."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "gen_lyrics bővítmény észlelve!$\r$\n$\r$\nA bővítmény régi verziói nem kompatibilisek a Winamp 5.6-os verziójától. A folytatás előtt bizonyosodjon meg róla, hogy a legújabb verziót használja: http://lyricsplugin.com."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"${DIRECTXINSTAL_WINVER_LO} vagy alacsonyabb észlelve"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "${DIRECTXINSTAL_WINVER_HI} vagy magasabb"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"${DIRECTXINSTAL_DIRECTXNAME} verzió ellenőrzése"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Legalább a ${DIRECTXINSTAL_DIRECTXNAME} verzió szükséges"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Nem sikerült a ${DIRECTXINSTAL_DIRECTXNAME} verzió észlelése"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"${DIRECTXINSTAL_DIRECTXNAME} verzió észlelve"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Nem támogatott ${DIRECTXINSTAL_DIRECTXNAME} verzió"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"$0 létezésének ellenőrzése"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Letöltés szükséges"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Internet-kapcsolat ellenőrzése"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"A legújabb ${DIRECTXINSTAL_DIRECTXNAME} letölthető innen:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"${DIRECTXINSTAL_DIRECTXNAME} telepítő letöltése"
${LangFileString} IDS_DIRECTX_FOUND						"Észlelve"
${LangFileString} IDS_DIRECTX_MISSING					"Nem található"
${LangFileString} IDS_DIRECTX_SUCCESS					"Sikeres"
${LangFileString} IDS_DIRECTX_ABORTED					"Megszakítva"
${LangFileString} IDS_DIRECTX_FAILED					"Sikertelen"
${LangFileString} IDS_DIRECTX_DONE						"Kész"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"${DIRECTXINSTAL_DIRECTXNAME} telepítő futtatása"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"A ${DIRECTXINSTAL_WINAMPNAME} számára legalább ${DIRECTXINSTALL_DIRECTXMINVER} verziójú ${DIRECTXINSTAL_DIRECTXNAME} szükséges a megfelelő működéshez.$\r$\nSzeretné most telepíteni?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"A ${DIRECTXINSTAL_WINAMPNAME} számára legalább ${DIRECTXINSTALL_DIRECTXMINVER} verziójú ${DIRECTXINSTAL_DIRECTXNAME} szükséges a megfelelő működéshez."
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Nem sikerült letölteni: ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Nem sikerült telepíteni: ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"A számítógépéről hiányzik a ${DIRECTXINSTAL_DIRECTXNAME} összetevő, amely a ${DIRECTXINSTAL_WINAMPNAME} számára szükséges"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Nem sikerült a hiányzó ${DIRECTXINSTAL_DIRECTXNAME} összetevő letöltése"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Nem sikerült a hiányzó ${DIRECTXINSTAL_DIRECTXNAME} összetevő telepítése"

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"A Winamp fő elemei (kötelező)"
${LangFileString} IDS_SEC_AGENT_DESC			"Winamp ügynök, a Winamp gyors eléréséhez az értesítési területről és a fájltársítások karbantartásához"
${LangFileString} IDS_GRP_MMEDIA_DESC			"Multimédia támogatás (be- ill. kimeneti rendszer)"
${LangFileString} IDS_SEC_CDDB_DESC				"CDDB támogatás, a lemezek adatainak automatikus letöltéséhez a Gracenote online adatbázisából"
${LangFileString} IDS_SEC_DSP_DESC				"DSP bővítmény, extra hatások létrehozásához, pl. kórus, késleltetés, tempó, hajlítás vezérlés"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Hanglejászás támogatás (Bemeneti bővítmények: hang dekódolók)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"MP3, MP2, MP1, AAC formátumok lejátszásának támogatása (kötelező)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"WMA formátum lejátszásának támogatása (DRM támogatással)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"MIDI formátumok (MID, RMI, KAR, MUS, CMF, stb.) lejátszásának támogatása"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Module formátumok (MOD, XM, IT, S3M, ULT, stb.) lejátszásának támogatása"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Ogg Vorbis formátum (OGG) lejátszásának támogatása"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"MPEG-4 hangformátumok (MP4, M4A) lejátszásának támogatása"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"FLAC formátum lejátszásának támogatása"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Hang CD-k lejátszásának támogatása"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Hullámforma formátumok (WAV, VOC, AU, AIFF, stb.) lejátszásának támogatása"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Videólejátszás támogatás (Bemeneti bővítmények: videó dekódolók)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Windows Media videó formátumok (WMV, ASF) lejátszásának támogatása"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Nullsoft Video formátumok (NSV, NSA) lejátszásának támogatása"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"MPEG-1/2 és más videóformátumok lejátszásának támogatása"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"AVI videók lejátszásának támogatása"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Flash videók (FLV) lejátszásának támogatása"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Matroska videók (MKV) lejátszásának támogatása"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"MPEG-4 videók (MP4, M4V) lejátszásának támogatása"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Adobe Flash adatfolyam formátumok (SWF, RTMP) lejátszásának támogatása"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Kódolás és átkódolás támogatása (szükséges a CD bemásoláshoz és a fájlok formátumának átalakításához)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"WMA formátum kódolásának támogatása"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"WAV formátum kódolásának támogatása"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"M4A és AAC formátum kódolásának támogatása"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"FLAC formátum kódolásának támogatása"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Ogg Vorbis formátum kódolásának támogatása"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Kimeneti bővítmények (ezek vezérlik a hangok feldolgozását és útját a hangkártyáig)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Régimódi WAV/MME író (elavult, de néhány felhasználó még mindig előnyben részesíti)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"DirectSound kimenet (kötelező / alapértelmezett kimeneti bővítmény)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Régimódi WaveOut kimenet (opcionális, és már nem javasolt vagy kötelező)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Felhasználó felület bővítmények"
${LangFileString} IDS_SEC_HOTKEY_DESC			"Billentyűparancsok bővítmény, a Winamp irányítása a billentyűzettel, akkor is ha más program van fókuszban"
${LangFileString} IDS_SEC_JUMPEX_DESC			"Kibővített ugrás fájlhoz támogatás, mellyel sorbaállíthat számokat a listáján, és még sok sok mást is tehet"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Nullsoft Értesítési ikon vezérlő, amellyel vezérlőikonokat vehet fel az értesítési területre (az óra mellé a tálcán)"
${LangFileString} IDS_SEC_FREEFORM_DESC			"Modern témák támogatása, pl. a Winamp Modern vagy a Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Látvány bővítmények"
${LangFileString} IDS_SEC_NSFS_DESC				"Nullsoft Tiny Fullscreen látvány bővítmény"
${LangFileString} IDS_SEC_AVS_DESC				"Advanced Visualization Studio látvány bővítmény"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Milkdrop látvány bővítmény"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Milkdrop2 látvány bővítmény (alapértelmezett)"
${LangFileString} IDS_SEL_LINEIN_DESC			"Vonalbemenet támogatás a linein:// parancs használatával (a látványt rákapcsolja a vonalbemenetre)"
${LangFileString} IDS_GRP_WALIB_DESC			"Winamp Médiatár"
${LangFileString} IDS_SEC_ML_DESC				"Winamp Médiatár (kötelező)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Átkódoló eszköz, fájlok formátumának átalakításához egyikből a másikba"
${LangFileString} IDS_SEC_ML_RG_DESC			"Replay Gain elemző eszköz, a számok közti hangerőkülönbségek kiegyenlítéshez"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Winamp Auto-címkéző (a Gracenote támogatásával), a hiányzó metaadatok kitöltéséhez"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Podcast könyvtár, a podcastokra való feliratkozáshoz és letöltésükhöz"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Online szolgáltatások, pl. SHOUTcast rádiók és tévék, AOL rádió, benne a CBS rádió, Winamp toplisták, stb."
${LangFileString} IDS_SEC_ML_PLG_DESC			"Winamp lista generáló (a Gracenote támogatásával), akusztikusan dinamikus listák létrehozásához"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Fő Médiatár összetevők"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Helyi médiaadatbázis, hatékony lekérdező rendszerrel és egyéni intelligens nézetekkel"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Listakezelő, listák létrehozásához, szerkesztéséhez és legfőképp tárolásához"
${LangFileString} IDS_SEC_ML_DISC_DESC			"CD másolás és írás, a Médiatár felülete a hang CD-k másolásához és írásához"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Könyvjelző kezelő, a kedvenc állomások, fájlok vagy könyvtárak elmentéséhez"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Előzmények, közvetlen hozzáférés az összes korábban hallgatott helyi vagy távoli fájlokhoz és állomásokhoz"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Most játszott, vagyis az éppen játszott szám információinak megjelenítése"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Hordozható lejátszók támogatása"
${LangFileString} IDS_SEC_ML_PMP_DESC			"Hordozható lejátszók támogatásának fő összetevője (kötelező)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"iPod® támogatás"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Creative® hordozható lejátszók (Nomad™, Zen™ és MuVo™ lejátszók kezelése)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Microsoft® PlaysForSure® (P4S kompatibilis lejátszók kezelése)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"USB eszközök támogatása (általános USB pendrive-ok és lejátszók kezelése)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Microsoft® ActiveSync® (Windows Mobile®, Smartphone és Pocket PC eszközök kezelése)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Android eszközök támogatása"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Android WiFi támogatás"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"iTunes-komptibilis médiatár adatbázist importáló/exportáló bővítmény"

${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Eltávolítja a $(^NameDA)ot a számítógépéről."

${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Eltávolítás útvonala:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Médialejátszó"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Eltávolítja a $(^NameDA) médialejátszó összes összetevőjét, a kapcsolódó külső bővítményekkel együtt."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Felhasználói beállítások"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Eltávolítja a $(^NameDA) összes mentett beállítását és bővítményeit."

${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"Segítsen a $(^NameDA) fejlesztőinek visszajelzések küldésével"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"$(^NameDA) könyvtárának megnyitása"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nMegjegyzés: Nem sikerült eltávolítani az összes fájlt. A fájlok megtekintéséhez nyissa meg a Winamp könyvtárát."
