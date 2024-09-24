#include <precomp.h>
#include <api/skin/groupmgr.h>
#include <api/skin/skinparse.h>
#include <api/skin/guitree.h>
#include <api/config/items/attrint.h>
#include <api/config/items/cfgitem.h>

Group *GroupMgr::instantiate(const wchar_t *groupid, int grouptype, SkinItem *specific_item, int scripts_enabled) 
{
  InCriticalSection in_cs(&cs_grp);
  GuiObject *go = SkinParser::newDynamicGroup(groupid, grouptype, specific_item, 0, scripts_enabled);
  Group *g = static_cast<Group *>(go->guiobject_getScriptObject()->vcpu_getInterface(groupGuid));
  if (!g) return NULL;
  grouplist.addItem(g);
  return g;
}

#ifdef WASABI_COMPILE_CONFIG

Group *GroupMgr::instantiate(const wchar_t *groupid, CfgItem *i, const wchar_t *name, int scripts_enabled) {
  InCriticalSection in_cs(&cs_grp);
  CfgGroup *g = static_cast<CfgGroup *>(instantiate(groupid, GROUP_CFGGROUP, NULL, scripts_enabled));
  ASSERTPR(g != NULL, StringPrintf("couldn't load groupid '%S'", groupid));
  g->setAttr(i, name);
  grouplist.addItem(g);
  return g;
}

#endif

int GroupMgr::destroy(Group *group) {
  InCriticalSection in_cs(&cs_grp);
  if (!grouplist.haveItem(group)) { 
    DebugStringW(L"you cannot destroy a group that you have not instatiated\n");
    return 0;
  }
  delete group;
  grouplist.removeItem(group);
  return 1;
}

int GroupMgr::exists(const wchar_t *groupid) 
{
  SkinItem *si = guiTree->getGroupDef(groupid);
  if (si) return 1;
  return 0;
}


PtrList<Group> GroupMgr::grouplist;
CriticalSection GroupMgr::cs_grp;
