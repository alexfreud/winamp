#ifndef _OBJDIRWND_H
#define _OBJDIRWND_H

#include <api/wnd/wndclass/treewnd.h>
#include <api/service/svcs/svc_objectdir.h>
#include <bfc/depview.h>

enum OBJ_type { OBJTYPE_OBJ, OBJTYPE_FOLDER };
class BaseObjItem : public TreeItemParam<OBJ_type> {
protected:
  BaseObjItem(OBJ_type type, const wchar_t *label) : TreeItemParam<OBJ_type>(type, label), sortorder(0) { }
public:
  int sortorder;
};

class ObjItem : public BaseObjItem {
public:
  ObjItem(ObjectHandle _handle, const wchar_t *label=NULL) : hittable(TRUE), handle(_handle), BaseObjItem(OBJTYPE_OBJ, label) {}
  
  ObjectHandle getObjectHandle() const { return handle; }

  virtual int onBeginLabelEdit() { return 0; } // allow editing
  virtual int isHitTestable() { return hittable; }

  int hittable;

private:
  ObjectHandle handle;
};

class ObjDirFolderItem : public BaseObjItem {
public:
  ObjDirFolderItem(const wchar_t *label) : BaseObjItem(OBJTYPE_FOLDER, label), dying(0) {
    setIcon(new SkinBitmap(L"player.button.thinger"));
  }
  virtual void onChildItemRemove(TreeItem *item) {
    if (getNumChildren() == 0) {
      dying = 1;
      getTree()->delItemDeferred(this);
    }
  }

  int dying;
};

#define OBJDIRWND_PARENT TreeWnd
class ObjDirWnd : public OBJDIRWND_PARENT, DependentViewerTPtr<svc_objectDir> {
public:
  ObjDirWnd();
  virtual ~ObjDirWnd();

  void setTargetDirName(const wchar_t *dirname);

  void setActionTarget(const wchar_t *targetname);

  void setDisplayTarget(const wchar_t *name);

  void setDefaultDisplay(const wchar_t *display);
  
  virtual int onInit();
  
  virtual int onContextMenu(int x, int y);
  virtual int onPreItemContextMenu(TreeItem *item, int x, int y);
  virtual int compareItem(TreeItem *p1, TreeItem *p2);
  
  virtual int viewer_onEvent(svc_objectDir *item, int event, intptr_t param2, void *ptr, size_t ptrlen);

protected:
  ObjItem *createObjItem(ObjectHandle handle);
  ObjItem *getObjItemForHandle(ObjectHandle handle);

  void folderify(ObjItem *item);

private:
  virtual void onItemSelected(TreeItem *item);
  virtual void onItemDeselected(TreeItem *item);

  svc_objectDir *objdir;
  StringW objectDirName, objectDirTarget, displayTarget, defaultDisplay;
  ifc_window *displaygroup;
  StringW displaygroupname;
};

template <class TREEITEMCLASS>
class ObjDirWndT : public ObjDirWnd {
public:
  virtual ObjItem *createObjItem(ObjectHandle handle) { return new TREEITEMCLASS(handle); }
};

#endif
