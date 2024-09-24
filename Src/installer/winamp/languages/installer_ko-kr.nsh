; Language-Country:	EN-US
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
# example: ${LangFileString} installFull "이 값만 편집하십시오!"
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
; 26.03 > djegg: removed "(Google®에 의해 개선됨)" from IDS_BUNDLE21_DESCRIPTION
; 02.05 > koopa: moved text "(기본 VIS 플러그인) " from IDS_SEC_AVS_DESC to IDS_SEC_MILKDROP2_DESC

!insertmacro LANGFILE_EXT "Korean"
 
${LangFileString} installFull "Full"
${LangFileString} installStandard "Standard"
${LangFileString} installLite "Lite"
${LangFileString} installMinimal "최소"
${LangFileString} installPrevious "이전 설치"

${LangFileString} installWinampTop "Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}을(를) 설치합니다.  "
${LangFileString} installerContainsFull "Full 설치를 포함합니다."
${LangFileString} installerContainsLite "Lite 설치를 포함합니다."
${LangFileString} licenseTop "설치하기 전에 아래 라이센스 조건을 읽고 동의하십시오."
${LangFileString} directoryTop "$(^NameDA)에 대한 최적의 위치를 결정했습니다. 폴더를 변경하려면 지금 변경하십시오."

${LangFileString} uninstallPrompt "Winamp가 제거됩니다. 계속하시겠습니까?"

${LangFileString} msgCancelInstall "설치를 취소하시겠습니까?"
${LangFileString} msgReboot "설치를 완료하려면 다시 부팅해야 합니다.$\r$\n지금 다시 부팅하시겠습니까? 나중에 다시 부팅하려면 $\"아니오$\"를 선택하십시오."
${LangFileString} msgCloseWinamp "계속하려면 Winamp를 닫아야 합니다.$\r$\n$\r$\n	Winamp를 닫은 후 $\"재시도$\"를 선택하십시오.$\r$\n$\r$\n	설치하려면 $\"무시$\"를 선택하십시오.$\r$\n$\r$\n	설치를 중단하려면 $\"중단$\"을 선택하십시오."
${LangFileString} msgInstallAborted "설치가 사용자에 의해 중단됨"

${LangFileString} secWinamp "Winamp(필수)"
${LangFileString} compAgent "Winamp 에이전트"
${LangFileString} compModernSkin "현대 스킨 지원"
${LangFileString} uninstallWinamp "Winamp 제거"

${LangFileString} secWMA "Windows 미디어 오디오(WMA)"
${LangFileString} secWMV "Windows 미디어 동영상(WMV, ASF)"
${LangFileString} secWMFDist "Windows Media Format 다운로드 및 설치"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD/XM/S3M/IT"
${LangFileString} secOGGPlay "OGG Vorbis 재생"
${LangFileString} secOGGEnc "OGG Vorbis 인코딩"
${LangFileString} secAACE "AAC/aacPlus 인코딩"
${LangFileString} secMP3E "MP3 인코딩"
${LangFileString} secMP4E "MP4 지원"
${LangFileString} secWMAE "WMA 인코딩"
${LangFileString} msgWMAError "구성요소를 설치하는 데 문제가 발생했습니다. WMA 인코더가 설치되지 않습니다. http://www.microsoft.com/windows/windowsmedia/9series/encoder/에서 인코더를 다운로드한 후 다시 시도하십시오."
${LangFileString} secCDDA "CD 재생 및 추출"
${LangFileString} msgCDError "구성요소를 설치하는 데 문제가 발생했습니다. CD 변환/굽기가 올바로 작동하지 않을 수 있습니다. "
${LangFileString} secCDDB "CD를 인식하기 위한 CDDB"
${LangFileString} secWAV "WAV/VOC/AU/AIFF"

${LangFileString} secDSP "Signal Processor Studio 플러그인"
${LangFileString} secWriteWAV "구식 WAV 라이터"
${LangFileString} secLineInput "라인 입력 지원"
${LangFileString} secDirectSound "DirectSound 출력 지원"

${LangFileString} secHotKey "글로벌 핫키 지원"
${LangFileString} secJmp "파일로 이동 확장 지원"
${LangFileString} secTray "Nullsoft 트레이 관리"

${LangFileString} msgRemoveMJUICE "시스템에서 MJuice 지원을 제거하시겠습니까?$\r$\n$\r$\nWinamp 이외의 다른 프로그램에서 MJF 파일을 사용하지 않는 경우 '예'를 선택하십시오."
${LangFileString} msgNotAllFiles "일부 파일은 제거되지 않았습니다.$\r$\n이 경우에는 파일을 직접 제거하십시오."


${LangFileString} secNSV "Nullsoft 동영상(NSV)"
${LangFileString} secDSHOW "AVI/MPG"

${LangFileString} secFLV "플래시 동영상(FLV)"

${LangFileString} secSWF "플래시 미디어 프로토콜(RTMP)"

${LangFileString} secTiny "Nullsoft Tiny 전체 화면"
${LangFileString} secAVS "Advanced Visualization Studio"
${LangFileString} secMilkDrop "Milkdrop"

${LangFileString} secML "Winamp 미디어 라이브러리"
${LangFileString} secOM "온라인 미디어"
${LangFileString} secWire "팟캐스트 디렉터리"
${LangFileString} secPmp "휴대용 미디어 플레이어"
${LangFileString} secPmpIpod "iPod 지원"
${LangFileString} secPmpCreative "Creative® 플레이어에 대한 지원"
${LangFileString} secPmpP4S "Microsoft® PlaysForSure®에 대한 지원"
${LangFileString} secPmpUSB "USB 장치 지원"
${LangFileString} secPmpActiveSync "Microsoft® ActiveSync®에 대한 지원"

${LangFileString} sec_ML_LOCAL "로컬 미디어"
${LangFileString} sec_ML_PLAYLISTS "재생목록"
${LangFileString} sec_ML_DISC "CD 변환 및 굽기"
${LangFileString} sec_ML_BOOKMARKS "북마크"
${LangFileString} sec_ML_HISTORY "내역"
${LangFileString} sec_ML_NOWPLAYING "지금 재생 중"
${LangFileString} sec_ML_RG "Replay Gain 분석 도구"
${LangFileString} sec_ML_TRANSCODE "트랜스코딩 도구"

;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "$(^NameDA) 설치 프로그램"
${LangFileString} IDS_SELECT_LANGUAGE  "설치 프로그램의 언어를 선택하십시오."

; Groups
${LangFileString} IDS_GRP_MMEDIA			"멀티미디어 엔진"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"출력"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"오디오 재생"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"오디오 인코더"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"동영상 재생"
${LangFileString} IDS_GRP_VISUALIZATION		"시각화"
${LangFileString} IDS_GRP_UIEXTENSION		"사용자 인터페이스 확장"
${LangFileString} IDS_GRP_WALIB				"Winamp 라이브러리"
${LangFileString} IDS_GRP_WALIB_CORE		"코어 미디어 라이브러리 구성요소"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"휴대용 미디어 플레이어 지원"
${LangFileString} IDS_GRP_LANGUAGES 	    "언어"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"WaveOut/MME 출력"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC 인코딩"
${LangFileString} IDS_SEC_MILKDROP2             "Milkdrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG		"Auto-Tagger"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"온라인 서비스 구성 중..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Winamp의 다른 인스턴스가 실행 중인지 확인 중..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"인터넷 연결 여는 중..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"인터넷이 사용 가능한지 확인 중..."
${LangFileString} IDS_RUN_NOINET				"인터넷에 연결되지 않음"
${LangFileString} IDS_RUN_EXTRACT				"추출 중"
${LangFileString} IDS_RUN_DOWNLOAD				"다운로드 중"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"다운로드 완료."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"다운로드 실패. 이유: "
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"다운로드 취소."
${LangFileString} IDS_RUN_INSTALL				"설치 중"
${LangFileString} IDS_RUN_INSTALLFIALED			"설치 실패."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"파일을 찾을 수 없습니다. 다운로드를 예약하는 중입니다."
${LangFileString} IDS_RUN_DONE					"완료."

${LangFileString} IDS_DSP_PRESETS 	"SPS 프리셋"
${LangFileString} IDS_DEFAULT_SKIN	"기본 스킨"
${LangFileString} IDS_AVS_PRESETS	"AVS 프리셋"
${LangFileString} IDS_MILK_PRESETS	"MilkDrop 프리셋"
${LangFileString} IDS_MILK2_PRESETS	"MilkDrop2 프리셋"

; download
${LangFileString} IDS_DOWNLOADING	"%s 다운로드 중"
${LangFileString} IDS_CONNECTING	"연결 중..."
${LangFileString} IDS_SECOND		" (1초 남음)"
${LangFileString} IDS_MINUTE		" (1분 남음)"
${LangFileString} IDS_HOUR			" (1시간 남음)"
${LangFileString} IDS_SECONDS		" (%u초 남음)"
${LangFileString} IDS_MINUTES		" (%u분 남음)"
${LangFileString} IDS_HOURS			" (%u시간 남음)"
${LangFileString} IDS_PROGRESS		"%skB (%d%%)/%skB @ %u.%01ukB/s"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"설치 완료"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA)이(가) 컴퓨터에 설치되었습니다.$\r$\n$\r$\n\
													'마침'을 클릭하여 이 마법사를 닫으십시오."
${LangFileString} IDS_PAGE_FINISH_RUN		"설치 프로그램이 닫히면 $(^NameDA) 실행"
${LangFileString} IDS_PAGE_FINISH_LINK		"여기를 클릭하여 Winamp.com을 방문하십시오."

; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE		"$(^NameDA) 설치 프로그램에 오신 것을 환영합니다."
${LangFileString} IDS_PAGE_WELCOME_TEXT		"최고의 미디어 관리자 그 이상을 경험해 보세요.$\r$\n$\r$\n\
													좋아하는 음악, 동영상 및 팟캐스트를 즐기세요. 변환하고 CD를 굽고 재생목록을 만들고 \
                          휴대용 음악 플레이어와 동기화하고 친구들과 공유하세요. 수 천 개의 라디오 방송국, 동영상, 아티스트 리뷰 등에서 \
                          새로운 음악을 발견하세요. $\r$\n  \
													•  최신 Bento 스킨$\r$\n  \
													•  iPod®를 포함하여 다중 음악 장치 지원$\r$\n  \
													•  앨범 아트 지원 - Winamp가 찾고 사용자는 즐깁니다!$\r$\n  \
													•  미디어 모니터로 웹의 베스트 음악 즐기기$\r$\n  \
													•  동적 곡 추천$\r$\n  \
													•  MP3 서라운드 사운드 지원$\r$\n  \
													•  원격 음악과 동영상 액세스 및 공유$\r$\n  \
													•  자동 태깅 기능으로 음악 분류"

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"참고: Bento 스킨(권장)의 새로운 기능과 \
															설계를 사용하려면 구성요소를 모두\
															선택해야 합니다."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"시작 옵션 선택"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"다음 시작 옵션 중에서 선택하십시오."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"다음 옵션 중에서 원하는 옵션을 선택하여 Winamp 시작 옵션을 구성하십시오."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"시작 메뉴 항목 만들기"
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"빠른 실행 아이콘 만들기"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"바탕화면 아이콘 만들기"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Winamp 에이전트를 닫을 수 없습니다.$\r$\n\
                                                   다른 사용자가 Windows에 로그인되어 있지 않는지 확인하십시오.\
                                                   $\r$\n$\r$\n	WinampAgent를 닫은 후 '재시도'를 선택하십시오.\
                                                   $\r$\n$\r$\n	설치하려면 '무시'를 선택하십시오.\
                                                   $\r$\n$\r$\n	설치를 중단하려면 '중단'을 선택하십시오."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"이 버전의 Windows는 더 이상 지원되지 않습니다.$\r$\n\
                                                 $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND}의 경우 Windows 2000 이상이 필요합니다."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Winamp 코어(필수)"
${LangFileString} IDS_SEC_AGENT_DESC			"Winamp 에이전트(시스템 트레이에 빠르게 액세스하고 파일 형식 연결 유지)"
${LangFileString} IDS_GRP_MMEDIA_DESC			"멀티미디어 엔진(입력/출력 시스템)"
${LangFileString} IDS_SEC_CDDB_DESC				"CDDB 지원(온라인 Gracenote 데이터베이스에서 CD 제목을 자동으로 가져올 수 있음)"
${LangFileString} IDS_SEC_DSP_DESC				"DSP 플러그인(코러스, 플랜저, 템포 및 피치 제어와 같은 추가 효과 적용)"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"오디오 재생 지원(입력 플러그인: 오디오 디코더)"
${LangFileString} IDS_SEC_MP3_DEC_DESC			"MP3, MP2, MP1, AAC 형식에 대한 재생 지원(필수)"
${LangFileString} IDS_SEC_WMA_DEC_DESC			"WMA 형식에 대한 재생 지원(DRM 지원 포함)"
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"MIDI 형식(MID, RMI, KAR, MUS, CMF 등)에 대한 재생 지원"
${LangFileString} IDS_SEC_MOD_DEC_DESC			"모듈 형식(MOD, XM, IT, S3M, ULT 등)에 대한 재생 지원"
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Ogg Vorbis 형식(OGG)에 대한 재생 지원"
${LangFileString} IDS_SEC_MP4_DEC_DESC			"MPEG-4 오디오 형식(MP4, M4A)에 대한 재생 지원"
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"FLAC 형식에 대한 재생 지원"
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"오디오 CD에 대한 재생 지원"
${LangFileString} IDS_SEC_WAV_DEC_DESC			"파형 형식(WAV, VOC, AU, AIFF 등)에 대한 재생 지원"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"동영상 재생 지원(입력 플러그인: 비디오 디코더)"
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Windows Media 동영상 형식(WMV, ASF)에 대한 재생 지원"
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Nullsoft 동영상 형식(NSV, NSA)에 대한 재생 지원"
${LangFileString} IDS_SEC_DSHOW_DEC_DESC			"AVI & MPEG 동영상 형식에 대한 재생 지원"
${LangFileString} IDS_SEC_FLV_DEC_DESC			"vp6 FLV(Flash Video)에 대한 재생 지원"
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Adobe Flash의 스트리밍 형식(RTMP)에 대한 재생 지원"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"인코딩 및 변환 지원(CD 변환 및 파일 형식 간 변환 시 필요)"
${LangFileString} IDS_SEC_WMA_ENC_DESC			"WMA 형식으로 변환 및 트랜스코딩 지원"
${LangFileString} IDS_SEC_WAV_ENC_DESC			"WAV 형식으로 변환 및 트랜스코딩 지원"
${LangFileString} IDS_SEC_AAC_ENC_DESC			"M4A 및 AAC 형식으로 변환 및 트랜스코딩 지원"
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"FLAC 형식으로 변환 및 트랜스코딩 지원"
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Ogg Vorbis 형식으로 변환 및 트랜스코딩 지원"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"출력 플러그인(오디오가 처리되고 사운드카드로 전송되는 방법 제어)"
${LangFileString} IDS_SEC_OUT_DISK_DESC			"구식 WAV/MME 라이터(권장되지 않지만, 일부 사용자의 경우 인코더 플러그인 대신 WAV/MME 라이터 선호)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"DirectSound 출력(필수/기본 출력 플러그인)"
${LangFileString} IDS_SEC_OUT_WAV_DESC			"구식 WaveOut 출력(선택사항. 더 이상 권장 또는 필수 사항이 아님)"
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"사용자 인터페이스 확장"
${LangFileString} IDS_SEC_HOTKEY_DESC			"글로벌 핫키 플러그인(다른 응용프로그램이 활성 상태일 때 키보드로 Winamp 제어)"
${LangFileString} IDS_SEC_JUMPEX_DESC			"파일로 이동 확장 지원(재생목록에 훨씬 더 많은 곡 삽입)"
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Nullsoft 트레이 제어 플러그인(시스템 트레이에 재생 제어 아이콘 추가)"
${LangFileString} IDS_SEC_FREEFORM_DESC			"현대 스킨 지원(Winamp Modern 및 Bento와 같은 자유형 스킨 사용 시 필요)"
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"가상화 플러그인"
${LangFileString} IDS_SEC_NSFS_DESC				"Nullsoft Tiny Fullscreen 가상화 플러그인"
${LangFileString} IDS_SEC_AVS_DESC				"Advanced Visualization Studio 플러그인"
${LangFileString} IDS_SEC_MILKDROP_DESC			"Milkdrop 가상화 플러그인"
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Milkdrop2 가상화 플러그인(기본 VIS 플러그인)"
${LangFileString} IDS_SEL_LINEIN_DESC			"linein:// 명령을 사용하여 라인 입력 지원(마이크/라인 입력에 비주얼라이저 적용)"
${LangFileString} IDS_GRP_WALIB_DESC			"Winamp 라이브러리"
${LangFileString} IDS_SEC_ML_DESC				"Winamp 미디어 라이브러리(필수)"
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"트랜스코딩 도구(특정 파일 형식에서 다른 파일 형식으로 변환하는 데 사용)"
${LangFileString} IDS_SEC_ML_RG_DESC			"Replay Gain 분석 도구(음량 조정 시 사용)"
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Gracenote에 의해 성능이 강화된 Winamp Auto-Tagger(누락된 메타데이터 입력)"
${LangFileString} IDS_SEC_ML_WIRE_DESC			"팟캐스트 디렉터리(팟캐스트 가입 및 다운로드)"
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"온라인 서비스(SHOUTcast 라디오 & TV, In2TV, AOL 동영상 및 XM 라디오 스트림 포함)"
${LangFileString} IDS_SEC_ML_PLG_DESC			"Gracenote에 의해 성능 강화된 Winamp 재생목록 생성기(음향적으로 동적 재생목록 작성)"
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"코어 미디어 라이브러리 구성요소"
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"로컬 미디어 데이터베이스(강력한 쿼리 시스템 및 사용자 지정 스마트 보기 포함)"
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"재생목록 관리자(모든 재생목록 작성, 편집 및 저장)"
${LangFileString} IDS_SEC_ML_DISC_DESC			"CD 변환 및 굽기(오디오 CD를 변환하고 굽기 위한 미디어 라이브러리 인터페이스)"
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"북마크 관리자(즐겨찾는 스트림, 파일 또는 폴더 북마크)"
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"내역(최근에 재생된 모든 로컬 또는 원격 파일 및 스트림에 즉시 액세스)"
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"지금 재생 중(현재 재생 중인 곡에 대한 정보 확인)"
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"휴대용 미디어 플레이어 지원"
${LangFileString} IDS_SEC_ML_PMP_DESC			"Core Portable Media Player Support 플러그인(필수)"
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"iPod 지원"
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Creative® 휴대 장치에 대한 지원(Nomad™, Zen™ 및 MuVo™ 플레이어 관리용)"
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Microsoft® PlaysForSure®에 대한 지원(모든 P4S 호환 가능 플레이어 관리용)"
${LangFileString} IDS_SEC_PMP_USB_DESC			"USB 장치 지원(일반 USB 썸드라이브 및 플레이어 관리용)"
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Microsoft® ActiveSync®에 대한 지원(Windows Mobile®, Smartphone 및 Pocket PC 장치 관리용)"
