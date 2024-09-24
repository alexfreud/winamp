/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: huffmanbitobj.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1997-12-29
 *   contents/description: HEADER - Huffman Bit Object
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/18 23:00:53 $
 * $Id: huffmanbitobj.h,v 1.3 2011/01/18 23:00:53 audiodsp Exp $
 */

#ifndef __HUFFMANBITOBJ_H__
#define __HUFFMANBITOBJ_H__

/* ------------------------ includes --------------------------------------*/

/*-------------------------- defines --------------------------------------*/

class CBitStream;
class CHuffmanTable;

/*-------------------------------------------------------------------------*/

//
// Class holding one huffman value.
//
//  This object reads and decodes one huffman value from a CBitStream
//  object. One huffman value represents either two (big value part) or four
//  spectral lines (count-one part).
//

class CHuffmanBitObj
{
public:
  CHuffmanBitObj(const CHuffmanTable &HT);
  virtual ~CHuffmanBitObj();

  int ReadFrom(CBitStream &BS) const;

private:
  const CHuffmanTable& m_HuffmanTable;
};

/*-------------------------------------------------------------------------*/
#endif
