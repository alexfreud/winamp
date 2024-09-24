
/*!
 ************************************************************************
 *  \file
 *     parset.c
 *  \brief
 *     Parameter Sets
 *  \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Stephan Wenger          <stewe@cs.tu-berlin.de>
 *
 ***********************************************************************
 */

#include "global.h"
#include "image.h"
#include "parsetcommon.h"
#include "parset.h"
#include "nalu.h"
#include "memalloc.h"
#include "fmo.h"
#include "cabac.h"
#include "vlc.h"
#include "mbuffer.h"
#include "erc_api.h"

#if TRACE
#define SYMTRACESTRING(s) strncpy(sym->tracestring,s,TRACESTRING_SIZE)
#else
#define SYMTRACESTRING(s) // do nothing
#endif


extern void init_frext(VideoParameters *p_Vid);

// syntax for scaling list matrix values
void Scaling_List(int *scalingList, int sizeOfScalingList, Boolean *UseDefaultScalingMatrix, Bitstream *s)
{
  int j, scanj;
  int delta_scale, lastScale, nextScale;

  lastScale      = 8;
  nextScale      = 8;

  for(j=0; j<sizeOfScalingList; j++)
  {
    scanj = (sizeOfScalingList==16) ? ZZ_SCAN[j]:ZZ_SCAN8[j];

    if(nextScale!=0)
    {
      delta_scale = se_v (   "   : delta_sl   "                           , s);
      nextScale = (lastScale + delta_scale + 256) % 256;
      *UseDefaultScalingMatrix = (Boolean) (scanj==0 && nextScale==0);
    }

    scalingList[scanj] = (nextScale==0) ? lastScale:nextScale;
    lastScale = scalingList[scanj];
  }
}
// fill sps with content of p

static void InterpretSPS (VideoParameters *p_Vid, DataPartition *p, seq_parameter_set_rbsp_t *sps)
{
  unsigned i;
  unsigned n_ScalingList;
  int reserved_zero;
  Bitstream *s = p->bitstream;

  assert (p != NULL);
  assert (p->bitstream != NULL);
  assert (p->bitstream->streamBuffer != 0);
  assert (sps != NULL);

  sps->profile_idc                            = u_v  (8, "SPS: profile_idc"                           , s);

  if ((sps->profile_idc!=BASELINE       ) &&
      (sps->profile_idc!=MAIN           ) &&
      (sps->profile_idc!=EXTENDED       ) &&
      (sps->profile_idc!=FREXT_HP       ) &&
      (sps->profile_idc!=FREXT_Hi10P    ) &&
      (sps->profile_idc!=FREXT_Hi422    ) &&
      (sps->profile_idc!=FREXT_Hi444    ) &&
      (sps->profile_idc!=FREXT_CAVLC444 ))
  {
    printf("Invalid Profile IDC (%d) encountered. \n", sps->profile_idc);
    return;
  }

  sps->constrained_set0_flag                  = u_1  (   "SPS: constrained_set0_flag"                 , s);
  sps->constrained_set1_flag                  = u_1  (   "SPS: constrained_set1_flag"                 , s);
  sps->constrained_set2_flag                  = u_1  (   "SPS: constrained_set2_flag"                 , s);
  sps->constrained_set3_flag                  = u_1  (   "SPS: constrained_set3_flag"                 , s);
  reserved_zero                               = u_v  (4, "SPS: reserved_zero_4bits"                   , s);
  assert (reserved_zero==0);

  sps->level_idc                              = u_v  (8, "SPS: level_idc"                             , s);

  sps->seq_parameter_set_id                   = ue_v ("SPS: seq_parameter_set_id"                     , s);

  // Fidelity Range Extensions stuff
  sps->chroma_format_idc = 1;
  sps->bit_depth_luma_minus8   = 0;
  sps->bit_depth_chroma_minus8 = 0;
  p_Vid->lossless_qpprime_flag   = 0;
  sps->separate_colour_plane_flag = 0;

  if((IS_FREXT_PROFILE(sps->profile_idc)))/*==FREXT_HP   ) ||
     (sps->profile_idc==FREXT_Hi10P) ||
     (sps->profile_idc==FREXT_Hi422) ||
     (sps->profile_idc==FREXT_Hi444) ||
     (sps->profile_idc==FREXT_CAVLC444))*/
  {
    sps->chroma_format_idc                      = ue_v ("SPS: chroma_format_idc"                       , s);

    if(sps->chroma_format_idc == YUV444)
    {
      sps->separate_colour_plane_flag           = u_1  ("SPS: separate_colour_plane_flag"              , s);
    }

    sps->bit_depth_luma_minus8                  = ue_v ("SPS: bit_depth_luma_minus8"                   , s);
    sps->bit_depth_chroma_minus8                = ue_v ("SPS: bit_depth_chroma_minus8"                 , s);
    p_Vid->lossless_qpprime_flag                  = u_1  ("SPS: lossless_qpprime_y_zero_flag"            , s);

    sps->seq_scaling_matrix_present_flag        = u_1  (   "SPS: seq_scaling_matrix_present_flag"       , s);

    if(sps->seq_scaling_matrix_present_flag)
    {
      n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
      for(i=0; i<n_ScalingList; i++)
      {
        sps->seq_scaling_list_present_flag[i]   = u_1  (   "SPS: seq_scaling_list_present_flag"         , s);
        if(sps->seq_scaling_list_present_flag[i])
        {
          if(i<6)
            Scaling_List(sps->ScalingList4x4[i], 16, &sps->UseDefaultScalingMatrix4x4Flag[i], s);
          else
            Scaling_List(sps->ScalingList8x8[i-6], 64, &sps->UseDefaultScalingMatrix8x8Flag[i-6], s);
        }
      }
    }
  }

  sps->log2_max_frame_num_minus4              = ue_v ("SPS: log2_max_frame_num_minus4"                , s);
  sps->pic_order_cnt_type                     = ue_v ("SPS: pic_order_cnt_type"                       , s);

  if (sps->pic_order_cnt_type == 0)
    sps->log2_max_pic_order_cnt_lsb_minus4 = ue_v ("SPS: log2_max_pic_order_cnt_lsb_minus4"           , s);
  else if (sps->pic_order_cnt_type == 1)
  {
    sps->delta_pic_order_always_zero_flag      = u_1  ("SPS: delta_pic_order_always_zero_flag"       , s);
    sps->offset_for_non_ref_pic                = se_v ("SPS: offset_for_non_ref_pic"                 , s);
    sps->offset_for_top_to_bottom_field        = se_v ("SPS: offset_for_top_to_bottom_field"         , s);
    sps->num_ref_frames_in_pic_order_cnt_cycle = ue_v ("SPS: num_ref_frames_in_pic_order_cnt_cycle"  , s);
    for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
      sps->offset_for_ref_frame[i]               = se_v ("SPS: offset_for_ref_frame[i]"              , s);
  }
  sps->num_ref_frames                        = ue_v ("SPS: num_ref_frames"                         , s);
  sps->gaps_in_frame_num_value_allowed_flag  = u_1  ("SPS: gaps_in_frame_num_value_allowed_flag"   , s);
  sps->pic_width_in_mbs_minus1               = ue_v ("SPS: pic_width_in_mbs_minus1"                , s);
  sps->pic_height_in_map_units_minus1        = ue_v ("SPS: pic_height_in_map_units_minus1"         , s);
  sps->frame_mbs_only_flag                   = u_1  ("SPS: frame_mbs_only_flag"                    , s);
  if (!sps->frame_mbs_only_flag)
  {
    sps->mb_adaptive_frame_field_flag        = u_1  ("SPS: mb_adaptive_frame_field_flag"           , s);
  }
  sps->direct_8x8_inference_flag             = u_1  ("SPS: direct_8x8_inference_flag"              , s);
  sps->frame_cropping_flag                   = u_1  ("SPS: frame_cropping_flag"                    , s);

  if (sps->frame_cropping_flag)
  {
    sps->frame_cropping_rect_left_offset      = ue_v ("SPS: frame_cropping_rect_left_offset"           , s);
    sps->frame_cropping_rect_right_offset     = ue_v ("SPS: frame_cropping_rect_right_offset"          , s);
    sps->frame_cropping_rect_top_offset       = ue_v ("SPS: frame_cropping_rect_top_offset"            , s);
    sps->frame_cropping_rect_bottom_offset    = ue_v ("SPS: frame_cropping_rect_bottom_offset"         , s);
  }
  sps->vui_parameters_present_flag           = (Boolean) u_1  ("SPS: vui_parameters_present_flag"      , s);

  InitVUI(sps);
  ReadVUI(p, sps);

  sps->Valid = TRUE;
}


void InitVUI(seq_parameter_set_rbsp_t *sps)
{
  sps->vui_seq_parameters.matrix_coefficients = 2;
}


int ReadVUI(DataPartition *p, seq_parameter_set_rbsp_t *sps)
{
  Bitstream *s = p->bitstream;
  if (sps->vui_parameters_present_flag)
  {
    sps->vui_seq_parameters.aspect_ratio_info_present_flag = u_1  ("VUI: aspect_ratio_info_present_flag"   , s);
    if (sps->vui_seq_parameters.aspect_ratio_info_present_flag)
    {
      sps->vui_seq_parameters.aspect_ratio_idc             = u_v  ( 8, "VUI: aspect_ratio_idc"              , s);
      if (255==sps->vui_seq_parameters.aspect_ratio_idc)
      {
        sps->vui_seq_parameters.sar_width                  = (unsigned short) u_v  (16, "VUI: sar_width"                     , s);
        sps->vui_seq_parameters.sar_height                 = (unsigned short) u_v  (16, "VUI: sar_height"                    , s);
      }
  }

    sps->vui_seq_parameters.overscan_info_present_flag     = u_1  ("VUI: overscan_info_present_flag"        , s);
    if (sps->vui_seq_parameters.overscan_info_present_flag)
    {
      sps->vui_seq_parameters.overscan_appropriate_flag    = u_1  ("VUI: overscan_appropriate_flag"         , s);
    }

    sps->vui_seq_parameters.video_signal_type_present_flag = u_1  ("VUI: video_signal_type_present_flag"    , s);
    if (sps->vui_seq_parameters.video_signal_type_present_flag)
    {
      sps->vui_seq_parameters.video_format                    = u_v  ( 3,"VUI: video_format"                      , s);
      sps->vui_seq_parameters.video_full_range_flag           = u_1  (   "VUI: video_full_range_flag"             , s);
      sps->vui_seq_parameters.colour_description_present_flag = u_1  (   "VUI: color_description_present_flag"    , s);
      if(sps->vui_seq_parameters.colour_description_present_flag)
      {
        sps->vui_seq_parameters.colour_primaries              = u_v  ( 8,"VUI: colour_primaries"                  , s);
        sps->vui_seq_parameters.transfer_characteristics      = u_v  ( 8,"VUI: transfer_characteristics"          , s);
        sps->vui_seq_parameters.matrix_coefficients           = u_v  ( 8,"VUI: matrix_coefficients"               , s);
      }
    }
    sps->vui_seq_parameters.chroma_location_info_present_flag = u_1  (   "VUI: chroma_loc_info_present_flag"      , s);
    if(sps->vui_seq_parameters.chroma_location_info_present_flag)
    {
      sps->vui_seq_parameters.chroma_sample_loc_type_top_field     = ue_v  ( "VUI: chroma_sample_loc_type_top_field"    , s);
      sps->vui_seq_parameters.chroma_sample_loc_type_bottom_field  = ue_v  ( "VUI: chroma_sample_loc_type_bottom_field" , s);
    }
    sps->vui_seq_parameters.timing_info_present_flag          = u_1  ("VUI: timing_info_present_flag"           , s);
    if (sps->vui_seq_parameters.timing_info_present_flag)
    {
      sps->vui_seq_parameters.num_units_in_tick               = u_v  (32,"VUI: num_units_in_tick"               , s);
      sps->vui_seq_parameters.time_scale                      = u_v  (32,"VUI: time_scale"                      , s);
      sps->vui_seq_parameters.fixed_frame_rate_flag           = u_1  (   "VUI: fixed_frame_rate_flag"           , s);
    }
    sps->vui_seq_parameters.nal_hrd_parameters_present_flag   = u_1  ("VUI: nal_hrd_parameters_present_flag"    , s);
    if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
    {
      ReadHRDParameters(p, &(sps->vui_seq_parameters.nal_hrd_parameters));
    }
    sps->vui_seq_parameters.vcl_hrd_parameters_present_flag   = u_1  ("VUI: vcl_hrd_parameters_present_flag"    , s);
    if (sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
    {
      ReadHRDParameters(p, &(sps->vui_seq_parameters.vcl_hrd_parameters));
    }
    if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag || sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
    {
      sps->vui_seq_parameters.low_delay_hrd_flag             =  u_1  ("VUI: low_delay_hrd_flag"                 , s);
    }
    sps->vui_seq_parameters.pic_struct_present_flag          =  u_1  ("VUI: pic_struct_present_flag   "         , s);
    sps->vui_seq_parameters.bitstream_restriction_flag       =  u_1  ("VUI: bitstream_restriction_flag"         , s);
    if (sps->vui_seq_parameters.bitstream_restriction_flag)
    {
      sps->vui_seq_parameters.motion_vectors_over_pic_boundaries_flag =  u_1  ("VUI: motion_vectors_over_pic_boundaries_flag", s);
      sps->vui_seq_parameters.max_bytes_per_pic_denom                 =  ue_v ("VUI: max_bytes_per_pic_denom"                , s);
      sps->vui_seq_parameters.max_bits_per_mb_denom                   =  ue_v ("VUI: max_bits_per_mb_denom"                  , s);
      sps->vui_seq_parameters.log2_max_mv_length_horizontal           =  ue_v ("VUI: log2_max_mv_length_horizontal"          , s);
      sps->vui_seq_parameters.log2_max_mv_length_vertical             =  ue_v ("VUI: log2_max_mv_length_vertical"            , s);
      sps->vui_seq_parameters.num_reorder_frames                      =  ue_v ("VUI: num_reorder_frames"                     , s);
      sps->vui_seq_parameters.max_dec_frame_buffering                 =  ue_v ("VUI: max_dec_frame_buffering"                , s);
    }
  }

  return 0;
}


int ReadHRDParameters(DataPartition *p, hrd_parameters_t *hrd)
{
  Bitstream *s = p->bitstream;
  unsigned int SchedSelIdx;

  hrd->cpb_cnt_minus1                                      = ue_v (   "VUI: cpb_cnt_minus1"                       , s);
  hrd->bit_rate_scale                                      = u_v  ( 4,"VUI: bit_rate_scale"                       , s);
  hrd->cpb_size_scale                                      = u_v  ( 4,"VUI: cpb_size_scale"                       , s);

  for( SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1; SchedSelIdx++ )
  {
    hrd->bit_rate_value_minus1[ SchedSelIdx ]             = ue_v  ( "VUI: bit_rate_value_minus1"                  , s);
    hrd->cpb_size_value_minus1[ SchedSelIdx ]             = ue_v  ( "VUI: cpb_size_value_minus1"                  , s);
    hrd->cbr_flag[ SchedSelIdx ]                          = u_1   ( "VUI: cbr_flag"                               , s);
  }

  hrd->initial_cpb_removal_delay_length_minus1            = u_v  ( 5,"VUI: initial_cpb_removal_delay_length_minus1" , s);
  hrd->cpb_removal_delay_length_minus1                    = u_v  ( 5,"VUI: cpb_removal_delay_length_minus1"         , s);
  hrd->dpb_output_delay_length_minus1                     = u_v  ( 5,"VUI: dpb_output_delay_length_minus1"          , s);
  hrd->time_offset_length                                 = u_v  ( 5,"VUI: time_offset_length"          , s);

  return 0;
}


static void InterpretPPS (VideoParameters *p_Vid, DataPartition *p, pic_parameter_set_rbsp_t *pps)
{
  unsigned i;
  unsigned n_ScalingList;
  int chroma_format_idc;
  int NumberBitsPerSliceGroupId;
  Bitstream *s = p->bitstream;

  assert (p != NULL);
  assert (p->bitstream != NULL);
  assert (p->bitstream->streamBuffer != 0);
  assert (pps != NULL);

  pps->pic_parameter_set_id                  = ue_v ("PPS: pic_parameter_set_id"                   , s);
  pps->seq_parameter_set_id                  = ue_v ("PPS: seq_parameter_set_id"                   , s);
  pps->entropy_coding_mode_flag              = u_1  ("PPS: entropy_coding_mode_flag"               , s);

  //! Note: as per JVT-F078 the following bit is unconditional.  If F078 is not accepted, then
  //! one has to fetch the correct SPS to check whether the bit is present (hopefully there is
  //! no consistency problem :-(
  //! The current encoder code handles this in the same way.  When you change this, don't forget
  //! the encoder!  StW, 12/8/02
  pps->bottom_field_pic_order_in_frame_present_flag                = u_1  ("PPS: bottom_field_pic_order_in_frame_present_flag"                 , s);

  pps->num_slice_groups_minus1               = ue_v ("PPS: num_slice_groups_minus1"                , s);

  // FMO stuff begins here
  if (pps->num_slice_groups_minus1 > 0)
  {
    pps->slice_group_map_type               = ue_v ("PPS: slice_group_map_type"                , s);
    if (pps->slice_group_map_type == 0)
    {
      for (i=0; i<=pps->num_slice_groups_minus1; i++)
        pps->run_length_minus1 [i]                  = ue_v ("PPS: run_length_minus1 [i]"              , s);
    }
    else if (pps->slice_group_map_type == 2)
    {
      for (i=0; i<pps->num_slice_groups_minus1; i++)
      {
        //! JVT-F078: avoid reference of SPS by using ue(v) instead of u(v)
        pps->top_left [i]                          = ue_v ("PPS: top_left [i]"                        , s);
        pps->bottom_right [i]                      = ue_v ("PPS: bottom_right [i]"                    , s);
      }
    }
    else if (pps->slice_group_map_type == 3 ||
             pps->slice_group_map_type == 4 ||
             pps->slice_group_map_type == 5)
    {
      pps->slice_group_change_direction_flag     = u_1  ("PPS: slice_group_change_direction_flag"      , s);
      pps->slice_group_change_rate_minus1        = ue_v ("PPS: slice_group_change_rate_minus1"         , s);
    }
    else if (pps->slice_group_map_type == 6)
    {
      if (pps->num_slice_groups_minus1+1 >4)
        NumberBitsPerSliceGroupId = 3;
      else if (pps->num_slice_groups_minus1+1 > 2)
        NumberBitsPerSliceGroupId = 2;
      else
        NumberBitsPerSliceGroupId = 1;
      pps->pic_size_in_map_units_minus1      = ue_v ("PPS: pic_size_in_map_units_minus1"               , s);
      if ((pps->slice_group_id = calloc (pps->pic_size_in_map_units_minus1+1, 1)) == NULL)
        no_mem_exit ("InterpretPPS: slice_group_id");
      for (i=0; i<=pps->pic_size_in_map_units_minus1; i++)
        pps->slice_group_id[i] = (byte) u_v (NumberBitsPerSliceGroupId, "slice_group_id[i]", s);
    }
  }

  // End of FMO stuff

  pps->num_ref_idx_l0_active_minus1          = ue_v ("PPS: num_ref_idx_l0_active_minus1"           , s);
  pps->num_ref_idx_l1_active_minus1          = ue_v ("PPS: num_ref_idx_l1_active_minus1"           , s);
  pps->weighted_pred_flag                    = u_1  ("PPS: weighted_pred_flag"                     , s);
  pps->weighted_bipred_idc                   = u_v  ( 2, "PPS: weighted_bipred_idc"                , s);
  pps->pic_init_qp_minus26                   = se_v ("PPS: pic_init_qp_minus26"                    , s);
  pps->pic_init_qs_minus26                   = se_v ("PPS: pic_init_qs_minus26"                    , s);

  pps->chroma_qp_index_offset                = se_v ("PPS: chroma_qp_index_offset"                 , s);

  pps->deblocking_filter_control_present_flag = u_1 ("PPS: deblocking_filter_control_present_flag" , s);
  pps->constrained_intra_pred_flag           = u_1  ("PPS: constrained_intra_pred_flag"            , s);
  pps->redundant_pic_cnt_present_flag        = u_1  ("PPS: redundant_pic_cnt_present_flag"         , s);

  if(more_rbsp_data(s->streamBuffer, s->frame_bitoffset,s->bitstream_length)) // more_data_in_rbsp()
  {
    //Fidelity Range Extensions Stuff
    pps->transform_8x8_mode_flag           = u_1  ("PPS: transform_8x8_mode_flag"                , s);
    pps->pic_scaling_matrix_present_flag   =  u_1  ("PPS: pic_scaling_matrix_present_flag"        , s);

    if(pps->pic_scaling_matrix_present_flag)
    {
      chroma_format_idc = p_Vid->SeqParSet[pps->seq_parameter_set_id].chroma_format_idc;
      n_ScalingList = 6 + ((chroma_format_idc != YUV444) ? 2 : 6) * pps->transform_8x8_mode_flag;
      for(i=0; i<n_ScalingList; i++)
      {
        pps->pic_scaling_list_present_flag[i]= u_1  ("PPS: pic_scaling_list_present_flag"          , s);

        if(pps->pic_scaling_list_present_flag[i])
        {
          if(i<6)
            Scaling_List(pps->ScalingList4x4[i], 16, &pps->UseDefaultScalingMatrix4x4Flag[i], s);
          else
            Scaling_List(pps->ScalingList8x8[i-6], 64, &pps->UseDefaultScalingMatrix8x8Flag[i-6], s);
        }
      }
    }
    pps->second_chroma_qp_index_offset      = se_v ("PPS: second_chroma_qp_index_offset"          , s);
  }
  else
  {
    pps->second_chroma_qp_index_offset      = pps->chroma_qp_index_offset;
  }

  pps->Valid = TRUE;
}


void PPSConsistencyCheck (pic_parameter_set_rbsp_t *pps)
{
  printf ("Consistency checking a picture parset, to be implemented\n");
//  if (pps->seq_parameter_set_id invalid then do something)
}

void SPSConsistencyCheck (seq_parameter_set_rbsp_t *sps)
{
  printf ("Consistency checking a sequence parset, to be implemented\n");
}

void MakePPSavailable (VideoParameters *p_Vid, int id, pic_parameter_set_rbsp_t *pps)
{
  assert (pps->Valid == TRUE);

  if (p_Vid->PicParSet[id].Valid == TRUE && p_Vid->PicParSet[id].slice_group_id != NULL)
    free (p_Vid->PicParSet[id].slice_group_id);

  memcpy (&p_Vid->PicParSet[id], pps, sizeof (pic_parameter_set_rbsp_t));

  // we can simply use the memory provided with the pps. the PPS is destroyed after this function
  // call and will not try to free if pps->slice_group_id == NULL
  p_Vid->PicParSet[id].slice_group_id = pps->slice_group_id;
  pps->slice_group_id          = NULL;
}

void CleanUpPPS(VideoParameters *p_Vid)
{
  int i;

  for (i=0; i<MAXPPS; i++)
  {
    if (p_Vid->PicParSet[i].Valid == TRUE && p_Vid->PicParSet[i].slice_group_id != NULL)
      free (p_Vid->PicParSet[i].slice_group_id);

    p_Vid->PicParSet[i].Valid = FALSE;
  }
}


void MakeSPSavailable (VideoParameters *p_Vid, int id, seq_parameter_set_rbsp_t *sps)
{
  assert (sps->Valid == TRUE);
  memcpy (&p_Vid->SeqParSet[id], sps, sizeof (seq_parameter_set_rbsp_t));
}

void ProcessSPS_Memory(VideoParameters *p_Vid, const void *buffer, size_t bufferlen)
{
  
}

void ProcessSPS(VideoParameters *p_Vid, NALU_t *nalu)
{ 
	DataPartition *dp = AllocPartition(1);
  seq_parameter_set_rbsp_t *sps = AllocSPS();
	
  //memcpy (dp->bitstream->streamBuffer, buffer, bufferlen);
	dp->bitstream->streamBuffer = &nalu->buf[1];
  dp->bitstream->code_len = dp->bitstream->bitstream_length = RBSPtoSODB (dp->bitstream->streamBuffer, nalu->len-1);
  dp->bitstream->read_len = dp->bitstream->frame_bitoffset = 0;
  InterpretSPS (p_Vid, dp, sps);

  if (sps->Valid)
  {
    if (p_Vid->active_sps)
    {
      if (sps->seq_parameter_set_id == p_Vid->active_sps->seq_parameter_set_id)
      {
        if (!sps_is_equal(sps, p_Vid->active_sps))
        {
          if (p_Vid->dec_picture)
          {
            // this may only happen on slice loss
            exit_picture(p_Vid, &p_Vid->dec_picture);
          }
          p_Vid->active_sps=NULL;
        }
      }
    }
    // SPSConsistencyCheck (pps);
    MakeSPSavailable (p_Vid, sps->seq_parameter_set_id, sps);
    p_Vid->profile_idc = sps->profile_idc;
    p_Vid->separate_colour_plane_flag = sps->separate_colour_plane_flag;
    if( p_Vid->separate_colour_plane_flag )
    {
      p_Vid->ChromaArrayType = 0;
    }
    else
    {
      p_Vid->ChromaArrayType = sps->chroma_format_idc;
    }
  }

  FreePartition (dp, 1);
  FreeSPS (sps);
	
}

void ProcessPPS (VideoParameters *p_Vid, NALU_t *nalu)
{
  DataPartition *dp = AllocPartition(1);
  pic_parameter_set_rbsp_t *pps = AllocPPS();

  //memcpy (dp->bitstream->streamBuffer, &nalu->buf[1], nalu->len-1);
	dp->bitstream->streamBuffer = &nalu->buf[1];
  dp->bitstream->code_len = dp->bitstream->bitstream_length = RBSPtoSODB (dp->bitstream->streamBuffer, nalu->len-1);
  dp->bitstream->read_len = dp->bitstream->frame_bitoffset = 0;
  InterpretPPS (p_Vid, dp, pps);
  // PPSConsistencyCheck (pps);
  if (p_Vid->active_pps)
  {
    if (pps->pic_parameter_set_id == p_Vid->active_pps->pic_parameter_set_id)
    {
      if (!pps_is_equal(pps, p_Vid->active_pps))
      {
        if (p_Vid->dec_picture)
        {
          // this may only happen on slice loss
          exit_picture(p_Vid, &p_Vid->dec_picture);
        }
        p_Vid->active_pps = NULL;
      }
    }
  }
  MakePPSavailable (p_Vid, pps->pic_parameter_set_id, pps);
  FreePartition (dp, 1);
  FreePPS (pps);
}

/*!
 ************************************************************************
 * \brief
 *    Updates images max values
 *
 ************************************************************************
 */
static void updateMaxValue(FrameFormat *format)
{
  format->max_value[0] = (1 << format->bit_depth[0]) - 1;
  format->max_value_sq[0] = format->max_value[0] * format->max_value[0];
  format->max_value[1] = (1 << format->bit_depth[1]) - 1;
  format->max_value_sq[1] = format->max_value[1] * format->max_value[1];
  format->max_value[2] = (1 << format->bit_depth[2]) - 1;
  format->max_value_sq[2] = format->max_value[2] * format->max_value[2];
}

/*!
 ************************************************************************
 * \brief
 *    Reset format information
 *
 ************************************************************************
 */
static void reset_format_info(seq_parameter_set_rbsp_t *sps, VideoParameters *p_Vid, FrameFormat *output)
{
  InputParameters *p_Inp = p_Vid->p_Inp;
  static const int SubWidthC  [4]= { 1, 2, 2, 1};
  static const int SubHeightC [4]= { 1, 2, 1, 1};

  int crop_left, crop_right;
  int crop_top, crop_bottom;

  // cropping for luma
  if (sps->frame_cropping_flag)
  {
    crop_left   = SubWidthC [sps->chroma_format_idc] * sps->frame_cropping_rect_left_offset;
    crop_right  = SubWidthC [sps->chroma_format_idc] * sps->frame_cropping_rect_right_offset;
    crop_top    = SubHeightC[sps->chroma_format_idc] * ( 2 - sps->frame_mbs_only_flag ) *  sps->frame_cropping_rect_top_offset;
    crop_bottom = SubHeightC[sps->chroma_format_idc] * ( 2 - sps->frame_mbs_only_flag ) *  sps->frame_cropping_rect_bottom_offset;
  }
  else
  {
    crop_left = crop_right = crop_top = crop_bottom = 0;
	}

	output->width_crop = p_Vid->width - crop_left - crop_right;
	output->height_crop = p_Vid->height - crop_top - crop_bottom;

  output->width     = p_Vid->width;
  output->height    = p_Vid->height;

  output->width_cr  = p_Vid->width_cr;
  output->height_cr = p_Vid->height_cr;

  // output size (excluding padding)
  output->size_cmp[0] = output->width * output->height;
  output->size_cmp[1] = output->width_cr * output->height_cr;
  output->size_cmp[2] = output->size_cmp[1];
  output->size        = output->size_cmp[0] + output->size_cmp[1] + output->size_cmp[2];
  output->mb_width    = output->width  / MB_BLOCK_SIZE;
  output->mb_height   = output->height / MB_BLOCK_SIZE;

  output->bit_depth[0] = p_Vid->bitdepth_luma;
  output->bit_depth[1] = p_Vid->bitdepth_chroma;
  output->bit_depth[2] = p_Vid->bitdepth_chroma;  

  output->yuv_format  = (ColorFormat) sps->chroma_format_idc;

  updateMaxValue(output);
}

/*!
 ************************************************************************
 * \brief
 *    Activate Sequence Parameter Sets
 *
 ************************************************************************
 */
void activate_sps (VideoParameters *p_Vid, seq_parameter_set_rbsp_t *sps)
{
  InputParameters *p_Inp = p_Vid->p_Inp;  

  if (p_Vid->active_sps != sps)
  {
    if (p_Vid->dec_picture)
    {
      // this may only happen on slice loss
      exit_picture(p_Vid, &p_Vid->dec_picture);
    }
    p_Vid->active_sps = sps;

    p_Vid->bitdepth_chroma = 0;
    p_Vid->width_cr        = 0;
    p_Vid->height_cr       = 0;

    // maximum vertical motion vector range in luma quarter pixel units
    if (p_Vid->active_sps->level_idc <= 10)
    {
      p_Vid->max_vmv_r = 64 * 4;
    }
    else if (p_Vid->active_sps->level_idc <= 20)
    {
      p_Vid->max_vmv_r = 128 * 4;
    }
    else if (p_Vid->active_sps->level_idc <= 30)
    {
      p_Vid->max_vmv_r = 256 * 4;
    }
    else
    {
      p_Vid->max_vmv_r = 512 * 4; // 512 pixels in quarter pixels
    }

    // Fidelity Range Extensions stuff (part 1)
    p_Vid->bitdepth_luma       = (short) (sps->bit_depth_luma_minus8 + 8);
    p_Vid->bitdepth_scale[0]   = 1 << sps->bit_depth_luma_minus8;
    if (sps->chroma_format_idc != YUV400)
    {
      p_Vid->bitdepth_chroma   = (short) (sps->bit_depth_chroma_minus8 + 8);
      p_Vid->bitdepth_scale[1] = 1 << sps->bit_depth_chroma_minus8;
    }

    p_Vid->MaxFrameNum = 1<<(sps->log2_max_frame_num_minus4+4);
    p_Vid->PicWidthInMbs = (sps->pic_width_in_mbs_minus1 +1);
    p_Vid->PicHeightInMapUnits = (sps->pic_height_in_map_units_minus1 +1);
    p_Vid->FrameHeightInMbs = ( 2 - sps->frame_mbs_only_flag ) * p_Vid->PicHeightInMapUnits;
    p_Vid->FrameSizeInMbs = p_Vid->PicWidthInMbs * p_Vid->FrameHeightInMbs;

    p_Vid->yuv_format=sps->chroma_format_idc;

    p_Vid->width = p_Vid->PicWidthInMbs * MB_BLOCK_SIZE;
    p_Vid->height = p_Vid->FrameHeightInMbs * MB_BLOCK_SIZE;
    
    if (sps->chroma_format_idc == YUV420)
    {
      p_Vid->width_cr  = (p_Vid->width  >> 1);
      p_Vid->height_cr = (p_Vid->height >> 1);
    }
    else if (sps->chroma_format_idc == YUV422)
    {
      p_Vid->width_cr  = (p_Vid->width >> 1);
      p_Vid->height_cr = p_Vid->height;
    }
    else if (sps->chroma_format_idc == YUV444)
    {
      //YUV444
      p_Vid->width_cr = p_Vid->width;
      p_Vid->height_cr = p_Vid->height;
    }

    init_frext(p_Vid);
    init_global_buffers(p_Vid);

    if (!p_Vid->no_output_of_prior_pics_flag)
    {
      flush_dpb(p_Vid);
    }
    init_dpb(p_Vid);

    ercInit(p_Vid, p_Vid->width, p_Vid->height, 1);
		image_cache_set_dimensions(&p_Vid->image_cache[0], p_Vid->width, p_Vid->height);
		image_cache_set_dimensions(&p_Vid->image_cache[1], p_Vid->width_cr, p_Vid->height_cr);
		motion_cache_set_dimensions(&p_Vid->motion_cache, p_Vid->width / BLOCK_SIZE, p_Vid->height / BLOCK_SIZE);
		
  }
  
  reset_format_info(sps, p_Vid, &p_Inp->output);

}

void activate_pps(VideoParameters *p_Vid, pic_parameter_set_rbsp_t *pps)
{  
  if (p_Vid->active_pps != pps)
  {
    if (p_Vid->dec_picture)
    {
      // this may only happen on slice loss
      exit_picture(p_Vid, &p_Vid->dec_picture);
    }

    p_Vid->active_pps = pps;

    // Fidelity Range Extensions stuff (part 2)
    p_Vid->Transform8x8Mode = pps->transform_8x8_mode_flag;

  }
}

void UseParameterSet (Slice *currSlice, int PicParsetId)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  seq_parameter_set_rbsp_t *sps = &p_Vid->SeqParSet[p_Vid->PicParSet[PicParsetId].seq_parameter_set_id];
  pic_parameter_set_rbsp_t *pps = &p_Vid->PicParSet[PicParsetId];

  if (p_Vid->PicParSet[PicParsetId].Valid != TRUE)
    printf ("Trying to use an invalid (uninitialized) Picture Parameter Set with ID %d, expect the unexpected...\n", PicParsetId);
  if (p_Vid->SeqParSet[p_Vid->PicParSet[PicParsetId].seq_parameter_set_id].Valid != TRUE)
    printf ("PicParset %d references an invalid (uninitialized) Sequence Parameter Set with ID %d, expect the unexpected...\n", PicParsetId, (int) p_Vid->PicParSet[PicParsetId].seq_parameter_set_id);

  sps =  &p_Vid->SeqParSet[p_Vid->PicParSet[PicParsetId].seq_parameter_set_id];


  // In theory, and with a well-designed software, the lines above
  // are everything necessary.  In practice, we need to patch many values
  // in p_Vid-> (but no more in p_Inp-> -- these have been taken care of)

  // Sequence Parameter Set Stuff first

//  printf ("Using Picture Parameter set %d and associated Sequence Parameter Set %d\n", PicParsetId, p_Vid->PicParSet[PicParsetId].seq_parameter_set_id);

  if ((int) sps->pic_order_cnt_type < 0 || sps->pic_order_cnt_type > 2)  // != 1
  {
    printf ("invalid sps->pic_order_cnt_type = %d\n", (int) sps->pic_order_cnt_type);
    error ("pic_order_cnt_type != 1", -1000);
  }

  if (sps->pic_order_cnt_type == 1)
  {
    if(sps->num_ref_frames_in_pic_order_cnt_cycle >= MAXnum_ref_frames_in_pic_order_cnt_cycle)
    {
      error("num_ref_frames_in_pic_order_cnt_cycle too large",-1011);
    }
  }

  activate_sps(p_Vid, sps);
  activate_pps(p_Vid, pps);

  // currSlice->dp_mode is set by read_new_slice (NALU first byte available there)
  if (pps->entropy_coding_mode_flag == CAVLC)
  {
    currSlice->nal_startcode_follows = uvlc_startcode_follows;
  }
  else
  {
    currSlice->nal_startcode_follows = cabac_startcode_follows;
  }
}

