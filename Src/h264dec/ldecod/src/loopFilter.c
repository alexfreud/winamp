
/*!
*************************************************************************************
* \file loopFilter.c
*
* \brief
*    Filter to reduce blocking artifacts on a macroblock level.
*    The filter strength is QP dependent.
*
* \author
*    Contributors:
*    - Peter List       Peter.List@t-systems.de:  Original code                                 (13-Aug-2001)
*    - Jani Lainema     Jani.Lainema@nokia.com:   Some bug fixing, removal of recursiveness     (16-Aug-2001)
*    - Peter List       Peter.List@t-systems.de:  inplace filtering and various simplifications (10-Jan-2002)
*    - Anthony Joch     anthony@ubvideo.com:      Simplified switching between filters and
*                                                 non-recursive default filter.                 (08-Jul-2002)
*    - Cristina Gomila  cristina.gomila@thomson.net: Simplification of the chroma deblocking
*                                                    from JVT-E089                              (21-Nov-2002)
*    - Alexis Michael Tourapis atour@dolby.com:   Speed/Architecture improvements               (08-Feb-2007)
*************************************************************************************
*/

#include "global.h"
#include "image.h"
#include "mb_access.h"
#include "loopfilter.h"

/*********************************************************************************************************/

// NOTE: In principle, the alpha and beta tables are calculated with the formulas below
//       Alpha( qp ) = 0.8 * (2^(qp/6)  -  1)
//       Beta ( qp ) = 0.5 * qp  -  7

// The tables actually used have been "hand optimized" though (by Anthony Joch). So, the
// table values might be a little different to formula-generated values. Also, the first
// few values of both tables is set to zero to force the filter off at low qp’s

static const byte ALPHA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,4,4,5,6,  7,8,9,10,12,13,15,17,  20,22,25,28,32,36,40,45,  50,56,63,71,80,90,101,113,  127,144,162,182,203,226,255,255} ;
static const byte  BETA_TABLE[52]  = {0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,2,2,2,3,  3,3,3, 4, 4, 4, 6, 6,   7, 7, 8, 8, 9, 9,10,10,  11,11,12,12,13,13, 14, 14,   15, 15, 16, 16, 17, 17, 18, 18} ;
static const byte CLIP_TAB[52][5]  =
{
	{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},{ 0, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 0, 1, 1},{ 0, 0, 1, 1, 1},{ 0, 0, 1, 1, 1},{ 0, 1, 1, 1, 1},
	{ 0, 1, 1, 1, 1},{ 0, 1, 1, 1, 1},{ 0, 1, 1, 1, 1},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 1, 2, 2},{ 0, 1, 2, 3, 3},
	{ 0, 1, 2, 3, 3},{ 0, 2, 2, 3, 3},{ 0, 2, 2, 4, 4},{ 0, 2, 3, 4, 4},{ 0, 2, 3, 4, 4},{ 0, 3, 3, 5, 5},{ 0, 3, 4, 6, 6},{ 0, 3, 4, 6, 6},
	{ 0, 4, 5, 7, 7},{ 0, 4, 5, 8, 8},{ 0, 4, 6, 9, 9},{ 0, 5, 7,10,10},{ 0, 6, 8,11,11},{ 0, 6, 8,13,13},{ 0, 7,10,14,14},{ 0, 8,11,16,16},
	{ 0, 9,12,18,18},{ 0,10,13,20,20},{ 0,11,15,23,23},{ 0,13,17,25,25}
} ;

static const char chroma_edge[2][4][4] = //[dir][edge][yuv_format]
{ { {-4, 0, 0, 0},
{-4,-4,-4, 4},
{-4, 4, 4, 8},
{-4,-4,-4, 12}},

{ {-4, 0,  0,  0},
{-4,-4,  4,  4},
{-4, 4,  8,  8},
{-4,-4, 12, 12}}};

static const int pelnum_cr[2][4] =  {{0,8,16,16}, {0,8, 8,16}};  //[dir:0=vert, 1=hor.][yuv_format]

/* YUV420 & non-aff optimized functions */
void EdgeLoopLuma_Vert_YUV420(VideoImage *image, const uint8_t Strength[4], Macroblock *MbQ, PixelPos pixMB, Macroblock *MbP);
void EdgeLoopLuma_Horiz_YUV420(VideoImage *image, const byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, PixelPos pixMB, Macroblock *MbP);
void EdgeLoopLumaMBAff_Vert_YUV420(VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, StorablePicture *p);
void EdgeLoopChroma_Vert_YUV420(VideoImage *image, const uint8_t Strength[4], Macroblock *MbQ, int uv, PixelPos pixMB, Macroblock *MbP);
void EdgeLoopChroma_Horiz_YUV420(VideoImage *image, const byte Strength[4], Macroblock *MbQ, int uv, PixelPos pixMB, Macroblock *MbP);
void EdgeLoopChromaMBAff_Vert_YUV420(VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, int uv, StorablePicture *p);
void GetStrength_Vert_YUV420(uint8_t Strength[4], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p, PixelPos pixMB, Macroblock *MbP);
void GetStrength_Vert_YUV420_All(uint8_t Strength[4][4], Macroblock *MbQ, int mvlimit, StorablePicture *p, int pos_x, int pos_y, Macroblock *MbP, int luma_transform_size_8x8_flag);
void GetStrength_Horiz_YUV420(uint8_t Strength[16], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p, PixelPos pixMB, Macroblock *MbP);
void GetStrength_Horiz_YUV420_All(uint8_t Strength[4][4], Macroblock *MbQ, int mvlimit, StorablePicture *p, int pos_x, int pos_y, Macroblock *MbP, int luma_transform_size_8x8_flag);
void GetStrength_MBAff_Vert_YUV420(byte Strength[16], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p);
void GetStrengthMBAff_Horiz_YUV420(byte Strength[16], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p);
static void Deblock_YUV420(VideoParameters *p_Vid, StorablePicture *p, int MbQAddr);
static void Deblock_YUV420_MBAFF(VideoParameters *p_Vid, StorablePicture *p, int MbQAddr);
/* */
void EdgeLoopChromaNormal_Vert(VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, int uv, StorablePicture *p);
void EdgeLoopLumaNormal_Vert(ColorPlane pl, VideoImage *image, const byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, StorablePicture *p);
void GetStrengthNormal_Vert(byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p);
void GetStrengthNormal_Horiz(byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p);
static void GetStrengthNormal (byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int dir,int edge, int mvlimit,StorablePicture *p);
static void GetStrengthMBAff  (byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int dir,int edge, int mvlimit,StorablePicture *p);
static void EdgeLoopLumaNormal(ColorPlane pl, struct video_image *image, const byte Strength[MB_BLOCK_SIZE],Macroblock *MbQ, int dir, int edge, StorablePicture *p);
static void EdgeLoopLumaMBAff (ColorPlane pl, struct video_image *image, const byte Strength[MB_BLOCK_SIZE],Macroblock *MbQ, int dir, int edge, StorablePicture *p);
static void EdgeLoopChromaNormal(struct video_image *image, const byte Strength[MB_BLOCK_SIZE],Macroblock *MbQ, int dir, int edge, int uv, StorablePicture *p);
static void EdgeLoopChromaMBAff(struct video_image *image, const byte Strength[MB_BLOCK_SIZE],Macroblock *MbQ, int dir, int edge, int uv, StorablePicture *p);
static void DeblockMb(VideoParameters *p_Vid, StorablePicture *p, int MbQAddr);
static void EdgeLoopLumaMBAff_Horiz(ColorPlane pl, VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, StorablePicture *p);
static void EdgeLoopLumaMBAff_Vert(ColorPlane pl, VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, StorablePicture *p);

/*!
*****************************************************************************************
* \brief
*    Filter all macroblocks in order of increasing macroblock address.
*****************************************************************************************
*/
void DeblockPicture(VideoParameters *p_Vid, StorablePicture *p)
{
	unsigned i;

	if (!p->mb_aff_frame_flag && p_Vid->active_sps->chroma_format_idc==YUV420 && p_Vid->getNeighbour == getNonAffNeighbour && !p_Vid->mixedModeEdgeFlag)
	{
		for (i = 0; i < p->PicSizeInMbs; ++i)
		{
			Deblock_YUV420( p_Vid, p, i ) ;
		}
	}
	else if (p->mb_aff_frame_flag && p_Vid->active_sps->chroma_format_idc==YUV420 && p_Vid->getNeighbour == getAffNeighbour)
	{
		for (i = 0; i < p->PicSizeInMbs; ++i)
		{
			Deblock_YUV420_MBAFF( p_Vid, p, i ) ;
		}
	}
	else
	{
		if (p->mb_aff_frame_flag == 1) 
		{
			p_Vid->GetStrength    = GetStrengthMBAff;
			p_Vid->EdgeLoopLuma   = EdgeLoopLumaMBAff;
			p_Vid->EdgeLoopChroma = EdgeLoopChromaMBAff;
		}
		else
		{
			p_Vid->GetStrength    = GetStrengthNormal;
			p_Vid->EdgeLoopLuma   = EdgeLoopLumaNormal;
			p_Vid->EdgeLoopChroma = EdgeLoopChromaNormal;
		}

		for (i = 0; i < p->PicSizeInMbs; ++i)
		{
			DeblockMb( p_Vid, p, i ) ;
		}
	}
}


/*!
*****************************************************************************************
* \brief
*    Deblocking filter for one macroblock.
*****************************************************************************************
*/

static void DeblockMb(VideoParameters *p_Vid, StorablePicture *p, int MbQAddr)
{
	int           EdgeCondition;
	int           dir, edge;
	__declspec(align(32)) byte          Strength[16];
	short         mb_x, mb_y;

	int           filterNon8x8LumaEdgesFlag[4] = {1,1,1,1};
	int           filterLeftMbEdgeFlag;
	int           filterTopMbEdgeFlag;
	int           fieldModeMbFlag;
	int           mvlimit = 4;
	int           i, StrengthSum;
	Macroblock    *MbQ = &(p_Vid->mb_data[MbQAddr]) ; // current Mb
	VideoImage *imgY   = p->imgY;
	VideoImage *imgUV[2] = {p->imgUV[0], p->imgUV[1]};

	int           edge_cr;

	// return, if filter is disabled
	if (MbQ->DFDisableIdc==1) 
	{
		p_Vid->DeblockCall = 0;
		return;
	}
	p_Vid->DeblockCall = 1;
	get_mb_pos (p_Vid, MbQAddr, p_Vid->mb_size[IS_LUMA], &mb_x, &mb_y);

	filterLeftMbEdgeFlag = (mb_x != 0);
	filterTopMbEdgeFlag  = (mb_y != 0);

	if (MbQ->mb_type == I8MB)
		assert(MbQ->luma_transform_size_8x8_flag);

	filterNon8x8LumaEdgesFlag[1] =
		filterNon8x8LumaEdgesFlag[3] = !(MbQ->luma_transform_size_8x8_flag);

	if (p->mb_aff_frame_flag && mb_y == MB_BLOCK_SIZE && MbQ->mb_field)
		filterTopMbEdgeFlag = 0;

	fieldModeMbFlag = (p->structure!=FRAME) || (p->mb_aff_frame_flag && MbQ->mb_field);
	if (fieldModeMbFlag)
		mvlimit = 2;

	if (MbQ->DFDisableIdc==2)
	{
		// don't filter at slice boundaries
		filterLeftMbEdgeFlag = MbQ->mb_avail_left;
		// if this the bottom of a frame macroblock pair then always filter the top edge
		filterTopMbEdgeFlag  = (p->mb_aff_frame_flag && !MbQ->mb_field && (MbQAddr & 0x01)) ? 1 : MbQ->mb_avail_up;
	}

	CheckAvailabilityOfNeighbors(MbQ);

	for( dir = 0 ; dir < 2 ; ++dir )                                                      // filter first vertical edges, followed by horizontal 
	{
		EdgeCondition = (dir && filterTopMbEdgeFlag) || (!dir && filterLeftMbEdgeFlag); // can not filter beyond picture boundaries
		for( edge=0; edge<4 ; ++edge )                                            // first 4 vertical strips of 16 pel
		{                                                                               // then  4 horizontal
			if( edge || EdgeCondition )
			{
				edge_cr = chroma_edge[dir][edge][p->chroma_format_idc];

				p_Vid->GetStrength(Strength, MbQ, dir, edge << 2, mvlimit, p); // Strength for 4 blks in 1 stripe
				StrengthSum = Strength[0];
				for (i = 1; i < MB_BLOCK_SIZE && StrengthSum == 0 ; ++i)
				{
					StrengthSum += (int) Strength[i];
				}

				if( StrengthSum )                      // only if one of the 16 Strength bytes is != 0
				{
					if (filterNon8x8LumaEdgesFlag[edge])
					{
						p_Vid->EdgeLoopLuma( PLANE_Y, imgY, Strength, MbQ, dir, edge << 2, p) ;
						if( p_Vid->active_sps->chroma_format_idc==YUV444 && !IS_INDEPENDENT(p_Vid) )
						{
							p_Vid->EdgeLoopLuma(PLANE_U, imgUV[0], Strength, MbQ, dir, edge << 2, p);
							p_Vid->EdgeLoopLuma(PLANE_V, imgUV[1], Strength, MbQ, dir, edge << 2, p);
						}
					}
					if (p_Vid->active_sps->chroma_format_idc==YUV420 || p_Vid->active_sps->chroma_format_idc==YUV422)
					{
						if( (imgUV != NULL) && (edge_cr >= 0))
						{
							p_Vid->EdgeLoopChroma( imgUV[0], Strength, MbQ, dir, edge_cr, 0, p);
							p_Vid->EdgeLoopChroma( imgUV[1], Strength, MbQ, dir, edge_cr, 1, p);
						}
					}
				}

				if (dir && !edge && !MbQ->mb_field && p_Vid->mixedModeEdgeFlag) 
				{
					// this is the extra horizontal edge between a frame macroblock pair and a field above it
					p_Vid->DeblockCall = 2;
					p_Vid->GetStrength(Strength, MbQ, 1, MB_BLOCK_SIZE, mvlimit, p); // Strength for 4 blks in 1 stripe
					//if( *((int*)Strength) )                      // only if one of the 4 Strength bytes is != 0
					{            
						if (filterNon8x8LumaEdgesFlag[edge])
						{             
							p_Vid->EdgeLoopLuma(PLANE_Y, imgY, Strength, MbQ, dir, MB_BLOCK_SIZE, p) ;
							if( p_Vid->active_sps->chroma_format_idc==YUV444 && !IS_INDEPENDENT(p_Vid) )
							{
								p_Vid->EdgeLoopLuma(PLANE_U, imgUV[0], Strength, MbQ, dir, MB_BLOCK_SIZE, p) ;
								p_Vid->EdgeLoopLuma(PLANE_V, imgUV[1], Strength, MbQ, dir, MB_BLOCK_SIZE, p) ;
							}
						}
						if (p_Vid->active_sps->chroma_format_idc==YUV420 || p_Vid->active_sps->chroma_format_idc==YUV422) 
						{
							if( (imgUV != NULL) && (edge_cr >= 0))
							{
								p_Vid->EdgeLoopChroma( imgUV[0], Strength, MbQ, dir, MB_BLOCK_SIZE, 0, p) ;
								p_Vid->EdgeLoopChroma( imgUV[1], Strength, MbQ, dir, MB_BLOCK_SIZE, 1, p) ;
							}
						}
					}
					p_Vid->DeblockCall = 1;
				}
			}
		}//end edge
	}//end loop dir

	p_Vid->DeblockCall = 0;
}



static void Deblock_YUV420_MBAFF(VideoParameters *p_Vid, StorablePicture *p, int MbQAddr)
{
	int           EdgeCondition;
	int           edge;
	__declspec(align(32)) byte          Strength[16];
	short         mb_x, mb_y;

	int           filterNon8x8LumaEdgesFlag[4] = {1,1,1,1};
	int           filterLeftMbEdgeFlag;
	int           filterTopMbEdgeFlag;
	int           fieldModeMbFlag;
	int           mvlimit = 4;
	int           i, StrengthSum;
	Macroblock    *MbQ = &(p_Vid->mb_data[MbQAddr]) ; // current Mb
	VideoImage *imgY   = p->imgY;
	VideoImage *imgUV[2] = {p->imgUV[0], p->imgUV[1]};

	int           edge_cr;

	// return, if filter is disabled
	if (MbQ->DFDisableIdc==1) 
	{
		p_Vid->DeblockCall = 0;
		return;
	}
	p_Vid->DeblockCall = 1;
	get_mb_block_pos_mbaff(p_Vid->PicPos, MbQAddr, &mb_x, &mb_y);

	filterLeftMbEdgeFlag = (mb_x != 0);
	filterTopMbEdgeFlag  = (mb_y != 0);

	if (MbQ->mb_type == I8MB)
		assert(MbQ->luma_transform_size_8x8_flag);

	filterNon8x8LumaEdgesFlag[1] =
		filterNon8x8LumaEdgesFlag[3] = !(MbQ->luma_transform_size_8x8_flag);

	if (1 && mb_y == 1 && MbQ->mb_field)
		filterTopMbEdgeFlag = 0;

	fieldModeMbFlag = (p->structure!=FRAME) || MbQ->mb_field;
	if (fieldModeMbFlag)
		mvlimit = 2;

	if (MbQ->DFDisableIdc==2)
	{
		// don't filter at slice boundaries
		filterLeftMbEdgeFlag = MbQ->mb_avail_left;
		// if this the bottom of a frame macroblock pair then always filter the top edge
		filterTopMbEdgeFlag  = (1 && !MbQ->mb_field && (MbQAddr & 0x01)) ? 1 : MbQ->mb_avail_up;
	}

	CheckAvailabilityOfNeighbors(MbQ);


		EdgeCondition = filterLeftMbEdgeFlag; // can not filter beyond picture boundaries
		for( edge=0; edge<4 ; ++edge )                                            // first 4 vertical strips of 16 pel
		{                                                                               // then  4 horizontal
			if( edge || EdgeCondition )
			{
				edge_cr = chroma_edge[0][edge][YUV420];

				GetStrength_MBAff_Vert_YUV420(Strength, MbQ, edge << 2, mvlimit, p); // Strength for 4 blks in 1 stripe
				StrengthSum = Strength[0];
				for (i = 1; i < MB_BLOCK_SIZE && StrengthSum == 0 ; ++i)
				{
					StrengthSum += (int) Strength[i];
				}

				if( StrengthSum )                      // only if one of the 16 Strength bytes is != 0
				{
					if (filterNon8x8LumaEdgesFlag[edge])
					{
						EdgeLoopLumaMBAff_Vert_YUV420(imgY, Strength, MbQ, edge << 2, p) ;
					}
					if( (imgUV != NULL) && (edge_cr >= 0))
					{
						EdgeLoopChromaMBAff_Vert_YUV420( imgUV[0], Strength, MbQ, edge_cr, 0, p);
						EdgeLoopChromaMBAff_Vert_YUV420( imgUV[1], Strength, MbQ, edge_cr, 1, p);
					}
				}
			}
		}//end edge

			EdgeCondition = filterTopMbEdgeFlag; // can not filter beyond picture boundaries
		for( edge=0; edge<4 ; ++edge )                                            // first 4 vertical strips of 16 pel
		{                                                                               // then  4 horizontal
			if( edge || EdgeCondition )
			{
				edge_cr = chroma_edge[1][edge][YUV420];

				GetStrengthMBAff_Horiz_YUV420(Strength, MbQ, edge << 2, mvlimit, p); // Strength for 4 blks in 1 stripe
				StrengthSum = Strength[0];
				for (i = 1; i < MB_BLOCK_SIZE && StrengthSum == 0 ; ++i)
				{
					StrengthSum += (int) Strength[i];
				}

				if( StrengthSum )                      // only if one of the 16 Strength bytes is != 0
				{
					if (filterNon8x8LumaEdgesFlag[edge])
					{
						EdgeLoopLumaMBAff_Horiz( PLANE_Y, imgY, Strength, MbQ, edge << 2, p) ;
					}
					if( (imgUV != NULL) && (edge_cr >= 0))
					{
						EdgeLoopChromaMBAff( imgUV[0], Strength, MbQ, 1, edge_cr, 0, p);
						EdgeLoopChromaMBAff( imgUV[1], Strength, MbQ, 1, edge_cr, 1, p);
					}
				}
						if (!edge && !MbQ->mb_field && p_Vid->mixedModeEdgeFlag) 
				{
					// this is the extra horizontal edge between a frame macroblock pair and a field above it
					p_Vid->DeblockCall = 2;
					GetStrengthMBAff(Strength, MbQ, 1, MB_BLOCK_SIZE, mvlimit, p); // Strength for 4 blks in 1 stripe
					//if( *((int*)Strength) )                      // only if one of the 4 Strength bytes is != 0
					{            
						EdgeLoopLumaMBAff_Horiz(PLANE_Y, imgY, Strength, MbQ, MB_BLOCK_SIZE, p) ;

						EdgeLoopChromaMBAff( imgUV[0], Strength, MbQ, 1, MB_BLOCK_SIZE, 0, p) ;
						EdgeLoopChromaMBAff( imgUV[1], Strength, MbQ, 1, MB_BLOCK_SIZE, 1, p) ;

					}
					p_Vid->DeblockCall = 1;
				}
			}
		}//end edge
	

	p_Vid->DeblockCall = 0;
}



static void Deblock_YUV420(VideoParameters *p_Vid, StorablePicture *p, int MbQAddr)
{
	__declspec(align(32)) union 
	{
		uint32_t as32[4];
		uint8_t as8[16];
		uint8_t edge[4][4];
	} strength;
	uint8_t alphas[2], alphas_chroma[2][2];
	uint8_t betas[2], betas_chroma[2][2];
	__declspec(align(32)) union 
	{
		uint32_t as32[4];
		uint8_t as8[16];
	} thresholds;
	short         mb_x, mb_y;
	int           filterLeftMbEdgeFlag;
	int           filterTopMbEdgeFlag;
	int           mvlimit = 4;
	Macroblock    *MbQ = &(p_Vid->mb_data[MbQAddr]) ; // current Mb
	Macroblock *MbP=0;
	VideoImage *imgY   = p->imgY;
	imgpel *YQ, *UQ, *VQ;
	VideoImage *imgUV[2] = {p->imgUV[0], p->imgUV[1]};
	int QPQ = MbQ->qp;
	int indexAQ = iClip3(0, MAX_QP, QPQ + MbQ->DFAlphaC0Offset);
	int indexBQ = iClip3(0, MAX_QP, QPQ + MbQ->DFBetaOffset);
	const byte *ClipTabQ = CLIP_TAB[indexAQ], *ClipTabQ_Chroma[2], *ClipTabP_Chroma[2];

	// return, if filter is disabled
	if (MbQ->DFDisableIdc==1) 
	{
		p_Vid->DeblockCall = 0;
		return;
	}

	alphas[1]  = ALPHA_TABLE[indexAQ];
	betas[1]  = BETA_TABLE [indexBQ];

	indexAQ = iClip3(0, MAX_QP, MbQ->qpc[0] + MbQ->DFAlphaC0Offset);
	alphas_chroma[0][1]  = ALPHA_TABLE[indexAQ];
	ClipTabQ_Chroma[0] = CLIP_TAB   [indexAQ];
	indexAQ = iClip3(0, MAX_QP, MbQ->qpc[1] + MbQ->DFAlphaC0Offset);
	alphas_chroma[1][1]  = ALPHA_TABLE[indexAQ];
	ClipTabQ_Chroma[1] = CLIP_TAB   [indexAQ];

	indexBQ = iClip3(0, MAX_QP, MbQ->qpc[0] + MbQ->DFBetaOffset);
	betas_chroma[0][1]  = BETA_TABLE[indexBQ];
	indexBQ = iClip3(0, MAX_QP, MbQ->qpc[1] + MbQ->DFBetaOffset);
	betas_chroma[1][1]  = BETA_TABLE[indexBQ];

	p_Vid->DeblockCall = 1;
	get_mb_block_pos_normal(p_Vid->PicPos, MbQAddr, &mb_x, &mb_y);

	filterLeftMbEdgeFlag = (mb_x != 0);
	filterTopMbEdgeFlag  = (mb_y != 0);
	YQ = imgY->base_address + mb_y*16 * imgY->stride + mb_x*16;
	UQ = imgUV[0]->base_address + mb_y * 8 * imgUV[0]->stride + mb_x * 8;
	VQ = imgUV[1]->base_address + mb_y * 8 * imgUV[1]->stride + mb_x * 8;

	if (p->structure!=FRAME)
		mvlimit = 2;

	if (MbQ->DFDisableIdc==2)
	{
		// don't filter at slice boundaries
		filterLeftMbEdgeFlag = MbQ->mb_avail_left;
		// if this the bottom of a frame macroblock pair then always filter the top edge
		filterTopMbEdgeFlag  = MbQ->mb_avail_up;
	}

	//CheckAvailabilityOfNeighbors(MbQ);

#pragma region vertical
	if(filterLeftMbEdgeFlag)  // can not filter beyond picture boundaries
	{
		MbP = &(p_Vid->mb_data[MbQ->mb_addr_left]);
	}
	else
		MbP=0;

	GetStrength_Vert_YUV420_All(strength.edge, MbQ, mvlimit, p, mb_x*4, mb_y*4, MbP, MbQ->luma_transform_size_8x8_flag);

	{
		int i;
		if (MbP)
		{
			int QP_Chroma0 = (MbP->qpc[0] + MbQ->qpc[0] + 1) >> 1;
			int QP_Chroma1 = (MbP->qpc[1] + MbQ->qpc[1] + 1) >> 1;
			int QP = (MbP->qp + QPQ + 1) >> 1;

			int indexA = iClip3(0, MAX_QP, QP + MbQ->DFAlphaC0Offset);
			int indexB = iClip3(0, MAX_QP, QP + MbQ->DFBetaOffset);
			const byte *ClipTab = CLIP_TAB[indexA];

			alphas[0]  = ALPHA_TABLE[indexA];
			betas[0]  = BETA_TABLE [indexB];
			for (i=0;i<4;i++)
			{
				thresholds.as8[i] = ClipTab[strength.as8[i]];
			}

			indexA = iClip3(0, MAX_QP, QP_Chroma0 + MbQ->DFAlphaC0Offset);
			alphas_chroma[0][0] = ALPHA_TABLE[indexA];
			ClipTabP_Chroma[0] = CLIP_TAB[indexA];
			indexB = iClip3(0, MAX_QP, QP_Chroma0 + MbQ->DFBetaOffset);
			betas_chroma[0][0] =BETA_TABLE[indexB];

			indexA = iClip3(0, MAX_QP, QP_Chroma1 + MbQ->DFAlphaC0Offset);
			alphas_chroma[1][0] = ALPHA_TABLE[indexA];
			ClipTabP_Chroma[1] = CLIP_TAB[indexA];
			indexB = iClip3(0, MAX_QP, QP_Chroma1 + MbQ->DFBetaOffset);
			betas_chroma[1][0] = BETA_TABLE[indexB];
		}

		for (i=4;i<16;i++)
		{
			thresholds.as8[i] = ClipTabQ[strength.as8[i]];
		}

		ippiFilterDeblockingLuma_VerEdge_H264_8u_C1IR(YQ, imgY->stride, alphas, betas, thresholds.as8, strength.as8);

		if (MbP)
		{
			for (i=0;i<4;i++)
			{
				thresholds.as8[i] = ClipTabP_Chroma[0][strength.as8[i]];
			}
		}
		for (i=4;i<8;i++)
		{
			thresholds.as8[i] = ClipTabQ_Chroma[0][strength.as8[i+4]];
		}		
		ippiFilterDeblockingChroma_VerEdge_H264_8u_C1IR(UQ, imgUV[0]->stride, alphas_chroma[0], betas_chroma[0], thresholds.as8, strength.as8);

		if (MbP)
		{
			for (i=0;i<4;i++)
			{
				thresholds.as8[i] = ClipTabP_Chroma[1][strength.as8[i]];
			}
		}
		for (i=4;i<8;i++)
		{
			thresholds.as8[i] = ClipTabQ_Chroma[1][strength.as8[i+4]];
		}		
		ippiFilterDeblockingChroma_VerEdge_H264_8u_C1IR(VQ, imgUV[1]->stride, alphas_chroma[1], betas_chroma[1], thresholds.as8, strength.as8);

	}

#pragma endregion


#pragma region horizontal
	MbP = 0;
	/* ---- horizontal ---- */
	//	edge=0;
	if(filterTopMbEdgeFlag) // can not filter beyond picture boundaries
	{
		MbP = &(p_Vid->mb_data[MbQ->mb_addr_up]);
	}
	else
	{
		MbP = 0;
	}

	GetStrength_Horiz_YUV420_All(strength.edge, MbQ, mvlimit, p, mb_x*4, mb_y*4, MbP, MbQ->luma_transform_size_8x8_flag);

	{
		int i;

		if (MbP)
		{
			int QP_Chroma0 = (MbP->qpc[0] + MbQ->qpc[0] + 1) >> 1;
			int QP_Chroma1 = (MbP->qpc[1] + MbQ->qpc[1] + 1) >> 1;
			int QP = (MbP->qp + QPQ + 1) >> 1;

			int indexA = iClip3(0, MAX_QP, QP + MbQ->DFAlphaC0Offset);
			int indexB = iClip3(0, MAX_QP, QP + MbQ->DFBetaOffset);
			const byte *ClipTab = CLIP_TAB[indexA];

			alphas[0]  = ALPHA_TABLE[indexA];
			betas[0]  = BETA_TABLE [indexB];
			for (i=0;i<4;i++)
			{
				thresholds.as8[i] = ClipTab[strength.as8[i]];
			}

			indexA = iClip3(0, MAX_QP, QP_Chroma0 + MbQ->DFAlphaC0Offset);
			alphas_chroma[0][0] = ALPHA_TABLE[indexA];
			ClipTabP_Chroma[0] = CLIP_TAB[indexA];
			indexB = iClip3(0, MAX_QP, QP_Chroma0 + MbQ->DFBetaOffset);
			betas_chroma[0][0] =BETA_TABLE[indexB];

			indexA = iClip3(0, MAX_QP, QP_Chroma1 + MbQ->DFAlphaC0Offset);
			alphas_chroma[1][0] = ALPHA_TABLE[indexA];
			ClipTabP_Chroma[1] = CLIP_TAB[indexA];
			indexB = iClip3(0, MAX_QP, QP_Chroma1 + MbQ->DFBetaOffset);
			betas_chroma[1][0] = BETA_TABLE[indexB];
		}

		for (i=4;i<16;i++)
		{
			thresholds.as8[i] = ClipTabQ[strength.as8[i]];
		}

		ippiFilterDeblockingLuma_HorEdge_H264_8u_C1IR(YQ, imgY->stride, alphas, betas, thresholds.as8, strength.as8);

		if (MbP)
		{
			for (i=0;i<4;i++)
			{
				thresholds.as8[i] = ClipTabP_Chroma[0][strength.as8[i]];
			}
		}
		for (i=4;i<8;i++)
		{
			thresholds.as8[i] = ClipTabQ_Chroma[0][strength.as8[i+4]];
		}		
		ippiFilterDeblockingChroma_HorEdge_H264_8u_C1IR(UQ, imgUV[0]->stride, alphas_chroma[0], betas_chroma[0], thresholds.as8, strength.as8);

		if (MbP)
		{
			for (i=0;i<4;i++)
			{
				thresholds.as8[i] = ClipTabP_Chroma[1][strength.as8[i]];
			}
		}
		for (i=4;i<8;i++)
		{
			thresholds.as8[i] = ClipTabQ_Chroma[1][strength.as8[i+4]];
		}		
		ippiFilterDeblockingChroma_HorEdge_H264_8u_C1IR(VQ, imgUV[1]->stride, alphas_chroma[1], betas_chroma[1], thresholds.as8, strength.as8);

	}
#pragma endregion

	p_Vid->DeblockCall = 0;
}


#define ANY_INTRA (MbP->mb_type==I4MB||MbP->mb_type==I8MB||MbP->mb_type==I16MB||MbP->mb_type==IPCM||MbQ->mb_type==I4MB||MbQ->mb_type==I8MB||MbQ->mb_type==I16MB||MbQ->mb_type==IPCM)

/*!
*********************************************************************************************
* \brief
*    returns a buffer of 16 Strength values for one stripe in a mb (for different Frame or Field types)
*********************************************************************************************
*/
void GetStrengthNormal_Horiz(byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p);
void GetStrengthNormal_Vert(byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p);

static void GetStrengthNormal(byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int dir, int edge, int mvlimit, StorablePicture *p)
{
	if (dir == 0)
		GetStrengthNormal_Vert(Strength, MbQ, edge, mvlimit, p);
	else
		GetStrengthNormal_Horiz(Strength, MbQ, edge, mvlimit, p);
}

/*!
*********************************************************************************************
* \brief
*    returns a buffer of 16 Strength values for one stripe in a mb (for MBAFF)
*********************************************************************************************
*/
static void GetStrengthMBAff_Horiz(byte Strength[16], Macroblock *MbQ, int dir, int edge, int mvlimit, StorablePicture *p)
{
	// dir == 1
	short  blkP, blkQ, idx;
	short  blk_x, blk_x2, blk_y, blk_y2 ;
	h264_ref_t  ref_p0,ref_p1,ref_q0,ref_q1;
	int    xQ, yQ;
	short  mb_x, mb_y;
	Macroblock *MbP;

	PixelPos pixP;
	int dir_m1 = 0;

	PicMotionParams *motion = &p->motion;
	PicMotion **motion0 = motion->motion[LIST_0];
	PicMotion **motion1 = motion->motion[LIST_1];
	yQ = (edge < MB_BLOCK_SIZE ? edge : 1);

	for( idx = 0; idx < 16; ++idx )
	{
		VideoParameters *p_Vid = MbQ->p_Vid;
		xQ = idx;

		p_Vid->getNeighbourLuma(MbQ, xQ , yQ - 1, &pixP);
		blkQ = (short) ((yQ & 0xFFFC) + (xQ >> 2));
		blkP = (short) ((pixP.y & 0xFFFC) + (pixP.x >> 2));

		MbP = &(p_Vid->mb_data[pixP.mb_addr]);
		p_Vid->mixedModeEdgeFlag = (byte) (MbQ->mb_field != MbP->mb_field);   

		if ((p->slice_type==SP_SLICE)||(p->slice_type==SI_SLICE) )
		{
			Strength[idx] = (edge == 0 && (((!p->mb_aff_frame_flag && (p->structure==FRAME)) ||
				(p->mb_aff_frame_flag && !MbP->mb_field && !MbQ->mb_field)))) ? 4 : 3;
		}
		else
		{
			// Start with Strength=3. or Strength=4 for Mb-edge
			Strength[idx] = (edge == 0 && (((!p->mb_aff_frame_flag && (p->structure==FRAME)) ||
				(p->mb_aff_frame_flag && !MbP->mb_field && !MbQ->mb_field)))) ? 4 : 3;

			if(  !(MbP->mb_type==I4MB || MbP->mb_type==I16MB || MbP->mb_type==I8MB || MbP->mb_type==IPCM)
				&& !(MbQ->mb_type==I4MB || MbQ->mb_type==I16MB || MbQ->mb_type==I8MB || MbQ->mb_type==IPCM) )
			{
				if( ((MbQ->cbp_blk[0] &  ((int64)1 << blkQ )) != 0) || ((MbP->cbp_blk[0] &  ((int64)1 << blkP)) != 0) )
					Strength[idx] = 2 ;
				else
				{
					// if no coefs, but vector difference >= 1 set Strength=1
					// if this is a mixed mode edge then one set of reference pictures will be frame and the
					// other will be field
					if (p_Vid->mixedModeEdgeFlag)
					{
						(Strength[idx] = 1);
					}
					else
					{
						p_Vid->get_mb_block_pos (p_Vid->PicPos, MbQ->mbAddrX, &mb_x, &mb_y);
						blk_y  = (short) ((mb_y<<2) + (blkQ >> 2));
						blk_x  = (short) ((mb_x<<2) + (blkQ  & 3));
						blk_y2 = (short) (pixP.pos_y >> 2);
						blk_x2 = (short) (pixP.pos_x >> 2);
						{
							PicMotion *motion_p0, *motion_q0, *motion_p1, *motion_q1;
							motion_p0=&motion0[blk_y ][blk_x ];
							motion_q0=&motion0[blk_y2][blk_x2];
							motion_p1=&motion1[blk_y ][blk_x ];
							motion_q1=&motion1[blk_y2][blk_x2];

							ref_p0 = motion_p0->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_p0->ref_pic_id;
							ref_q0 = motion_q0->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_q0->ref_pic_id;
							ref_p1 = motion_p1->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_p1->ref_pic_id;
							ref_q1 = motion_q1->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_q1->ref_pic_id;
							if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) ||
								((ref_p0==ref_q1) && (ref_p1==ref_q0)))
							{
								Strength[idx]=0;
								// L0 and L1 reference pictures of p0 are different; q0 as well
								if (ref_p0 != ref_p1)
								{
									// compare MV for the same reference picture
									if (ref_p0==ref_q0)
									{
										Strength[idx] =  (byte) (
											(abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
											(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit) ||
											(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
											(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit));
									}
									else
									{
										Strength[idx] =  (byte) (
											(abs( motion_p0->mv[0] - motion_q1->mv[0]) >= 4) ||
											(abs( motion_p0->mv[1] - motion_q1->mv[1]) >= mvlimit) ||
											(abs( motion_p1->mv[0] - motion_q0->mv[0]) >= 4) ||
											(abs( motion_p1->mv[1] - motion_q0->mv[1]) >= mvlimit));
									}
								}
								else
								{ // L0 and L1 reference pictures of p0 are the same; q0 as well

									Strength[idx] = (byte) (
										((abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
										(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit ) ||
										(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
										(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit))
										&&
										((abs( motion_p0->mv[0] - motion_q1->mv[0]) >= 4) ||
										(abs( motion_p0->mv[1] - motion_q1->mv[1]) >= mvlimit) ||
										(abs( motion_p1->mv[0] - motion_q0->mv[0]) >= 4) ||
										(abs( motion_p1->mv[1] - motion_q0->mv[1]) >= mvlimit)));
								}
							}
							else
							{
								Strength[idx] = 1;
							}
						}
					}
				}
			}
		}
	}
}

static void GetStrengthMBAff_Vert(byte Strength[16], Macroblock *MbQ, int dir, int edge, int mvlimit, StorablePicture *p)
{
	// dir == 0
	short  blkP, blkQ, idx;
	short  blk_x, blk_x2, blk_y, blk_y2 ;
	h264_ref_t  ref_p0,ref_p1,ref_q0,ref_q1;
	int    xQ, yQ;
	short  mb_x, mb_y;
	Macroblock *MbP;

	PixelPos pixP;
	int dir_m1 = 1;

	PicMotionParams *motion = &p->motion;
	PicMotion **motion0 = motion->motion[LIST_0];
	PicMotion **motion1 = motion->motion[LIST_1];
	xQ = edge;
	for( idx = 0; idx < 16; ++idx )
	{
		VideoParameters *p_Vid = MbQ->p_Vid;

		yQ = idx;
		p_Vid->getNeighbourLuma(MbQ, xQ - 1, yQ, &pixP);
		blkQ = (short) ((yQ & 0xFFFC) + (xQ >> 2));
		blkP = (short) ((pixP.y & 0xFFFC) + (pixP.x >> 2));

		MbP = &(p_Vid->mb_data[pixP.mb_addr]);
		p_Vid->mixedModeEdgeFlag = (byte) (MbQ->mb_field != MbP->mb_field);   

		if ((p->slice_type==SP_SLICE)||(p->slice_type==SI_SLICE) )
		{
			Strength[idx] = (edge == 0 && (((!p->mb_aff_frame_flag && (p->structure==FRAME)) ||
				(p->mb_aff_frame_flag && !MbP->mb_field && !MbQ->mb_field)) ||
				((p->mb_aff_frame_flag || (p->structure != FRAME))))) ? 4 : 3;
		}
		else
		{
			// Start with Strength=3. or Strength=4 for Mb-edge
			Strength[idx] = (edge == 0 && (((!p->mb_aff_frame_flag && (p->structure==FRAME)) ||
				(p->mb_aff_frame_flag && !MbP->mb_field && !MbQ->mb_field)) ||
				((p->mb_aff_frame_flag || (p->structure!=FRAME))))) ? 4 : 3;

			if(  !(MbP->mb_type==I4MB || MbP->mb_type==I16MB || MbP->mb_type==I8MB || MbP->mb_type==IPCM)
				&& !(MbQ->mb_type==I4MB || MbQ->mb_type==I16MB || MbQ->mb_type==I8MB || MbQ->mb_type==IPCM) )
			{
				if( ((MbQ->cbp_blk[0] &  ((int64)1 << blkQ )) != 0) || ((MbP->cbp_blk[0] &  ((int64)1 << blkP)) != 0) )
					Strength[idx] = 2 ;
				else
				{
					// if no coefs, but vector difference >= 1 set Strength=1
					// if this is a mixed mode edge then one set of reference pictures will be frame and the
					// other will be field
					if (p_Vid->mixedModeEdgeFlag)
					{
						(Strength[idx] = 1);
					}
					else
					{
						p_Vid->get_mb_block_pos (p_Vid->PicPos, MbQ->mbAddrX, &mb_x, &mb_y);
						blk_y  = (short) ((mb_y<<2) + (blkQ >> 2));
						blk_x  = (short) ((mb_x<<2) + (blkQ  & 3));
						blk_y2 = (short) (pixP.pos_y >> 2);
						blk_x2 = (short) (pixP.pos_x >> 2);
						{
							PicMotion *motion_p0, *motion_q0, *motion_p1, *motion_q1;
							motion_p0=&motion0[blk_y ][blk_x ];
							motion_q0=&motion0[blk_y2][blk_x2];
							motion_p1=&motion1[blk_y ][blk_x ];
							motion_q1=&motion1[blk_y2][blk_x2];

							ref_p0 = motion_p0->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_p0->ref_pic_id;
							ref_q0 = motion_q0->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_q0->ref_pic_id;
							ref_p1 = motion_p1->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_p1->ref_pic_id;
							ref_q1 = motion_q1->ref_idx < 0 ? UNDEFINED_REFERENCE : motion_q1->ref_pic_id;

							if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) ||
								((ref_p0==ref_q1) && (ref_p1==ref_q0)))
							{
								Strength[idx]=0;
								// L0 and L1 reference pictures of p0 are different; q0 as well
								if (ref_p0 != ref_p1)
								{
									// compare MV for the same reference picture
									if (ref_p0==ref_q0)
									{
										Strength[idx] =  (byte) (
											(abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
											(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit) ||
											(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
											(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit));
									}
									else
									{
										Strength[idx] =  (byte) (
											(abs( motion_p0->mv[0] - motion_q1->mv[0]) >= 4) ||
											(abs( motion_p0->mv[1] - motion_q1->mv[1]) >= mvlimit) ||
											(abs( motion_p1->mv[0] - motion_q0->mv[0]) >= 4) ||
											(abs( motion_p1->mv[1] - motion_q0->mv[1]) >= mvlimit));
									}
								}
								else
								{ // L0 and L1 reference pictures of p0 are the same; q0 as well

									Strength[idx] = (byte) (
										((abs( motion_p0->mv[0] - motion_q0->mv[0]) >= 4) ||
										(abs( motion_p0->mv[1] - motion_q0->mv[1]) >= mvlimit ) ||
										(abs( motion_p1->mv[0] - motion_q1->mv[0]) >= 4) ||
										(abs( motion_p1->mv[1] - motion_q1->mv[1]) >= mvlimit))
										&&
										((abs( motion_p0->mv[0] - motion_q1->mv[0]) >= 4) ||
										(abs( motion_p0->mv[1] - motion_q1->mv[1]) >= mvlimit) ||
										(abs( motion_p1->mv[0] - motion_q0->mv[0]) >= 4) ||
										(abs( motion_p1->mv[1] - motion_q0->mv[1]) >= mvlimit)));
								}
							}
							else
							{
								Strength[idx] = 1;
							}
						}
					}
				}
			}
		}
	}
}

static void GetStrengthMBAff(byte Strength[16], Macroblock *MbQ, int dir, int edge, int mvlimit, StorablePicture *p)
{
	if (dir == 0)
		GetStrengthMBAff_Vert(Strength, MbQ, dir, edge, mvlimit, p);
	else
		GetStrengthMBAff_Horiz(Strength, MbQ, dir, edge, mvlimit, p);
}

/*!
*****************************************************************************************
* \brief
*    Filters 16 pel block edge of Frame or Field coded MBs 
*****************************************************************************************
*/


static void EdgeLoopLumaNormal(ColorPlane pl, VideoImage *image, const byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, 
															 int dir, int edge, StorablePicture *p)
{
	if (dir == 0)
		EdgeLoopLumaNormal_Vert(pl, image, Strength, MbQ, edge, p);
	else if (sse2_flag)
		EdgeLoopLumaNormal_Horiz_sse2(pl, image, Strength, MbQ, edge, p);
	else
		EdgeLoopLumaNormal_Horiz(pl, image, Strength, MbQ, edge, p);
}

/*!
*****************************************************************************************
* \brief
*    Filters 16 pel block edge of Super MB Frame coded MBs
*****************************************************************************************
*/
static void EdgeLoopLumaMBAff_Horiz(ColorPlane pl, VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, StorablePicture *p)
{
	// dir == 1
	imgpel **Img = image->img;
	int      width = image->stride;
	int      pel, ap = 0, aq = 0, Strng ;
	int      incP, incQ;
	int      C0, tc0, dif;
	imgpel   L0, R0;
	int      Alpha = 0, Beta = 0 ;
	const byte* ClipTab = NULL;
	int      small_gap;
	int      indexA, indexB;
	int      PelNum = pl? pelnum_cr[1][p->chroma_format_idc] : MB_BLOCK_SIZE;

	int      QP;
	int      xQ, yQ;

	PixelPos pixP, pixQ;
	VideoParameters *p_Vid = MbQ->p_Vid;
	int      bitdepth_scale = pl? p_Vid->bitdepth_scale[IS_CHROMA] : p_Vid->bitdepth_scale[IS_LUMA];
	int      max_imgpel_value = p_Vid->max_pel_value_comp[pl];

	int AlphaC0Offset = MbQ->DFAlphaC0Offset;
	int BetaOffset = MbQ->DFBetaOffset;
	byte fieldModeFilteringFlag;

	Macroblock *MbP;
	imgpel   *SrcPtrP, *SrcPtrQ;

	for( pel = 0 ; pel < PelNum ; ++pel )
	{
		xQ = pel ;
		yQ = (edge < 16 ? edge : 1) ;
		p_Vid->getNeighbourLuma(MbQ, xQ, yQ - 1, &pixP);     

		if (pixP.available || (MbQ->DFDisableIdc== 0))
		{
			if( (Strng = Strength[pel]) != 0)
			{
				p_Vid->getNeighbourLuma(MbQ, xQ, yQ, &pixQ);

				MbP = &(p_Vid->mb_data[pixP.mb_addr]);
				fieldModeFilteringFlag = (byte) (MbQ->mb_field || MbP->mb_field);

				incQ    = ((fieldModeFilteringFlag && !MbQ->mb_field) ? 2 * width : width);
				incP    = ((fieldModeFilteringFlag && !MbP->mb_field) ? 2 * width : width);
				SrcPtrQ = &(Img[pixQ.pos_y][pixQ.pos_x]);
				SrcPtrP = &(Img[pixP.pos_y][pixP.pos_x]);

				// Average QP of the two blocks
				QP = pl? ((MbP->qpc[pl-1] + MbQ->qpc[pl-1] + 1) >> 1) : (MbP->qp + MbQ->qp + 1) >> 1;

				indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
				indexB = iClip3(0, MAX_QP, QP + BetaOffset);

				Alpha   = ALPHA_TABLE[indexA] * bitdepth_scale;
				Beta    = BETA_TABLE [indexB] * bitdepth_scale;
				ClipTab = CLIP_TAB[indexA];


				L0  = SrcPtrP[0] ;
				R0  = SrcPtrQ[0] ;      



				if( abs( R0 - L0 ) < Alpha )
				{          
					imgpel L1  = SrcPtrP[-incP];
					imgpel R1  = SrcPtrQ[ incQ];      
					if ((abs( R0 - R1) < Beta )   && (abs(L0 - L1) < Beta ))
					{
						imgpel L2  = SrcPtrP[-incP*2];
						imgpel R2  = SrcPtrQ[ incQ*2];
						if(Strng == 4 )    // INTRA strong filtering
						{
							int RL0 = L0 + R0;
							small_gap = (abs( R0 - L0 ) < ((Alpha >> 2) + 2));
							aq  = ( abs( R0 - R2) < Beta ) & small_gap;               
							ap  = ( abs( L0 - L2) < Beta ) & small_gap;

							if (ap)
							{
								imgpel L3  = SrcPtrP[-incP*3];
								SrcPtrP[-incP * 2] = (imgpel) ((((L3 + L2) << 1) + L2 + L1 + RL0 + 4) >> 3);
								SrcPtrP[-incP    ] = (imgpel) (( L2 + L1 + L0 + R0 + 2) >> 2);
								SrcPtrP[    0    ] = (imgpel) (( R1 + ((L1 + RL0) << 1) +  L2 + 4) >> 3);
							}
							else
							{
								SrcPtrP[     0     ] = (imgpel) (((L1 << 1) + L0 + R1 + 2) >> 2) ;
							}

							if (aq)
							{
								imgpel R3  = SrcPtrQ[ incQ*3];
								SrcPtrQ[    0     ] = (imgpel) (( L1 + ((R1 + RL0) << 1) +  R2 + 4) >> 3);
								SrcPtrQ[ incQ     ] = (imgpel) (( R2 + R0 + R1 + L0 + 2) >> 2);
								SrcPtrQ[ incQ * 2 ] = (imgpel) ((((R3 + R2) << 1) + R2 + R1 + RL0 + 4) >> 3);
							}
							else
							{
								SrcPtrQ[    0     ] = (imgpel) (((R1 << 1) + R0 + L1 + 2) >> 2);
							}
						}
						else   // normal filtering
						{              
							int RL0 = (L0 + R0 + 1) >> 1;
							aq  = (abs( R0 - R2) < Beta);
							ap  = (abs( L0 - L2) < Beta);

							C0  = ClipTab[ Strng ] * bitdepth_scale;
							tc0  = (C0 + ap + aq) ;
							dif = iClip3( -tc0, tc0, (((R0 - L0) << 2) + (L1 - R1) + 4) >> 3) ;

							if( ap )
								*(SrcPtrP - incP) += iClip3( -C0,  C0, ( L2 + RL0 - (L1 << 1)) >> 1 ) ;

							*SrcPtrP  = (imgpel) iClip1 (max_imgpel_value, L0 + dif) ;
							*SrcPtrQ  = (imgpel) iClip1 (max_imgpel_value, R0 - dif) ;

							if( aq  )
								*(SrcPtrQ + incQ) += iClip3( -C0,  C0, ( R2 + RL0 - (R1 << 1)) >> 1 ) ;
						}            
					}
				}
			}
		}
	}
}

static void EdgeLoopLumaMBAff_Vert(ColorPlane pl, VideoImage *image, const byte Strength[16], Macroblock *MbQ, int edge, StorablePicture *p)
{
	// dir == 0
	imgpel **Img = image->img;
	int      width = image->stride;
	int      pel, ap = 0, aq = 0, Strng ;

	int      C0, tc0, dif;
	imgpel   L0, R0;
	int      Alpha = 0, Beta = 0 ;
	const byte* ClipTab = NULL;
	int      small_gap;
	int      indexA, indexB;
	int      PelNum = pl? pelnum_cr[0][p->chroma_format_idc] : MB_BLOCK_SIZE;

	int      QP;
	int      xQ, yQ;

	PixelPos pixP, pixQ;
	VideoParameters *p_Vid = MbQ->p_Vid;
	int      bitdepth_scale = pl? p_Vid->bitdepth_scale[IS_CHROMA] : p_Vid->bitdepth_scale[IS_LUMA];
	int      max_imgpel_value = p_Vid->max_pel_value_comp[pl];

	int AlphaC0Offset = MbQ->DFAlphaC0Offset;
	int BetaOffset = MbQ->DFBetaOffset;
	byte fieldModeFilteringFlag;

	Macroblock *MbP;
	imgpel   *SrcPtrP, *SrcPtrQ;

	for( pel = 0 ; pel < PelNum ; ++pel )
	{
		xQ = edge;
		yQ = pel;
		p_Vid->getNeighbourXPLuma(MbQ, xQ - 1, yQ, &pixP);     

		if (pixP.available || (MbQ->DFDisableIdc== 0))
		{
			if( (Strng = Strength[pel]) != 0)
			{
				p_Vid->getNeighbourLuma(MbQ, xQ, yQ, &pixQ);

				MbP = &(p_Vid->mb_data[pixP.mb_addr]);
				fieldModeFilteringFlag = (byte) (MbQ->mb_field || MbP->mb_field);

				SrcPtrQ = &(Img[pixQ.pos_y][pixQ.pos_x]);
				SrcPtrP = &(Img[pixP.pos_y][pixP.pos_x]);

				// Average QP of the two blocks
				QP = pl? ((MbP->qpc[pl-1] + MbQ->qpc[pl-1] + 1) >> 1) : (MbP->qp + MbQ->qp + 1) >> 1;

				indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
				indexB = iClip3(0, MAX_QP, QP + BetaOffset);

				Alpha   = ALPHA_TABLE[indexA] * bitdepth_scale;
				Beta    = BETA_TABLE [indexB] * bitdepth_scale;
				ClipTab = CLIP_TAB[indexA];


				L0  = SrcPtrP[0] ;
				R0  = SrcPtrQ[0] ;      



				if( abs( R0 - L0 ) < Alpha )
				{          
					imgpel L1  = SrcPtrP[-1];
					imgpel R1  = SrcPtrQ[ 1];      
					if ((abs( R0 - R1) < Beta )   && (abs(L0 - L1) < Beta ))
					{
						imgpel L2  = SrcPtrP[-2];
						imgpel R2  = SrcPtrQ[ 2];
						if(Strng == 4 )    // INTRA strong filtering
						{
							int RL0 = L0 + R0;
							small_gap = (abs( R0 - L0 ) < ((Alpha >> 2) + 2));
							aq  = ( abs( R0 - R2) < Beta ) & small_gap;               
							ap  = ( abs( L0 - L2) < Beta ) & small_gap;

							if (ap)
							{
								imgpel L3  = SrcPtrP[-3];
								SrcPtrP[-2] = (imgpel) ((((L3 + L2) << 1) + L2 + L1 + RL0 + 4) >> 3);
								SrcPtrP[-1    ] = (imgpel) (( L2 + L1 + L0 + R0 + 2) >> 2);
								SrcPtrP[    0    ] = (imgpel) (( R1 + ((L1 + RL0) << 1) +  L2 + 4) >> 3);
							}
							else
							{
								SrcPtrP[     0     ] = (imgpel) (((L1 << 1) + L0 + R1 + 2) >> 2) ;
							}

							if (aq)
							{
								imgpel R3  = SrcPtrQ[ 3];
								SrcPtrQ[    0     ] = (imgpel) (( L1 + ((R1 + RL0) << 1) +  R2 + 4) >> 3);
								SrcPtrQ[ 1     ] = (imgpel) (( R2 + R0 + R1 + L0 + 2) >> 2);
								SrcPtrQ[  2 ] = (imgpel) ((((R3 + R2) << 1) + R2 + R1 + RL0 + 4) >> 3);
							}
							else
							{
								SrcPtrQ[    0     ] = (imgpel) (((R1 << 1) + R0 + L1 + 2) >> 2);
							}
						}
						else   // normal filtering
						{              
							int RL0 = (L0 + R0 + 1) >> 1;
							aq  = (abs( R0 - R2) < Beta);
							ap  = (abs( L0 - L2) < Beta);

							C0  = ClipTab[ Strng ] * bitdepth_scale;
							tc0  = (C0 + ap + aq) ;
							dif = iClip3( -tc0, tc0, (((R0 - L0) << 2) + (L1 - R1) + 4) >> 3) ;

							if( ap )
								*(SrcPtrP - 1) += iClip3( -C0,  C0, ( L2 + RL0 - (L1 << 1)) >> 1 ) ;

							*SrcPtrP  = (imgpel) iClip1 (max_imgpel_value, L0 + dif) ;
							*SrcPtrQ  = (imgpel) iClip1 (max_imgpel_value, R0 - dif) ;

							if( aq  )
								*(SrcPtrQ + 1) += iClip3( -C0,  C0, ( R2 + RL0 - (R1 << 1)) >> 1 ) ;
						}            
					}
				}
			}
		}
	}
}

static void EdgeLoopLumaMBAff(ColorPlane pl, VideoImage *image, const byte Strength[16], Macroblock *MbQ, int dir, int edge, StorablePicture *p)
{
	if (dir == 0)
		EdgeLoopLumaMBAff_Vert(pl, image, Strength, MbQ, edge, p);
	else
		EdgeLoopLumaMBAff_Horiz(pl, image, Strength, MbQ, edge, p);
}

/*!
*****************************************************************************************
* \brief
*    Filters chroma block edge for Frame or Field coded pictures
*****************************************************************************************
*/


static void EdgeLoopChromaNormal(VideoImage *image, const byte Strength[16], Macroblock *MbQ, int dir, int edge, int uv, StorablePicture *p)
{ 
	if (dir == 0)
		EdgeLoopChromaNormal_Vert(image, Strength, MbQ, edge, uv, p);
	else 
		EdgeLoopChromaNormal_Horiz(image, Strength, MbQ, edge, uv, p);

}
/*!
*****************************************************************************************
* \brief
*    Filters chroma block edge for MBAFF types
*****************************************************************************************
*/
static void EdgeLoopChromaMBAff(VideoImage *image, const byte Strength[16], Macroblock *MbQ, int dir, int edge, int uv, StorablePicture *p)
{
	imgpel** Img = image->img;

	int      pel, Strng ;
	int      incP, incQ;
	int      C0, tc0, dif;
	imgpel   L0, R0;
	int      Alpha = 0, Beta = 0;
	const byte* ClipTab = NULL;
	int      indexA, indexB;
	VideoParameters *p_Vid = MbQ->p_Vid;
	int      PelNum = pelnum_cr[dir][p->chroma_format_idc];
	int      StrengthIdx;
	int      QP;
	int      xQ, yQ;
	PixelPos pixP, pixQ;
	int      dir_m1 = 1 - dir;
	int      bitdepth_scale = p_Vid->bitdepth_scale[IS_CHROMA];
	int      max_imgpel_value = p_Vid->max_pel_value_comp[uv + 1];

	int      AlphaC0Offset = MbQ->DFAlphaC0Offset;
	int      BetaOffset    = MbQ->DFBetaOffset;
	byte fieldModeFilteringFlag;
	Macroblock *MbP;
	imgpel   *SrcPtrP, *SrcPtrQ;
	int      width = image->stride;

	for( pel = 0 ; pel < PelNum ; ++pel )
	{
		xQ = dir ? pel : edge;
		yQ = dir ? (edge < 16? edge : 1) : pel;
		p_Vid->getNeighbour(MbQ, xQ, yQ, p_Vid->mb_size[IS_CHROMA], &pixQ);
		p_Vid->getNeighbour(MbQ, xQ - (dir_m1), yQ - dir, p_Vid->mb_size[IS_CHROMA], &pixP);    
		MbP = &(p_Vid->mb_data[pixP.mb_addr]);    
		StrengthIdx = (PelNum == 8) ? ((MbQ->mb_field && !MbP->mb_field) ? pel << 1 :((pel >> 1) << 2) + (pel & 0x01)) : pel;

		if (pixP.available || (MbQ->DFDisableIdc == 0))
		{
			if( (Strng = Strength[StrengthIdx]) != 0)
			{
				fieldModeFilteringFlag = (byte) (MbQ->mb_field || MbP->mb_field);
				incQ = dir ? ((fieldModeFilteringFlag && !MbQ->mb_field) ? 2 * width : width) : 1;
				incP = dir ? ((fieldModeFilteringFlag && !MbP->mb_field) ? 2 * width : width) : 1;
				SrcPtrQ = &(Img[pixQ.pos_y][pixQ.pos_x]);
				SrcPtrP = &(Img[pixP.pos_y][pixP.pos_x]);

				// Average QP of the two blocks
				QP = (MbP->qpc[uv] + MbQ->qpc[uv] + 1) >> 1;

				indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
				indexB = iClip3(0, MAX_QP, QP + BetaOffset);

				Alpha   = ALPHA_TABLE[indexA] * bitdepth_scale;
				Beta    = BETA_TABLE [indexB] * bitdepth_scale;
				ClipTab = CLIP_TAB[indexA];


				L0  = SrcPtrP[0] ;
				R0  = SrcPtrQ[0] ;      


				if( abs( R0 - L0 ) < Alpha )
				{          
					imgpel L1  = SrcPtrP[-incP];
					imgpel R1  = SrcPtrQ[ incQ];      
					//if( ((abs( R0 - R1) - Beta )  & (abs(L0 - L1) - Beta )) < 0  )
					if( ((abs( R0 - R1) - Beta < 0)  && (abs(L0 - L1) - Beta < 0 ))  )
					{
						if( Strng == 4 )    // INTRA strong filtering
						{
							SrcPtrQ[0] = (imgpel) ( ((R1 << 1) + R0 + L1 + 2) >> 2 );
							SrcPtrP[0] = (imgpel) ( ((L1 << 1) + L0 + R1 + 2) >> 2 );
						}
						else
						{
							C0  = ClipTab[ Strng ] * bitdepth_scale;
							tc0  = (C0 + 1);
							dif = iClip3( -tc0, tc0, ( ((R0 - L0) << 2) + (L1 - R1) + 4) >> 3 );

							SrcPtrP[0] = (imgpel) iClip1 ( max_imgpel_value, L0 + dif );
							SrcPtrQ[0] = (imgpel) iClip1 ( max_imgpel_value, R0 - dif );
						}
					}
				}
			}
		}
	}
}
