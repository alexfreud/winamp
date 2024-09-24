#include <lib/std.mi>
#include "attribs.m"

Function setOverlay();

Global Layer VideoVisTextOverlay;

System.onScriptLoaded() {
	initAttribs();
	VideoVisTextOverlay=getScriptGroup().findObject("menubar.right.textoverlay");
	setOverlay();
}

video_detach_attrib.onDataChanged() {
	setOverlay();
}

vis_detach_attrib.onDataChanged() {
	setOverlay();
}

setOverlay() {
	if (vis_detach_attrib.getData() == "1" && video_detach_attrib.getData() == "1") VideoVisTextOverlay.hide();
	else VideoVisTextOverlay.show();
}