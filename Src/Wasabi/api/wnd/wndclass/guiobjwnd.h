#ifndef __GUIOBJWND_H
#define __GUIOBJWND_H

#if defined(WIN32) || defined(WIN64)
#define USEAPPBAR
#endif

#ifdef USEAPPBAR
#include <api/wnd/wndclass/appbarwnd.h>
#define GUIOBJECTWND_PARENT AppBarWnd
#else
#include <api/wnd/wndclass/clickwnd.h>
#define GUIOBJECTWND_PARENT ClickWnd
#endif

#include <api/script/scriptobj.h>
#include <api/script/objects/guiobject.h>
#include <api/script/objects/rootobj.h>
#include <api/skin/xmlobject.h>
#include <api/service/svcs/svc_xuiobject.h>

// {E5760861-5489-4ffc-BE02-061D9DA6CD1B}
const GUID guiObjectWndGuid =
    { 0xe5760861, 0x5489, 0x4ffc, { 0xbe, 0x2, 0x6, 0x1d, 0x9d, 0xa6, 0xcd, 0x1b } };

#define XUI_ATTRIBUTE_IMPLIED  XML_ATTRIBUTE_IMPLIED
#define XUI_ATTRIBUTE_REQUIRED XML_ATTRIBUTE_REQUIRED

class GuiObjectWnd : public GUIOBJECTWND_PARENT, public RootObjectInstance, public XmlObjectI
{
public:
	GuiObjectWnd();
	virtual ~GuiObjectWnd();

#ifdef WASABI_COMPILE_CONFIG
	virtual int onReloadConfig() { return 1; }
#endif

	// XmlObject

	virtual int setXmlParamById(int xmlhandle, int attrid, const wchar_t *name, const wchar_t *value);
	virtual int onUnknownXmlParam(const wchar_t *param, const wchar_t *value);
	virtual int newXuiHandle() { return newXmlHandle(); }

	// ClickWnd

	virtual int onRightButtonDown(int x, int y);
	virtual int onRightButtonUp(int x, int y);
	virtual int onLeftButtonDown(int x, int y);
	virtual int onLeftButtonUp(int x, int y);
	virtual int onMouseMove(int x, int y);
	virtual int onLeftButtonDblClk(int x, int y);
	virtual int onRightButtonDblClk(int x, int y);
	virtual int onMouseWheelUp(int click, int lines);
	virtual int onMouseWheelDown(int click, int lines);
	virtual int onResize();
	virtual int onActivate();
	virtual int onDeactivate();
	virtual void onEnterArea();
	virtual void onLeaveArea();
	virtual void onSetVisible(int show);
	virtual int onEnable(int en);
	virtual void onCancelCapture();
	virtual int dragDrop(ifc_window *sourceWnd, int x, int y);
	virtual int acceptExternalDrops() { return 1; }
	virtual int dragEnter(ifc_window *sourceWnd);
	virtual int dragOver(int x, int y, ifc_window *sourceWnd);
	virtual int dragLeave(ifc_window *sourceWnd);
	virtual void *getInterface(GUID g);
	virtual int onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source);
	virtual const wchar_t *getId();
	virtual int onAcceleratorEvent(const wchar_t *name);

	virtual void setContent(const wchar_t *groupid_orguid, int autoresizefromcontent = 0);
	virtual void setContentBySkinItem(SkinItem *item, int autoresizefromcontent = 0);
	virtual void onNewContent() {}

	// AbstractWndHolder
	virtual void abstract_onNewContent();


	virtual int onUnknownXuiParam(const wchar_t *param, const wchar_t *value) { return 0; }

	virtual GuiObject *findObject(const wchar_t *object_id);
#ifdef WASABI_COMPILE_SCRIPT
	virtual ScriptObject *findScriptObject(const wchar_t *object_id);
#endif
#ifdef WASABI_COMPILE_SKIN
	virtual GuiObject *getContent() { return abstract_getContent(); }
	virtual ScriptObject *getContentScriptObject() { return abstract_getContentScriptObject(); }
	virtual ifc_window *getContentRootWnd() { return abstract_getContentRootWnd(); }
#endif

	// BaseWnd

	virtual int onInit();
	virtual int onPostOnInit();
	virtual int onChar(unsigned int c);
	virtual int onKeyDown(int vkcode);
	virtual int onKeyUp(int vkcode);

	virtual int onGetFocus();
	virtual int onKillFocus();
	virtual int wantFocus();

	const wchar_t *getTip();
	// GuiObjectWnd
	int wantTranslation();
	GuiObject *getGuiObject();
	RootObject *getRootObject();
	int cfg_reentry;

	// XuiObject
	virtual int setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *value);

protected:
	/*static */void CreateXMLParameters(int master_handle);
private:
	GuiObject *my_gui_object;
	int xuihandle;
	static XMLParamPair params[];
	

public:

	enum {
	    GUIOBJECT_ID = 0,
	    GUIOBJECT_ALPHA,
	    GUIOBJECT_ACTIVEALPHA,
	    GUIOBJECT_INACTIVEALPHA,
	    GUIOBJECT_SYSREGION,
	    GUIOBJECT_RECTRGN,
	    GUIOBJECT_TOOLTIP,
	    GUIOBJECT_SYSMETRICSX,
	    GUIOBJECT_SYSMETRICSY,
	    GUIOBJECT_SYSMETRICSW,
	    GUIOBJECT_SYSMETRICSH,
	    GUIOBJECT_MOVE,
	    GUIOBJECT_RENDERBASETEXTURE,
	    GUIOBJECT_CFGATTR,
	    GUIOBJECT_X,
	    GUIOBJECT_Y,
	    GUIOBJECT_W,
	    GUIOBJECT_H,
	    GUIOBJECT_VISIBLE,
	    GUIOBJECT_ENABLED,
	    GUIOBJECT_RELATX,
	    GUIOBJECT_RELATY,
	    GUIOBJECT_RELATW,
	    GUIOBJECT_RELATH,
	    GUIOBJECT_DROPTARGET,
	    GUIOBJECT_GHOST,
	    GUIOBJECT_NOTIFY,
	    GUIOBJECT_FOCUSONCLICK,
	    GUIOBJECT_TABORDER,
	    GUIOBJECT_WANTFOCUS,
	    GUIOBJECT_SETNODBLCLICK,
	    GUIOBJECT_SETNOLEFTCLICK,
	    GUIOBJECT_SETNORIGHTCLICK,
	    GUIOBJECT_SETNOMOUSEMOVE,
	    GUIOBJECT_SETNOCONTEXTMENU,
	    GUIOBJECT_SETX1,
	    GUIOBJECT_SETY1,
	    GUIOBJECT_SETX2,
	    GUIOBJECT_SETY2,
	    GUIOBJECT_SETANCHOR,
	    GUIOBJECT_SETCURSOR,
	    GUIOBJECT_FITTOPARENT,
	    GUIOBJECT_USERDATA,
#ifdef USEAPPBAR
	    GUIOBJECT_APPBAR,
#endif
			GUIOBJECT_TRANSLATE,
	    GUIOBJECT_NUMPARAMS
	};


};

template <class T, const wchar_t XMLTAG[], char SVCNAME[]>
class XuiObjectSvc : public svc_xuiObjectI
{
public:
	static const wchar_t *xuisvc_getXmlTag() { return XMLTAG; }

	int testTag(const wchar_t *xmltag)
	{
		if (!WCSICMP(xmltag, XMLTAG)) return 1;
		return 0;
	}
	GuiObject *instantiate(const wchar_t *xmltag, ifc_xmlreaderparams *params = NULL)
	{
		if (testTag(xmltag))
		{
			T * obj = new T;
			ASSERT(obj != NULL);
			return obj->getGuiObject();
		}
		return NULL;
	}
	void destroy(GuiObject *g)
	{
		T *obj = static_cast<T *>(g->guiobject_getRootWnd());
		delete obj;
	}
	static const char *getServiceName() { return SVCNAME; }
};

template <class T>
class XuiObjectSvc2 : public svc_xuiObjectI
{
public:
	static const wchar_t *xuisvc_getXmlTag() { return T::xuiobject_getXmlTag(); }
	int testTag(const wchar_t *xmltag)
	{
		if (!WCSICMP(xmltag, T::xuiobject_getXmlTag())) return 1;
		return 0;
	}
	GuiObject *instantiate(const wchar_t *xmltag, ifc_xmlreaderparams *params = NULL)
	{
		if (testTag(xmltag))
		{
			T * obj = new T;
			ASSERT(obj != NULL);
			return obj->getGuiObject();
		}
		return NULL;
	}
	void destroy(GuiObject *g)
	{
		T *obj = static_cast<T *>(g->guiobject_getRootWnd());
		delete obj;
	}
	static const char *getServiceName() { return T::xuiobject_getServiceName(); }
};

#endif
