/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mpeg.h
 *   project : ISO/MPEG-Decoder
 *   author  : Markus Werner, addings: Martin Sieler
 *   date    : 1995-07-07
 *   contents/description: HEADER - iso/mpeg-definitions
 *
 *
\***************************************************************************/

/*
 * $Date: 2011/01/20 22:14:40 $
 * $Id: mpeg.h,v 1.3 2011/01/20 22:14:40 audiodsp Exp $
 */

/*-------------------------------------------------------------------------*/

#ifndef __MPEG_H__
#define __MPEG_H__
#include "foundation/align.h"

/* ------------------------ includes --------------------------------------*/

/* ------------------------------------------------------------------------*/

//
// MPEG ID (fhgVersion)
//
#define         MPG_MPEG1               1
#define         MPG_MPEG2               0
#define         MPG_MPEG25              2

/* ------------------------------------------------------------------------*/

//
// sample rate
//
#define         MPG_SF_LOW              2

/* ------------------------------------------------------------------------*/

//
// header-mode field
//
#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

/*-------------------------------------------------------------------------*/

//
// channels
//
#define         MONO                    1
#define         STEREO                  2

/* ------------------------------------------------------------------------*/

//
// subbands, samples/subband
//
#define         SBLIMIT                 32
#define         SSLIMIT                 18

/* ------------------------------------------------------------------------*/

//
// info structure
//
typedef struct
  {
  int  stereo;
  int  sample_rate_ndx;
  int  frame_bits;
  int  mode;
  int  mode_ext;
  int  header_size;
  int  fhgVersion;
  int  protection;
  bool IsMpeg1;
  } MPEG_INFO;

/* ------------------------------------------------------------------------*/

//
// MPEG Layer-3 sideinfo (per channel/granule)
//
typedef struct
  {
  int part2_3_length;
  int big_values;
  int global_gain;
  int scalefac_compress;
  int window_switching_flag;
  int block_type;
  int mixed_block_flag;
  int table_select[3];
  int subblock_gain[3];
  int region0_count;
  int region1_count;
  int preflag;
  int scalefac_scale;
  int count1table_select;

  // additional calced values
  int intensity_scale; // MPEG 2, MPEG 2.5 only
  int zeroStartNdx;
  int zeroSfbStartNdxIsLong;
  int zeroSfbStartNdxL;
  int zeroSfbStartNdxSMax;
  int zeroSfbStartNdxS[3];
  int zeroSbStartNdx;
  } MP3SI_GRCH;

/* ------------------------------------------------------------------------*/

//
// MPEG Layer-3 sideinfo
//
typedef struct
  {
  int main_data_begin;
  int private_bits;
  struct
    {
    int        scfsi[4];
    MP3SI_GRCH gr[2];
    } ch[2];
  } MP3SI;

/* ------------------------------------------------------------------------*/

//
// MPEG Layer-3 scalefactors
//
typedef struct
  {
  // scalefactors
  int l[23];
  int s[3][13];

  // illegal intensity position
  int l_iip[23];
  int s_iip[13];
  } MP3SCF;

/* ------------------------------------------------------------------------*/

//
// spectrum (as transmitted)
//
typedef NALIGN(16) float SPECTRUM[2][SBLIMIT][SSLIMIT];

//
// spectrum (after mdct)
//
typedef NALIGN(16) float POLYSPECTRUM[2][SSLIMIT][SBLIMIT];

/* Nullsoft added 25 Oct 2007 */
struct DecoderHooks
{
	void (*layer3_vis)(SPECTRUM vistable,int gr, int nch);
	void (*layer2_eq)(float *xr, int nch, int srate, int nparts);
	void (*layer3_eq)(float *xr, int nch, int srate);
};

/* ------------------------------------------------------------------------*/
#endif
