#include <precomp.h>
#include "xuicheckbox.h"
#include <bfc/parse/paramparser.h>
#include <api/script/objects/c_script/c_text.h>

// -----------------------------------------------------------------------
const wchar_t ScriptCheckBoxXuiObjectStr[] = L"Wasabi:CheckBox"; // This is the xml tag
char ScriptCheckBoxXuiSvcName[] = "Wasabi:CheckBox xui object"; 

XMLParamPair ScriptCheckBox::params[] = {
	{SCRIPTCHECKBOX_ACTION, L"ACTION"},
  {SCRIPTCHECKBOX_ACTIONTARGET, L"ACTION_TARGET"},
  {SCRIPTCHECKBOX_ACTIONPARAM, L"PARAM"},
{SCRIPTCHECKBOX_RADIOID, L"RADIOID"},
  {SCRIPTCHECKBOX_RADIOVAL, L"RADIOVAL"},
	{SCRIPTCHECKBOX_TEXT, L"TEXT"},
  
	};
// -----------------------------------------------------------------------
ScriptCheckBox::ScriptCheckBox() : SCRIPTCHECKBOX_PARENT() {
  getScriptObject()->vcpu_setInterface(checkBoxGuid, (void *)static_cast<ScriptCheckBox*>(this));
  getScriptObject()->vcpu_setClassName(L"CheckBox"); // this is the script class name
  getScriptObject()->vcpu_setController(checkBoxController);

  myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);
}


void ScriptCheckBox::CreateXMLParameters(int master_handle)
{
	//SCRIPTCHECKBOX_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

// -----------------------------------------------------------------------
ScriptCheckBox::~ScriptCheckBox() {
}

// -----------------------------------------------------------------------
int ScriptCheckBox::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return SCRIPTCHECKBOX_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  // Parcel the values out to the window object we multiply inherit from
  switch (xmlattributeid) {
    case  SCRIPTCHECKBOX_TEXT:
      setText(value);
    break;
    case  SCRIPTCHECKBOX_RADIOID:
      setRadioid(value);
    break;
    case  SCRIPTCHECKBOX_RADIOVAL:
      setRadioVal(value);
    break;
    case  SCRIPTCHECKBOX_ACTION:
      setAction(value);
    break;
    case  SCRIPTCHECKBOX_ACTIONPARAM:
      setActionParam(value);
    break;
    case  SCRIPTCHECKBOX_ACTIONTARGET:
      setActionTarget(value);
    break;
    default:
      return 0;
  }
  return 1;
}

void ScriptCheckBox::onToggle() {
  SCRIPTCHECKBOX_PARENT::onToggle();
  Accessible *a = getAccessibleObject();
  if (a != NULL) {
    a->onStateChange();
  }
  CheckBoxController::onToggle(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(isActivated()));
}

// -----------------------------------------------------------------------
// Script Object

CheckBoxController _checkBoxController;
CheckBoxController *checkBoxController = &_checkBoxController;

// -- Functions table -------------------------------------
function_descriptor_struct CheckBoxController::exportedFunction[] = {
  {L"onToggle",            1, (void*)CheckBoxController::onToggle},
  {L"setChecked",          1, (void*)CheckBoxController::setChecked},
  {L"isChecked",           0, (void*)CheckBoxController::isChecked},
  {L"setText",             1, (void*)CheckBoxController::setText},
  {L"getText",             0, (void*)CheckBoxController::getText},
};

ScriptObject *CheckBoxController::instantiate() {
  ScriptCheckBox *sb = new ScriptCheckBox;
  ASSERT(sb != NULL);
  return sb->getScriptObject();
}

void CheckBoxController::destroy(ScriptObject *o) {
  ScriptCheckBox *sb = static_cast<ScriptCheckBox *>(o->vcpu_getInterface(checkBoxGuid));
  ASSERT(sb != NULL);
  delete sb;
}

void *CheckBoxController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for checkboxes yet
}

void CheckBoxController::deencapsulate(void *o) {
}

int CheckBoxController::getNumFunctions() { 
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *CheckBoxController::getExportedFunctions() { 
  return exportedFunction; 
}


scriptVar CheckBoxController::onToggle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar newstate) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, checkBoxController, newstate);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, newstate);
}

scriptVar CheckBoxController::setChecked(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar state) {
  SCRIPT_FUNCTION_INIT
  ScriptCheckBox *sb = static_cast<ScriptCheckBox *>(o->vcpu_getInterface(checkBoxGuid));
  if (sb) sb->setActivated(GET_SCRIPT_INT(state));
  RETURN_SCRIPT_VOID;  
}

scriptVar CheckBoxController::isChecked(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  int a = 0;
  ScriptCheckBox *sb = static_cast<ScriptCheckBox *>(o->vcpu_getInterface(checkBoxGuid));
  if (sb) a = sb->isActivated();
  return MAKE_SCRIPT_INT(a);  
}

scriptVar CheckBoxController::setText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar text) {
  SCRIPT_FUNCTION_INIT
  ScriptCheckBox *sb = static_cast<ScriptCheckBox *>(o->vcpu_getInterface(checkBoxGuid));
  if (sb)
		sb->setText(GET_SCRIPT_STRING(text));
  RETURN_SCRIPT_VOID;  
}

scriptVar CheckBoxController::getText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptCheckBox *sb = static_cast<ScriptCheckBox *>(o->vcpu_getInterface(checkBoxGuid));
  if (sb) 
	{
		 return MAKE_SCRIPT_STRING(sb->getText());  
		
	}
  return MAKE_SCRIPT_STRING(L"");  
}
