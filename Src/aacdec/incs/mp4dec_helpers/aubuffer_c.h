/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   $Header: /cvs/root/winamp/aacdec/incs/mp4dec_helpers/aubuffer_c.h,v 1.3 2012/05/08 20:16:50 audiodsp Exp $
 *   project : MPEG-4 Audio Decoder
 *   contents/description: access unit module public interface
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this 
 *   software and/or program, or any portion of it, may result in severe 
 *   civil and criminal penalties, and will be prosecuted to the maximum 
 *   extent possible under law.
 *
\***************************************************************************/

#ifndef __AUBUFFERC_H__
#define __AUBUFFERC_H__

#include "mp4dec_helpers/err_code.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(WIN64)
  #pragma pack(push, 8)
#endif


/* Opaque declaration of access unit handle */
struct CAccessUnit;
typedef struct CAccessUnit* CAccessUnitPtr;

CAccessUnitPtr MP4AUDIODECAPI CAccessUnit_Create(const unsigned char *pBuffer, const unsigned int size);

MP4_RESULT MP4AUDIODECAPI CAccessUnit_Reset (const CAccessUnitPtr self);

MP4_RESULT MP4AUDIODECAPI CAccessUnit_Assign(const CAccessUnitPtr self,
                                             const unsigned char  *pBuffer,
                                             const unsigned int    size);

MP4_RESULT MP4AUDIODECAPI CAccessUnit_GetBitCount(const CAccessUnitPtr self, unsigned int* nBits);
MP4_RESULT MP4AUDIODECAPI CAccessUnit_GetValidBits(const CAccessUnitPtr self, unsigned int* nBits);

MP4_RESULT MP4AUDIODECAPI CAccessUnit_MarkAsInvalid(const CAccessUnitPtr self);

int MP4AUDIODECAPI CAccessUnit_IsValid(const CAccessUnitPtr self);

MP4_RESULT MP4AUDIODECAPI CAccessUnit_Destroy(CAccessUnitPtr *self);

#if defined(WIN32) || defined(WIN64)
  #pragma pack(pop)
#endif


#ifdef __cplusplus
}
#endif

#endif
