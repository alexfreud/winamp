#ifndef __APPDEACTIVATIONMGR_H
#define __APPDEACTIVATIONMGR_H

#include <bfc/ptrlist.h>

class ifc_window;

class AppDeactivationMgr {
	
	public:

  	static int is_deactivation_allowed(ifc_window *w);
		static void push_disallow(ifc_window *w);
		static void pop_disallow(ifc_window *w);
    static void setbypass(int i);

  private:
    static PtrList<ifc_window> list;

};

#endif
