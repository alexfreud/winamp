#include <lib/std.mi>
#include "attribs.m"

Function updateLinkage(int type);

System.onScriptLoaded() {
  initAttribs();
  if (windowshade_linkall_attrib.getData() == "1") windowshade_linkall_attrib.onDataChanged();
  if (windowshade_linkposition_attrib.getData() == "1") windowshade_linkposition_attrib.onDataChanged();
  if (windowshade_linknone_attrib.getData() == "1") windowshade_linknone_attrib.onDataChanged();
}

windowshade_linkall_attrib.onDataChanged() {
  if (getData() == "1") updateLinkage(2);
}

windowshade_linkposition_attrib.onDataChanged() {
  if (getData() == "1") updateLinkage(1);
}

windowshade_linknone_attrib.onDataChanged() {
  if (getData() == "1") updateLinkage(0);
}

updateLinkage(int type) {
  Layout shade = getScriptGroup().getParentLayout();
  Layout normal = shade.getContainer().getLayout("normal");
  if (type == 0) {
    shade.setXmlParam("unlinked", "1");
    shade.setXmlParam("linkwidth", "");
    normal.setXmlParam("linkwidth", "");
  } else if (type == 1) {
    shade.setXmlParam("unlinked", "0");
    shade.setXmlParam("linkwidth", "");
    normal.setXmlParam("linkwidth", "");
  } else if (type == 2) {
    shade.setXmlParam("unlinked", "0");
    shade.setXmlParam("linkwidth", "normal");
    normal.setXmlParam("linkwidth", "shade");
  }
}
