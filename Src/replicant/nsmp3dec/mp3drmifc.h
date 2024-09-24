/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3drmifc.h
 *   project : MPEG Decoder
 *   author  :
 *   date    : 2004-12-06
 *   contents/description: DRM Interface
 *
 *
\***************************************************************************/

#ifndef __MP3DRMIFC_H__
#define __MP3DRMIFC_H__

#include "mp3sscdef.h"

SSC MP3DECAPI mp3decGetScfBuffer(MP3DEC_HANDLE  handle,
                                 const unsigned char** ppBuffer,
                                 unsigned int* pBufSize);

#endif  /* __MP3DRMIFC_H__ */
