#ifndef _GRADIENTWND_H
#define _GRADIENTWND_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <bfc/draw/gradient.h>
#include <tataki/canvas/bltcanvas.h>

#define GRADIENTWND_PARENT GuiObjectWnd
class GradientWnd : public GRADIENTWND_PARENT, public Gradient, public SkinCallbackI {
public:
	GradientWnd();
	virtual ~GradientWnd();

	virtual int onPaint(Canvas *canvas);

protected:
	virtual void onParamChange();

private:
	int recreate;

	int last_w, last_h;
	int cache_w, cache_h;
	BltCanvas bitmap;

protected:
	int onInit();
	int skincb_onColorThemeChanged(const wchar_t *newcolortheme);
};

#endif
