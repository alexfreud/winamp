#include <precomp.h>

#include "xuiobjdirwnd.h"

const wchar_t ScriptObjDirWndXuiObjectStr[] = L"ObjDirView"; // This is the xml tag
char ScriptObjDirWndXuiSvcName[] = "ObjDirView xui object";

XMLParamPair ScriptObjDirWnd::params[] = {
	{DEFAULTDISPLAY, L"DEFAULTDISPLAY"},
  {SCRIPTOBJDIRWND_DIR, L"DIR"},
		  {DISPLAYTARGET, L"DISPLAYTARGET"},
			{FORCEVIRTUAL, L"FORCEVIRTUAL"},
  {SCRIPTOBJDIRWND_ACTION_TARGET, L"TARGET"},
	};

ScriptObjDirWnd::ScriptObjDirWnd() 
{
  myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);
}

void ScriptObjDirWnd::CreateXMLParameters(int master_handle)
{
	//SCRIPTOBJDIRWND_PARENT::CreateXMLParameters(master_handle);
		int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

ScriptObjDirWnd::~ScriptObjDirWnd() { }

int ScriptObjDirWnd::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return SCRIPTOBJDIRWND_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);
  switch (xmlattributeid) {
    case SCRIPTOBJDIRWND_DIR:
      setTargetDirName(value);
    break;
    case SCRIPTOBJDIRWND_ACTION_TARGET:
      setActionTarget(value);
    break;
    case DISPLAYTARGET:
      setDisplayTarget(value);
    break;
    case DEFAULTDISPLAY:
      setDefaultDisplay(value);
    break;
    case FORCEVIRTUAL:
      setVirtual(WTOI(value));
    break;
    default:
      return 0;
  }
  return 1;
}

