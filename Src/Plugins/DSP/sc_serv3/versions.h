#pragma once
#ifndef versions_H_
#define versions_H_

#define SERV_OSNAME "posix"

#ifdef _WIN32
	#undef SERV_OSNAME
	#ifndef _WIN64
		#define SERV_OSNAME "win32"
		#define SERV_UPDATE_NAME "win32"
	#else
		#define SERV_OSNAME "win64"
		#define SERV_UPDATE_NAME "win64"
	#endif
#endif

#ifdef __APPLE_CC__
	#undef SERV_OSNAME
	#define SERV_OSNAME "mac"
#endif

#ifdef PLATFORM_LINUX
	#undef SERV_OSNAME
	#ifndef __LP64__
		#define SERV_OSNAME "posix(linux x86)"
		#define SERV_UPDATE_NAME "linux_x86"
	#else
		#define SERV_OSNAME "posix(linux x64)"
		#define SERV_UPDATE_NAME "linux_x64"
	#endif
#endif

#ifdef PLATFORM_BSD
	#undef SERV_OSNAME
	#define SERV_OSNAME "posix(bsd)"
	#define SERV_UPDATE_NAME "bsd"
#endif

#ifdef PLATFORM_ARMv6
	#undef SERV_OSNAME
	#define SERV_OSNAME "armv6(rpi)"
	#define SERV_UPDATE_NAME "rpi"
#endif

#ifdef PLATFORM_ARMv7
	#undef SERV_OSNAME
	#define SERV_OSNAME "armv7(rpi2)"
	#define SERV_UPDATE_NAME "rpi2"
#endif

#endif
