#include <precomp.h>

#include "svc_playlist.h"

#define CBCLASS svc_playlistReaderI
START_DISPATCH;
  CB(GETEXTENSION, getExtension);
  CB(TESTFILENAME, testFilename);
  CB(GETDESCRIPTION, getDescription);
  CB(READPLAYLIST, readPlaylist);
  CB(GETLABEL, getLabel);
  CB(GETNUMENTRIES, getNumEntries);
  CB(ENUMENTRY, enumEntry);
  VCB(ENABLEDATABASEADD, enableDatabaseAdd);
END_DISPATCH;
#undef CBCLASS

#define CBCLASS svc_playlistWriterI
START_DISPATCH;
  CB(GETEXTENSION, getExtension);
  CB(GETDESCRIPTION, getDescription);
  CB(WRITEPLAYLIST, writePlaylist);
  CB(BEGINWRITE, beginWrite);
  VCB(WRITEENTRY, writeEntry);
  VCB(ENDWRITE, endWrite);
  VCB(ENABLEMETADATA, enableMetadata);
END_DISPATCH;
#undef CBCLASS
