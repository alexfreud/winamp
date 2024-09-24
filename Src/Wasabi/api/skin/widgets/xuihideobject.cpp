#include <precomp.h>
#include "xuihideobject.h"
// -----------------------------------------------------------------------
const wchar_t HideObjectXuiObjectStr[] = L"HideObject"; // This is the xml tag
char HideObjectXuiSvcName[] = "HideObject xui object"; 

XMLParamPair HideObject::params[] = {
  {     HIDEOBJECT_HIDE, L"HIDE"},
};
// -----------------------------------------------------------------------
HideObject::HideObject() : HIDEOBJECT_PARENT() 
{
  myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);  
}

void HideObject::CreateXMLParameters(int master_handle)
{
	//HIDEOBJECT_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}


// -----------------------------------------------------------------------
int HideObject::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return HIDEOBJECT_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) {
    case HIDEOBJECT_HIDE:
      actuator_setTarget(value);
      break;
    default:
      return 0;
  }
  return 1; 
}

// -----------------------------------------------------------------------
void HideObject::actuator_onPerform(GuiObject *target) { // guaranteed non NULL
  ifc_window *w = target->guiobject_getRootWnd();
  if (w != NULL) {
    w->setVisible(0);
  }
}


