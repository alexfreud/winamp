#ifndef __PARAMPARSE_H
#define __PARAMPARSE_H

#include <bfc/parse/pathparse.h>
#include <bfc/node.h>
#include <bfc/string/StringW.h>


// typedef NodeC<String> HPNode   // OH YAH, YOU CAN'T FWD REF TYPEDEFS.  STOOOOOOPID.
class HPNode : public NodeC<StringW> {
public:
  HPNode( const StringW & myPayload, NodeC<StringW> * myParent = NULL ) : NodeC<StringW>(myPayload, myParent) {}
};

class HierarchyParser {
public:    
  HierarchyParser(const wchar_t *str = NULL, const wchar_t *_sibling=L";", const wchar_t *_escape=L"\\", const wchar_t *_parent_open=L"(", const wchar_t *_parent_close=L")") ;
  ~HierarchyParser();

  HPNode *findGuid(GUID g);
  HPNode *findString(const wchar_t *str);

  int hasGuid(GUID g) { return findGuid(g) != NULL; }
  int hasString(const wchar_t *str) { return findString(str) != NULL; }

  HPNode *rootNode() { return rootnode; }

private:
  HPNode *rootnode;
  int myalloc;

  StringW sibling;
  StringW escape;
  StringW parent_open;
  StringW parent_close;

  HierarchyParser(HPNode *_rootnode, const wchar_t *_sibling=L";", const wchar_t *_escape=L"\\", const wchar_t *_parent_open=L"(", const wchar_t *_parent_close=L")");
  void processSibling(const wchar_t *sibling);


  inline int isSibling(wchar_t c) {
    return sibling.lFindChar(c) != -1;
  }
  inline int isEscape(wchar_t c) {
    return escape.lFindChar(c) != -1;
  }
  inline int isParentOpen(wchar_t c) {
    return parent_open.lFindChar(c) != -1;
  }
  inline int isParentClose(wchar_t c) {
    return parent_close.lFindChar(c) != -1;
  }
};

#endif
