/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
/* implementation ideas, notes and todos

Memory mapped file to use as temporary storage
mark downloaded pages with bitfield vector
use content length header
use content-disposition for filename, if available (instead of temp filename)

*/
#include "ProgressiveHTTPReader.h"
#include <windows.h>

int ProgressiveHTTPReader::isMine(const wchar_t *filename, int mode)
{
	return 0; // only want to use the progressive downloader/reader on demand.
}

int ProgressiveHTTPReader::open(const wchar_t *filename, int mode)
{
	pagePosition.QuadPart=0;

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	pageSize = info.dwPageSize;
	
  char tempPath[MAX_PATH];
	GetTempPathA(MAX_PATH, tempPath);
	GetTempFileNameA(tempPath, "phr", 0, tempPath);
	
	hFile=CreateFile((LPCWSTR)tempPath, GENERIC_WRITE|GENERIC_READ, 0, 0, CREATE_ALWAYS, 0, 0);

	
	LARGE_INTEGER contentLength;
	contentLength.QuadPart = 100*1024*1024; // TODO: use content length header for filesize
	
	hMap=CreateFileMapping(hFile, 0, PAGE_READWRITE, contentLength.HighPart, contentLength.LowPart, 0);
	// TODO: spawn a thread and start downloading data..
	// thread should get a duplicate file handle

	return 1;
}

void ProgressiveHTTPReader::IncrementPosition(uint64_t inc)
{
	pagePosition.QuadPart += offset;
	pagePosition.QuadPart += inc;
	offset = (uint32_t)(pagePosition.QuadPart & (uint64_t)pageSize);
	pagePosition.QuadPart-=offset;
}

size_t ProgressiveHTTPReader::GetPageNumber() const
{
	return (size_t)(pagePosition.QuadPart / (uint64_t)pageSize);
}

size_t ProgressiveHTTPReader::read(__int8 *buffer, size_t length)
{
	/* TODO:
	   is this area of the file downloaded yet?  If so, just return it as is
		 otherwise, what should we do?  return what we can?  or sit and wait until we get the data?
		 */
	while (length)
	{
		// if page is available
		{
			// TODO: calculate maximum length we can map
			MapViewOfFile(hMap, FILE_MAP_READ, pagePosition.HighPart, pagePosition.LowPart, pageSize);
			// map page
			// copy to buffer
			// increment buffer
			// decrement length
			// increment position
		}
		//else
		{
			// queue up read request to background thread
			// wait for signal
			continue;
		}
	}
	return 0;
}