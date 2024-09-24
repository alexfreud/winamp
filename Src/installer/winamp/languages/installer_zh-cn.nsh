; Language-Country:	ZH-CN
; LangId:			?
; CodePage:			?
; Revision:			1
; Last udpdated:	12.05.2008
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
; 12.05 > barabanger:  integrated in build system.


!insertmacro LANGFILE_EXT "SimpChinese"
 
${LangFileString} installFull "完全"
${LangFileString} installStandard "标准"
${LangFileString} installLite "精简"
${LangFileString} installMinimal "最小"
${LangFileString} installPrevious "上一次安装"

${LangFileString} installWinampTop "这将安装 Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}。"
${LangFileString} installerContainsFull "此安装程序包含完全安装。"
${LangFileString} installerContainsLite "此安装程序包含精简安装。"
${LangFileString} licenseTop "请在安装之前阅读并同意下面的许可条款。"
${LangFileString} directoryTop "安装程序已为 $(^NameDA)确定了最佳位置。如果要更改此文件夹，现在即可更改。"

${LangFileString} uninstallPrompt "这将卸载 Winamp。是否继续?"

${LangFileString} msgCancelInstall "是否取消安装?"
${LangFileString} msgReboot "需要重新启动才能完成安装。$\r$\n是否立即重新启动? (如果要以后再重新启动，请选择“否”)"
${LangFileString} msgCloseWinamp "必须关闭 Winamp 才能继续。$\r$\n$\r$\n	在关闭 Winamp 后，请选择“重试”。$\r$\n$\r$\n	如果要尝试继续安装，请选择“忽略”。$\r$\n$\r$\n	如果要中止安装过程，请选择“中止”。"
${LangFileString} msgInstallAborted "用户已中止安装"

${LangFileString} secWinamp "Winamp (必需)"
${LangFileString} compAgent "Winamp Agent"
${LangFileString} compModernSkin "现代面板支持"
${LangFileString} uninstallWinamp "卸载 Winamp"

${LangFileString} secWMA "Windows Media 音频(WMA)"
${LangFileString} secWMV "Windows Media 视频(WMV、ASF)"
${LangFileString} secWMFDist "下载并安装 Windows Media Format"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "OGG Vorbis 播放"
${LangFileString} secOGGEnc "OGG Vorbis 编码"
${LangFileString} secAACE "AAC/aacPlus 编码"
${LangFileString} secMP3E "MP3 编码"
${LangFileString} secMP4E "MP4 支持"
${LangFileString} secWMAE "WMA 编码"
${LangFileString} msgWMAError "安装组件时出现问题。将不会安装 WMA 编码器。请访问 http://www.microsoft.com/windows/windowsmedia/9series/encoder/，下载该编码器并重试。"
${LangFileString} secCDDA "CD 播放和提取"
${LangFileString} msgCDError "安装组件时出现问题。CD 翻录/刻录可能无法正常运行。"
${LangFileString} secCDDB "用于识别 CD 的 CDDB"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"

${LangFileString} secDSP "Signal Processor Studio 插件"
${LangFileString} secWriteWAV "旧式 WAV 写入程序"
${LangFileString} secLineInput "线路输入支持"
${LangFileString} secDirectSound "DirectSound 输出支持"

${LangFileString} secHotKey "全局热键支持"
${LangFileString} secJmp "“跳转到文件”扩展支持"
${LangFileString} secTray "Nullsoft 托盘控制"

${LangFileString} msgRemoveMJUICE "是否从您的系统中移除 MJuice 支持?$\r$\n$\r$\n除非您在除 Winamp 之外的程序中使用 MJF 文件，否则选择“是”。"
${LangFileString} msgNotAllFiles "尚未移除某些文件。$\r$\n如果您要亲自移除这些文件，请移除。"


${LangFileString} secNSV "Nullsoft 视频(NSV)"
${LangFileString} secDSHOW "AVI/MPG"

${LangFileString} secFLV "Flash 视频(FLV)"

${LangFileString} secTiny "Nullsoft 微型全屏"
${LangFileString} secAVS "高级可视化效果工作室"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Winamp 媒体库"
${LangFileString} secOM "在线媒体"
${LangFileString} secWire "播客目录"
${LangFileString} secPmp "便携式媒体播放器"
${LangFileString} secPmpIpod "iPod® 支持"
${LangFileString} secPmpCreative "支持 Creative® 播放器"
${LangFileString} secPmpP4S "支持 Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "USB 设备支持"
${LangFileString} secPmpActiveSync "支持 Microsoft® ActiveSync®"

${LangFileString} sec_ML_LOCAL "本地媒体"
${LangFileString} sec_ML_PLAYLISTS "播放列表"
${LangFileString} sec_ML_DISC "CD 翻录和刻录"
${LangFileString} sec_ML_BOOKMARKS "书签"
${LangFileString} sec_ML_HISTORY "历史"
${LangFileString} sec_ML_NOWPLAYING "正在播放"
${LangFileString} sec_ML_RG "回放增益分析工具"
${LangFileString} sec_ML_TRANSCODE "编码转换工具"

;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "$(^NameDA) 安装程序"
${LangFileString} IDS_SELECT_LANGUAGE  "请选择安装程序的语言"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"多媒体引擎"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"输出"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"音频播放"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"音频编码器"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"视频播放"
${LangFileString} IDS_GRP_VISUALIZATION		"可视化效果"
${LangFileString} IDS_GRP_UIEXTENSION		"用户界面扩展"
${LangFileString} IDS_GRP_WALIB				"Winamp 媒体库"
${LangFileString} IDS_GRP_WALIB_CORE		"核心媒体库组件"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"便携式媒体播放器支持"
${LangFileString} IDS_GRP_LANGUAGES 	    "语言"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"WaveOut/MME 输出"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC 编码"
${LangFileString} IDS_SEC_MILKDROP2             "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG		"自动标记"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"正在配置在线服务..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"正在检查是否有 Winamp 的另一个实例正在运行..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"正在打开 Internet 连接..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"正在检查 Internet 是否可用..."
${LangFileString} IDS_RUN_NOINET				"没有 Internet 连接"
${LangFileString} IDS_RUN_EXTRACT				"正在解压缩"
${LangFileString} IDS_RUN_DOWNLOAD				"正在下载"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"下载已完成。"
${LangFileString} IDS_RUN_DOWNLOADFAILED		"下载失败。原因:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"下载已取消。"
${LangFileString} IDS_RUN_INSTALL				"正在安装"
${LangFileString} IDS_RUN_INSTALLFIALED			"安装失败。"
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"找不到文件。正在计划下载。"
${LangFileString} IDS_RUN_DONE					"完成。"

${LangFileString} IDS_DSP_PRESETS 	"SPS 预设"
${LangFileString} IDS_DEFAULT_SKIN	"默认面板"
${LangFileString} IDS_AVS_PRESETS	"AVS 预设"
${LangFileString} IDS_MILK_PRESETS	"MilkDrop 预设"
${LangFileString} IDS_MILK2_PRESETS	"MilkDrop2 预设"

; download
${LangFileString} IDS_DOWNLOADING	"正在下载 %s"
${LangFileString} IDS_CONNECTING	"正在连接 ..."
${LangFileString} IDS_SECOND		" (剩余 1 秒)"
${LangFileString} IDS_MINUTE		" (剩余 1 分钟)"
${LangFileString} IDS_HOUR			" (剩余 1 小时)"
${LangFileString} IDS_SECONDS		" (剩余 %u 秒)"
${LangFileString} IDS_MINUTES		" (剩余 %u 分钟)"
${LangFileString} IDS_HOURS			" (剩余 %u 小时)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%)，共 %skB，速度为 %u.%01ukB/秒"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"安装完成"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) 已安装在您的计算机上。$\r$\n$\r$\n\
													单击“完成”以关闭此向导。"
${LangFileString} IDS_PAGE_FINISH_RUN		"在安装程序关闭后启动 $(^NameDA)"
${LangFileString} IDS_PAGE_FINISH_LINK		"单击此处以访问 Winamp.com"

; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE		"欢迎使用 $(^NameDA) 安装程序"
${LangFileString} IDS_PAGE_WELCOME_TEXT		"准备好体验最佳媒体管理器以及许多其他功能。$\r$\n$\r$\n\
													欣赏您最喜欢的音乐、视频和播客。翻录和刻录 CD，创建播放列表，\
与您的便携音乐播放器同步，以及与朋友分享。从上千个无线电台、\
视频、艺术家评论等内容中发现新音乐。$\r$\n  \
													•  新 Bento 面板$\r$\n  \
													•  多种音乐设备支持，包括 iPod®$\r$\n  \
													•  专辑画面支持 - Winamp 发现了它，您享用它!$\r$\n  \
													•  使用媒体监视器欣赏 Web 上最好的音乐$\r$\n  \
													•  动态歌曲推荐$\r$\n  \
													•  MP3 环绕声支持$\r$\n  \
													•  远程音乐和视频访问及共享$\r$\n  \
													•  自动标记功能对音乐进行分类"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"注意: 要享用 Bento 面板(推荐)的 \
															新功能和设计，必须选中 \
															所有组件。"

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"选择启动选项"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"从以下启动选项中选择。"
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"从以下选项中选择以配置您的 Winamp 启动选项。"
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"创建“开始”菜单项"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"创建快速启动图标"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"创建桌面图标"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"无法关闭 Winamp Agent。$\r$\n\
确保没有其他用户登录到 Windows。\
$\r$\n$\r$\n	关闭 WinampAgent 后，选择“重试”。\
$\r$\n$\r$\n	如果要尝试继续安装，请选择“忽略”。\
$\r$\n$\r$\n	如果要中止安装过程，请选择“中止”。"

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"不再支持此版本的 Windows。$\r$\n\
$(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} 要求的最低版本为 Windows 2000 或更高版本。"

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Winamp 核心(必需)"
${LangFileString} IDS_SEC_AGENT_DESC			"Winamp Agent，用于快速系统托盘访问和维护文件类型关联"
${LangFileString} IDS_GRP_MMEDIA_DESC			"多媒体引擎(输入/输出系统)"
${LangFileString} IDS_SEC_CDDB_DESC				"CDDB 支持，用于自动从在线 Gracenote 数据库获取 CD 曲目"
${LangFileString} IDS_SEC_DSP_DESC				"DSP 插件，用于应用附加效果，例如合声、除雪器、拍子和音调控制"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"音频播放支持(输入插件: 音频解码器)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"支持播放 MP3、MP2、MP1、AAC 格式(必需)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"支持播放 WMA 格式(包括 DRM 支持)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"支持播放 MIDI 格式(MID、RMI、KAR、MUS、CMF 等)"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"支持播放 Module 格式(MOD、XM、IT、S3M、ULT 等)"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"支持播放 Ogg Vorbis 格式(OGG)"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"支持播放 MPEG-4 音频格式(MP4、M4A)"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"支持播放 FLAC 格式"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"支持播放音频 CD"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"支持播放 Waveform 格式(WAV、VOC、AU、AIFF 等)"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"视频播放支持(输入插件: 视频解码器)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"支持播放 Windows Media 视频格式(WMV、ASF)"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"支持播放 Nullsoft 视频格式(NSV、NSA)"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC			"支持播放 AVI 和 MPEG 视频格式"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"支持播放 vp6 Flash 视频(FLV)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"编码和代码转换支持(翻录 CD 和将 CD 从一种文件格式转换为另一种文件格式所必需的)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"支持翻录和代码转换为 WMA 格式"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"支持翻录和代码转换为 WAV 格式"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"支持翻录和代码转换为 M4A 和 AAC 格式"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"支持翻录和代码转换为 FLAC 格式"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"支持翻录和代码转换为 Ogg Vorbis 格式"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"输出插件(控制如何处理音频以及如何将音频发送到您的声卡)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"旧式 WAV/MME 写入程序(不推荐使用，但一些用户仍喜欢使用它，而不是编码器插件)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"DirectSound 输出(必需/默认输出插件)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"旧式 WaveOut 输出(可选，不再推荐或必须使用)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"用户界面扩展"
${LangFileString} IDS_SEC_HOTKEY_DESC			"全局热键插件，用于在其他应用程序处于焦点时使用键盘控制 Winamp"
${LangFileString} IDS_SEC_JUMPEX_DESC			"“跳转到文件”扩展支持，用于在播放列表中排列歌曲以及许多其他功能"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Nullsoft 托盘控制插件，用于在系统托盘中添加“播放”控制图标"
${LangFileString} IDS_SEC_FREEFORM_DESC			"现代面板支持，使用任意形状的面板(例如 Winamp 现代和 Bento)所必需的"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"可视化效果插件"
${LangFileString} IDS_SEC_NSFS_DESC				"Nullsoft 微型全屏可视化效果插件"
${LangFileString} IDS_SEC_AVS_DESC				"高级可视化效果工作室插件(默认可视化效果插件)"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Milkdrop 可视化效果插件"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Milkdrop2 可视化效果插件"
${LangFileString} IDS_SEL_LINEIN_DESC			"线路输入支持使用 linein:// 命令(对 mic/线路输入应用可视化工具)"
${LangFileString} IDS_GRP_WALIB_DESC			"Winamp 媒体库"
${LangFileString} IDS_SEC_ML_DESC				"Winamp 媒体库(必需)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"代码转换工具，用于从一种文件格式转换为另一种文件格式"
${LangFileString} IDS_SEC_ML_RG_DESC			"回放增益分析工具，用于设置音量级别"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Winamp 自动标记(由 Gracenote 提供技术支持)，用于填入缺少的元数据"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"播客目录，用于订阅和下载播客"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"在线服务，包括 SHOUTcast 广播和电视、In2TV、AOL 视频及 XM 广播流"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Winamp 播放列表生成器(由 Gracenote 提供技术支持)，用于创建声音动态播放列表"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"核心媒体库组件"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"本地媒体数据库，具有功能强大的查询系统和自定义智能视图"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"播放列表管理器，用于创建、编辑和存储您的所有播放列表"
${LangFileString} IDS_SEC_ML_DISC_DESC			"CD 翻录和刻录，用于翻录和刻录音频 CD 的媒体库界面"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"书签管理器，用于对您最喜欢的流、文件或文件夹设置书签"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"历史记录，用于即时访问所有最近播放的本地或远程文件和流"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"正在播放，用于查看有关当前播放的曲目的信息"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"便携式媒体播放器支持"
${LangFileString} IDS_SEC_ML_PMP_DESC			"核心便携式媒体播放器支持插件(必需)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"iPod® 支持"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"支持 Creative® 便携设备(用于管理 Nomad™、Zen™ 和 MuVo™ 播放器)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"支持 Microsoft® PlaysForSure® (用于管理所有 P4S 兼容播放器)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"USB 设备支持(用于管理通用 USB 拇指驱动器和播放器)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"支持 Microsoft® ActiveSync® (用于管理 Windows Mobile®、Smartphone 和 Pocket PC 设备)"
