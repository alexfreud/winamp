#ifndef _ZIPREAD_H
#define _ZIPREAD_H

#include <api/service/svcs/svc_fileread.h>
#include <api/service/servicei.h>
#include <bfc/memblock.h>
#include <bfc/string/string.h>
#include <bfc/ptrlist.h>
#include <bfc/tlist.h>

class ZipRead : public svc_fileReaderI {
public:
  // service
  static const char *getServiceName() { return "ZIP file reader"; }

  int open(const char *filename, int mode=SvcFileReader::READ);
  int read(char *buffer, int length);
  int write(const char *buffer, int length) { return 0; }
  void close();
  int getPos();
  int getLength();
  
private:
  FILE *openInTempDir(const char *walName, const char *file);
  
  typedef struct {
    String *name;
    String *zipName;
    Std::fileInfoStruct checksum;
  } openedZipEntry;

  static wasabi::TList<openedZipEntry> openedZipHandles;

  String zipTmpDir;
  
  FILE *handle;
};

#endif//_ZIPREAD_H
