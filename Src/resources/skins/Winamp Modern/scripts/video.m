#include <lib/std.mi>
#include "attribs.m"

Global Button btnVideoReattach;

System.onScriptLoaded() {
	initAttribs();
	BtnVideoReattach = getScriptGroup().findObject("button.video.reattach");
}

BtnVideoReattach.onLeftClick() {
  Container c = getContainer("main");
  if (c.getLayout("shade").isVisible()) c.switchToLayout("normal");
  video_detach_attrib.setData("0");
}
