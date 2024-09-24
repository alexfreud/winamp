#include "file_avi_reader.h"

AVIReaderFILE::AVIReaderFILE(const wchar_t *filename)
{
	f = _wfopen(filename, L"rb");
}

int AVIReaderFILE::Read(void *buffer, uint32_t read_length, uint32_t *bytes_read)
{
	*bytes_read = fread(buffer, 1, read_length, f);
	return nsavi::READ_OK;
}

	int AVIReaderFILE::Peek(void *buffer, uint32_t read_length, uint32_t *bytes_read)
	{
		*bytes_read = fread(buffer, 1, read_length, f);
		fseek(f, -read_length, SEEK_CUR);
		return nsavi::READ_OK;
	}

	int AVIReaderFILE::Seek(uint64_t position)
	{
fsetpos(f, (const fpos_t *)&position);
return nsavi::READ_OK;
	}

	uint64_t AVIReaderFILE::Tell()
	{
		uint64_t pos;
		fgetpos(f, (fpos_t *)&pos);
		return pos;
	}

	int AVIReaderFILE::Skip(uint32_t skip_bytes)
	{
		fseek(f, skip_bytes, SEEK_CUR);
		return nsavi::READ_OK;
	}

	AVIReaderFILE::~AVIReaderFILE()
	{
		fclose(f);
	}