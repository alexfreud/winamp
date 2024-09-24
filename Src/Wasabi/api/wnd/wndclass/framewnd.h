//NONPORTABLE
#ifndef _FRAMEWND_H
#define _FRAMEWND_H

#include <bfc/common.h>
#include <api/wnd/wndclass/labelwnd.h>
#include <tataki/bitmap/autobitmap.h>

#define MAXCHILD 2	// this had better never not be 2

typedef enum {
  DIVIDER_HORIZONTAL, DIVIDER_VERTICAL, DIVIDER_UNDEFINED = -1
} FrameWndDividerType;
enum { SDP_FROMLEFT, SDP_FROMRIGHT };
#define SDP_FROMTOP SDP_FROMLEFT
#define SDP_FROMBOTTOM SDP_FROMRIGHT

typedef enum {
  FRAMEWND_SQUISH,
  FRAMEWND_COVER
} FrameWndSlideMode;

#define SIZERWIDTH 8

// this window holds other basewnd derived classes
#define FRAMEWND_PARENT LabelWnd
class FrameWnd : public FRAMEWND_PARENT {
public:
	FrameWnd();
	virtual ~FrameWnd();

	virtual int onInit();

	virtual int getCursorType(int x, int y);

	virtual int onPaint(Canvas *canvas);
	virtual int onResize();

	virtual int onLeftButtonDown(int x, int y);
	virtual int onMouseMove(int x, int y);	// only called when mouse captured
	virtual int onLeftButtonUp(int x, int y);

	virtual int childNotify(ifc_window *which, int msg, intptr_t param1, intptr_t param2);

//	virtual int forceFocus();

	// unique to this class
	int setChildren(BaseWnd *child1, BaseWnd *child2=NULL);
	int setChildrenRootWnd(ifc_window *child1, ifc_window *child2=NULL);
	ifc_window *enumChild(int which);
	// horizontal or vertical?
	void setDividerType(FrameWndDividerType type);
	FrameWndDividerType getDividerType();
	// where is the divider?
	int setDividerPos(int from, int pos);
	// this version doesn't check the cfg file for last position
	int setDividerPosNoCfg(int from, int pos);
	void getDividerPos(int *from, int *pos);

	int setResizeable(int is);
	void setMinWidth(int min);
	void setMaxWidth(int max);
	void setSnap(int snap);

	virtual int onGetFocus();
	virtual int onDeferredCallback(intptr_t p1, intptr_t p2);

	// cover or squish
	void setSlideMode(FrameWndSlideMode mode);
	FrameWndSlideMode getSlideMode();

	virtual void onResizeChildren(RECT leftr, RECT rightr);

	// drag and drops are forwarded into windowshaded windows
	virtual int dragEnter(ifc_window *sourceWnd);
	virtual int dragOver(int x, int y, ifc_window *sourceWnd);
	virtual int dragLeave(ifc_window *sourceWnd);
	virtual int dragDrop(ifc_window *sourceWnd, int x, int y);

protected:
	int convertPropToPix(int prop);
	int ConvertPixToProp();

	void windowshade(int which, int shaded);
	ifc_window *getWindowShadedChild();

	void Set_v_bitmap(const wchar_t *new_v_bitmap);
	void Set_v_grabber(const wchar_t *new_v_grabber);

private:
	int _setChildren(ifc_window *child1, ifc_window *child2, BaseWnd *child1b, BaseWnd *child2b);
	int nchild;
	BaseWnd *children[MAXCHILD];
	ifc_window *rwchildren[MAXCHILD];
	int hidey[MAXCHILD];
	int windowshaded[MAXCHILD];

	FrameWndDividerType vert;

	int resizeable;
	FrameWndSlideMode slidemode;

	int divideside;
	int pullbarpos;	// 0..65536
	int prevpullbarpos;
	int minwidth, maxwidth;
	int maxpixels;
	boolean noMaxRestriction;
	int minpixels;
	int snapoffsetx, snapoffsety;
	int SNAP;
	RECT sizerRect;

	AutoSkinBitmap h_bitmap, v_bitmap, h_grabber, v_grabber, ws_bitmap;
	int resizing;
};

#define PULLBAR_FULL 65536L
#define PULLBAR_HALF (PULLBAR_FULL/2)
#define PULLBAR_QUARTER (PULLBAR_FULL/4)
#define PULLBAR_THREEQUARTER (PULLBAR_FULL-PULLBAR_QUARTER)
#define PULLBAR_EIGHTH (PULLBAR_FULL/8)

#endif
