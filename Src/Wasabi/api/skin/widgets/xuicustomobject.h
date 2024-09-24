#ifndef __CustomObject_H
#define __CustomObject_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/skin/widgets/customobject.h>

#define  CUSTOMOBJECT_PARENT GuiObjectWnd

// -----------------------------------------------------------------------
class XuiCustomObject : public  CUSTOMOBJECT_PARENT, public CustomObjectI {
  
  public:
                             
    XuiCustomObject();
    virtual ~XuiCustomObject();

    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    void setGroup(const wchar_t *elementname);
    virtual void customobject_setRootWnd(ifc_window *w);
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:

    enum {
       CUSTOMOBJECT_SETGROUP        = 0,
    };
		static XMLParamPair params[];
    int myxuihandle;
    StringW groupid;
};


// -----------------------------------------------------------------------

extern const wchar_t CustomObjectXuiObjectStr[];
extern char CustomObjectXuiSvcName[];
class CustomObjectXuiSvc : public XuiObjectSvc<XuiCustomObject, CustomObjectXuiObjectStr, CustomObjectXuiSvcName> {};

#endif
