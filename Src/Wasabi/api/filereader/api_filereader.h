#ifndef __API_FILEREADER_H
#define __API_FILEREADER_H

#include <bfc/dispatch.h>

class NOVTABLE api_fileReader : public Dispatchable 
{
  public:
    void *fileOpen(const wchar_t *filename, OSFNCSTR mode);
    void fileClose(void *fileHandle);
    size_t fileRead(void *buffer, size_t size, void *fileHandle);
    size_t fileWrite(const void *buffer, int size, void *fileHandle);
    int fileSeek(int64_t offset, int origin, void *fileHandle);
    uint64_t fileTell(void *fileHandle);
    uint64_t fileGetFileSize(void *fileHandle);
    //int fileExists(const wchar_t *filename);
    int fileRemove(const wchar_t *filename);
    int fileRemoveUndoable(const wchar_t *filename);
    int fileMove(const wchar_t *filename, const wchar_t *destfilename);

  enum {
    API_FILEREADER_FILEOPEN = 0,
    API_FILEREADER_FILECLOSE = 10,
    API_FILEREADER_FILEREAD = 20,
    API_FILEREADER_FILEWRITE = 30,
    API_FILEREADER_FILESEEK = 40,
    API_FILEREADER_FILETELL = 50,
    API_FILEREADER_FILEGETFILESIZE = 60,
    //API_FILEREADER_FILEEXISTS = 70,
    API_FILEREADER_FILEREMOVE = 80,
    API_FILEREADER_FILEREMOVEUNDOABLE = 90,
    API_FILEREADER_FILEMOVE = 100,
  };
};

inline void *api_fileReader::fileOpen(const wchar_t *filename, OSFNCSTR mode) {
  return _call(API_FILEREADER_FILEOPEN, (void *)NULL, filename, mode);
}

inline void api_fileReader::fileClose(void *fileHandle) {
  _voidcall(API_FILEREADER_FILECLOSE, fileHandle);
}

inline size_t api_fileReader::fileRead(void *buffer, size_t size, void *fileHandle) {
  return _call(API_FILEREADER_FILEREAD, (size_t)0, buffer, size, fileHandle);
}

inline size_t api_fileReader::fileWrite(const void *buffer, int size, void *fileHandle) {
  return _call(API_FILEREADER_FILEWRITE, (size_t)0, buffer, size, fileHandle);
}

inline int api_fileReader::fileSeek(int64_t offset, int origin, void *fileHandle) {
  return _call(API_FILEREADER_FILESEEK, (int)0, offset, origin, fileHandle);
}

inline uint64_t api_fileReader::fileTell(void *fileHandle) {
  return _call(API_FILEREADER_FILETELL, (uint64_t)0, fileHandle);
}

inline uint64_t api_fileReader::fileGetFileSize(void *fileHandle) {
  return _call(API_FILEREADER_FILEGETFILESIZE, (uint64_t)0, fileHandle);
}

/*inline int api_fileReader::fileExists(const wchar_t *filename) {
  return _call(API_FILEREADER_FILEEXISTS, (int)0, filename);
}*/

inline int api_fileReader::fileRemove(const wchar_t *filename) {
  return _call(API_FILEREADER_FILEREMOVE, (int)0, filename);
}

inline int api_fileReader::fileRemoveUndoable(const wchar_t *filename) {
  return _call(API_FILEREADER_FILEREMOVEUNDOABLE, (int)0, filename);
}

inline int api_fileReader::fileMove(const wchar_t *filename, const wchar_t *destfilename) {
  return _call(API_FILEREADER_FILEMOVE, (int)0, filename, destfilename);
}

class api_fileReaderI : public api_fileReader {
  public:
    virtual void *fileOpen(const wchar_t *filename, const wchar_t *mode)=0;
    virtual void fileClose(void *fileHandle)=0;
    virtual size_t fileRead(void *buffer, size_t size, void *fileHandle)=0;
    virtual int fileWrite(const void *buffer, int size, void *fileHandle)=0;
    virtual int fileSeek(int64_t offset, int origin, void *fileHandle)=0;
    virtual uint64_t fileTell(void *fileHandle)=0;
    virtual uint64_t fileGetFileSize(void *fileHandle)=0;
    //virtual int fileExists(const wchar_t *filename)=0;
    virtual int fileRemove(const wchar_t *filename)=0;
    virtual int fileRemoveUndoable(const wchar_t *filename)=0;
    virtual int fileMove(const wchar_t *filename, const wchar_t *destfilename)=0;

  protected:
    RECVS_DISPATCH;
};

// {E357E736-4967-4279-B948-5073A186F565}
static const GUID fileReaderApiServiceGuid = 
{ 0xe357e736, 0x4967, 0x4279, { 0xb9, 0x48, 0x50, 0x73, 0xa1, 0x86, 0xf5, 0x65 } };

extern api_fileReader *fileApi;

#endif
