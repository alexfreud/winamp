#ifndef __API_DEF_CFG_H
#define __API_DEF_CFG_H

#define WASABINOMAINAPI

#ifdef WASABI_COMPILE_APP
# define WASABI_API_APP         applicationApi
#endif

#ifdef WASABI_COMPILE_SVC
# define WASABI_API_SVC         serviceApi
#endif

#ifdef WASABI_COMPILE_SYSCB
# define WASABI_API_SYSCB       sysCallbackApi
#endif

#ifdef WASABI_COMPILE_COMPONENTS
# define WASABI_API_COMPONENT   componentApi
#endif

#ifdef WASABI_COMPILE_SCRIPT
# define WASABI_API_MAKI        makiApi
#endif

#ifdef WASABI_COMPILE_UTF
# define WASABI_API_UTF         utfApi
#endif

#ifdef WASABI_COMPILE_WND
# define WASABI_API_WND         wndApi
#endif

#ifdef WASABI_COMPILE_IMGLDR
# define WASABI_API_IMGLDR      imgLoaderApi
#endif

#ifdef WASABI_COMPILE_FILEREADER
# define WASABI_API_FILE        fileApi
#endif

#ifdef WASABI_COMPILE_TIMERS
# define WASABI_API_TIMER       timerApi
#endif

#ifdef WASABI_COMPILE_WNDMGR
# define WASABI_API_WNDMGR      wndManagerApi
#endif

#ifdef WASABI_COMPILE_SKIN
# define WASABI_API_SKIN        skinApi
#endif

#ifdef WASABI_COMPILE_METADB
# define WASABI_API_METADB      metadbApi
#endif

#ifdef WASABI_COMPILE_LOCALES
# define WASABI_API_LOCALE      localeApi
#endif

#ifdef WASABI_COMPILE_CONFIG
# define WASABI_API_CONFIG      configApi
#endif

#ifdef WASABI_COMPILE_FONTS
# define WASABI_API_FONT        fontApi
// This sets the static font renderer. If you are compiling with api_config, the attribute to set is { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } }, "Font Renderer"
# if defined(WASABI_FONT_RENDERER_USE_WIN32)
#  define WASABI_FONT_RENDERER "" // "" is Win32
# elif defined(WASABI_FONT_RENDERER_USE_FREETYPE)
#  define WASABI_FONT_RENDERER "Freetype" // Freetype lib
# else
#  define WASABI_FONT_RENDERER "" // "" default for OS
# endif
#endif

#ifdef WASABI_COMPILE_MEMMGR
# define WASABI_API_MEMMGR      memmgrApi
#endif

#ifdef WASABI_COMPILE_XMLPARSER
# define WASABI_API_XML         xmlApi
#endif

#ifdef WASABI_COMPILE_MEDIACORE
# define WASABI_API_MEDIACORE   coreApi
#endif

#ifdef WASABI_COMPILE_TEXTMODE
# define WASABI_API_TEXTMODE    textmodeApi
#endif

#ifdef LINUX
# define WASABI_COMPILE_LINUX
# define WASABI_API_LINUX       linuxApi
#endif

#ifdef WASABI_COMPILE_STATSWND
# if defined(_DEBUG) | defined(WASABI_DEBUG)
#  define WASABI_COMPILE_STATSWND
#  ifndef WASABI_DEBUG
#   define WASABI_DEBUG
#  endif
# endif
#endif

#ifdef WASABI_COMPILE_APP
# include <api/application/api_application.h>
#endif

#ifdef WASABI_COMPILE_SVC
# include <api/service/api_service.h>
#endif

#ifdef WASABI_COMPILE_SYSCB
# include <api/syscb/api_syscb.h>
#endif

#ifdef WASABI_COMPILE_MEMMGR
# include <api/memmgr/api_memmgr.h>
#endif

#ifdef WASABI_COMPILE_SCRIPT
# include <api/script/api_maki.h>
#endif

#ifdef WASABI_COMPILE_FONTS
# include <api/font/api_font.h>
#endif

#ifdef WASABI_COMPILE_WND
# include <api/wnd/api_wnd.h>
#endif

#ifdef WASABI_COMPILE_IMGLDR
# include <api/imgldr/api_imgldr.h>
#endif

#ifdef WASABI_COMPILE_FILEREADER
# include <api/filereader/api_filereader.h>
#endif

#ifdef WASABI_COMPILE_TIMERS
# include <api/timer/api_timer.h>
#endif

#ifdef WASABI_COMPILE_WNDMGR
# include <api/wndmgr/api_wndmgr.h>
#endif

#ifdef WASABI_COMPILE_LOCALES
# include <api/locales/api_locales.h>
#endif

#ifdef WASABI_COMPILE_CONFIG
# include <api/config/api_config.h>
#endif

#ifdef WASABI_COMPILE_SKIN
# include <api/skin/api_skin.h>
#endif

#ifdef WASABI_COMPILE_MAKIDEBUG
# include <api/script/debugger/api_makidebug.h>
#endif

#ifdef WASABI_COMPILE_TEXTMODE
# include <api/textmode/api_textmode.h>
#endif

#ifdef WASABI_API_LINUX
#include <api/linux/api_linux.h>
#endif

#endif
