#ifndef _VIRTUALWND_H
#define _VIRTUALWND_H

#include <api/wnd/basewnd.h>

#define VIRTUALWND_PARENT BaseWnd
#define AUTOWH 0xFFFE
#define NOCHANGE 0xFFFD

class NOVTABLE VirtualWnd : public VIRTUALWND_PARENT
{
protected:
	VirtualWnd();
	virtual ~VirtualWnd();
public:
	virtual int init( ifc_window *parent, int nochild = FALSE );
	virtual int init( OSMODULEHANDLE moduleHandle, OSWINDOWHANDLE parent, int nochild = FALSE );

	virtual void bringToFront();
	virtual void bringToBack();
	virtual void bringAbove( BaseWnd *w );
	virtual void bringBelow( BaseWnd *w );

	//NONPORTABLE--avoid prolonged use
	virtual HWND getOsWindowHandle();
	virtual HINSTANCE getOsModuleHandle();

public:
	virtual void resize( int x, int y, int w, int h, int wantcb = 1 ) override;
	virtual void resize( RECT *r, int wantcb = 1 );
	virtual void move( int x, int y ) override;
	virtual void invalidate() override;
	virtual void invalidateRect( RECT *r ) override;
	virtual void invalidateRgn( api_region *reg ) override;
	virtual void validate() override;
	virtual void validateRect( RECT *r ) override;
	virtual void validateRgn( api_region *reg ) override;
	virtual void getClientRect( RECT * ) override;
	virtual void getNonClientRect( RECT * ) override;
	virtual void getWindowRect( RECT * ) override;
	virtual int beginCapture() override;
	virtual int endCapture() override;
	virtual int getCapture() override;
	virtual void setVirtualChildCapture( BaseWnd *child );
	virtual void repaint() override;
	/*  virtual int focusNextSibbling(int dochild);
	  virtual int focusNextVirtualChild(BaseWnd *child);*/
	virtual int cascadeRepaint( int pack = 1 ) override;
	virtual int cascadeRepaintRect( RECT *r, int pack = 1 ) override;
	virtual int cascadeRepaintRgn( api_region *r, int pack = 1 ) override;
	virtual ifc_window *rootWndFromPoint( POINT *pt ) override;
	virtual double getRenderRatio() override;
	virtual int reparent( ifc_window *newparent ) override;
	virtual int setVirtual( int i ) override;
	virtual ifc_window *getRootParent() override;
	virtual int gotFocus() override;
	virtual int onGetFocus() override;
	virtual int onKillFocus() override;
	virtual void setFocus() override;
	virtual int onActivate() override;
	virtual int onDeactivate() override;
	virtual void setVirtualChildFocus( ifc_window *child ) override;
	virtual int wantFocus() override
	{
		return 0;
	}
	virtual void setAllowDeactivation( int allow ) override;
	virtual int allowDeactivation() override;

public:
	virtual int isVirtual() override
	{
		return !bypassvirtual;
	}

protected:
	int virtualX, virtualY, virtualH, virtualW;
	int bypassvirtual;
	int focus;
	int resizecount;
	double lastratio;
};

#endif
