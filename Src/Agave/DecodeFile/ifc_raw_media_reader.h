#pragma once

#include <bfc/dispatch.h>
#include <bfc/error.h>

class ifc_raw_media_reader : public Dispatchable
{
protected:
	ifc_raw_media_reader() {}
	~ifc_raw_media_reader() {}

public:
	int Read(void *buffer, size_t buffer_size, size_t *bytes_read);
	/* TODO: we'll probably need stuff in here like EndOfFile, determining a good buffer size, etc */

	DISPATCH_CODES
	{
		RAW_READ
	};
};

inline int ifc_raw_media_reader::Read(void *buffer, size_t buffer_size, size_t *bytes_read)
{
	return _call(RAW_READ, (int)NErr_NotImplemented, buffer, buffer_size, bytes_read);
}

