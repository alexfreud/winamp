; Language-Country:	ZH-TW
; LangId:			1033
; CodePage:			1252
; Revision:			5
; Last udpdated:	21.03.2008
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
; 04.10 > barabanger:  added 360 after Microsoft Xbox.
; 05.10 > barabanger:  in IDS_SEC_FREEFORM_DESC Winamp Bento changed to  "Bento"
; 05.10 > djegg: fixed typos in header comments, added extra notes
; 06.10 > barabanger: milkdrop2 strings added
; 27.10 > djegg: removed some trailing spaces
; 01.11 > benski: added in_flv, line 112
; 02.11 > djegg: added description for in_flv, line 290
; 15.11 > barabanger: added old os message - IDS_MSG_WINDOWS_TOO_OLD, line 268
; 14.01 > barabanger: changed winamp remote bundle text (IDS_BUNDLE1_DESCRIPTION).
; 20.03 > barabanger: added toolbar search (IDS_BUNDLE21_XXX).
; 21.03 > barabanger: added winamp search (IDS_WINAMP_SEARCH).
; 26.03 > djegg: removed "(enhanced by Google®)" from IDS_BUNDLE21_DESCRIPTION

!insertmacro LANGFILE_EXT "TradChinese"
 
${LangFileString} installFull "Full 版"
${LangFileString} installStandard "Standard 版"
${LangFileString} installLite "Lite 版"
${LangFileString} installMinimal "最少安裝元件"
${LangFileString} installPrevious "舊版"

${LangFileString} installWinampTop "這會安裝 Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}。"
${LangFileString} installerContainsFull "這個安裝程式可安裝完整版軟體。"
${LangFileString} installerContainsLite "這個安裝程式可安裝精簡版軟體。"
${LangFileString} licenseTop "請閱讀以下授權合約，同意後再開始安裝."
${LangFileString} directoryTop "安裝程式已確認 $(^NameDA) 的最佳位置。如果想要變更資料夾，請立即變更."

${LangFileString} uninstallPrompt "這將解除安裝 Winamp。是否要繼續?"

${LangFileString} msgCancelInstall "是否要取消安裝?"
${LangFileString} msgReboot "重新啟動才會完成安裝程序。$\r$\n現在重新啟動?(若要稍後再重新啟動，請選取 [否])"
${LangFileString} msgCloseWinamp "您必須先關閉 Winamp，才能繼續進行。$\r$\n$\r$\n	關閉 Winamp 後，選取 [重試]。$\r$\n$\r$\n	如果仍然要繼續安裝，請選取 [忽略]。$\r$\n$\r$\n	若要中止安裝程序，請選取 [中止]。"
${LangFileString} msgInstallAborted "使用者已中止安裝程序"

${LangFileString} secWinamp "Winamp (要求使用)"
${LangFileString} compAgent "Winamp Agent"
${LangFileString} compModernSkin "支援新式面板"
${LangFileString} uninstallWinamp "解除安裝 Winamp"

${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "下載並安裝 Windows Media Format"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "OGG Vorbis Playback"
${LangFileString} secOGGEnc "OGG Vorbis 編碼"
${LangFileString} secAACE "AAC/aacPlus 編碼"
${LangFileString} secMP3E "MP3 編碼"
${LangFileString} secMP4E "支援 MP4"
${LangFileString} secWMAE "WMA 編碼"
${LangFileString} msgWMAError "安裝元件時出現問題。不會安裝 WMA Encoder。 請造訪 http://www.microsoft.com/windows/windowsmedia/9series/encoder/，下載編碼器並再試一次。"
${LangFileString} secCDDA "CD 播放和擷取"
${LangFileString} msgCDError "安裝元件時出現問題。CD 擷取/燒錄可能未正常運作。"
${LangFileString} secCDDB "辨識 CD 的 CDDB"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"

${LangFileString} secDSP "Signal Processor Studio 外掛程式"
${LangFileString} secWriteWAV "舊式 WAV 寫入器"
${LangFileString} secLineInput "支援線路輸入"
${LangFileString} secDirectSound "支援 DirectSound 輸出"

${LangFileString} secHotKey "支援全域快速鍵"
${LangFileString} secJmp "支援擴充的跳至指定檔案"
${LangFileString} secTray "Nullsoft 系統匣控制"

${LangFileString} msgRemoveMJUICE "是否從系統移除 MJuice 支援?$\r$\n$\r$\n除非您要在程式中使用 MJF 檔案，而非 Winamp，否則請選取 [是]。"
${LangFileString} msgNotAllFiles "未移除所有檔案。$\r$\n如果要移除檔案，請自行移除。"


${LangFileString} secNSV "Nullsoft Video (NSV)"
${LangFileString} secDSHOW "AVI/MPG"

${LangFileString} secFLV "Flash Video (FLV)"

${LangFileString} secTiny "Nullsoft Tiny Fullscreen"
${LangFileString} secAVS "Advanced Visualization Studio"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Winamp 媒體櫃"
${LangFileString} secOM "線上媒體"
${LangFileString} secWire "Podcast 目錄"
${LangFileString} secPmp "可攜式媒體播放器"
${LangFileString} secPmpIpod "支援 iPod®"
${LangFileString} secPmpCreative "支援 Creative® players"
${LangFileString} secPmpP4S "支援 Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "支援 USB 裝置"
${LangFileString} secPmpActiveSync "支援 Microsoft® ActiveSync®"

${LangFileString} sec_ML_LOCAL "本機媒體"
${LangFileString} sec_ML_PLAYLISTS "播放清單"
${LangFileString} sec_ML_DISC "CD 擷取和燒錄"
${LangFileString} sec_ML_BOOKMARKS "書籤"
${LangFileString} sec_ML_HISTORY "歷程記錄"
${LangFileString} sec_ML_NOWPLAYING "目前播放"
${LangFileString} sec_ML_RG "重新播放音量差異分析工具"
${LangFileString} sec_ML_TRANSCODE "轉碼工具"

;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "$(^NameDA) 安裝程式"
${LangFileString} IDS_SELECT_LANGUAGE  "請選擇安裝程式的語言"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"多媒體引擎"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"輸出"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"音效播放"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"音效編碼器"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"影像播放"
${LangFileString} IDS_GRP_VISUALIZATION		"視覺效果"
${LangFileString} IDS_GRP_UIEXTENSION		"使用者介面擴充"
${LangFileString} IDS_GRP_WALIB				"Winamp 程式庫"
${LangFileString} IDS_GRP_WALIB_CORE		"核心媒體櫃元件"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"支援可攜式媒體播放器"
${LangFileString} IDS_GRP_LANGUAGES 	    "語言"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"WaveOut/MME 輸出"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC 編碼"
${LangFileString} IDS_SEC_MILKDROP2             "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG		"Auto-Tagger"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"設定線上服務..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"檢查是否有另一個 Winamp 作業正在執行..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"正在開啟網際網路連線..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"正在檢查網際網路是否運作..."
${LangFileString} IDS_RUN_NOINET				"網際網路未連線"
${LangFileString} IDS_RUN_EXTRACT				"正在解壓縮"
${LangFileString} IDS_RUN_DOWNLOAD				"正在下載"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"下載完成。"
${LangFileString} IDS_RUN_DOWNLOADFAILED		"下載失敗。Ô­Òò£º"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"下載取消。"
${LangFileString} IDS_RUN_INSTALL				"正在安裝"
${LangFileString} IDS_RUN_INSTALLFIALED			"安裝失敗。"
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"未找到檔案。ÕýÔÚ¹ÀËãÏÂÝd•rég¡£"
${LangFileString} IDS_RUN_DONE					"完成。"

${LangFileString} IDS_DSP_PRESETS 	"SPS 預設"
${LangFileString} IDS_DEFAULT_SKIN	"預設面板"
${LangFileString} IDS_AVS_PRESETS	"AVS 預設"
${LangFileString} IDS_MILK_PRESETS	"MilkDrop 預設"
${LangFileString} IDS_MILK2_PRESETS	"MilkDrop2 預設"

; download
${LangFileString} IDS_DOWNLOADING	"正在下載 %s"
${LangFileString} IDS_CONNECTING	"正在連線 ..."
${LangFileString} IDS_SECOND		" (剩餘 1 秒)"
${LangFileString} IDS_MINUTE		" (剩餘 1 分鐘)"
${LangFileString} IDS_HOUR			" (剩餘 1 小時)"
${LangFileString} IDS_SECONDS		" (剩餘 %u 秒)"
${LangFileString} IDS_MINUTES		" (剩餘 %u 分鐘)"
${LangFileString} IDS_HOURS			" (剩餘 %u 小時)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%) 共 %skB，速度為 %u.%01ukB /秒"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"安裝完成"
${LangFileString} IDS_PAGE_FINISH_TEXT		"已在電腦中安裝 $(^NameDA)。$\r$\n$\r$\n\
													按一下 [完成] 以關閉精靈。"
${LangFileString} IDS_PAGE_FINISH_RUN		"於安裝程式關閉後啟動 $(^NameDA)"
${LangFileString} IDS_PAGE_FINISH_LINK		"按一下此處以造訪 Winamp.com"

; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE		"歡迎使用 $(^NameDA) 安裝程式"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"您即將體驗最優秀的媒體管理員以及更多其他功能。$\r$\n$\r$\n\
													享受您喜愛的音樂、影像及廣播。擷取和燒錄 CD、建立播放清單、\
同步至您的可攜式音樂播放器，並與朋友分享。體驗上千個廣播電台與新音樂、\
影像、藝人介紹與更多內容。$\r$\n  \
													•  新 Bento 面板$\r$\n  \
													•  支援多種音樂裝置，包括 iPod®$\r$\n  \
													•  支援專輯封面 - Winamp 負責，您無須費力!$\r$\n  \
													•  透過媒體監視器，在網路上享受最棒的音樂$\r$\n  \
													•  動態歌曲推薦$\r$\n  \
													•  MP3 環繞音效支援$\r$\n  \
													•  可存取並分享遠距音樂與影像$\r$\n  \
													•  Auto-tag 功能可將音樂分類管理"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"請注意：為了讓您享受全新功能 \
															Bento 面板設計 (建議使用)，必須 \
															檢查所有元件。"

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"選擇開始選項"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"請選取下列開始選項。"
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"請選擇下列選項以設定您的 Winamp 開始選項。"
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"建立開始功能表項目"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"建立快速啟動圖示"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"建立桌面圖示"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"無法關閉 Winamp Agent。$\r$\n\
請確認無其他使用者登入 Windows。\
$\r$\n$\r$\n	關閉 WinampAgent 後，選取 [重試]。\
$\r$\n$\r$\n	如果仍然要繼續安裝，請選取 [忽略]。\
$\r$\n$\r$\n	若要中止安裝程序，請選取 [中止]。"

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"不再支援此版 Windows。$\r$\n\
$(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} 最低系統要求為 Windows 2000 或更高版本。"

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Winamp 核心 (要求使用)"
${LangFileString} IDS_SEC_AGENT_DESC			"Winamp Agent，可快速存取系統匣並維持檔案類型關聯性"
${LangFileString} IDS_GRP_MMEDIA_DESC			"多媒體引擎 (輸入/輸出系統)"
${LangFileString} IDS_SEC_CDDB_DESC				"支援 CDDB，可自動從線上 Gracenote 資料庫提取 CD 標題"
${LangFileString} IDS_SEC_DSP_DESC				"DSP 外掛程式，可套用合音、失真音、節拍與音高控制等效果"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"支援音樂播放 (輸入外掛程式：ÒôÐ§½â´aÆ÷)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"支援 ²¥•Å MP3¡¢MP2¡¢MP1¼°AAC ¸ñÊ½ (ÒªÇóÊ¹ÓÃ)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"支援 ²¥•Å WMA ¸ñÊ½ (°üÀ¨Ö§Ô® DRM)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"支援播放 MIDI 格式 (MID、RMI、KAR、MUS、CMF 及更多)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"支援播放 Module 格式 (MOD、XM、IT、S3M、ULT 及更多)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"支援播放 Ogg Vorbis 格式 (OGG)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"支援播放 MPEG-4 音樂格式 (MP4、M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"支援播放 FLAC 格式"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"支援播放 Audio CD"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"支援播放 Waveform 格式 (WAV、VOC、AU、AIFF 及更多)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"支援影像播放 (輸入外掛程式：Ó°Ïñ½â´aÆ÷)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"支援播放 Windows Media 影像格式 (WMV、ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"支援播放 Nullsoft 影像格式 (NSV、NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC			"支援播放 AVI 及 MPEG 影像格式"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"支援播放 vp6 Flash Video (FLV)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"支援編碼與轉碼 (CD 擷取以及從一個檔案格式轉換成另一檔案格式必備功能)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"支援擷取與轉碼至 WMA 格式"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"支援擷取與轉碼至 WAV 格式"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"支援擷取與轉碼至 M4A 及 AAC 格式"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"支援擷取與轉碼至 FLAC 格式"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"支援擷取與轉碼至 Ogg Vorbis 格式"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"輸出外掛程式 (可控制音效處理過程及其傳送至音效卡的過程)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"舊式 WAV/MME 寫入器 (不建議使用，但某些使用者仍偏好此功能勝於編碼器外掛程式)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"DirectSound 輸出 (要求使用/預設輸出外掛程式)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"舊式 WaveOut 輸出 (可選，不再建議或要求使用)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"使用者介面擴充"
${LangFileString} IDS_SEC_HOTKEY_DESC			"全域快速鍵外掛程式，可在專注於其他應用程式時透過鍵盤控制 Winamp"
${LangFileString} IDS_SEC_JUMPEX_DESC			"擴充的跳至指定檔案支援，可排列播放清單中的曲目並提供更多功能"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Nullsoft 系統匣控制，可在系統匣中加入播放控制圖示"
${LangFileString} IDS_SEC_FREEFORM_DESC			"新式面板支援，使用例如 Winamp Modern 及 Bento 等手繪面板必備功能"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"視覺效果外掛程式"
${LangFileString} IDS_SEC_NSFS_DESC				"Nullsoft Tiny Fullscreen 視覺效果外掛程式"
${LangFileString} IDS_SEC_AVS_DESC				"Advanced Visualization Studio 外掛程式 (預設視覺效果外掛程式)"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Milkdrop 視覺效果外掛程式"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Milkdrop2 視覺效果外掛程式"
${LangFileString} IDS_SEL_LINEIN_DESC			"線路輸入支援使用 linein:// 指令 (將視覺化檢視套用至麥克風/線路輸入)"
${LangFileString} IDS_GRP_WALIB_DESC			"Winamp 程式庫"
${LangFileString} IDS_SEC_ML_DESC				"Winamp 媒體櫃 (要求使用)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"轉碼工具，可將一個檔案格式轉換成另一檔案格式"
${LangFileString} IDS_SEC_ML_RG_DESC			"重新播放音量差異分析工具，可用於音量分級"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Winamp Auto-Tagger (由 Gracenote 提供)，可補足遺失的詮釋資料"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Podcast 目錄，可訂閱並下載網路廣播"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"線上服務，包括 SHOUTcast Radio、&& TV、In2TV、AOL Videos 及 XM Radio 串流"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Winamp 播放清單產生器 (由 Gracenote 提供)，可建立動態音響播放清單"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"核心媒體櫃元件"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"本機媒體資料庫，提供強大排列系統與自訂智慧檢視畫面"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"播放清單管理員，可建立、編輯並儲存所有播放清單"
${LangFileString} IDS_SEC_ML_DISC_DESC			"CD 擷取與燒錄，提供擷取與燒錄音樂 CD 的媒體櫃介面"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"書籤管理員，可標示您喜愛的串流、檔案或資料夾"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"歷程紀錄，可即時存取所有最近播放的本機或遠距檔案及串流"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"目前播放，可檢視目前播放曲目資訊"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"可攜式媒體播放器支援"
${LangFileString} IDS_SEC_ML_PMP_DESC			"支援核心可攜式媒體播放器之外掛程式 (要求使用)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"支援 iPod®"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"支援 Creative® 可攜式裝置 (可管理 Nomad™、Zen™ 及 MuVo™ 播放器)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"支援 Microsoft® PlaysForSure® (可管理所有 P4S 相容播放器)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"支援 USB 裝置 (可管理一般 usb 隨身碟及播放器)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"支援 Microsoft® ActiveSync® (可管理 Windows Mobile®、Smartphone 及 Pocket PC 裝置)"
