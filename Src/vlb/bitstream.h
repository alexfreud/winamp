/* $Header: /cvs/root/winamp/vlb/bitstream.h,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: bitstream.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: input abstract base class
 *
\***************************************************************************/

#ifndef __VLB_BITSTREAM_H__
#define __VLB_BITSTREAM_H__

#include "exception.h"

/** General Bitstream Input Base Class.

    This abstract base class defines the interface that the \Ref{CAacDecoder} object will
    read all of its bitstream input data from. Specialized classes for specific
    application scenarios have to be derived from this interface.
*/

class CDolbyBitStream
{

public :

  /// Object Constructor.

  CDolbyBitStream ()
  {
    m_AdifHeaderPresent = false ;
	m_RiffHeaderPresent = false ;
	m_IsADTSCompliant = false;
  }

  /// Object Destructor.

  virtual ~CDolbyBitStream () {}

  /** Byte Alignment Method.

      This method performs the byte_alignment() syntactic method on the input stream,
      i.e. some bits will be discarded so that the next bits to be read would be aligned
      on a byte boundary with respect to the last call to ByteAlign().
  */

  virtual void ByteAlign (void) = 0 ;

  /** Push Back Method.

      This method ungets a number of bits erroneously read by the last Get() call.
      NB: The number of bits to be stuffed back into the stream may never exceed the
      number of bits returned by the immediately preceding Get() call.

      @param n  The number of bits to be pushed back.
  */

  virtual void PushBack (int n) = 0 ;

  /** Get Bits Method.

      This method returns a number of sequential bits from the input bitstream.
      This is the one and only read operation that the decoder will ever perform,
      i.e. _all_ the data goes through this call.

      @param n  The number of bits to be retrieved.
  */

  virtual long Get (int n) = 0 ;

  /** ADIF detection Method.

      This method indicates whether the decoder should read in and parse an ADIF header
      before starting to decode the first frame of audio data.
  */

  bool IsAdifHeaderPresent (void)
  {
    return m_AdifHeaderPresent ;
  }


  /** RIFF/ADTS detection Method.

      This method indicates whether the decoder should read in and parse a RIFF header
      before starting to decode the first frame of audio data.
  */

  bool IsRiffHeaderPresent (void)
  {
	  return m_RiffHeaderPresent;
  }

  bool IsADTSCompliant (void)
  {
	return m_IsADTSCompliant;
  }

  // CRC Markers for SetPositionMarker ().

  typedef enum
  {
    ChannelElementStart,
    ChannelElementStop,
    AncillaryElementStart,
    AncillaryElementStop,
    SecondIndividualChannelStart,
    CurrentPosition
  } MarkerPosition ;

  /** Position Marker Setting Method.

      The decoder core can call this method to set position markers
      in the bitstream. this facility can be used to implement
      error-protection mechanisms in a derived class, for example,
      if the underlying transport layer supports such a feature.
  */

  virtual void SetPositionMarker (MarkerPosition position) {}
  virtual void DecrementBlocks(){}
  virtual void ResetBlocks() {}
  virtual unsigned int GetFrameLength(){return 0;}	  //UNKNOWN IN ADIF
  virtual unsigned int GetNRDB(){return 0;}           //UNKNOWN IN ADIF
  virtual void SetFrameReadButNotDecoded(void){}
  virtual void ClearFrameReadButNotDecoded(void){}
  virtual bool FrameReadButNotDecoded(void){return false;}

  virtual unsigned int GetBitCount(void) {return 0;};


protected :

  DECLARE_EXCEPTION(EDoesNotExist, AAC_DOESNOTEXIST, "Bitstream does not exist") ;
  DECLARE_EXCEPTION(EEndOfFile,    AAC_ENDOFSTREAM,  "End of file reached") ;

  bool	m_AdifHeaderPresent;
  bool	m_RiffHeaderPresent;
  bool  m_IsADTSCompliant;

} ;

#endif
