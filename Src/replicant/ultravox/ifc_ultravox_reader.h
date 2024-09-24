#pragma once
#include "foundation/dispatch.h"

class ifc_ultravox_reader : public Wasabi2::Dispatchable
{
protected:
	ifc_ultravox_reader() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_ultravox_reader() {}
public:
	enum
	{
		DISPATCHABLE_VERSION=0,
	};

	size_t BytesBuffered() { return UltravoxReader_BytesBuffered(); }
	int Read(void *buffer, size_t buffer_length, size_t *bytes_read) { return UltravoxReader_Read(buffer, buffer_length, bytes_read); }
	int Peek(void *buffer, size_t buffer_length, size_t *bytes_read) { return UltravoxReader_Peek(buffer, buffer_length, bytes_read); }
	int IsClosed() { return UltravoxReader_IsClosed(); }
	/* gives you back the contents of exactly one packet.  used when Ultravox is provided codec framing */
	int ReadPacket(void *buffer, size_t buffer_length, size_t *bytes_read) { return UltravoxReader_ReadPacket(buffer, buffer_length, bytes_read); }
private:
	virtual int WASABICALL UltravoxReader_Read(void *buffer, size_t buffer_length, size_t *bytes_read)=0;
	virtual int WASABICALL UltravoxReader_Peek(void *buffer, size_t buffer_length, size_t *bytes_read)=0;
	virtual size_t WASABICALL UltravoxReader_BytesBuffered()=0;
	virtual int WASABICALL UltravoxReader_IsClosed()=0;
	virtual int WASABICALL UltravoxReader_ReadPacket(void *buffer, size_t buffer_length, size_t *bytes_read)=0;
	
};
