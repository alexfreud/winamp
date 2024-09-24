#ifndef _FILEREADERAPI_H
#define _FILEREADERAPI_H

#include <api/filereader/api_filereader.h>

class FileReaderApi : public api_fileReaderI 
{
  public:
    FileReaderApi();
    virtual ~FileReaderApi();
    
    virtual void *fileOpen(const wchar_t *filename, const wchar_t *mode);
    virtual void fileClose(void *fileHandle);
    virtual size_t fileRead(void *buffer, size_t size, void *fileHandle);
    virtual int fileWrite(const void *buffer, int size, void *fileHandle);
    virtual int fileSeek(int64_t offset, int origin, void *fileHandle);
    virtual uint64_t fileTell(void *fileHandle);
    virtual uint64_t fileGetFileSize(void *fileHandle);
    //virtual int fileExists(const wchar_t *filename);
    virtual int fileRemove(const wchar_t *filename);
    virtual int fileRemoveUndoable(const wchar_t *filename);
    virtual int fileMove(const wchar_t *filename, const wchar_t *destfilename);
};

#endif
