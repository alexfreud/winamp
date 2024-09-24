/* $Header: /cvs/root/winamp/vlb/block.h,v 1.2 2011/06/13 02:06:02 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: block.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description:  Defines abstract base class CBlock
 *
 * $Header: /cvs/root/winamp/vlb/block.h,v 1.2 2011/06/13 02:06:02 audiodsp Exp $
 *
\***************************************************************************/

#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "exception.h"

#include "pulsedata.h"
#include "tns.h"
#include "DolbyPayload.h"

class CChannelInfo ;
class CDolbyBitStream ;
class COverlapAddBuffer ;

#ifdef MAIN_PROFILE
class CPrediction ;
#endif

/** Spectral Data For Current Frame.

    This abstract base class defines the interface and some common helper functions
    for the classes holding the spectral coefficients of the current frame.
*/

class CBlock
{
  // allow tools to call AccessXXX() methods

  friend class CJointStereo ;

#ifdef MAIN_PROFILE
  friend class CPrediction ;
#endif

  friend class CTns ;

  friend class CCouplingChannel ;

  friend CBlock &operator+= (CBlock &, CBlock &) ;

public :

  CBlock (CChannelInfo &) ;
  virtual ~CBlock () ;

  void Read (CDolbyBitStream &bs, const int global_gain) ;

  virtual void ApplyTools (void) ;

  virtual void FrequencyToTime (COverlapAddBuffer &, float [], const int) = 0 ;
  //MSV
  virtual void FrequencyToTime_Fast (COverlapAddBuffer&) = 0 ;
  virtual void FrequencyToTime (COverlapAddBuffer &) = 0;
  virtual SECTION_INFORMATION_STRUCT* GetSectionInfo() = 0;
  // gimmicks 

  virtual void ApplyEqualizationMask (float Mask []) = 0 ;

  // // //

  enum
  {
    EqualizationMaskLength = 16,

    ZERO_HCB = 0,
    ESCBOOK  = 11,
    NSPECBOOKS = ESCBOOK + 1,
    BOOKSCL    = NSPECBOOKS,
    RESERVED_HCB = 13,
    INTENSITY_HCB2 = 14,
    INTENSITY_HCB  = 15,

    InverseQuantTableSize = 1024,
    ExpTableSize = 128,

    HuffmanBits = 4,
    HuffmanEntries = (1 << HuffmanBits)
    
  } ;

  virtual float *AccessSpectralData (int window = 0) = 0 ;
  virtual float *AccessOutput() =0;
  virtual short *AccessShortOutput() =0;
  short FloatToShort (float pcm)
  {
    if (pcm < 0.0F)
    {
      pcm  = -pcm ;
      pcm += 0.5F ;

      return (pcm > 32768.0F) ? -32768 : -((short) pcm) ; 
    }
    else 
    {
      pcm += 0.5F ;

      return (pcm > 32767.0F) ? 32767 : (short) pcm ;
    }
  }
protected :

  // overridable readers

  virtual void ReadSectionData (CDolbyBitStream &bs) = 0 ;
  virtual void ReadScaleFactorData (CDolbyBitStream &bs, const int global_gain) = 0 ;
  virtual void ReadSpectralData (CDolbyBitStream &bs) = 0 ;

  // tools interface

  

  virtual int   *AccessCodeBooks (int group) = 0 ;
  virtual int   *AccessScaleFactors (int group) = 0 ;

  //

  CChannelInfo &m_IcsInfo ;
  CPulseData m_PulseData ;
  CTns m_Tns ;

  // Huffman Decoding

  typedef struct tagCodeBookDescription
  {
    int Dimension ;
    bool IsSigned ;
    const unsigned int (*CodeBook)[HuffmanEntries] ;
    const int *NTuples ;
  } CodeBookDescription ;

  int DecodeHuffmanWord (CDolbyBitStream &bs, const unsigned int (*CodeBook) [HuffmanEntries]) ;
  int UnpackIndex (int idx, int *qp, const CodeBookDescription *hcb) ;
  int GetEscape (CDolbyBitStream &bs, const int q) ;

  static const CodeBookDescription HuffmanCodeBooks [13] ;

  // 

  float InverseQuantize (int q) ;

  int m_SectBits ;



  // window functions

  static const float m_OnlyLongWindowSine [1024] ;
  static const float m_OnlyLongWindowKBD [1024] ;

  static const float m_OnlyShortWindowSine [128] ;
  static const float m_OnlyShortWindowKBD [128] ;

  const float *m_LongWindow [2] ;
  const float *m_ShortWindow [2] ;

  // imdct

  void Split (const float [], float [], float [], const float [], int) ;
  void Merge (float [], const float [], const float [], int ) ;

  void Dct1024 (float vec [], const float cosTerms []) ;
  void Dct128  (float vec [], const float cosTerms []) ;
  void Dct16   (float vec [], const float cosTerms []) ;

  // speed-up tabes

  static const float m_InverseQuantTable [CBlock::InverseQuantTableSize] ;
  static const float m_ExpTable [CBlock::ExpTableSize] ;

  // exceptions

  DECLARE_EXCEPTION(EInvalidCodeBook, AAC_INVALIDCODEBOOK, "Invalid Huffman Codebook") ;
  DECLARE_EXCEPTION(EUnimplemented, AAC_UNIMPLEMENTED, "Unimplemented Feature Used") ;

  // platform dependent stuff

#if defined (WIN32) && defined (_M_IX86)

void PentiumOverlap (float output [], float spec [], float prev [], unsigned int stride);

#endif

} ;

/** Spectral Data For One Long Block.

    This class holds the spectral coefficients for the current frame in case it's
    a start block, a stop block or a regular long block.
*/

class CLongBlock : public CBlock
{
  friend CBlock &operator+= (CBlock &, CBlock &) ;

public :

  CLongBlock (CChannelInfo &) ;
  virtual ~CLongBlock () ;

  virtual void FrequencyToTime (COverlapAddBuffer &, float [], const int) ;
  //MSV
  virtual void FrequencyToTime_Fast (COverlapAddBuffer &) ;
  virtual void FrequencyToTime (COverlapAddBuffer &);
  // gimmicks

  virtual void ApplyEqualizationMask (float Mask []) ;
  //MSV: 
  virtual SECTION_INFORMATION_STRUCT* GetSectionInfo(){return (&sSectionInfoStruct);}

  virtual float *AccessSpectralData (int window = 0) ;
  virtual float *AccessOutput() {return m_Output;}
  virtual short *AccessShortOutput() {return m_ShortOutput;}
private:
  SECTION_INFORMATION_STRUCT sSectionInfoStruct;
protected :

  // overridden readers

  virtual void ReadSectionData (CDolbyBitStream &bs) ;
  virtual void ReadScaleFactorData (CDolbyBitStream &bs, int const global_gain) ;
  virtual void ReadSpectralData (CDolbyBitStream &bs) ;

  // tools interface

  

  virtual int   *AccessCodeBooks (int group) ;
  virtual int   *AccessScaleFactors (int group) ;
  int GetMaximumBins() { return MaximumBins; } ;

  // private

  void InverseTransform (float data []) ;
  void ApplyWindowFunction (COverlapAddBuffer &Previous) ;

  enum
  {
    MaximumScaleFactorBands = 64,
    MaximumBins = 1024
  } ;

  int m_CodeBook [CLongBlock::MaximumScaleFactorBands] ; 
  int m_ScaleFactor [CLongBlock::MaximumScaleFactorBands] ;

  float m_SpectralCoefficient [2 * CLongBlock::MaximumBins] ;
  float m_Output[1024];
  short m_ShortOutput[1024];
  // platform dependent stuff

#if defined (WIN32) && defined (_M_IX86)

  void PentiumWindow (float spec [], const float prev [], const float curr []) ;

#endif

} ;

/** Spectral Data For One Short Block.

    This class holds the spectral coefficients for the current frame in case it's
    a short block, i.e. consists of eight short windows.
*/

class CShortBlock : public CBlock
{
  friend CBlock &operator+= (CBlock &, CBlock &) ;

public :
  
  CShortBlock (CChannelInfo &) ;
  virtual ~CShortBlock () ;

  virtual void FrequencyToTime (COverlapAddBuffer &, float [], const int) ;
  //MSV
  virtual void FrequencyToTime_Fast (COverlapAddBuffer &) ;
  virtual void FrequencyToTime (COverlapAddBuffer &);
  // gimmicks

  virtual void ApplyEqualizationMask (float Mask []) ;
  virtual float *AccessSpectralData (int window = 0) ;
  virtual float *AccessOutput() {return m_Output;}
  virtual short *AccessShortOutput() {return m_ShortOutput;}
  virtual SECTION_INFORMATION_STRUCT* GetSectionInfo(){return (&sSectionInfoStruct);}
private:
  SECTION_INFORMATION_STRUCT sSectionInfoStruct;
protected :

  // overridden readers

  virtual void ReadSectionData (CDolbyBitStream &bs) ;
  virtual void ReadScaleFactorData (CDolbyBitStream &bs, const int global_gain) ;
  virtual void ReadSpectralData (CDolbyBitStream &bs) ;

  // tools interface

  

  virtual int   *AccessCodeBooks (int group) ;
  virtual int   *AccessScaleFactors (int group) ;

  // private

  void InverseTransform (float data []) ;
  void TransformWindows (COverlapAddBuffer &Previous, float EightWindowsBuffer []) ;

  //

  enum
  {
    MaximumWindows = 8,
    MaximumGroups = MaximumWindows,

    MaximumScaleFactorBands = 16,
    MaximumBins = 128
  } ;

  int m_CodeBook [CShortBlock::MaximumGroups][CShortBlock::MaximumScaleFactorBands] ; 
  int m_ScaleFactor [CShortBlock::MaximumGroups][CShortBlock::MaximumScaleFactorBands] ;

  float m_SpectralCoefficient [CShortBlock::MaximumWindows][2 * CShortBlock::MaximumBins] ;
  float m_Output[1024];
  short m_ShortOutput[1024];
} ;

#endif
