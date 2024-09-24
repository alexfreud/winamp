#pragma once

#include "foundation/dispatch.h"

class ifc_raw_media_reader : public Wasabi2::Dispatchable
{
protected:
	ifc_raw_media_reader() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_raw_media_reader() {}
public:
	int Read(void *buffer, size_t buffer_size, size_t *bytes_read) { return RawMediaReader_Read(buffer, buffer_size, bytes_read); }
	/* TODO: we'll probably need stuff in here like EndOfFile, determining a good buffer size, etc */

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
protected:
	virtual int WASABICALL RawMediaReader_Read(void *buffer, size_t buffer_size, size_t *bytes_read)=0;
};
