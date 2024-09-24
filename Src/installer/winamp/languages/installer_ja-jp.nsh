; Language-Country:	JA-JP
; LangId:			1041
; CodePage:			932
; Revision:			14
; Last udpdated:	01.12.2013
; Author:			T-Matsuo(win32lab.com)
; Email:			tms3@win32lab.com

!insertmacro LANGFILE_EXT "Japanese"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "インストーラ言語"
${LangFileString} LANGUAGE_DLL_INFO "言語を選択してください。"

${LangFileString} installFull "全て(Full)"
${LangFileString} installStandard "標準(Std)"
${LangFileString} installLite "軽量(Lite)"
${LangFileString} installMinimal "最小"
${LangFileString} installPrevious "前回のインストール構成"

; BrandingText
${LangFileString} BuiltOn "ビルド日時"
${LangFileString} at ","

${LangFileString} installWinampTop "Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType} をインストールします。"
${LangFileString} installerContainsFull "このインストーラには全て(Full)版が含まれています。"
${LangFileString} installerContainsLite "このインストーラには軽量(Lite)版が含まれています。"
${LangFileString} licenseTop "Winampをインストールする前に、ライセンス条件を確認してください。"
${LangFileString} directoryTop "インストーラによって $(^NameDA) に最適な場所が決定されました。フォルダを変更する場合は、ここで変更してください。"

${LangFileString} uninstallPrompt "この操作は Winamp をアンインストールします。続けますか？"

${LangFileString} msgCancelInstall "インストールを中止しますか？"
${LangFileString} msgReboot "インストールを完了するにはコンピュータを再起動する必要があります。$\r$\n今すぐ再起動しますか？(後で再起動する場合は[いいえ]を選んでください)"
${LangFileString} msgCloseWinamp "操作を続行するには Winamp を閉じる必要があります。$\r$\n$\r$\n	Winamp を閉じたあと、[再試行]をクリックしてください。$\r$\n$\r$\n	インストール作業を強行するなら、[無視]をクリックしてください。$\r$\n$\r$\n	インストール作業を中止するなら、[中止]をクリックしてください。"
${LangFileString} msgInstallAborted "インストールはユーザによって中止されました"

${LangFileString} secWinamp "Winamp (必須)"
${LangFileString} compAgent "Winamp エージェント"
${LangFileString} compModernSkin "モダン スキン サポート"
${LangFileString} safeMode "Winamp (セーフモード)"
${LangFileString} uninstallWinamp "Winamp のアンインストール"

${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV, ASF)"
${LangFileString} secWMFDist "Windows Media Format のダウンロードとインストール"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "OGG Vorbis 再生"
${LangFileString} secOGGEnc "OGG Vorbis エンコード"
${LangFileString} secAACE "HE-AAC エンコード"
${LangFileString} secMP3E "MP3 エンコード"
${LangFileString} secMP4E "MP4 サポート"
${LangFileString} secWMAE "WMA エンコード"
${LangFileString} msgWMAError "コンポーネントのインストール中に問題が発生しました。WMA エンコーダはインストールされませんでした。http://www.microsoft.com/windows/windowsmedia/9series/encoder/ を参照してエンコーダをダウンロードしてもう一度試してください。"
${LangFileString} secCDDA "CDの再生とリッピング"
${LangFileString} msgCDError "コンポーネントのインストール中に問題が発生しました。リッピング/CD書き込みはきちんと機能しないかもしれません。"
${LangFileString} secCDDB "CD識別のためのCDDB"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"

${LangFileString} secDSP "Signal Processor Studio プラグイン"
${LangFileString} secWriteWAV "旧バージョンの WAV ライター"
${LangFileString} secLineInput "ライン入力 サポート"
${LangFileString} secDirectSound "DirectSound 出力サポート"

${LangFileString} secHotKey "グローバル ホットキーのサポート"
${LangFileString} secJmp "拡張ジャンプ先ファイル機能のサポート"
${LangFileString} secTray "Nullsoft トレイコントロール"

${LangFileString} msgRemoveMJUICE "あなたのシステムから MJuice サポートを取り除きますか？$\r$\n$\r$\nWinamp以外のプログラムでMJFファイルを使用しない場合に限り、[はい]を選んでください。"
${LangFileString} msgNotAllFiles "全てのファイルを削除できませんでした。$\r$\n残ったファイルはユーザ自身の手によって削除を行ってください。"


${LangFileString} secNSV "Nullsoft ビデオ (NSV)"
${LangFileString} secDSHOW "DirectShow フォーマット (MPG, M2V)"
${LangFileString} secAVI "AVI ビデオ"
${LangFileString} secFLV "Flash ビデオ (FLV)"

${LangFileString} secMKV "Matroska (MKV, MKA)"
${LangFileString} secM4V "MPEG-4 ビデオ (MP4, M4V)"

${LangFileString} secSWF "Flash メディア プロトコル (SWF, RTMP)"

${LangFileString} secTiny "Nullsoft Tiny Fullscreen"
${LangFileString} secAVS "Advanced Visualization Studio"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Winamp メディア ライブラリ"
${LangFileString} secOM "オンライン メディア"
${LangFileString} secWire "ポッドキャスト ディレクトリ"
${LangFileString} secPmp "ポータブル メディア プレイヤー"
${LangFileString} secPmpIpod "iPod(R) のサポート"
${LangFileString} secPmpCreative "Creative(R) プレイヤーのサポート"
${LangFileString} secPmpP4S "Microsoft(R) PlaysForSure(R) のサポート"
${LangFileString} secPmpUSB "USB デバイスのサポート"
${LangFileString} secPmpActiveSync "Microsoft(R) ActiveSync(R) のサポート"
${LangFileString} secPmpAndroid "Android デバイスのサポート"
${LangFileString} secPmpWifi "Android Wifi のサポート"

${LangFileString} sec_ML_LOCAL "ローカル メディア"
${LangFileString} sec_ML_PLAYLISTS "プレイリスト"
${LangFileString} sec_ML_DISC "CDリッピングと書き込み"
${LangFileString} sec_ML_BOOKMARKS "ブックマーク"
${LangFileString} sec_ML_HISTORY "履歴"
${LangFileString} sec_ML_NOWPLAYING "現在再生中"
${LangFileString} sec_ML_RG "リプレイゲイン解析ツール"
${LangFileString} sec_ML_TRANSCODE "トランスコードツール"
${LangFileString} sec_ML_PLG "プレイリスト生成器"
${LangFileString} sec_ML_IMPEX "iTunes インポートツール"

;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "$(^NameDA) インストーラ"
${LangFileString} IDS_SELECT_LANGUAGE  "インストーラの言語を選択してください"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"マルチメディア エンジン"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"出力"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"オーディオ再生"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"オーディオエンコーダ"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"ビデオ再生"
${LangFileString} IDS_GRP_VISUALIZATION		"視覚エフェクト"
${LangFileString} IDS_GRP_UIEXTENSION		"ユーザインタフェース拡張"
${LangFileString} IDS_GRP_WALIB				"Winampライブラリ"
${LangFileString} IDS_GRP_WALIB_CORE		"コア メディアライブラリ コンポーネント"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"ポータブルメディアプレイヤーのサポート"
${LangFileString} IDS_GRP_LANGUAGES 	    "言語"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"WaveOut/MME 出力"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC エンコード"
${LangFileString} IDS_SEC_MILKDROP2             "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG		"自動タグ付け"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO "フランスのラジオプラグイン"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"オンラインサービスの設定..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Winamp の別のインスタンスが実行しているか確認しています..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"インターネット接続を開いています..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"インターネットが利用できるかチェックしています..."
${LangFileString} IDS_RUN_NOINET				"インターネット接続が利用できません"
${LangFileString} IDS_RUN_EXTRACT				"抽出中"
${LangFileString} IDS_RUN_DOWNLOAD				"ダウンロード中"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"ダウンロード完了。"
${LangFileString} IDS_RUN_DOWNLOADFAILED		"ダウンロード失敗。 原因:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"ダウンロードはキャンセルされました。"
${LangFileString} IDS_RUN_INSTALL				"インストール中"
${LangFileString} IDS_RUN_INSTALLFIALED			"インストール失敗。"
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"ファイルが見つかりません。ダウンロードをスケジューリングします。"
${LangFileString} IDS_RUN_DONE					"完了."

${LangFileString} IDS_DSP_PRESETS 	"SPS プリセット"
${LangFileString} IDS_DEFAULT_SKIN	"デフォルトスキン"
${LangFileString} IDS_AVS_PRESETS	"AVS プリセット"
${LangFileString} IDS_MILK_PRESETS	"MilkDrop プリセット"
${LangFileString} IDS_MILK2_PRESETS	"MilkDrop2 プリセット"

${LangFileString} IDS_CLEANUP_PLUGINS	"プラグインのクリーンアップ..."
${LangFileString} IDS_REMOVE_SKINS		"デフォルトスキンの削除..."


; download
${LangFileString} IDS_DOWNLOADING	"ダウンロード中"
${LangFileString} IDS_CONNECTING	"接続中 ..."
${LangFileString} IDS_SECOND		" (1 秒の残り時間)"
${LangFileString} IDS_MINUTE		" (1 分の残り時間)"
${LangFileString} IDS_HOUR			" (1 時間の残り時間)"
${LangFileString} IDS_SECONDS		" (%u 秒の残り時間)"
${LangFileString} IDS_MINUTES		" (%u 分の残り時間)"
${LangFileString} IDS_HOURS			" (%u 時間の残り時間)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%) of %skB @ %u.%01ukB/s"

; AutoplayHandler
${LangFileString} AutoplayHandler	"再生"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"インストール完了"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) がコンピュータにインストールされました。$\r$\n$\r$\n\
													[完了] をクリックしてウィザードを終了してください。"
${LangFileString} IDS_PAGE_FINISH_RUN		"インストーラを閉じた後に $(^NameDA) を起動します"
${LangFileString} IDS_PAGE_FINISH_LINK		"ここをクリックすると Winamp.com に接続します"


; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE		"$(^NameDA) インストーラへようこそ"

!ifdef EXPRESS_MODE
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) はメディアライブラリを管理し、インターネットラジオを聴くことができます。 \
											Winamp クラウドを使用すれば、車の中、職場、家庭、任意の場所で聞くことができます。$\r$\n$\r$\n\
											含まれる機能:$\r$\n$\r$\n  \
												•  Winamp クラウドで手持ちの音楽ライブラリを便利に活用$\r$\n$\r$\n  \
												•  すべてのデバイス間で音楽ライブラリ全体を管理$\r$\n$\r$\n  \
												•  Android版Winampとワイヤレス同期$\r$\n$\r$\n  \
													・自動プレイリストジェネレータを使ってプレイリストを作成$\r$\n$\r$\n  \
													・3万以上のポッドキャスト"
											
${LangFileString} IDS_PAGE_WELCOME_LEGAL	"「次へ」をクリックして $(^NameDA) の<a id=$\"winamp_eula$\">使用権許諾契約</a>と<a id=$\"winamp_privacy_policy$\">プライバシーポリシー</a>に同意して下さい。"
!else
${LangFileString} IDS_PAGE_WELCOME_TEXT		"$(^NameDA) はメディアライブラリを管理し、インターネットラジオを聴くことができます。$\r$\n$\r$\n\
											含まれる機能:$\r$\n$\r$\n  \
												•  Android版Winampとワイヤレス同期$\r$\n$\r$\n  \
												•  自動プレイリストジェネレータを使ってプレイリストを作成$\r$\n$\r$\n  \
												•  3万以上のポッドキャスト"

!endif ; defined (EXPRESS_MODE)


; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"補足: Bento スキン (推奨)の新しい機能、およびデザインを楽しむには、\
															全てのコンポーネントをチェックする必要があります。"

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"スタートオプションの選択"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"以下のスタートオプションから選択してください。"
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"以下のオプションから選択して Winamp スタートオプションを設定してください。"
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"スタートメニュー項目の作成"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"クイック起動アイコンの作成"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"デスクトップアイコンの作成"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Winamp エージェント を閉じることができません。$\r$\n\
                                                   別のユーザが Windows にログインしていないか確認してください。\
                                                   $\r$\n$\r$\n	Winamp エージェント を閉じた後に[再試行]を選択してください。\
                                                   $\r$\n$\r$\n	インストールを強行するのであれば、[無視]を選択してください。\
                                                   $\r$\n$\r$\n	インストールを中止する場合は[中止]を選択してください。"

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"このバージョンの Windows は既にサポートされていません。$\r$\n\
                                                 $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} はWindowsXP かそれよりも新しい Windows を必要とします。"


; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"サードパーティー製の非互換プラグイン gen_msn7.dll を検出しました！$\r$\n$\r$\nこのプラグインは Winamp 5.57 とそれ以降でのロード時にクラッシュします。$\r$\nこのプラグインを削除します。続行するには OK をクリックしてください。"

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"非互換のサードパーティ gen_lyrics.dll プラグインを検出しました！$\r$\n$\r$\nこのプラグインは Winamp 5.59 以降はロード時にクラッシュします。$\r$\nプラグインを無効化します。OK をクリックしてください。"
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "サードパーティ gen_lyrics.dll プラグインを検出しました！$\r$\n$\r$\nこのプラグインの古いバージョンは、Winamp 5.6とそれ以降では動作しません。 進める前に、 http://lyricsplugin.com から最新版を必ず持ってください。"
${LangFileString} IDS_LYRICS_IE_PLUGIN_DISABLE	"互換性のないサードパーティ gen_lyrics_ie.dll プラグインが検出されました！$\r$\n$\r$\nこのプラグインが Winamp がクラッシュする原因となります。$\r$\nプラグインは無効化されました。OKをクリックすると続行します。"

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
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"Your computer missing ${DIRECTXINSTAL_DIRECTXNAME} component required by ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Unable to download missing ${DIRECTXINSTAL_DIRECTXNAME} component"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Unable to install missing ${DIRECTXINSTAL_DIRECTXNAME} component"

;French Radio Section
${LangFileString} IDS_FRENCHRADIO_INSTALLING			"$(IDS_SEC_GEN_FRENCHRADIO) をインストール中..."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Winamp コア (必須)"
${LangFileString} IDS_SEC_AGENT_DESC			"Winamp エージェント。素早いシステムトレイへのアクセスとファイルタイプ関連付けの維持を行います。"
${LangFileString} IDS_GRP_MMEDIA_DESC			"マルチメディアエンジン (入力/出力 システム)"
${LangFileString} IDS_SEC_CDDB_DESC				"CDDB サポート。オンライン Gracenote データベースからCDタイトルを自動的に取得します。"
${LangFileString} IDS_SEC_DSP_DESC				"DSP プラグイン。コーラス、フランジャー、テンポおよびピッチコントロールなどの特殊効果を適用します"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"オーディオ再生サポート (入力プラグイン: オーディオデコーダー)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"MP3, MP2, MP1, AAC フォーマット(required)の再生サポート"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"WMA フォーマット(including DRM support)の再生サポート"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"MIDI フォーマット(MID, RMI, KAR, MUS, CMF and more)の再生サポート"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Module フォーマット(MOD, XM, IT, S3M, ULT and more)の再生サポート"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Ogg Vorbis フォーマット(OGG, OGA)の再生サポート"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"MPEG-4 オーディオフォーマット(MP4, M4A)の再生サポート"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"FLAC フォーマットの再生サポート"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Audio CDの再生サポート"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Waveform フォーマット(WAV, VOC, AU, AIFF and more)の再生サポート"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"ビデオ再生サポート (入力プラグイン: ビデオデコーダー)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Windows Media video フォーマット(WMV, ASF)の再生サポート"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Nullsoft Video フォーマット(NSV, NSA)の再生サポート"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"MPEG-1/2 とその他ビデオフォーマットの再生サポート"
${LangFileString} IDS_SEC_AVI_DEC_DESC			"AVI Video フォーマットの再生サポート"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"vp6 Flash Video (FLV)の再生サポート"
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Matroska Video (MKV)の再生サポート"
${LangFileString} IDS_SEC_M4V_DEC_DESC			"MPEG-4 Video (MP4, M4V)の再生サポート"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Adobe Flash streaming フォーマット(SWF, RTMP)の再生サポート"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"エンコーディングとトランスコーディングのサポート (CDリッピングおよびファイルフォーマット変換に必要)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"WMA フォーマットのリッピングおよびトランスコーディングのサポート"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"WAV フォーマットのリッピングおよびトランスコーディングのサポート"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"M4A および AAC フォーマットのリッピングおよびトランスコーディングのサポート"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"FLAC フォーマットのリッピングおよびトランスコーディングのサポート"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Ogg Vorbis フォーマットのリッピングおよびトランスコーディングのサポート"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"出力プラグイン (オーディオの処理とサウンドカードへの送信方法をコントロールします)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"旧バージョンのWAV/MMEライター(あまり使用されませんが、Encoderプラグインの代わりに好んで使うユーザもいます)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"DirectSound 出力 (必須 / デフォルトの出力プラグイン)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"旧バージョンの WaveOut 出力 (あくまでもオプションで、推奨でも必須でもありません。)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"ユーザインタフェース拡張機能"
${LangFileString} IDS_SEC_HOTKEY_DESC			"グローバルホットキープラグイン。他のアプリケーションにフォーカスがあるときに Winamp をキーボードでコントロールします。"
${LangFileString} IDS_SEC_JUMPEX_DESC			"拡張ジャンプ先ファイル機能のサポート。曲をプレイリストに並べるほか、たくさんの機能があります。"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Nullsoft トレイコントロールプラグイン。再生コントロール アイコンをシステムトレイに追加します。"
${LangFileString} IDS_SEC_FREEFORM_DESC			"モダンスキンサポート。 Winamp Moderun および Bento 等のフリーフォームスキンを使用するときに必要です。"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"視覚エフェクトプラグイン"
${LangFileString} IDS_SEC_NSFS_DESC				"Nullsoft Tiny Fullscreen 視覚エフェクトプラグイン"
${LangFileString} IDS_SEC_AVS_DESC				"Advanced Visualization Studio プラグイン"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Milkdrop 視覚エフェクトプラグイン"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Milkdrop2 視覚エフェクトプラグイン (デフォルト vis プラグイン)"
${LangFileString} IDS_SEL_LINEIN_DESC			"linein:// コマンドを使用したライン入力サポート (マイク/ライン入力にビジュアライザを適用)"
${LangFileString} IDS_GRP_WALIB_DESC			"Winamp ライブラリ"
${LangFileString} IDS_SEC_ML_DESC				"Winamp メディアライブラリ (必須)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"トランスコーディング ツール。ファイルフォーマットを別のフォーマットに変換するために使用します。"
${LangFileString} IDS_SEC_ML_RG_DESC			"リプレイゲイン解析ツール。音量レベル調整に使用します。Replay Gain Analysis Tool, used for volume-levelling"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Winamp 自動タグ付け。(Powered by Gracenote) 欠落したメタデータの穴埋めをします。"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"ポッドキャスト ディレクトリ。ポッドキャストの申し込み、およびダウンロード用です。"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"オンラインサービス。SHOUTcastラジオおよび、TV、AOLラジオ、CBS Radio, Winampチャートとその他を含みます。"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Winamp プレイリスト生成。 (powered by Gracenote) 音響的な観点から動的にプレイリストを作成します。"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"コアメディアライブラリ コンポーネント"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"ローカルメディア データベース。強力なクエリシステムおよび、カスタムスマートビューを備えています。"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"プレイリストマネージャー。プレイリストを作成、編集、および保存します。"
${LangFileString} IDS_SEC_ML_DISC_DESC			"CDリッピングと書き込み。音楽CDのリッピングと書き込み用のメディアライブラリインタフェースです。"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"ブックマークマネージャー。お気に入りストリーム、ファイルまたはフォルダをブックマークします。"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"履歴。最近再生したローカルまたはリモートファイルおよびストリームへのアクセスを簡単にします。"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"現在再生中。現在再生しているトラックに関する情報を表示します。"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"ポータブルメディアプレイヤーサポート"
${LangFileString} IDS_SEC_ML_PMP_DESC			"コア ポータブルメディアプレイヤーサポート プラグイン。(必須)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"iPod(R) サポート"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Creative(R) ポータブルのサポート (Nomad(TM)、Zen(TM)およびMuVo(TM)プレイヤーの管理用)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Microsoft(R) PlaysForSure(R) のサポート。 (すべての MTP と P4S 互換プレイヤーの管理用)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"USB デバイスのサポート (汎用 USB ドライブおよびプレイヤーの管理用)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Microsoft(R) ActiveSync(R) のサポート (Windows Mobile(R)、スマートフォンおよび Pocket PC デバイスの管理用)"
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Android デバイスのサポート"
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Android Wifi のサポート"
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"iTunes互換メディアライブラリデータベースのインポート・エクスポートプラグイン"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO_DESC	"300以上のフランスのラジオ局、$(^NameDA) の生放送 (Virgin radio, NRJ, RTL, Skyrock, RMC...)"

${LangFileString} IDS_FIREWALL					"Adding Firewall Records"

${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"コンピューターから $(^NameDA) を取り除きます。"

${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"アンインストールパス:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"メディアプレイヤー"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"サードパーティープラグインを含む全ての $(^NameDA) メディアプレイヤーコンポーネントをアンインストールします。"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"ユーザー設定"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"全ての $(^NameDA) 設定とプラグイン。"

${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"フィードバックを送って $(^NameDA) を助けてください"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"$(^NameDA) フォルダーを開く"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\n補足:  このアンインストールでは全てのファイルが削除されませんでした。確認するには Winamp フォルダを開いてください。"
${LangFileString} IDS_UNINSTALL_SUBHEADER				"$(^NameDA) はお使いのコンピュータからアンインストールされました。$\r$\n$\r$\n閉じるには「完了」をクリックします。"

!ifdef EXPRESS_MODE
${LangFileString} IDS_EXPRESS_MODE_HEADER "$(^NameDA) インストールモード"
${LangFileString} IDS_EXPRESS_MODE_SUBHEADER "インストールモードを選択"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_INSTALL_RADIO "標準インストール(&S)"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_INSTALL_TEXT "推奨コンポーネントと $(^NameDA) をインストールします$\r$\n\
                                                          '$INSTDIR'"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_REINSTALL_TEXT "以前に選択したコンポーネントと $(^NameDA) をインストールします$\r$\n\
                                                          '$INSTDIR'"
${LangFileString} IDS_EXPRESS_MODE_CUSTOM_INSTALL_RADIO "カスタムインストール(&C)"
${LangFileString} IDS_EXPRESS_MODE_CUSTOM_INSTALL_TEXT "カスタムインストールはコンポーネントを手動選択して、$\r$\n\
                                                        $(^NameDA) を好みの状態に調整することが出来ます"
!endif ; defined (EXPRESS_MODE)