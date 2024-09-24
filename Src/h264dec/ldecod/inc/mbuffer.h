
/*!
 ***********************************************************************
 *  \file
 *      mbuffer.h
 *
 *  \brief
 *      Frame buffer functions
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Karsten Sühring          <suehring@hhi.de>
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 
 *      - Jill Boyce               <jill.boyce@thomson.net>
 *      - Saurav K Bandyopadhyay   <saurav@ieee.org>
 *      - Zhenyu Wu                <Zhenyu.Wu@thomson.net
 *      - Purvin Pandit            <Purvin.Pandit@thomson.net>
 *
 ***********************************************************************
 */
#ifndef _MBUFFER_H_
#define _MBUFFER_H_

#include "global.h"
#include <bfc/platform/types.h>

#define MAX_LIST_SIZE 33
//! definition of pic motion parameters

typedef struct pic_motion_params
{
	PicMotion **motion[2];
	h264_ref_t ***field_references;
  byte *      mb_field;      //!< field macroblock indicator
  byte **     field_frame;   //!< indicates if co_located is field or frame.
	int padding[1];
} PicMotionParams;

typedef struct video_image
{
	imgpel **img;
	imgpel *base_address;
	size_t stride;
	struct video_image *next; // for the memory cacher
} VideoImage;
//! definition a picture (field or frame)
typedef struct storable_picture
{
  PictureStructure structure;

  int         poc;
  int         top_poc;
  int         bottom_poc;
  int         frame_poc;
  h264_ref_t       ref_pic_num        [MAX_NUM_SLICES][6][MAX_LIST_SIZE];
  h264_ref_t       frm_ref_pic_num    [MAX_NUM_SLICES][6][MAX_LIST_SIZE];
  h264_ref_t       top_ref_pic_num    [MAX_NUM_SLICES][6][MAX_LIST_SIZE];
  h264_ref_t       bottom_ref_pic_num [MAX_NUM_SLICES][6][MAX_LIST_SIZE];
  unsigned    frame_num;
  unsigned    recovery_frame;

  int         pic_num;
  int         long_term_pic_num;
  int         long_term_frame_idx;

  byte        is_long_term;
  int         used_for_reference;
  int         is_output;
  int         non_existing;

  short       max_slice_id;

  int         size_x, size_y, size_x_cr, size_y_cr;
  int         size_x_m1, size_y_m1, size_x_cr_m1, size_y_cr_m1;
  int         chroma_vector_adjustment;
  int         coded_frame;
  int         mb_aff_frame_flag;
  unsigned    PicWidthInMbs;
  unsigned    PicSizeInMbs;

  //imgpel **     imgY;         //!< Y picture component
	union
	{
		VideoImage *plane_images[3]; // to ensure array alignment
		struct
		{
			VideoImage *imgY;
			VideoImage *imgUV[2];        //!< U and V picture components
		};
	};
  
  struct pic_motion_params motion;              //!< Motion info
  struct pic_motion_params JVmotion[MAX_PLANE]; //!< Motion info for 4:4:4 independent mode decoding

  short **     slice_id;      //!< reference picture   [mb_x][mb_y]

  struct storable_picture *top_field;     // for mb aff, if frame for referencing the top field
  struct storable_picture *bottom_field;  // for mb aff, if frame for referencing the bottom field
  struct storable_picture *frame;         // for mb aff, if field for referencing the combined frame

  int         slice_type;
  int         idr_flag;
  int         no_output_of_prior_pics_flag;
  int         long_term_reference_flag;
  int         adaptive_ref_pic_buffering_flag;

  int         chroma_format_idc;
  int         frame_mbs_only_flag;
  int         frame_cropping_flag;
  int         frame_cropping_rect_left_offset;
  int         frame_cropping_rect_right_offset;
  int         frame_cropping_rect_top_offset;
  int         frame_cropping_rect_bottom_offset;
  int         qp;
  int         chroma_qp_offset[2];
  int         slice_qp_delta;
  DecRefPicMarking_t *dec_ref_pic_marking_buffer;                    //!< stores the memory management control operations

  // picture error concealment
  int         concealed_pic; //indicates if this is a concealed picture
  
  // variables for tone mapping
  int         seiHasTone_mapping;
  int         tone_mapping_model_id;
  int         tonemapped_bit_depth;  
  imgpel*     tone_mapping_lut;                //!< tone mapping look up table

	int retain_count; // benski> we're going to reference count these things
	uint64_t time_code; // user-passed timecode for this frame
} StorablePicture;

//! definition a picture (field or frame)
typedef struct colocated_params
{
  int         mb_adaptive_frame_field_flag;
  int         size_x, size_y;
  byte        is_long_term;

  MotionParams frame;
  MotionParams top;
  MotionParams bottom;

} ColocatedParams;

//! Frame Stores for Decoded Picture Buffer
typedef struct frame_store
{
  int       is_used;                //!< 0=empty; 1=top; 2=bottom; 3=both fields (or frame)
  int       is_reference;           //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
  int       is_long_term;           //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
  int       is_orig_reference;      //!< original marking by nal_ref_idc: 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used

  int       is_non_existent;

  unsigned  frame_num;
  unsigned  recovery_frame;

  int       frame_num_wrap;
  int       long_term_frame_idx;
  int       is_output;
  int       poc;

  // picture error concealment
  int concealment_reference;

  StorablePicture *frame;
  StorablePicture *top_field;
  StorablePicture *bottom_field;

} FrameStore;


//! Decoded Picture Buffer
typedef struct decoded_picture_buffer
{
  VideoParameters *p_Vid;
  InputParameters *p_Inp;
  FrameStore  **fs;
  FrameStore  **fs_ref;
  FrameStore  **fs_ltref;
  unsigned      size;
  unsigned      used_size;
  unsigned      ref_frames_in_buffer;
  unsigned      ltref_frames_in_buffer;
  int           last_output_poc;
  int           max_long_term_pic_idx;

  int           init_done;
  int           num_ref_frames;

  FrameStore   *last_picture;
} DecodedPictureBuffer;

extern void             init_dpb(VideoParameters *p_Vid);
extern void             free_dpb(VideoParameters *p_Vid);
extern FrameStore*      alloc_frame_store(void);
extern void             free_frame_store(VideoParameters *p_Vid, FrameStore* f);
extern StorablePicture* alloc_storable_picture(VideoParameters *p_Vid, PictureStructure type, int size_x, int size_y, int size_x_cr, int size_y_cr);
extern void             free_storable_picture(VideoParameters *p_Vid, StorablePicture* p);
extern void             store_picture_in_dpb(VideoParameters *p_Vid, StorablePicture* p);
extern void             flush_dpb(VideoParameters *p_Vid);

extern void             dpb_split_field  (VideoParameters *p_Vid, FrameStore *fs);
extern void             dpb_combine_field(VideoParameters *p_Vid, FrameStore *fs);
extern void             dpb_combine_field_yuv(VideoParameters *p_Vid, FrameStore *fs);

extern void             init_lists          (Slice *currSlice);
extern void             reorder_ref_pic_list(VideoParameters *p_Vid, StorablePicture **list, char *list_size,
                                      int num_ref_idx_lX_active_minus1, int *reordering_of_pic_nums_idc,
                                      int *abs_diff_pic_num_minus1, int *long_term_pic_idx);

extern void             init_mbaff_lists(VideoParameters *p_Vid);
extern void             alloc_ref_pic_list_reordering_buffer(Slice *currSlice);
extern void             free_ref_pic_list_reordering_buffer(Slice *currSlice);

extern void             fill_frame_num_gap(VideoParameters *p_Vid);

extern ColocatedParams* alloc_colocated(VideoParameters *p_Vid, int size_x, int size_y,int mb_adaptive_frame_field_flag);
extern void free_colocated(VideoParameters *p_Vid, ColocatedParams* p);
extern void compute_colocated            (Slice *currSlice, ColocatedParams* p, StorablePicture **listX[6]);
extern void compute_colocated_frames_mbs (Slice *currSlice, ColocatedParams* p, StorablePicture **listX[6]);

// For 4:4:4 independent mode
extern void compute_colocated_JV  ( Slice *currSlice, ColocatedParams* p, StorablePicture **listX[6]);
extern void copy_storable_param_JV( VideoParameters *p_Vid, PicMotionParams  *JVplane, PicMotionParams  *motion );

// benski> decoded output pictures
void out_storable_picture_get(VideoParameters *img, StorablePicture **pic);
void out_storable_picture_add(VideoParameters *img, StorablePicture *pic);
void out_storable_pictures_init(VideoParameters *img, size_t count);
void out_storable_pictures_destroy(VideoParameters *img);

#endif

