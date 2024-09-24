#pragma once
#include <bfc/platform/types.h>
class BitReader
{
public:
	uint8_t getbits1();
	uint32_t getbits(uint32_t n);
	uint8_t showbits1() const;
	uint32_t showbits(uint32_t n) const;
	void flushbits(uint32_t n);
	bool empty();
	uint32_t size() const; // in bits
	void alignbyte(); // aligns bitstream to the next byte (or current byte if already aligned)
	void getbytes(void *data, uint32_t n);
//private:
	const uint8_t *data;
	uint32_t numBits;
};