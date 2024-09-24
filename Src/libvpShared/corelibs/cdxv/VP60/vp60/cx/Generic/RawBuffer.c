/****************************************************************************
*
*   Module Title :     RawBuffer.c
*
*   Description  :     Functions to handle bit-wise writing to raw buffer.
*
****************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
*  Header Files
****************************************************************************/
#include "RawBuffer.h"
#include "codec_common.h"
/****************************************************************************
 * 
 *  ROUTINE       : WriteLongToBuffer
 *
 *  INPUTS        : RAW_BUFFER *buf : Pointer to the buffer instance to be written to.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Writes the 32-bits of buf->DataBlock into the byte
 *                  buffer buf->Buffer in big-endian format.
 *
 *  SPECIAL NOTES : None.
 *
 ***************************************************************************/
INLINE
void WriteLongToBuffer ( RAW_BUFFER *buf )
{
    buf->Buffer[buf->pos++] = (buf->DataBlock>>24);
    buf->Buffer[buf->pos++] = (buf->DataBlock>>16) & 0x000000FF;
	buf->Buffer[buf->pos++] = (buf->DataBlock>> 8) & 0x000000FF;
	buf->Buffer[buf->pos++] =  buf->DataBlock      & 0x000000FF;
}

/****************************************************************************
 * 
 *  ROUTINE       : InitAddRawBitsToBuffer
 *
 *  INPUTS        : RAW_BUFFER *buf : Pointer to the buffer instance to be written to.
 *                  UINT8 *Buffer   : Array to be used by RAW_BUFFER to write to.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Initializes a Raw Buffer instance given a pointer to an
 *                  array of UINT8s to be used as the storage buffer.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void InitAddRawBitsToBuffer ( RAW_BUFFER *buf, UINT8 *Buffer )
{                      
    buf->Buffer          = Buffer;
    buf->byte_bit_offset = 32;
    buf->DataBlock       = 0;
	buf->pos             = 0;
}

/****************************************************************************
 * 
 *  ROUTINE       : AddRawBitsToBuffer
 *
 *  INPUTS        : RAW_BUFFER *buf : Pointer to the buffer instance to be written to.
 *                  UINT32 data     : Bit pattern to be written to the buffer.
 *                  UINT32 bits     : Number of significant bits of data to write.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Writes data to the buffer to the specified number of bits
 *                  (UINT32 bits).
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void AddRawBitsToBuffer( RAW_BUFFER *buf, UINT32 data, UINT32 bits )
{
	// how many bits  should we shift by?
	buf->byte_bit_offset -= bits;

	if ( buf->byte_bit_offset < 0 )
	{
		// only write the left most bits in this datablock
		buf->DataBlock |= (data >> (-buf->byte_bit_offset));

		// output block 
		WriteLongToBuffer ( buf );
		buf->DataBlock = 0;
		buf->byte_bit_offset += 32;
	}
	// note we may have bits getting shifted off the left side (like in above case)
	buf->DataBlock |= (data << buf->byte_bit_offset);
}

/****************************************************************************
 * 
 *  ROUTINE       : EndAddRawBitsToBuffer
 *
 *  INPUTS        : RAW_BUFFER *buf : Pointer to the buffer instance to be written to.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Finalizes all writes to the buffer.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void EndAddRawBitsToBuffer ( RAW_BUFFER *buf )
{      
	UINT8 shift = 24;

	while ( buf->byte_bit_offset < 32 )
	{
		buf->Buffer[buf->pos++] = (buf->DataBlock>>shift) & 0xff;
		shift -= 8;
		buf->byte_bit_offset += 8;
	}

    buf->byte_bit_offset = 32;
    buf->DataBlock = 0;
}
