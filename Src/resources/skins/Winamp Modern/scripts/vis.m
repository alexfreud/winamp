#include <lib/std.mi>
#include "attribs.m"

Function updateVisCmd();

Global Button btnVisReattach;
Global Layout thislayout;
Global Int evershown;

System.onScriptLoaded() {
	initAttribs();
	thislayout = getScriptGroup().getParentLayout();
	BtnVisReattach = getScriptGroup().findObject("button.vis.reattach");
}

thisLayout.onSetVisible(int show) {
	if (!evershown) {
		evershown = 1;
		updateVisCmd();
	}
}

BtnVisReattach.onLeftClick() {
  Container c = getContainer("main");
  if (c.getLayout("shade").isVisible()) c.switchToLayout("normal");
  vis_detach_attrib.setData("0");
}

viscmd_menu_attrib.onDataChanged() {
  updateVisCmd();
}

updateVisCmd() {
  Button btn = getScriptGroup().findObject("button.vis.misc");
  if (btn) {
    if (viscmd_menu_attrib.getData() == "1") {
      btn.setXmlParam("action", "Vis_Menu");
    } else {
      btn.setXmlParam("action", "Vis_Cfg");
    }
  }
}