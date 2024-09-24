#ifndef _STD_WIN_H
#define _STD_WIN_H

#include <bfc/platform/platform.h>
#include <bfc/wasabi_std.h>

#define ROOTSTRING L"RootWnd"
#define BASEWNDCLASSNAME L"BaseWindow_" ROOTSTRING

#ifdef __cplusplus

class ifc_window;
namespace Wasabi
{
	namespace Std
	{
		namespace Wnd {
			OSWINDOWHANDLE createWnd(RECT* r, int nochild, int acceptdrops, OSWINDOWHANDLE parent, OSMODULEHANDLE module, ifc_window* rw);
			void destroyWnd(OSWINDOWHANDLE wnd);

			int isValidWnd(OSWINDOWHANDLE wnd);

			void setWndPos(OSWINDOWHANDLE wnd, OSWINDOWHANDLE zorder, int x, int y, int w, int h,
				int nozorder, int noactive, int nocopybits, int nomove, int noresize);
			void bringToFront(OSWINDOWHANDLE wnd);
			void sendToBack(OSWINDOWHANDLE wnd);
			int isWndVisible(OSWINDOWHANDLE wnd);
			void showWnd(OSWINDOWHANDLE wnd, int noactivate = FALSE);
			void hideWnd(OSWINDOWHANDLE wnd);
			int isPopup(OSWINDOWHANDLE wnd);
			void setEnabled(OSWINDOWHANDLE wnd, int enabled);
			void setFocus(OSWINDOWHANDLE wnd);
			OSWINDOWHANDLE getFocus();
			void setTopmost(OSWINDOWHANDLE, int topmost);

			void invalidateRect(OSWINDOWHANDLE wnd, RECT* r = NULL);
			void invalidateRegion(OSWINDOWHANDLE wnd, OSREGIONHANDLE region);
			void validateRect(OSWINDOWHANDLE wnd, RECT* r = NULL);
			void validateRegion(OSWINDOWHANDLE wnd, OSREGIONHANDLE region);
			void update(OSWINDOWHANDLE wnd);
			int getUpdateRect(OSWINDOWHANDLE wnd, RECT* r);
			void getUpdateRegion(OSWINDOWHANDLE wnd, OSREGIONHANDLE region);
			int haveGetRandomRegion();
			void getRandomRegion(HDC hdc, OSREGIONHANDLE region);	// sorry, HDC

			void setWndRegion(OSWINDOWHANDLE wnd, OSREGIONHANDLE region, int redraw = FALSE);

			int isDesktopAlphaAvailable();
			int isTransparencyAvailable();
			void setLayeredWnd(OSWINDOWHANDLE wnd, int layered);
			int isLayeredWnd(OSWINDOWHANDLE wnd);
			void setLayeredAlpha(OSWINDOWHANDLE wnd, int amount);
			void updateLayeredWnd(OSWINDOWHANDLE wnd, int x, int y, int w, int h, HDC surfdc, int alpha);
			void moveLayeredWnd(OSWINDOWHANDLE wnd, int x, int y);

			void getClientRect(OSWINDOWHANDLE wnd, RECT* r);
			void getWindowRect(OSWINDOWHANDLE wnd, RECT* r);
			void clientToScreen(OSWINDOWHANDLE wnd, int* x, int* y);
			void screenToClient(OSWINDOWHANDLE wnd, int* x, int* y);

			void setParent(OSWINDOWHANDLE child, OSWINDOWHANDLE newparent);
			OSWINDOWHANDLE getParent(OSWINDOWHANDLE wnd);
			//  void reparent(OSWINDOWHANDLE child, OSWINDOWHANDLE newparent);
			OSWINDOWHANDLE getTopmostChild(OSWINDOWHANDLE wnd);

			void setCapture(OSWINDOWHANDLE wnd);
			void releaseCapture();
			OSWINDOWHANDLE getCapture();

			void revokeDragNDrop(OSWINDOWHANDLE wnd);

			void setWndName(OSWINDOWHANDLE wnd, const wchar_t* name);
			void getWndName(OSWINDOWHANDLE wnd, wchar_t* name, int maxlen);
			void setIcon(OSWINDOWHANDLE wnd, OSICONHANDLE icon, int large = FALSE);

			OSWINDOWHANDLE getActiveWindow();
			void setActiveWindow(OSWINDOWHANDLE wnd);
			void clipOSChildren(OSWINDOWHANDLE wnd, OSREGIONHANDLE reg);
			int alphaStretchBlit(HDC destHDC, int dstx, int dsty, int dstw, int dsth, HDC sourceHDC, int srcx, int srcy, int srcw, int srch);
			OSWINDOWHANDLE getWindowFromPoint(POINT pt);

		};
	}
}
#endif
#endif
