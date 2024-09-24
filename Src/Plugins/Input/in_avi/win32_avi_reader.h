#pragma once
#include "../nsavi/avi_reader.h"
#include "../nu/ringbuffer.h"

class AVIReaderWin32 : public nsavi::avi_reader, private Filler
{
public:
	AVIReaderWin32();
	~AVIReaderWin32();
	int Open(const wchar_t *filename);
	void Close();

/* avi_reader implementation */
	uint64_t GetContentLength();
	int Seek(uint64_t position);

private:
	int Read(void *p_read_buffer, uint32_t read_length, uint32_t *bytes_read);
	int Peek(void *p_read_buffer, uint32_t read_length, uint32_t *bytes_read);
	void OverlappedHint(uint32_t read_length);
	uint64_t Tell();
	int Skip(uint32_t skip_bytes);
	void GetFilename(wchar_t *fn, size_t len);
	
	/* internal helpers */
	void DoRead();
/* RingBuffer Filler implementation */
		size_t Read(void *dest, size_t len);

	HANDLE hFile; // i hate hungarian notation, but calling this hFile is a force of habit :)
	RingBuffer _ring_buffer;
	bool end_of_file;
	LARGE_INTEGER position; // since we read ahead, we need to keep track of this separately
	wchar_t *local_filename;

};