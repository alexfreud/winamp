#ifndef __POPUPEXITCHECKER_H
#define __POPUPEXITCHECKER_H

#include <bfc/depend.h>
#include <bfc/ptrlist.h>

class ifc_window;
class PopupExitCallback;

class PopupExitCallbackEntry
{
public:
	PopupExitCallbackEntry(PopupExitCallback *_cb, ifc_window *_watched, api_dependent *_cbd, api_dependent *_wd) : cb(_cb), watched(_watched), cbd(_cbd), wd(_wd) {}
	virtual ~PopupExitCallbackEntry() {}

	PopupExitCallback *cb;
	ifc_window *watched;
	api_dependent *cbd;
	api_dependent *wd;
};

class PopupExitChecker : public DependentViewerI
{
public:
	PopupExitChecker();
	virtual ~PopupExitChecker();

	void registerCallback(PopupExitCallback *cb, ifc_window *watched);
	void deregisterCallback(PopupExitCallback *cb);
#undef check
	int check(ifc_window *w);
	void signal();
	int isGrandChildren(ifc_window *parent, ifc_window *child); // recursive
	PtrList<PopupExitCallbackEntry> watchers;

	virtual int viewer_onItemDeleted(api_dependent *item);
};

#endif
