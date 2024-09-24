#include <precomp.h>
#include "qpaintwnd.h"
#include <tataki/canvas/bltcanvas.h>
#include "../nu/threadpool/TimerHandle.hpp"

#define TIMER_QUICKPAINT 0x650

// thread context, this is here so we can avoid the windows types in quickpaintwnd.h
class QuickPaintContext
{
public:
	QuickPaintContext(QuickPaintWnd *_wnd, int timeout)
	{
		killswitch = 0;
		wnd = _wnd;
		death = CreateEvent(NULL, FALSE, FALSE, NULL);
		timer_ms = timeout;
		WASABI_API_THREADPOOL->AddHandle(0, timer_handle, QPThreadPoolFunc, (void *)this, 0, 0/*api_threadpool::FLAG_LONG_EXECUTION*/);
		timer_handle.Wait(timer_ms);
	}

	~QuickPaintContext()
	{
		CloseHandle(death);
		timer_handle.Close();
	}

	void Kill()
	{
		InterlockedExchangePointer((volatile PVOID*)&wnd, 0);
		killswitch=1;
		WaitForSingleObject(death, INFINITE);
	}

	QuickPaintWnd *wnd;
	TimerHandle timer_handle;
	HANDLE death;
	volatile int killswitch;
	int timer_ms;
	static int QPThreadPoolFunc(HANDLE h, void *user_data, intptr_t t);
};

int QuickPaintContext::QPThreadPoolFunc(HANDLE h, void *user_data, intptr_t t)
{
	QuickPaintContext *context = (QuickPaintContext *)user_data;

	if (context->killswitch)
	{
		WASABI_API_THREADPOOL->RemoveHandle(0, h);
		SetEvent(context->death);
	}
	else
	{
		DWORD start = GetTickCount();
		QuickPaintWnd *wnd = 0;
		InterlockedExchangePointer((volatile PVOID*)&wnd, context->wnd);
		if (wnd)
		{
			wnd->quickPaint();
			TimerHandle timer_handle(h);
			DWORD end = GetTickCount();
			if (end-start > (DWORD)context->timer_ms)
				timer_handle.Wait(1);
			else
				timer_handle.Wait(context->timer_ms - (end - start));
		}
	}

	return 0;
}


// -----------------------------------------------------------------------
QuickPaintWnd::QuickPaintWnd()
{
	invalidates_required = 0;
	realtime = 1;
	canvas_w = -1;
	canvas_h = -1;
	timerset = 0;
	speed = 25;
	enabled = 0;
	render_canvas1 = NULL;
	render_canvas2 = NULL;
	paint_canvas = NULL;
	thread_context = 0;

	while (1)
	{
		svc_skinFilter *obj = sfe.getNext();
		if (!obj) break;
		filters.addItem(obj);
	}
	invalidated = 0;
}

// -----------------------------------------------------------------------
QuickPaintWnd::~QuickPaintWnd()
{
	foreach(filters)
		sfe.release(filters.getfor());
	endfor;

	KillThread();

	delete render_canvas1;
	delete render_canvas2;
}

// -----------------------------------------------------------------------
void QuickPaintWnd::setRealtime(int rt)
{
	realtime = rt;
}

// -----------------------------------------------------------------------
int QuickPaintWnd::getRealtime() const
{
	return realtime;
}

// -----------------------------------------------------------------------
void QuickPaintWnd::setSpeed(int ms)
{
	speed = ms;
	if (enabled && timerset)
	{
		if (thread_context)
			thread_context->timer_ms = ms;
		// let it change the timer value on the invalidate() timer
		killTimer(TIMER_QUICKPAINT);
		setTimer(TIMER_QUICKPAINT, getSpeed());
	}
}

// -----------------------------------------------------------------------
void QuickPaintWnd::startQuickPaint()
{
	enabled = 1;
	if (!isInited()) return;
	CreateRenderThread();
	timerset=1;
}

// -----------------------------------------------------------------------
void QuickPaintWnd::stopQuickPaint()
{
	enabled = 0;
	if (!isInited()) return;
	KillThread();
	timerset=0;
}

// -----------------------------------------------------------------------
int QuickPaintWnd::isQuickPainting()
{
	return enabled;
}

// -----------------------------------------------------------------------
int QuickPaintWnd::getSpeed()
{
	return speed;
}

// -----------------------------------------------------------------------
int QuickPaintWnd::onInit()
{
	QUICKPAINTWND_PARENT::onInit();
	if (enabled)
	{
		ASSERT(!thread_context);
		CreateRenderThread();
		timerset = 1;
	}
	return 1;
}

// -----------------------------------------------------------------------
void QuickPaintWnd::timerCallback(int id)
{
	switch (id)
	{
	case TIMER_QUICKPAINT:
		if (invalidates_required)
		{
			invalidated = 1;

			if (getRealtime() && isVisible() && !isMinimized())
				cascadeRepaint();
			else
				invalidate();

			InterlockedExchange(&invalidates_required, 0);
		}
		//quickPaint();
		break;
	default:
		QUICKPAINTWND_PARENT::timerCallback(id);
	}
}

void QuickPaintWnd::SetPaintingCanvas(BltCanvas *c)
{
	InterlockedExchangePointer((volatile PVOID*)&paint_canvas, c);
}

BltCanvas *&QuickPaintWnd::GetDrawingConvas()
{
	if (paint_canvas == render_canvas2)
		return render_canvas1;
	else
		return render_canvas2;
}

// -----------------------------------------------------------------------
int QuickPaintWnd::quickPaint()
{
	int repaint=0;

	int w, h;
	getQuickPaintSize(&w, &h);

	if (wantEvenAlignment())
	{
		if (w & 1) w++;
		if (h & 1) h++;
	}

	if (w == 0 && h == 0) return 0;

	BltCanvas *&render_canvas = GetDrawingConvas();
	int newone = 0;

	if (canvas_w != w || canvas_h != h)
	{
		delete render_canvas1; render_canvas1=0;
		delete render_canvas2; render_canvas2=0;
	}

	if (!render_canvas)
	{
		render_canvas = new BltCanvas(w, wantNegativeHeight() ? -h : h, getOsWindowHandle());
		canvas_w = w;
		canvas_h = h;
		newone = 1;
	}

	repaint = onQuickPaint(render_canvas, canvas_w, canvas_h, newone);

	SetPaintingCanvas(render_canvas);
	if (repaint)
		InterlockedIncrement(&invalidates_required);

	return repaint;
}

// -----------------------------------------------------------------------
void QuickPaintWnd::getQuickPaintSize(int *w, int *h)
{
	RECT r;
	getClientRect(&r);
	if (w) *w = r.right - r.left;
	if (h) *h = r.bottom - r.top;
}

// -----------------------------------------------------------------------
void QuickPaintWnd::getQuickPaintSource(RECT *r)
{
	ASSERT(r != NULL);
	r->left = 0;
	r->right = canvas_w;
	r->top = 0;
	r->bottom = canvas_h;
}

// -----------------------------------------------------------------------
void QuickPaintWnd::getQuickPaintDest(RECT *r)
{
	ASSERT(r != NULL);
	getClientRect(r);
}

// -----------------------------------------------------------------------
void QuickPaintWnd::onSetVisible(int show)
{
	QUICKPAINTWND_PARENT::onSetVisible(show);
	if (!show)
	{
		if (timerset)
		{
			KillThread();
			timerset = 0;
		}
	}
	else
	{
		if (enabled && !timerset)
		{
			CreateRenderThread();

			timerset = 1;
		}
	}
}

// -----------------------------------------------------------------------
int QuickPaintWnd::onPaint(Canvas *canvas)
{
	QUICKPAINTWND_PARENT::onPaint(canvas);

	if (!enabled) return 1;

	BltCanvas *render_canvas;
	InterlockedExchangePointer((volatile PVOID*)&render_canvas, paint_canvas);
	if (!render_canvas) return 1;

	RECT r;
	getQuickPaintDest(&r);
	RECT sr;
	getQuickPaintSource(&sr);

	if (invalidated && wantFilters())
	{
		foreach(filters)
			filters.getfor()->filterBitmap((unsigned char *)render_canvas->getBits(), canvas_w, canvas_h, 32, NULL, getFiltersGroup());
		endfor;
		invalidated = 0;
	}

	render_canvas->/*getSkinBitmap()->*/stretchToRectAlpha(canvas, &sr, &r, getPaintingAlpha());
	InterlockedExchange(&invalidates_required, 0);
	return 1;
}

void QuickPaintWnd::KillThread()
{
	if (thread_context)
	{
		killTimer(TIMER_QUICKPAINT);
		thread_context->Kill();
		delete thread_context;
		thread_context = 0;
	}
}

void QuickPaintWnd::CreateRenderThread()
{
	int sp = getSpeed();
	if (!thread_context)
	{
		thread_context = new QuickPaintContext(this, sp);
	}
	else
		thread_context->timer_ms = sp;
	setTimer(TIMER_QUICKPAINT, sp);
}