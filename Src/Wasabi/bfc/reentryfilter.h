#ifndef __REENTRYFILTER_H
#define __REENTRYFILTER_H

#include <bfc/tlist.h>

class ReentryFilterObject {
  public:

    ReentryFilterObject() {}
    virtual ~ReentryFilterObject() {}

    void enterScope(int scope) {
      scopes.addItem(scope);
    }

    int isInScope(int scope) {
      for (int i=0;i<scopes.getNumItems();i++)
        if (scopes.enumItem(i) == scope) return 1;
      return 0;
    }

    void leaveScope(int scope) {
      for (int i=0;i<scopes.getNumItems();i++)
        if (scopes.enumItem(i) == scope) {
          scopes.delByPos(i);
          return;
        }
    }

  private:
    
    TList<int> scopes;
};

class ReentryFilter {
  public:

    ReentryFilter() : reentering(0), thisscope(-1), filter(NULL) {} // call setFilterObject, enterScope and leaveScope manually

    ReentryFilter(ReentryFilterObject *filterobj, intptr_t scope) {
      setFilterObject(filterobj);
      enterScope(scope);
    }

    void setFilterObject(ReentryFilterObject *filterobj) {
      filter = filterobj;
    }

    void enterScope(intptr_t scope) {
      ASSERT(filter != NULL);
      if (filter->isInScope(scope)) {
        reentering = 1;
      } else {
        filter->enterScope(scope);
        reentering = 0;
      }
      thisscope = scope;
    }

    void leaveScope() {
      ASSERT(filter != NULL);
      filter->leaveScope(thisscope);
    }

    virtual ~ReentryFilter() {
      if (reentering) return;
      leaveScope();
    }

    int mustLeave() { return reentering; }

  private:
    intptr_t thisscope;
    int reentering;
    ReentryFilterObject *filter;
};

#endif