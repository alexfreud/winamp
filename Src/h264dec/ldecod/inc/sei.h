
/*!
 *************************************************************************************
 * \file sei.h
 *
 * \brief
 *    Prototypes for sei.c
 *************************************************************************************
 */

#ifndef SEI_H
#define SEI_H

typedef enum {
  SEI_BUFFERING_PERIOD = 0,
  SEI_PIC_TIMING,
  SEI_PAN_SCAN_RECT,
  SEI_FILLER_PAYLOAD,
  SEI_USER_DATA_REGISTERED_ITU_T_T35,
  SEI_USER_DATA_UNREGISTERED,
  SEI_RECOVERY_POINT,
  SEI_DEC_REF_PIC_MARKING_REPETITION,
  SEI_SPARE_PIC,
  SEI_SCENE_INFO,
  SEI_SUB_SEQ_INFO,
  SEI_SUB_SEQ_LAYER_CHARACTERISTICS,
  SEI_SUB_SEQ_CHARACTERISTICS,
  SEI_FULL_FRAME_FREEZE,
  SEI_FULL_FRAME_FREEZE_RELEASE,
  SEI_FULL_FRAME_SNAPSHOT,
  SEI_PROGRESSIVE_REFINEMENT_SEGMENT_START,
  SEI_PROGRESSIVE_REFINEMENT_SEGMENT_END,
  SEI_MOTION_CONSTRAINED_SLICE_GROUP_SET,
  SEI_FILM_GRAIN_CHARACTERISTICS,
  SEI_DEBLOCKING_FILTER_DISPLAY_PREFERENCE,
  SEI_STEREO_VIDEO_INFO,
  SEI_POST_FILTER_HINTS,
  SEI_TONE_MAPPING,

  SEI_MAX_ELEMENTS  //!< number of maximum syntax elements
} SEI_type;

#define MAX_FN 256
// tone mapping information
#define MAX_CODED_BIT_DEPTH  12
#define MAX_SEI_BIT_DEPTH    12
#define MAX_NUM_PIVOTS     (1<<MAX_CODED_BIT_DEPTH)

#if (ENABLE_OUTPUT_TONEMAPPING)
typedef struct tone_mapping_struct_s
{
  Boolean seiHasTone_mapping;
  unsigned int  tone_map_repetition_period;
  unsigned char coded_data_bit_depth;
  unsigned char sei_bit_depth;
  unsigned int  model_id;
  unsigned int count;
  
  imgpel lut[1<<MAX_CODED_BIT_DEPTH];                 //<! look up table for mapping the coded data value to output data value

  Bitstream *data;
  int payloadSize;
} ToneMappingSEI;

#endif

void InterpretSEIMessage(byte* msg, int size, VideoParameters *p_Vid);
void interpret_spare_pic( byte* payload, int size, VideoParameters *p_Vid );
void interpret_subsequence_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_subsequence_layer_characteristics_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_subsequence_characteristics_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_scene_information( byte* payload, int size, VideoParameters *p_Vid ); // JVT-D099
void interpret_user_data_registered_itu_t_t35_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_user_data_unregistered_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_pan_scan_rect_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_recovery_point_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_filler_payload_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_dec_ref_pic_marking_repetition_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_full_frame_freeze_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_full_frame_freeze_release_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_full_frame_snapshot_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_progressive_refinement_start_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_progressive_refinement_end_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_motion_constrained_slice_group_set_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_reserved_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_buffering_period_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_picture_timing_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_film_grain_characteristics_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_deblocking_filter_display_preference_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_stereo_video_info_info( byte* payload, int size, VideoParameters *p_Vid );
void interpret_post_filter_hints_info( byte* payload, int size, VideoParameters *p_Vid );
// functions for tone mapping SEI message
void interpret_tone_mapping( byte* payload, int size, VideoParameters *p_Vid );

#if (ENABLE_OUTPUT_TONEMAPPING)
void tone_map(imgpel** imgX, imgpel* lut, int size_x, int size_y);
void init_tone_mapping_sei(ToneMappingSEI *seiToneMapping);
void update_tone_mapping_sei(ToneMappingSEI *seiToneMapping);
#endif
#endif
