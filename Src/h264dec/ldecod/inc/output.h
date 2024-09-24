
/*!
 **************************************************************************************
 * \file
 *    output.h
 * \brief
 *    Picture writing routine headers
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Karsten Suehring        <suehring@hhi.de>
 ***************************************************************************************
 */

#ifndef _OUTPUT_H_
#define _OUTPUT_H_


extern void write_stored_frame(VideoParameters *p_Vid, FrameStore *fs);
extern void direct_output     (VideoParameters *p_Vid, StorablePicture *p);
extern void init_out_buffer   (VideoParameters *p_Vid);
extern void uninit_out_buffer (VideoParameters *p_Vid);

#if (PAIR_FIELDS_IN_OUTPUT)
extern void flush_pending_output(VideoParameters *p_Vid);
#endif

#endif //_OUTPUT_H_
