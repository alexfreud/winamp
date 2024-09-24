/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: bitstream.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1997-12-05
 *   contents/description: generic bitbuffer - HEADER
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/18 18:22:02 $
 * $Id: bitstream.h,v 1.4 2011/01/18 18:22:02 audiodsp Exp $
 */

#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

/* ------------------------ includes --------------------------------------*/

/*-------------------------- defines --------------------------------------*/

class CGioBase;

/*-------------------------------------------------------------------------*/

//
// Bitstream input class.
//
//    This class defines the interface that the mp3 decoder object will
//    read all of its bitstream input data from.
//

class CBitStream
{
public:

  CBitStream(int cbSize);
  CBitStream(unsigned char *pBuf, int cbSize, bool fDataValid = false);
  virtual ~CBitStream();

  virtual void   Reset();
	bool ByteAligned() const { return !(m_BitNdx & 7); }
  bool           ResetOccurred()           { return m_ResetOccurred;  }
  void           SetResetState(bool state) { m_ResetOccurred = state; }

  void           Connect(CGioBase *pGB);

  void           ResetBitCnt()     { m_BitCnt = 0;    }
  int            GetBitCnt() const { return m_BitCnt; }

  unsigned int   GetBits(unsigned int nBits); // gets 16 bits or less
	unsigned int   GetBits8(unsigned int nBits); // gets 8 bits or less
  unsigned int   Get1Bit();
  unsigned long  Get32Bits();

  bool           Ff(int nBits)     { return ( (nBits > 0) ? Seek(nBits)  : false); }
  bool           Rewind(int nBits) { return ( (nBits > 0) ? Seek(-nBits) : false); }
  bool           Seek(int nBits)
    {
    m_BitCnt    += nBits;
    m_ValidBits -= nBits;
    m_BitNdx     = (m_BitNdx+nBits) & m_bitMask;
    return true;
    }

  int            GetValidBits() const { return m_ValidBits; }
  int            GetFree()      const;

  void           SetEof();
  int            Fill(const unsigned char *pBuf, int cbSize);
  int            Fill(CBitStream &Bs, int cbSize);

protected:

  int            Refill();
  bool           IsEof()       const;
  bool           IsConnected() const;

private:

  CGioBase      *m_pGB;           // I/O object
  int            m_nBytes;        // size of buffer in bytes
	int						 m_mask;
  int            m_nBits;         // size of buffer in bits
	int					   m_bitMask;
  int            m_ValidBits;     // valid bits in buffer
  int            m_ReadOffset;    // where to write next
  int            m_BitCnt;        // bit counter
  int            m_BitNdx;        // position of next bit in byte
  bool           m_fEof;          // indication of input eof
  unsigned char *m_Buf;           // the buffer
  bool           m_fBufferIntern; // did we allocate the buffer ourselves
  bool           m_ResetOccurred; // reset just occurred, only for dynamic buffer used
};

/*-------------------------------------------------------------------------*/
#endif
