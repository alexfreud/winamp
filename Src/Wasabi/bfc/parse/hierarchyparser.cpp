#include "precomp_wasabi_bfc.h"
#include "hierarchyparser.h"
#include <bfc/nsguid.h>
#include <wchar.h>
// This uses an AMAZINGLY inefficient algorithm!  woo hoo!

HierarchyParser::HierarchyParser(const wchar_t *str, const wchar_t *_sibling, const wchar_t *_escape, const wchar_t *_parent_open, const wchar_t *_parent_close) {
  // Create a new rootnode.
  rootnode = new HPNode(str);
  // Parse the rootnode's contents into the rootnode's children.
  HierarchyParser downparse(rootnode, _sibling, _escape, _parent_open, _parent_close);
  // Set the actual name of the rootnode ("")
  (*rootnode)() = L"";
  // Mark that this was our allocation
  myalloc = 1;
}

HierarchyParser::~HierarchyParser() {
  // If we alloc, we must delete.
  if (myalloc) {
    delete rootnode;
  }
}

HPNode *HierarchyParser::findGuid(GUID g) {
  return NULL;
}

HPNode *HierarchyParser::findString(const wchar_t *str) {
  return NULL;
}

HierarchyParser::HierarchyParser(HPNode *_rootnode, const wchar_t *_sibling, const wchar_t *_escape, const wchar_t *_parent_open, const wchar_t *_parent_close) {
  // We did not alloc, we should not delete.
  rootnode = _rootnode;
  sibling = _sibling;
  escape = _escape;
  parent_open = _parent_open;
  parent_close = _parent_close;

  myalloc = 0;

  const wchar_t *parsestr = (*rootnode)();

  size_t i, length = wcslen(parsestr), depth = 0;

  StringW curr_sibling;

  for (i = 0; i < length; i++ ) {
    wchar_t c = parsestr[i];

    if (isEscape(c)) {
      // always add the next character
      curr_sibling += parsestr[++i];  
    } else if (isSibling(c)) {
      // if we're not inside someone else,
      if (!depth) {
        // okay, we're done with the current sibling.  ship him off.
        processSibling(curr_sibling);
        // on to the next sibling!
        curr_sibling = L"";
      } else {
        curr_sibling += c;
      }      
    } else if (isParentOpen(c)) {
      // increment depth
      curr_sibling += c;
      depth++;
    } else if (isParentClose(c)) {
      // decrement depth
      curr_sibling += c;
      depth--;
    } else {
      curr_sibling += c;
    }
  }

  // If there is anything left over, process it as a sibling.
  if (curr_sibling.len()) {
    processSibling(curr_sibling);
  }
}

void HierarchyParser::processSibling(const wchar_t *sibstr) {
  StringW curr_sibling = sibstr;
  // slice the name out of the front of the string.
  StringW sibling_name = curr_sibling.lSpliceChar(parent_open);
  // curr_sibling will contain the children of this sibling (or nothing).
  curr_sibling.rSpliceChar(parent_close);
  // create a new child for our root node to contain the sibling's child info
  HPNode *child = new HPNode(curr_sibling, rootnode);
  // parse the child hierarchically for its children
  HierarchyParser childparser(child, sibling, escape, parent_open, parent_close);
  // once parsed. set its name to be this sibling
  (*child)() = sibling_name;
  // and lastly add him as a child to the root node.
  rootnode->addChild(child);
}

