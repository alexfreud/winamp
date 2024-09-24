/* $Header: /cvs/root/winamp/vlb/channel.h,v 1.1 2009/04/28 20:21:08 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: channel.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: independent channel stream object
 *
 ***************************************************************************/

#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "block.h"
#include "bitsequence.h"
#include "channelinfo.h"
#include "overlapadd.h"
#include "stereo.h"

#ifdef MAIN_PROFILE
#include "prediction.h"
#endif

#include "dolby_imdct.h"
#include "DolbyPayload.h"
#include "audio_io.h"
#include "audio_io_dsp.h"

class CDolbyBitStream ;
class CStreamInfo ;
class AudioIODSP ;

/** Base Class For Channel Elements.

    This abstract base class defines the interface and some common internal
    helper functions for channel elements.
*/

class CChannelElement
{

public :

  CChannelElement (CDolbyBitStream &bs) ;
  virtual ~CChannelElement () ;

  /** Data type for time-domain output samples.

      Change this to float if you prefer the decoder core to deliver its time 
      domain output data as floating point rather than 16 bit short integer.
  */

  typedef short PcmType ;

  /** Main serialization method.

      This method completely retrieves the channel element's state from the
      associated bitstream object. All syntactic element members will be
      called to perform their respective bitstream parsing and read in 
      their values from the MPEG stream.

      This interface method must be implemented by any derived class.

      @param info This container objects holds relevant information on
                  the current stream's sampling rate, profile etc. which
                  is required to correctly interpret some bitfields.
  */

  virtual void Read (const CStreamInfo &) = 0 ;

  /** Main decoding method.

      This is the actual workhorse function, it performs all processing
      required for reconstruction of one complete frame of output data.
      All active tools will be called to perform their work on the
      spectral coefficients of our \Ref{CBlock} member object(s) and frequency
      to time domain transform will be performed on all subwindows.

      @param pcmbuf The output array to hold one frame of decoded data.
      @param stride The interleave stride used to write the audio chunks.
  */

  virtual void Decode (AudioIOControl *poAudioIO, CStreamInfo &info, int stride = 1) = 0 ;

  /** Number of channels information method.

      This method simply returns the number of output channels that the
      implementing class will provide.

      @return Number of channels.
  */

  virtual int GetNumberOfChannels (void)
  {
    return 0 ;
  }

  /** Pseudo-EQ configuration method.

      This is really just a gimmick and not encouraged for use in any
      serious application. A coefficient mask can be provided that will
      be applied to the spectral bins of our \Ref{CBlock} Members. Will
      introduce bad aliasing artefacts.

      @param wantEQ Flag to turn pseudo-EQ on/off. False by default.
      @param Mask   An array of CBlock::EqualizationMaskLength coefficients.
  */

  void SetEqualization (bool wantEQ, float Mask []) ;

protected :

  CBlock *ReadICS (CChannelInfo &ics_info, unsigned long *memory) ;

  enum
  {
    BlockMemorySize = ((sizeof (CLongBlock) > sizeof (CShortBlock)) ?
                        sizeof (CLongBlock) : sizeof (CShortBlock)) /
                        sizeof (unsigned long) + 1
  } ;

protected :

  CDolbyBitStream &m_bs ;

  CVLBBitSequence m_GlobalGain ;

  bool m_WantEqualizer ;
  bool m_WantSpectralData ;

  float m_EqualizerMask [CBlock::EqualizationMaskLength] ;

  AudioIODSP	* m_poAudioDSP;

} ;

/** SCE Single Channel Element.

    This class holds the data for one single audio channel, as represented
    by the ID_SCE bitstream element, and defines its parsing and decoding order.

    It consists of one \Ref{CBlock} member that holds the actual spectral bins for
    a long block or a short block, and a time domain overlap-add buffer.
    This class implements the \Ref{CChannelElement} interface.
*/
	
#ifdef MAIN_PROFILE
/*	If MAIN_PROFILE is enabled, ID_SCE also includes an
    instance of the \Ref{CPrediction} class to hold the state information for the
    optional main profile backward-adaptive prediction tool. The predictor state
    and overlap-add buffers are persistent between calls, so instances of this
    class are not stateless and should only be used for one specific element
    instance tag.
    
*/
#endif

class CSingleChannel : public CChannelElement
{

public :

  CSingleChannel (CDolbyBitStream &bs) ;
  ~CSingleChannel () ;

  virtual void Read (const CStreamInfo &) ;
  virtual void Decode (AudioIOControl *poAudioIO, CStreamInfo &info, int stride = 1) ;
  void DecodeDolby (AudioIOControl *poAudioIO,
					DOLBY_PAYLOAD_STRUCT *psDSEInfo, 
					CStreamInfo &info,
					int stride = 1) ;
  virtual int GetNumberOfChannels (void)
  {
    return 1 ;
  }
	
  //MSV:
  CBlock* GetBlock(){return m_Block;}
  CChannelInfo *GetChannelInfo(){return &m_IcsInfo;} 
protected :

  CChannelInfo m_IcsInfo ;
  CBlock *m_Block ;

  COverlapAddBuffer m_Previous ;

#ifdef MAIN_PROFILE
  CPrediction m_Prediction ;
#endif

  unsigned long m_BlockMemory [CChannelElement::BlockMemorySize] ;
private:
	IMDCTObject *poIMDCTObject;
	IMDCTObject *poNormalIMDCTObject;
	float **ppfData;
	short **ppshData;
} ;

/** CPE Channel Pair Element.

    This class holds the data for one pair of audio channels, as represented
    by the ID_CPE bitstream element, and defines their parsing and decoding order.

    It consists of a Left/Right-Pair of \Ref{CBlock} members that hold the actual
    spectral bins for a long block or a short window sequence, a pair of
    time domain overlap-add buffers and prediction tools, plus a joint stereo
    processing tool and associated side information. The predictor state
    and overlap-add buffers are persistent between calls, so instances of this
    class are state-dependent and should only be used for one specific element
    instance tag.

    This class implements the \Ref{CChannelElement} interface.

*/

class CChannelPair : public CChannelElement
{

public :

  CChannelPair (CDolbyBitStream &bs) ;
  ~CChannelPair () ;

  virtual void Read (const CStreamInfo &) ;
  virtual void Decode (AudioIOControl *poAudioIO, CStreamInfo &info, int stride = 1) ;
  void DecodeDolby (AudioIOControl *poAudioIO,
					DOLBY_PAYLOAD_STRUCT *psDSEInfo,
					CStreamInfo &info,
					int stride = 1) ;
  virtual int GetNumberOfChannels (void)
  {
    return CChannelPair::Channels ;
  }
  
  //MSV:
  CBlock* GetLeftBlock(){return m_Block[0];}
  CBlock* GetRightBlock(){return m_Block[1];}
  CChannelInfo *GetChannelInfo(){return m_IcsInfo;} 
protected :

  enum
  {
    L = 0,
    R = 1,
    Channels = 2
  } ;

  CVLBBitSequence m_CommonWindow ;

  CChannelInfo m_IcsInfo [Channels] ;
  CBlock *m_Block [Channels] ;

  COverlapAddBuffer m_Previous [Channels] ;

#ifdef MAIN_PROFILE
  CPrediction m_Prediction [Channels] ;
#endif

  CJointStereo m_JointStereo ;

  unsigned long m_BlockMemory [Channels][CChannelElement::BlockMemorySize] ;
private:
	IMDCTObject **ppoIMDCTObject;
	IMDCTObject **ppoNormalIMDCTObject;
	float **ppfData;
	short **ppshData;
} ;

  DECLARE_EXCEPTION(EIllegalProfile, AAC_ILLEGAL_PROFILE, "Illegal Profile") ;

#endif


