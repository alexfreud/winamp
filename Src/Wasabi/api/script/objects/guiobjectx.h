#ifndef NULLSOFT_WASABI_GUIOBJECTX_H
#define NULLSOFT_WASABI_GUIOBJECTX_H

#include <wasabicfg.h>
#include <api/script/objects/guiobject.h>

class GuiObjectX : public GuiObject
{
public:
	virtual ~GuiObjectX() {}

	virtual ifc_window *guiobject_getRootWnd()=0;
	virtual void guiobject_setRootWnd(ifc_window *w)=0;
	virtual void guiobject_onInit()=0;

	virtual void guiobject_getGuiPosition(int *x, int *y, int *w, int *h, int *rx, int *ry, int *rw, int *rh)=0;
	virtual void guiobject_setGuiPosition(int *x, int *y, int *w, int *h, int *rx, int *ry, int *rw, int *rh)=0;
	virtual int guiobject_getAnchoragePosition(int *x1, int *y1, int *x2, int *y2, int *anchor)=0;
	virtual void guiobject_setAnchoragePosition(int *x1, int *y1, int *x2, int *y2, int *anchor)=0;
	virtual void guiobject_validateAnchorage()=0;

	virtual void guiobject_setParentGroup(Group *g)=0;
	virtual ScriptObject *guiobject_getScriptObject()=0;
	virtual RootObject *guiobject_getRootObject()=0;
	virtual Group *guiobject_getParentGroup()=0;
	virtual GuiObject *guiobject_getParent()=0;
	virtual void guiobject_setTabOrder(int a)=0;

	virtual void guiobject_setId(const wchar_t *id)=0;
	virtual const wchar_t *guiobject_getId()=0;

	virtual void guiobject_setTargetX(int tx)=0;
	virtual void guiobject_setTargetY(int ty)=0;
	virtual void guiobject_setTargetW(int tw)=0;
	virtual void guiobject_setTargetH(int th)=0;
	virtual void guiobject_setTargetA(int th)=0;
	virtual void guiobject_setTargetSpeed(float s)=0;
	virtual void guiobject_gotoTarget(void)=0;
	virtual void guiobject_cancelTarget()=0;
	virtual void guiobject_reverseTarget(int reverse)=0;

	virtual int guiobject_getAutoWidth()=0;
	virtual int guiobject_getAutoHeight()=0;
	virtual int guiobject_movingToTarget()=0;

	virtual void guiobject_bringToFront()=0;
	virtual void guiobject_bringToBack()=0;
	virtual void guiobject_bringAbove(GuiObject *o)=0;
	virtual void guiobject_bringBelow(GuiObject *o)=0;

	virtual void guiobject_setClickThrough(int ct)=0;
	virtual int guiobject_isClickThrough()=0;

	virtual void guiobject_setAutoSysMetricsX(int a)=0;
	virtual void guiobject_setAutoSysMetricsY(int a)=0;
	virtual void guiobject_setAutoSysMetricsW(int a)=0;
	virtual void guiobject_setAutoSysMetricsH(int a)=0;
	virtual int guiobject_getAutoSysMetricsX()=0;
	virtual int guiobject_getAutoSysMetricsY()=0;
	virtual int guiobject_getAutoSysMetricsW()=0;
	virtual int guiobject_getAutoSysMetricsH()=0;

	virtual int guiobject_getRegionOp()=0;
	virtual void guiobject_setRegionOp(int op)=0;
	virtual int guiobject_isRectRgn()=0;
	virtual void guiobject_setRectRgn(int rrgn)=0;

	virtual void guiobject_onLeftButtonDown(int x, int y)=0;
	virtual void guiobject_onLeftButtonUp(int x, int y)=0;
	virtual void guiobject_onRightButtonDown(int x, int y)=0;
	virtual void guiobject_onRightButtonUp(int x, int y)=0;
	virtual void guiobject_onLeftButtonDblClk(int x, int y)=0;
	virtual void guiobject_onRightButtonDblClk(int x, int y)=0;
	virtual int guiobject_onMouseWheelUp(int click, int lines)=0;
	virtual int guiobject_onMouseWheelDown(int click, int lines)=0;
	virtual void guiobject_onMouseMove(int x, int y)=0;
	virtual void guiobject_onEnterArea()=0;
	virtual void guiobject_onLeaveArea()=0;
	virtual void guiobject_onEnable(int en)=0;
	virtual void guiobject_setEnabled(int en)=0;
	virtual void guiobject_onResize(int x, int y, int w, int h)=0;
	virtual void guiobject_onSetVisible(int v)=0;
	virtual void guiobject_onTargetReached()=0;
	virtual void guiobject_setAlpha(int a)=0;
	virtual void guiobject_setActiveAlpha(int a)=0;
	virtual void guiobject_setInactiveAlpha(int a)=0;
	virtual int guiobject_getAlpha()=0;
	virtual int guiobject_getActiveAlpha()=0;
	virtual int guiobject_getInactiveAlpha()=0;
	virtual int guiobject_isActive()=0;
	virtual void guiobject_onStartup()=0;
	virtual int guiobject_setXmlParam(const wchar_t *param, const wchar_t *value)=0;
	virtual const wchar_t *guiobject_getXmlParam(const wchar_t *param)=0;
	virtual int guiobject_setXmlParamById(int id, const wchar_t *value)=0;
	virtual svc_xuiObject *guiobject_getXuiService()=0;
	virtual void guiobject_setXuiService(svc_xuiObject *svc)=0;
	virtual waServiceFactory *guiobject_getXuiServiceFactory()=0;
	virtual void guiobject_setXuiServiceFactory(waServiceFactory *fac)=0;
	virtual GuiObject *guiobject_getTopParent()=0;
#ifdef WASABI_COMPILE_WNDMGR
	virtual Layout *guiobject_getParentLayout()=0;
	virtual int guiobject_runModal()=0;
	virtual void guiobject_endModal(int retcode)=0;
	virtual void guiobject_popParentLayout()=0;
	virtual void guiobject_registerStatusCB(GuiStatusCallback *cb)=0;
	virtual void guiobject_setStatusText(const wchar_t *txt, int overlay = FALSE)=0;
	virtual void guiobject_addAppCmds(AppCmds *commands)=0;
	virtual void guiobject_removeAppCmds(AppCmds *commands)=0;
	virtual void guiobject_pushCompleted(int max = 100)=0;
	virtual void guiobject_incCompleted(int add = 1)=0;
	virtual void guiobject_setCompleted(int pos)=0;
	virtual void guiobject_popCompleted()=0;
#endif
	virtual GuiObject *guiobject_findObject(const wchar_t *id)=0;
	virtual GuiObject *guiobject_findObjectXY(int x, int y)=0; // in client coordinates relative to this object
	virtual GuiObject *guiobject_findObjectByInterface(GUID interface_guid)=0;
	virtual GuiObject *guiobject_findObjectByCallback(FindObjectCallback *cb)=0;
	virtual void guiobject_setMover(int m)=0;
	virtual int guiobject_getMover()=0;
	virtual void guiobject_onCancelCapture()=0;
	virtual void guiobject_onChar(wchar_t c)=0;
	virtual void guiobject_onKeyDown(int vkcode)=0;
	virtual void guiobject_onKeyUp(int vkcode)=0;

	virtual FOURCC guiobject_getDropTarget()=0;
	virtual void guiobject_setDropTarget(const wchar_t *strval)=0;

	virtual void onTargetTimer()=0;
#ifdef USEAPPBAR
	virtual int guiobject_getAppBar()=0;
	virtual void guiobject_setAppBar(int en)=0;
	virtual void setAppBar(const wchar_t *en)=0;
#endif

#ifdef WASABI_COMPILE_CONFIG
	virtual void guiobject_setCfgAttrib(CfgItem *item, const wchar_t *name)=0;

	virtual CfgItem *guiobject_getCfgItem()=0;
	virtual const wchar_t *guiobject_getCfgAttrib()=0;

	virtual int guiobject_getCfgInt()=0;
	virtual void guiobject_setCfgInt(int i)=0;
	virtual float guiobject_getCfgFloat()=0;
	virtual void guiobject_setCfgFloat(float f)=0;
	virtual const wchar_t *guiobject_getCfgString()=0;
	virtual void guiobject_setCfgString(const wchar_t *s)=0;
	virtual int guiobject_hasCfgAttrib()=0;
#endif

	virtual const wchar_t *guiobject_getName()=0;
	virtual void guiobject_onAccelerator(const wchar_t *accel)=0;
	virtual int guiobject_onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source)=0;

	virtual int guiobject_wantFocus()=0;
	virtual void guiobject_setNoDoubleClick(int no)=0;
	virtual void guiobject_setNoLeftClick(int no)=0;
	virtual void guiobject_setNoRightClick(int no)=0;
	virtual void guiobject_setNoMouseMove(int no)=0;
	virtual void guiobject_setNoContextMenu(int no)=0;

	virtual void guiobject_setCursor(const wchar_t *c)=0;
	virtual int guiobject_wantTranslation()=0;

	virtual int guiobject_dragEnter(ifc_window *sourceWnd)=0;
	virtual int guiobject_dragOver(int x, int y, ifc_window *sourceWnd)=0;
	virtual int guiobject_dragLeave(ifc_window *sourceWnd)=0;

protected:
	RECVS_DISPATCH;

};





#endif