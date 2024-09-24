
/*!
 *************************************************************************************
 * \file header.c
 *
 * \brief
 *    H.264 Slice headers
 *
 *************************************************************************************
 */

#include "global.h"
#include "elements.h"
#include "defines.h"
#include "fmo.h"
#include "vlc.h"
#include "mbuffer.h"
#include "header.h"

#include "ctx_tables.h"


#if TRACE
#define SYMTRACESTRING(s) strncpy(sym.tracestring,s,TRACESTRING_SIZE)
#else
#define SYMTRACESTRING(s) // do nothing
#endif

static void ref_pic_list_reordering(Slice *currSlice);
static void pred_weight_table(Slice *currSlice);


/*!
 ************************************************************************
 * \brief
 *    calculate Ceil(Log2(uiVal))
 ************************************************************************
 */
unsigned CeilLog2( unsigned uiVal)
{
  unsigned uiTmp = uiVal-1;
  unsigned uiRet = 0;

  while( uiTmp != 0 )
  {
    uiTmp >>= 1;
    uiRet++;
  }
  return uiRet;
}

unsigned CeilLog2_sf( unsigned uiVal)
{
  unsigned uiTmp = uiVal-1;
  unsigned uiRet = 0;

  while( uiTmp > 0 )
  {
    uiTmp >>= 1;
    uiRet++;
  }
  return uiRet;
}

/*!
 ************************************************************************
 * \brief
 *    read the first part of the header (only the pic_parameter_set_id)
 * \return
 *    Length of the first part of the slice header (in bits)
 ************************************************************************
 */
void FirstPartOfSliceHeader(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  byte dP_nr = assignSE2partition[currSlice->dp_mode][SE_HEADER];
  DataPartition *partition = &(currSlice->partArr[dP_nr]);
  Bitstream *currStream = partition->bitstream;
  int tmp;

  // Get first_mb_in_slice
  currSlice->start_mb_nr = ue_v ("SH: first_mb_in_slice", currStream);

  tmp = ue_v ("SH: slice_type", currStream);

  if (tmp > 4) tmp -= 5;

  p_Vid->type = currSlice->slice_type = (SliceType) tmp;

  currSlice->pic_parameter_set_id = ue_v ("SH: pic_parameter_set_id", currStream);

  if( p_Vid->separate_colour_plane_flag )
    p_Vid->colour_plane_id = u_v (2, "SH: colour_plane_id", currStream);
  else
    p_Vid->colour_plane_id = PLANE_Y;
}

/*!
 ************************************************************************
 * \brief
 *    read the scond part of the header (without the pic_parameter_set_id
 * \return
 *    Length of the second part of the Slice header in bits
 ************************************************************************
 */
void RestOfSliceHeader(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

  byte dP_nr = assignSE2partition[currSlice->dp_mode][SE_HEADER];
  DataPartition *partition = &(currSlice->partArr[dP_nr]);
  Bitstream *currStream = partition->bitstream;

  int val, len;

  p_Vid->frame_num = u_v (active_sps->log2_max_frame_num_minus4 + 4, "SH: frame_num", currStream);

  /* Tian Dong: frame_num gap processing, if found */
  if (p_Vid->idr_flag)
  {
    p_Vid->pre_frame_num = p_Vid->frame_num;
    // picture error concealment
    p_Vid->last_ref_pic_poc = 0;
    assert(p_Vid->frame_num == 0);
  }

  if (active_sps->frame_mbs_only_flag)
  {
    p_Vid->structure = FRAME;
    p_Vid->field_pic_flag=0;
  }
  else
  {
    // field_pic_flag   u(1)
    p_Vid->field_pic_flag = u_1("SH: field_pic_flag", currStream);
    if (p_Vid->field_pic_flag)
    {
      // bottom_field_flag  u(1)
      p_Vid->bottom_field_flag = (byte)u_1("SH: bottom_field_flag", currStream);
      p_Vid->structure = p_Vid->bottom_field_flag ? BOTTOM_FIELD : TOP_FIELD;
    }
    else
    {
      p_Vid->structure = FRAME;
      p_Vid->bottom_field_flag = FALSE;
    }
  }

  currSlice->structure = (PictureStructure) p_Vid->structure;

  p_Vid->mb_aff_frame_flag=(active_sps->mb_adaptive_frame_field_flag && (p_Vid->field_pic_flag==0));
  currSlice->mb_aff_frame_flag = p_Vid->mb_aff_frame_flag;

  if (p_Vid->structure == FRAME       ) 
    assert (p_Vid->field_pic_flag == 0);
  if (p_Vid->structure == TOP_FIELD   ) 
    assert (p_Vid->field_pic_flag == 1 && (p_Vid->bottom_field_flag == FALSE));
  if (p_Vid->structure == BOTTOM_FIELD) 
    assert (p_Vid->field_pic_flag == 1 && (p_Vid->bottom_field_flag == TRUE ));

  if (p_Vid->idr_flag)
  {
    p_Vid->idr_pic_id = ue_v("SH: idr_pic_id", currStream);
  }

  if (active_sps->pic_order_cnt_type == 0)
  {
    p_Vid->pic_order_cnt_lsb = u_v(active_sps->log2_max_pic_order_cnt_lsb_minus4 + 4, "SH: pic_order_cnt_lsb", currStream);
    if( p_Vid->active_pps->bottom_field_pic_order_in_frame_present_flag  ==  1 &&  !p_Vid->field_pic_flag )
      p_Vid->delta_pic_order_cnt_bottom = se_v("SH: delta_pic_order_cnt_bottom", currStream);
    else
      p_Vid->delta_pic_order_cnt_bottom = 0;
  }
  if( active_sps->pic_order_cnt_type == 1 && !active_sps->delta_pic_order_always_zero_flag )
  {
    p_Vid->delta_pic_order_cnt[ 0 ] = se_v("SH: delta_pic_order_cnt[0]", currStream);
    if( p_Vid->active_pps->bottom_field_pic_order_in_frame_present_flag  ==  1  &&  !p_Vid->field_pic_flag )
      p_Vid->delta_pic_order_cnt[ 1 ] = se_v("SH: delta_pic_order_cnt[1]", currStream);
  }else
  {
    if (active_sps->pic_order_cnt_type == 1)
    {
      p_Vid->delta_pic_order_cnt[ 0 ] = 0;
      p_Vid->delta_pic_order_cnt[ 1 ] = 0;
    }
  }

  //! redundant_pic_cnt is missing here
  if (p_Vid->active_pps->redundant_pic_cnt_present_flag)
  {
    p_Vid->redundant_pic_cnt = ue_v ("SH: redundant_pic_cnt", currStream);
  }

  if(currSlice->slice_type == B_SLICE)
  {
    currSlice->direct_spatial_mv_pred_flag = u_1 ("SH: direct_spatial_mv_pred_flag", currStream);
  }

  currSlice->num_ref_idx_l0_active = p_Vid->active_pps->num_ref_idx_l0_active_minus1 + 1;
  currSlice->num_ref_idx_l1_active = p_Vid->active_pps->num_ref_idx_l1_active_minus1 + 1;

  if(p_Vid->type==P_SLICE || p_Vid->type == SP_SLICE || p_Vid->type==B_SLICE)
  {
    val = u_1 ("SH: num_ref_idx_override_flag", currStream);
    if (val)
    {
      currSlice->num_ref_idx_l0_active = 1 + ue_v ("SH: num_ref_idx_l0_active_minus1", currStream);

      if(p_Vid->type==B_SLICE)
      {
        currSlice->num_ref_idx_l1_active = 1 + ue_v ("SH: num_ref_idx_l1_active_minus1", currStream);
      }
    }
  }
  if (currSlice->slice_type!=B_SLICE)
  {
    currSlice->num_ref_idx_l1_active = 0;
  }

  ref_pic_list_reordering(currSlice);

  currSlice->apply_weights = ((p_Vid->active_pps->weighted_pred_flag && (currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE) )
          || ((p_Vid->active_pps->weighted_bipred_idc > 0 ) && (currSlice->slice_type == B_SLICE)));

  if ((p_Vid->active_pps->weighted_pred_flag&&(p_Vid->type==P_SLICE|| p_Vid->type == SP_SLICE))||
      (p_Vid->active_pps->weighted_bipred_idc==1 && (p_Vid->type==B_SLICE)))
  {
    pred_weight_table(currSlice);
  }

  if (p_Vid->nal_reference_idc)
    dec_ref_pic_marking(p_Vid, currStream);

  if (p_Vid->active_pps->entropy_coding_mode_flag && p_Vid->type!=I_SLICE && p_Vid->type!=SI_SLICE)
  {
    currSlice->model_number = ue_v("SH: cabac_init_idc", currStream);
  }
  else
  {
    currSlice->model_number = 0;
  }

  currSlice->slice_qp_delta = val = se_v("SH: slice_qp_delta", currStream);
  currSlice->qp = p_Vid->qp = 26 + p_Vid->active_pps->pic_init_qp_minus26 + val;

  if ((p_Vid->qp < -p_Vid->bitdepth_luma_qp_scale) || (p_Vid->qp > 51))
    error ("slice_qp_delta makes slice_qp_y out of range", 500);

  if(p_Vid->type==SP_SLICE || p_Vid->type == SI_SLICE)
  {
    if(p_Vid->type==SP_SLICE)
    {
      p_Vid->sp_switch = u_1 ("SH: sp_for_switch_flag", currStream);
    }
    currSlice->slice_qs_delta = val = se_v("SH: slice_qs_delta", currStream);
    currSlice->qs = 26 + p_Vid->active_pps->pic_init_qs_minus26 + val;    
    if ((currSlice->qs < 0) || (currSlice->qs > 51))
      error ("slice_qs_delta makes slice_qs_y out of range", 500);
  }

  if ( !HI_INTRA_ONLY_PROFILE || (HI_INTRA_ONLY_PROFILE && (p_Inp->intra_profile_deblocking == 1) ))
  //then read flags and parameters from bistream
  {
    if (p_Vid->active_pps->deblocking_filter_control_present_flag)
    {
      currSlice->DFDisableIdc = (short)ue_v ("SH: disable_deblocking_filter_idc", currStream);

      if (currSlice->DFDisableIdc!=1)
      {
        currSlice->DFAlphaC0Offset = (short) (2 * se_v("SH: slice_alpha_c0_offset_div2", currStream));
        currSlice->DFBetaOffset    = (short) (2 * se_v("SH: slice_beta_offset_div2", currStream));
      }
      else
      {
        currSlice->DFAlphaC0Offset = currSlice->DFBetaOffset = 0;
      }
    }
    else
    {
      currSlice->DFDisableIdc = currSlice->DFAlphaC0Offset = currSlice->DFBetaOffset = 0;
    }
  }
  else //By default the Loop Filter is Off
  { //444_TEMP_NOTE: change made below. 08/07/07
    //still need to parse the SEs (read flags and parameters from bistream) but will ignore
    if (p_Vid->active_pps->deblocking_filter_control_present_flag)
    {
      currSlice->DFDisableIdc = (short) ue_v ("SH: disable_deblocking_filter_idc", currStream);

      if (currSlice->DFDisableIdc!=1)
      {
        currSlice->DFAlphaC0Offset = (short) (2 * se_v("SH: slice_alpha_c0_offset_div2", currStream));
        currSlice->DFBetaOffset    = (short) (2 * se_v("SH: slice_beta_offset_div2", currStream));
      }
    }//444_TEMP_NOTE. the end of change. 08/07/07
    //Ignore the SEs, by default the Loop Filter is Off
    currSlice->DFDisableIdc =1;
    currSlice->DFAlphaC0Offset = currSlice->DFBetaOffset = 0;
  }


  if (p_Vid->active_pps->num_slice_groups_minus1>0 && p_Vid->active_pps->slice_group_map_type>=3 &&
      p_Vid->active_pps->slice_group_map_type<=5)
  {
    len = (active_sps->pic_height_in_map_units_minus1+1)*(active_sps->pic_width_in_mbs_minus1+1)/
          (p_Vid->active_pps->slice_group_change_rate_minus1+1);
    if (((active_sps->pic_height_in_map_units_minus1+1)*(active_sps->pic_width_in_mbs_minus1+1))%
          (p_Vid->active_pps->slice_group_change_rate_minus1+1))
          len +=1;

    len = CeilLog2(len+1);

    p_Vid->slice_group_change_cycle = u_v (len, "SH: slice_group_change_cycle", currStream);
  }
  p_Vid->PicHeightInMbs = p_Vid->FrameHeightInMbs / ( 1 + p_Vid->field_pic_flag );
  p_Vid->PicSizeInMbs   = p_Vid->PicWidthInMbs * p_Vid->PicHeightInMbs;
  p_Vid->FrameSizeInMbs = p_Vid->PicWidthInMbs * p_Vid->FrameHeightInMbs;
}


/*!
 ************************************************************************
 * \brief
 *    read the reference picture reordering information
 ************************************************************************
 */
static void ref_pic_list_reordering(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  byte dP_nr = assignSE2partition[currSlice->dp_mode][SE_HEADER];
  DataPartition *partition = &(currSlice->partArr[dP_nr]);
  Bitstream *currStream = partition->bitstream;
  int i, val;

  alloc_ref_pic_list_reordering_buffer(currSlice);

  if (p_Vid->type!=I_SLICE && p_Vid->type!=SI_SLICE)
  {
    val = currSlice->ref_pic_list_reordering_flag_l0 = u_1 ("SH: ref_pic_list_reordering_flag_l0", currStream);

    if (val)
    {
      i=0;
      do
      {
        val = currSlice->reordering_of_pic_nums_idc_l0[i] = ue_v("SH: reordering_of_pic_nums_idc_l0", currStream);
        if (val==0 || val==1)
        {
          currSlice->abs_diff_pic_num_minus1_l0[i] = ue_v("SH: abs_diff_pic_num_minus1_l0", currStream);
        }
        else
        {
          if (val==2)
          {
            currSlice->long_term_pic_idx_l0[i] = ue_v("SH: long_term_pic_idx_l0", currStream);
          }
        }
        i++;
        // assert (i>currSlice->num_ref_idx_l0_active);
      } while (val != 3);
    }
  }

  if (p_Vid->type==B_SLICE)
  {
    val = currSlice->ref_pic_list_reordering_flag_l1 = u_1 ("SH: ref_pic_list_reordering_flag_l1", currStream);

    if (val)
    {
      i=0;
      do
      {
        val = currSlice->reordering_of_pic_nums_idc_l1[i] = ue_v("SH: reordering_of_pic_nums_idc_l1", currStream);
        if (val==0 || val==1)
        {
          currSlice->abs_diff_pic_num_minus1_l1[i] = ue_v("SH: abs_diff_pic_num_minus1_l1", currStream);
        }
        else
        {
          if (val==2)
          {
            currSlice->long_term_pic_idx_l1[i] = ue_v("SH: long_term_pic_idx_l1", currStream);
          }
        }
        i++;
        // assert (i>currSlice->num_ref_idx_l1_active);
      } while (val != 3);
    }
  }

  // set reference index of redundant slices.
  if(p_Vid->redundant_pic_cnt && (p_Vid->type != I_SLICE) )
  {
    p_Vid->redundant_slice_ref_idx = currSlice->abs_diff_pic_num_minus1_l0[0] + 1;
  }
}


static void reset_wp_params(Slice *currSlice)
{
  int i,comp;
  int log_weight_denom;

  for (i=0; i<MAX_REFERENCE_PICTURES; i++)
  {
    for (comp=0; comp<3; comp++)
    {
      log_weight_denom = (comp == 0) ? currSlice->luma_log2_weight_denom : currSlice->chroma_log2_weight_denom;
      currSlice->wp_weight[0][i][comp] = 1 << log_weight_denom;
      currSlice->wp_weight[1][i][comp] = 1 << log_weight_denom;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    read the weighted prediction tables
 ************************************************************************
 */
static void pred_weight_table(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
  byte dP_nr = assignSE2partition[currSlice->dp_mode][SE_HEADER];
  DataPartition *partition = &(currSlice->partArr[dP_nr]);
  Bitstream *currStream = partition->bitstream;
  int luma_weight_flag_l0, luma_weight_flag_l1, chroma_weight_flag_l0, chroma_weight_flag_l1;
  int i,j;

  currSlice->luma_log2_weight_denom = ue_v ("SH: luma_log2_weight_denom", currStream);
  currSlice->wp_round_luma = currSlice->luma_log2_weight_denom ? 1<<(currSlice->luma_log2_weight_denom - 1): 0;

  if ( 0 != active_sps->chroma_format_idc)
  {
    currSlice->chroma_log2_weight_denom = ue_v ("SH: chroma_log2_weight_denom", currStream);
    currSlice->wp_round_chroma = currSlice->chroma_log2_weight_denom ? 1<<(currSlice->chroma_log2_weight_denom - 1): 0;
  }

  reset_wp_params(currSlice);

  for (i=0; i<currSlice->num_ref_idx_l0_active; i++)
  {
    luma_weight_flag_l0 = u_1("SH: luma_weight_flag_l0", currStream);

    if (luma_weight_flag_l0)
    {
      currSlice->wp_weight[0][i][0] = se_v ("SH: luma_weight_l0", currStream);
      currSlice->wp_offset[0][i][0] = se_v ("SH: luma_offset_l0", currStream);
      currSlice->wp_offset[0][i][0] = currSlice->wp_offset[0][i][0]<<(p_Vid->bitdepth_luma - 8);
    }
    else
    {
      currSlice->wp_weight[0][i][0] = 1 << currSlice->luma_log2_weight_denom;
      currSlice->wp_offset[0][i][0] = 0;
    }

    if (active_sps->chroma_format_idc != 0)
    {
      chroma_weight_flag_l0 = u_1 ("SH: chroma_weight_flag_l0", currStream);

      for (j=1; j<3; j++)
      {
        if (chroma_weight_flag_l0)
        {
          currSlice->wp_weight[0][i][j] = se_v("SH: chroma_weight_l0", currStream);
          currSlice->wp_offset[0][i][j] = se_v("SH: chroma_offset_l0", currStream);
          currSlice->wp_offset[0][i][j] = currSlice->wp_offset[0][i][j]<<(p_Vid->bitdepth_chroma-8);
        }
        else
        {
          currSlice->wp_weight[0][i][j] = 1<<currSlice->chroma_log2_weight_denom;
          currSlice->wp_offset[0][i][j] = 0;
        }
      }
    }
  }
  if ((p_Vid->type == B_SLICE) && p_Vid->active_pps->weighted_bipred_idc == 1)
  {
    for (i=0; i<currSlice->num_ref_idx_l1_active; i++)
    {
      luma_weight_flag_l1 = u_1("SH: luma_weight_flag_l1", currStream);

      if (luma_weight_flag_l1)
      {
        currSlice->wp_weight[1][i][0] = se_v ("SH: luma_weight_l1", currStream);
        currSlice->wp_offset[1][i][0] = se_v ("SH: luma_offset_l1", currStream);
        currSlice->wp_offset[1][i][0] = currSlice->wp_offset[1][i][0]<<(p_Vid->bitdepth_luma-8);
      }
      else
      {
        currSlice->wp_weight[1][i][0] = 1<<currSlice->luma_log2_weight_denom;
        currSlice->wp_offset[1][i][0] = 0;
      }

      if (active_sps->chroma_format_idc != 0)
      {
        chroma_weight_flag_l1 = u_1 ("SH: chroma_weight_flag_l1", currStream);

        for (j=1; j<3; j++)
        {
          if (chroma_weight_flag_l1)
          {
            currSlice->wp_weight[1][i][j] = se_v("SH: chroma_weight_l1", currStream);
            currSlice->wp_offset[1][i][j] = se_v("SH: chroma_offset_l1", currStream);
            currSlice->wp_offset[1][i][j] = currSlice->wp_offset[1][i][j]<<(p_Vid->bitdepth_chroma-8);
          }
          else
          {
            currSlice->wp_weight[1][i][j] = 1<<currSlice->chroma_log2_weight_denom;
            currSlice->wp_offset[1][i][j] = 0;
          }
        }
      }
    }
  }
}


/*!
 ************************************************************************
 * \brief
 *    read the memory control operations
 ************************************************************************
 */
void dec_ref_pic_marking(VideoParameters *p_Vid, Bitstream *currStream)
{
  int val;

  DecRefPicMarking_t *tmp_drpm,*tmp_drpm2;

  // free old buffer content
  while (p_Vid->dec_ref_pic_marking_buffer)
  {
    tmp_drpm=p_Vid->dec_ref_pic_marking_buffer;

    p_Vid->dec_ref_pic_marking_buffer=tmp_drpm->Next;
    free (tmp_drpm);
  }

  if (p_Vid->idr_flag)
  {
    p_Vid->no_output_of_prior_pics_flag = u_1("SH: no_output_of_prior_pics_flag", currStream);
    p_Vid->long_term_reference_flag = u_1("SH: long_term_reference_flag", currStream);
  }
  else
  {
    p_Vid->adaptive_ref_pic_buffering_flag = u_1("SH: adaptive_ref_pic_buffering_flag", currStream);
    if (p_Vid->adaptive_ref_pic_buffering_flag)
    {
      // read Memory Management Control Operation
      do
      {
        tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t));
        tmp_drpm->Next=NULL;

        val = tmp_drpm->memory_management_control_operation = ue_v("SH: memory_management_control_operation", currStream);

        if ((val==1)||(val==3))
        {
          tmp_drpm->difference_of_pic_nums_minus1 = ue_v("SH: difference_of_pic_nums_minus1", currStream);
        }
        if (val==2)
        {
          tmp_drpm->long_term_pic_num = ue_v("SH: long_term_pic_num", currStream);
        }

        if ((val==3)||(val==6))
        {
          tmp_drpm->long_term_frame_idx = ue_v("SH: long_term_frame_idx", currStream);
        }
        if (val==4)
        {
          tmp_drpm->max_long_term_frame_idx_plus1 = ue_v("SH: max_long_term_pic_idx_plus1", currStream);
        }

        // add command
        if (p_Vid->dec_ref_pic_marking_buffer==NULL)
        {
          p_Vid->dec_ref_pic_marking_buffer=tmp_drpm;
        }
        else
        {
          tmp_drpm2=p_Vid->dec_ref_pic_marking_buffer;
          while (tmp_drpm2->Next!=NULL) tmp_drpm2=tmp_drpm2->Next;
          tmp_drpm2->Next=tmp_drpm;
        }

      }
      while (val != 0);
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    To calculate the poc values
 *        based upon JVT-F100d2
 *  POC200301: Until Jan 2003, this function will calculate the correct POC
 *    values, but the management of POCs in buffered pictures may need more work.
 * \return
 *    none
 ************************************************************************
 */
void decode_poc(VideoParameters *p_Vid)
{
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
  int i;
  // for POC mode 0:
  unsigned int MaxPicOrderCntLsb = (1<<(active_sps->log2_max_pic_order_cnt_lsb_minus4+4));

  switch ( active_sps->pic_order_cnt_type )
  {
  case 0: // POC MODE 0
    // 1st
    if(p_Vid->idr_flag)
    {
      p_Vid->PrevPicOrderCntMsb = 0;
      p_Vid->PrevPicOrderCntLsb = 0;
    }
    else
    {
      if (p_Vid->last_has_mmco_5)
      {
        if (p_Vid->last_pic_bottom_field)
        {
          p_Vid->PrevPicOrderCntMsb = 0;
          p_Vid->PrevPicOrderCntLsb = 0;
        }
        else
        {
          p_Vid->PrevPicOrderCntMsb = 0;
          p_Vid->PrevPicOrderCntLsb = p_Vid->toppoc;
        }
      }
    }
    // Calculate the MSBs of current picture
    if( p_Vid->pic_order_cnt_lsb  <  p_Vid->PrevPicOrderCntLsb  &&
      ( p_Vid->PrevPicOrderCntLsb - p_Vid->pic_order_cnt_lsb )  >=  ( MaxPicOrderCntLsb / 2 ) )
      p_Vid->PicOrderCntMsb = p_Vid->PrevPicOrderCntMsb + MaxPicOrderCntLsb;
    else if ( p_Vid->pic_order_cnt_lsb  >  p_Vid->PrevPicOrderCntLsb  &&
      ( p_Vid->pic_order_cnt_lsb - p_Vid->PrevPicOrderCntLsb )  >  ( MaxPicOrderCntLsb / 2 ) )
      p_Vid->PicOrderCntMsb = p_Vid->PrevPicOrderCntMsb - MaxPicOrderCntLsb;
    else
      p_Vid->PicOrderCntMsb = p_Vid->PrevPicOrderCntMsb;

    // 2nd

    if(p_Vid->field_pic_flag==0)
    {           //frame pix
      p_Vid->toppoc = p_Vid->PicOrderCntMsb + p_Vid->pic_order_cnt_lsb;
      p_Vid->bottompoc = p_Vid->toppoc + p_Vid->delta_pic_order_cnt_bottom;
      p_Vid->ThisPOC = p_Vid->framepoc = (p_Vid->toppoc < p_Vid->bottompoc)? p_Vid->toppoc : p_Vid->bottompoc; // POC200301
    }
    else if (p_Vid->bottom_field_flag == FALSE)
    {  //top field
      p_Vid->ThisPOC= p_Vid->toppoc = p_Vid->PicOrderCntMsb + p_Vid->pic_order_cnt_lsb;
    }
    else
    {  //bottom field
      p_Vid->ThisPOC= p_Vid->bottompoc = p_Vid->PicOrderCntMsb + p_Vid->pic_order_cnt_lsb;
    }
    p_Vid->framepoc=p_Vid->ThisPOC;

    if ( p_Vid->frame_num!=p_Vid->PreviousFrameNum)
      p_Vid->PreviousFrameNum=p_Vid->frame_num;

    if(p_Vid->nal_reference_idc)
    {
      p_Vid->PrevPicOrderCntLsb = p_Vid->pic_order_cnt_lsb;
      p_Vid->PrevPicOrderCntMsb = p_Vid->PicOrderCntMsb;
    }

    break;

  case 1: // POC MODE 1
    // 1st
    if(p_Vid->idr_flag)
    {
      p_Vid->FrameNumOffset=0;     //  first pix of IDRGOP,
      p_Vid->delta_pic_order_cnt[0]=0;                        //ignore first delta
      if(p_Vid->frame_num)
        error("frame_num not equal to zero in IDR picture", -1020);
    }
    else
    {
      if (p_Vid->last_has_mmco_5)
      {
        p_Vid->PreviousFrameNumOffset = 0;
        p_Vid->PreviousFrameNum = 0;
      }
      if (p_Vid->frame_num<p_Vid->PreviousFrameNum)
      {             //not first pix of IDRGOP
        p_Vid->FrameNumOffset = p_Vid->PreviousFrameNumOffset + p_Vid->MaxFrameNum;
      }
      else
      {
        p_Vid->FrameNumOffset = p_Vid->PreviousFrameNumOffset;
      }
    }

    // 2nd
    if(active_sps->num_ref_frames_in_pic_order_cnt_cycle)
      p_Vid->AbsFrameNum = p_Vid->FrameNumOffset+p_Vid->frame_num;
    else
      p_Vid->AbsFrameNum=0;
    if( (!p_Vid->nal_reference_idc) && p_Vid->AbsFrameNum > 0)
      p_Vid->AbsFrameNum--;

    // 3rd
    p_Vid->ExpectedDeltaPerPicOrderCntCycle=0;

    if(active_sps->num_ref_frames_in_pic_order_cnt_cycle)
    for(i=0;i<(int) active_sps->num_ref_frames_in_pic_order_cnt_cycle;i++)
      p_Vid->ExpectedDeltaPerPicOrderCntCycle += active_sps->offset_for_ref_frame[i];

    if(p_Vid->AbsFrameNum)
    {
      p_Vid->PicOrderCntCycleCnt = (p_Vid->AbsFrameNum-1)/active_sps->num_ref_frames_in_pic_order_cnt_cycle;
      p_Vid->FrameNumInPicOrderCntCycle = (p_Vid->AbsFrameNum-1)%active_sps->num_ref_frames_in_pic_order_cnt_cycle;
      p_Vid->ExpectedPicOrderCnt = p_Vid->PicOrderCntCycleCnt*p_Vid->ExpectedDeltaPerPicOrderCntCycle;
      for(i=0;i<=(int)p_Vid->FrameNumInPicOrderCntCycle;i++)
        p_Vid->ExpectedPicOrderCnt += active_sps->offset_for_ref_frame[i];
    }
    else
      p_Vid->ExpectedPicOrderCnt=0;

    if(!p_Vid->nal_reference_idc)
      p_Vid->ExpectedPicOrderCnt += active_sps->offset_for_non_ref_pic;

    if(p_Vid->field_pic_flag==0)
    {           //frame pix
      p_Vid->toppoc = p_Vid->ExpectedPicOrderCnt + p_Vid->delta_pic_order_cnt[0];
      p_Vid->bottompoc = p_Vid->toppoc + active_sps->offset_for_top_to_bottom_field + p_Vid->delta_pic_order_cnt[1];
      p_Vid->ThisPOC = p_Vid->framepoc = (p_Vid->toppoc < p_Vid->bottompoc)? p_Vid->toppoc : p_Vid->bottompoc; // POC200301
    }
    else if (p_Vid->bottom_field_flag == FALSE)
    {  //top field
      p_Vid->ThisPOC = p_Vid->toppoc = p_Vid->ExpectedPicOrderCnt + p_Vid->delta_pic_order_cnt[0];
    }
    else
    {  //bottom field
      p_Vid->ThisPOC = p_Vid->bottompoc = p_Vid->ExpectedPicOrderCnt + active_sps->offset_for_top_to_bottom_field + p_Vid->delta_pic_order_cnt[0];
    }
    p_Vid->framepoc=p_Vid->ThisPOC;

    p_Vid->PreviousFrameNum=p_Vid->frame_num;
    p_Vid->PreviousFrameNumOffset=p_Vid->FrameNumOffset;

    break;


  case 2: // POC MODE 2
    if(p_Vid->idr_flag) // IDR picture
    {
      p_Vid->FrameNumOffset=0;     //  first pix of IDRGOP,
      p_Vid->ThisPOC = p_Vid->framepoc = p_Vid->toppoc = p_Vid->bottompoc = 0;
      if(p_Vid->frame_num)
        error("frame_num not equal to zero in IDR picture", -1020);
    }
    else
    {
      if (p_Vid->last_has_mmco_5)
      {
        p_Vid->PreviousFrameNum = 0;
        p_Vid->PreviousFrameNumOffset = 0;
      }
      if (p_Vid->frame_num<p_Vid->PreviousFrameNum)
        p_Vid->FrameNumOffset = p_Vid->PreviousFrameNumOffset + p_Vid->MaxFrameNum;
      else
        p_Vid->FrameNumOffset = p_Vid->PreviousFrameNumOffset;


      p_Vid->AbsFrameNum = p_Vid->FrameNumOffset+p_Vid->frame_num;
      if(!p_Vid->nal_reference_idc)
        p_Vid->ThisPOC = (2*p_Vid->AbsFrameNum - 1);
      else
        p_Vid->ThisPOC = (2*p_Vid->AbsFrameNum);

      if (p_Vid->field_pic_flag==0)
        p_Vid->toppoc = p_Vid->bottompoc = p_Vid->framepoc = p_Vid->ThisPOC;
      else if (p_Vid->bottom_field_flag == FALSE)
         p_Vid->toppoc = p_Vid->framepoc = p_Vid->ThisPOC;
      else p_Vid->bottompoc = p_Vid->framepoc = p_Vid->ThisPOC;
    }

    p_Vid->PreviousFrameNum=p_Vid->frame_num;
    p_Vid->PreviousFrameNumOffset=p_Vid->FrameNumOffset;
    break;


  default:
    //error must occurs
    assert( 1==0 );
    break;
  }
}

/*!
 ************************************************************************
 * \brief
 *    A little helper for the debugging of POC code
 * \return
 *    none
 ************************************************************************
 */
int dumppoc(VideoParameters *p_Vid) 
{
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

  printf ("\nPOC locals...\n");
  printf ("toppoc                                %d\n", (int) p_Vid->toppoc);
  printf ("bottompoc                             %d\n", (int) p_Vid->bottompoc);
  printf ("frame_num                             %d\n", (int) p_Vid->frame_num);
  printf ("field_pic_flag                        %d\n", (int) p_Vid->field_pic_flag);
  printf ("bottom_field_flag                     %d\n", (int) p_Vid->bottom_field_flag);
  printf ("POC SPS\n");
  printf ("log2_max_frame_num_minus4             %d\n", (int) active_sps->log2_max_frame_num_minus4);         // POC200301
  printf ("log2_max_pic_order_cnt_lsb_minus4     %d\n", (int) active_sps->log2_max_pic_order_cnt_lsb_minus4);
  printf ("pic_order_cnt_type                    %d\n", (int) active_sps->pic_order_cnt_type);
  printf ("num_ref_frames_in_pic_order_cnt_cycle %d\n", (int) active_sps->num_ref_frames_in_pic_order_cnt_cycle);
  printf ("delta_pic_order_always_zero_flag      %d\n", (int) active_sps->delta_pic_order_always_zero_flag);
  printf ("offset_for_non_ref_pic                %d\n", (int) active_sps->offset_for_non_ref_pic);
  printf ("offset_for_top_to_bottom_field        %d\n", (int) active_sps->offset_for_top_to_bottom_field);
  printf ("offset_for_ref_frame[0]               %d\n", (int) active_sps->offset_for_ref_frame[0]);
  printf ("offset_for_ref_frame[1]               %d\n", (int) active_sps->offset_for_ref_frame[1]);
  printf ("POC in SLice Header\n");
  printf ("bottom_field_pic_order_in_frame_present_flag                %d\n", (int) p_Vid->active_pps->bottom_field_pic_order_in_frame_present_flag);
  printf ("delta_pic_order_cnt[0]                %d\n", (int) p_Vid->delta_pic_order_cnt[0]);
  printf ("delta_pic_order_cnt[1]                %d\n", (int) p_Vid->delta_pic_order_cnt[1]);
  printf ("delta_pic_order_cnt[2]                %d\n", (int) p_Vid->delta_pic_order_cnt[2]);
  printf ("idr_flag                              %d\n", (int) p_Vid->idr_flag);
  printf ("MaxFrameNum                           %d\n", (int) p_Vid->MaxFrameNum);

  return 0;
}

/*!
 ************************************************************************
 * \brief
 *    return the poc of p_Vid as per (8-1) JVT-F100d2
 *  POC200301
 ************************************************************************
 */
int picture_order(VideoParameters *p_Vid)
{
  if (p_Vid->field_pic_flag==0) // is a frame
    return p_Vid->framepoc;
  else if (p_Vid->bottom_field_flag == FALSE) // top field
    return p_Vid->toppoc;
  else // bottom field
    return p_Vid->bottompoc;
}

