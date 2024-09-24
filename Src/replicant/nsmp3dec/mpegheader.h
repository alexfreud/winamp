/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mpegheader.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1997-12-05
 *   contents/description: ISO/MPEG Header
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:04 $
 * $Id: mpegheader.h,v 1.1 2010/11/17 20:46:04 audiodsp Exp $
 */

#ifndef __MPEGHEADER_H__
#define __MPEGHEADER_H__

/* ------------------------ includes --------------------------------------*/

/*-------------------------- defines --------------------------------------*/

class CBitStream;

/*-------------------------------------------------------------------------*/

//
// MPEG header class.
//
//  This object reads and decodes an ISO/MPEG header.
//

class CMpegHeader
{
public:
  CMpegHeader();
  virtual ~CMpegHeader();

  int ReadFrom(CBitStream &sBS);
  int FromInt(unsigned long dwHdrBits);

  int   GetMpegVersion()     const { return m_MpegVersion;}
  int   GetLayer()           const { return m_Layer;}
  int   GetChannels()        const { return m_Channels;}
  int   GetSampleRate()      const { return m_SampleRate;}
  int   GetSampleRateNdx()   const { return m_SampleRateNdx;}
  int   GetBitrate()         const { return m_Bitrate;}
  int   GetBitrateNdx()      const { return m_BitrateNdx;}
  int   GetMode()            const { return m_Mode;}
  int   GetModeExt()         const { return m_ModeExt;}
  int   GetPadding()         const { return m_Padding; }
  int   GetCrcCheck()        const { return m_CrcCheck;}
  int   GetCopyright()       const { return m_Copyright;}
  int   GetOriginal()        const { return m_Original;}
  int   GetEmphasis()        const { return m_Emphasis;}

  int   GetHeaderLen()       const
    { return MPEG_HDRLEN+(m_CrcCheck?MPEG_CRCLEN:0); }
  int   GetFrameLen()        const { return m_FrameLen;}
  float GetDuration()        const { return m_Duration;}
  int   GetSamplesPerFrame() const;

protected:

private:

  enum { MPEG_HDRLEN = 32, MPEG_CRCLEN = 16 };

  int  CalcFrameLen();
  void ResetMembers();
  void SetMembers();

  // header fields
  int m_Syncword;
  int m_Idex;
  int m_Id;
  int m_Layer;
  int m_CrcCheck;
  int m_BitrateNdx;
  int m_SampleRateNdx;
  int m_Padding;
  int m_Private;
  int m_Mode;
  int m_ModeExt;
  int m_Copyright;
  int m_Original;
  int m_Emphasis;

  // calculated data
  int   m_HeaderValid;
  int   m_MpegVersion;
  int   m_Channels;
  int   m_SampleRate;
  int   m_Bitrate;
  int   m_FrameLen;
  float m_Duration;
};

/*-------------------------------------------------------------------------*/
#endif
