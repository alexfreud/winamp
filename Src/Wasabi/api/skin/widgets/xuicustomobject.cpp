#include <precomp.h>
#include "xuicustomobject.h"
#include <api/wnd/notifmsg.h>

// -----------------------------------------------------------------------
const wchar_t CustomObjectXuiObjectStr[] = L"CustomObject"; // This is the xml tag
char CustomObjectXuiSvcName[] = "CustomObject xui object"; 

XMLParamPair XuiCustomObject::params[] = {
	{CUSTOMOBJECT_SETGROUP, L"GROUPID"},
};

// -----------------------------------------------------------------------
XuiCustomObject::XuiCustomObject() 
{
  myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);	

  ScriptObject *o = getGuiObject()->guiobject_getScriptObject();
  o->vcpu_setInterface(customObjectGuid, static_cast<CustomObject *>(this));

}

void XuiCustomObject::CreateXMLParameters(int master_handle)
{
	//CUSTOMOBJECT_PARENT::CreateXMLParameters(master_handle);
		int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);

}
// -----------------------------------------------------------------------
XuiCustomObject::~XuiCustomObject() {
}

// -----------------------------------------------------------------------
int XuiCustomObject::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return CUSTOMOBJECT_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) {
    case  CUSTOMOBJECT_SETGROUP:
      setContent(value);
      break;
    default:
      return 0;
  }
  return 1;
}

// -----------------------------------------------------------------------
void XuiCustomObject::setGroup(const wchar_t *elementname) {
  setContent(elementname);
}

// -----------------------------------------------------------------------
void XuiCustomObject::customobject_setRootWnd(ifc_window *w) {
  rootwndholder_setRootWnd(NULL);
  groupid = L"";
  setContent(groupid);
  if (w != NULL) rootwndholder_setRootWnd(w);
  notifyParent(ChildNotify::AUTOWHCHANGED);
}
