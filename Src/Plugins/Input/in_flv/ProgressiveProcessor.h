#pragma once
#include "FileProcessor.h"

class ProgressiveProcessor : public FileProcessor
{
public:
	ProgressiveProcessor();
	~ProgressiveProcessor();

private:
	/* FLVProcessor virtual method overrides */
	int Write(void *data, size_t datalen, size_t *written);
private:
	HANDLE writeCursor;
	wchar_t tempFile[MAX_PATH];
};