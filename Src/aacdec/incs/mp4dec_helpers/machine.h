/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_helpers/machine.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: machine dependent type definitions
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __MACHINE_H
#define __MACHINE_H

#define inline __inline

#include <math.h>

#ifdef _MSC_VER
#include <basetsd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif 

typedef signed char   INT8;
typedef signed short  INT16;
typedef signed int    INT32;
typedef signed int    INT;
typedef float         FLOAT;
typedef double        DOUBLE;

/* typedef long            LONG;  */
/* typedef unsigned long   ULONG; */
/* typedef unsigned int    UINT;  */
/* typedef  signed int     INT;   */
typedef  unsigned char  UINT8;
typedef  unsigned short UINT16;
typedef  unsigned int   UINT32;

#if defined _MSC_VER
#define ALIGN16 __declspec(align(16))
#elif defined __GNUC__ && !defined __sparc__ && !defined __sparc_v9__ 
#define ALIGN16 __attribute__((aligned(16)))
#else 
#define ALIGN16 
#endif

#ifndef INT64
#if !(defined(WIN32) || defined(WIN64))
#define INT64  long long
#else
#define INT64 __int64
#endif

#endif


static inline double FhgSqrt(DOUBLE a){
  return(sqrt(a));
}
static inline double FhgExp(DOUBLE a){
  return(exp((double) a));
}
static inline double FhgPow(DOUBLE a, DOUBLE b){
  return(pow((double) a, (double) b));
}
static inline double FhgSin(DOUBLE a){
  return(sin((double) a));
}
static inline double FhgFloor(DOUBLE a){
  return(floor((double) a));
}
static inline double FhgCos(DOUBLE a){
  return(cos((double) a));
}
static inline double FhgFabs(DOUBLE a){
  return(fabs((double) a));
}
static inline double FhgTan(DOUBLE a){
  return(tan((double) a));
}
static inline double FhgCeil(DOUBLE a){
  return(ceil((double) a));
}
static inline double FhgLog(DOUBLE a){
  return(log((double) a));
}

#ifdef __cplusplus
}
#endif

#endif  /* #define __MACHINE_H */
