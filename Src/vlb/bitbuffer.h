/* $Header: /cvs/root/winamp/vlb/bitbuffer.h,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: bitbuffer.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: memory input class with transport format
 *
\***************************************************************************/

#ifndef __BITBUFFER_H__
#define __BITBUFFER_H__

#include "bitstream.h"
#include "aacdecoderapi.h"


class CStreamInfo ;

/** A Memory-Based Input Class.

    This class implements a memory-based \Ref{CDolbyBitStream} interface with an ADTS
    transport layer. It performs basic detection and synchronization work.
*/

class CDolbyBitBuffer : public CDolbyBitStream
{

public :

  CDolbyBitBuffer () ;
  virtual ~CDolbyBitBuffer () ;

  // implementation of the CDolbyBitStream interface

  virtual void ByteAlign (void) ;
  virtual void PushBack (int n) ;
  virtual long Get (int n) ;

  unsigned int GetBitState() {return m_ValidBits;}
  virtual void DecrementBlocks() { m_BlocksLeftInFrame--;}
  virtual void ResetBlocks() {m_BlocksLeftInFrame=0;}
  virtual void SetPositionMarker (MarkerPosition position) ;

  /** Data Feed Method.

      This method feeds a number of input bytes from an external source into
      the modulo bitbuffer that the decoder will read from. The call consumes
      up to cbSize bytes from a given array pBuf. cbValid will be updated
      to indicate the number of valid bytes left in pBuf. If cbValid
      equals zero, all the data in the input array has been consumed and
      the buffer should be refilled from the external source.

      @param pBuf    Pointer to input buffer with bitstream data.
      @param cbSize  Total size of pBuf input buffer array.
      @param cbValid Number of valid bytes left in pBuf array.
  */

  virtual void Feed (unsigned char pBuf [], unsigned int cbSize, unsigned int &cbValid) ;

  /** Bitstream Synchronization Method.

      Call this method to see if there is decodable data in this bitbuffer
      object. Only ever let the decoder read a frame from this object if
      the IsDecodableFrame() method returned true. In addition, this method
      will fill in a \Ref{CStreamInfo} object on success to indicate the detected
      bitstream parameters.

      @param info The configuration information.
  */

  bool IsDecodableFrame (CStreamInfo &info) ;



  /** CRC Evaluation Method.

      Call this method to see if the optional CRC error protection checksum
      carried in the ADTS transport header matches the most recently decoded
      audio frame.

      If error protection information is of concern, this method must be
      called immediately after the decoder has read an entire audio frame
      from this bitbuffer object.
  */

  void IsCrcConsistent (unsigned int *crcValue) ;

  /** EOF Notification Method.

      After being notified by this method, the bitbuffer will throw an
      EEndOfFile exception the next time it runs out of data.
  */

  void SetEOF (void)
  {
	  m_EOF = true ;
  }
  
  bool EndOf(void)
  {
	  return m_EOF;
  }

  virtual void SetFrameReadButNotDecoded(void)
  {
	  m_bFrameReadButNotDecoded = true;
  }
  
  virtual void ClearFrameReadButNotDecoded(void)
  {
	  m_bFrameReadButNotDecoded = false;
  }

  virtual bool FrameReadButNotDecoded(void)
  {
	  return m_bFrameReadButNotDecoded;
  }

  virtual unsigned int GetBitCount(void)
  {
	  return m_BitCnt;
  }

  /** Initialization/Reset Method.

      Clears the buffer's internal memory and removes any possibly pending data.
  */

  virtual void Initialize (void) ;

  virtual unsigned int GetFrameLength(){return frame_length;}
  virtual unsigned int GetNRDB(){return number_of_raw_data_blocks_in_frame;}

  long GetBytesSkipped(void) { return bytes_skipped; }
  void ClearBytesSkipped(void);
  void HoldBytesSkipped(void) { hold_bytes_skipped = true; }

protected :

  enum
  {
    BufferSize = 8192,
    BufferBits = BufferSize*8,

    InvalidCrcValue = 0xFFFFFFFF,
    MaximumMarkers = 25*4
  } ;

  unsigned int m_ValidBits;
  unsigned int m_ReadOffset;
  unsigned int m_BitCnt;
  unsigned int m_BitNdx;

  unsigned int m_BlocksLeftInFrame ;
  unsigned int m_FrameCrcValue ;

  bool m_EOF ;
  bool m_bFrameReadButNotDecoded;

  unsigned char m_Buffer [BufferSize] ;

  // adts_fixed_header

  unsigned int id;
  unsigned int layer;
  unsigned int protection_absent;
  unsigned int profile;
  unsigned int sampling_frequency_index;
  unsigned int private_bit;
  unsigned int channel_configuration;
  unsigned int original_copy;
  unsigned int home;

  // adts_variable_header

  unsigned int copyright_identification_bit;
  unsigned int copyright_identification_start;
  unsigned int frame_length;
  unsigned int buffer_fullness;
  unsigned int last_buffer_fullness;
  unsigned int last_frame_length;
  unsigned int number_of_raw_data_blocks_in_frame;

  // used to track # of bytes skipped 
  // during a re-synchronization

  long bytes_skipped;
  bool hold_bytes_skipped;

  DECLARE_EXCEPTION(EUnimplemented, AAC_UNIMPLEMENTED, "Unimplemented Feature Used") ;

  // bitrate averaging helper class

  class CAverageNumber
  {
    enum
    {
      MaximumSize = 16
    } ;

  public :

    CAverageNumber ()
    {
      m_Index = 0 ;
	  m_ActualCount=0;
      for (int i = 0 ; i < MaximumSize ; i++)
        m_Value [i] = 0 ;
    }

    ~CAverageNumber () {}

    CAverageNumber &operator += (unsigned int val)
    {
      m_Value [m_Index++ % MaximumSize] = val ;
	  if(m_ActualCount<MaximumSize) m_ActualCount++;
      return *this ;
    }

    operator unsigned int() const
    {
      unsigned int val = 0 ;

      for (unsigned int i = 0 ; i < m_ActualCount ; i++)
        val += m_Value [i] ;
      if(m_ActualCount) val/=m_ActualCount;
	  else val=0;
      return (val) ;
    }

  protected :

    unsigned int m_Value [MaximumSize] ;
    unsigned int m_Index ;
	unsigned int m_ActualCount;
  } ;

  CAverageNumber m_ActualBitrate ;


  // crc

  void UpdateCrc (unsigned int &crcValue, int numberOfBits, int minimumBits = 0) ;

  class CMarker
  {

  public :

    CMarker () {}
    ~CMarker () {}

    CMarker (MarkerPosition pos, unsigned int validBits, unsigned int bitCnt, unsigned int bitNdx)
    {
      what = pos ;

      m_ValidBits = validBits ;
      m_BitCnt = bitCnt ;
      m_BitNdx = bitNdx ;
    }

    MarkerPosition what ;

    unsigned int m_elementBits ;

    unsigned int m_ValidBits;
    unsigned int m_BitCnt;
    unsigned int m_BitNdx;

  } ;

  CMarker m_MarkerList [MaximumMarkers] ;
  unsigned int m_Markers ;

  void RestoreMarker (CMarker &marker)
  {
    m_ValidBits = marker.m_ValidBits ;
    m_BitCnt    = marker.m_BitCnt ;
    m_BitNdx    = marker.m_BitNdx ;
  }

} ;

#endif
