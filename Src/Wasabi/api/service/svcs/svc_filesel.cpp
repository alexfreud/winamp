#include <precomp.h>

#include "svc_filesel.h"

#define CBCLASS svc_fileSelectorI
START_DISPATCH;
  CB(TESTPREFIX, testPrefix);
  CB(GETPREFIX, getPrefix);
  CB(SETEXTLIST, setExtList);
  CB(RUNSELECTOR, runSelector);
  CB(GETNUMFILESSELECTED, getNumFilesSelected);
  CB(ENUMFILENAME, enumFilename);
  CB(GETDIRECTORY, getDirectory);
END_DISPATCH;
#undef CBCLASS
