
/*!
 ************************************************************************
 * \file  memalloc.h
 *
 * \brief
 *    Memory allocation and free helper funtions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 *
 ************************************************************************
 */

#ifndef _MEMALLOC_H_
#define _MEMALLOC_H_

#include "global.h"
#include "quant_params.h"

#if defined(USEMMX) // && (IMGTYPE == 0) // MMX, SSE, SSE2 intrinsic support
#if defined(_MSC_VER) || defined(__INTEL_COMPILER) // ICC
#  include <emmintrin.h>
# else
#  include <xmmintrin.h>
# endif
#endif

extern int  get_mem2D(byte ***array2D, int dim0, int dim1);
extern int  get_mem3D(byte ****array3D, int dim0, int dim1, int dim2);
extern int  get_mem4D(byte *****array4D, int dim0, int dim1, int dim2, int dim3);

extern int  get_mem2Dint(int ***array2D, int rows, int columns);
extern int  get_mem3Dint(int ****array3D, int frames, int rows, int columns);
extern int  get_mem4Dint(int *****array4D, int idx, int frames, int rows, int columns );

extern int  get_mem2DPicMotion(struct pic_motion ***array3D, int rows, int columns);
extern int  get_mem3Dref(h264_ref_t ****array3D, int frames, int rows, int columns);

extern int  get_mem2Dshort(short ***array2D, int dim0, int dim1);
extern MotionVector ***get_mem3DMotionVector(int dim0, int dim1, int dim2);
extern int  get_mem4Dshort(short *****array4D, int dim0, int dim1, int dim2, int dim3);
extern int  get_mem2Dpel(imgpel ***array2D, int rows, int columns);

extern struct video_image *get_memImage(int width, int height);
extern void free_memImage(struct video_image *image);

extern void free_mem2D     (byte      **array2D);
extern void free_mem3D     (byte     ***array3D);
extern void free_mem4D     (byte    ****array4D);
//
extern void free_mem2Dint  (int       **array2D);
extern void free_mem3Dint  (int      ***array3D);

extern void free_mem3Dref(h264_ref_t    ***array3D);
extern void free_mem2DPicMotion(struct pic_motion    **array3D);
//
extern void free_mem2Dshort(short      **array2D);

extern void free_mem3DMotionVector(MotionVector ***);

extern void free_mem2Dpel  (imgpel    **array2D);
extern int init_top_bot_planes(imgpel **imgFrame, int height, imgpel ***imgTopField, imgpel ***imgBotField);
extern void free_top_bot_planes(imgpel **imgTopField, imgpel **imgBotField);

extern void no_mem_exit(char *where);


#endif
