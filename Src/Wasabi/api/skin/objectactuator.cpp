#include <precomp.h>
#include "objectactuator.h"
#include <api/console/console.h>
#include <bfc/parse/paramparser.h>
#include <api/script/scriptguid.h>

XMLParamPair ObjectActuator::params[]=
{
	{OBJECTACTUATOR_GROUP, L"GROUP"},
	{OBJECTACTUATOR_TARGET, L"TARGET"}
};

// -----------------------------------------------------------------------
ObjectActuator::ObjectActuator() 
{
  myxuihandle = newXuiHandle();
  CreateXMLParameters(myxuihandle);
}

void ObjectActuator::CreateXMLParameters(int master_handle)
{
	//OBJECTACTUATOR_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

// -----------------------------------------------------------------------
ObjectActuator::~ObjectActuator() {
}

// -----------------------------------------------------------------------
int ObjectActuator::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return OBJECTACTUATOR_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) {
    case OBJECTACTUATOR_TARGET:
			if (actuator_wantTargetParam())
				actuator_setTarget(value);
      break;
    case OBJECTACTUATOR_GROUP:
			if (actuator_wantGroupParam())
				actuator_setGroup(value);
      break;
    default:
      return 0;
  }
  return 1; 
}

// -----------------------------------------------------------------------
void ObjectActuator::actuator_setTarget(const wchar_t *value) {
  objectsid = value;
  if (isInited() && actuator_wantAutoPerform())
    performActions();
}

// -----------------------------------------------------------------------
void ObjectActuator::actuator_setGroup(const wchar_t *value) {
  groupid = value;
}

// -----------------------------------------------------------------------
int ObjectActuator::onInit() {
  int rt = OBJECTACTUATOR_PARENT::onInit();
  
  if (actuator_wantAutoPerform()) performActions();

  return rt;
}


// -----------------------------------------------------------------------
void ObjectActuator::performActions() 
{
  ifc_window *group = getParent();
  
  if (!groupid.isempty()) 
	{
    GuiObject *o = getGuiObject()->guiobject_findObject(groupid);
    if (o != NULL) {
      group = o->guiobject_getRootWnd();
    }
  }

  GuiObject *go = static_cast<GuiObject *>(group->getInterface(guiObjectGuid));
  if (go == NULL) {
    DebugStringW(L"%s:group:%s\n", getActuatorTag(), groupid.getValue());
    return;
  }

  ParamParser pp(objectsid);
  for (int i=0;i<pp.getNumItems();i++) {
    GuiObject *target = go->guiobject_findObject(pp.enumItem(i));
    if (target != NULL)
      actuator_onPerform(target);
    else
      DebugStringW(L"%s:%s/%s\n", getActuatorTag(), groupid.getValue(), objectsid.getValue());
  }
}

const wchar_t *ObjectActuator::getActuatorTag() {
  return L"ObjectActuator";
}
