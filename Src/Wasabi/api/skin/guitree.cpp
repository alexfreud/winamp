#include <precomp.h>
#include "guitree.h"
#include <api/skin/skinparse.h>
#include <api/syscb/callbacks/wndcb.h>
#ifdef WASABI_COMPILE_WNDMGR
#include <api/wndmgr/autopopup.h>
#endif
#include <api/skin/skin.h>
#include <bfc/bfc_assert.h>

GuiTree *guiTree=NULL;

GuiTreeItem::GuiTreeItem(int type, const wchar_t *name, ifc_xmlreaderparams *p, int scriptid, const wchar_t *path, GUID g, const wchar_t *windowtype, const wchar_t *xuitag) 
{
  if (p) 
	{
    for (size_t i=0;i!=p->getNbItems();i++)
      params.addItem(p->getItemName(i), p->getItemValue(i));
  }
  object_type = type;
  object_name = name;
  scriptId = scriptid;
  rootpath = path;
  guid = g;
  idx=-1;
  seccount = incrementor++;
  wndtype = windowtype;
  tag = xuitag;
}

int GuiTreeItem::incrementor=0;

GuiTreeItem::~GuiTreeItem() {
}

int GuiTreeItem::getScriptId() {
  return scriptId;
}

ifc_xmlreaderparams *GuiTreeItem::getParams() 
{
	// TODO: benski> helps replicate old logic.. we really fix the parts of the code that are assuming getValue() always succeeds with this returned object
	if (params.getNbItems())
		return &params;
	else
		return 0;
}

const wchar_t *GuiTreeItem::getXmlRootPath() {
  return rootpath;
}


int GuiTreeItem::getType() {
  return object_type;
}

const wchar_t *GuiTreeItem::getName() 
{
  return object_name;
}

SkinItem *GuiTreeItem::getAncestor() { 
  if (object_type == XML_TAG_GROUPDEF)
    return guiTree->getGroupDefAncestor(this); 
  if (object_type == XML_TAG_CONTAINER)
    return guiTree->getContainerAncestor(this); 
  return NULL;
}

GuiTree::GuiTree() {
  cached=-1;
  cachedtype=-1;
  cached_guid_idx=-1;
  cached_guid=INVALID_GUID;
  lastdefinedgroupdef = NULL;
}

GuiTree::~GuiTree() {
  list.deleteAll();
  groupdefs.removeAll();
  wndtypes.removeAll();
  //groupdefsbyguid.removeAll();
}

void GuiTree::addItem(int object_type, const wchar_t *name, ifc_xmlreaderparams *params, int scriptid, const wchar_t *rootpath) {
  cached = -1;
  cached_guid_idx = -1;
  GUID guid=INVALID_GUID;
  const wchar_t *wtype=NULL;
  int has_ancestor=0;
  const wchar_t *xuitag=NULL;

  const wchar_t *_id = NULL;
  if (params) 
	{
    _id = params->getItemValue(L"id");
    xuitag = params->getItemValue(L"xuitag");
#ifdef WASABI_COMPILE_WNDMGR
    if (params->getItemValueInt(L"register_autopopup"))
      AutoPopup::registerGroupId(scriptid, _id, params->getItemValue(L"name"));
#endif
    const wchar_t *strguid = params->getItemValue(L"guid");
    if (strguid)
      guid = nsGUID::fromCharW(strguid);
    wtype = params->getItemValue(L"windowtype");
  }
  if (object_type == XML_TAG_GROUPDEF && params) 
	{
    has_ancestor = (getGroupDef(_id) != NULL);
  }

  GuiTreeItem *item = new GuiTreeItem(object_type, name, params, scriptid, rootpath, guid, wtype, xuitag);
  item->setIdx(list.getNumItems());
  list.addItem(item);
  
  if (object_type == XML_TAG_GROUPDEF && params) {
    lastdefinedgroupdef = item;
//    groupdefsbyguid.setAutoSort(0);
    groupdefs.addItem(item);
//    groupdefsbyguid.addItem(item);
    if (wtype && *wtype) {
      wndtypes.addItem(item);
      deferredInvalidateType(wtype);
    }
    if (xuitag && *xuitag) {
      xuigroupdefs.addItem(item);
    }
    if (has_ancestor) 
			deferredInvalidateGroup(_id);
  } else if (object_type == XML_TAG_CONTAINER && params && (params->getItemValueInt(L"dynamic", 0) == 1)) {
    containers_by_id.setAutoSort(0);
    containers_by_id.addItem(item);
  }
}

int GuiTree::getNumObject() {
  return list.getNumItems();
}

int GuiTree::getNumObject(int object_type) {
  if (cachedtype == object_type && cached != -1) return cached;
  int n=0;
  for (int i=0;i<list.getNumItems();i++) {  
    GuiTreeItem *it = list.enumItem(i);
    if (it->getType() == object_type)
      n++;
  }
  cachedtype = object_type;
  cached = n;
  return n;
}

SkinItem *GuiTree::getObject(int object_type, int nth) {
  int n=0;
  for (int i=0;i<list.getNumItems();i++) {
    GuiTreeItem *it = list.enumItem(i);
    if (it && it->getType() == object_type) {
      if (n++ == nth) return it;
    }
  }
  return NULL;
}

SkinItem *GuiTree::getGroupDef(const wchar_t *id) 
{
  return groupdefs.findLastItem(id);
}

SkinItem *GuiTree::getXuiGroupDef(const wchar_t *xuitag) 
{
  xuigroupdefs.sort(); // doesn't sort if not necessary
  return xuigroupdefs.findLastItem(xuitag);
}

SkinItem *GuiTree::getGroupDefAncestor(SkinItem *_item) 
{
  //groupdefs.sort(); // doesn't sort if not necessary
  int pos = -1;
  GuiTreeItem *item = static_cast<GuiTreeItem *>(_item);
  if (!item) return NULL;
  ASSERT(item->getParams() != NULL);
  const wchar_t *iid = item->getParams()->getItemValue(L"id");

  pos = groupdefs.searchItem(item);
  if (pos <= 0) return NULL;
  pos--;

  GuiTreeItem *ritem = groupdefs.enumItem(pos);
  if (!ritem) return NULL;
  ASSERT(ritem->getParams() != NULL);
  if (WCSICMP(iid, ritem->getParams()->getItemValue(L"id"))) return NULL;
  return ritem;
}

SkinItem *GuiTree::getContainerAncestor(SkinItem *_item) {
  int pos = -1;
  GuiTreeItem *item = static_cast<GuiTreeItem *>(_item);
  if (!item) return NULL;
  ASSERT(item->getParams() != NULL);
  const wchar_t *iid = item->getParams()->getItemValue(L"id");

  pos = containers_by_id.searchItem(item);

  if (pos <= 0) return NULL;
  pos--;

  GuiTreeItem *ritem = containers_by_id.enumItem(pos);
  if (!ritem) return NULL;
  ASSERT(ritem->getParams() != NULL);
  if (WCSICMP(iid, ritem->getParams()->getItemValue(L"id"))) return NULL;
  return ritem;
}


/*int GuiTree::getGroupDef(GUID guid) {
  groupdefsbyguid.sort(); // doesn't sort if not necessary
  GuiTreeItem *item = groupdefsbyguid.findItem(reinterpret_cast<const char *>(&guid));
  if (item)
    return item->getIdx();
  return -1;
}*/

SkinItem *GuiTree::enumGroupDefOfType(const wchar_t *type, int n) {
  int c = 0;
  foreach(wndtypes)
    GuiTreeItem *item = wndtypes.getfor();
    if (WCSCASEEQLSAFE(type, item->getWindowType())) {
      if (c == n) {
/*        if (item->getGuid() != INVALID_GUID) { // see if it has a GUID, in which case we need to instantiate the last overriden version
          int n = getGroupDef(item->getGuid());
          if (n != -1) return n;
        }*/
        // take its groupid and find its latest overriden version
        ifc_xmlreaderparams *p = item->getParams();
        if (p) 
				{
          const wchar_t *id = p->getItemValue(L"id");
					if (id)
					{
						SkinItem *m = getGroupDef(id);
						if (m != NULL) return m;
					}
        }
        return item;
      }
      c++;
    }
  endfor;
  return NULL;
}


int GuiTree::getObjectType(SkinItem *item) {
  GuiTreeItem *i = static_cast<GuiTreeItem *>(item);
  return i->getType();
}

PtrList<GuiTreeItem> *GuiTree::getList() {
  return &list;
}

PtrList<GuiTreeItem> *GuiTree::getGroupList() {
  return &groupdefs;
}

SkinItem *GuiTree::getContainerById(const wchar_t *id) 
{
  containers_by_id.sort(); // doesn't sort if not necessary
  return containers_by_id.findLastItem(id);
}

void GuiTree::removeSkinPart(int scriptid) {
  for (int i=0;i<list.getNumItems();i++) {
    GuiTreeItem *item = list.enumItem(i);
    if (item->getScriptId() == scriptid) {
      ifc_xmlreaderparams *par = item->getParams();
      if (item->getType() == XML_TAG_CONTAINER && par != NULL) {
        int p = containers_by_id.searchItem(item);
        if (p != -1) 
          containers_by_id.removeByPos(p);
			}
			if (item->getType() == XML_TAG_GROUPDEF && par != NULL) 
			{
				const wchar_t *grpid = par->getItemValue(L"id");
				if (grpid)
				{
					int p = groupdefs.searchItem(item);
					if (p != -1) 
						groupdefs.removeByPos(p);
					p = xuigroupdefs.searchItem(item);
					if (p != -1)
						xuigroupdefs.removeByPos(p);
					p = wndtypes.searchItem(item);
					if (p != -1) {
						deferredInvalidateType(item->getWindowType());
						wndtypes.removeByPos(p);
					}
					deferredInvalidateGroup(grpid);
				}
      }
      delete item;
      list.removeByPos(i);
      i--;
    }
  }
  foreach(list)
    list.getfor()->setIdx(foreach_index);
  endfor;
}

void GuiTree::reset() {
  list.deleteAll();
  groupdefs.removeAll();
  wndtypes.removeAll();
  xuigroupdefs.removeAll();
  containers_by_id.removeAll();
}

void GuiTree::deferredInvalidateGroup(const wchar_t *id) {
  if (!Skin::isDynamicGroupReloadEnabled()) return;
  if (!id || !*id) return;
  StringW *s = new StringW;
  s->setValue(id);

  GuiTreeCB *cb = new GuiTreeCB;
  cb->cmd = INVALIDATEGRP;
  cb->ptr = s;

  timerclient_postDeferredCallback(CB_GUITREE, reinterpret_cast<intptr_t>(cb));
}

void GuiTree::deferredInvalidateType(const wchar_t *type) {
  if (!Skin::isDynamicGroupReloadEnabled()) return;
  StringW *s = new StringW;
  s->setValue(type);

  GuiTreeCB *cb = new GuiTreeCB;
  cb->cmd = INVALIDATETYPE;
  cb->ptr = s;

  timerclient_postDeferredCallback(CB_GUITREE, reinterpret_cast<intptr_t>(cb));
}

int GuiTree::timerclient_onDeferredCallback(intptr_t param1, intptr_t param2)
{
  if (param1 == CB_GUITREE) {
    GuiTreeCB *c = reinterpret_cast<GuiTreeCB *>(param2);
    switch (c->cmd) {
      case INVALIDATEGRP: {
        StringW *s = reinterpret_cast<StringW *>(c->ptr);
        WndInfo wi;
        wi.groupid = s->getValue();
        wi.wndtype = NULL;
        wi.guid = INVALID_GUID;
        WASABI_API_SYSCB->syscb_issueCallback(SysCallback::WINDOW, WndCallback::GROUPCHANGE, reinterpret_cast<intptr_t>(&wi));
        delete s;
        break;
      }
      case INVALIDATETYPE: {
        StringW *s = reinterpret_cast<StringW *>(c->ptr);
        WndInfo wi;
        ZERO(wi);
        wi.wndtype = s->getValue();
        wi.guid = INVALID_GUID;
        WASABI_API_SYSCB->syscb_issueCallback(SysCallback::WINDOW, WndCallback::TYPECHANGE, reinterpret_cast<intptr_t>(&wi));
        delete s;
        break;
      }
    }
    return 1;
  }
  return GUITREE_PARENT::timerclient_onDeferredCallback(param1, param2);
}

int GuiTree::getObjectIdx(SkinItem *item) {
  if (item == NULL) return -1;
  return (static_cast<GuiTreeItem*>(item))->getIdx();
}
