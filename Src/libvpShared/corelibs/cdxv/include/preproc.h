/****************************************************************************
*
*   Module Title :     preproc.h
*
*   Description  :     simple preprocessor
*
****************************************************************************/

#ifndef __INC_PREPROC_H
#define __INC_PREPROC_H

/****************************************************************************
*  Header Files
****************************************************************************/
#include "duck_mem.h"

/****************************************************************************
*  Types
****************************************************************************/

typedef struct 
{
	unsigned char* frameBuffer;
	int frame;
	unsigned int *fixedDivide;

	unsigned char*frameBufferAlloc;
	unsigned int *fixedDivideAlloc;
} PreProcInstance;

/****************************************************************************
*  Functions.
****************************************************************************/

void DeletePreProc( PreProcInstance *ppi);
int InitPreProc( PreProcInstance *ppi, int FrameSize);
extern void spatialFilter_c( PreProcInstance *ppi,unsigned char *s,unsigned char *d,int width,int height,int pitch,int strength);
extern void (*tempFilter)( PreProcInstance *ppi,unsigned char *s,unsigned char *d,int bytes,int strength);

#endif
