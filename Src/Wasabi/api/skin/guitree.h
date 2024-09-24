#ifndef __GUITREE_H
#define __GUITREE_H

#include <bfc/string/bfcstring.h>
#include <bfc/ptrlist.h>
#include <bfc/nsguid.h>
#include <api/timer/timerclient.h>
#include <api/xml/xmlreader.h>
#include <api/xml/xmlparamsi.h>
#include <api/skin/skinitem.h>
#include <bfc/string/StringW.h>

#define CB_GUITREE 0x987

#define INVALIDATEGRP  1
#define INVALIDATETYPE 2

class GuiTreeCB {
  public:
    int cmd;
    StringW *ptr;
};

class GuiTreeItem : public SkinItemI 
{
  public:
    GuiTreeItem(int type, const wchar_t *name, ifc_xmlreaderparams *p, int scriptid, const wchar_t *rootpath, GUID g, const wchar_t *windowtype, const wchar_t *xuitag);
    virtual ~GuiTreeItem();
    virtual int getType();
    virtual const wchar_t *getName();
    virtual int getSkinPartId() { return getScriptId(); }
    virtual int getScriptId();
    virtual ifc_xmlreaderparams *getParams();
    virtual const wchar_t *getXmlRootPath();
    virtual void setIdx(int i) { idx = i; }
    virtual int getIdx() { return idx; }
    virtual int getSecCount() { return seccount; }
    virtual void setGuid(GUID g) { guid = g; }
    virtual GUID getGuid() { return guid; }
    virtual const wchar_t *getWindowType() { return wndtype; }
    virtual const wchar_t *getXuiTag() { return tag; }
    virtual void setXuiTag(const wchar_t *xuitag) { tag = xuitag; }
    virtual SkinItem *getAncestor();
  
  private:
    XmlReaderParamsI params;
    int object_type;
    StringW object_name;
    int scriptId;
    StringW rootpath;
    int idx;
    int seccount;
    GUID guid;
    StringW wndtype;
    StringW tag;

    static int incrementor;
};

class SortGuiTreeItem
{
public:
  static int compareItem(GuiTreeItem *p1, GuiTreeItem *p2) {
    int r = WCSICMP(p1->getParams()->getItemValue(L"id"), p2->getParams()->getItemValue(L"id"));
    if (!r) {
      if (p1->getScriptId() < p2->getScriptId()) return -1;
      if (p1->getScriptId() > p2->getScriptId()) return 1;
      if (p1->getSecCount() < p2->getSecCount()) return -1;
      if (p1->getSecCount() > p2->getSecCount()) return 1;
      return 0;
    }
    return r;
  }
  static int compareAttrib(const wchar_t *attrib, GuiTreeItem *item) {
    return WCSICMP(attrib, item->getParams()->getItemValue(L"id"));
  }
};

class SortGuiTreeItemByXuiTag {
public:
  static int compareItem(GuiTreeItem *p1, GuiTreeItem *p2) {
    int r = WCSICMP(p1->getParams()->getItemValue(L"xuitag"), p2->getParams()->getItemValue(L"xuitag"));
    if (!r) {
      if (p1->getScriptId() < p2->getScriptId()) return -1;
      if (p1->getScriptId() > p2->getScriptId()) return 1;
      if (p1->getSecCount() < p2->getSecCount()) return -1;
      if (p1->getSecCount() > p2->getSecCount()) return 1;
      return 0;
    }
    return r;
  }
  static int compareAttrib(const wchar_t *attrib, GuiTreeItem *item) {
    return WCSICMP(attrib, item->getParams()->getItemValue(L"xuitag"));
  }
};

class SortGuiTreeItemByGuid
{
public:
  static int compareItem(GuiTreeItem *p1, GuiTreeItem *p2) {
    GUID g1 = p1->getGuid();
    GUID g2 = p2->getGuid();
    if (g1 == g2) {
      if (p1->getScriptId() < p2->getScriptId()) return -1;
      if (p1->getScriptId() > p2->getScriptId()) return 1;
      if (p1->getSecCount() < p2->getSecCount()) return -1;
      if (p1->getSecCount() > p2->getSecCount()) return 1;
      return 0;
    }
    return nsGUID::compare(g1, g2) < 0 ? -1 : 1;
  }
  static int compareAttrib(const wchar_t *attrib, GuiTreeItem *item) {
    const GUID *g = reinterpret_cast<const GUID *>(attrib);
    ASSERT(g);
    GUID g1 = *g;
    GUID g2 = item->getGuid();
    if (g1 == g2) return 0;
    return nsGUID::compare(g1, g2) < 0 ? -1 : 1;
  }
};

#define GUITREE_PARENT TimerClientDI 
class GuiTree : public GUITREE_PARENT {
  public:

    GuiTree();
    virtual ~GuiTree();

    void addItem(int object_type, const wchar_t *object_name, ifc_xmlreaderparams *params, int scriptId, const wchar_t *rootpath);
    SkinItem *getGroupDef(const wchar_t *id);
    SkinItem *enumGroupDefOfType(const wchar_t *type, int n);
    SkinItem *getGroupDefAncestor(SkinItem *item);
    SkinItem *getContainerAncestor(SkinItem *item);
    //int getGroupDef(GUID g);
    SkinItem *getXuiGroupDef(const wchar_t *xuitag);
    int getObjectType(SkinItem *item); 
    int getNumObject(); // total number of objects
    int getNumObject(int object_type); // return the number of objects of this type
    SkinItem *getObject(int object_type, int nth); // get nth object_type, return its index
    SkinItem *getContainerById(const wchar_t *id);
    int getObjectIdx(SkinItem *item);
    void reset(void);
    PtrList<GuiTreeItem> *getList();;
    PtrList<GuiTreeItem> *getGroupList();;
    void removeSkinPart(int scriptid);
    void deferredInvalidateGroup(const wchar_t *id);
    void deferredInvalidateType(const wchar_t *type);
    int timerclient_onDeferredCallback(intptr_t param1, intptr_t param2);
    SkinItem *getLastDefinedGroup() { return lastdefinedgroupdef; }
    SkinItem *enumGroupDef(int n) { return groupdefs.enumItem(n); }
    int getNumGroupDefs() { return groupdefs.getNumItems(); }


  private:
    int cached;
    int cachedtype;
    PtrList<GuiTreeItem> list;
    PtrListInsertMultiSorted<GuiTreeItem, SortGuiTreeItem> groupdefs;
//    PtrListQuickMultiSorted<GuiTreeItem, SortGuiTreeItemByGuid> groupdefsbyguid;
    PtrListQuickMultiSorted<GuiTreeItem, SortGuiTreeItemByXuiTag> xuigroupdefs;
    PtrListQuickMultiSorted<GuiTreeItem, SortGuiTreeItem> containers_by_id;
    PtrList<GuiTreeItem> wndtypes;
    int cached_guid_idx;
    GUID cached_guid;
    SkinItem *lastdefinedgroupdef;
};

extern GuiTree *guiTree;

#endif

