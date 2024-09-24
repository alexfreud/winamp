/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   filename: regtypes.h
 *   project : -
 *   author  : Stefan Gewinner gew@iis.fhg.de
 *   date    : 1998-06-08
 *   contents/description: absolute minimum to make l3reg.h compile without windef.h
 *
 * $Header: /cvs/root/nullsoft/Replicant/jni/nsmp3/regtypes.h,v 1.1 2010/11/17 20:46:05 audiodsp Exp $
 *
\***************************************************************************/

/* the typedefs should be in place if we already got windows.h included */

#ifndef _INC_WINDOWS

#ifndef __REGTYPES_H__
#define __REGTYPES_H__

/*-------------------------------------------------------------------------*/

#ifdef _MSC_VER
  #pragma warning(disable:4103)
  #pragma pack(push, 1) /* assume byte packing throughout */
#endif

/*-------------------------------------------------------------------------*/

#define FAR
#define NEAR

typedef unsigned long  DWORD ;
typedef unsigned short WORD ;

/*
 *  extended waveform format structure used for all non-PCM formats. this
 *  structure is common to all non-PCM formats.
 */
typedef struct tagWAVEFORMATEX
{
    WORD        wFormatTag;         /* format type */
    WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
    DWORD       nSamplesPerSec;     /* sample rate */
    DWORD       nAvgBytesPerSec;    /* for buffer estimation */
    WORD        nBlockAlign;        /* block size of data */
    WORD        wBitsPerSample;     /* number of bits per sample of mono data */
    WORD        cbSize;             /* the count in bytes of the size of */
                                    /* extra information (after cbSize) */
} WAVEFORMATEX, *PWAVEFORMATEX, NEAR *NPWAVEFORMATEX, FAR *LPWAVEFORMATEX ;

typedef const WAVEFORMATEX FAR *LPCWAVEFORMATEX ;

/*-------------------------------------------------------------------------*/

#ifdef _MSC_VER
  #pragma pack(pop) /* revert to previous packing */
#endif

/*-------------------------------------------------------------------------*/

#endif

#endif
