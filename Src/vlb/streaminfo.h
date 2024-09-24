/* $Header: /cvs/root/winamp/vlb/streaminfo.h,v 1.1 2009/04/28 20:21:11 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: streaminfo.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: current bitstream parameters
 *
\***************************************************************************/

#ifndef __STREAMINFO_H__
#define __STREAMINFO_H__

#define COPYRIGHT_SIZE 9
#define	BYTE_SIZE 8

typedef struct
{
  unsigned int m_SamplingRateIndex ;
  unsigned int m_SamplingRate ;
  unsigned int m_Profile ;
  unsigned int m_ChannelConfig ;
  unsigned int m_Channels ;
  unsigned int m_BitRate ;

  unsigned int m_ProtectionAbsent;
  unsigned int m_OriginalCopy;
  unsigned int m_Home;
  char m_CopyrightID[COPYRIGHT_SIZE];
  unsigned int m_FrameLength;

  unsigned int m_ChannelMask ;

  unsigned int m_NumberOfFrontChannels ;
  unsigned int m_NumberOfSideChannels ;
  unsigned int m_NumberOfBackChannels ;
  unsigned int m_NumberOfLfeChannels ;

  unsigned int m_ADTSFrameLength;
  unsigned int m_ADTSNumberOfRawDataBlocks;

  bool	m_UseDblLengthXfrm;

} STREAMINFO, * PSTREAMINFO ;

#ifdef __cplusplus

/** Stream Configuration and Information.

    This class holds configuration and information data for a stream to be decoded. It
    provides the calling application as well as the decoder with substantial information,
    e.g. profile, sampling rate, number of channels found in the bitstream etc.
*/

class CStreamInfo : protected STREAMINFO
{

public :

  /// Object Constructor. Initializes all the STREAMINFO members to zero.

  CStreamInfo ()
  {
    m_SamplingRateIndex = 0 ;
    m_SamplingRate = 0 ;
    m_Profile = 0 ;
    m_ChannelConfig = 0 ;
    m_Channels = 0 ;
    m_BitRate = 0 ;
    m_UseDblLengthXfrm = false;
  }

  virtual ~CStreamInfo()
  {
  }

  void SetUseDblLengthXfrm(bool flag)
  {
	  m_UseDblLengthXfrm = flag;
  }


  /// Configures the sampling rate by index.

  void SetSamplingRateIndex (unsigned int sf)
  {
    m_SamplingRateIndex = sf ;
  }

  /// Retrieves the sampling rate by index.

  unsigned int GetSamplingRateIndex (void) const
  {
    return m_SamplingRateIndex ;
  }

  /// Configures the decoder profile.

  void SetProfile (unsigned int profile)
  {
    m_Profile = profile ;
  }

  /// Retrieves the decoder profile.

  unsigned int GetProfile (void) const
  {
    return m_Profile ;
  }

  /// Configures the channel configuration.

  void SetChannelConfig (unsigned int config)
  {
    m_ChannelConfig = config ;
  }

  /// Retrieves the channel configuration.

  unsigned int GetChannelConfig (void) const
  {
    return m_ChannelConfig ;
  }

  /// Configures the number of decoded channels (by value).

  void SetChannels (unsigned int chan)
  {
    m_Channels = chan ;
  }

  /// Configures the number of decoded channels (incrementally).

  void IncChannels (unsigned int chan)
  {
    m_Channels += chan ;
  }

  /// Retrieves the number of decoded channels.

  unsigned int GetChannels (void) const
  {
    return m_Channels ;
  }

  /// Configures the bitrate.

  void SetBitRate (unsigned int bps)
  {
    m_BitRate = bps ;
  }

  /// Retrieves the bitrate.

  unsigned int GetBitRate (void) const
  {
    return m_BitRate ;
  }

  /// Configures the sampling rate by value.

  void SetSamplingRate (unsigned int sf)
  {
    m_SamplingRate = sf ;
  }

  /// Retrieves the sampling rate by value.

  unsigned int GetSamplingRate (void) const
  {
	  if (m_UseDblLengthXfrm)
	  {
		  return m_SamplingRate << 1;
	  }
	  else
	  {
		  return m_SamplingRate ;
	  }
  }

  /// Retrieves the sampling rate by value.

  unsigned int GetRawSamplingRate (void) const
  {
    return m_SamplingRate ;
  }

  /// Configures the protection absent flag.

  void SetProtectionAbsent (unsigned int pa)
  {
    m_ProtectionAbsent = pa ;
  }

  /// Retrieves the protection absent flag.

  unsigned int GetProtectionAbsent (void) const
  {
    return m_ProtectionAbsent ;
  }

  /// Configures the original copy flag.

  void SetOriginalCopy (unsigned int oc)
  {
    m_OriginalCopy = oc ;
  }

  /// Retrieves the original copy flag.

  unsigned int GetOriginalCopy (void) const
  {
    return m_OriginalCopy ;
  }

  /// Configures the home flag.

  void SetHome (unsigned int h)
  {
    m_Home = h ;
  }

  /// Retrieves the home flag.

  unsigned int GetHome (void) const
  {
    return m_Home ;
  }

  /// Configures the copyright id.

  void SetCopyrightID (char cid, int index)
  {
    m_CopyrightID[index] = cid ;
  }

  /// Retrieves the copyright id.

  char GetCopyrightID (int index) const
  {
    return m_CopyrightID[index] ;
  }

  /// Configures the frame length.

  void SetFrameLength (unsigned int fl)
  {
    m_FrameLength = fl ;
  }

  /// Retrieves the frame length.

  unsigned int GetFrameLength (void) const
  {
    return m_FrameLength ;
  }

  // Configures the channel present indicator mask

  void SetChannelMask (unsigned int channelMask)
  {
    m_ChannelMask = channelMask ;
  }

  /// Retrieves the channel present indicator mask

  unsigned int GetChannelMask (void)
  {
    return m_ChannelMask ;
  }

  /// Configures the number of front channels

  void SetNumberOfFrontChannels (unsigned int frontChannels)
  {
    m_NumberOfFrontChannels = frontChannels ;
  }

  /// Retrieves the number of front channels

  int GetNumberOfFrontChannels (void)
  {
    return m_NumberOfFrontChannels ;
  }

  /// Configures the number of side channels

  void SetNumberOfSideChannels (unsigned int sideChannels)
  {
    m_NumberOfSideChannels = sideChannels ;
  }

  /// Retrieves the number of side channels

  int GetNumberOfSideChannels (void)
  {
    return m_NumberOfSideChannels ;
  }

  /// Configures the number of back channels

  void SetNumberOfBackChannels (unsigned int backChannels)
  {
    m_NumberOfBackChannels = backChannels ;
  }

  /// Retrieves the number of back channels

  int GetNumberOfBackChannels (void)
  {
    return m_NumberOfBackChannels ;
  }

  // Configures the number of Low Frequency Enhancement channels

  void SetNumberOfLfeChannels (unsigned int lfeChannels)
  {
    m_NumberOfLfeChannels = lfeChannels ;
  }

  // Retrieves the number of Low Frequency Enhancement channels

  int GetNumberOfLfeChannels (void)
  {
    return m_NumberOfLfeChannels ;
  }

  //

  enum
  {
    SamplesPerFrame = 1024
  } ;

  /// Retrieves the number of delivered samples per audio frame.

  unsigned int GetSamplesPerFrame (void) const
  {
    return SamplesPerFrame ;
  }

  /// Retrieves the number of delivered bytes per audio frame

  unsigned int GetOutputBytes (void) const
  {
    return GetSamplesPerFrame () * GetChannels () * sizeof (short) ;
  }

  void setSampleRate();
} ;

#endif

/// constants for speaker positions and channel index mapping

enum
{
  Channel_FrontLeft = 0,
  Channel_FrontRight = 1,
  Channel_FrontCenter = 2,
  Channel_LowFrequency = 3,
  Channel_BackLeft = 4,
  Channel_BackRight = 5,
  Channel_FrontLeftOfCenter = 6,
  Channel_FrontRightOfCenter = 7,
  Channel_BackCenter = 8,
  Channel_SideLeft = 9,
  Channel_SideRight = 10,
  Speaker_StandardPositions,

  Speaker_FrontLeft = 1 << Channel_FrontLeft,
  Speaker_FrontRight = 1 << Channel_FrontRight,
  Speaker_FrontCenter = 1 << Channel_FrontCenter,
  Speaker_LowFrequency = 1 << Channel_LowFrequency,
  Speaker_BackLeft = 1 << Channel_BackLeft,
  Speaker_BackRight = 1 << Channel_BackRight,
  Speaker_FrontLeftOfCenter = 1 << Channel_FrontLeftOfCenter,
  Speaker_FrontRightOfCenter = 1 << Channel_FrontRightOfCenter,
  Speaker_BackCenter = 1 << Channel_BackCenter,
  Speaker_SideLeft = 1 << Channel_SideLeft,
  Speaker_SideRight = 1 << Channel_SideRight,

  Speaker_FrontSpeakers = Speaker_FrontLeft + Speaker_FrontRight + Speaker_FrontCenter

} ;

#endif
