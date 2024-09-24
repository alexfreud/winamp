
/*!
 ************************************************************************
 *  \file
 *     global.h
 *  \brief
 *     global definitions for H.264 decoder.
 *  \author
 *     Copyright (C) 1999  Telenor Satellite Services,Norway
 *                         Ericsson Radio Systems, Sweden
 *
 *     Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
 *
 *     Telenor Satellite Services
 *     Keysers gt.13                       tel.:   +47 23 13 86 98
 *     N-0130 Oslo,Norway                  fax.:   +47 22 77 79 80
 *
 *     Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
 *
 *     Ericsson Radio Systems
 *     KI/ERA/T/VV
 *     164 80 Stockholm, Sweden
 *
 ************************************************************************
 */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/timeb.h>

#include <bfc/platform/types.h>
#include "win32.h"
#include "defines.h"
#include "ifunctions.h"
#include "parsetcommon.h"
#include "types.h"
#include "frame.h"
#include "nalucommon.h"
#include "memcache.h"
#include <mmintrin.h>
#ifdef H264_IPP
//#include "../tools/staticlib/ipp_px.h"
#include "ippdefs.h"
#include "ippcore.h"
#include "ipps.h"
#include "ippi.h"
#include "ippvc.h"
#endif
/* benski> not the best place for this but it works for now */
#ifdef _M_IX86
// must be a multiple of 16
#pragma warning(disable: 4799)
static inline void memzero_cache32(void *dst, unsigned long i)
{
	
	__asm {
		pxor	mm0, mm0
		mov edi, dst

loopwrite:

		movq 0[edi], mm0
		movq 8[edi], mm0
		movq 16[edi], mm0
		movq 24[edi], mm0
		lea edi, [edi+32]
		sub i, 32
		jg loopwrite

	}
}

static inline void memzero_fast32(void *dst, unsigned long i)
{
	
	__asm {
		pxor	mm0, mm0
		mov edi, dst

loopwrite:

		movntq 0[edi], mm0
		movntq 8[edi], mm0
		movntq 16[edi], mm0
		movntq 24[edi], mm0

		lea edi, [edi+32]
		sub i, 32
		jg loopwrite

	}
}

static inline void memzero64(void *dst)
{
	__asm {
		pxor	mm0, mm0
		mov edi, dst

		movq 0[edi], mm0
		movq 8[edi], mm0
		movq 16[edi], mm0
		movq 24[edi], mm0
		movq 32[edi], mm0
		movq 40[edi], mm0
		movq 48[edi], mm0
		movq 56[edi], mm0
	}
}

static inline void memzero128(void *dst)
{
	__asm {
		pxor	mm0, mm0
		mov edi, dst

		movq 0[edi], mm0
		movq 8[edi], mm0
		movq 16[edi], mm0
		movq 24[edi], mm0
		movq 32[edi], mm0
		movq 40[edi], mm0
		movq 48[edi], mm0
		movq 56[edi], mm0
		movq 64[edi], mm0
		movq 72[edi], mm0
		movq 80[edi], mm0
		movq 88[edi], mm0
		movq 96[edi], mm0
		movq 104[edi], mm0
		movq 112[edi], mm0
		movq 120[edi], mm0
	}
}

static inline void memzero24(void *dst)
{
	__asm {
		pxor	mm0, mm0
		mov edi, dst

		movq 0[edi], mm0
		movq 8[edi], mm0
		movq 16[edi], mm0
	}
}

static inline void memzero48(void *dst)
{
	__asm {
		pxor	mm0, mm0
		mov edi, dst

		movq 0[edi], mm0
		movq 8[edi], mm0
		movq 16[edi], mm0
		movq 24[edi], mm0
		movq 32[edi], mm0
		movq 40[edi], mm0
	}
}

static inline void memzero16(void *dst)
{
	__asm {
		pxor	mm0, mm0
		mov edi, dst

		movq 0[edi], mm0
		movq 8[edi], mm0
	}
}

static inline void memzero8(void *dst)
{
	__asm {
		pxor	mm0, mm0
		mov edi, dst

		movq 0[edi], mm0
	}
}

static inline void memset_fast_end()
{
	_mm_empty();
}

// Very optimized memcpy() routine for all AMD Athlon and Duron family.
// This code uses any of FOUR different basic copy methods, depending
// on the transfer size.
// NOTE:  Since this code uses MOVNTQ (also known as "Non-Temporal MOV" or
// "Streaming Store"), and also uses the software prefetchnta instructions,
// be sure youre running on Athlon/Duron or other recent CPU before calling!

#define TINY_BLOCK_COPY 64       // upper limit for movsd type copy
// The smallest copy uses the X86 "movsd" instruction, in an optimized
// form which is an "unrolled loop".

#define IN_CACHE_COPY 64 * 1024  // upper limit for movq/movq copy w/SW prefetch
// Next is a copy that uses the MMX registers to copy 8 bytes at a time,
// also using the "unrolled loop" optimization.   This code uses
// the software prefetch instruction to get the data into the cache.

#define UNCACHED_COPY 197 * 1024 // upper limit for movq/movntq w/SW prefetch
// For larger blocks, which will spill beyond the cache, its faster to
// use the Streaming Store instruction MOVNTQ.   This write instruction
// bypasses the cache and writes straight to main memory.  This code also
// uses the software prefetch instruction to pre-read the data.
// USE 64 * 1024 FOR THIS VALUE IF YOURE ALWAYS FILLING A "CLEAN CACHE"

#define BLOCK_PREFETCH_COPY  infinity // no limit for movq/movntq w/block prefetch 
#define CACHEBLOCK 80h // number of 64-byte blocks (cache lines) for block prefetch
// For the largest size blocks, a special technique called Block Prefetch
// can be used to accelerate the read operations.   Block Prefetch reads
// one address per cache line, for a series of cache lines, in a short loop.
// This is faster than using software prefetch.  The technique is great for
// getting maximum read bandwidth, especially in DDR memory systems.

// Inline assembly syntax for use with Visual C++


static void * memcpy_amd(void *dest, const void *src, size_t n)
{
  __asm {

	mov		ecx, [n]		// number of bytes to copy
	mov		edi, [dest]		// destination
	mov		esi, [src]		// source
	mov		ebx, ecx		// keep a copy of count

	cld
	cmp		ecx, TINY_BLOCK_COPY
	jb		$memcpy_ic_3	// tiny? skip mmx copy

	cmp		ecx, 32*1024		// dont align between 32k-64k because
	jbe		$memcpy_do_align	//  it appears to be slower
	cmp		ecx, 64*1024
	jbe		$memcpy_align_done
$memcpy_do_align:
	mov		ecx, 8			// a trick thats faster than rep movsb...
	sub		ecx, edi		// align destination to qword
	and		ecx, 111b		// get the low bits
	sub		ebx, ecx		// update copy count
	neg		ecx				// set up to jump into the array
	add		ecx, offset $memcpy_align_done
	jmp		ecx				// jump to array of movsbs

align 4
	movsb
	movsb
	movsb
	movsb
	movsb
	movsb
	movsb
	movsb

$memcpy_align_done:			// destination is dword aligned
	mov		ecx, ebx		// number of bytes left to copy
	shr		ecx, 6			// get 64-byte block count
	jz		$memcpy_ic_2	// finish the last few bytes

	cmp		ecx, IN_CACHE_COPY/64	// too big 4 cache? use uncached copy
	jae		$memcpy_uc_test

// This is small block copy that uses the MMX registers to copy 8 bytes
// at a time.  It uses the "unrolled loop" optimization, and also uses
// the software prefetch instruction to get the data into the cache.
align 16
$memcpy_ic_1:			// 64-byte block copies, in-cache copy

	prefetchnta [esi + (200*64/34+192)]		// start reading ahead

	movq	mm0, [esi+0]	// read 64 bits
	movq	mm1, [esi+8]
	movq	[edi+0], mm0	// write 64 bits
	movq	[edi+8], mm1	//    note:  the normal movq writes the
	movq	mm2, [esi+16]	//    data to cache// a cache line will be
	movq	mm3, [esi+24]	//    allocated as needed, to store the data
	movq	[edi+16], mm2
	movq	[edi+24], mm3
	movq	mm0, [esi+32]
	movq	mm1, [esi+40]
	movq	[edi+32], mm0
	movq	[edi+40], mm1
	movq	mm2, [esi+48]
	movq	mm3, [esi+56]
	movq	[edi+48], mm2
	movq	[edi+56], mm3

	add		esi, 64			// update source pointer
	add		edi, 64			// update destination pointer
	dec		ecx				// count down
	jnz		$memcpy_ic_1	// last 64-byte block?

$memcpy_ic_2:
	mov		ecx, ebx		// has valid low 6 bits of the byte count
$memcpy_ic_3:
	shr		ecx, 2			// dword count
	and		ecx, 1111b		// only look at the "remainder" bits
	neg		ecx				// set up to jump into the array
	add		ecx, offset $memcpy_last_few
	jmp		ecx				// jump to array of movsds

$memcpy_uc_test:
	cmp		ecx, UNCACHED_COPY/64	// big enough? use block prefetch copy
	jae		$memcpy_bp_1

$memcpy_64_test:
	or		ecx, ecx		// _tail end of block prefetch will jump here
	jz		$memcpy_ic_2	// no more 64-byte blocks left

// For larger blocks, which will spill beyond the cache, its faster to
// use the Streaming Store instruction MOVNTQ.   This write instruction
// bypasses the cache and writes straight to main memory.  This code also
// uses the software prefetch instruction to pre-read the data.
align 16
$memcpy_uc_1:				// 64-byte blocks, uncached copy

	prefetchnta [esi + (200*64/34+192)]		// start reading ahead

	movq	mm0,[esi+0]		// read 64 bits
	add		edi,64			// update destination pointer
	movq	mm1,[esi+8]
	add		esi,64			// update source pointer
	movq	mm2,[esi-48]
	movntq	[edi-64], mm0	// write 64 bits, bypassing the cache
	movq	mm0,[esi-40]	//    note: movntq also prevents the CPU
	movntq	[edi-56], mm1	//    from READING the destination address
	movq	mm1,[esi-32]	//    into the cache, only to be over-written
	movntq	[edi-48], mm2	//    so that also helps performance
	movq	mm2,[esi-24]
	movntq	[edi-40], mm0
	movq	mm0,[esi-16]
	movntq	[edi-32], mm1
	movq	mm1,[esi-8]
	movntq	[edi-24], mm2
	movntq	[edi-16], mm0
	dec		ecx
	movntq	[edi-8], mm1
	jnz		$memcpy_uc_1	// last 64-byte block?

	jmp		$memcpy_ic_2		// almost done

// For the largest size blocks, a special technique called Block Prefetch
// can be used to accelerate the read operations.   Block Prefetch reads
// one address per cache line, for a series of cache lines, in a short loop.
// This is faster than using software prefetch, in this case.
// The technique is great for getting maximum read bandwidth,
// especially in DDR memory systems.
$memcpy_bp_1:			// large blocks, block prefetch copy

	cmp		ecx, CACHEBLOCK			// big enough to run another prefetch loop?
	jl		$memcpy_64_test			// no, back to regular uncached copy

	mov		eax, CACHEBLOCK / 2		// block prefetch loop, unrolled 2X
	add		esi, CACHEBLOCK * 64	// move to the top of the block
align 16
$memcpy_bp_2:
	mov		edx, [esi-64]		// grab one address per cache line
	mov		edx, [esi-128]		// grab one address per cache line
	sub		esi, 128			// go reverse order
	dec		eax					// count down the cache lines
	jnz		$memcpy_bp_2		// keep grabbing more lines into cache

	mov		eax, CACHEBLOCK		// now that its in cache, do the copy
align 16
$memcpy_bp_3:
	movq	mm0, [esi   ]		// read 64 bits
	movq	mm1, [esi+ 8]
	movq	mm2, [esi+16]
	movq	mm3, [esi+24]
	movq	mm4, [esi+32]
	movq	mm5, [esi+40]
	movq	mm6, [esi+48]
	movq	mm7, [esi+56]
	add		esi, 64				// update source pointer
	movntq	[edi   ], mm0		// write 64 bits, bypassing cache
	movntq	[edi+ 8], mm1		//    note: movntq also prevents the CPU
	movntq	[edi+16], mm2		//    from READING the destination address 
	movntq	[edi+24], mm3		//    into the cache, only to be over-written,
	movntq	[edi+32], mm4		//    so that also helps performance
	movntq	[edi+40], mm5
	movntq	[edi+48], mm6
	movntq	[edi+56], mm7
	add		edi, 64				// update dest pointer

	dec		eax					// count down

	jnz		$memcpy_bp_3		// keep copying
	sub		ecx, CACHEBLOCK		// update the 64-byte block count
	jmp		$memcpy_bp_1		// keep processing chunks

// The smallest copy uses the X86 "movsd" instruction, in an optimized
// form which is an "unrolled loop".   Then it handles the last few bytes.
align 4
	movsd
	movsd			// perform last 1-15 dword copies
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd			// perform last 1-7 dword copies
	movsd
	movsd
	movsd
	movsd
	movsd
	movsd

$memcpy_last_few:		// dword aligned from before movsds
	mov		ecx, ebx	// has valid low 2 bits of the byte count
	and		ecx, 11b	// the last few cows must come home
	jz		$memcpy_final	// no more, lets leave
	rep		movsb		// the last 1, 2, or 3 bytes

$memcpy_final: 
//	emms				// clean up the MMX state
	sfence				// flush the write buffer
	mov		eax, [dest]	// ret value = destination pointer

    }
}

#elif defined(_M_X64)
static inline void memzero24(void *dst)
{
	int32_t j;
	int32_t *d = (int32_t *)dst;
	for (j=0;j<24;j+=4)
	{
		d[j] = 0;
	}
}
static inline void memset_fast_end() {}
#else
static inline void memzero_fast16(void *dst,  unsigned long i)
{
	int32_t j;
	int32_t *d = (int32_t *)dst;
	for (j=0;j<i;j+=4)
	{
		d[j] = 0;
	}
}
static inline void memzero24(void *dst)
{
	int32_t j;
	int32_t *d = (int32_t *)dst;
	for (j=0;j<24;j+=4)
	{
		d[j] = 0;
	}
}
static inline void memset_fast_end() {}
#endif

#define UNDEFINED_REFERENCE ((int)0x80000000)
typedef int32_t h264_ref_t;

#define ET_SIZE 300      //!< size of error text buffer
extern char errortext[ET_SIZE]; //!< buffer for error message for exit with error()
extern int sse2_flag, mmx_flag, sse_flag, sse3_flag, sse4_1_flag;
/***********************************************************************
 * T y p e    d e f i n i t i o n s    f o r    J M
 ***********************************************************************
 */

typedef enum
{
  LumaComp = 0,
  CrComp = 1,
  CbComp = 2
} Color_Component;

/***********************************************************************
 * D a t a    t y p e s   f o r  C A B A C
 ***********************************************************************
 */

typedef struct pix_pos
{
  int   available;
  int   mb_addr;
  short x;
  short y;
  short pos_x;
  short pos_y;
} PixelPos;

//! struct to characterize the state of the arithmetic coding engine
typedef struct
{
  unsigned int    Drange;
  unsigned int    Dvalue;
  int             DbitsLeft;
  byte            *Dcodestrm;
  int             *Dcodestrm_len;
} DecodingEnvironment;

typedef DecodingEnvironment *DecodingEnvironmentPtr;

typedef short MotionVector[2];

//! definition of motion parameters
typedef struct pic_motion
{
	h264_ref_t ref_pic_id;
	h264_ref_t ref_id;
	MotionVector mv;
	char ref_idx;
} PicMotion;

// TODO: benski> might be more efficient to make a [list][subblock_y][subblock_x] array of these values instead of parallel arrays
typedef struct motion_params
{
	PicMotion **motion[2];
  byte **     moving_block;
} MotionParams;

//! struct for context management
typedef struct
{
  uint16_t state;         // index into state-table CP
  unsigned char  MPS;           // Least Probable Symbol 0/1 CP
  unsigned char dummy;          // for alignment
} BiContextType;

typedef BiContextType *BiContextTypePtr;


/**********************************************************************
 * C O N T E X T S   F O R   T M L   S Y N T A X   E L E M E N T S
 **********************************************************************
 */

#define NUM_MB_TYPE_CTX  11
#define NUM_B8_TYPE_CTX  9
#define NUM_MV_RES_CTX   10
#define NUM_REF_NO_CTX   6
#define NUM_DELTA_QP_CTX 4
#define NUM_MB_AFF_CTX 4
#define NUM_TRANSFORM_SIZE_CTX 3

// structures that will be declared somewhere else
struct storable_picture;
struct datapartition;
struct syntaxelement;

typedef struct
{
  BiContextType mb_type_contexts [3][NUM_MB_TYPE_CTX];
  BiContextType b8_type_contexts [2][NUM_B8_TYPE_CTX];
  BiContextType mv_res_contexts  [2][NUM_MV_RES_CTX];
  BiContextType ref_no_contexts  [2][NUM_REF_NO_CTX];
  BiContextType delta_qp_contexts[NUM_DELTA_QP_CTX];
  BiContextType mb_aff_contexts  [NUM_MB_AFF_CTX];
} MotionInfoContexts;

#define NUM_IPR_CTX    2
#define NUM_CIPR_CTX   4
#define NUM_CBP_CTX    4
#define NUM_BCBP_CTX   4
#define NUM_MAP_CTX   15
#define NUM_LAST_CTX  15
#define NUM_ONE_CTX    5
#define NUM_ABS_CTX    5


typedef struct
{
  BiContextType  transform_size_contexts [NUM_TRANSFORM_SIZE_CTX];
  BiContextType  ipr_contexts [NUM_IPR_CTX];
  BiContextType  cipr_contexts[NUM_CIPR_CTX];
  BiContextType  cbp_contexts [3][NUM_CBP_CTX];
  BiContextType  bcbp_contexts[NUM_BLOCK_TYPES][NUM_BCBP_CTX];
  BiContextType  map_contexts [2][NUM_BLOCK_TYPES][NUM_MAP_CTX+1]; // +1 for better alignment
  BiContextType  last_contexts[2][NUM_BLOCK_TYPES][NUM_LAST_CTX+1]; // +1 for better alignment
  BiContextType  one_contexts [NUM_BLOCK_TYPES][NUM_ONE_CTX];
  BiContextType  abs_contexts [NUM_BLOCK_TYPES][NUM_ABS_CTX];
} TextureInfoContexts;


//*********************** end of data type definition for CABAC *******************

/***********************************************************************
 * N e w   D a t a    t y p e s   f o r    T M L
 ***********************************************************************
 */

/*! Buffer structure for decoded reference picture marking commands */
typedef struct DecRefPicMarking_s
{
  int memory_management_control_operation;
  int difference_of_pic_nums_minus1;
  int long_term_pic_num;
  int long_term_frame_idx;
  int max_long_term_frame_idx_plus1;
  struct DecRefPicMarking_s *Next;
} DecRefPicMarking_t;


//! definition of pic motion parameters
typedef struct pic_motion_params2
{
  h264_ref_t    ref_pic_id;    //!< reference picture identifier [list][subblock_y][subblock_x]
  h264_ref_t    ref_id;        //!< reference picture identifier [list][subblock_y][subblock_x]
  short    mv[2];         //!< motion vector       [list][subblock_x][subblock_y][component]
  char     ref_idx;       //!< reference picture   [list][subblock_y][subblock_x]
  byte     mb_field;      //!< field macroblock indicator
  byte     field_frame;   //!< indicates if co_located is field or frame.
} PicMotionParams2;

//! Macroblock
typedef struct macroblock
{
  struct slice       *p_Slice;                    //!< pointer to the current slice
  struct img_par     *p_Vid;                      //!< pointer to VideoParameters
  struct inp_par     *p_Inp;
  int                 mbAddrX;                    //!< current MB address
  int mb_x;
  int mb_y;
  int block_x;
  int block_y;
  int block_y_aff;
  int pix_x;
  int pix_y;
  int pix_c_x;
  int pix_c_y;

  int subblock_x;
  int subblock_y;

  int           qp;                    //!< QP luma
  int           qpc[2];                //!< QP chroma
  int           qp_scaled[MAX_PLANE];  //!< QP scaled for all comps.
  Boolean       is_lossless;
  Boolean       is_intra_block;
  Boolean       is_v_block;

  short         slice_nr;
  short         delta_quant;          //!< for rate control

  struct macroblock   *mb_up;   //!< pointer to neighboring MB (CABAC)
  struct macroblock   *mb_left; //!< pointer to neighboring MB (CABAC)

  // some storage of macroblock syntax elements for global access
  int           mb_type;
  short         mvd[2][BLOCK_MULTIPLE][BLOCK_MULTIPLE][2];      //!< indices correspond to [forw,backw][block_y][block_x][x,y]
  int           cbp;
  int64         cbp_blk     [3];
  int64         cbp_bits    [3];
  int64         cbp_bits_8x8[3];

  int           i16mode;
  char          b8mode[4];
  char          b8pdir[4];
  char          ei_flag;             //!< error indicator flag that enables concealment
  char          dpl_flag;            //!< error indicator flag that signals a missing data partition
  char          ipmode_DPCM;

  short         DFDisableIdc;
  short         DFAlphaC0Offset;
  short         DFBetaOffset;

  char          c_ipred_mode;       //!< chroma intra prediction mode
  Boolean       mb_field;

  int          skip_flag;

  int mb_addr_left, mb_addr_up, mb_addr_upper_right, mb_addr_upper_left;
  Boolean mb_avail_left, mb_avail_up, mb_avail_upper_right, mb_avail_upper_left;

  Boolean       luma_transform_size_8x8_flag;
  Boolean       NoMbPartLessThan8x8Flag;

  void (*itrans_8x8)(struct macroblock *currMB, ColorPlane pl, int ioff, int joff);

  void (*GetMVPredictor) (struct macroblock *currMB, PixelPos *block, 
    short pmv[2], short ref_frame, struct pic_motion **motion, int mb_x, int mb_y, int blockshape_x, int blockshape_y);

  int (*read_and_store_CBP_block_bit) (struct macroblock *currMB, DecodingEnvironmentPtr  dep_dp, int type);
  char (*readRefPictureIdx)   (struct syntaxelement *currSE, struct datapartition *dP, int list);

} Macroblock;

//! Syntaxelement
typedef struct syntaxelement
{
  int           value1;                //!< numerical value of syntax element
  int           value2;                //!< for blocked symbols, e.g. run/level
  int           len;                   //!< length of code
  //int           inf;                   //!< info part of CAVLC code

#if TRACE
  #define       TRACESTRING_SIZE 100           //!< size of trace string
  char          tracestring[TRACESTRING_SIZE]; //!< trace string
#endif

  //! for mapping of CAVLC to syntaxElement
  void  (*mapping)(int len, int info, int *value1, int *value2);
} SyntaxElement;


//! Bitstream
typedef struct
{
  // CABAC Decoding
  int           read_len;           //!< actual position in the codebuffer, CABAC only
  int           code_len;           //!< overall codebuffer length, CABAC only
  // CAVLC Decoding
  int           frame_bitoffset;    //!< actual position in the codebuffer, bit-oriented, CAVLC only
  int           bitstream_length;   //!< over codebuffer lnegth, byte oriented, CAVLC only

  byte          *streamBuffer;      //!< actual codebuffer for read bytes
} Bitstream;


/* === 4x4 block typedefs === */
// 32 bit precision
typedef int h264_int_block_row_t[BLOCK_SIZE];
typedef h264_int_block_row_t h264_int_block_t[BLOCK_SIZE];
// 16 bit precision
typedef int16_t h264_short_block_row_t[BLOCK_SIZE];
typedef h264_short_block_row_t h264_short_block_t[BLOCK_SIZE];
// 8 bit precision

/* === 8x8 block typedefs === */
// 32 bit precision
typedef int h264_int_8x8block_row_t[BLOCK_SIZE_8x8];
typedef h264_int_8x8block_row_t h264_int_8x8block_t[BLOCK_SIZE_8x8];
// 16 bit precision
typedef int16_t h264_short_8x8block_row_t[BLOCK_SIZE_8x8];
typedef h264_short_8x8block_row_t h264_short_8x8block_t[BLOCK_SIZE_8x8];
// 8 bit precision
typedef imgpel h264_imgpel_8x8block_row_t[BLOCK_SIZE_8x8];
typedef h264_imgpel_8x8block_row_t h264_imgpel_8x8block_t[BLOCK_SIZE_8x8];

/* === 16x16 block typedefs === */
// 32 bit precision
typedef int h264_int_macroblock_row_t[MB_BLOCK_SIZE];
typedef h264_int_macroblock_row_t h264_int_macroblock_t[MB_BLOCK_SIZE];
// 16 bit precision
typedef int16_t h264_short_macroblock_row_t[MB_BLOCK_SIZE];
typedef h264_short_macroblock_row_t h264_short_macroblock_t[MB_BLOCK_SIZE];
// 8 bit precision
typedef imgpel h264_imgpel_macroblock_row_t[MB_BLOCK_SIZE];
typedef h264_imgpel_macroblock_row_t h264_imgpel_macroblock_t[MB_BLOCK_SIZE];




typedef int h264_pic_position[2];
typedef byte h264_4x4_byte[BLOCK_SIZE][BLOCK_SIZE];
typedef h264_4x4_byte h264_nz_coefficient[3];

//! DataPartition
typedef struct datapartition
{

  Bitstream           *bitstream;
  DecodingEnvironment de_cabac;

} DataPartition;

//! Slice
typedef struct slice
{
  struct img_par      *p_Vid;
  struct inp_par      *p_Inp;
  pic_parameter_set_rbsp_t *active_pps;
  seq_parameter_set_rbsp_t *active_sps;

  struct colocated_params *p_colocated;
  struct colocated_params *Co_located_JV[MAX_PLANE];  //!< p_colocated to be used during 4:4:4 independent mode decoding

  int                 mb_aff_frame_flag;
  int                 direct_spatial_mv_pred_flag;       //!< Indicator for direct mode type (1 for Spatial, 0 for Temporal)
  int                 num_ref_idx_l0_active;             //!< number of available list 0 references
  int                 num_ref_idx_l1_active;             //!< number of available list 1 references

  int                 qp;
  int                 slice_qp_delta;
  int                 qs;
  int                 slice_qs_delta;
  int                 slice_type;    //!< slice type
  int                 model_number;  //!< cabac model number
  PictureStructure    structure;     //!< Identify picture structure type
  int                 start_mb_nr;   //!< MUST be set by NAL even in case of ei_flag == 1
  int                 max_part_nr;
  int                 dp_mode;       //!< data partitioning mode
  int                 last_dquant;

  //  int                 last_mb_nr;    //!< only valid when entropy coding == CABAC
  DataPartition       *partArr;      //!< array of partitions
  MotionInfoContexts  *mot_ctx;      //!< pointer to struct of context models for use in CABAC
  TextureInfoContexts *tex_ctx;      //!< pointer to struct of context models for use in CABAC

  int mvscale[6][MAX_REFERENCE_PICTURES];

  int                 ref_pic_list_reordering_flag_l0;
  int                 *reordering_of_pic_nums_idc_l0;
  int                 *abs_diff_pic_num_minus1_l0;
  int                 *long_term_pic_idx_l0;
  int                 ref_pic_list_reordering_flag_l1;
  int                 *reordering_of_pic_nums_idc_l1;
  int                 *abs_diff_pic_num_minus1_l1;
  int                 *long_term_pic_idx_l1;
  

  short               DFDisableIdc;     //!< Disable deblocking filter on slice
  short               DFAlphaC0Offset;  //!< Alpha and C0 offset for filtering slice
  short               DFBetaOffset;     //!< Beta offset for filtering slice

  int                 pic_parameter_set_id;   //!<the ID of the picture parameter set the slice is reffering to

  int                 dpB_NotPresent;    //!< non-zero, if data partition B is lost
  int                 dpC_NotPresent;    //!< non-zero, if data partition C is lost


	__declspec(align(32)) h264_imgpel_macroblock_t mb_pred[MAX_PLANE];
	__declspec(align(32)) h264_imgpel_macroblock_t mb_rec[MAX_PLANE];
	__declspec(align(32)) union
	{
		__declspec(align(32)) h264_short_8x8block_t mb_rres8[MAX_PLANE][4];
		__declspec(align(32)) h264_short_macroblock_t cof[MAX_PLANE];
		__declspec(align(32)) h264_short_block_t cof4[MAX_PLANE][16]; // TODO: get this to work, one of these days
		__declspec(align(32)) h264_short_macroblock_t ipcm[MAX_PLANE];
	};

  int cofu[16];

  // Scaling matrix info
  int  InvLevelScale4x4_Intra[3][6][4][4];
  int  InvLevelScale4x4_Inter[3][6][4][4];
  int  InvLevelScale8x8_Intra[3][6][64];
  int  InvLevelScale8x8_Inter[3][6][64];

  int  *qmatrix[12];

  // Cabac
	// TODO: we could optimize coefficient reading by storing the levels/runs instead of coefficients
  int16_t  coeff[64]; // one more for EOB
  int  coeff_ctr;
  int  pos;

  //weighted prediction
  unsigned int apply_weights;
  unsigned int luma_log2_weight_denom;
  unsigned int chroma_log2_weight_denom;
  int wp_weight[2][MAX_REFERENCE_PICTURES][3];  // weight in [list][index][component] order
  int wp_offset[6][MAX_REFERENCE_PICTURES][3];  // offset in [list][index][component] order
  int wbp_weight[6][MAX_REFERENCE_PICTURES][MAX_REFERENCE_PICTURES][3]; //weight in [list][fw_index][bw_index][component] order
  int wp_round_luma;
  int wp_round_chroma;

  void (*read_CBP_and_coeffs_from_NAL) (Macroblock *currMB);
  int  (*decode_one_component     ) (Macroblock *currMB, ColorPlane curr_plane, struct video_image *image, struct storable_picture *dec_picture);
  int  (*readSlice                ) (struct img_par *, struct inp_par *);  
  int  (*nal_startcode_follows    ) (struct slice*, int );
  void (*read_motion_info_from_NAL) (Macroblock *currMB);
  void (*read_one_macroblock      ) (Macroblock *currMB);
  void (*interpret_mb_mode        ) (Macroblock *currMB);
	void (*compute_colocated        ) (struct slice *currSlice, struct colocated_params *p, struct storable_picture **listX[6]);

  void (*linfo_cbp_intra) (int len,int info,int *cbp, int *dummy);
  void (*linfo_cbp_inter) (int len,int info,int *cbp, int *dummy);
} Slice;

//****************************** ~DM ***********************************

// image parameters
typedef struct img_par
{
  struct inp_par      *p_Inp;
  pic_parameter_set_rbsp_t *active_pps;
  seq_parameter_set_rbsp_t *active_sps;
  seq_parameter_set_rbsp_t SeqParSet[MAXSPS];
  pic_parameter_set_rbsp_t PicParSet[MAXPPS];

  struct sei_params        *p_SEI;

  struct old_slice_par *old_slice;
  int number;                                 //!< frame number
  unsigned int current_mb_nr; // bitstream order
  unsigned int num_dec_mb;
  short        current_slice_nr;
  int *intra_block;
  
  int qp;                                     //!< quant for the current frame

  int sp_switch;                              //!< 1 for switching sp, 0 for normal sp  
  int type;                                   //!< image type INTER/INTRA
  int width;
  int height;
  int width_cr;                               //!< width chroma  
  int height_cr;                              //!< height chroma
  int mb_x;
  int mb_y;
  int block_x;
  int block_y;
  int pix_c_x;
  int pix_c_y;

  int allrefzero;

  byte **ipredmode;                  //!< prediction type [90][74]
  h264_nz_coefficient *nz_coeff;
  int **siblock;
  int cod_counter;                   //!< Current count of number of skipped macroblocks in a row

  int structure;                     //!< Identify picture structure type

  Slice      *currentSlice;          //!< pointer to current Slice data struct
  Macroblock *mb_data;               //!< array containing all MBs of a whole frame
  Macroblock *mb_data_JV[MAX_PLANE]; //!< mb_data to be used for 4:4:4 independent mode
  int colour_plane_id;               //!< colour_plane_id of the current coded slice
  int ChromaArrayType;

  // For MB level frame/field coding
  int mb_aff_frame_flag;

  // for signalling to the neighbour logic that this is a deblocker call
  int DeblockCall;
  byte mixedModeEdgeFlag;

  // picture error concealment
  // concealment_head points to first node in list, concealment_end points to
  // last node in list. Initialize both to NULL, meaning no nodes in list yet
  struct concealment_node *concealment_head;
  struct concealment_node *concealment_end;

  DecRefPicMarking_t *dec_ref_pic_marking_buffer;                    //!< stores the memory management control operations

  int num_ref_idx_l0_active;             //!< number of forward reference
  int num_ref_idx_l1_active;             //!< number of backward reference

  int slice_group_change_cycle;

  int redundant_pic_cnt;

  unsigned int pre_frame_num;           //!< store the frame_num in the last decoded slice. For detecting gap in frame_num.
  int non_conforming_stream;

  // End JVT-D101
  // POC200301: from unsigned int to int
  int toppoc;      //poc for this top field // POC200301
  int bottompoc;   //poc of bottom field of frame
  int framepoc;    //poc of this frame // POC200301
  unsigned int frame_num;   //frame_num for this frame
  unsigned int field_pic_flag;
  byte         bottom_field_flag;

  //the following is for slice header syntax elements of poc
  // for poc mode 0.
  unsigned int pic_order_cnt_lsb;
  int delta_pic_order_cnt_bottom;
  // for poc mode 1.
  int delta_pic_order_cnt[3];

  // ////////////////////////
  // for POC mode 0:
  signed   int PrevPicOrderCntMsb;
  unsigned int PrevPicOrderCntLsb;
  signed   int PicOrderCntMsb;

  // for POC mode 1:
  unsigned int AbsFrameNum;
  signed int ExpectedPicOrderCnt, PicOrderCntCycleCnt, FrameNumInPicOrderCntCycle;
  unsigned int PreviousFrameNum, FrameNumOffset;
  int ExpectedDeltaPerPicOrderCntCycle;
  int PreviousPOC, ThisPOC;
  int PreviousFrameNumOffset;
  // /////////////////////////

  int idr_flag;
  int nal_reference_idc;                       //!< nal_reference_idc from NAL unit

  int idr_pic_id;

  int MaxFrameNum;

  unsigned int PicWidthInMbs;
  unsigned int PicHeightInMapUnits;
  unsigned int FrameHeightInMbs;
  unsigned int PicHeightInMbs;
  unsigned int PicSizeInMbs;
  unsigned int FrameSizeInMbs;
  unsigned int oldFrameSizeInMbs;

  int no_output_of_prior_pics_flag;
  int long_term_reference_flag;
  int adaptive_ref_pic_buffering_flag;

  int last_has_mmco_5;
  int last_pic_bottom_field;

  // Fidelity Range Extensions Stuff
  short bitdepth_luma;
  short bitdepth_chroma;
  int bitdepth_scale[2];
  int bitdepth_luma_qp_scale;
  int bitdepth_chroma_qp_scale;
  unsigned int dc_pred_value_comp[MAX_PLANE]; //!< component value for DC prediction (depends on component pel bit depth)
  int max_pel_value_comp[MAX_PLANE];       //!< max value that one picture element (pixel) can take (depends on pic_unit_bitdepth)
  int Transform8x8Mode;
  int profile_idc;
  int yuv_format;
  int lossless_qpprime_flag;
  int num_blk8x8_uv;
  int num_uv_blocks;
  int num_cdc_coeff;
  int mb_cr_size_x;
  int mb_cr_size_y;
  int mb_cr_size_x_blk;
  int mb_cr_size_y_blk;
  int mb_size[3][2];                         //!< component macroblock dimensions
  int mb_size_blk[3][2];                     //!< component macroblock dimensions 
  int mb_size_shift[3][2];
  int subpel_x;
  int subpel_y;
  int shiftpel_x;
  int shiftpel_y;

  int max_vmv_r;                             //!< maximum vertical motion vector range in luma quarter frame pixel units for the current level_idc
  int max_mb_vmv_r;                          //!< maximum vertical motion vector range in luma quarter pixel units for the current level_idc

  // picture error concealment
  int last_ref_pic_poc;
  int ref_poc_gap;
  int poc_gap;
  int earlier_missing_poc;
  unsigned int frame_to_conceal;
  int IDR_concealment_flag;
  int conceal_slice_type;

  // random access point decoding
  int recovery_point;
  int recovery_point_found;
  int recovery_frame_cnt;
  int recovery_frame_num;
  int recovery_poc;

  int separate_colour_plane_flag;

  int frame_number;
  int init_bl_done;

  // Redundant slices. Should be moved to another structure and allocated only if extended profile
  unsigned int previous_frame_num; //!< frame number of previous slice
  int ref_flag[17];                //!< 0: i-th previous frame is incorrect
  //!< non-zero: i-th previous frame is correct
  int Is_primary_correct;          //!< if primary frame is correct, 0: incorrect
  int Is_redundant_correct;        //!< if redundant frame is correct, 0:incorrect
  int redundant_slice_ref_idx;     //!< reference index of redundant slice

  //FILE *p_log;                     //!< SNR file
  int LastAccessUnitExists;
  int NALUCount;

  Boolean global_init_done;

  int *qp_per_matrix;
  int *qp_rem_matrix;

  struct frame_store *last_out_fs;
  int pocs_in_dpb[100];


  struct storable_picture *dec_picture;
  struct storable_picture *dec_picture_JV[MAX_PLANE];  //!< dec_picture to be used during 4:4:4 independent mode decoding
  struct storable_picture *no_reference_picture; //!< dummy storable picture for recovery point
  struct storable_picture **listX[6];

  // Error parameters
  struct object_buffer  *erc_object_list;
  struct ercVariables_s *erc_errorVar;

  int erc_mvperMB;
  struct img_par *erc_img;
  int ec_flag[SE_MAX_ELEMENTS];        //!< array to set errorconcealment

	struct memory_input_struct *mem_input;

  struct frame_store *out_buffer;

  struct storable_picture *pending_output;
  int    pending_output_state;
  int    recovery_flag;

  // dpb
  struct decoded_picture_buffer *p_Dpb;

  char listXsize[6];
  // report
  char cslice_type[9];  
  // FMO
  int *MbToSliceGroupMap;
  int *MapUnitToSliceGroupMap;
  int  NumberOfSliceGroups;    // the number of slice groups -1 (0 == scan order, 7 == maximum)

#if (ENABLE_OUTPUT_TONEMAPPING)
  struct tone_mapping_struct_s *seiToneMapping;
#endif

	// benski> buffer of storablge pictures ready for output.
	// might be able to optimize a tad by making a ringbuffer, but i doubt it matters
	struct storable_picture **out_pictures;
	size_t size_out_pictures;
	size_t num_out_pictures;

	ImageCache image_cache[2]; // [0] is luma [1] is chroma (shared for both planes)
	MotionCache motion_cache;

	h264_pic_position *PicPos; //! Helper array to access macroblock positions. 

	NALU_t *nalu; // a cache so we don't re-alloc every time

  void (*getNeighbour)        (const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix);
	void (*getNeighbourPX_NoPos)(const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix);
	void (*getNeighbourXP_NoPos)(const Macroblock *currMB, int xN, int yN, const int mb_size[2], PixelPos *pix);
	void (*getNeighbourLuma)    (const Macroblock *currMB, int xN, int yN, PixelPos *pix);
	void (*getNeighbourPXLuma)  (const Macroblock *currMB, int xN, int yN, PixelPos *pix);
	void (*getNeighbourXPLuma)  (const Macroblock *currMB, int xN, int yN, PixelPos *pix);
	void (*getNeighbourLeftLuma)(const Macroblock *currMB, PixelPos *pix);
	void (*getNeighbourNXLuma)  (const Macroblock *currMB, int yN, PixelPos *pix); // xN<0, yN full range
	void (*getNeighbourLeft)    (const Macroblock *currMB, const int mb_size[2], PixelPos *pix); // xN<0, yN=0
	void (*getNeighbourUp)      (const Macroblock *currMB, const int mb_size[2], PixelPos *pix); // xN=0, yN<0
	void (*getNeighbourNX)      (const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix); // xN<0, yN full range
	void (*getNeighbourNP)      (const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix); // xN<0, yN>=0
	void (*getNeighbourNPChromaNB)(const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix); // xN<0, yN>=0
	void (*getNeighbour0X)      (const Macroblock *currMB, int yN, const int mb_size[2], PixelPos *pix); // xN=0, yN full range
	void (*getNeighbour0XLuma)      (const Macroblock *currMB, int yN, PixelPos *pix); // xN=0, yN full range
	void (*getNeighbourX0)      (const Macroblock *currMB, int xN, const int mb_size[2], PixelPos *pix); // xN full range, yN = 0
	void (*getNeighbourUpLuma)  (const Macroblock *currMB, PixelPos *pix); // xN=0, yN<0
	void (*getNeighbourNPLumaNB)(const Macroblock *currMB, int yN, PixelPos *pix);
	void (*getNeighbourPXLumaNB)  (const Macroblock *currMB, int xN, int yN, PixelPos *pix);
	void (*getNeighbourPXLumaNB_NoPos)(const Macroblock *currMB, int yN, PixelPos *pix);
	void (*getNeighbourPPLumaNB)  (const Macroblock *currMB, int xN, int yN, PixelPos *pix);
	void (*getNeighbourXPLumaNB)  (const Macroblock *currMB, int xN, int yN, PixelPos *pix);
	void (*getNeighbourXPLumaNB_NoPos)(const Macroblock *currMB, int xN, int yN, PixelPos *pix);
  void (*get_mb_block_pos) (const h264_pic_position *PicPos, int mb_addr, short *x, short *y);
  void (*GetStrength)      (byte Strength[16], Macroblock *MbQ, int dir,int edge, int mvlimit, struct storable_picture *p);
  void (*EdgeLoopLuma)     (ColorPlane pl, struct video_image *image, const byte Strength[16], Macroblock *MbQ, int dir, int edge, struct storable_picture *p);
  void (*EdgeLoopChroma)   (struct video_image *image, byte Strength[16], Macroblock *MbQ, int dir, int edge, int uv, struct storable_picture *p);
} VideoParameters;

// input parameters from configuration file
typedef struct inp_par
{
  int intra_profile_deblocking;               //!< Loop filter usage determined by flags and parameters in bitstream 

  // Output sequence format related variables
  FrameFormat output;                   //!< output related information

#ifdef _LEAKYBUCKET_
  unsigned long R_decoder;                //!< Decoder Rate in HRD Model
  unsigned long B_decoder;                //!< Decoder Buffer size in HRD model
  unsigned long F_decoder;                //!< Decoder Initial buffer fullness in HRD model
  char LeakyBucketParamFile[FILE_NAME_SIZE];         //!< LeakyBucketParamFile
#endif

  // picture error concealment
  int ref_poc_gap;
  int poc_gap;
} InputParameters;

typedef struct old_slice_par
{
   unsigned field_pic_flag;   
   unsigned frame_num;
   int      nal_ref_idc;
   unsigned pic_oder_cnt_lsb;
   int      delta_pic_oder_cnt_bottom;
   int      delta_pic_order_cnt[2];
   byte     bottom_field_flag;
   byte     idr_flag;
   int      idr_pic_id;
   int      pps_id;   
} OldSliceParams;

typedef struct decoder_params
{
  InputParameters   *p_Inp;          //!< Input Parameters
  VideoParameters   *p_Vid;          //!< Image Parameters
  
} DecoderParams;

#ifdef TRACE
extern FILE *p_trace; //!< Trace file
extern int bitcounter;
#endif

// prototypes

extern void error(char *text, int code);

// dynamic mem allocation
extern int  init_global_buffers(VideoParameters *p_Vid);
extern void free_global_buffers(VideoParameters *p_Vid);

extern int RBSPtoSODB(byte *streamBuffer, int last_byte_pos);
extern int EBSPtoRBSP(byte *streamBuffer, int end_bytepos);

void FreePartition (DataPartition *dp, int n);
DataPartition *AllocPartition(int n);

void tracebits(const char *trace_str,  int len,  int info,int value1);
void tracebits2(const char *trace_str, int len, int info);

unsigned CeilLog2   ( unsigned uiVal);
unsigned CeilLog2_sf( unsigned uiVal);

// For 4:4:4 independent mode
extern void change_plane_JV( VideoParameters *p_Vid, int nplane );
extern void make_frame_picture_JV(VideoParameters *p_Vid);


#endif


