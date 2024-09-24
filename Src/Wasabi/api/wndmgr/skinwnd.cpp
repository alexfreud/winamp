#include <precomp.h>
#include "skinwnd.h"
#include <api/wnd/api_window.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/scriptguid.h>
#include <api/script/objects/c_script/c_group.h>
#include <api/script/objects/c_script/c_layout.h>
#include <api/script/objects/c_script/c_container.h>
#include <api/wnd/wndclass/wndholder.h>

SkinWnd::SkinWnd(const wchar_t *group_id, const wchar_t *prefered_container, int container_flag, RECT *animated_rect_source, int transcient, int starthidden)
{
	isnew = 0;
	wnd = WASABI_API_WNDMGR->skinwnd_createByGroupId(group_id, prefered_container, container_flag, animated_rect_source, transcient, starthidden, &isnew);
	notifyMinMaxChanged();
}

SkinWnd::SkinWnd(GUID svc_or_group_guid, const wchar_t *prefered_container, int container_flag, RECT *animated_rect_source, int transcient, int starthidden)
{
	isnew = 0;
	wnd = WASABI_API_WNDMGR->skinwnd_createByGuid(svc_or_group_guid, prefered_container, container_flag, animated_rect_source, transcient, starthidden, &isnew);
	notifyMinMaxChanged();
}

SkinWnd::~SkinWnd()
{}

void SkinWnd::destroy(RECT *animated_rect_dest)
{
	if (wnd == NULL) return ;
	if (!isnew)
	{
		ifc_window *w = wnd->findWindowByInterface(windowHolderGuid);
		WindowHolder *wh = static_cast<WindowHolder*>(w->getInterface(windowHolderGuid));
		if (wh != NULL)
		{
			wh->onRemoveWindow();
			return ;
		}
	}
	WASABI_API_WNDMGR->skinwnd_destroy(wnd, animated_rect_dest);
}

int SkinWnd::runModal(int center)
{
	if (wnd == NULL) return 0;
	ifc_window *w = wnd->getDesktopParent();
	if (center)
	{
		C_Layout l(getLayout());
		l.center();
	}
	w->setVisible(1);
	return w->runModal();
}

void SkinWnd::endModal(int retcode)
{
	if (wnd == NULL) return ;
	wnd->endModal(retcode);
}

GuiObject *SkinWnd::findObject(const wchar_t *object_id)
{
	if (wnd == NULL) return NULL;
	GuiObject *obj = NULL;
	obj = static_cast<GuiObject *>(wnd->getInterface(guiObjectGuid));
	return obj->guiobject_findObject(object_id);
}

ScriptObject *SkinWnd::getContainer()
{
	if (wnd == NULL) return NULL;
	ifc_window *dw = wnd->getDesktopParent();
	if (!dw) return NULL;
	ScriptObject *o = static_cast<ScriptObject *>(dw->getInterface(scriptObjectGuid));
	if (o != NULL)
	{
		return C_Layout(o).getContainer();
	}
	return NULL;
}

ScriptObject *SkinWnd::getLayout()
{
	if (wnd == NULL) return NULL;
	ifc_window *dw = wnd->getDesktopParent();
	if (!dw) return NULL;
	ScriptObject *o = static_cast<ScriptObject *>(dw->getInterface(scriptObjectGuid));
	return o;
}

void SkinWnd::notifyMinMaxChanged()
{
	if (wnd != NULL)
	{
		wnd->signalMinMaxEnforcerChanged();
	}
}
