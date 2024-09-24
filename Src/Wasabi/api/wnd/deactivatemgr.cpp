#include <precomp.h>
#include "deactivatemgr.h"

#define FAKE_PTR (ifc_window *)-1
#define BYPASS_DEACTIVATE_ATOM "BYPASS_DEACTIVATE_MGR"

int AppDeactivationMgr::is_deactivation_allowed(ifc_window *w) {
#ifdef WIN32
  if (FindAtomA(BYPASS_DEACTIVATE_ATOM) != NULL) return 1; // so people don't _need_ an api pointer to bypass us, however, if you can please call api->appdeactivation_setbypass
#else
  DebugString( "portme -- AppDeactivationMgr::is_deactivation_allowed\n");
#endif
  return list.getNumItems() == 0;
}

void AppDeactivationMgr::push_disallow(ifc_window *w) {
  if (w == NULL)
    w = FAKE_PTR;
  list.addItem(w);
}

void AppDeactivationMgr::pop_disallow(ifc_window *w) {
  if (w == NULL)
    w = FAKE_PTR;
  if (list.getNumItems() == 0) {
    return;
  }
  while (list.getNumItems()>0) {
    int p = list.searchItem(w);
    if (p >= 0)
      list.removeByPos(p);
    else break;
  }
}

void AppDeactivationMgr::setbypass(int i) {
#ifdef WIN32
  if (i) {
    ATOM a = FindAtomA(BYPASS_DEACTIVATE_ATOM);
    if (a != NULL) return;
    AddAtomA(BYPASS_DEACTIVATE_ATOM);
  } else {
    ATOM a = FindAtomA(BYPASS_DEACTIVATE_ATOM);
    if (a != NULL) {
      DeleteAtom(a);
      return;
    }
  }
#else
  DebugString( "portme -- AppDeactivationMgr::setbypass\n" );
#endif
}

PtrList<ifc_window> AppDeactivationMgr::list;

