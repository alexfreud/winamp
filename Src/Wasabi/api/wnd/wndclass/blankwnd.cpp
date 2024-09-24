#include "precomp.h"

#include "blankwnd.h"
#include <tataki/canvas/canvas.h>
#include <api/wnd/PaintCanvas.h>

BlankWnd::BlankWnd(RGB32 _color) : color(_color)
{
}

int BlankWnd::onPaint(Canvas *canvas)
{
	PaintCanvas pc;
	if (canvas == NULL)
	{
		if (!pc.beginPaint(this)) return 0;
		canvas = &pc;
	}

	canvas->fillRect(&clientRect(), color);

	return 1;
}

