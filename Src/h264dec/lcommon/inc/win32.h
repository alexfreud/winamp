
/*!
 ************************************************************************
 *  \file
 *     win32.h
 *
 *  \brief
 *     win32 definitions for H.264 encoder.
 *
 *  \author
 *
 ************************************************************************
 */
#ifndef _H264_WIN32_H_
#define _H264_WIN32_H_
#pragma once

# include <fcntl.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <assert.h>

#if defined(WIN32)
# include <io.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <windows.h>
#ifndef strcasecmp
# define strcasecmp _strcmpi
#endif

# define  snprintf _snprintf
# define  open     _open
# define  close    _close
# define  read     _read
# define  write    _write
#ifndef lseek
# define  lseek    _lseeki64
#endif
# define  fsync    _commit
# define  tell     _telli64
# define  TIMEB    _timeb
# define  TIME_T    LARGE_INTEGER
# define  OPENFLAGS_WRITE _O_WRONLY|_O_CREAT|_O_BINARY|_O_TRUNC
# define  OPEN_PERMISSIONS _S_IREAD | _S_IWRITE
# define  OPENFLAGS_READ  _O_RDONLY|_O_BINARY
# define  inline   _inline
# define  forceinline __forceinline
#else
# include <unistd.h>
# include <sys/time.h>
# include <sys/stat.h>
# include <time.h>

# define  TIMEB    timeb
# define  TIME_T   struct timeval
# define  tell(fd) lseek(fd, 0, SEEK_CUR)
# define  OPENFLAGS_WRITE O_WRONLY|O_CREAT|O_TRUNC
# define  OPENFLAGS_READ  O_RDONLY
# define  OPEN_PERMISSIONS S_IRUSR | S_IWUSR

# if __STDC_VERSION__ >= 199901L
   /* "inline" is a keyword */
# else
#  define inline /* nothing */
# endif
# define  forceinline inline
#endif

#if defined(WIN32) && !defined(__GNUC__)
typedef __int64   int64;
typedef unsigned __int64   uint64;
# define FORMAT_OFF_T "I64d"
# ifndef INT64_MIN
#  define INT64_MIN        (-9223372036854775807i64 - 1i64)
# endif
#else

typedef long long int64;
typedef unsigned long long  uint64;
# define FORMAT_OFF_T "lld"
# ifndef INT64_MIN
#  define INT64_MIN        (-9223372036854775807LL - 1LL)
# endif
#endif

void   gettime(TIME_T* time);
int64 timediff(TIME_T* start, TIME_T* end);
int64 timenorm(int64 cur_time);

#endif
