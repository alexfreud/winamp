#include <precomp.h>
#ifndef NOCBMGR 
#include "cbmgr.h"
#include <api/syscb/callbacks/syscb.h>

#include <bfc/multimap.h>

static MultiMap<int, SysCallback> cblist;
static PtrList<SysCallback> delete_list;
static int reentry_counter=0;

void CallbackManager::registerCallback(SysCallback *cb, void *param, WaComponent *owner) {
  cblist.multiAddItem(cb->getEventType(), cb, TRUE);
}

void CallbackManager::deregisterCallback(SysCallback *cb, WaComponent *owner) {
  delete_list.addItem(cb);
  cblist.multiDelItem(cb->getEventType(), cb);	// remove ref
}

void CallbackManager::issueCallback(int eventtype, int msg, int param1, int param2) {
  ASSERT(reentry_counter >= 0);
  reentry_counter++;
  const PtrList<SysCallback> *mlist = cblist.getListForIndex(eventtype);
  if (mlist != NULL) {
    foreach(mlist)
      SysCallback *cb = mlist->getfor();
      ASSERT(cb != NULL);
      if (!delete_list.haveItem(cb))
        cb->notify(msg, param1, param2);
    endfor
  }
  reentry_counter--;
  if (reentry_counter == 0)
    delete_list.removeAll();
}

int CallbackManager::getNumCallbacks() {
  return cblist.getNumItems();
}
#endif