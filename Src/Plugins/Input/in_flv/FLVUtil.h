#ifndef NULLSOFT_FLVUTIL_H
#define NULLSOFT_FLVUTIL_H

#include <memory.h>
#include <bfc/platform/types.h>
namespace FLV
{
	// reads 32 bits from data and converts from big endian 
inline uint32_t Read32(uint8_t *data)
{
	uint32_t returnVal;
#ifdef __BIG_ENDIAN__
	returnVal = *(uint32_t  *)(&data[0]);
#else 
	// need to swap endianness	
	uint8_t *swap = (uint8_t  *)&returnVal;
	swap[0]=data[3];
	swap[1]=data[2];
	swap[2]=data[1];
	swap[3]=data[0];
#endif
	return returnVal;

}
// reads 24 bits from data, converts from big endian, and returns as a 32bit int
inline uint32_t Read24(uint8_t *data)
{
	uint32_t returnVal=0; 
	uint8_t *swap = (uint8_t  *)&returnVal;
#ifdef __BIG_ENDIAN__
	swap[1]=data[0];
	swap[2]=data[1];
	swap[3]=data[2];
	returnVal = *(uint32_t  *)(&data[0]);
#else 
	// need to swap endianness	
	swap[0]=data[2];
	swap[1]=data[1];
	swap[2]=data[0];
#endif
	return returnVal;

}

// reads 16 bits from data and converts from big endian
inline uint16_t Read16(uint8_t *data)
{
	uint16_t returnVal;
#ifdef __BIG_ENDIAN__
	returnVal = *(uint16_t  *)(&data[0]);
#else 
	// need to swap endianness	
	uint8_t *swap = (uint8_t  *)&returnVal;
	swap[0]=data[1];
	swap[1]=data[0];
#endif
	return returnVal;
}

// reads 16 bits from data and converts from big endian
inline uint8_t Read8(uint8_t *data)
{
	uint8_t returnVal;

	returnVal = *(uint8_t  *)(&data[0]);
	return returnVal;
}

// reads a double from data
inline double ReadDouble(uint8_t *data)
{
	double returnVal;
#ifdef __BIG_ENDIAN__
	memcpy(&returnVal, data, 8);
	#else 
	uint8_t *swap = (uint8_t  *)&returnVal;
	swap[0]=data[7];
	swap[1]=data[6];
	swap[2]=data[5];
	swap[3]=data[4];
	swap[4]=data[3];
	swap[5]=data[2];
	swap[6]=data[1];
	swap[7]=data[0];
	#endif
	return returnVal;
}

}
#endif