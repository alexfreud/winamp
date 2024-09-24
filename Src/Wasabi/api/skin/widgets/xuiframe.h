#ifndef __XUIFRAME_H
#define __XUIFRAME_H

#include <api/wnd/wndclass/framewnd.h>
#include <api/script/objects/guiobj.h>

#define SCRIPTFRAME_PARENT FrameWnd 

/* --------- Script Object for ScriptFrame --------- */
class FrameScriptController : public GuiObjectScriptController 
{
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return guiController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern FrameScriptController *frameController;

// -----------------------------------------------------------------------
// Your wnd object class

class ScriptFrame : public SCRIPTFRAME_PARENT {
  
  public:

    ScriptFrame();
    virtual ~ScriptFrame();

    virtual int onInit();

    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    void setOrientation(const wchar_t *elementname);
    void setLeft(const wchar_t *groupname);
    void setRight(const wchar_t *groupname);
    void setFrom(const wchar_t *from);
    void setWidth(const wchar_t *w);
    void setResize(const wchar_t *r);

    virtual int wantRenderBaseTexture() { return 0; }

    void onResizeChildren(RECT leftr, RECT rightr);

		virtual const wchar_t *vcpu_getClassName();
		virtual ScriptObjectController *vcpu_getController() { return frameController; }
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:

    // a list of IDs for our xml attributes, we use them in addParam() in the constructor
    enum {
      SCRIPTFRAME_SETORIENTATION = 0,
      SCRIPTFRAME_SETLEFT,
      SCRIPTFRAME_SETRIGHT,
      SCRIPTFRAME_SETFROM,
      SCRIPTFRAME_SETWIDTH,
      SCRIPTFRAME_SETRESIZEABLE,
			SCRIPTFRAME_SETMAXWIDTH,
			SCRIPTFRAME_SETMINWIDTH,
			SCRIPTFRAME_SETSNAP,
			SCRIPTFRAME_SETV_BITMAP,
			SCRIPTFRAME_SETV_GRABBER,
    };
		static XMLParamPair params[];
    int myxuihandle;
    StringW left, right;
    ifc_window *rootwndleft, *rootwndright;
    int from, orientation, resizable;
    int width;
		public:
  static scriptVar script_vcpu_setPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t);
	static scriptVar script_vcpu_getPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};


// -----------------------------------------------------------------------
// This defines the svc_xuiObject that exposes your wnd object

extern const wchar_t ScriptFrameXuiObjectStr[];
extern char ScriptFrameXuiSvcName[];
class FrameXuiSvc : public XuiObjectSvc<ScriptFrame, ScriptFrameXuiObjectStr, ScriptFrameXuiSvcName> {};

#endif
