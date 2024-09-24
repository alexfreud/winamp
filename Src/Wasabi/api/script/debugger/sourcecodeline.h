#ifndef __SOURCECODELINE_H
#define __SOURCECODELINE_H

#include <bfc/dispatch.h>
#include <bfc/string/StringW.h>


class SourceCodeLine : public Dispatchable {
  public:
    const wchar_t *getLine();
    void setLine(const wchar_t *line);
    int getPointer();
    void setPointer(int pointer);
    int getLength();
    void setLength(int length);
    void setSourceFile(const wchar_t *file);
    const wchar_t *getSourceFile();
    void setSourceFileLine(int linenumber);
    int getSourceFileLine();

  enum {
    SOURCECODELINE_GETLINE = 0,
    SOURCECODELINE_SETLINE = 10,
    SOURCECODELINE_GETPOINTER = 20,
    SOURCECODELINE_SETPOINTER = 30,
    SOURCECODELINE_GETLENGTH = 40,
    SOURCECODELINE_SETLENGTH = 50,
    SOURCECODELINE_SETSOURCEFILE = 60,
    SOURCECODELINE_GETSOURCEFILE = 70,
    SOURCECODELINE_SETSOURCEFILELINE = 80,
    SOURCECODELINE_GETSOURCEFILELINE = 90,
  };
};

inline const wchar_t *SourceCodeLine::getLine() {
  return _call(SOURCECODELINE_GETLINE, (const wchar_t*)NULL);
}

inline void SourceCodeLine::setLine(const wchar_t *line) {
  _voidcall(SOURCECODELINE_SETLINE, line);
}

inline int SourceCodeLine::getPointer() {
  return _call(SOURCECODELINE_GETPOINTER, (int)0);
}

inline void SourceCodeLine::setPointer(int pointer) {
  _voidcall(SOURCECODELINE_SETPOINTER, pointer);
}

inline int SourceCodeLine::getLength() {
  return _call(SOURCECODELINE_GETLENGTH, (int)0);
}

inline void SourceCodeLine::setLength(int length) {
  _voidcall(SOURCECODELINE_SETLENGTH, length);
}

inline void SourceCodeLine::setSourceFile(const wchar_t *file) {
  _voidcall(SOURCECODELINE_SETSOURCEFILE, file);
}

inline const wchar_t *SourceCodeLine::getSourceFile() {
  return _call(SOURCECODELINE_GETSOURCEFILE, (const wchar_t *)0);
}

inline void SourceCodeLine::setSourceFileLine(int linenumber) {
  _voidcall(SOURCECODELINE_SETSOURCEFILELINE, linenumber);
}

inline int SourceCodeLine::getSourceFileLine() {
  return _call(SOURCECODELINE_GETSOURCEFILELINE, (int)0);
}

class SourceCodeLineI : public SourceCodeLine {
  public:
    SourceCodeLineI();
    virtual ~SourceCodeLineI();
    virtual const wchar_t *getLine();
    virtual void setLine(const wchar_t *line);
    virtual int getPointer();
    virtual void setPointer(int pointer);
    virtual int getLength();
    virtual void setLength(int length);
    virtual void setSourceFile(const wchar_t *file);
    virtual const wchar_t *getSourceFile();
    virtual void setSourceFileLine(int linenumber);
    virtual int getSourceFileLine();

  protected:
    RECVS_DISPATCH;

      StringW line;
      StringW file;
      int fileline;
      int pointer;
      int length;
};

#endif