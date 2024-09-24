; Language-Country:	EN-US
; LangId:			1033
; CodePage:			1252
; Revision:			10
; Last udpdated:	28.04.2014
; Author:			Nullsoft
; Email:			

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
; 29 apr 2014 > dro: scrubbed the file of unused strings for the 5.8+ installer (and reset this file's history)

!insertmacro LANGFILE_EXT "English"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Installer Language"
${LangFileString} LANGUAGE_DLL_INFO "Please select a language."
 
${LangFileString} installFull "Full"
${LangFileString} installStandard "Standard"
${LangFileString} installLite "Lite"
${LangFileString} installMinimal "Minimal"
${LangFileString} installPrevious "Previous Installation"

; BrandingText
${LangFileString} BuiltOn "built on"
${LangFileString} at "at"

;${LangFileString} installWinampTop "This will install Winamp ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MINOR_SECOND} ${InstallType}."
${LangFileString} installWinampTop "This will install Winamp ${VERSION_MAJOR}.${VERSION_MINOR} ${InstallType}."
${LangFileString} installerContainsFull " This installer contains the full install."
${LangFileString} installerContainsLite "This installer contains the lite install."
${LangFileString} licenseTop "Please read and agree to the license terms below before installing."
${LangFileString} directoryTop "The installer has determined the optimal location for $(^NameDA). If you would like to change the folder, do so now."

${LangFileString} uninstallPrompt "This will uninstall Winamp. Continue?"

${LangFileString} msgCancelInstall "Cancel install?"
${LangFileString} msgReboot "A reboot is required to complete the installation.$\r$\nReboot now? (If you wish to reboot later, select No)"
${LangFileString} msgCloseWinamp "You must close Winamp before you can continue.$\r$\n$\r$\n	After you have closed Winamp, select Retry.$\r$\n$\r$\n	If you wish to try to install anyway, select Ignore.$\r$\n$\r$\n	If you wish to abort the installation, select Abort."
${LangFileString} msgInstallAborted "Install aborted by user"

${LangFileString} secWinamp "Winamp (required)"
${LangFileString} compAgent "Winamp Agent"
${LangFileString} compModernSkin "Modern Skin Support"
${LangFileString} safeMode "Winamp (Safe Mode)"
${LangFileString} uninstallWinamp "Uninstall Winamp"

${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Download and Install Windows Media Format"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "OGG Vorbis Playback"
${LangFileString} secOGGEnc "OGG Vorbis Encoding"
${LangFileString} secAACE "HE-AAC encoding"
${LangFileString} secMP3E "MP3 encoding"
${LangFileString} secMP4E "MP4 support"
${LangFileString} secWMAE "WMA encoding"
${LangFileString} msgWMAError "There was a problem installing components. WMA Encoder will not be installed. Please visit http://www.microsoft.com/windows/windowsmedia/9series/encoder/ , download the encoder and try again."
${LangFileString} secCDDA "CD playback and extraction"
${LangFileString} msgCDError "There was a problem installing components. CD Ripping/Burning may not function properly."
${LangFileString} secCDDB "CDDB for recognizing CDs"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"

${LangFileString} secDSP "Signal Processor Studio Plug-in"
${LangFileString} secWriteWAV "Old-school WAV writer"
${LangFileString} secLineInput "Line Input Support"
${LangFileString} secDirectSound "DirectSound output support"
${LangFileString} secWasapi "WASAPI output support"

${LangFileString} secHotKey "Global Hotkey Support"
${LangFileString} secTray "Nullsoft Tray Control"

${LangFileString} msgRemoveMJUICE "Remove MJuice support from your system?$\r$\n$\r$\nSelect YES unless you use MJF files in programs other than Winamp."
${LangFileString} msgNotAllFiles "Not all files were removed.$\r$\nIf you wish to remove the files yourself, please do so."

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

${LangFileString} secML "Winamp Media Library"
${LangFileString} secOM "Online Media"
${LangFileString} secWire "Podcast Directory"
${LangFileString} secPmp "Portable Media Players"
${LangFileString} secPmpIpod "iPod® support"
${LangFileString} secPmpCreative "Support for Creative® players"
${LangFileString} secPmpP4S "Support for Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "USB Devices Support"
${LangFileString} secPmpActiveSync "Support for Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid "Android device support"
${LangFileString} secPmpWifi "Android Wifi support"

${LangFileString} sec_ML_LOCAL "Local Media"
${LangFileString} sec_ML_PLAYLISTS "Playlists"
${LangFileString} sec_ML_DISC "CD Rip & Burn"
${LangFileString} sec_ML_BOOKMARKS "Bookmarks"
${LangFileString} sec_ML_HISTORY "History"
${LangFileString} sec_ML_NOWPLAYING "Now Playing"
${LangFileString} sec_ML_RG "Replay Gain Analysis Tool"
${LangFileString} sec_ML_TRANSCODE "Transcoding Tool"
${LangFileString} sec_ML_PLG "Playlist Generator"
${LangFileString} sec_ML_IMPEX "Database Import/Export Tool"
${LangFileString} sec_ML_NFT "NFT Library"
${LangFileString} sec_ML_FANZONE "Fanzone"
${LangFileString} sec_ML_HOTMIX "Hotmix Radio"

;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION "$(^NameDA) Installer"
${LangFileString} IDS_SELECT_LANGUAGE "Please select the language of the installer"

; Groups
${LangFileString} IDS_GRP_MMEDIA "Multimedia Engine"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT "Output"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC "Audio Playback"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC "Audio Encoders"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC "Video Playback"
${LangFileString} IDS_GRP_VISUALIZATION "Visualization"
${LangFileString} IDS_GRP_UIEXTENSION "User Interface Extensions"
${LangFileString} IDS_GRP_WALIB "Winamp Library"
${LangFileString} IDS_GRP_WALIB_CORE "Core Media Library Components"
${LangFileString} IDS_GRP_WALIB_PORTABLE "Portable Media Player Support"
${LangFileString} IDS_GRP_LANGUAGES "Languages"

; Sections
${LangFileString} IDS_SEC_OUT_WAV "WaveOut/MME Output"
${LangFileString} IDS_SEC_WAV_ENC "WAV"
${LangFileString} IDS_SEC_MP3_DEC "MP3"
${LangFileString} IDS_SEC_FLAC_DEC "FLAC"
${LangFileString} IDS_SEC_FLAC_ENC "FLAC encoding"
${LangFileString} IDS_SEC_MILKDROP2 "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG "Auto-Tagger"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO "French Radio Plugin"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE "Configuring online services..."
${LangFileString} IDS_RUN_CHECK_PROCESS "Checking if another instance of Winamp is running..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED "Opening internet connection..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE "Checking if internet available..."
${LangFileString} IDS_RUN_NOINET "No internet connection"
${LangFileString} IDS_RUN_EXTRACT "Extracting"
${LangFileString} IDS_RUN_DOWNLOAD "Downloading"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS "Download completed."
${LangFileString} IDS_RUN_DOWNLOADFAILED "Download failed. Reason:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED "Download canceled."
${LangFileString} IDS_RUN_INSTALL "Installing"
${LangFileString} IDS_RUN_INSTALLFIALED "Installation failed."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD "File not found. Scheduling download."
${LangFileString} IDS_RUN_DONE "Done."

${LangFileString} IDS_DSP_PRESETS "SPS presets"
${LangFileString} IDS_DEFAULT_SKIN "default skins"
${LangFileString} IDS_AVS_PRESETS "AVS presets"
${LangFileString} IDS_MILK_PRESETS "MilkDrop presets"
${LangFileString} IDS_MILK2_PRESETS "MilkDrop2 presets"

${LangFileString} IDS_CLEANUP_PLUGINS "Cleanup plug-ins..."
${LangFileString} IDS_REMOVE_SKINS "Remove default skins..."

; download
${LangFileString} IDS_DOWNLOADING "Downloading"
${LangFileString} IDS_CONNECTING "Connecting ..."
${LangFileString} IDS_SECOND " (1 second remaining)"
${LangFileString} IDS_MINUTE " (1 minute remaining)"
${LangFileString} IDS_HOUR " (1 hour remaining)"
${LangFileString} IDS_SECONDS " (%u seconds remaining)"
${LangFileString} IDS_MINUTES " (%u minutes remaining)"
${LangFileString} IDS_HOURS " (%u hours remaining)"
${LangFileString} IDS_PROGRESS "%skB (%d%%) of %skB @ %u.%01ukB/s"

; AutoplayHandler
${LangFileString} AutoplayHandler "Play"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE "Installation Complete"
${LangFileString} IDS_PAGE_FINISH_TEXT "$(^NameDA) has been installed on your computer.$\r$\n$\r$\nClick Finish to close this wizard."
${LangFileString} IDS_PAGE_FINISH_RUN "Launch $(^NameDA) after the installer closes"
${LangFileString} IDS_PAGE_FINISH_LINK "Click here to visit Winamp.com"

; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE "Welcome to the $(^NameDA) installer"

!ifdef EXPRESS_MODE
;${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) allows you to listen, watch and manage \
;											music, videos, podcasts and internet radio. Listen at \
;											home, at work, in the car.\
;											$\r$\n$\r$\n\
;											Features include:$\r$\n$\r$\n  \
;											•  Wirelessly sync media to the Winamp for Android app$\r$\n$\r$\n  \
;											•  Clean up media metadata with the Auto-Tag feature$\r$\n$\r$\n  \
;											•  Build playlists using the Automatic Playlist generator$\r$\n$\r$\n  \
;											•  Listen to over 50,000 SHOUTcast worldwide radio$\r$\n      \
;											   stations$\r$\n$\r$\n  \
;											•  Listen and subscribe to over 30,000 podcasts"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) allows you to play and manage your music library, listen to Shoutcast Radio, and much more.$\r$\n$\r$\n\
											Features include:$\r$\n$\r$\n  \
											•  Podcast sync; Mobile device management$\r$\n$\r$\n  \
											•  CD Ripper && Format Converter$\r$\n$\r$\n  \
											•  ReplayGain smart volume; Milkdrop && AVS visuals$\r$\n$\r$\n  \
											•  Lots of the usually expected Winamp loveliness$\r$\n$\r$\n  \
											•  I am Groot!"
${LangFileString} IDS_PAGE_WELCOME_LEGAL	"By clicking “Next”, you agree to the $(^NameDA) <a id=$\"winamp_eula$\">License Agreement</a> and <a id=$\"winamp_privacy_policy$\">Privacy Policy</a>."
!else
;${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) allows you to manage your media library and listen to internet radio.$\r$\n$\r$\n\
;											Features include:$\r$\n$\r$\n  \
;												•  Wirelessly sync media to the Winamp for Android app$\r$\n$\r$\n  \
;												•  Build playlists using the Automatic Playlist generator$\r$\n$\r$\n  \
;												•  Listen and subscribe to over 30,000 podcasts"
; TODO determine the correct messaging
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) allows you to play and manage your music library, listen to Shoutcast Radio, and much more.$\r$\n$\r$\n\
											Features include:$\r$\n$\r$\n  \
											•  Podcast sync; Mobile device management$\r$\n$\r$\n  \
											•  CD Ripper && Format Converter$\r$\n$\r$\n  \
											•  ReplayGain smart volume; Milkdrop && AVS visuals$\r$\n$\r$\n  \
											•  Lots of the usually expected Winamp loveliness$\r$\n$\r$\n  \
											•  I am Groot!"
!endif ; defined (EXPRESS_MODE)

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST "NOTE: To enjoy the features and design of the Bento skins (recommended), all components should be checked."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE "Choose Start Options"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE "Select from the following start options."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION "Choose from the following options to configure your Winamp start options."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START "Create Start menu entry"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH "Create Quick Launch icon"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP "Create Desktop icon"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Unable to close Winamp Agent.$\r$\n\
												Make sure that another user is not logged into Windows.\
												$\r$\n$\r$\n	After you have closed WinampAgent, select Retry.\
												$\r$\n$\r$\n	If you wish to try to install anyway, select Ignore.\
												$\r$\n$\r$\n	If you wish to abort the installation, select Abort."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"This version of Windows is no longer supported.$\r$\n$\r$\n\
											$(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} requires a minimum of Windows 7 SP1 or newer."

; Microsoft Visual C++ 2019 Redistributable Section (VC142 runtime)
${LangFileString} IDS_VC_REDIST_MSDOWNLOAD_URL "https://aka.ms/vs/16/release/vc_redist.x86.exe" # Do Not Change This!
${LangFileString} IDS_VC_REDIST_NAME "Microsoft Visual C++ 2019 Redistributable"
; ${LangFileString} IDS_VC_REDIST_CHECKING_VC_REDIST_VER "Checking for $(IDS_VC_REDIST_NAME) installation..."
; ${LangFileString} IDS_VC_REDIST_NOT_FOUND "Required $(IDS_VC_REDIST_NAME) not found!"
${LangFileString} IDS_MSG_VC_REDIST_LINK_TO_MSDOWNLOAD "$(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR} will not run until you install the required Microsoft VS2019 runtime from:"
${LangFileString} IDS_VC_DOWNLOAD "Download now?"
${LangFileString} IDS_VC_DOWNLOAD_OR_ABORT "Click Yes to download or No to abort."

; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE "Incompatible 3rd-party gen_msn7.dll plugin detected!$\r$\n$\r$\nThis plugin causes Winamp 5.57 and later to crash on load.$\r$\nPlugin will now be disabled. Click OK to proceed."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE "Incompatible 3rd-party gen_lyrics.dll plugin detected!$\r$\n$\r$\nThis plugin causes Winamp 5.59 and later to crash on load.$\r$\nPlugin will now be disabled. Click OK to proceed."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING "3rd-party gen_lyrics plugin detected!$\r$\n$\r$\nOld versions of this plugin are incompatible with Winamp 5.6 and newer. Make sure you've got the latest version from http://lyricsplugin.com before proceeding."
${LangFileString} IDS_LYRICS_IE_PLUGIN_DISABLE "Incompatible 3rd-party gen_lyrics_ie.dll plugin detected!$\r$\n$\r$\nThis plugin causes Winamp to crash.$\r$\nPlugin will now be disabled. Click OK to proceed."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER "Detected ${DIRECTXINSTAL_WINVER_LO} or lower"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "Detected ${DIRECTXINSTAL_WINVER_HI} or higher"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER "Checking ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER "Required minimal ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX "Unable to detect ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER "Detected ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER "Unsupported ${DIRECTXINSTAL_DIRECTXNAME} version"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT "Checking if $0 present"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED "Download required"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET "Checking internet connection"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD "Latest version of ${DIRECTXINSTAL_DIRECTXNAME} available at:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP "Downloading ${DIRECTXINSTAL_DIRECTXNAME} installer"
${LangFileString} IDS_DIRECTX_FOUND "Found"
${LangFileString} IDS_DIRECTX_MISSING "Missing"
${LangFileString} IDS_DIRECTX_SUCCESS "Success"
${LangFileString} IDS_DIRECTX_ABORTED "Aborted"
${LangFileString} IDS_DIRECTX_FAILED "Failed"
${LangFileString} IDS_DIRECTX_DONE "Done"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP "Running ${DIRECTXINSTAL_DIRECTXNAME} installer"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL "${DIRECTXINSTAL_WINAMPNAME} requires at least ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} to operate properly.$\r$\nInstall it right now?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED "${DIRECTXINSTAL_WINAMPNAME} requires at least ${DIRECTXINSTAL_DIRECTXNAME} ${DIRECTXINSTALL_DIRECTXMINVER} to operate properly"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED "Unable to download ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED "Unable to install ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED "Your computer is missing a ${DIRECTXINSTAL_DIRECTXNAME} component required by ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED "Unable to download missing ${DIRECTXINSTAL_DIRECTXNAME} component"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED "Unable to install missing ${DIRECTXINSTAL_DIRECTXNAME} component"

;French Radio Section
${LangFileString} IDS_FRENCHRADIO_INSTALLING "Installing $(IDS_SEC_GEN_FRENCHRADIO)..."

;Microsoft Visual C++ 2019 Redistributable (VC_REDIST_16_NAME) Section
${LangFileString} IDS_VC_REDIST_CHECKING_VC_REDIST_VER "Checking for $(VC_REDIST_16_NAME) installation..."
${LangFileString} IDS_VC_REDIST_16_NOT_FOUND "Required $(VC_REDIST_16_NAME) not found!"
${LangFileString} IDS_VC_REDIST_LINK_TO_MSDOWNLOAD "Winamp will not run until you install the required Microsoft runtime from:"
${LangFileString} IDS_VC_ABORT "Installer will now abort."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC "Winamp core (required)"
${LangFileString} IDS_SEC_AGENT_DESC "Winamp Agent, for quick system tray access and maintaining filetype associations"
${LangFileString} IDS_GRP_MMEDIA_DESC "Multimedia Engine (Input/Output system)"
${LangFileString} IDS_SEC_CDDB_DESC "CDDB support, for automatically fetching the CD titles from the online Gracenote database"
${LangFileString} IDS_SEC_DSP_DESC "DSP plugin, for applying extra effects such as chorus, flanger, tempo and pitch control"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC "Audio Playback Support (Input Plug-ins: Audio Decoders)"
${LangFileString} IDS_SEC_MP3_DEC_DESC "Playback support for MP3, MP2, MP1, AAC formats (required)"
${LangFileString} IDS_SEC_WMA_DEC_DESC "Playback support for WMA format (including DRM support)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC "Playback support for MIDI formats (MID, RMI, KAR, MUS, CMF and more)"
${LangFileString} IDS_SEC_MOD_DEC_DESC "Playback support for Module formats (MOD, XM, IT, S3M, ULT and more)"
${LangFileString} IDS_SEC_OGG_DEC_DESC "Playback support for Ogg Vorbis format (OGG, OGA)"
${LangFileString} IDS_SEC_MP4_DEC_DESC "Playback support for MPEG-4 Audio formats (MP4, M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC "Playback support for FLAC format"
${LangFileString} IDS_SEC_CDDA_DEC_DESC "Playback support for Audio CDs"
${LangFileString} IDS_SEC_WAV_DEC_DESC "Playback support for Waveform formats (WAV, VOC, AU, AIFF and more)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC "Video Playback Support (Input Plug-ins: Video Decoders)"
${LangFileString} IDS_SEC_WMV_DEC_DESC "Playback support for Windows Media video formats (WMV, ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC "Playback support for Nullsoft Video format (NSV, NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC "Playback support for MPEG-1/2 and other video formats"
${LangFileString} IDS_SEC_AVI_DEC_DESC "Playback support for AVI Video"
${LangFileString} IDS_SEC_FLV_DEC_DESC "Playback support for Flash Video (FLV)"
${LangFileString} IDS_SEC_MKV_DEC_DESC "Playback support for Matroska Video (MKV)"
${LangFileString} IDS_SEC_M4V_DEC_DESC "Playback support for MPEG-4 Video (MP4, M4V)"
${LangFileString} IDS_SEC_SWF_DEC_DESC "Playback support for Adobe Flash streaming format (SWF, RTMP)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC "Encoding and Transcoding Support (required for CD Ripping and converting from one file format to another)"
${LangFileString} IDS_SEC_WMA_ENC_DESC "Support for ripping and transcoding to WMA format"
${LangFileString} IDS_SEC_WAV_ENC_DESC "Support for ripping and transcoding to WAV format"
${LangFileString} IDS_SEC_AAC_ENC_DESC "Support for ripping and transcoding to M4A and AAC formats"
${LangFileString} IDS_SEC_FLAC_ENC_DESC "Support for ripping and transcoding to FLAC format"
${LangFileString} IDS_SEC_OGG_ENC_DESC "Support for ripping and transcoding to Ogg Vorbis format"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC "Output Plug-ins (which control how audio is processed and sent to your soundcard)"
${LangFileString} IDS_SEC_OUT_DISK_DESC "Old-school WAV/MME Writer (deprecated, but some users still prefer to use it instead of the Encoder Plug-ins)"
${LangFileString} IDS_SEC_OUT_WASAPI_DESC "Windows Audio (WASAPI) Output (experimental)"
${LangFileString} IDS_SEC_OUT_DS_DESC "DirectSound Output (required / default Output plugin)"
${LangFileString} IDS_SEC_OUT_WAV_DESC "Old-school WaveOut Output (optional, and no longer recommended or required)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC "User Interface Extensions"
${LangFileString} IDS_SEC_HOTKEY_DESC "Global Hotkeys plugin, for controlling Winamp with the keyboard when other applications are in focus"
${LangFileString} IDS_SEC_TRAYCTRL_DESC "Nullsoft Tray Control plugin, for adding Play control icons in the system tray"
${LangFileString} IDS_SEC_FREEFORM_DESC "Modern Skin Support, required for using freeform skins such as Winamp Modern and Bento"
${LangFileString} IDS_GRP_VISUALIZATION_DESC "Visualization Plug-ins"
${LangFileString} IDS_SEC_NSFS_DESC "Nullsoft Tiny Fullscreen visualization plugin"
${LangFileString} IDS_SEC_AVS_DESC "Advanced Visualization Studio plugin"
${LangFileString} IDS_SEC_MILKDROP_DESC "Milkdrop visualization plugin"
${LangFileString} IDS_SEC_MILKDROP2_DESC "Milkdrop2 visualization plugin (default vis plugin)"
${LangFileString} IDS_SEL_LINEIN_DESC "Line Input Support using linein:// command (applies visualizer to mic/line input)"
${LangFileString} IDS_GRP_WALIB_DESC "Winamp Library"
${LangFileString} IDS_SEC_ML_DESC "Winamp Media Library (required)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC "Transcoding Tool, used for converting from one file format to another"
${LangFileString} IDS_SEC_ML_RG_DESC "Replay Gain Analysis Tool, used for volume-levelling"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC "Winamp Auto-Tagger (Powered by Gracenote), for filling in missing metadata"
${LangFileString} IDS_SEC_ML_WIRE_DESC "Podcast Directory, for subscribing to and downloading podcasts"
${LangFileString} IDS_SEC_ML_ONLINE_DESC "Online Services, including SHOUTcast Radio && TV, AOL Radio feat. CBS Radio, Winamp Charts, and more"
${LangFileString} IDS_SEC_ML_PLG_DESC "Winamp Playlist Generator (powered by Gracenote), for creating acoustically dynamic playlists"
${LangFileString} IDS_GRP_WALIB_CORE_DESC "Core Media Library Components"
${LangFileString} IDS_SEC_ML_LOCAL_DESC "Local Media database, with powerful query system and custom smart views"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC "Playlists Manager, for creating, editing and storing all your playlists"
${LangFileString} IDS_SEC_ML_DISC_DESC "CD Rip && Burn, the media library interface for ripping && burning Audio CDs"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC "Bookmarks Manager, for bookmarking your favorite streams, files or folders"
${LangFileString} IDS_SEC_ML_HISTORY_DESC "History, for instant access to all recently played local or remote files and streams"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC "Now Playing, for viewing information about the currently playing track"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC "Portable Media Player Support"
${LangFileString} IDS_SEC_ML_PMP_DESC "Core Portable Media Player Support plugin (required)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC "iPod® support"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC "Support for Creative® portables (for managing Nomad™, Zen™ and MuVo™ players)"
${LangFileString} IDS_SEC_PMP_P4S_DESC "Support for Microsoft® PlaysForSure® (for managing all MTP && P4S compatible players)"
${LangFileString} IDS_SEC_PMP_USB_DESC "USB Devices support (for managing generic usb thumbdrives and players)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC "Support for Microsoft® ActiveSync® (for managing Windows Mobile®, Smartphone && Pocket PC devices)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC "Support for Android devices"
${LangFileString} IDS_SEC_PMP_WIFI_DESC "Android Wifi support"
${LangFileString} IDS_SEC_ML_IMPEX_DESC "iTunes-compatible Media Library database import/export plugin"
${LangFileString} IDS_SEC_ML_NFT_DESC "Play and download music from your NFT Library"
${LangFileString} IDS_SEC_ML_FANZONE_DESC "Connect to the online Winamp Fanzone"
${LangFileString} IDS_SEC_ML_HOTMIX_DESC "Connect and listen to Hotmix Radio stations"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO_DESC "Listen to more than 300 French radio stations, live in $(^NameDA) (Virgin radio, NRJ, RTL, Skyrock, RMC...)"

${LangFileString} IDS_FIREWALL "Adding Firewall Records"
${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC "Remove $(^NameDA) from your computer."
${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER "Uninstall path:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER "Media Player"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC "Uninstall all $(^NameDA) Media Player components including bundled third party plug-ins."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES "User Preferences"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC "Remove all $(^NameDA) preferences and plug-ins."
${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT "Help $(^NameDA) by submitting feedback"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT "Open $(^NameDA) folder"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED "$\r$\n$\r$\n$\r$\nNote:  Not all files were removed from this uninstall. To view, open the Winamp folder."
${LangFileString} IDS_UNINSTALL_SUBHEADER "$(^NameDA) has now been uninstalled from your computer.$\r$\n$\r$\nClick Finish to close."

;!ifdef EXPRESS_MODE
${LangFileString} IDS_EXPRESS_MODE_HEADER "$(^NameDA) Installation Mode"
${LangFileString} IDS_EXPRESS_MODE_SUBHEADER "Select Installation Mode"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_INSTALL_RADIO "&Standard Install"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_INSTALL_TEXT "This will install $(^NameDA) with recommended components selected at$\r$\n'$INSTDIR'"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_REINSTALL_TEXT "This will install $(^NameDA) with previously selected components at$\r$\n'$INSTDIR'"
${LangFileString} IDS_EXPRESS_MODE_CUSTOM_INSTALL_RADIO "&Custom Install"
${LangFileString} IDS_EXPRESS_MODE_CUSTOM_INSTALL_TEXT "Custom install allows you to tailor $(^NameDA) to your own personal taste$\r$\n\
                                                        by manually selecting the components you would like to be installed."
;!endif