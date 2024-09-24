#include "ProgressiveProcessor.h"

ProgressiveProcessor::ProgressiveProcessor()
{
	tempFile[0]=0;
	writeCursor=INVALID_HANDLE_VALUE;

	wchar_t tempPath[MAX_PATH-14] = {0};
	GetTempPath(MAX_PATH-14, tempPath);
	GetTempFileName(tempPath, L"wfv", 0, tempFile);

	writeCursor=CreateFile(tempFile, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	processedCursor=CreateFile(tempFile, GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	readCursor=CreateFile(tempFile, GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
}

ProgressiveProcessor::~ProgressiveProcessor()
{
	if (writeCursor != INVALID_HANDLE_VALUE)
		CloseHandle(writeCursor);

	if (tempFile[0])
		DeleteFile(tempFile);
}

int ProgressiveProcessor::Write(void *data, size_t datalen, size_t *written)
{
	DWORD dw_written=0;
	WriteFile(writeCursor, data, (DWORD)datalen, &dw_written, NULL);
	*written=dw_written;
	writePosition+=dw_written;

	return 0;
}
