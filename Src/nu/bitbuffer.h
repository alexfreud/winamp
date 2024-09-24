#ifndef NULLSOFT_UTILITY_BITBUFFER_H
#define NULLSOFT_UTILITY_BITBUFFER_H
#include <stddef.h>
#ifdef _WIN32
#include <stddef.h>
#else
#include <inttypes.h>
#endif
class BitBuffer
{
public:
  BitBuffer();
  void WriteBit(char bit);
  void WriteBits(uintptr_t num, size_t bitlen);
  void WriteBytes(void *buffer, size_t bytes);
	void WriteByte(unsigned char byte);
  unsigned char *Get() { return buffer; }
	size_t GetLength() { return length; }
  
private:
  void Resize(size_t newlen);
  unsigned char *buffer;
  size_t length;
  size_t bits;
};
#endif