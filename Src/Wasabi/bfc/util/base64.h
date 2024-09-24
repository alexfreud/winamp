#ifndef _BASE64_H
#define _BASE64_H

#include <bfc/memblock.h>

class Base64 {
public:
	static int decode(MemBlock<wchar_t> &h64, MemBlock<char> &htext);
	static int decode(MemBlock<char> &h64, MemBlock<char> &htext);
	static int encode(MemBlock<char> &htext, MemBlock<char> &h64, int linelength);
	static int encode(MemBlock<char> &htext, MemBlock<wchar_t> &h64, int linelength);
private:
  static char encodingTable[64];
};

#endif
