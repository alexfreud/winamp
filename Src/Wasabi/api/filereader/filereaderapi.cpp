#include <precomp.h>
#include "filereaderapi.h"
#include <api/filereader/local/fileread.h>

api_fileReader *fileApi = NULL;


FileReaderApi::FileReaderApi() {
}

FileReaderApi::~FileReaderApi() {
}

void *FileReaderApi::fileOpen(const wchar_t *filename, const wchar_t *mode) {
  return FileReaders::open(filename, mode);
}

void FileReaderApi::fileClose(void *fileHandle) {
  FileReaders::close(fileHandle);
}

size_t FileReaderApi::fileRead(void *buffer, size_t size, void *fileHandle) {
  return FileReaders::read(buffer, size, fileHandle);
}

int FileReaderApi::fileWrite(const void *buffer, int size, void *fileHandle) {
  return FileReaders::write(buffer, size, fileHandle);
}

int FileReaderApi::fileSeek(int64_t offset, int origin, void *fileHandle) {
  return FileReaders::seek(offset, origin, fileHandle);
}

uint64_t FileReaderApi::fileTell(void *fileHandle) {
  return FileReaders::tell(fileHandle);
}

uint64_t FileReaderApi::fileGetFileSize(void *fileHandle) {
  return FileReaders::getFileSize(fileHandle);
}

int FileReaderApi::fileRemove(const wchar_t *filename) {
  return FileReaders::remove(filename);
}

int FileReaderApi::fileRemoveUndoable(const wchar_t *filename) {
  return FileReaders::removeUndoable(filename);
}

int FileReaderApi::fileMove(const wchar_t *filename, const wchar_t *destfilename) {
  return FileReaders::move(filename, destfilename);
}

