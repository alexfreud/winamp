; Language-Country:	RU-RU
; LangId:			1049
; CodePage:			1251
; Revision:			4
; Last udpdated:	24.02.2016
; Author:			Alexander Nureyev, Eduard Galkin
; Email:			ED_Sln@ru.winamp.com

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

!insertmacro LANGFILE_EXT "Russian"

; Language selection dialog
${LangFileString} LANGUAGE_DLL_TITLE "Язык программы установки"
${LangFileString} LANGUAGE_DLL_INFO "Выберите язык:"
 
${LangFileString} installFull "Полная"
${LangFileString} installStandard "Стандартная"
${LangFileString} installLite "Облегченная"
${LangFileString} installMinimal "Минимальная"
${LangFileString} installPrevious "Предыдущая установка"

; BrandingText
${LangFileString} BuiltOn "собран"
${LangFileString} at "в"

${LangFileString} installWinampTop "Вы собираетесь установить Winamp ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} ${InstallType}. "
${LangFileString} installerContainsFull "Программа установки содержит полную версию Winamp."
${LangFileString} installerContainsLite "Программа установки содержит облегченную версию Winamp."
${LangFileString} licenseTop "Перед установкой прочтите и примите условия лицензионного соглашения."
${LangFileString} directoryTop "Программа установки определила наиболее подходящий путь для установки $(^NameDA). При необходимости путь можно изменить в следующем поле."

${LangFileString} uninstallPrompt "Проигрыватель Winamp будет удален. Продолжить?"

${LangFileString} msgCancelInstall "Отменить установку?"
${LangFileString} msgReboot "Для завершения установки необходимо перезагрузить компьютер.$\r$\n$\r$\nПерезагрузить прямо сейчас?$\r$\n$\r$\n(Чтобы перезагрузить компьютер позже, нажмите кнопку $\"Нет$\".)"
${LangFileString} msgCloseWinamp "Для продолжения установки необходимо завершить работу Winamp.$\r$\n$\r$\nПосле закрытия Winamp нажмите кнопку $\"Повтор$\". Чтобы все равно продолжить установку, нажмите кнопку $\"Пропустить$\". Либо прервите установку, нажав кнопку $\"Прервать$\"."
${LangFileString} msgInstallAborted "Установка прервана пользователем"

${LangFileString} secWinamp "Winamp (обязательно)"
${LangFileString} compAgent "Агент Winamp"
${LangFileString} compModernSkin "Поддержка современных обложек"
${LangFileString} safeMode "Winamp (безопасный режим)"
${LangFileString} uninstallWinamp "Удалить Winamp"

${LangFileString} secWMA "Windows Media Audio (WMA)"
${LangFileString} secWMV "Windows Media Video (WMV и ASF)"
${LangFileString} secWMFDist "Загрузить и установить Windows Media Format"

${LangFileString} secMIDI "MIDI"
${LangFileString} secMOD "MOD, XM, S3M и IT"
${LangFileString} secOGGPlay "Ogg Vorbis"
${LangFileString} secOGGEnc "Ogg Vorbis"
${LangFileString} secAACE "HE-AAC"
${LangFileString} secMP3E "MP3"
${LangFileString} secMP4E "MP4"
${LangFileString} secWMAE "WMA"
${LangFileString} msgWMAError "При установке некоторых компонентов возникли проблемы. Не удалось установить кодер Windows Media Audio. Чтобы загрузить и установить его вручную, перейдите по адресу http://www.microsoft.com/windows/windowsmedia/9series/encoder и повторите попытку"
${LangFileString} secCDDA "Аудиодиски"
${LangFileString} msgCDError "При установке некоторых компонентов возникли проблемы. Не исключено, что при копировании или записи аудиодисков могут происходить ошибки"
${LangFileString} secCDDB "CDDB для распознавания аудиодисков"
${LangFileString} secWAV "WAV, VOC, AU и AIFF"

${LangFileString} secDSP "Студия обработки сигнала (DSP)"
${LangFileString} secWriteWAV "Устаревший модуль записи WAV"
${LangFileString} secLineInput "Поддержка линейного входа"
${LangFileString} secDirectSound "Вывод DirectSound"
${LangFileString} secWasapi "Вывод WASAPI"

${LangFileString} secHotKey "Поддержка глобальных сочетаний клавиш"
${LangFileString} secJmp "Подключаемый модуль $\"Переход к файлу$\""
${LangFileString} secTray "Управление из области уведомлений"

${LangFileString} msgRemoveMJUICE "Удалить из системы поддержку MJuice?$\r$\n$\r$\nЕсли файлы формата MJF не используются в других программах, нажмите кнопку $\"Да$\"."
${LangFileString} msgNotAllFiles "Удалены не все файлы.$\r$\n$\r$\nЕсли необходимо удалить оставшиеся файлы, сделайте это вручную."

${LangFileString} secNSV "Nullsoft Video (NSV)"
${LangFileString} secDSHOW "Форматы DirectShow (MPG и M2V)"
${LangFileString} secAVI "AVI"
${LangFileString} secFLV "Видео Flash (FLV)"
${LangFileString} secMKV "Matroska (MKV и MKA)"
${LangFileString} secM4V "MPEG-4 (MP4 и M4V)"
${LangFileString} secSWF "Shockwave Flash (SWF и RTMP)"

${LangFileString} secTiny "Nullsoft Tiny Fullscreen"
${LangFileString} secAVS "Advanced Visualization Studio"
${LangFileString} secMilkDrop "MilkDrop"

${LangFileString} secML "Библиотека Winamp"
${LangFileString} secOM "Интерактивные службы"
${LangFileString} secWire "Каталог подкастов"
${LangFileString} secPmp "Портативные плееры"
${LangFileString} secPmpIpod "Поддержка iPod®"
${LangFileString} secPmpCreative "Поддержка Creative®"
${LangFileString} secPmpP4S "Поддержка Microsoft® PlaysForSure®"
${LangFileString} secPmpUSB "Поддержка USB-устройств"
${LangFileString} secPmpActiveSync "Поддержка Microsoft® ActiveSync®"
${LangFileString} secPmpAndroid "Поддержка Android-устройств"
${LangFileString} secPmpWifi "Поддержка Wi-Fi для Android-устройств"

${LangFileString} sec_ML_LOCAL "Домашняя библиотека"
${LangFileString} sec_ML_PLAYLISTS "Списки воспроизведения"
${LangFileString} sec_ML_DISC "Копирование и запись аудиодисков"
${LangFileString} sec_ML_BOOKMARKS "Закладки"
${LangFileString} sec_ML_HISTORY "История воспроизведения"
${LangFileString} sec_ML_NOWPLAYING "Служба $\"Проигрывается$\""
${LangFileString} sec_ML_RG "Анализатор Replay Gain"
${LangFileString} sec_ML_TRANSCODE "Аудиоконвертер"
${LangFileString} sec_ML_PLG "Генератор списков воспроизведения"
${LangFileString} sec_ML_IMPEX "Средство импорта и экспорта базы данных"

;=========================================================================== ver 3.0

${LangFileString} IDS_CAPTION          "Мастер установки $(^NameDA)"
${LangFileString} IDS_SELECT_LANGUAGE  "Выберите язык программы установки"

; Groups
${LangFileString} IDS_GRP_MMEDIA			"Мультимедийные обработчики"
${LangFileString} IDS_GRP_MMEDIA_OUTPUT 	"Вывод"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC	"Поддержка аудиоформатов"
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC	"Аудиокодеры"
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC	"Поддержка видеоформатов"
${LangFileString} IDS_GRP_VISUALIZATION		"Зрительные образы"
${LangFileString} IDS_GRP_UIEXTENSION		"Расширение пользовательского интерфейса"
${LangFileString} IDS_GRP_WALIB				"Библиотека Winamp"
${LangFileString} IDS_GRP_WALIB_CORE		"Основные компоненты библиотеки"
${LangFileString} IDS_GRP_WALIB_PORTABLE 	"Поддержка портативных плееров"
${LangFileString} IDS_GRP_LANGUAGES 	    "Языки"

; Sections
${LangFileString} IDS_SEC_OUT_WAV		"Вывод WaveOut и MME"
${LangFileString} IDS_SEC_WAV_ENC		"WAV"
${LangFileString} IDS_SEC_MP3_DEC		"MP3"
${LangFileString} IDS_SEC_FLAC_DEC		"FLAC"
${LangFileString} IDS_SEC_FLAC_ENC		"FLAC"
${LangFileString} IDS_SEC_MILKDROP2 	"MilkDrop2"

${LangFileString} IDS_SEC_ML_AUTOTAG		"Средство автозаполнения тегов"
${LangFileString} IDS_SEC_GEN_FRENCHRADIO	"Радио Франции"

; installation strings
${LangFileString} IDS_RUN_CONFIG_ONLINE			"Настройка интерактивных служб..."
${LangFileString} IDS_RUN_CHECK_PROCESS			"Проверка на активность других экземпляров Winamp..."
${LangFileString} IDS_RUN_CHECK_IFCONNECTED		"Открытие подключения к Интернету..."
${LangFileString} IDS_RUN_CHECK_IFINETAVAILABLE	"Проверка подключения к Интернету..."
${LangFileString} IDS_RUN_NOINET				"Подключение к Интернету отсутствует"
${LangFileString} IDS_RUN_EXTRACT				"Извлечение"
${LangFileString} IDS_RUN_DOWNLOAD				"Загрузка"
${LangFileString} IDS_RUN_DOWNLOADSUCCESS		"Загрузка завершена."
${LangFileString} IDS_RUN_DOWNLOADFAILED		"Ошибка загрузки. Причина:"
${LangFileString} IDS_RUN_DOWNLOADCANCELLED		"Загрузка отменена."
${LangFileString} IDS_RUN_INSTALL				"Установка"
${LangFileString} IDS_RUN_INSTALLFIALED			"Ошибка установки."
${LangFileString} IDS_RUN_FILE_NOT_FOUND_SCHEDULE_DOWNLOAD	"Файл не найден. Планирование загрузки."
${LangFileString} IDS_RUN_DONE					"Готово."

${LangFileString} IDS_DSP_PRESETS 	"заготовок SPS"
${LangFileString} IDS_DEFAULT_SKIN	"стандартных обложек"
${LangFileString} IDS_AVS_PRESETS	"заготовок AVS"
${LangFileString} IDS_MILK_PRESETS	"заготовок MilkDrop"
${LangFileString} IDS_MILK2_PRESETS	"заготовок MilkDrop2"

${LangFileString} IDS_CLEANUP_PLUGINS	"Удаление подключаемых модулей..."
${LangFileString} IDS_REMOVE_SKINS		"Удаление стандартных обложек..."

; download
${LangFileString} IDS_DOWNLOADING	"Загрузка"
${LangFileString} IDS_CONNECTING	"Подключение..."
${LangFileString} IDS_SECOND		" (осталась 1 сек.)"
${LangFileString} IDS_MINUTE		" (осталась 1 мин.)"
${LangFileString} IDS_HOUR			" (остался 1 ч.)"
${LangFileString} IDS_SECONDS		" (осталось %u сек.)"
${LangFileString} IDS_MINUTES		" (осталось %u мин.)"
${LangFileString} IDS_HOURS			" (осталось %u ч.)"
${LangFileString} IDS_PROGRESS		"%s (%d%%) из %s КБ со скоростью %u,%01u кбит/с"

; AutoplayHandler
${LangFileString} AutoplayHandler	"Воспроизводить"

;=========================================================================================
; pages
; finish page
${LangFileString} IDS_PAGE_FINISH_TITLE		"Установка завершена"
${LangFileString} IDS_PAGE_FINISH_TEXT		"$(^NameDA) успешно установлен на ваш компьютер.$\r$\n$\r$\n\Нажмите кнопку $\"Готово$\" для выхода из мастера установки."
${LangFileString} IDS_PAGE_FINISH_RUN		"Запустить $(^NameDA)"
${LangFileString} IDS_PAGE_FINISH_LINK		"Щелкните здесь для перехода на веб-сайт Winamp.com"

; welcome page
${LangFileString} IDS_PAGE_WELCOME_TITLE	"Вас приветствует мастер установки $(^NameDA)"

!ifdef EXPRESS_MODE
${LangFileString} IDS_PAGE_WELCOME_TEXT		"С помощью $(^NameDA) слушайте музыку, подкасты, \
											интернет-радио и смотрите любимые фильмы. Будьте \
											с музыкой дома, на работе и даже за рулем!\
											$\r$\n$\r$\n$\r$\n\
											Ключевые особенности:$\r$\n$\r$\n  \
											•  Беспроводная синхронизация с Android.$\r$\n$\r$\n  \
											•  Автозаполнение тегов у файлов.$\r$\n$\r$\n  \
											•  Генератор списков воспроизведения поможет найти$\r$\n      \
											   песни, похожие на те, которые вам нравятся.$\r$\n$\r$\n  \
											•  Доступ к каталогу SHOUTcast, где представлены$\r$\n      \
											   радиостанции всего мира (около 50 тыс.).$\r$\n$\r$\n  \
											•  Доступ к более чем 30 тыс. подкастов."
${LangFileString} IDS_PAGE_WELCOME_TEXT		"С помощью $(^NameDA) слушайте музыку, подкасты, \
											интернет-радио и смотрите любимые фильмы. Будьте \
											с музыкой дома, на работе и даже за рулем!.$\r$\n$\r$\n\
											Ключевые особенности:$\r$\n$\r$\n  \
											•  Будет добавлено много новых функций$\r$\n$\r$\n  \
											•  Это не финальная версия, поэтому возможны ошибки...$\r$\n$\r$\n  \
											•  НИКОМУ НЕ ДАВАЙТЕ ЭТУ ВЕРСИЮ!"
${LangFileString} IDS_PAGE_WELCOME_LEGAL	"Нажимая кнопку $\"Далее$\", вы принимаете условия <a id=$\"winamp_eula$\">соглашения</a> и <a id=$\"winamp_privacy_policy$\">политики конфиденциальности</a>."
!else
${LangFileString} IDS_PAGE_WELCOME_TEXT		"С помощью $(^NameDA) работайте с музыкальной \
											коллекцией и слушайте интернет-радио.$\r$\n$\r$\n\
											Ключевые особенности:$\r$\n$\r$\n  \
											•  Беспроводная синхронизация с Android.$\r$\n$\r$\n  \
											•  Генератор списков воспроизведения поможет найти$\r$\n      \
											   песни, похожие на те, которые вам нравятся.$\r$\n$\r$\n  \
											•  Доступ к более чем 30 тыс. подкастов."
; TODO determine the correct messaging
${LangFileString} IDS_PAGE_WELCOME_TEXT		"С помощью $(^NameDA) слушайте музыку, подкасты, \
											интернет-радио и смотрите любимые фильмы. Будьте \
											с музыкой дома, на работе и даже за рулем!.$\r$\n$\r$\n\
											Ключевые особенности.$\r$\n$\r$\n\
											Features include:$\r$\n$\r$\n  \
											•  Будет добавлено много новых функций$\r$\n$\r$\n  \
											•  Это не финальная версия, поэтому возможны ошибки...$\r$\n$\r$\n  \
											•  НИКОМУ НЕ ДАВАЙТЕ ЭТУ ВЕРСИЮ"
!endif ; defined (EXPRESS_MODE)

; components
${LangFileString} IDS_PAGE_COMPONENTS_COMPLIST		"Примечание. Для получения всех возможностей и обложки Bento (рекомендуется) отметьте все компоненты."

; start menu page
${LangFileString} IDS_PAGE_STARTMENU_TITLE			"Создание ярлыков"
${LangFileString} IDS_PAGE_STARTMENU_SUBTITLE		"Укажите, где нужно создать ярлыки."
${LangFileString} IDS_PAGE_STARTMENU_CAPTION		"Укажите, где необходимо создать ярлыки для запуска Winamp."
${LangFileString} IDS_PAGE_STARTMENU_CHK_START		"Создать ярлык в меню $\"Пуск$\""
${LangFileString} IDS_PAGE_STARTMENU_CHK_QUICKLAUNCH	"Создать ярлык на панели быстрого запуска"
${LangFileString} IDS_PAGE_STARTMENU_CHK_DESKTOP	"Создать ярлык на рабочем столе"

; messages
${LangFileString} IDS_MSG_AGENTONOTHERSESSION	"Не удалось завершить работу Агента Winamp.$\r$\n$\r$\n\
                                                   Убедитесь, что никакой другой пользователь Windows не вошел в систему.\
                                                   $\r$\n$\r$\nПосле завершения работы Агента нажмите кнопку $\"Повтор$\". \
                                                   Чтобы все равно продолжить установку, нажмите кнопку $\"Пропустить$\". \
                                                   Либо прервите установку, нажав кнопку $\"Прервать$\"."

${LangFileString} IDS_MSG_WINDOWS_TOO_OLD	"Эта версия Windows больше не поддерживается.$\r$\n$\r$\n\
                                                 Для работы $(^NameDA) ${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_MINOR_SECOND} требуется Windows XP SP3 или новее."

; Disable incompatible 3rd-party gen_msn7.dll plugin, if present (renames it to gen_msn7.dll.off)
${LangFileString} IDS_MSN7_PLUGIN_DISABLE		"Обнаружен несовместимый подключаемый модуль gen_msn7.dll, разработанный сторонним разработчиком.$\r$\n$\r$\nОн приводит к аварийному завершению работы Winamp (начиная с версии 5.57).$\r$\n$\r$\nДанный подключаемый модуль будет отключен. Для продолжения нажмите кнопку $\"ОК$\"."

; Disable incompatible 3rd-party gen_lyrics.dll plugin, if present (renames it to gen_lyrics.dll.off)
${LangFileString} IDS_LYRICS_PLUGIN_DISABLE		"Обнаружен несовместимый подключаемый модуль gen_lyrics.dll, разработанный сторонним разработчиком.$\r$\n$\r$\nОн приводит к аварийному завершению работы Winamp (начиная с версии 5.59).$\r$\n$\r$\nДанный подключаемый модуль будет отключен. Для продолжения нажмите кнопку $\"ОК$\"."
${LangFileString} IDS_LYRICS_PLUGIN_WARNING     "Обнаружен несовместимый подключаемый модуль gen_lyrics.dll, разработанный сторонним разработчиком.$\r$\n$\r$\nСтарые версии данного подключаемого модуля несовместимы с Winamp 5.6 и более новыми версиями. Загрузите последнюю версию gen_lyrics.dll с веб-сайта lyricsplugin.com."
${LangFileString} IDS_LYRICS_IE_PLUGIN_DISABLE	"Обнаружен несовместимый подключаемый модуль gen_lyrics_ie.dll, разработанный сторонним разработчиком.$\r$\n$\r$\nОн приводит к аварийному завершению работы Winamp.$\r$\n$\r$\nДанный подключаемый модуль будет отключен. Для продолжения нажмите кнопку $\"ОК$\"."

;DirectX Section
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_LOWER	"Обнаружен ${DIRECTXINSTAL_WINVER_LO} или более старой версии"
${LangFileString} IDS_DIRECTX_DETECTED_WINVER_OR_HIGHER "Обнаружен ${DIRECTXINSTAL_WINVER_HI} или более новой версии"
${LangFileString} IDS_DIRECTX_CHECKING_DIRECTX_VER		"Проверка версии ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_REQUIRED_DIRECTX_MINVER 	"Требуется версия ${DIRECTXINSTAL_DIRECTXNAME} не ниже"
${LangFileString} IDS_DIRECTX_UNABLE_DETECT_DIRECTX		"Не удалось определить версию ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_DETECTED_DIRECTX_VER		"Установленная версия ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_UNSUPPORTED_DIRECTX_VER	"Неподдерживаемая версия ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_CHECKING_D3DX_COMPONENT	"Проверка на наличие $0"
${LangFileString} IDS_DIRECTX_DOWNLOAD_REQUIRED			"Требуется загрузка"
${LangFileString} IDS_DIRECTX_CHECKING_INTERNET			"Проверка подключения к Интернету"
${LangFileString} IDS_DIRECTX_LINK_TO_MSDOWNLOAD		"Доступна новая версия ${DIRECTXINSTAL_DIRECTXNAME}:"
${LangFileString} IDS_DIRECTX_DOWNLOADING_SETUP			"Загрузка установщика ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FOUND						"Найдено"
${LangFileString} IDS_DIRECTX_MISSING					"Отсутствует"
${LangFileString} IDS_DIRECTX_SUCCESS					"Успешно"
${LangFileString} IDS_DIRECTX_ABORTED					"Прервано"
${LangFileString} IDS_DIRECTX_FAILED					"Ошибка"
${LangFileString} IDS_DIRECTX_DONE						"Готово"
${LangFileString} IDS_DIRECTX_RUNNING_SETUP				"Запуск установщика ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_APPROVAL		"Для нормальной работы ${DIRECTXINSTAL_WINAMPNAME} требуется версия ${DIRECTXINSTAL_DIRECTXNAME} не ниже ${DIRECTXINSTALL_DIRECTXMINVER}.$\r$\n$\r$\nУстановить?"
${LangFileString} IDS_DIRECTX_FULL_CONNECT_FAILED		"Для нормальной работы ${DIRECTXINSTAL_WINAMPNAME} требуется версия ${DIRECTXINSTAL_DIRECTXNAME} не ниже ${DIRECTXINSTALL_DIRECTXMINVER}"
${LangFileString} IDS_DIRECTX_FULL_DOWNLOAD_FAILED		"Не удалось загрузить ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_FULL_INSTALL_FAILED		"Не удалось установить ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_CONNECT_FAILED		"На компьютере не установлен компонент для ${DIRECTXINSTAL_DIRECTXNAME}, необходимый для работы ${DIRECTXINSTAL_WINAMPNAME}"
${LangFileString} IDS_DIRECTX_EMBED_DOWNLOAD_FAILED		"Не удалось загрузить недостающий компонент для ${DIRECTXINSTAL_DIRECTXNAME}"
${LangFileString} IDS_DIRECTX_EMBED_INSTALL_FAILED		"Не удалось установить недостающий компонент для ${DIRECTXINSTAL_DIRECTXNAME}"

;French Radio Section
${LangFileString} IDS_FRENCHRADIO_INSTALLING			"Установка $(IDS_SEC_GEN_FRENCHRADIO)..."

;========================================================================================
; descriptions

${LangFileString} IDS_SEC_WINAMP_DESC			"Ядро проигрывателя Winamp (обязательно)."
${LangFileString} IDS_SEC_AGENT_DESC			"В области уведомлений появится значок, с помощью которого можно получить быстрый доступ к возможностям Winamp."
${LangFileString} IDS_GRP_MMEDIA_DESC			"Мультимедийные обработчики (система ввода и вывода)."
${LangFileString} IDS_SEC_CDDB_DESC				"Получает из базы данных Gracenote сведения об аудиодисках."
${LangFileString} IDS_SEC_DSP_DESC				"Позволяет использовать такие звуковые эффекты, как хор и флэнджер, а также позволяет регулировать темп и высоту тона."
${LangFileString} IDS_GRP_MMEDIA_AUDIO_DEC_DESC	"Поддержка воспроизведения аудио (аудиодекодеры)."
${LangFileString} IDS_SEC_MP3_DEC_DESC			"Обеспечивает поддержку форматов MP3, MP2, MP1 и AAC (обязателен)."
${LangFileString} IDS_SEC_WMA_DEC_DESC			"Обеспечивает поддержку формата WMA (включая поддержку DRM)."
${LangFileString} IDS_SEC_MIDI_DEC_DESC			"Обеспечивает поддержку форматов MIDI (MID, RMI, KAR, MUS, CMF и др.)."
${LangFileString} IDS_SEC_MOD_DEC_DESC			"Обеспечивает поддержку форматов трекерной музыки (MOD, XM, IT, S3M, ULT и др.)."
${LangFileString} IDS_SEC_OGG_DEC_DESC			"Обеспечивает поддержку формата Ogg Vorbis (OGG)."
${LangFileString} IDS_SEC_MP4_DEC_DESC			"Обеспечивает поддержку форматов MPEG-4 Audio (MP4 и M4A)."
${LangFileString} IDS_SEC_FLAC_DEC_DESC			"Обеспечивает поддержку формата FLAC."
${LangFileString} IDS_SEC_CDDA_DEC_DESC			"Обеспечивает поддержку аудиодисков."
${LangFileString} IDS_SEC_WAV_DEC_DESC			"Обеспечивает поддержку форматов Waveform (WAV, VOC, AU, AIFF и др.)."
${LangFileString} IDS_GRP_MMEDIA_VIDEO_DEC_DESC	"Поддержка воспроизведения видео (видеодекодеры)."
${LangFileString} IDS_SEC_WMV_DEC_DESC			"Обеспечивает поддержку видео в формате Windows Media Video (WMV и ASF)."
${LangFileString} IDS_SEC_NSV_DEC_DESC			"Обеспечивает поддержку видео в формате Nullsoft Video (NSV и NSA)."
${LangFileString} IDS_SEC_DSHOW_DEC_DESC		"Обеспечивает поддержку видео в форматах MPEG."
${LangFileString} IDS_SEC_AVI_DEC_DESC			"Обеспечивает поддержку видео в форматах AVI."
${LangFileString} IDS_SEC_FLV_DEC_DESC			"Обеспечивает поддержку видео в формате Flash VP6 (FLV)."
${LangFileString} IDS_SEC_MKV_DEC_DESC			"Обеспечивает поддержку видео в формате Matroska (MKV)."
${LangFileString} IDS_SEC_M4V_DEC_DESC			"Обеспечивает поддержку видео в форматах MPEG-4 Video (MP4 и M4V)."
${LangFileString} IDS_SEC_SWF_DEC_DESC			"Обеспечивает поддержку потокового формата Shockwave Flash (SWF и RTMP)."
${LangFileString} IDS_GRP_MMEDIA_AUDIO_ENC_DESC	"Поддержка копирования аудиодисков и конвертации файлов."
${LangFileString} IDS_SEC_WMA_ENC_DESC			"Копирует дорожки с аудиодисков и конвертирует файлы в формат WMA."
${LangFileString} IDS_SEC_WAV_ENC_DESC			"Копирует дорожки с аудиодисков и конвертирует файлы в формат WAV."
${LangFileString} IDS_SEC_AAC_ENC_DESC			"Копирует дорожки с аудиодисков и конвертирует файлы в форматы M4A и AAC."
${LangFileString} IDS_SEC_FLAC_ENC_DESC			"Копирует дорожки с аудиодисков и конвертирует файлы в формат FLAC."
${LangFileString} IDS_SEC_OGG_ENC_DESC			"Копирует дорожки с аудиодисков и конвертирует файлы в формат Ogg Vorbis."
${LangFileString} IDS_GRP_MMEDIA_OUTPUT_DESC	"Подключаемые модули вывода (управляют обработкой и отправкой звука на звуковую карту)."
${LangFileString} IDS_SEC_OUT_DISK_DESC			"Записывает звук в файлы WAV (устарел, но некоторыми пользователями используется вместо аудиоконвертеров)."
${LangFileString} IDS_SEC_OUT_WASAPI_DESC "Вывод Windows Audio (WASAPI) (экспериментально)"
${LangFileString} IDS_SEC_OUT_DS_DESC			"Обеспечивает поддержку вывода DirectSound (стандартный и необходимый подключаемый модуль)."
${LangFileString} IDS_SEC_OUT_WAV_DESC			"Обеспечивает поддержку вывода WaveOut (устарел и более не рекомендуется для использования)."
${LangFileString} IDS_GRP_UIEXTENSION_DESC		"Расширение пользовательского интерфейса."
${LangFileString} IDS_SEC_HOTKEY_DESC			"Позволяет управлять Winamp из любого активного приложения с помощью сочетаний клавиш."
${LangFileString} IDS_SEC_JUMPEX_DESC			"Позволяет организовать в Winamp очередь воспроизведения, а также предоставляет другие возможности."
${LangFileString} IDS_SEC_TRAYCTRL_DESC			"Позволяет управлять Winamp с помощью значков из области уведомлений."
${LangFileString} IDS_SEC_FREEFORM_DESC			"Требуется для поддержки современных обложек (например, Winamp Modern и Bento)."
${LangFileString} IDS_GRP_VISUALIZATION_DESC	"Подключаемые модули зрительных образов."
${LangFileString} IDS_SEC_NSFS_DESC				"Подключаемый модуль зрительных образов Nullsoft Tiny Fullscreen."
${LangFileString} IDS_SEC_AVS_DESC				"Подключаемый модуль зрительных образов Advanced Visualization Studio."
${LangFileString} IDS_SEC_MILKDROP_DESC			"Подключаемый модуль зрительных образов MilkDrop."
${LangFileString} IDS_SEC_MILKDROP2_DESC		"Подключаемый модуль зрительных образов MilkDrop2 (по умолчанию)."
${LangFileString} IDS_SEL_LINEIN_DESC			"Принимает сигнал с линейного входа (например, с микрофона или радиоприемника)."
${LangFileString} IDS_GRP_WALIB_DESC			"Библиотека Winamp."
${LangFileString} IDS_SEC_ML_DESC				"Библиотека Winamp (обязательна)."
${LangFileString} IDS_SEC_ML_TRANSCODE_DESC		"Конвертирует файлы из одного формата в другой."
${LangFileString} IDS_SEC_ML_RG_DESC			"Выравнивает громкость у аудиофайлов."
${LangFileString} IDS_SEC_ML_AUTOTAG_DESC		"Автоматически заполняет теги метаданных (информация берется из базы данных Gracenote)."
${LangFileString} IDS_SEC_ML_WIRE_DESC			"Позволяет загружать подкасты и организовывать на них подписку."
${LangFileString} IDS_SEC_ML_ONLINE_DESC		"Предоставляет доступ к интерактивным службам, включая SHOUTcast, AOL Radio и др."
${LangFileString} IDS_SEC_ML_PLG_DESC			"Создает случайные списки воспроизведения с похожей музыкой (при поддержке Gracenote)."
${LangFileString} IDS_GRP_WALIB_CORE_DESC		"Основные компоненты библиотеки."
${LangFileString} IDS_SEC_ML_LOCAL_DESC			"Позволяет работать с мультимедийными файлами."
${LangFileString} IDS_SEC_ML_PLAYLISTS_DESC		"Позволяет создавать, изменять и сортировать списки воспроизведения."
${LangFileString} IDS_SEC_ML_DISC_DESC			"Предоставляет интерфейс для копирования и записи аудиодисков."
${LangFileString} IDS_SEC_ML_BOOKMARKS_DESC		"Позволяет добавлять в закладки потоки, файлы и папки."
${LangFileString} IDS_SEC_ML_HISTORY_DESC		"Ведет историю воспроизведения потоков и файлов." 
${LangFileString} IDS_SEC_ML_NOWPLAYING_DESC	"Находит в Интернете сведения о проигрываемых композициях."
${LangFileString} IDS_GRP_WALIB_PORTABLE_DESC	"Поддержка портативных плееров."
${LangFileString} IDS_SEC_ML_PMP_DESC			"Основной подключаемый модуль для поддержки портативных плееров (обязателен)."
${LangFileString} IDS_SEC_PMP_IPOD_DESC			"Обеспечивает поддержку плееров iPod®."
${LangFileString} IDS_SEC_PMP_CREATIVE_DESC		"Обеспечивает поддержку плееров Creative® (для Nomad™, Zen™ и MuVo™)."
${LangFileString} IDS_SEC_PMP_P4S_DESC			"Обеспечивает поддержку Microsoft® PlaysForSure® (для всех плееров, совместимых с MTP и P4S)."
${LangFileString} IDS_SEC_PMP_USB_DESC			"Обеспечивает поддержку USB-устройств (для USB-накопителей и большинства плееров)."
${LangFileString} IDS_SEC_PMP_ACTIVESYNC_DESC	"Обеспечивает поддержку Microsoft® ActiveSync® (для смартфонов и планшетов под управлением Windows Mobile®)."
${LangFileString} IDS_SEC_PMP_ANDROID_DESC		"Обеспечивает поддержку Android-устройств."
${LangFileString} IDS_SEC_PMP_WIFI_DESC			"Обеспечивает поддержку Android-устройств, подключенных к сети Wi-Fi."
${LangFileString} IDS_SEC_ML_IMPEX_DESC			"Позволяет экспортировать базу данных Winamp для использования в iTunes и наоборот."
${LangFileString} IDS_SEC_GEN_FRENCHRADIO_DESC	"Слушайте более трехсот французских радиостанций прямо в $(^NameDA) (Virgin radio, NRJ, RTL, Skyrock, RMC и др.)."

${LangFileString} IDS_FIREWALL					"Установка правил для брандмауэра..."
${LangFileString} IDS_UNINSTALL_COMPONENTS_GROUP_DESC	"Удаление $(^NameDA) с компьютера."
${LangFileString} IDS_UNINSTALL_COMPONENTS_FOOTER		"Удаление из папки:$\r$\n$INSTDIR$\r$\n"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER 			"Проигрыватель"
${LangFileString} IDS_UNINSTALL_MEDIA_PLAYER_DESC 		"Удалить все компоненты $(^NameDA), включая стандартные подключаемые модули."
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES 		"Настройки пользователя"
${LangFileString} IDS_UNINSTALL_USER_PREFERENCES_DESC 	"Удалить настройки $(^NameDA), а также настройки всех подключаемых модулей."
${LangFileString} IDS_UNINSTALL_FEEDBACK_CHECKBOX_TEXT	"Отправить отзыв о $(^NameDA)"
${LangFileString} IDS_UNINSTALL_EXPLORER_CHECKBOX_TEXT	"Открыть папку $(^NameDA)"
${LangFileString} IDS_UNINSTALL_FILES_NOT_REMOVED		"$\r$\n$\r$\n$\r$\nПримечание. Удалены не все файлы. Чтобы увидеть их, откройте папку Winamp."
${LangFileString} IDS_UNINSTALL_SUBHEADER				"$(^NameDA) удален с вашего компьютера.$\r$\n$\r$\nНажмите кнопку $\"Готово$\" для выхода из мастера удаления."

!ifdef EXPRESS_MODE
${LangFileString} IDS_EXPRESS_MODE_HEADER "Режим установки"
${LangFileString} IDS_EXPRESS_MODE_SUBHEADER "Выберите режим установки Winamp."
${LangFileString} IDS_EXPRESS_MODE_STANDARD_INSTALL_RADIO "&Обычная"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_INSTALL_TEXT "$(^NameDA) будет установлен со стандартным набором компонентов в папку:$\r$\n'$INSTDIR"
${LangFileString} IDS_EXPRESS_MODE_STANDARD_REINSTALL_TEXT "$(^NameDA) будет установлен с компонентами, выбранными в прошлый раз, в папку:$\r$\n'$INSTDIR
${LangFileString} IDS_EXPRESS_MODE_CUSTOM_INSTALL_RADIO "&Выборочная"
${LangFileString} IDS_EXPRESS_MODE_CUSTOM_INSTALL_TEXT "Можно вручную выбрать компоненты $(^NameDA), которые необходимо \
                                                        установить."
!endif ;