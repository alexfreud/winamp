#include <precomp.h>
#define REAL_STDIO
#include "zipread.h"
#include <zlib/unzip.h>
#include <bfc/parse/pathparse.h>
#include <api/skin/api_skin.h>

#define UNZIPBUFSIZE 65536

int ZipRead::open(const char *filename, int mode) {
  unzFile f=NULL;
  int success=0;

  if (WASABI_API_SKIN == NULL) return 0;
  PathParser pp1(WASABI_API_SKIN->getSkinsPath());
  PathParser pp2(filename);
  int v;
  for (v=0;v<pp1.getNumStrings();v++) 
    if (!STRCASEEQLSAFE(pp1.enumString(v), pp2.enumString(v))) return 0;
  String walName = pp2.enumString(v);
  String file;
  for (v=v+1;v<pp2.getNumStrings();v++) {
    if (!file.isempty()) file.cat(DIRCHARSTR);
    file += pp2.enumString(v);
  }

  // is there a zip file?
  String zipName;
  Std::fileInfoStruct zipFi;
  if(!Std::getFileInfos(zipName=StringPrintf("%s%s.wal",WASABI_API_SKIN->getSkinsPath(),walName.getValue()),&zipFi) &&
     !Std::getFileInfos(zipName=StringPrintf("%s%s.wsz",WASABI_API_SKIN->getSkinsPath(),walName.getValue()),&zipFi) &&
     !Std::getFileInfos(zipName=StringPrintf("%s%s.zip",WASABI_API_SKIN->getSkinsPath(),walName.getValue()),&zipFi))
     return 0; // zip not found

  if(zipTmpDir.isempty()) {
    char tmpPath[WA_MAX_PATH];
    Std::getTempPath(sizeof(tmpPath)-1,tmpPath);
    zipTmpDir=StringPrintf("%s_wa3sktmp",tmpPath);
    Std::createDirectory(zipTmpDir);
  }

  // check in cached opened zip dirs
  int badcrc=0;
  for(int i=0;i<openedZipHandles.getNumItems();i++) {
    if(!STRICMP(openedZipHandles[i].name->getValue(), walName)) {
      if(!MEMCMP(&openedZipHandles[i].checksum,&zipFi,sizeof(zipFi))) {
        // try to find it in the dezipped temp dir
        handle=openInTempDir(walName,file);
        if(handle) return 1;
        else return 0;
      } else {
        // bad checksum
        badcrc=1;
        break; 
      }
    }
  }

  // is the dezipped dir is here?
  if(!badcrc) {
    StringPrintf tmpf("%s%s%s%s_wa3chksum",zipTmpDir.getValue(),DIRCHARSTR,walName.getValue(),DIRCHARSTR);
    FILE *fh=fopen(tmpf,"rb");
    if(fh) {
      Std::fileInfoStruct tmpFi={0,};
      fread(&tmpFi,1,sizeof(tmpFi),fh);
      fclose(fh);
      if(!MEMCMP(&tmpFi,&zipFi,sizeof(tmpFi))) {
        // checksum correct
        openedZipEntry ze={new String(walName), new String(zipName)};
        ze.checksum=tmpFi;
        openedZipHandles.addItem(ze);
        handle=openInTempDir(walName,file);
        if(handle) return 1;
        else return 0;
      }
    }
  }

  // not found, so try to find it in a zip file
	f = unzOpen(zipName);
  if(!f) return 0;

  StringPrintf zDir("%s%s%s",zipTmpDir.getValue(),DIRCHARSTR,walName.getValue());
  Std::removeDirectory(zDir,1);

  // unpack the zip in temp folder
  String dirmask;
  unzGoToFirstFile(f);
  Std::createDirectory(zDir);
  do {
    char filename[MAX_PATH];
    unzGetCurrentFileInfo(f,NULL,filename,sizeof(filename),NULL,0,NULL,0);
    if (unzOpenCurrentFile(f) == UNZ_OK) {
      int l;
      dirmask.printf("%s%s%s",zDir.getValue(),DIRCHARSTR,filename);
      if (Std::isDirChar(dirmask.lastChar())) {
        // create dir
        Std::createDirectory(dirmask);
      } else {
        // create file
        FILE *fp = fopen(dirmask,"wb");
        if(!fp) {
          String dir=dirmask;
          char *p=(char *)Std::filename(dir);
          if(p) {
            *p=0;
            Std::createDirectory(dir);
            fp = fopen(dirmask,"wb");
          }
        }
        if (fp) {
          do {
            MemBlock<char> buf(UNZIPBUFSIZE);
            l=unzReadCurrentFile(f,buf.getMemory(),buf.getSizeInBytes());
            if (l > 0) fwrite(buf.getMemory(),1,l,fp);
          } while (l > 0);
          fclose(fp);
          success=1;
        }
      }
      if (unzCloseCurrentFile(f) == UNZ_CRCERROR) success=0;
    }
  } while (unzGoToNextFile(f) == UNZ_OK);
  unzClose(f);

  // write the checksum file
  Std::fileInfoStruct fi;
  Std::getFileInfos(zipName, &fi);
  FILE *fh=fopen(StringPrintf("%s%s_wa3chksum",zDir.getValue(),DIRCHARSTR),"wt");
  fwrite(&fi,1,sizeof(fi),fh);
  fclose(fh);

  openedZipEntry ze={new String(walName), new String(zipName)};
  ze.checksum=fi;
  openedZipHandles.addItem(ze);

  // try to find it (again) in the dezipped temp dir
  handle=openInTempDir(walName,file);
  if(handle) return 1;
  return 0;
}

FILE *ZipRead::openInTempDir(const char *walName, const char *file) {
  StringPrintf tmpf("%s%s%s%s%s",zipTmpDir.getValue(),DIRCHARSTR,walName,DIRCHARSTR,file);
  FILE *fh=fopen(tmpf,"rb");
  if(fh) return fh;
  // okay maybe the file isn't in the root dir of the zip file
  fh=fopen(StringPrintf("%s%s%s%s%s%s%s",zipTmpDir.getValue(),DIRCHARSTR,walName,DIRCHARSTR,walName,DIRCHARSTR,file),"rb");
  if(fh) return fh;
  // definitely not here
  return 0;
}

void ZipRead::close() {
  fclose(handle);
}

int ZipRead::read(char *buffer, int size) {
  return fread(buffer,1,size,handle);
}

int ZipRead::getPos() {
  return ftell(handle);
}

int ZipRead::getLength() {
  int pos=ftell(handle);
  fseek(handle,0,SEEK_END);
  int length=ftell(handle);
  fseek(handle,pos,SEEK_SET);
  return length;
}

using namespace wasabi;

TList<ZipRead::openedZipEntry> ZipRead::openedZipHandles;;
