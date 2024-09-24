/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: huffmandecoder.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-02-08
 *   contents/description: HEADER - huffman decoder
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/21 22:25:58 $
 * $Id: huffmandecoder.h,v 1.4 2011/01/21 22:25:58 audiodsp Exp $
 */

#ifndef __HUFFMANDECODER_H__
#define __HUFFMANDECODER_H__

/* ------------------------ includes --------------------------------------*/

#include "bitsequence.h"
#include "huffmanbitobj.h"
#include "huffmantable.h"

/*-------------------------- defines --------------------------------------*/

class CBitStream;

/*-------------------------------------------------------------------------*/

//
// Huffman decoder (helper) class.
//
//  This object reads and decodes MPEG Layer-3 huffman data.
//

class CHuffmanDecoder
{
public:
  CHuffmanDecoder();
  virtual ~CHuffmanDecoder();

  int ReadHuffmanCode(CBitStream &Bs,
                      int        *pIsp,
                      const int  *pTableSelect,
                      const int  *pRegionEnd,
                      int         Count1TableSelect,
                      int         Part2_3Length);

protected:

private:
  int  ReadBigValues(CBitStream  &Bs,
                     int         *pIsp,
                     const int   *pTableSelect,
                     const int   *pRegionEnd);

  int  ReadCount1Area(CBitStream &Bs,
                      int        *pIsp,
                      int         Count1TableSelect,
                      int         Count1Start,
                      int         Part2_3Length);
#ifdef _MSC_VER
	// these only have one caller and inlining shows notable improvements in the profiler
  __forceinline void ReadHuffmanDual   (CBitStream &Bs, int *pIsp);
  __forceinline void ReadHuffmanDualLin(CBitStream &Bs, int *pIsp);
  __forceinline bool ReadHuffmanQuad   (CBitStream &Bs, int *pIsp);
#else
	void ReadHuffmanDual   (CBitStream &Bs, int *pIsp);
  void ReadHuffmanDualLin(CBitStream &Bs, int *pIsp);
  bool ReadHuffmanQuad   (CBitStream &Bs, int *pIsp);
#endif

  CHuffmanTable  m_HuffmanTable;
  CHuffmanBitObj m_HuffmanBitObj;
  CBitSequence   m_LinBits;
};

/*-------------------------------------------------------------------------*/
#endif
