/***************************************************************************\
*
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
*                        All Rights Reserved
*
*   filename: mp3ancofl.h
*   project : MPEG Decoder
*   author  : Dieter Weninger
*   date    : 2003-05-14
*   contents: ancillary data and original file length - HEADER
*
\***************************************************************************/
#ifndef __MP3ANCOFL_H__
#define __MP3ANCOFL_H__

#include "mpegbitstream.h"

#define ID_OFL 0xB
#define VERSION_0_LEN 8  /* bytes */
#define VERSION_1_LEN 10 /* bytes */

class CMp3AncOfl
{
 public:
  CMp3AncOfl(CBitStream &__Db);
  ~CMp3AncOfl();

  void Reset(void);

  int	       getVersion(void);
  unsigned int getTotalLength(void);
  unsigned int getCodecDelay(void);
  unsigned int getAddDelay(void);

  bool validOfl(void);

  void fetchOfl(int oflOn,
                CBitStream &Db,
                int beforeScf,
                unsigned int* startDelay,
                unsigned int* totalLength);

  int  readAnc(unsigned char *ancBytes,
               CBitStream  &Db,
               const int numAncBits);

  int doReadBytes(){return m_readBytes;}

 private:
  void crcOfl(unsigned short crcPoly,
              unsigned short crcMask,
              unsigned long *crc,
              unsigned char byte);

  void cleanUp(void);
  bool isFhGAnc( int size);
  bool readOfl(CBitStream &Db, int beforeScaleFactors);
  bool isOfl(void);
  bool justSearched(void);
  int  toSkip(void);
  void getOfl(CBitStream &Db, const int len);

  CBitStream     &m_Db;   // dynamic buffer

  unsigned char oflArray[10];

  bool m_valid;
  bool m_searched;
  bool m_semaphor;
  bool m_FhGAncChecked;
  bool m_collecting;
  bool m_mp3pro;

  unsigned char* m_FhGAncBuf;
  unsigned char* m_tmpAncBuf;

  int m_pFhGAncBuf;
  int m_FhGAncBufSize;

  // flag signalling byte- or bit-wise reading
  int m_readBytes;

};

#endif /* __MP3ANCOFL_H__ */
