#ifndef _XUIOBJDIRWND_H
#define _XUIOBJDIRWND_H

#include <api/skin/widgets/objdirwnd.h>

#define SCRIPTOBJDIRWND_PARENT ObjDirWnd
class ScriptObjDirWnd : public SCRIPTOBJDIRWND_PARENT {
public:
  ScriptObjDirWnd();
  virtual ~ScriptObjDirWnd();

  virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);
protected:
	/*static */void CreateXMLParameters(int master_handle);
private:
	
enum {
  SCRIPTOBJDIRWND_DIR=10,
  SCRIPTOBJDIRWND_ACTION_TARGET=20,
  DISPLAYTARGET,
  DEFAULTDISPLAY,
  FORCEVIRTUAL,
};
static XMLParamPair params[];
  int myxuihandle;
};

extern const wchar_t ScriptObjDirWndXuiObjectStr[];
extern char ScriptObjDirWndXuiSvcName[];
class ScriptObjDirWndXuiSvc : public XuiObjectSvc<ScriptObjDirWnd, ScriptObjDirWndXuiObjectStr, ScriptObjDirWndXuiSvcName> {};

#endif
