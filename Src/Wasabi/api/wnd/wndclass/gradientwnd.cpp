#include <precomp.h>

#include "gradientwnd.h"


// NOTE
// works now ;)

GradientWnd::GradientWnd()
: bitmap(4,4, getOsWindowHandle())
{
	cache_w = cache_h = 4;
	last_w = last_h = -1;
	recreate = 1;
	setReverseColors(TRUE);
}

GradientWnd::~GradientWnd() 
{
	WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SkinCallbackI*>(this));
}

int GradientWnd::onInit ()
{
	int r = GRADIENTWND_PARENT::onInit();
	WASABI_API_SYSCB->syscb_registerCallback(static_cast<SkinCallbackI*>(this));
	return r;
}

int GradientWnd::onPaint(Canvas *canvas) 
{
	ASSERT(canvas != NULL);
	RECT cr = clientRect();

	int w = cr.right - cr.left, h = cr.bottom - cr.top;
	if (w && h)
	{
		if (w != last_w || h != last_h)
		{
			recreate=1;
		}
		if (w > cache_w || h > cache_h)
		{
			cache_w = max(w, cache_w);
			cache_h = max(h, cache_h);
			// round up to nearest 4
			cache_w = (cache_w+3) & ~3;
			cache_h = (cache_h+3) & ~3;

			bitmap.DestructiveResize(cache_w,cache_h,32);
			recreate = 1;
		}

		if (recreate) 
		{
			ARGB32 *bits = static_cast<ARGB32*>(bitmap.getBits());
			renderGradient(bits, w, h, /*pitch=*/cache_w);
			last_w = w;
			last_h = h;
			recreate=0;
		}
		RECT src = {0,0,w,h};
		bitmap./*getSkinBitmap()->*/blitToRect(canvas, &src, &cr, getPaintingAlpha());
		//bitmap./*getSkinBitmap()->*/blitAlpha(canvas, cr.left, cr.top, getPaintingAlpha());
	}
	return 1;
}

void GradientWnd::onParamChange() 
{
	invalidate();
	recreate = 1;
}

int GradientWnd::skincb_onColorThemeChanged(const wchar_t *newcolortheme)
{
	// TODO: This will refresh after ca 1 sec - we need an instand redraw
	invalidate();
	recreate = 1;
	return 0;
}

