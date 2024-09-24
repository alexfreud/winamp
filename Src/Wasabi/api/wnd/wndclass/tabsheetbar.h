#ifndef __TABSHEETBAR_H
#define __TABSHEETBAR_H

#include <api/wnd/wndclass/guiobjwnd.h>

class GroupTabButton;

#define TABSHEETBAR_PARENT GuiObjectWnd


/**
  Class 
 
  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class TabSheetBar : public TABSHEETBAR_PARENT
{
public:
	TabSheetBar();
	virtual ~TabSheetBar();
	virtual int onInit();
	virtual int onResize();
	virtual void addChild(GroupTabButton *child);
	virtual int getHeight();
	virtual int childNotify(ifc_window *child, int msg, intptr_t param1 = 0, intptr_t param2 = 0);
	void setMargin(int m) { margin = m; if (isInited()) onResize(); }
	void setSpacing(int s) { spacing = s; if (isInited()) onResize(); }

private:
	int maxheightsofar;
	PtrList<GroupTabButton> btns;
	int margin, spacing;
	GuiObjectWnd bottombar;
};

#endif
