#include <precomp.h>
#include "fileread.h"
#include <api/service/svc_enum.h>

void *FileReaders::open(const wchar_t *filename, const wchar_t *mode) {
  int m=0;

  const wchar_t *p=mode;
  wchar_t c;
  while(c=*(p++)) 
	{
    switch(c) 
		{
    case 'r': m=SvcFileReader::READ; break;
    case 'w': m=SvcFileReader::WRITE; break;
    case 'a': m=SvcFileReader::APPEND; break;
    case '+': m|=SvcFileReader::PLUS; break;
    case 'b': m|=SvcFileReader::BINARY; break;
    case 't': m|=SvcFileReader::TEXT; break;
    }
  }

  return FileReaderEnum(filename, m).getFirst();
}

void FileReaders::close(void *handle) 
{
  svc_fileReader *fr=(svc_fileReader *)handle;
  fr->close();
  SvcEnum::release(fr);
}

size_t FileReaders::read(void *buffer, size_t size, void *handle) 
{
  svc_fileReader *fr=(svc_fileReader *)handle;
  return fr->read((int8_t *)buffer,size);
}

size_t FileReaders::write(const void *buffer, size_t size, void *handle) 
{
  svc_fileReader *fr=(svc_fileReader *)handle;
  return fr->write((int8_t *)buffer,size);
}

int FileReaders::seek(int64_t offset, int origin, void *handle)
{
  svc_fileReader *fr=(svc_fileReader *)handle;
  if(!fr->canSeek()) return -1;
  if(origin==SEEK_SET) return fr->seek(offset);
  if(origin==SEEK_CUR) return fr->seek(fr->getPos()+offset);
  return fr->seek(fr->getLength()-offset);
}

uint64_t FileReaders::tell(void *handle) 
{
  svc_fileReader *fr=(svc_fileReader *)handle;
  return fr->getPos();
}

uint64_t FileReaders::getFileSize(void *handle) 
{
  svc_fileReader *fr=(svc_fileReader *)handle;
  return fr->getLength();
}

int FileReaders::exists(const wchar_t *filename) {
  // Note that we do not do a system lock on the service since we can just
  // release it directly via the factory
  waService *s;
  for(int i=0;s=WASABI_API_SVC->service_enumService(WaSvc::FILEREADER,i);i++) {
    svc_fileReader *fr=(svc_fileReader *)s->getInterface(FALSE);
    if(fr->isMine(filename) == 1 || fr->open(filename)) {
      int ret=fr->exists(filename);
      fr->close();
      s->releaseInterface(fr); fr = NULL;
      return ret;
    }
    s->releaseInterface(fr); fr = NULL;
  }
  return 0;
}

int FileReaders::remove(const wchar_t *filename) {
  // Note that we do not do a system lock on the service since we can just
  // release it directly via the factory
  waService *s;
  for(int i=0;s=WASABI_API_SVC->service_enumService(WaSvc::FILEREADER,i);i++) {
    svc_fileReader *fr=(svc_fileReader *)s->getInterface(FALSE);
    if (fr->isMine(filename) == 1 || fr->open(filename)) {
      fr->close();
      int ret = fr->remove(filename);
      s->releaseInterface(fr); fr = NULL;
      return ret;
    }
    s->releaseInterface(fr); fr = NULL;
  }
  return 0;
}

int FileReaders::removeUndoable(const wchar_t *filename) {
  // Note that we do not do a system lock on the service since we can just
  // release it directly via the factory
  waService *s;
  for(int i=0;s=WASABI_API_SVC->service_enumService(WaSvc::FILEREADER,i);i++) {
    svc_fileReader *fr=(svc_fileReader *)s->getInterface(FALSE);
    if(fr->isMine(filename) == 1 || fr->open(filename)) {
      fr->close();
      int ret = fr->removeUndoable(filename);
      s->releaseInterface(fr); fr = NULL;
      return ret;
    }
    s->releaseInterface(fr); fr = NULL;
  }
  return 0;
}

int FileReaders::move(const wchar_t *filename, const wchar_t *destfilename) {
  // Note that we do not do a system lock on the service since we can just
  // release it directly via the factory
  waService *s;
  for(int i=0;s=WASABI_API_SVC->service_enumService(WaSvc::FILEREADER,i);i++) {
    svc_fileReader *fr=(svc_fileReader *)s->getInterface(FALSE);
    if(fr->isMine(filename) == 1 || fr->open(filename)) {
      fr->close();
      int ret = fr->move(filename,destfilename);
      s->releaseInterface(fr); fr = NULL;
      return ret;
    }
    s->releaseInterface(fr); fr = NULL;
  }
  return 0;
}
