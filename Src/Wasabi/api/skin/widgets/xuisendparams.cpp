#include <precomp.h>
#include "xuisendparams.h"
#include <api/script/scriptguid.h>

// -----------------------------------------------------------------------
const wchar_t SendParamsXuiObjectStr[] = L"SendParams"; // This is the xml tag
char SendParamsXuiSvcName[] = "SendParams xui object"; 

// -----------------------------------------------------------------------
SendParams::SendParams():myxuihandle(0) {
}

// -----------------------------------------------------------------------
SendParams::~SendParams() {
  pastlist.deleteAll();
}

// -----------------------------------------------------------------------
int SendParams::setXmlParam(const wchar_t *param, const wchar_t *value) 
{ 
  int r = SENDPARAMS_PARENT::setXmlParam(param, value);
  if (!WCSCASEEQLSAFE(param, L"group") && !WCSCASEEQLSAFE(param, L"target")) {
    Pair<StringW, StringW> *pair = new Pair<StringW, StringW>(param, value);
    pastlist.addItem(pair);
  }
  return r;
}

// -----------------------------------------------------------------------
void SendParams::actuator_onPerform(GuiObject *target) { // guaranteed non NULL
  SENDPARAMS_PARENT::actuator_onPerform(target);
  XmlObject *xtarget = static_cast<XmlObject *>(target->guiobject_getScriptObject()->vcpu_getInterface(xmlObjectGuid));
  foreach(pastlist)
    xtarget->setXmlParam(pastlist.getfor()->a, pastlist.getfor()->b);
  endfor;
}

