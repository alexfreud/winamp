#include <precomp.h>
#include <api/script/api_maki.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/scriptobj.h>
#include <api/script/scriptguid.h>
#include <api/service/svcs/svc_droptarget.h>
#include <api/wnd/notifmsg.h>
#include <api/script/objects/rootobject.h>
#include <api/locales/xlatstr.h>

XMLParamPair GuiObjectWnd::params[] =
    {
        {GUIOBJECT_ACTIVEALPHA, L"ACTIVEALPHA"},
        {GUIOBJECT_ALPHA, L"ALPHA"},
        {GUIOBJECT_SETANCHOR, L"ANCHOR"},
#ifdef USEAPPBAR
        {GUIOBJECT_APPBAR, L"APPBAR"},
#endif
        {GUIOBJECT_CFGATTR, L"CFGATTRIB"},
        {GUIOBJECT_SETCURSOR, L"CURSOR"},
        {GUIOBJECT_DROPTARGET, L"DROPTARGET"},
        {GUIOBJECT_ENABLED, L"ENABLED"},
        {GUIOBJECT_FITTOPARENT, L"FITPARENT"},
        {GUIOBJECT_FOCUSONCLICK, L"FOCUSONCLICK"},
        {GUIOBJECT_GHOST, L"GHOST"},
        {GUIOBJECT_H, L"H"},
        {GUIOBJECT_ID, L"ID"},
        {GUIOBJECT_INACTIVEALPHA, L"INACTIVEALPHA"},
        {GUIOBJECT_MOVE, L"MOVE"},
        {GUIOBJECT_SETNOCONTEXTMENU, L"NOCONTEXTMENU"},
        {GUIOBJECT_SETNODBLCLICK, L"NODBLCLICK"},
        {GUIOBJECT_SETNOLEFTCLICK, L"NOLEFTCLICK"},
				{GUIOBJECT_SETNOMOUSEMOVE, L"NOMOUSEMOVE"},
        {GUIOBJECT_SETNORIGHTCLICK, L"NORIGHTCLICK"},
        {GUIOBJECT_NOTIFY, L"NOTIFY"},
        {GUIOBJECT_NOTIFY, L"NOTIFY0"},
        {GUIOBJECT_NOTIFY, L"NOTIFY1"},
        {GUIOBJECT_NOTIFY, L"NOTIFY2"},
        {GUIOBJECT_NOTIFY, L"NOTIFY3"},
        {GUIOBJECT_NOTIFY, L"NOTIFY4"},
        {GUIOBJECT_NOTIFY, L"NOTIFY5"},
        {GUIOBJECT_NOTIFY, L"NOTIFY6"},
        {GUIOBJECT_NOTIFY, L"NOTIFY7"},
        {GUIOBJECT_NOTIFY, L"NOTIFY8"},
        {GUIOBJECT_NOTIFY, L"NOTIFY9"},
        {GUIOBJECT_RECTRGN, L"RECTRGN"},
        {GUIOBJECT_SYSREGION, L"REGIONOP"},
        {GUIOBJECT_RELATH, L"RELATH"},
        {GUIOBJECT_RELATW, L"RELATW"},
        {GUIOBJECT_RELATX, L"RELATX"},
        {GUIOBJECT_RELATY, L"RELATY"},
        {GUIOBJECT_RENDERBASETEXTURE, L"RENDERBASETEXTURE"},
        {GUIOBJECT_SYSMETRICSX, L"SYSMETRICSX"},
        {GUIOBJECT_SYSMETRICSY, L"SYSMETRICSY"},
        {GUIOBJECT_SYSMETRICSW, L"SYSMETRICSW"},
        {GUIOBJECT_SYSMETRICSH, L"SYSMETRICSH"},
        {GUIOBJECT_SYSREGION, L"SYSREGION"},
        {GUIOBJECT_TABORDER, L"TABORDER"},
        {GUIOBJECT_TOOLTIP, L"TOOLTIP"},
				{GUIOBJECT_TRANSLATE, L"TRANSLATE"},
        {GUIOBJECT_USERDATA, L"USERDATA"},
        {GUIOBJECT_VISIBLE, L"VISIBLE"},
        {GUIOBJECT_W, L"W"},
        {GUIOBJECT_WANTFOCUS, L"WANTFOCUS"},
        {GUIOBJECT_X, L"X"},
        {GUIOBJECT_SETX1, L"X1"},
        {GUIOBJECT_SETX2, L"X2"},
        {GUIOBJECT_Y, L"Y"},
        {GUIOBJECT_SETY1, L"Y1"},
        {GUIOBJECT_SETY2, L"Y2"},
    };

GuiObjectWnd::GuiObjectWnd()
{
	my_gui_object = static_cast<GuiObject *>(WASABI_API_MAKI->maki_encapsulate(guiObjectGuid, getScriptObject()));
	getScriptObject()->vcpu_setInterface(xmlObjectGuid, static_cast<XmlObject *>(this));
	getScriptObject()->vcpu_setInterface(guiObjectWndGuid, static_cast<GuiObjectWnd *>(this));
#ifdef USEAPPBAR
	getScriptObject()->vcpu_setInterface(appBarGuid, static_cast<AppBar*>(this));
#endif
	my_gui_object->guiobject_setRootWnd(this);
	getScriptObject()->vcpu_setClassName(L"GuiObject");
	getScriptObject()->vcpu_setController(WASABI_API_MAKI->maki_getController(guiObjectGuid));
	cfg_reentry = 0;
	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);
}

void GuiObjectWnd::CreateXMLParameters(int master_handle)
{
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

GuiObjectWnd::~GuiObjectWnd()
{
	WASABI_API_MAKI->maki_deencapsulate(guiObjectGuid, my_gui_object);
	my_gui_object = NULL;
}

const wchar_t *GuiObjectWnd::getTip()
{
	switch(wantTranslation())
	{
	case 1:
		return _(tip);
	case 2:
return __(tip);
	default:
		return tip;
	}
}

int GuiObjectWnd::onRightButtonDown(int x, int y)
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onRightButtonDown(x, y);
	my_gui_object->guiobject_onRightButtonDown(x, y);
	return 1;
}

int GuiObjectWnd::onRightButtonUp(int x, int y)
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onRightButtonUp(x, y);
	my_gui_object->guiobject_onRightButtonUp(x, y);
	return 1;
}

int GuiObjectWnd::onLeftButtonDown(int x, int y)
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onLeftButtonDown(x, y);
	my_gui_object->guiobject_onLeftButtonDown(x, y);
	return 1;
}

int GuiObjectWnd::onLeftButtonUp(int x, int y)
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onLeftButtonUp(x, y);
	my_gui_object->guiobject_onLeftButtonUp(x, y);
	return 1;
}

int GuiObjectWnd::onMouseMove(int x, int y)
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onMouseMove(x, y);
	my_gui_object->guiobject_onMouseMove(x, y);
	return 1;
}

int GuiObjectWnd::wantTranslation()
{
		if (!my_gui_object) return 1;
		return my_gui_object->guiobject_wantTranslation();
}

int GuiObjectWnd::onLeftButtonDblClk(int x, int y)
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onLeftButtonDblClk(x, y);
	my_gui_object->guiobject_onLeftButtonDblClk(x, y);
	return 1;
}

int GuiObjectWnd::onRightButtonDblClk(int x, int y)
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onRightButtonDblClk(x, y);
	my_gui_object->guiobject_onRightButtonDblClk(x, y);
	return 1;
}

// Martin> For the next two functions, we need to ensure that we don't kill volume change if nothing is done
int GuiObjectWnd::onMouseWheelDown (int click, int line)
{
	if (!my_gui_object) return 1;
	int ret = GUIOBJECTWND_PARENT::onMouseWheelDown(click, line);
	if (!ret) ret = my_gui_object->guiobject_onMouseWheelDown(click, line);
	return ret;
}

int GuiObjectWnd::onMouseWheelUp (int click, int line)
{
	if (!my_gui_object) return 1;
	int ret = GUIOBJECTWND_PARENT::onMouseWheelUp(click, line);
	if (!ret) ret = my_gui_object->guiobject_onMouseWheelUp(click, line);
	return ret;
}

int GuiObjectWnd::onResize()
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onResize();
	if (!isInited()) return 1;
	ifc_window *w = my_gui_object->guiobject_getRootWnd();
	RECT r;
	w->getClientRect(&r);
	my_gui_object->guiobject_onResize(r.left, r.top, r.right - r.left, r.bottom - r.top);
	return 1;
}

void GuiObjectWnd::onEnterArea()
{
	if (!my_gui_object) return ;
	GUIOBJECTWND_PARENT::onEnterArea();
	my_gui_object->guiobject_onEnterArea();
}

void GuiObjectWnd::onLeaveArea()
{
	if (!my_gui_object) return ;
	GUIOBJECTWND_PARENT::onLeaveArea();
	my_gui_object->guiobject_onLeaveArea();
}

void GuiObjectWnd::onSetVisible(int show)
{
	if (!my_gui_object)
		return ;

	GUIOBJECTWND_PARENT::onSetVisible(show);
	my_gui_object->guiobject_onSetVisible(show);
}

int GuiObjectWnd::onEnable(int en)
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onEnable(en);
	my_gui_object->guiobject_onEnable(en);
	return 1;
}

GuiObject *GuiObjectWnd::getGuiObject()
{
	return my_gui_object;
}

RootObject *GuiObjectWnd::getRootObject()
{
	return my_gui_object->guiobject_getRootObject();
}

int GuiObjectWnd::dragDrop(ifc_window *sourceWnd, int x, int y)
{
	int r = DropTargetEnum::throwDrop(my_gui_object->guiobject_getDropTarget(), sourceWnd, x, y);
	if (r == 0)
	{
		ifc_window *p = getParent();
		if (p != NULL)
		{
			DragInterface *d = p->getDragInterface();
			if (d != NULL)
				return d->dragDrop(sourceWnd, x, y);
		}
	}
	return r;
}

int GuiObjectWnd::dragEnter(ifc_window *sourceWnd) 
{
	my_gui_object->guiobject_dragEnter(sourceWnd);
	return 1; 
}

int GuiObjectWnd::dragOver(int x, int y, ifc_window *sourceWnd) 
{
	my_gui_object->guiobject_dragOver(x, y, sourceWnd);
	return 1; 
}

int GuiObjectWnd::dragLeave(ifc_window *sourceWnd) 
{
	my_gui_object->guiobject_dragLeave(sourceWnd);
	return 1; 
}

int GuiObjectWnd::onActivate()
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onActivate();
	invalidate();
	return 1;
}

int GuiObjectWnd::onDeactivate()
{
	if (!my_gui_object) return 1;
	GUIOBJECTWND_PARENT::onDeactivate();
	invalidate();
	return 1;
}

void GuiObjectWnd::onCancelCapture()
{
	if (!my_gui_object) return ;
	GUIOBJECTWND_PARENT::onCancelCapture();
	my_gui_object->guiobject_onCancelCapture();
}

int GuiObjectWnd::setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *value)
{
	if (_xuihandle == xuihandle)
	{
		switch (attrid)
		{
		case GUIOBJECT_FOCUSONCLICK:
			setFocusOnClick(WTOI(value));
			break;
		default:
			getGuiObject()->guiobject_setXmlParamById(attrid, value);
			break;
		}
	}
	return 0;
}

int GuiObjectWnd::onUnknownXmlParam(const wchar_t *param, const wchar_t *value)
{
	return onUnknownXuiParam(param, value);
}

int GuiObjectWnd::setXmlParamById(int xmlhandle, int attrid, const wchar_t *name, const wchar_t *value)
{
	return setXuiParam(xmlhandle, attrid, name, value);
}

void *GuiObjectWnd::getInterface(GUID interface_guid)
{
	void *r = GUIOBJECTWND_PARENT::getInterface(interface_guid);
	if (r) return r;
	return getRootObject()->rootobject_getScriptObject()->vcpu_getInterface(interface_guid);
}

int GuiObjectWnd::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source)
{
#ifdef WASABI_COMPILE_CONFIG
	if (!_wcsicmp(action, L"reload_config") && isInited())
	{
		if (cfg_reentry) return 1;
		cfg_reentry = 1;
		int r = onReloadConfig();
		cfg_reentry = 0;
		return r;
	}
#endif
	int rt = GUIOBJECTWND_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
	getGuiObject()->guiobject_onAction(action, param, x, y, p1, p2, data, datalen, source);
	return rt;
}

void GuiObjectWnd::setContent(const wchar_t *groupid_orguid, int autoresizefromcontent)
{
#ifdef WASABI_COMPILE_SKIN

	//  abstract_setAllowDeferredContent(0);
	abstract_setContent(groupid_orguid, autoresizefromcontent);
#endif
}

void GuiObjectWnd::setContentBySkinItem(SkinItem *item, int autoresizefromcontent)
{
	//  abstract_setAllowDeferredContent(0);
#ifdef WASABI_COMPILE_SKIN
	abstract_setContentBySkinItem(item, autoresizefromcontent);
#endif
}

void GuiObjectWnd::abstract_onNewContent()
{

#ifdef WASABI_COMPILE_SKIN
	GUIOBJECTWND_PARENT::abstract_onNewContent();
#endif
	onNewContent();
#ifdef WASABI_COMPILE_CONFIG
	if (getGuiObject()->guiobject_hasCfgAttrib())
		onReloadConfig();
#endif
}

GuiObject *GuiObjectWnd::findObject(const wchar_t *object_id)
{
	return getGuiObject()->guiobject_findObject(object_id);
}

#ifdef WASABI_COMPILE_SCRIPT
ScriptObject *GuiObjectWnd::findScriptObject(const wchar_t *object_id)
{
	GuiObject *fo = getGuiObject()->guiobject_findObject(object_id);
	if (fo != NULL) return fo->guiobject_getScriptObject();
	return NULL;
}
#endif
const wchar_t *GuiObjectWnd::getId()
{
	if (my_gui_object)
		return my_gui_object->guiobject_getId();
	return GUIOBJECTWND_PARENT::getId();
}

int GuiObjectWnd::onPostOnInit()
{
	int r = GUIOBJECTWND_PARENT::onPostOnInit();
#ifdef WASABI_COMPILE_CONFIG
	if (getGuiObject()->guiobject_hasCfgAttrib())
		onReloadConfig();
#endif
	return r;
}

int GuiObjectWnd::onInit()
{
	int r = GUIOBJECTWND_PARENT::onInit();
	getGuiObject()->guiobject_onInit();
	return r;
}

int GuiObjectWnd::onChar(unsigned int c)
{
	if (!my_gui_object) return 1;
	int r = GUIOBJECTWND_PARENT::onChar(c);
	getGuiObject()->guiobject_onChar(c);
	return r;
}

int GuiObjectWnd::onKeyDown(int vkcode)
{
	if (!my_gui_object) return 1;
	int r = GUIOBJECTWND_PARENT::onKeyDown(vkcode);
	getGuiObject()->guiobject_onKeyDown(vkcode);
	return r;
}

int GuiObjectWnd::onKeyUp(int vkcode)
{
	if (!my_gui_object) return 1;
	int r = GUIOBJECTWND_PARENT::onKeyUp(vkcode);
	getGuiObject()->guiobject_onKeyUp(vkcode);
	return r;
}

int GuiObjectWnd::onGetFocus()
{
	if (!my_gui_object) return 1;
	int r = GUIOBJECTWND_PARENT::onGetFocus();
	getGuiObject()->guiobject_onGetFocus();
	return r;
}

int GuiObjectWnd::onKillFocus()
{
	if (!my_gui_object) return 1;
	int r = GUIOBJECTWND_PARENT::onKillFocus();
	getGuiObject()->guiobject_onKillFocus();
	return r;
}

int GuiObjectWnd::onAcceleratorEvent(const wchar_t *name)
{
	int r = GUIOBJECTWND_PARENT::onAcceleratorEvent(name);
	getGuiObject()->guiobject_onAccelerator(name);
	return r;
}

int GuiObjectWnd::wantFocus()
{
	if (GUIOBJECTWND_PARENT::wantFocus()) return 1;
	if (my_gui_object)
		return my_gui_object->guiobject_wantFocus();
	return 0;
}
