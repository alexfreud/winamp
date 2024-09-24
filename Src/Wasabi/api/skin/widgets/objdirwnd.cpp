#include <precomp.h>

#include "objdirwnd.h"

#include <api/wnd/contextmenu.h>
#include <api/wnd/dragitemi.h>
#include <api/skin/widgets/customobject.h>
#include <bfc/parse/pathparse.h>

ObjDirWnd::ObjDirWnd() :
   objdir(NULL), displaygroup(NULL) { }

ObjDirWnd::~ObjDirWnd() {
  WASABI_API_SVC->service_release(objdir);
  if (displaygroup != NULL) WASABI_API_SKIN->group_destroy(displaygroup);
}

void ObjDirWnd::setTargetDirName(const wchar_t *dirname) {
  objectDirName = dirname;
//FUCKO: reset here
}

void ObjDirWnd::setActionTarget(const wchar_t *targetname) {
  objectDirTarget = targetname;
}

void ObjDirWnd::setDisplayTarget(const wchar_t *name) {
  displayTarget = name;
}

void ObjDirWnd::setDefaultDisplay(const wchar_t *display) {
  defaultDisplay = display;
}

int ObjDirWnd::onInit() {
  OBJDIRWND_PARENT::onInit();

  setSorted(TRUE);

  ASSERT(!objectDirName.isempty());
  objdir = ObjectDirEnum(objectDirName).getNext();

  if (objdir != NULL) {
    setHilitedColor(L"pledit.currentOutline");//FUCKO from service
    viewer_addViewItem(objdir);

    int nobj = objdir->getNumObjects();
    for (int i = 0; i < nobj; i++) {
      ObjectHandle hand = objdir->enumObject(i);
      folderify(createObjItem(hand));
    }
  }

  return 1;
}

int ObjDirWnd::onContextMenu(int x, int y) {
  if (objdir != NULL) {
    DragItemT<svc_objectDir> di(objdir);
    ContextMenu(this, &di);
  }
  return 1;
}

int ObjDirWnd::onPreItemContextMenu(TreeItem *item, int x, int y) {
  BaseObjItem *boi = static_cast<BaseObjItem*>(item);
  if (boi->getParam() == OBJTYPE_OBJ) {
    ObjItem *oi = static_cast<ObjItem*>(item);
    int r = objdir->contextMenu(this, x, y, oi->getObjectHandle());
    if (r == -1) {
      editItemLabel(item);
    }
  } else {
    // handle other item types here
  }
  return 1;
}

int ObjDirWnd::compareItem(TreeItem *p1, TreeItem *p2) {
  BaseObjItem *b1 = static_cast<BaseObjItem*>(p1);
  BaseObjItem *b2 = static_cast<BaseObjItem*>(p2);
  int r = CMP3(b1->sortorder, b2->sortorder);
  if (r == 0) return OBJDIRWND_PARENT::compareItem(p1, p2);
  return r;
}

int ObjDirWnd::viewer_onEvent(svc_objectDir *item, int event, intptr_t param2, void *ptr, size_t ptrlen) {
  switch (event) {
    case svc_objectDir::Event_OBJECT_ADDED: {
      int sel_after = (enumAllItems(0) == NULL);
      ObjItem *oi = createObjItem(param2);
      folderify(oi);
      if (sel_after) selectItemDeferred(oi);
    }
    break;
    case svc_objectDir::Event_OBJECT_REMOVED: {
      ObjItem *pli = getObjItemForHandle(param2);
      delItemDeferred(pli);
    }
    break;
    case svc_objectDir::Event_OBJECT_LABELCHANGE: {
      ObjItem *pli = getObjItemForHandle(param2);
      const wchar_t *name = objdir->getObjectLabel(pli->getObjectHandle());
      pli->setLabel(name);
    }
    break;
    case svc_objectDir::Event_OBJECT_PATHCHANGE: {
      ObjItem *pli = getObjItemForHandle(param2);
      folderify(pli);
    }
    break;
    case svc_objectDir::Event_OBJECT_SORTORDERCHANGE: {
      ObjItem *pli = getObjItemForHandle(param2);
      ASSERT(pli != NULL);
      pli->sortorder = objdir->getObjectSortOrder(param2);
      sortTreeItems();
    }
    break;
  }
  return 1;
}

ObjItem *ObjDirWnd::createObjItem(ObjectHandle handle) {
  ObjItem *ret = new ObjItem(handle, objdir->getObjectLabel(handle));
  const wchar_t *nn = objdir->getObjectIcon(handle);
  if (nn && *nn) {
    SkinBitmap *ic = new SkinBitmap(nn);
    ret->setIcon(ic);
  }
  ret->hittable = objdir->getObjectSelectable(handle);
  ret->sortorder = objdir->getObjectSortOrder(handle);
  return ret;
}

ObjItem *ObjDirWnd::getObjItemForHandle(ObjectHandle handle) {
  for (int i = 0; ; i++) {
    TreeItem *it = enumAllItems(i);
    if (it == NULL) break;
    BaseObjItem *boi = static_cast<BaseObjItem*>(it);
    if (boi->getParam() != OBJTYPE_OBJ) continue;
    ObjItem *plit = static_cast<ObjItem *>(it);
    if (plit->getObjectHandle() == handle) return plit;
  }
  return NULL;
}

void ObjDirWnd::folderify(ObjItem *item) {
  if (item->getTree() != NULL)
    removeTreeItem(item);

  PathParserW pp(objdir->getObjectPath(item->getObjectHandle()));
  TreeItem *prevpar = NULL;

  for (int i = 0; i < pp.getNumStrings(); i++) {
    TreeItem *newpar = NULL;
    // check for already-existing folder
    for (int j = 0; ; j++) {
      if (prevpar == NULL) {	// search top level
        newpar = enumRootItem(j);
      } else {	// search prevpar children
        newpar = prevpar->getNthChild(j);
      }

      if (newpar == NULL) break;	// out of thingies
      BaseObjItem *boi = static_cast<BaseObjItem*>(newpar);
      if (boi->getParam() != OBJTYPE_FOLDER) continue;
      ObjDirFolderItem *fi = static_cast<ObjDirFolderItem*>(newpar);
      if (fi->dying) continue;
    
      if (!wcscmp(boi->getLabel(), pp.enumString(i))) {
        break;
      }
    }
    if (newpar == NULL) {
      newpar = new ObjDirFolderItem(pp.enumString(i));
      addTreeItem(newpar, prevpar, FALSE, TRUE);
      expandItemDeferred(newpar);
    }
    prevpar = newpar;
  }

  // now attach it
  addTreeItem(item, prevpar);
}

void ObjDirWnd::onItemSelected(TreeItem *item) {
  BaseObjItem *boi = static_cast<BaseObjItem*>(item);
  if (boi->getParam() == OBJTYPE_OBJ) {
    ObjItem *objitem = static_cast<ObjItem *>(item);
    ObjectHandle handle = objitem->getObjectHandle();
    StringW display(objdir->getObjectDisplayGroup(handle));
    if (display.isempty()) display = defaultDisplay;
    if (!display.isempty()) 
		{
      GuiObject *go = findObject(displayTarget);
      if (go != NULL) 
			{
        CustomObject *co = static_cast<CustomObject *>(go->guiobject_getScriptObject()->vcpu_getInterface(customObjectGuid));
        if (co != NULL) {
          if (displaygroupname != display) {
            co->customobject_setRootWnd(NULL);
            ifc_window *prev = displaygroup;
            if (prev != NULL) WASABI_API_SKIN->group_destroy(prev);
            displaygroup = WASABI_API_SKIN->group_create(display);
            co->customobject_setRootWnd(displaygroup);
            displaygroupname = display;
          }
        }
      }
    }
    // tell the objdir
    objdir->onAction(svc_objectDir::ODACTION_SELECTED, this, objectDirTarget, handle);
  } else {
    // handle other item types here
  }
}

void ObjDirWnd::onItemDeselected(TreeItem *item) {
  BaseObjItem *boi = static_cast<BaseObjItem*>(item);
  if (boi->getParam() == OBJTYPE_OBJ) {
    ObjItem *objitem = static_cast<ObjItem *>(item);
    ObjectHandle handle = objitem->getObjectHandle();
    objdir->onAction(svc_objectDir::ODACTION_DESELECTED, this, objectDirTarget, handle);
  } else {
    // handle other item types here
  }
}
