/****************************************************************************

 This program is protected under international copyright laws as an
 unpublished work. Do not copy.

               (C) Copyright Coding Technologies (2004 - 2007)
                 (C) Copyright Dolby Sweden AB (2008 - 2009)
                             All Rights Reserved

 This software is company confidential information and the property of
 Dolby Sweden AB, and can not be reproduced or disclosed in any form
 without written authorization of Dolby Sweden AB.

 Those intending to use this software module for other purposes are advised
 that this infringe existing or pending patents. Dolby Sweden AB has no
 liability for use of this software module or derivatives thereof in any
 implementation. Copyright is not released for any means. Dolby Sweden AB
 retains full right to use the code for its own purpose, assign or sell the
 code to a third party and to inhibit any user or third party from using the
 code. This copyright notice must be included in all copies or derivative
 works.

 $Id: aacplusdectypes.h,v 1.2 2009/10/12 13:27:37 audiodsp Exp $

*******************************************************************************/

/*!
  \file
  \brief  aacPlus Decoder Library Interface Datatypes
  \author Holger Hoerich
*/


#ifndef _AACPLUSDECTYPES_H_
#define _AACPLUSDECTYPES_H_



/*** 
     MACROS                        *****/


#define AACPLUSDEC_MAXPROGRAMS  16
#define AACPLUSDEC_MAXDATASTREAMELEMENTS  16



typedef struct STRUCT_AACPLUSDECODER *HANDLE_AACPLUSDEC_DECODER;



/*** 
     ENUMERATION TYPES             *****/


/*!
  \brief    Different Supported Output Formats
 */
typedef enum ENUM_AACPLUSDEC_OUTPUTFORMAT {
  AACPLUSDEC_OUTPUTFORMAT_INT16_HOSTENDIAN = 0  /*!< 16 bit signed integer as short, 2 bytes/sample, 
                                                   machine dep. endianess, clipped */
  ,AACPLUSDEC_OUTPUTFORMAT_FLOAT                 /*!< -1.0 .. +1.0 scaled float, 4 bytes/sample, 
                                                   not clipped */
  ,AACPLUSDEC_OUTPUTFORMAT_INT24_LITTLEENDIAN    /*!< 24 bit signed integer packed in 3 bytes/sample, 
                                                   little endian, clipped */

} AACPLUSDEC_OUTPUTFORMAT;


/*!
  \brief    List of supported MPEG-4 Audio Object Types or MPEG-2 Profiles
 */
typedef enum ENUM_AACPLUSDEC_STREAMTYPE {
  AACPLUSDEC_STREAMTYPE_UNDEFINED     = 0

  /* MPEG-2 Profiles */
  ,AACPLUSDEC_MPEG2_PROFILE_AACMAIN   = 0x0200 |  0
  ,AACPLUSDEC_MPEG2_PROFILE_AACLC     = 0x0200 |  1

  /* MPEG-4 Audio Object Types */
  ,AACPLUSDEC_MPEG4_AOT_AACMAIN       = 0x0400 |  1
  ,AACPLUSDEC_MPEG4_AOT_AACLC         = 0x0400 |  2
  ,AACPLUSDEC_MPEG4_AOT_SBR           = 0x0400 |  5

  ,AACPLUSDEC_STREAMTYPE_UNSUPPORTED  = -1

} AACPLUSDEC_STREAMTYPE;


/*!
  \brief    Decoding States
 */
typedef enum ENUM_AACPLUSDEC_DECODINGSTATE {
  AACPLUSDEC_DECODINGSTATE_IDLE = 0         /*!< decoder awaits first frame, has not yet found a valid frame */
  ,AACPLUSDEC_DECODINGSTATE_STREAMPARSED    /*!< first frame parsed or decoded */
  ,AACPLUSDEC_DECODINGSTATE_STREAMVERIFIED  /*!< stream properties verified by second frame */
  ,AACPLUSDEC_DECODINGSTATE_BADFRAME        /*!< a bad frame occured */
  ,AACPLUSDEC_DECODINGSTATE_FLUSHING        /*!< flushing decoder */
  ,AACPLUSDEC_DECODINGSTATE_ENDOFSTREAM     /*!< final state, destroy instance */

} AACPLUSDEC_DECODINGSTATE;


/*!
  \brief    Possible Limitations to Decoder Behaviour
 */
typedef enum ENUM_AACPLUSDEC_CAPABILITY {
  AACPLUSDEC_CAPABILITY_FULL = 0
  ,AACPLUSDEC_CAPABILITY_V1_ONLY = 1010

} AACPLUSDEC_CAPABILITY;

typedef enum ENUM_AACPLUSDEC_CHANNELMODE {
  AACPLUSDEC_CHANNELMODE_UNDEFINED = 0
  ,AACPLUSDEC_CHANNELMODE_MONO
  ,AACPLUSDEC_CHANNELMODE_STEREO
  ,AACPLUSDEC_CHANNELMODE_2_1_CHANNEL
  ,AACPLUSDEC_CHANNELMODE_PARAMETRIC_STEREO
  ,AACPLUSDEC_CHANNELMODE_DUAL_CHANNEL
  ,AACPLUSDEC_CHANNELMODE_3_CHANNEL_1REAR
  ,AACPLUSDEC_CHANNELMODE_3_CHANNEL_MPEG
  ,AACPLUSDEC_CHANNELMODE_3_CHANNEL_3SCE
  ,AACPLUSDEC_CHANNELMODE_4_CHANNEL_MPEG
  ,AACPLUSDEC_CHANNELMODE_4_CHANNEL_4SCE
  ,AACPLUSDEC_CHANNELMODE_4_CHANNEL_2CPE
  ,AACPLUSDEC_CHANNELMODE_5_CHANNEL
  ,AACPLUSDEC_CHANNELMODE_5_1_CHANNEL
  ,AACPLUSDEC_CHANNELMODE_5_CHANNEL_5SCE
  ,AACPLUSDEC_CHANNELMODE_6_1_CHANNEL
  ,AACPLUSDEC_CHANNELMODE_6_CHANNEL_6SCE
  ,AACPLUSDEC_CHANNELMODE_6_CHANNEL_3CPE
  ,AACPLUSDEC_CHANNELMODE_7_1_CHANNEL
  ,AACPLUSDEC_CHANNELMODE_7_CHANNEL_7SCE
  ,AACPLUSDEC_CHANNELMODE_8_CHANNEL_8SCE
  ,AACPLUSDEC_CHANNELMODE_8_CHANNEL_4CPE

} AACPLUSDEC_CHANNELMODE ;


typedef enum ENUM_AACPLUSDEC_CONFIGTYPE {
  AACPLUSDEC_CONFIGTYPE_UNSPECIFIED = 0
  ,AACPLUSDEC_CONFIGTYPE_STREAMMUXCONFIG
  ,AACPLUSDEC_CONFIGTYPE_AUDIOSPECIFICCONFIG
  ,AACPLUSDEC_CONFIGTYPE_DECODERCONFIGDESCRIPTOR
  ,AACPLUSDEC_CONFIGTYPE_DECODERSPECIFICINFO

} AACPLUSDEC_CONFIGTYPE;

typedef enum ENUM_AACPLUSDEC_BITSTREAMFORMAT {
  AACPLUSDEC_BITSTREAMFORMAT_AUTO = 0
  ,AACPLUSDEC_BITSTREAMFORMAT_RAW
  ,AACPLUSDEC_BITSTREAMFORMAT_ADIF
  ,AACPLUSDEC_BITSTREAMFORMAT_ADTS
  ,AACPLUSDEC_BITSTREAMFORMAT_LATM
  ,AACPLUSDEC_BITSTREAMFORMAT_LOAS

} AACPLUSDEC_BITSTREAMFORMAT;

typedef enum ENUM_AACPLUSDEC_ERROR {
  AACPLUSDEC_OK = 0

  ,AACPLUSDEC_ERROR_GENERALERROR = 0x1100
  ,AACPLUSDEC_ERROR_INVALIDHANDLE
  ,AACPLUSDEC_ERROR_OUTOFMEMORY
  ,AACPLUSDEC_ERROR_UNSUPPORTEDSETTING
  ,AACPLUSDEC_ERROR_TARGETBUFFERTOOSMALL
  ,AACPLUSDEC_ERROR_NEEDMOREDATA
  ,AACPLUSDEC_ERROR_ENDOFSTREAM
  ,AACPLUSDEC_ERROR_WRONGARGUMENT
  ,AACPLUSDEC_ERROR_AACMAINNOTSUPORTED

} AACPLUSDEC_ERROR;

typedef enum ENUM_AACPLUSDEC_SIGNALLING {
  AACPLUSDEC_SIGNALLING_IMPLICIT = -1
  ,AACPLUSDEC_SIGNALLING_DISABLED = 0
  ,AACPLUSDEC_SIGNALLING_ENABLED_BC
  ,AACPLUSDEC_SIGNALLING_ENABLED_NONBC
  
} AACPLUSDEC_SIGNALLING;



/*** 
     STRUCTURED TYPES              *****/



/*!
  \brief    Properties Of A Program Embeddded In The aacPlus Audio Stream
 */
typedef struct {
  int bProgramFound;           /*!< true if this program is available in the bitstream */

  int nOutputSamplingRate;     /*!< output sampling frequency */
  int nOutputSamplesPerFrame;  /*!< number of output samples per frame */
  int nStreamOutputChannels;   /*!< number of audio output channels in the stream */
  int nOutputChannels;         /*!< number of audio output channels after downmix */

  int bProgramSbrEnabled;      /*!< non-zero when SBR is decoded */
  int nAacSamplingRate;        /*!< core sampling rate */
  int nAacChannels;            /*!< core channels */

  AACPLUSDEC_CHANNELMODE nChannelMode; /*!< audio channel configuration */
  int nNumberOfFrontChannels;  /*!< number of front channels in this program */
  int nNumberOfSideChannels;   /*!< number of side channels in this program */
  int nNumberOfRearChannels;   /*!< number of rear channels in this program */
  int nNumberOfLfeChannels;    /*!< number of LFE channels in this program */


  AACPLUSDEC_STREAMTYPE nStreamType; /*!< Audio Object Type if accessible (profile if MPEG-2) */
  AACPLUSDEC_SIGNALLING nSbrSignalling; /*!< SBR signalling */
  AACPLUSDEC_SIGNALLING nPsSignalling;  /*!< PS signalling */

} AACPLUSDEC_PROGRAMPROPERTIES;

/*!
  \brief    Properties Of The aacPlus Audio Stream
 */
typedef struct {
  AACPLUSDEC_DECODINGSTATE nDecodingState; /*!< state of decoding */
  int bSbrDecodingFailed;                  /*!< state of sbr decoding */

  AACPLUSDEC_BITSTREAMFORMAT nBitstreamFormat; /*!< bitstream format, may be set for streams without synch */

  int nBitrate;                      /*!< estimated bitrate (frequently updated) */
  
  int nProgramsInStream;             /*!< number of programs embedded in audio stream */
  int nCurrentProgram;               /*!< index of current decoded program (defaults to 0) */

  AACPLUSDEC_PROGRAMPROPERTIES programProperties[AACPLUSDEC_MAXPROGRAMS]; /*!< properties of embedded programs */


} AACPLUSDEC_STREAMPROPERTIES;


/*!
  \brief    Settings of the decoder library
 */
typedef struct {
  int bEnableConcealment;       /*!< set for concealment, erroneous frames muted otherwise */
  int bEnableOutputLimiter;     /*!< activate limiter on the PCM output data */
  int bDoLowPowerSBR;           /*!< use Low Power SBR */
  int bDoUpsampling;            /*!< do upsampling by 2 when AAC sample rate is below 32kHz */
  int nDownsampleSbrAtTargetFs; /*!< do downsampled SBR when core sample rate is at or above given value in Hz */
  int bForceAdtsCRCheck;        /*!< tell the decoder ADTS CRC is present even if not recognized (ist this correct?) */
  int bForceMpeg2Behaviour;     /*!< tell the decoder to act like an MPEG-2 decoder, i.e. apply concealment on PNS frames */
  int bNoChannelReordering;     /*!< tell the decoder to keep the channels in the order as they appear in the bitstream */

  AACPLUSDEC_CAPABILITY eDecCapability; /*!< set decoding capability */

  int reserved1;
  int reserved2;

  int nMaxMainAudioChannels;                     /*!< allocate num. audio channels */
  int nMaxLFEChannels;                           /*!< allocate num. low frequency enhancement channels */
  int nMaxIndependentlySwitchedCouplingChannels; /*!< allocate num. independently switched CCE */
  int nMaxDependentlySwitchedCouplingChannels;   /*!< allocate num. dependently switched CCE */

  int nDmxOutputChannels;                        /*!< number of downmix channels (0..2; 0 means no downmix) */
  int nDmxCoefficients;                          /*!< coefficient selection according to 13818-7, Table 8.3.8.5 */




} AACPLUSDEC_EXPERTSETTINGS;

/*** 
     BUFFER DESCRIPTIONS           *****/


/*!
  \brief    Audio Buffer Description
*/
typedef struct {
  int nBytesBufferSizeIn;
  int nBytesWrittenOut;
  int nBytesMissedOut;
} AACPLUSDEC_AUDIOBUFFERINFO;


/*!
  \brief    Bitstream Buffer Description
*/
typedef struct {
  int nBytesGivenIn;
  int nBitsOffsetIn;
  int nBytesReadOut;
} AACPLUSDEC_BITSTREAMBUFFERINFO;


/*!
  \brief    Datastream Element Description
*/
typedef struct {
  int nInstanceTagOut;
  int nBytesOffsetOut;
  int nBytesLengthOut;
} AACPLUSDEC_DATASTREAMELEMENT;


/*!
  \brief    Datastream Buffer Description
*/
typedef struct {
  int nBytesBufferSizeIn;
  int nBytesWrittenOut;
  int nBytesMissedOut;
  int nNumDataStreamElementsOut;
  AACPLUSDEC_DATASTREAMELEMENT elementDescription[AACPLUSDEC_MAXDATASTREAMELEMENTS];
} AACPLUSDEC_DATASTREAMBUFFERINFO;









/*** 
     END   *****************************/



#endif /* _AACPLUSDECTYPES_H_ */
