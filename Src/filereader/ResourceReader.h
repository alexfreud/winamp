#ifndef NULLSOFT_RESOURCEREADER_H
#define NULLSOFT_RESOURCEREADER_H

#include <api/service/svcs/svc_fileread.h>
class ResourceReader : public svc_fileReader
{
public:
	public:
  ResourceReader();
  virtual ~ResourceReader() { close(); }

  int open(const wchar_t *filename, int mode=SvcFileReader::READ);

  size_t read(__int8 *buffer, size_t length);
  size_t write(const __int8 *buffer, size_t length);
  void close();
  unsigned __int64 getPos();
  unsigned __int64 getLength();
  int canSeek();
  int seek(unsigned __int64 position);
  int exists(const wchar_t *filename);
  /*int remove(const char *filename) { return 0; }
  int move(const char *filename, const char *destfilename) { return 0; }*/
protected:
	RECVS_DISPATCH;
private:
  __int8 *data;
  size_t ptr;
  size_t size;
  HGLOBAL g;

};

#endif