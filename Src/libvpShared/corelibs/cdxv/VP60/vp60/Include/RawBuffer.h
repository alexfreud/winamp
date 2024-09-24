/****************************************************************************
*
*   Module Title :     RAW_BUFFER.h
*
*   Description  :     Raw bit manipulation routines header file.
*
****************************************************************************/
#ifndef __INC_RAWBUFFER_H
#define __INC_RAWBUFFER_H

/****************************************************************************
*   Header Files
****************************************************************************/
#include "type_aliases.h"

/****************************************************************************
*   Typedefs
****************************************************************************/
typedef struct RAW_BUFFER
{
    UINT32 pos;                  // Offset of "current" UINT32 in buffer
    INT32  byte_bit_offset;      // Offset of next free bit in current UINT8
    UINT32 DataBlock;
    UINT8 *Buffer;
} RAW_BUFFER;

/****************************************************************************
*  Exports
****************************************************************************/
extern void InitAddRawBitsToBuffer ( RAW_BUFFER *buf, UINT8 *Buffer );
extern void AddRawBitsToBuffer( RAW_BUFFER *buf, UINT32 data, UINT32 bits );
extern void EndAddRawBitsToBuffer( RAW_BUFFER *buf );

#endif
