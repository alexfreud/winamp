/**
 * simlple maximize script
 * 
 * doesn't require a registry store, so ideal for standardframes
 * required objects: maximize & restore as buttons
 * 
 * @author mpdeimos
 * @version 0.1
 */

#include <lib/std.mi>

Global Button restore, maximize;
Global Layout parent;

Global Int lx, ly, lw, lh;

System.onScriptLoaded ()
{
	restore = getScriptGroup().findObject("restore");
	maximize = getScriptGroup().findObject("maximize");
	parent = getScriptGroup().getParentLayout();

	lx = -1;
	ly = -1;
	lh = -1;
	lw = -1;
}

parent.onResize (int x, int y, int w, int h)
{
		double d = getScale();
		if (getLeft() == getViewPortLeftfromGuiObject(parent) && getTop() == getViewPortTopfromGuiObject(parent) && getWidth() == getViewPortWidthfromGuiObject(parent)/d && getHeight() == getViewPortHeightfromGuiObject(parent)/d)
		{
			restore.show();
			maximize.hide();
		}
		else
		{
			restore.hide();
			maximize.show();
		}
}

maximize.onLeftClick ()
{
	lx = parent.getLeft();
	ly = parent.getTop();
	lw = parent.getWidth();
	lh = parent.getHeight();

	double d = parent.getScale();
	parent.resize(getViewPortLeftfromGuiObject(parent), getViewPortTopfromGuiObject(parent), getViewPortWidthfromGuiObject(parent)/d, getViewPortHeightfromGuiObject(parent)/d);
}


restore.onLeftClick ()
{
	if (lx == -1)
		lx = parent.getLeft() - 75;
	if (ly == -1)
		ly = parent.getTop() - 75;
	if (lw == -1)
		lw = parent.getWidth() - 150;
	if (lh == -1)
		lh = parent.getHeight() - 150;

	parent.resize(lx,ly,lw,lh);
}


// TODO (mpdeimos) add scale recognizing
