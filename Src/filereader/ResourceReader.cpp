#include "ResourceReader.h"

ResourceReader::ResourceReader() : data(NULL), g(NULL), ptr(0)
{}

int ResourceReader::open(const wchar_t *filename, int mode)
{
	if (_wcsnicmp(filename, L"res://", 6)) return 0;
	filename += 6;
	wchar_t blah[MAX_PATH];
	lstrcpynW(blah, filename, MAX_PATH);

	wchar_t *p = blah;
	while (p && *p && *p != ',') p++;
	if (p && *p != ',') return 0;
	if (p) *p++ = 0;
	HINSTANCE hInst = (HINSTANCE)_wtoi64(blah);
	int id = _wtoi(p);

	HRSRC r = FindResource(hInst, MAKEINTRESOURCE(id), RT_RCDATA);
	if (r == NULL) return 0;
	g = LoadResource(hInst, r);
	if (g == NULL) return 0;
	data = (char*)LockResource(g);
	if (data == NULL)
	{
		FreeResource(g); // see win32 doc
		g = NULL;
		return 0;
	}
	size = SizeofResource(hInst, r);
	ptr = 0;

	return 1;
}

size_t ResourceReader::read(__int8 *buffer, size_t length)
{
	size_t s = min(size - ptr, length);
	if (s)
		memcpy(buffer, data + ptr, s);
	ptr += s;
	return s;
}

size_t ResourceReader::write(const __int8 *buffer, size_t length)
{

	return 0;
}

void ResourceReader::close()
{
	if (g)
	{
		UnlockResource(g);
		FreeResource(g); // see win32 doc
		g = NULL;
		data = NULL;
	}
}

unsigned __int64 ResourceReader::getPos()
{
	return ptr;
}

unsigned __int64 ResourceReader::getLength()
{
	return size;
}

int ResourceReader::canSeek()
{
	return 1;
}

int ResourceReader::seek(unsigned __int64 position)
{
	ptr = (size_t)min(position, size); 
	return 1;
}

int ResourceReader::exists(const wchar_t *filename)
{
	return 1;
} // always exists if open succeeded


#define CBCLASS ResourceReader
START_DISPATCH;
//  CB(ISMINE, isMine);
  CB(OPEN, open);
  CB(READ, read);
  CB(WRITE, write);
  VCB(CLOSE, close);
//  VCB(ABORT, abort);
  CB(GETLENGTH, getLength);
  CB(GETPOS, getPos);
  CB(CANSEEK, canSeek);
//  CB(SEEK, seek);
//  CB(HASHEADERS,hasHeaders);
//  CB(GETHEADER,getHeader);
  CB(EXISTS,exists);
//  CB(REMOVE,remove);
//  CB(REMOVEUNDOABLE,removeUndoable);
//  CB(MOVE,move);
//  CB(BYTESAVAILABLE,bytesAvailable);
//  VCB(SETMETADATACALLBACK,setMetaDataCallback);
//  CB(CANPREFETCH,canPrefetch);
//  CB(CANSETEOF, canSetEOF);
//  CB(SETEOF, setEOF);
END_DISPATCH;
#undef CBCLASS

