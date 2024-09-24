#pragma once
#include "mkv_reader.h"
#include <stdio.h>

class MKVReaderFILE : public nsmkv::MKVReader
{
public:
	MKVReaderFILE(FILE *f);
	MKVReaderFILE(const wchar_t *filename);
	~MKVReaderFILE();

	/* avi_reader implementation */
	int Read(void *buffer, size_t read_length, size_t *bytes_read);
	int Peek(void *buffer, size_t read_length, size_t *bytes_read);
	int Seek(uint64_t position);
	uint64_t Tell();
	int Skip(uint64_t skip_bytes);
	void GetFilename(wchar_t *fn, size_t fn_len) {}
	uint64_t GetContentLength();
private:
	FILE *f;	
};