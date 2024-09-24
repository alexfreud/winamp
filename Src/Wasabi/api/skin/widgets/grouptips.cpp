#include "precomp.h"
#include <bfc/wasabi_std.h>
#include "grouptips.h"
#include <api/wnd/api_window.h>
#include <api/script/objects/c_script/c_group.h>
#include <api/script/objects/c_script/c_text.h>
#include <api/script/scriptguid.h>
#include <api/script/objects/guiobject.h>
#ifndef _WASABIRUNTIME

BEGIN_SERVICES(GroupTips_Svc);
DECLARE_SERVICETMULTI(svc_toolTipsRenderer, GroupTips);
END_SERVICES(GroupTips_Svc, _GroupTips_Svc);

#ifdef _X86_
extern "C" { int _link_GroupTipsSvc; }
#else
extern "C" { int __link_GroupTipsSvc; }
#endif

#endif

GroupTips::GroupTips()
{
	tipwnd = NULL;
}

GroupTips::~GroupTips()
{
	if (tipwnd)
		WASABI_API_SKIN->group_destroy(tipwnd);
}

int GroupTips::spawnTooltip(const wchar_t *text)
{
	int x, y;
	Wasabi::Std::getMousePos(&x, &y);

	ifc_window *wnd = WASABI_API_SKIN->group_create_layout(L"wasabi.tooltip.group");

	if (wnd)
	{
		wnd->setStartHidden(1);
		wnd->setParent(WASABI_API_WND->main_getRootWnd());
		wnd->init(WASABI_API_WND->main_getRootWnd(), TRUE);
		wnd->getGuiObject()->guiobject_onStartup();

		RECT r;
		wnd->getClientRect(&r);
		int w = r.right - r.left;
		int h = r.bottom - r.top;

		y -= h; // move tip above mouse by default

		POINT pt = {x, y};
		RECT vpr;

		Wasabi::Std::getViewport(&vpr, &pt, NULL, NULL);

		if (x + w > vpr.right) x -= vpr.right - w;
		if (x < vpr.left) x = vpr.left;
		if (x + w > vpr.right)
		{
			w = vpr.right - vpr.left; 
			x = 0;
		}
		if (y + h > vpr.bottom) y -= vpr.bottom - w;
		if (y < vpr.top) y = vpr.top;
		if (y + h > vpr.bottom)
		{
			h = vpr.bottom - vpr.top; 
			y = 0;
		}

		wnd->resize(x, y, w, h);

		ScriptObject *group = static_cast<ScriptObject *>(wnd->getInterface(scriptObjectGuid));
		if (group)
		{
			ScriptObject *txt = C_Group(group).getObject(L"tooltip.text");
			if (txt)
				C_Text(txt).setText(text);
		}

		// tooltips should always be on top otherwise they're pointless <dro>
		Wasabi::Std::Wnd::setTopmost(wnd->getOsWindowHandle(), TRUE);
		wnd->setVisible(1);

		tipwnd = wnd;
	}
	return 1;
}