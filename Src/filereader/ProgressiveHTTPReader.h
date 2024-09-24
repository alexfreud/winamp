#ifndef NULLSOFT_PROGRESSIVEHTTPREADER_H
#define NULLSOFT_PROGRESSIVEHTTPREADER_H

#include <api/service/svcs/svc_fileread.h>
#include <windows.h>

class ProgressiveHTTPReader : public svc_fileReader
{
public:
	ProgressiveHTTPReader() : hFile(INVALID_HANDLE_VALUE), hMap(0), offset(0)
	{}
	int isMine(const wchar_t *filename, int mode=SvcFileReader::READ);
	int open(const wchar_t *filename, int mode=SvcFileReader::READ);
	size_t read(__int8 *buffer, size_t length);
	void IncrementPosition(uint64_t inc);
	size_t GetPageNumber() const;
private:
	char tempFile[MAX_PATH];
	HANDLE hFile;
	HANDLE hMap;
	uint32_t pageSize;
	LARGE_INTEGER pagePosition; // will always be % pageSize == 0
	uint32_t offset; // offset into the page
	
};

#endif