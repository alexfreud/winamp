#ifndef __SCRIPTTITLEBOX_H
#define __SCRIPTTITLEBOX_H

#include <api/skin/widgets/titlebox.h>
#include <api/script/objects/c_script/h_guiobject.h>
#include <api/wnd/accessible.h>

#define  SCRIPTTITLEBOX_PARENT TitleBox

// -----------------------------------------------------------------------
// Your wnd object class
class ScriptTitleBox : public SCRIPTTITLEBOX_PARENT {
  
  public:

    ScriptTitleBox();
    virtual ~ScriptTitleBox();

    // XuiObject automatically calls this back for all parameters registered using addParam
    // encountered in the xml source
    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:

    // a list of IDs for our xml attributes, we use them in addParam() in the constructor
    enum {
       SCRIPTTITLEBOX_TITLE= 0,
       SCRIPTTITLEBOX_CONTENT,
       SCRIPTTITLEBOX_CENTERED,
       SCRIPTTITLEBOX_SUFFIX,
    };
    int myxuihandle;
		static XMLParamPair params[];
};

// -----------------------------------------------------------------------
// This defines the svc_xuiObject that exposes your wnd object

extern const wchar_t ScriptTitleBoxXuiObjectStr[];
extern char ScriptTitleBoxXuiSvcName[];
class ScriptTitleBoxXuiSvc : public XuiObjectSvc<ScriptTitleBox, ScriptTitleBoxXuiObjectStr, ScriptTitleBoxXuiSvcName> {};

#endif
