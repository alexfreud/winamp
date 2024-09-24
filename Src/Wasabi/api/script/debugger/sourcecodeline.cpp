#include <precomp.h>
#include "sourcecodeline.h"

#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS SourceCodeLineI
START_DISPATCH;
  CB(SOURCECODELINE_GETLINE, getLine);
  VCB(SOURCECODELINE_SETLINE, setLine);
  CB(SOURCECODELINE_GETPOINTER, getPointer);
  VCB(SOURCECODELINE_SETPOINTER, setPointer);
  CB(SOURCECODELINE_GETLENGTH, getLength);
  VCB(SOURCECODELINE_SETLENGTH, setLength);
  VCB(SOURCECODELINE_SETSOURCEFILE, setSourceFile);
  CB(SOURCECODELINE_GETSOURCEFILE, getSourceFile);
  VCB(SOURCECODELINE_SETSOURCEFILELINE, setSourceFileLine);
  CB(SOURCECODELINE_GETSOURCEFILELINE, getSourceFileLine);
END_DISPATCH;

SourceCodeLineI::SourceCodeLineI() {
  pointer = -1;
  fileline = -1;
  length = 0;
}

SourceCodeLineI::~SourceCodeLineI() {
}

const wchar_t *SourceCodeLineI::getLine() 
{
  return line;
}

void SourceCodeLineI::setLine(const wchar_t *_line) {
  line = _line;
}

int SourceCodeLineI::getPointer() {
  return pointer;
}

void SourceCodeLineI::setPointer(int _pointer) {
  pointer = _pointer;
}

int SourceCodeLineI::getLength() {
  return length;
}

void SourceCodeLineI::setLength(int _length) {
  length = _length;
}

void SourceCodeLineI::setSourceFile(const wchar_t *_file) {
  file = _file;
}

const wchar_t *SourceCodeLineI::getSourceFile() {
  return file;
}

void SourceCodeLineI::setSourceFileLine(int _linenumber) {
  fileline = _linenumber;
}

int SourceCodeLineI::getSourceFileLine() {
  return fileline;
}