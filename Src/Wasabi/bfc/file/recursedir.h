#ifndef _RECURSEDIR_H
#define _RECURSEDIR_H

#include <bfc/wasabi_std.h>
#include <bfc/ptrlist.h>
#include <bfc/common.h>
#include <bfc/file/readdir.h>

class ReadDir;

/**
  Read the contents of a directory, recursively.
  Also possible to use search match patterns.
  
  @short Recursive directory reading.
  @author Nullsoft
  @ver 1.0
  @see ReadDir
*/
class RecurseDir {
public:
  /**
    Sets the directory to read and the match pattern.
    If no match pattern is set, it will match against
    all files.
    
    @param path   The path of the directory to read.
    @param match  The match pattern to use.
  */
  RecurseDir(const wchar_t *path, const wchar_t *match=NULL);
  
  /**
    Deletes the directory stack.
  */
  ~RecurseDir();

  /**
    Advance to the next file.
    
    @ret 0, No more files to read; > 0, Files left to read.
  */
  int next();
  
  /**
    Restart from the top of the directory tree.
    
    @ret 0
  */
  int restart();

  /**
    Get the current directory path.
    
    @ret The path.
  */
  const wchar_t *getPath();
  
  /**
    Get the filename for the current file.
    
    @ret The filename.
  */
  const wchar_t *getFilename();

  const wchar_t *getOriginalPath();

private:
  StringW path, match;
  ReadDir *curdir;
  PtrList<ReadDir> dirstack;
};

#endif
