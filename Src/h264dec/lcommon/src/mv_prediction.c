/*!
 *************************************************************************************
 * \file mv_prediction.c
 *
 * \brief
 *    Motion Vector Prediction Functions
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *      - Karsten Sühring          <suehring@hhi.de>
 *************************************************************************************
 */

#include "global.h"
#include "mbuffer.h"
/*!
 ************************************************************************
 * \brief
 *    Get motion vector predictor
 ************************************************************************
 */
static void GetMotionVectorPredictorMBAFF (Macroblock *currMB, 
                                    PixelPos *block,        // <--> block neighbors
                                    short  pmv[2],
                                    short  ref_frame,
                                    PicMotion **motion, 
                                    int    mb_x,
                                    int    mb_y,
                                    int    blockshape_x,
                                    int    blockshape_y)
{
  int mv_a, mv_b, mv_c, pred_vec=0;
  int mvPredType, rFrameL, rFrameU, rFrameUR;
  int hv;
  VideoParameters *p_Vid = currMB->p_Vid;

  mvPredType = MVPRED_MEDIAN;


  if (currMB->mb_field)
	{
		rFrameL  = block[0].available
			? (p_Vid->mb_data[block[0].mb_addr].mb_field
			? motion[block[0].pos_y][block[0].pos_x].ref_idx
			: motion[block[0].pos_y][block[0].pos_x].ref_idx * 2) : -1;
		rFrameU  = block[1].available
			? (p_Vid->mb_data[block[1].mb_addr].mb_field
			? motion[block[1].pos_y][block[1].pos_x].ref_idx
			: motion[block[1].pos_y][block[1].pos_x].ref_idx * 2) : -1;
		rFrameUR = block[2].available
			? (p_Vid->mb_data[block[2].mb_addr].mb_field
			? motion[block[2].pos_y][block[2].pos_x].ref_idx
			: motion[block[2].pos_y][block[2].pos_x].ref_idx * 2) : -1;
	}
	else
  {
    rFrameL = block[0].available
      ? (p_Vid->mb_data[block[0].mb_addr].mb_field
      ? motion[block[0].pos_y][block[0].pos_x].ref_idx >>1
      : motion[block[0].pos_y][block[0].pos_x].ref_idx) : -1;
    rFrameU  = block[1].available
      ? (p_Vid->mb_data[block[1].mb_addr].mb_field
      ? motion[block[1].pos_y][block[1].pos_x].ref_idx >>1
      : motion[block[1].pos_y][block[1].pos_x].ref_idx) : -1;
    rFrameUR = block[2].available
      ? (p_Vid->mb_data[block[2].mb_addr].mb_field
      ? motion[block[2].pos_y][block[2].pos_x].ref_idx >>1
      : motion[block[2].pos_y][block[2].pos_x].ref_idx) : -1;
  }


  /* Prediction if only one of the neighbors uses the reference frame
  *  we are checking
  */
  if(rFrameL == ref_frame && rFrameU != ref_frame && rFrameUR != ref_frame)       
    mvPredType = MVPRED_L;
  else if(rFrameL != ref_frame && rFrameU == ref_frame && rFrameUR != ref_frame)  
    mvPredType = MVPRED_U;
  else if(rFrameL != ref_frame && rFrameU != ref_frame && rFrameUR == ref_frame)  
    mvPredType = MVPRED_UR;
  // Directional predictions
  if(blockshape_x == 8 && blockshape_y == 16)
  {
    if(mb_x == 0)
    {
      if(rFrameL == ref_frame)
        mvPredType = MVPRED_L;
    }
    else
    {
      if( rFrameUR == ref_frame)
        mvPredType = MVPRED_UR;
    }
  }
  else if(blockshape_x == 16 && blockshape_y == 8)
  {
    if(mb_y == 0)
    {
      if(rFrameU == ref_frame)
        mvPredType = MVPRED_U;
    }
    else
    {
      if(rFrameL == ref_frame)
        mvPredType = MVPRED_L;
    }
  }

  for (hv=0; hv < 2; hv++)
  {
    if (hv == 0)
    {
      mv_a = block[0].available ? motion[block[0].pos_y][block[0].pos_x].mv[hv] : 0;
      mv_b = block[1].available ? motion[block[1].pos_y][block[1].pos_x].mv[hv] : 0;
      mv_c = block[2].available ? motion[block[2].pos_y][block[2].pos_x].mv[hv] : 0;
    }
    else
    {
			if (currMB->mb_field)
			{
				mv_a = block[0].available  ? p_Vid->mb_data[block[0].mb_addr].mb_field
					? motion[block[0].pos_y][block[0].pos_x].mv[hv]
				: motion[block[0].pos_y][block[0].pos_x].mv[hv] / 2
					: 0;
				mv_b = block[1].available  ? p_Vid->mb_data[block[1].mb_addr].mb_field
					? motion[block[1].pos_y][block[1].pos_x].mv[hv]
				: motion[block[1].pos_y][block[1].pos_x].mv[hv] / 2
					: 0;
				mv_c = block[2].available  ? p_Vid->mb_data[block[2].mb_addr].mb_field
					? motion[block[2].pos_y][block[2].pos_x].mv[hv]
				: motion[block[2].pos_y][block[2].pos_x].mv[hv] / 2
					: 0;
			}
			else
			{
				mv_a = block[0].available  ? p_Vid->mb_data[block[0].mb_addr].mb_field
					? motion[block[0].pos_y][block[0].pos_x].mv[hv] * 2
					: motion[block[0].pos_y][block[0].pos_x].mv[hv]
				: 0;
				mv_b = block[1].available  ? p_Vid->mb_data[block[1].mb_addr].mb_field
					? motion[block[1].pos_y][block[1].pos_x].mv[hv] * 2
					: motion[block[1].pos_y][block[1].pos_x].mv[hv]
				: 0;
				mv_c = block[2].available  ? p_Vid->mb_data[block[2].mb_addr].mb_field
					? motion[block[2].pos_y][block[2].pos_x].mv[hv] * 2
					: motion[block[2].pos_y][block[2].pos_x].mv[hv]
				: 0;
			}
    }

    switch (mvPredType)
    {
    case MVPRED_MEDIAN:
      if(!(block[1].available || block[2].available))
      {
        pred_vec = mv_a;
      }
      else
      {
        pred_vec = mv_a + mv_b + mv_c - imin(mv_a, imin(mv_b, mv_c)) - imax(mv_a, imax(mv_b ,mv_c));
      }
      break;
    case MVPRED_L:
      pred_vec = mv_a;
      break;
    case MVPRED_U:
      pred_vec = mv_b;
      break;
    case MVPRED_UR:
      pred_vec = mv_c;
      break;
    default:
      break;
    }

    pmv[hv] = (short) pred_vec;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Get motion vector predictor
 ************************************************************************
 */
// TODO: benski> make SSE3/MMX version
static void GetMotionVectorPredictorNormal (Macroblock *currMB, 
                                            PixelPos *block,      // <--> block neighbors
                                            short  pmv[2],
                                            short  ref_frame,
                                            PicMotion **motion, 
                                            int    mb_x,
                                            int    mb_y,
                                            int    blockshape_x,
																						int    blockshape_y)
{
	int rFrameL    = block[0].available ? motion[block[0].pos_y][block[0].pos_x].ref_idx : -1;
	int rFrameU    = block[1].available ? motion[block[1].pos_y][block[1].pos_x].ref_idx : -1;
	int rFrameUR   = block[2].available ? motion[block[2].pos_y][block[2].pos_x].ref_idx : -1;

	/* Prediction if only one of the neighbors uses the reference frame
	*  we are checking
	*/
	if (rFrameL == ref_frame && 
		((rFrameU != ref_frame && rFrameUR != ref_frame) || (blockshape_x == 8 && blockshape_y == 16 && mb_x == 0) || (blockshape_x == 16 && blockshape_y == 8 && mb_y != 0)))
	{ // left
		pmv[0] = block[0].available ? motion[block[0].pos_y][block[0].pos_x].mv[0] : 0;
		pmv[1] = block[0].available ? motion[block[0].pos_y][block[0].pos_x].mv[1] : 0;
	}
	else if (rFrameU == ref_frame && 
		((rFrameL != ref_frame && rFrameUR != ref_frame) || (blockshape_x == 16 && blockshape_y == 8 && mb_y == 0)))
	{ // up
		pmv[0] = block[1].available ? motion[block[1].pos_y][block[1].pos_x].mv[0] : 0;
		pmv[1] = block[1].available ? motion[block[1].pos_y][block[1].pos_x].mv[1] : 0;
	}
	else if (rFrameUR == ref_frame &&
		((rFrameL != ref_frame && rFrameU != ref_frame) || (blockshape_x == 8 && blockshape_y == 16 && mb_x != 0)))
	{ // upper right
		pmv[0] = block[2].available ? motion[block[2].pos_y][block[2].pos_x].mv[0] : 0;   
		pmv[1] = block[2].available ? motion[block[2].pos_y][block[2].pos_x].mv[1] : 0;   
	}
	else
	{ // median
		if(!(block[1].available || block[2].available))
		{
			pmv[0] = block[0].available ? motion[block[0].pos_y][block[0].pos_x].mv[0] : 0;
			pmv[1] = block[0].available ? motion[block[0].pos_y][block[0].pos_x].mv[1] : 0;
		}
		else
		{
			int mv_a = block[0].available ? motion[block[0].pos_y][block[0].pos_x].mv[0] : 0;
			int mv_b = block[1].available ? motion[block[1].pos_y][block[1].pos_x].mv[0] : 0;
			int mv_c = block[2].available ? motion[block[2].pos_y][block[2].pos_x].mv[0] : 0;   
			pmv[0] = mv_a + mv_b + mv_c - imin(mv_a, imin(mv_b, mv_c)) - imax(mv_a, imax(mv_b ,mv_c));
			mv_a = block[0].available ? motion[block[0].pos_y][block[0].pos_x].mv[1] : 0;
			mv_b = block[1].available ? motion[block[1].pos_y][block[1].pos_x].mv[1] : 0;
			mv_c = block[2].available ? motion[block[2].pos_y][block[2].pos_x].mv[1] : 0;   
			pmv[1] = mv_a + mv_b + mv_c - imin(mv_a, imin(mv_b, mv_c)) - imax(mv_a, imax(mv_b ,mv_c));
		}
	}
}

void init_motion_vector_prediction(Macroblock *currMB, int mb_aff_frame_flag)
{
  if (mb_aff_frame_flag)
    currMB->GetMVPredictor = GetMotionVectorPredictorMBAFF;
  else
    currMB->GetMVPredictor = GetMotionVectorPredictorNormal;
}
