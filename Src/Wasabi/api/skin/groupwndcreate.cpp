#include <precomp.h>

#include "groupwndcreate.h"

#include <api/skin/guitree.h>
#include <api/skin/groupmgr.h>

// 5/15/2002, removed serving of groups by guid xui param, guid is 'non-overriden window guaranteed', groupid is 'allow override implicitly'

/*int GroupWndCreateSvc::testGuid(GUID g) {
  return (GuiTree::getGroupDef(g) >= 0);
}
 
api_window *GroupWndCreateSvc::createWindowByGuid(GUID g, api_window *parent) { 
  int n = GuiTree::getGroupDef(g);
  if (n < 0) return NULL;
 
  return createGuiTreeItem(n, parent);
}*/

ifc_window *GroupWndCreateSvc::createWindowOfType(const wchar_t *windowtype, ifc_window *parent, int i)
{
	SkinItem *item = guiTree->enumGroupDefOfType(windowtype, i);
	if (item == NULL) return NULL;

	return createGuiTreeItem(item, parent);
}

ifc_window *GroupWndCreateSvc::createGuiTreeItem(SkinItem*item, ifc_window *parent)
{
	ifc_window *wnd = GroupMgr::instantiate(NULL, GROUP_GROUP, item);
	if (!wnd) return NULL;
	wnd->setParent(parent);
	group_list.addItem(wnd);
	num_group_list = group_list.getNumItems();
	return wnd;
}

int GroupWndCreateSvc::destroyWindow(ifc_window *w)
{
	if (group_list.haveItem(w))
	{
		group_list.removeItem(w);
		WASABI_API_SKIN->group_destroy(w);
		num_group_list = group_list.getNumItems();
		return 1;
	}
	return 0;
}

int GroupWndCreateSvc::num_group_list = 0;
