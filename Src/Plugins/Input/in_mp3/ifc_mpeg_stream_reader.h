#pragma once

#include <bfc/dispatch.h>

class ifc_mpeg_stream_reader : public Dispatchable
{
protected:
	ifc_mpeg_stream_reader()                                          {}
	~ifc_mpeg_stream_reader()                                         {}

public:
	int MPEGStream_Peek( void *buffer, size_t to_read, size_t *bytes_read );
	int MPEGStream_Read( void *buffer, size_t to_read, size_t *bytes_read );
	int MPEGStream_EOF();
	float MPEGStream_Gain();

	DISPATCH_CODES
	{
		MPEGSTREAM_PEEK = 0,
		MPEGSTREAM_READ = 1,
		MPEGSTREAM_EOF  = 2,
		MPEGSTREAM_GAIN = 3,
	};
};

inline int ifc_mpeg_stream_reader::MPEGStream_Peek( void *buffer, size_t to_read, size_t *bytes_read )
{
	return _call( MPEGSTREAM_PEEK, (int)1, buffer, to_read, bytes_read );
}

inline int ifc_mpeg_stream_reader::MPEGStream_Read( void *buffer, size_t to_read, size_t *bytes_read )
{
	return _call( MPEGSTREAM_READ, (int)1, buffer, to_read, bytes_read );
}

inline int ifc_mpeg_stream_reader::MPEGStream_EOF()
{
	return _call( MPEGSTREAM_EOF, (int)true );
}

inline float ifc_mpeg_stream_reader::MPEGStream_Gain()
{
	return _call( MPEGSTREAM_GAIN, (float)1.0f );
}

