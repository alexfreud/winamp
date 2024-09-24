#ifndef _PATHPARSE_H
#define _PATHPARSE_H

#include <bfc/ptrlist.h>
#include <bfc/string/bfcstring.h>
#include <bfc/string/StringW.h>

/**
  PathParser is a class that parses a DOS/Windows (example: C:\DOS) 
  style path as well as a UNIX style path (example: /usr/local/bin/).
 
  @short    Path parser for Windows and UNIX style paths.
  @author Nullsoft
  @ver  1.0
*/
class PathParser
{
public:
  /**
    When PathParser is instantiated, the contructor takes the path to 
    parse and the separators to use to parse the path. It will then
    parse the path using the separators and make the parsed elements
    available. If no separators are given \ and / are used.
   
    @param str         The path to parse.
    @param separators  String that contains the separators to use. Single character separators only. 
                       No delimiters between separators in the string.
  */
  PathParser(const char *_str, const char *sep = "\\/", int uniquestrs = 0);

  void setString(const char *string, const char *separators = "\\/");

  /**
    Gets the number of path elements found in the path.
   
    @ret  The number of path elements found.
  */
  int getNumStrings();

  /**
    Gets the number of path elements found in the path.
   
    @ret  The number of path elements found.
  */
  char *enumString(int i);

  char *enumStringSafe(int i, char *def_val="");

  /**
    Gets the last path element from the parsed path.
   
    @ret  The last path element from the parsed path.
  */
  char *getLastString() { return enumString(getNumStrings()-1); }

protected:
  /**
    Override available for pre-processing of the 
    string before it's split. This is done at the start
    of the process() call.
    
    @param str A reference to the string to pre-process.
  */
  virtual void preProcess(String &str) { }
  
  /**
    Override available for post-processing of the pieces
    of the command line that are obtained after it's
    been split.
    
    @param str The command line piece to post-process.
  */
  virtual void postProcess(char *str) { }

private:
  void process();
  int processed;
  String str;
  String separators;
  PtrList<char> strings;
  int uniques;
};

class PathParserW
{
public:
  /**
    When PathParser is instantiated, the contructor takes the path to 
    parse and the separators to use to parse the path. It will then
    parse the path using the separators and make the parsed elements
    available. If no separators are given \ and / are used.
   
    @param str         The path to parse.
    @param separators  String that contains the separators to use. Single character separators only. 
                       No delimiters between separators in the string.
  */
  PathParserW(const wchar_t *_str, const wchar_t *sep = L"\\/", int uniquestrs = 0);

  void setString(const wchar_t *string, const wchar_t *separators = L"\\/");

  /**
    Gets the number of path elements found in the path.
   
    @ret  The number of path elements found.
  */
  int getNumStrings();

  /**
    Gets the number of path elements found in the path.
   
    @ret  The number of path elements found.
  */
  wchar_t *enumString(int i);

  wchar_t *enumStringSafe(int i, wchar_t *def_val=L"");

  /**
    Gets the last path element from the parsed path.
   
    @ret  The last path element from the parsed path.
  */
  wchar_t *getLastString() { return enumString(getNumStrings()-1); }

protected:
  /**
    Override available for pre-processing of the 
    string before it's split. This is done at the start
    of the process() call.
    
    @param str A reference to the string to pre-process.
  */
  virtual void preProcess(StringW &str) { }
  
  /**
    Override available for post-processing of the pieces
    of the command line that are obtained after it's
    been split.
    
    @param str The command line piece to post-process.
  */
  virtual void postProcess(wchar_t *str) { }

private:
  void process();
  int processed;
  StringW str;
  StringW separators;
  PtrList<wchar_t> strings;
  int uniques;
};

#endif
