/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3streaminfo.h
 *   project : MPEG Layer-3 Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-27
 *   contents/description: current bitstream parameters
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:04 $
 * $Id: mp3streaminfo.h,v 1.1 2010/11/17 20:46:04 audiodsp Exp $
 */

#ifndef __MP3STREAMINFO_H__
#define __MP3STREAMINFO_H__

/* ------------------------ structure alignment ---------------------------*/

#ifdef WIN32
  #pragma pack(push, 8)
#endif

/*-------------------------------------------------------------------------*/

typedef struct
{
  int   m_Layer;              /* ISO/MPEG Layer                      */
  int   m_MpegVersion;        /* ISO/MPEG Version                    */
  int   m_Bitrate;            /* Bitrate (Bit/s)                     */
  int   m_BitrateIndex;       /* ISO/MPEG Bitrate index of frame     */
  int   m_Channels;           /* Number of Channels (as indicated)   */
  int   m_SFreq;              /* Sampling Frequency (as indicated)   */
  int   m_EffectiveChannels;  /* Number of effective output channels */
  int   m_EffectiveSFreq;     /* Effective Sampling Frequency        */
  int   m_BitsPerFrame;       /* Number of bits in frame             */
  float m_Duration;           /* Duration of frame in milli seconds  */
  int   m_CrcError;           /* Indication of CRC Errors            */
  int   m_NoMainData;         /* Indication of missing main data     */
	int   m_SamplesPerFrame;
  } MP3STREAMINFO;

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus

//
// Mp3 Streaminfo object.
//
//  Object holding information on the last successfully decode frame.
//

class CMp3StreamInfo : protected MP3STREAMINFO
{
public:

  CMp3StreamInfo() { Reset(); }

  int   GetLayer()             const { return m_Layer; }
  int   GetMpegVersion()       const { return m_MpegVersion; }
  int   GetBitrate()           const { return m_Bitrate; }
  int   GetBitrateIndex()      const { return m_BitrateIndex; }
  int   GetChannels()          const { return m_Channels; }
  int   GetSFreq()             const { return m_SFreq; }
  int   GetBitsPerFrame()      const { return m_BitsPerFrame; }
  float GetDuration()          const { return m_Duration; }
  int   GetCrcError()          const { return m_CrcError; }
  int   GetNoMainData()        const { return m_NoMainData; }
	int   GetSamplesPerFrame()   const { return m_SamplesPerFrame; }

protected:

  friend class CMpgaDecoder;

  void SetLayer(int nValue)             { m_Layer = nValue; }
  void SetMpegVersion(int nValue)       { m_MpegVersion = nValue; }
  void SetBitrate(int nValue)           { m_Bitrate = nValue; }
  void SetBitrateIndex(int nValue)      { m_BitrateIndex = nValue; }
  void SetChannels(int nValue)          { m_Channels = nValue; }
  void SetSFreq(int nValue)             { m_SFreq = nValue; }
  void SetBitsPerFrame(int nValue)      { m_BitsPerFrame = nValue; }
  void SetDuration(float fValue)        { m_Duration = fValue; }
  void SetCrcError(int nValue)          { m_CrcError = nValue; }
  void SetNoMainData(int nValue)        { m_NoMainData = nValue; }
	void SetSamplesPerFrame(int nValue)   { m_SamplesPerFrame = nValue; }

  void Reset()
    {
    m_Layer             = 0;
    m_MpegVersion       = 0;
    m_Bitrate           = 0;
    m_BitrateIndex      = 0;
    m_Channels          = 0;
    m_SFreq             = 0;
    m_BitsPerFrame      = 0;
    m_Duration          = 0.0f;
    m_CrcError          = 0;
    m_NoMainData        = 0;
		m_SamplesPerFrame=0;
    }
};

#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#ifdef WIN32
  #pragma pack(pop)
#endif

/*-------------------------------------------------------------------------*/
#endif
