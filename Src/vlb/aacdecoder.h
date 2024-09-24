/* $Header: /cvs/root/winamp/vlb/aacdecoder.h,v 1.1 2009/04/28 20:21:07 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: aacdecoder.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: decoder main object
 *
\***************************************************************************/

#ifndef __AACDECODER_H__
#define __AACDECODER_H__

#include "channel.h"
#include "programcfg.h"
#include "datastream.h"
#include "exception.h"
#include "adif.h"
#include "streaminfo.h"
#include "aacdecoderapi.h"
#include "bitbuffer.h"
#include "DolbyPayload.h"
#include "audio_io.h"


//typedef struct {
//	short	file_format;
//	long	bitrate;
//} AACStreamParameters;

typedef struct {
	long	frame_length;
	short	protection_absent;
	short	copyright;
	short	original_copy;
	char	copyright_id[9];
} AACFrameParameters;

typedef struct {
	short	pce_instance_tag;
	short	profile;
	long	sampling_frequency;
	short	num_channels;
	short	num_front_channels;	
	short	num_side_channels;	
	short	num_back_channels;
	short	num_lfe_channels;
	short	num_coupling_channels;
	short	*pcoupling_channel_tags;
	short	mono_mixdown_present;
	short	stereo_mixdown_present;
	short	matrix_mixdown_present;
	short	pseudo_surround_present;
	short	drc_present;
	unsigned int	prog_ref_level;
	char	*comment_field_data;
} AACProgramParameters;

typedef struct{
	short	pce_instance_tag;
	short	channel_config;
	unsigned int	*pcoupling_channel_tags;
	short	drc_factor_high;
	short	drc_factor_low;
	unsigned int	target_level;
} AACDecodeParameters;




class CDolbyBitStream ;

/** MPEG-2 AAC Decoder Top Level Object.

    This is the main decoder object that interfaces with the application code.
    It wraps the bitstream element objects and calls their individual decoding methods.
    To indicate unexpected events and error conditions that prevent correct operation,
    an exception derived from \Ref{CAacException} will be thrown that should be caught
    by the calling application's decoding loop.

    \URL[High Level Objects - Overview Diagram]{overview.html}

    \URL[MPEG-2 AAC Decoder Block Diagram]{scheme.html}
*/

class CAacDecoder
{

public :

/** Former API parameter structures */

	AACStreamParameters		*pAACStreamParameters;
	AACFrameParameters		*pAACFrameParameters;
	AACProgramParameters	*pAACProgramParameters[AAC_MAX_PROGRAMS];
	AACDecodeParameters		*pAACDecodeParameters;


  /** Object Constructor.

      Instantiates a decoder object capable of decoding one stream of
      MPEG-2 AAC Audio Data.

      @param bs The \Ref{CDolbyBitStream} object that the decoder will read from.
  */

  CAacDecoder (CDolbyBitStream &bs) ;
  ~CAacDecoder () ;


  /** Frame Initialization Method.

	  This method initializes the frame and encodes the ADIF header.

      @param info   The configuration information.
	*/

  void	FrameInit(CStreamInfo &info);



  /** Main Decoding Method.

      This method delivers one frame of decoded pcm audio data.

      @param pcmbuf The decoded data will be written to the buffer provided by the application.
                    This buffer must be allocated large enough to hold one frame.
      @param info   The configuration information.
  */


  void DecodeFrame (AudioIOControl *poAudioIO, CStreamInfo &info) ;

  /** Equalizer Configuration Method.

      This method configures the built-in spectral equalizer.

      @param wantEQ Turn the Equalizer on/off.
      @param Mask   An array of 16 coefficients that will be applied to the spectral data
                    of all active channel elements in ascending order.
  */

  void SetEqualization (bool wantEQ, float Mask []) ;

  /** Status Information Method.

      This method returns the number of raw_data_blocks decoded until now.
  */

  int GetNumberOfBlocksDecoded (void)
  {
    return m_BlockNumber ;
  }

  int GetDolbyBitstreamWarning(void)
  {
	  return sDSEInfo.iDolbyBitStreamWarning;
  }

  bool HasDolbySpectralExtension(){return bHasDSEInfoStream;}
  bool HasDoubleLengthXForm(){return (sDSEInfo.iUsesDoubleLengthXForm ? true : false);}
  void IgnoreDolbyStream(bool bIgnore){bIgnoreDolbyStream=bIgnore;}

  CAdifHeader* GetAdifHeader(void) { return &m_AdifHeader; }
  /*Default Not Ignore*/
private:
	//MSV:
   void InitDSEInfo(CDolbyBitStream*poBS,CChannelElement*poChannelElement);
private:
	bool bLookForDSEInfoStream;
	bool bHasDSEInfoStream;
    bool bIgnoreDolbyStream;
	DOLBY_PAYLOAD_STRUCT sDSEInfo;
protected :

  void ReadFillElement (void) ;
  void ReadDolbyFillElement (void) ;
  enum
  {
    ID_SCE = 0,
    ID_CPE,
    ID_CCE,
    ID_LFE,
    ID_DSE,
    ID_PCE,
    ID_FIL,
    ID_END
  } ;

  CDolbyBitStream &m_bs ;

  CSingleChannel sce ;
  CChannelPair cpe ;
  CSingleChannel lfe ;
  CDataStream dse ;

  CAdifHeader m_AdifHeader ;

  int m_BlockNumber ;
  int m_SelectedProgram ;

  DECLARE_EXCEPTION(EUnimplemented, AAC_UNIMPLEMENTED, "Unimplemented Feature Used") ;
  DECLARE_EXCEPTION(EDolbyNotSupported, AAC_DOLBY_NOT_SUPPORTED, "Not Supported") ;
  DECLARE_EXCEPTION(EIllegalProfile, AAC_ILLEGAL_PROFILE, "Illegal Profile") ;

} ;

#endif
