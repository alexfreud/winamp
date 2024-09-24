#include "BitReader.h"
#include <string.h>

static uint32_t mask[8]=
{
		0x1,
		0x3,
		0x7,
		0xF,
		0x1F,
		0x3F,
		0x7F,
		0xFF
};

static uint32_t msk[33] =
{
  0x00000000,0x00000001,0x00000003,0x00000007,
  0x0000000f,0x0000001f,0x0000003f,0x0000007f,
  0x000000ff,0x000001ff,0x000003ff,0x000007ff,
  0x00000fff,0x00001fff,0x00003fff,0x00007fff,
  0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
  0x000fffff,0x001fffff,0x003fffff,0x007fffff,
  0x00ffffff,0x01ffffff,0x03ffffff,0x07ffffff,
  0x0fffffff,0x1fffffff,0x3fffffff,0x7fffffff,
  0xffffffff
};

void BitReader::alignbyte()
{
	flushbits(numBits&7);
}

void BitReader::getbytes(void *data, uint32_t n)
{
	memcpy(data, this->data, n);
	flushbits(n*8);
}

uint8_t BitReader::getbits1()
{
	uint8_t byte = data[0];
	uint32_t count = (numBits-1) & 7;
	byte &= mask[count];
	byte >>= count;

	numBits--;
	if ((numBits % 8) == 0)
		data++;
	return byte;
}

uint32_t BitReader::getbits(uint32_t n)
{
	uint32_t val = showbits(n);
	flushbits(n);
	return val;
}

uint8_t BitReader::showbits1() const
{
	uint8_t byte = data[0];
	uint32_t count = (numBits-1) & 7;
	byte &= mask[count];
	byte >>= count;
	return byte;
}

uint32_t BitReader::showbits(uint32_t n) const
{
	uint32_t val;
	switch((numBits+7) >> 3)
	{
	case 0:
		return 0;
	case 1:
		val=(data[0]<<24);
		break;
	case 2:
		val=(data[0]<<24) | (data[1]<<16);
		break;
	case 3:
		val=(data[0]<<24) | (data[1]<<16) | (data[2]<<8);
		break;
	default:
		val=(data[0]<<24) | (data[1]<<16) | (data[2]<<8) | data[3];
		break;
	}
	uint32_t c = ((numBits-1) & 7) + 25;
	return (val>>(c-n)) & msk[n];
}

void BitReader::flushbits(uint32_t n)
{
	uint32_t oldpos = (numBits+7)>>3;
	numBits-=n;
	uint32_t newpos = (numBits+7)>>3;
	data  += (oldpos - newpos);
}

bool BitReader::empty()
{
	return numBits==0;
}

uint32_t BitReader::size() const
{
	return numBits;
}
