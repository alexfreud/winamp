#ifndef __GROUPMGR_H
#define __GROUPMGR_H

#include <bfc/ptrlist.h>
#include <bfc/string/bfcstring.h>
#include <api/skin/skinparse.h>
#include <bfc/critsec.h>

class Group;
class CfgItem;

class ifc_window;
class SkinItem;

class CfgGroupEntry 
{
 public:
  CfgGroupEntry(const wchar_t *_id, int _type) : id(_id), type(_type) {}
  ~CfgGroupEntry() {}

  StringW id;
  int type;
};

class GroupMgr 
{
  public:
    static Group *instantiate(const wchar_t *groupid, int grouptype=GROUP_GROUP, SkinItem *specific_item=NULL, int scripts_enabled=1); 
#ifdef WASABI_COMPILE_CONFIG
    static Group *instantiate(const wchar_t *groupid, CfgItem *i, const wchar_t *name, int scripts_enabled);
#endif
    static int destroy(Group *group);
    static int hasGroup(Group *g) { return grouplist.haveItem(g); }
    static int getNumGroups() { return grouplist.getNumItems(); }
    static int exists(const wchar_t *groupid);

  private:
    static PtrList<Group> grouplist;
    static CriticalSection cs_grp;
};

#endif
