#ifndef _READDIR_H
#define _READDIR_H



#include <bfc/common.h>
#include <bfc/string/StringW.h>

/* intended use:
  ReadDir dir(path);
  while (dir.next()) {
    const char *fn = dir.getFilename();
  }
*/

class ReadDir 
{
public:
  ReadDir(const wchar_t *path, const wchar_t *match=NULL, bool skipdots=true);
  ~ReadDir();

  int next();	// done when returns 0
  const wchar_t *getFilename();
  int isDir();	// if current file is a dir
  int isReadonly();	// if current file is readonly

  int isDotDir();	// if given dir iteself is being enumerated (usually ".")
  int isDotDotDir();	// if parent dir of cur dir is showing (usually "..")

  const wchar_t *getPath() { return path; }

private:
  StringW path, match;
  int skipdots, first;
//PORT
#ifdef WIN32
  HANDLE files;
  WIN32_FIND_DATAW data; // (shrug) so we have two?  so what?
  //StringW filename;
#endif
#ifdef LINUX
  DIR *d;
  struct dirent *de;
  struct stat st;
#endif
};

#endif
