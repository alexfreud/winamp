/**********************************************************************

 * (c) 1996 Voxware, Inc.  All Rights Reserved.  This file contains

 * proprietary information of trade secrets of Voxware, Inc. and may

 * not be distributed without the authorization of Voxware, Inc.

 **********************************************************************

 *

 *	File:			VOXCHUNK.H

 *	Purpose:		Types and #defines which describe the handling of

 *					the INFO chunks in the vox files.

 *	Created:		5/29/96 - George T. Talbot

 *	Modified:		6/17/96 - GTT - Added code to set lengths to -1 on

 *									the VOXCHUNK_SET() macro so that

 *									the default behaviour is to read

 *									existing info data.

 *	Notes:

 *

 **********************************************************************/



#ifndef _VOXCHUNK_H_

#define _VOXCHUNK_H_

typedef struct tagVox_Chunk_Info

{

	unsigned long		id;

	long				length;

	char				info[256];

} VOX_CHUNK_INFO;



// Important note:  When adding a new VOXCHUNK_xxx, make sure to add

// the cooresponding elements to the gRIFF_VoxChunk_IDs[] and

// gAIFF_VoxChunk_IDs[] arrays in INFOCHNK.C



#define	VOXCHUNK_NONE		0

#define	VOXCHUNK_URL		1

#define	VOXCHUNK_COPYRIGHT	2

#define	VOXCHUNK_TITLE		3

#define	VOXCHUNK_SOFTWARE	4

#define	VOXCHUNK_AUTHOR		5

#define	VOXCHUNK_COMMENT	6



#define	VOXCHUNK_NUM_KINDS	7



// Convenience macros...



// Clear out a set of VOX_CHUNK_INFO records for use...

//

//	Parameters are:  	_zz1	- Pointer to array of VOX_CHUNK_INFO.

//						_zz2	- Number of elements in the array.



#define	VOXCHUNK_SET(_zz1, _zz2)	{														\
										memset((_zz1), 0, (_zz2) * sizeof(VOX_CHUNK_INFO));	\
										for (short i=0; i<(_zz2); ++i)						\
										{													\
											(_zz1)[i].length	= -1;						\
										}													\
									}



// Write a C string into a VOX_CHUNK_INFO record...

//

//	Parameters are:		_zz1	- VOX_CHUNK_INFO record

//						_zz2	- Chunk ID from above list

//						_zz3	- A C string



#define	VOXCHUNK_INFO(_zz1, _zz2, _zz3)											\
								{												\
									(_zz1).id		= (_zz2);					\
									(_zz1).length	= strlen(_zz3)+1;			\
									memcpy((_zz1).info, (_zz3), (_zz1).length);	\
								}

#endif

