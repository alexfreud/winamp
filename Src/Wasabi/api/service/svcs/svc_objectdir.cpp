#include <precomp.h>

#include "svc_objectdir.h"
#include <api/api.h>

#define CBCLASS svc_objectDirI
START_DISPATCH;
  CB(GETDEPENDENCYPTR, getDependencyPtr);
  CB(GETDIRTYPE, getDirType);
  CB(GETNUMOBJECTS, getNumObjects);
  CB(ENUMOBJECT, enumObject);
  CB(GETOBJECT, getObject);
  CB(GETOBJECTLABEL, getObjectLabel);
  CB(SETOBJECTLABEL, setObjectLabel);
  CB(INSERTOBJECT, insertObject);
  CB(REMOVEOBJECT, removeObject);
  VCB(CLEARALL, clearAll);
  CB(ONACTION, onAction);
  VCB(ONPRERENDER, onPrerender);
  VCB(ONPOSTRENDER, onPostrender);
  CB(GETOBJECTPATH, getObjectPath);
  CB(GETOBJECTDISPLAYGROUP, getObjectDisplayGroup);
  CB(GETOBJECTICON, getObjectIcon);
  CB(GETOBJECTSELECTABLE, getObjectSelectable);
  CB(GETOBJECTSORTORDER, getObjectSortOrder);
  CB(TAGOBJECT, tagObject);
  CB(UNTAGOBJECT, untagObject);
  CB(ENUMOBJECTBYTAG, enumObjectByTag);
  CB(ISTAGGED, isTagged);
  CB(CONTEXTMENU, contextMenu);
END_DISPATCH;
#undef CBCLASS
