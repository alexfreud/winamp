#pragma once
#include "avi_reader.h"
#include <stdio.h>

class AVIReaderFILE : public nsavi::avi_reader
{
public:
	AVIReaderFILE(const wchar_t *filename);
	~AVIReaderFILE();

	/* avi_reader implementation */
	int Read(void *buffer, uint32_t read_length, uint32_t *bytes_read);
	int Peek(void *buffer, uint32_t read_length, uint32_t *bytes_read);
	int Seek(uint64_t position);
	uint64_t Tell();
	int Skip(uint32_t skip_bytes);
	void GetFilename(wchar_t *fn, size_t fn_len) {}
	uint64_t GetContentLength() 
	{
		return 1; 
	}
private:
	FILE *f;
};