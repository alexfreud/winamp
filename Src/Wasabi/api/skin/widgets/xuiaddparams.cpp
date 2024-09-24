#include <precomp.h>

#include "xuiaddparams.h"
#include <api/script/scriptguid.h>

// -----------------------------------------------------------------------
const wchar_t AddParamsXuiObjectStr[] = L"AddParams"; // This is the xml tag
char AddParamsXuiSvcName[] = "AddParams xui object"; 

// -----------------------------------------------------------------------
AddParams::AddParams():myxuihandle(0) {
}

// -----------------------------------------------------------------------
AddParams::~AddParams() {
  pastlist.deleteAll();
}

// -----------------------------------------------------------------------
int AddParams::setXmlParam(const wchar_t *param, const wchar_t *value) 
{
  int r = ADDPARAMS_PARENT::setXmlParam(param, value);
  if (!WCSCASEEQLSAFE(param, L"group") && !WCSCASEEQLSAFE(param, L"target")) {
    Pair<StringW, StringW> *pair = new Pair<StringW, StringW>(param, value);
    pastlist.addItem(pair);
  }
  return r;
}

// -----------------------------------------------------------------------
void AddParams::actuator_onPerform(GuiObject *target) { // guaranteed non NULL
  ADDPARAMS_PARENT::actuator_onPerform(target);
  XmlObject *xtarget = static_cast<XmlObject *>(target->guiobject_getScriptObject()->vcpu_getInterface(xmlObjectGuid));
  foreach(pastlist)
    const wchar_t *a = pastlist.getfor()->a;
    int xp = xtarget->getXmlParam(a);
    StringW newval((xp == -1) ? L"" : xtarget->getXmlParamValue(xp));
    newval.cat(pastlist.getfor()->b);
    xtarget->setXmlParam(a, newval);
  endfor;
}
