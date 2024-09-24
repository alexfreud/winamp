
/*!
 **************************************************************************************
 * \file
 *    parsetcommon.h
 * \brief
 *    Picture and Sequence Parameter Sets, structures common to encoder and decoder
 *
 * \date 25 November 2002
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Stephan Wenger        <stewe@cs.tu-berlin.de>
 ***************************************************************************************
 */



// In the JVT syntax, frequently flags are used that indicate the presence of
// certain pieces of information in the NALU.  Here, these flags are also
// present.  In the encoder, those bits indicate that the values signaled to
// be present are meaningful and that this part of the syntax should be
// written to the NALU.  In the decoder, the flag indicates that information
// was received from the decoded NALU and should be used henceforth.
// The structure names were chosen as indicated in the JVT syntax

#ifndef _PARSETCOMMON_H_
#define _PARSETCOMMON_H_

#include "defines.h"

#define MAXIMUMPARSETRBSPSIZE   1500
#define MAXIMUMPARSETNALUSIZE   1500

#define MAXSPS  32
#define MAXPPS  256

#define MAXIMUMVALUEOFcpb_cnt   32
typedef struct
{
  unsigned int cpb_cnt_minus1;                                   // ue(v)
  unsigned int bit_rate_scale;                                   // u(4)
  unsigned int cpb_size_scale;                                   // u(4)
  unsigned int bit_rate_value_minus1 [MAXIMUMVALUEOFcpb_cnt];    // ue(v)
  unsigned int cpb_size_value_minus1 [MAXIMUMVALUEOFcpb_cnt];    // ue(v)
  unsigned int cbr_flag              [MAXIMUMVALUEOFcpb_cnt];    // u(1)
  unsigned int initial_cpb_removal_delay_length_minus1;          // u(5)
  unsigned int cpb_removal_delay_length_minus1;                  // u(5)
  unsigned int dpb_output_delay_length_minus1;                   // u(5)
  unsigned int time_offset_length;                               // u(5)
} hrd_parameters_t;


typedef struct
{
  Boolean      aspect_ratio_info_present_flag;                   // u(1)
  unsigned int aspect_ratio_idc;                                 // u(8)
  unsigned short sar_width;                                        // u(16)
  unsigned short sar_height;                                       // u(16)
  Boolean      overscan_info_present_flag;                       // u(1)
  Boolean      overscan_appropriate_flag;                        // u(1)
  Boolean      video_signal_type_present_flag;                   // u(1)
  unsigned int video_format;                                     // u(3)
  Boolean      video_full_range_flag;                            // u(1)
  Boolean      colour_description_present_flag;                  // u(1)
  unsigned int colour_primaries;                                 // u(8)
  unsigned int transfer_characteristics;                         // u(8)
  unsigned int matrix_coefficients;                              // u(8)
  Boolean      chroma_location_info_present_flag;                // u(1)
  unsigned int  chroma_sample_loc_type_top_field;                // ue(v)
  unsigned int  chroma_sample_loc_type_bottom_field;             // ue(v)
  Boolean      timing_info_present_flag;                         // u(1)
  unsigned int num_units_in_tick;                                // u(32)
  unsigned int time_scale;                                       // u(32)
  Boolean      fixed_frame_rate_flag;                            // u(1)
  Boolean      nal_hrd_parameters_present_flag;                  // u(1)
  hrd_parameters_t nal_hrd_parameters;                           // hrd_paramters_t
  Boolean      vcl_hrd_parameters_present_flag;                  // u(1)
  hrd_parameters_t vcl_hrd_parameters;                           // hrd_paramters_t
  // if ((nal_hrd_parameters_present_flag || (vcl_hrd_parameters_present_flag))
  Boolean      low_delay_hrd_flag;                               // u(1)
  Boolean      pic_struct_present_flag;                          // u(1)
  Boolean      bitstream_restriction_flag;                       // u(1)
  Boolean      motion_vectors_over_pic_boundaries_flag;          // u(1)
  unsigned int max_bytes_per_pic_denom;                          // ue(v)
  unsigned int max_bits_per_mb_denom;                            // ue(v)
  unsigned int log2_max_mv_length_vertical;                      // ue(v)
  unsigned int log2_max_mv_length_horizontal;                    // ue(v)
  unsigned int num_reorder_frames;                               // ue(v)
  unsigned int max_dec_frame_buffering;                          // ue(v)
} vui_seq_parameters_t;


#define MAXnum_slice_groups_minus1  8
typedef struct
{
  Boolean   Valid;                  // indicates the parameter set is valid
  unsigned int pic_parameter_set_id;                             // ue(v)
  unsigned int seq_parameter_set_id;                             // ue(v)
  Boolean   entropy_coding_mode_flag;                            // u(1)
  Boolean   transform_8x8_mode_flag;                             // u(1)

  Boolean   pic_scaling_matrix_present_flag;                     // u(1)
  int       pic_scaling_list_present_flag[12];                   // u(1)
  int       ScalingList4x4[6][16];                               // se(v)
  int       ScalingList8x8[6][64];                               // se(v)
  Boolean   UseDefaultScalingMatrix4x4Flag[6];
  Boolean   UseDefaultScalingMatrix8x8Flag[6];

  // if( pic_order_cnt_type < 2 )  in the sequence parameter set
  Boolean      bottom_field_pic_order_in_frame_present_flag;                           // u(1)
  unsigned int num_slice_groups_minus1;                          // ue(v)
  unsigned int slice_group_map_type;                        // ue(v)
  // if( slice_group_map_type = = 0 )
  unsigned int run_length_minus1[MAXnum_slice_groups_minus1]; // ue(v)
  // else if( slice_group_map_type = = 2 )
  unsigned int top_left[MAXnum_slice_groups_minus1];         // ue(v)
  unsigned int bottom_right[MAXnum_slice_groups_minus1];     // ue(v)
  // else if( slice_group_map_type = = 3 || 4 || 5
  Boolean   slice_group_change_direction_flag;            // u(1)
  unsigned int slice_group_change_rate_minus1;               // ue(v)
  // else if( slice_group_map_type = = 6 )
  unsigned int pic_size_in_map_units_minus1;             // ue(v)
  byte      *slice_group_id;                              // complete MBAmap u(v)

  int num_ref_idx_l0_active_minus1;                     // ue(v)
  int num_ref_idx_l1_active_minus1;                     // ue(v)
  Boolean   weighted_pred_flag;                               // u(1)
  unsigned int  weighted_bipred_idc;                              // u(2)
  int       pic_init_qp_minus26;                              // se(v)
  int       pic_init_qs_minus26;                              // se(v)
  int       chroma_qp_index_offset;                           // se(v)

  int       second_chroma_qp_index_offset;                    // se(v)

  Boolean   deblocking_filter_control_present_flag;           // u(1)
  Boolean   constrained_intra_pred_flag;                      // u(1)
  Boolean   redundant_pic_cnt_present_flag;                   // u(1)
} pic_parameter_set_rbsp_t;


#define MAXnum_ref_frames_in_pic_order_cnt_cycle  256
typedef struct
{
  Boolean   Valid;                  // indicates the parameter set is valid

  unsigned int profile_idc;                                       // u(8)
  Boolean   constrained_set0_flag;                                // u(1)
  Boolean   constrained_set1_flag;                                // u(1)
  Boolean   constrained_set2_flag;                                // u(1)
  Boolean   constrained_set3_flag;                                // u(1)
  unsigned  int level_idc;                                        // u(8)
  unsigned  int seq_parameter_set_id;                             // ue(v)
  unsigned  int chroma_format_idc;                                // ue(v)

  Boolean   seq_scaling_matrix_present_flag;                   // u(1)
  int       seq_scaling_list_present_flag[12];                 // u(1)
  int       ScalingList4x4[6][16];                             // se(v)
  int       ScalingList8x8[6][64];                             // se(v)
  Boolean   UseDefaultScalingMatrix4x4Flag[6];
  Boolean   UseDefaultScalingMatrix8x8Flag[6];

  unsigned int bit_depth_luma_minus8;                            // ue(v)
  unsigned int bit_depth_chroma_minus8;                          // ue(v)
  unsigned int log2_max_frame_num_minus4;                        // ue(v)
  unsigned int pic_order_cnt_type;
  // if( pic_order_cnt_type == 0 )
  unsigned int log2_max_pic_order_cnt_lsb_minus4;                 // ue(v)
  // else if( pic_order_cnt_type == 1 )
    Boolean delta_pic_order_always_zero_flag;               // u(1)
    int     offset_for_non_ref_pic;                         // se(v)
    int     offset_for_top_to_bottom_field;                 // se(v)
    unsigned int num_ref_frames_in_pic_order_cnt_cycle;          // ue(v)
    // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
      int   offset_for_ref_frame[MAXnum_ref_frames_in_pic_order_cnt_cycle];   // se(v)
  unsigned int num_ref_frames;                                   // ue(v)
  Boolean   gaps_in_frame_num_value_allowed_flag;             // u(1)
  unsigned int pic_width_in_mbs_minus1;                          // ue(v)
  unsigned int pic_height_in_map_units_minus1;                   // ue(v)
  Boolean   frame_mbs_only_flag;                              // u(1)
  // if( !frame_mbs_only_flag )
    Boolean   mb_adaptive_frame_field_flag;                   // u(1)
  Boolean   direct_8x8_inference_flag;                        // u(1)
  Boolean   frame_cropping_flag;                              // u(1)
    unsigned int frame_cropping_rect_left_offset;                // ue(v)
    unsigned int frame_cropping_rect_right_offset;               // ue(v)
    unsigned int frame_cropping_rect_top_offset;                 // ue(v)
    unsigned int frame_cropping_rect_bottom_offset;              // ue(v)
  Boolean   vui_parameters_present_flag;                      // u(1)
    vui_seq_parameters_t vui_seq_parameters;                  // vui_seq_parameters_t
    unsigned  separate_colour_plane_flag;                       // u(1)
} seq_parameter_set_rbsp_t;

pic_parameter_set_rbsp_t *AllocPPS (void);
seq_parameter_set_rbsp_t *AllocSPS (void);

void FreePPS (pic_parameter_set_rbsp_t *pps);
void FreeSPS (seq_parameter_set_rbsp_t *sps);

int sps_is_equal(seq_parameter_set_rbsp_t *sps1, seq_parameter_set_rbsp_t *sps2);
int pps_is_equal(pic_parameter_set_rbsp_t *pps1, pic_parameter_set_rbsp_t *pps2);

#endif
