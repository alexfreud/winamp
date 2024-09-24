#ifndef __WNDCREATE_H
#define __WNDCREATE_H

#include <api/service/svcs/svc_wndcreate.h>
#include <bfc/ptrlist.h>

class GroupWndCreateSvc : public svc_windowCreateI
{
public:
	static const char *getServiceName() { return "WindowType to Group window creator"; }

	virtual int testType(const wchar_t *windowtype) { return 1; }
	//    virtual int testGuid(GUID g) ;
	//    virtual ifc_window *createWindowByGuid(GUID g, ifc_window *parent);
	virtual ifc_window *createWindowOfType(const wchar_t *windowtype, ifc_window *parent, int n);
	virtual int destroyWindow(ifc_window *w);

	static int num_group_list;

private:
	ifc_window *createGuiTreeItem(SkinItem *item, ifc_window *parent);
	PtrList<ifc_window> group_list;
};

#endif
