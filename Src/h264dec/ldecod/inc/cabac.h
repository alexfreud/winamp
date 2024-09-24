
/*!
 ***************************************************************************
 * \file
 *    cabac.h
 *
 * \brief
 *    Header file for entropy coding routines
 *
 * \author
 *    Detlev Marpe                                                         \n
 *    Copyright (C) 2000 HEINRICH HERTZ INSTITUTE All Rights Reserved.
 *
 * \date
 *    21. Oct 2000 (Changes by Tobias Oelbaum 28.08.2001)
 ***************************************************************************
 */

#ifndef _CABAC_H_
#define _CABAC_H_

#include "global.h"

typedef struct Run_Level
{
	int level;
	int run;
} RunLevel;
extern MotionInfoContexts*  create_contexts_MotionInfo(void);
extern TextureInfoContexts* create_contexts_TextureInfo(void);
extern void delete_contexts_MotionInfo(MotionInfoContexts *enco_ctx);
extern void delete_contexts_TextureInfo(TextureInfoContexts *enco_ctx);

extern void cabac_new_slice(Slice *currSlice);

extern int readMB_typeInfo_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp);
extern int readB8_typeInfo_CABAC(Slice *currSlice, DecodingEnvironmentPtr dep_dp);
extern int readIntraPredMode_CABAC(Slice *currSlice, DecodingEnvironmentPtr dep_dp);
extern char readRefFrame_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp, int list, int x, int y);
extern char readRefFrame_CABAC0(Macroblock *currMB, DecodingEnvironmentPtr dep_dp, int list, int y);
extern int readMVD_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp, int k, int list, int x, int y);
extern int readCBP_CABAC                   (Macroblock *currMB, DecodingEnvironmentPtr dep_dp);

// readRunLevel_CABAC returns level and sets *run
extern RunLevel readRunLevel_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp, int context);
extern short readDquant_CABAC(Slice *currSlice, DecodingEnvironmentPtr dep_dp);
extern char readCIPredMode_CABAC            (Macroblock *currMB, DecodingEnvironmentPtr dep_dp);
extern int readMB_skip_flagInfo_CABAC      (Macroblock *currMB, DecodingEnvironmentPtr dep_dp);
extern Boolean readFieldModeInfo_CABAC         (Macroblock *currMB, DecodingEnvironmentPtr dep_dp);
extern Boolean readMB_transform_size_flag_CABAC(Macroblock *currMB, DecodingEnvironmentPtr dep_dp);

extern void readIPCM_CABAC(Slice *currSlice, struct datapartition *dP);

extern int  cabac_startcode_follows(Slice *currSlice, int eos_bit);


extern int check_next_mb_and_get_field_mode_CABAC(Slice *currSlice, DataPartition  *act_dp);

extern void CheckAvailabilityOfNeighborsCABAC(Macroblock *currMB);

extern void set_read_and_store_CBP(Macroblock **currMB, int chroma_format_idc);

#endif  // _CABAC_H_

