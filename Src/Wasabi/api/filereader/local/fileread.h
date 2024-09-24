#ifndef _FILEREADERS_H
#define _FILEREADERS_H

#include <api/filereader/svc_filereadI.h>
#include <api/service/servicei.h>

class FileReaders 
{
public:
  static void *open(const wchar_t *filename, const wchar_t *mode);
  static void close(void *handle);
  static size_t read(void *buffer, size_t size, void *handle);
  static size_t write(const void *buffer, size_t size, void *handle);
  static int seek(int64_t offset, int origin, void *handle);
  static uint64_t tell(void *handle);
  static uint64_t getFileSize(void *handle);
  static int exists(const wchar_t *filename);
  static int remove(const wchar_t *filename);
  static int removeUndoable(const wchar_t *filename);
  static int move(const wchar_t *filename, const wchar_t *destfilename);
};

#endif//_FILEREADERS_H
