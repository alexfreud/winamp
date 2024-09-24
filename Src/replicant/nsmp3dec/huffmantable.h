/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: huffmantable.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-01-05
 *   contents/description: HEADER - huffman table object
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/18 23:00:53 $
 * $Id: huffmantable.h,v 1.3 2011/01/18 23:00:53 audiodsp Exp $
 */

#ifndef __HUFFMANTABLE_H__
#define __HUFFMANTABLE_H__

/* ------------------------ includes --------------------------------------*/

/*-------------------------- defines --------------------------------------*/

#define HUFFMAN_BITS_4

/*-------------------------------------------------------------------------*/

// Huffman tables.
//
//  This object holds the huffman table for ISO/MPEG Layer-3.
//
  typedef struct
    {
	    const unsigned char length;
			const unsigned char value;
    } huffman_entry_t;

class CHuffmanTable
{
public:
  
  /*unsigned int nTableIndex;	*/
  CHuffmanTable();
  virtual ~CHuffmanTable();

  void SetTableIndex(unsigned int _nTableIndex)
    { nTableIndex = _nTableIndex; }

  unsigned int GetBitsPerLevel() const
    { return BITS_PER_LEVEL; }

  unsigned int GetLinBits() const
    { return ht[nTableIndex].linbits; }

  unsigned char GetCode  (unsigned int nIndex, unsigned int nValue) const
    { return (ht[nTableIndex].table[nIndex][nValue] & 0xff); }

  unsigned char GetLength(unsigned int nIndex, unsigned int nValue) const
    {  return ((ht[nTableIndex].table[nIndex][nValue] >> 8) & 0xff); }

  bool IsTableValid() const
    {  return (ht[nTableIndex].table ? true:false); }

  bool IsLengthZero(unsigned int nIndex, unsigned int nValue) const
    { return ((ht[nTableIndex].table[nIndex][nValue] & 0xff00) == 0); }

  enum 
  { 
#if defined HUFFMAN_BITS_2     /* HuffmanBits parallel huffman tables */
    BITS_PER_LEVEL = 2, 
    ENTRIES_PER_LEVEL = 4
#elif defined HUFFMAN_BITS_3
    BITS_PER_LEVEL = 3, 
    ENTRIES_PER_LEVEL = 8
#elif defined HUFFMAN_BITS_4   /* HuffmanBits parallel huffman tables */
    BITS_PER_LEVEL = 4, 
    ENTRIES_PER_LEVEL = 16 
#endif
	
  };

protected:

private:

  typedef struct
    {
    unsigned int linbits;
    const unsigned short(*table)[ENTRIES_PER_LEVEL];
    } huffmantab;

  static const huffmantab ht[];

  unsigned int nTableIndex;
};

/*-------------------------------------------------------------------------*/
#endif
