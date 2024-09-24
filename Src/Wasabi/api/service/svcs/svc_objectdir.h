#ifndef _SVC_OBJECTDIR_H
#define _SVC_OBJECTDIR_H

#include <bfc/dispatch.h>
#include <bfc/string/StringW.h>
#include <api/service/services.h>
#include <api/syscb/callbacks/runlevelcb.h>

// there is class ObjectDir in bfc/wnds, you should derive from it
// also class ContextCmdObjDir

typedef size_t ObjectHandle;
#define INVALID_OBJECT_HANDLE ((ObjectHandle)0)

#define DD_OBJECTDIR L"service:svc_objectDir"

class ifc_window;
class ifc_dependent;
class BaseCanvas;

class svc_objectDir : public Dispatchable 
{
public:
  static int getServiceType() { return WaSvc::OBJECTDIR; }
  static const wchar_t *dragitem_getDatatype() { return DD_OBJECTDIR; }
  static const GUID *depend_getClassGuid() {
    // {2364D110-0F12-40d4-BBAE-D2DA174751B5}
    static const GUID ret = 
    { 0x2364d110, 0xf12, 0x40d4, { 0xbb, 0xae, 0xd2, 0xda, 0x17, 0x47, 0x51, 0xb5 } };
    return &ret;
  }

  api_dependent *getDependencyPtr();

  const wchar_t *getDirType();

  int getNumObjects();
  ObjectHandle enumObject(int n);

  void *getObject(ObjectHandle handle);

  const wchar_t *getObjectLabel(ObjectHandle handle);
  int setObjectLabel(ObjectHandle handle, const wchar_t *newlabel);

  ObjectHandle insertObject(const wchar_t *parameter=NULL, const wchar_t *label=NULL, const wchar_t *path=NULL);
  int removeObject(ObjectHandle handle);

  void clearAll();

  const wchar_t *getObjectPath(ObjectHandle handle);
  const wchar_t *getObjectDisplayGroup(ObjectHandle handle);
  const wchar_t *getObjectIcon(ObjectHandle handle);
  int getObjectSelectable(ObjectHandle handle);
  int getObjectSortOrder(ObjectHandle handle);	// -32767..32767

  // tagging
  int tagObject(const wchar_t *tag, ObjectHandle handle, int exclusive=FALSE);
  int untagObject(const wchar_t *tag, ObjectHandle handle);
  ObjectHandle enumObjectByTag(const wchar_t *tag, int n);
  int isTagged(const wchar_t *tag, ObjectHandle handle);

  int onAction(int action, ifc_window *from, const wchar_t *target, ObjectHandle handle);
  enum {
    ODACTION_SELECTED=100,
    ODACTION_DESELECTED=200,
    ODACTION_CONTEXTMENU=300,
  };

  int contextMenu(ifc_window *from, int x, int y, ObjectHandle handle);

  void onPrerender(ObjectHandle handle, const RECT *r, BaseCanvas *c, int style);
  void onPostrender(ObjectHandle handle, const RECT *r, BaseCanvas *c, int style);
  // render styles
  enum {
    RENDERSTYLE_TREEWND=10,
  };

  // dependency events, param is handle of object in question
  enum {
    Event_OBJECT_ADDED=100,
    Event_OBJECT_REMOVED=110,
    Event_OBJECT_LABELCHANGE=200,
    Event_OBJECT_ICONCHANGE=300,
    Event_OBJECT_PATHCHANGE=400,
    Event_OBJECT_SELECTABLECHANGE=500,
    Event_OBJECT_SORTORDERCHANGE=600,
    Event_OBJECT_TAGCHANGE=700,
  };

  // dispatchable codes
  enum {
    GETDEPENDENCYPTR=100,
    GETNUMOBJECTS=200,
    ENUMOBJECT=300,
    GETOBJECT=400,
    GETOBJECTLABEL=500,
    SETOBJECTLABEL=510,
    INSERTOBJECT=600,
    REMOVEOBJECT=610,
    CLEARALL=700,
    GETDIRTYPE=800,
    ONACTION=900,
    ONPRERENDER=1000,
    ONPOSTRENDER=1010,
    GETOBJECTPATH=1100,
    GETOBJECTDISPLAYGROUP=1200,
    GETOBJECTICON=1300,
    GETOBJECTSELECTABLE=1400,
    GETOBJECTSORTORDER=1500,
    TAGOBJECT=1600,
    UNTAGOBJECT=1700,
    ENUMOBJECTBYTAG=1800,
    ISTAGGED=1900,
    CONTEXTMENU=3000,
  };
};

inline
api_dependent *svc_objectDir::getDependencyPtr() {
  return _call(GETDEPENDENCYPTR, (api_dependent*)NULL);
}

inline
const wchar_t *svc_objectDir::getDirType() {
  return _call(GETDIRTYPE, (const wchar_t *)NULL);
}

inline
int svc_objectDir::getNumObjects() {
  return _call(GETNUMOBJECTS, 0);
}

inline
ObjectHandle svc_objectDir::enumObject(int n) {
  return _call(ENUMOBJECT, INVALID_OBJECT_HANDLE, n);
}

inline
void *svc_objectDir::getObject(ObjectHandle handle) {
  return _call(GETOBJECT, (void*)NULL, handle);
}

inline
const wchar_t *svc_objectDir::getObjectLabel(ObjectHandle handle) {
  return _call(GETOBJECTLABEL, (const wchar_t *)NULL, handle);
}

inline
int svc_objectDir::setObjectLabel(ObjectHandle handle, const wchar_t *newlabel) {
  return _call(SETOBJECTLABEL, 0, handle, newlabel);
}

inline
ObjectHandle svc_objectDir::insertObject(const wchar_t *parameter, const wchar_t *label, const wchar_t *path) {
  return _call(INSERTOBJECT, INVALID_OBJECT_HANDLE, parameter, label, path);
}

inline
int svc_objectDir::removeObject(ObjectHandle handle) {
  return _call(REMOVEOBJECT, 0, handle);
}

inline
void svc_objectDir::clearAll() {
  _voidcall(CLEARALL);
}

inline
const wchar_t *svc_objectDir::getObjectPath(ObjectHandle handle) {
  return _call(GETOBJECTPATH, (const wchar_t *)NULL, handle);
}

inline
const wchar_t *svc_objectDir::getObjectDisplayGroup(ObjectHandle handle) {
  return _call(GETOBJECTDISPLAYGROUP, L"", handle);
}

inline
const wchar_t *svc_objectDir::getObjectIcon(ObjectHandle handle) {
  return _call(GETOBJECTICON, (const wchar_t *)NULL, handle);
}

inline
int svc_objectDir::getObjectSelectable(ObjectHandle handle) {
  return _call(GETOBJECTSELECTABLE, TRUE, handle);
}

inline
int svc_objectDir::getObjectSortOrder(ObjectHandle handle) {
  return _call(GETOBJECTSORTORDER, 0, handle);
}

inline
int svc_objectDir::tagObject(const wchar_t *tag, ObjectHandle handle, int exclusive) {
  return _call(TAGOBJECT, 0, tag, handle, exclusive);
}
inline
int svc_objectDir::untagObject(const wchar_t *tag, ObjectHandle handle) {
  return _call(UNTAGOBJECT, 0, tag, handle);
}
inline
ObjectHandle svc_objectDir::enumObjectByTag(const wchar_t *tag, int n) {
  return _call(ENUMOBJECTBYTAG, INVALID_OBJECT_HANDLE, tag, n);
}
inline
int svc_objectDir::isTagged(const wchar_t *tag, ObjectHandle handle) {
  return _call(ISTAGGED, 0, tag, handle);
}

inline
int svc_objectDir::onAction(int action, ifc_window *from, const wchar_t *target, ObjectHandle handle) {
  return _call(ONACTION, 0, action, from, target, handle);
}

inline
int svc_objectDir::contextMenu(ifc_window *from, int x, int y, ObjectHandle handle) {
  return _call(CONTEXTMENU, 0, from, x, y, handle);
}

inline
void svc_objectDir::onPrerender(ObjectHandle handle, const RECT *r, BaseCanvas *c, int style) {
  _voidcall(ONPRERENDER, handle, r, c, style);
}
inline
void svc_objectDir::onPostrender(ObjectHandle handle, const RECT *r, BaseCanvas *c, int style) {
  _voidcall(ONPOSTRENDER, handle, r, c, style);
}


/**
  Service implementation. Usually you'll derive from ObjectDir, not this.
  You can still derive from this if you want to fully implement the interface yourself for some reason though.
  @see ObjectDir
*/
class svc_objectDirI : public svc_objectDir {
public:
  virtual api_dependent *getDependencyPtr()=0;

  virtual const wchar_t *getDirType()=0;

  virtual int getNumObjects()=0;
  virtual ObjectHandle enumObject(int n)=0;

  virtual void *getObject(ObjectHandle handle)=0;

  virtual const wchar_t *getObjectLabel(ObjectHandle handle)=0;
  virtual int setObjectLabel(ObjectHandle handle, const wchar_t *newlabel)=0;

  virtual ObjectHandle insertObject(const wchar_t *parameter=NULL, const wchar_t *label=NULL, const wchar_t *path=NULL)=0;
  virtual int removeObject(ObjectHandle handle)=0;

  virtual void clearAll()=0;

  virtual const wchar_t *getObjectPath(ObjectHandle handle)=0;
  virtual const wchar_t *getObjectDisplayGroup(ObjectHandle handle)=0;
  virtual const wchar_t *getObjectIcon(ObjectHandle handle)=0;
  virtual int getObjectSelectable(ObjectHandle handle)=0;
  virtual int getObjectSortOrder(ObjectHandle handle)=0;

  virtual int tagObject(const wchar_t *tag, ObjectHandle handle, int exclusive=FALSE)=0;
  virtual int untagObject(const wchar_t *tag, ObjectHandle handle)=0;
  virtual ObjectHandle enumObjectByTag(const wchar_t *tag, int n)=0;
  virtual int isTagged(const wchar_t *tag, ObjectHandle handle)=0;

  virtual int onAction(int action, ifc_window *from, const wchar_t *target, ObjectHandle handle)=0;

  // return -1 to request renaming the item, 0 normally
  virtual int contextMenu(ifc_window *from, int x, int y, ObjectHandle handle)=0;

  virtual void onPrerender(ObjectHandle handle, const RECT *r, BaseCanvas *c, int style) { }
  virtual void onPostrender(ObjectHandle handle, const RECT *r, BaseCanvas *c, int style) { }

protected:
  RECVS_DISPATCH;
};

#include <api/service/servicei.h>

template <class T>
class ObjectDirCreator : public waServiceFactoryTSingle<svc_objectDir, T> { };

#include <api/service/svc_enum.h>

class ObjectDirEnum : public SvcEnumT<svc_objectDir> {
public:
  ObjectDirEnum(const wchar_t *_name) : name(_name) {}
  virtual int testService(svc_objectDir *svc) {
    return !WCSICMP(svc->getDirType(), name);
  }
private:
  StringW name;
};

#endif
