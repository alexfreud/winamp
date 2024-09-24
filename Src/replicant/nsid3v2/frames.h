#pragma once
#include "foundation/types.h"
#ifdef __cplusplus
extern "C" {
#endif

#define FRAMEID(__frame_id) ((const int8_t*)__frame_id)
	
typedef struct frameid_struct_t
{
	const int8_t *v2;
	const int8_t *v3;
	const int8_t *v4;
} FrameID;

extern const FrameID frame_ids[];

int ValidFrameID(int frame_id);
#ifdef __cplusplus
}
#endif
