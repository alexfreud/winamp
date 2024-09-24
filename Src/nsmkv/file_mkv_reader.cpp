#include "file_mkv_reader.h"

MKVReaderFILE::MKVReaderFILE(FILE *f) : f(f)
{
}
MKVReaderFILE::MKVReaderFILE(const wchar_t *filename)
{
	f = _wfopen(filename, L"rb");
}

int MKVReaderFILE::Read(void *buffer, size_t read_length, size_t *bytes_read)
{
	*bytes_read = fread(buffer, 1, read_length, f);
	return nsmkv::READ_OK;
}

int MKVReaderFILE::Peek(void *buffer, size_t read_length, size_t *bytes_read)
{
	*bytes_read = fread(buffer, 1, read_length, f);
	fseek(f, (long)(-read_length), SEEK_CUR);
	return nsmkv::READ_OK;
}

int MKVReaderFILE::Seek(uint64_t position)
{
	fsetpos(f, (const fpos_t *)&position);
	return nsmkv::READ_OK;
}

uint64_t MKVReaderFILE::Tell()
{
	uint64_t pos;
	fgetpos(f, (fpos_t *)&pos);
	return pos;
}

int MKVReaderFILE::Skip(uint64_t skip_bytes)
{
	_fseeki64(f, skip_bytes, SEEK_CUR);
	return nsmkv::READ_OK;
}

MKVReaderFILE::~MKVReaderFILE()
{
	fclose(f);
}

uint64_t MKVReaderFILE::GetContentLength() 
{
	uint64_t old = Tell();
	Seek(0);
	uint64_t content_length = Tell();
	Seek(old);
	return content_length; 
}