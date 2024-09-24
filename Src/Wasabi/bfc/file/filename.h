#ifndef _FILENAME_H
#define _FILENAME_H

#include <bfc/string/StringW.h>
#include <bfc/string/playstring.h>
#include <bfc/dispatch.h>

// a simple class to drag-and-drop filenames around

#define DD_FILENAME L"DD_Filename v1"

// another implementation that uses the central playstring table
class FilenamePS : private Playstring
{
public:
  FilenamePS(const wchar_t *str) : Playstring(str) {}
  const wchar_t *getFilename() { return getValue(); }
	operator const wchar_t *() { return getFilename(); }	
  static const wchar_t *dragitem_getDatatype() { return DD_FILENAME; }

protected:
  FilenamePS(const FilenamePS &fn) {}
  FilenamePS& operator =(const FilenamePS &ps) { return *this; }
};

#endif
