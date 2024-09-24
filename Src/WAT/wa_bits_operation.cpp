#include "WAT.h"


unsigned char* wa::bits_operation::GetBits(unsigned char* Source, unsigned int NbrOfBits, unsigned int* BufferSize)
{
	// check for wrong parameter
	if (Source == nullptr || NbrOfBits == 0 || BufferSize == nullptr)
		return nullptr;

	// variable
	unsigned int bitMask = 0;
	unsigned int nbrOfByteToRead = 1 + (NbrOfBits-1) / 8;
	unsigned char* bufferToReturn = (unsigned char*)malloc(nbrOfByteToRead);
	memset(bufferToReturn, 0, nbrOfByteToRead);
	*BufferSize = nbrOfByteToRead;
	// copy all bytes 
	if (nbrOfByteToRead > 1)
	{
		memcpy(bufferToReturn, Source, nbrOfByteToRead - 1);
	}
	// copy the specific end bits
	bitMask = (1 << NbrOfBits - ((nbrOfByteToRead - 1)*8)) - 1;
	bufferToReturn[nbrOfByteToRead - 1] = Source[nbrOfByteToRead - 1] & bitMask;
	return bufferToReturn;
}

wa::strings::wa_string wa::bits_operation::PrintInBinary(unsigned char* buffer, unsigned int size)
{
	wa::strings::wa_string ToReturn = "";
	for (unsigned int NbrOfByte = 0; NbrOfByte < size; ++NbrOfByte)
	{
		for (int IndexBit = 0; IndexBit < 8; ++IndexBit)
			ToReturn.append((buffer[NbrOfByte] & (1 << IndexBit)) ? "1" : "0");
		ToReturn.append(" ' ");
	}
	return ToReturn;

}
