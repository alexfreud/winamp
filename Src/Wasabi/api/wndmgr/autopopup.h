#ifndef __AUTOPOPUP_H
#define __AUTOPOPUP_H

#include <bfc/ptrlist.h>
#include <bfc/string/StringW.h>

class AutoPopupEntry;
class AutoPopupEntrySort;

#define SKINPARTID_NONE -1

class AutoPopup {

  public:

    static int registerGuid(int skinpartid/*SKINPARTID_NONE*/, GUID g, const wchar_t *desc, const wchar_t *prefered_container=NULL, int required=FALSE);
    static int registerGroupId(int skinpartid/*SKINPARTID_NONE*/, const wchar_t *groupid, const wchar_t *desc, const wchar_t *prefered_container=NULL, int required=FALSE);
    static void unregister(int id);

    static int getNumItems();
    static AutoPopupEntry *enumItem(int n);
    static AutoPopupEntry *getByDesc(const wchar_t *desc);

    static void reset();
    static void removeSkinPart(int id);

    static int allocNid();
    static void removeAllAddons();

    static int getNumGuids();
    static GUID enumGuid(int n);
    static int getNumGroups();
    static const wchar_t *enumGroup(int n);
    static const wchar_t *enumGuidDescription(int n);
    static const wchar_t *enumGroupDescription(int n);

    static const wchar_t *getDefaultContainerParams(const wchar_t *groupid, GUID g, int *flag);
};

class AutoPopupEntry {

  public:

    AutoPopupEntry(int skinpartid, GUID g, const wchar_t *grpid, const wchar_t *description, const wchar_t *prefered_container=NULL, int required=TRUE) : guid(g), groupid(grpid), desc(description), container(prefered_container), container_how(required), skinpart(skinpartid) { nid = AutoPopup::allocNid(); }
    virtual ~AutoPopupEntry() {  }

    GUID getGuid() { return guid; }
    const wchar_t *getGroupId() { return groupid; }
    const wchar_t *getDescription() { return desc; }
    int getNid() { return nid; }
    const wchar_t *getPreferedContainer() { return container; }
    int getContainerHow() { return container_how; }
    int getSkinpart() { return skinpart; }

  private:

    GUID guid;
    StringW groupid;
    StringW desc;
    int nid;
    StringW container;
    int container_how;
    int skinpart;
};

class AutoPopupEntrySort {
public:
  static int compareItem(AutoPopupEntry *p1, AutoPopupEntry *p2) {
    return WCSICMP(p1->getDescription(), p2->getDescription());
  }
  static int compareAttrib(const wchar_t *attrib, AutoPopupEntry *item) {
    return WCSICMP(attrib, item->getDescription());
  }
};

#endif
