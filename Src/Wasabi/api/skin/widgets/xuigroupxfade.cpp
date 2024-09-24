#include <precomp.h>
#include "xuigroupxfade.h"


#ifndef _WASABIRUNTIME

BEGIN_SERVICES(GroupXFade_Svc);
DECLARE_SERVICE(XuiObjectCreator<GroupXFadeXuiSvc>);
END_SERVICES(GroupXFade_Svc, _GroupXFade_Svc);

#ifdef _X86_
extern "C" { int _link_GroupXFadeXuiSvc; }
#else
extern "C" { int __link_GroupXFadeXuiSvc; }
#endif

#endif


	
XMLParamPair GroupXFade::params[] = {
  {GROUPXFADE_SETGROUP, L"GROUP"},
  {GROUPXFADE_SETGROUP, L"GROUPID"},
  {GROUPXFADE_SETSPEED, L"SPEED"},
	};
GroupXFade::GroupXFade() {
  child[0] = child[1] = NULL;
  curchild = 0;
  myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);
  speed = 0.25;	
}

void GroupXFade::CreateXMLParameters(int master_handle)
{
	//GROUPXFADE_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);

}

GroupXFade::~GroupXFade() {
  if (child[0]) {
    ifc_window *w = child[0]->guiobject_getRootWnd();
    if (w) WASABI_API_SKIN->group_destroy(w);
  }
  if (child[1]) {
    ifc_window *w = child[1]->guiobject_getRootWnd();
    if (w) WASABI_API_SKIN->group_destroy(w);
  }
}

int GroupXFade::onInit() {
  GROUPXFADE_PARENT::onInit();
  return 1;
}

int GroupXFade::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return GROUPXFADE_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch(xmlattributeid) {
    case GROUPXFADE_SETGROUP:
      setNewGroup(value);
      return 1;
    case GROUPXFADE_SETSPEED:
      speed = WTOF(value);
      return 1;
    default:
      return 0;
  }
}

int GroupXFade::onResize() {
  GROUPXFADE_PARENT::onResize();
  RECT r;
  getClientRect(&r);
  if (child[curchild]) 
    child[curchild]->guiobject_getRootWnd()->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
  int nextchild = curchild == 1 ? 0 : 1;
  if (child[nextchild]) 
    child[nextchild]->guiobject_getRootWnd()->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
  return 1;
}

void GroupXFade::setNewGroup(const wchar_t *grp) {
  if (child[curchild] && id[curchild].iscaseequal(grp)) return;
  if (child[curchild]) {
    child[curchild]->guiobject_setTargetA(0);
    child[curchild]->guiobject_setTargetSpeed((float)speed);
    child[curchild]->guiobject_gotoTarget();
  }
  int nextchild = curchild == 1 ? 0 : 1;
  if (child[nextchild]) {
    ifc_window *w = child[nextchild]->guiobject_getRootWnd();
    WASABI_API_SKIN->group_destroy(w);
    child[nextchild] = NULL;
  }
  if (grp && *grp) {
    ifc_window *w = WASABI_API_SKIN->group_create(grp);
    if (w) {
      child[nextchild] = w->getGuiObject();
      w->setParent(this);
      w->setStartHidden(1);
      RECT r;
      getClientRect(&r);
      w->setAlpha(0);
      w->init(this);
      w->setVisible(1);
      w->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
      child[nextchild]->guiobject_setTargetA(255);
      child[nextchild]->guiobject_setTargetSpeed((float)speed);
      child[nextchild]->guiobject_gotoTarget();
      id[nextchild] = grp;
    }
  }
  curchild = nextchild;
}
