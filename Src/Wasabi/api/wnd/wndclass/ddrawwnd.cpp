#include "precomp.h"
#include <process.h>

#include "ddrawwnd.h"
#include "../bfc/canvas.h"
#include "../bfc/region.h"

DDrawWnd::DDrawWnd() {
  m_lpDD = NULL;
  lpClipper = NULL;
  m_lpRenderSurf = NULL;
  m_lpPrimSurf = NULL;
}

DDrawWnd::~DDrawWnd() {
  deleteFrameBuffer(NULL);
}

void DDrawWnd::deleteFrameBuffer(Canvas *canvas) {
  if (m_lpRenderSurf) m_lpRenderSurf->Release();
  if (m_lpPrimSurf) m_lpPrimSurf->Release();
  if (lpClipper) lpClipper->Release();
  if (m_lpDD) m_lpDD->Release();
  m_lpRenderSurf = NULL;
  m_lpPrimSurf = NULL;
  m_lpDD = NULL;
  lpClipper = NULL;
  ddlist.removeItem(this);
}

int DDrawWnd::onInit() {
  DDRAWWND_PARENT::onInit();
  if (!allow_dd) return 1;
  return 1;
}

Canvas *DDrawWnd::createFrameBuffer(int _w, int _h) { 

  if (!allow_dd) return DDRAWWND_PARENT::createFrameBuffer(_w, _h);

  if (virtualCanvas && !m_lpPrimSurf)
    DDRAWWND_PARENT::deleteFrameBuffer(virtualCanvas);

  deleteFrameBuffer(NULL);

  int resize_h = 8;
  int resize_w = 8;

  w = _w;
  h = _h;

  if (DirectDrawCreate(NULL,&m_lpDD,NULL) != DD_OK) {
    m_lpDD=NULL;
		MessageBox(gethWnd(),"Error creating ddraw object","DDraw",0);
		return NULL;
	}

  int dbl=0;

  m_lpDD->SetCooperativeLevel(gethWnd(), DDSCL_NORMAL);
  resize_w=(((w>>dbl)+3)&~3);
//  g_noshoww=resize_w-(w>>dbl);
  resize_h=h>>dbl;

	DDSURFACEDESC DDsd={sizeof(DDsd),};

	DDsd.dwFlags = DDSD_CAPS;
	DDsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (m_lpDD->CreateSurface(&DDsd, &m_lpPrimSurf, NULL) != DD_OK) {
    m_lpPrimSurf=0;
		return NULL;
	}

  if (m_lpDD->CreateClipper(0, &lpClipper, NULL) != DD_OK ) {
    m_lpPrimSurf->Release();
    m_lpPrimSurf=0;
	  return NULL;
  }
    
  lpClipper->SetHWnd(0, gethWnd());
  m_lpPrimSurf->SetClipper(lpClipper);

	DDsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PITCH|DDSD_PIXELFORMAT;
	DDsd.dwWidth=resize_w;
	DDsd.dwHeight=resize_h;
	DDsd.lPitch=resize_w*sizeof(int);
	DDsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
	DDsd.ddpfPixelFormat.dwSize = sizeof(DDsd.ddpfPixelFormat);
	DDsd.ddpfPixelFormat.dwFlags=DDPF_RGB;
	DDsd.ddpfPixelFormat.dwRGBBitCount = 32;
	DDsd.ddpfPixelFormat.dwRBitMask=0xff0000;
	DDsd.ddpfPixelFormat.dwGBitMask=0x00ff00;
	DDsd.ddpfPixelFormat.dwBBitMask=0x0000ff;
	if (m_lpDD->CreateSurface(&DDsd, &m_lpRenderSurf, NULL) != DD_OK) {
    m_lpRenderSurf->Release();
    m_lpPrimSurf->Release();
    lpClipper->Release();
    m_lpRenderSurf=0;
    m_lpPrimSurf=0;
    lpClipper=0;
		return NULL;
	}
  fb_canvas = new DDSurfaceCanvas(m_lpRenderSurf, w, h);

  ddlist.addItem(this);

  if (!thread)
    startThread();

  return fb_canvas;
}

int DDrawWnd::virtualBeforePaint(api_region *r) {
  if (!allow_dd) return DDRAWWND_PARENT::virtualBeforePaint(r);
  EnterCriticalSection(&DDrawWnd::cs);
  fb_canvas->enter();
  return 1;
}

int DDrawWnd::virtualAfterPaint(api_region *r) {
  if (!allow_dd) return DDRAWWND_PARENT::virtualAfterPaint(r);
  fb_canvas->exit();
  LeaveCriticalSection(&DDrawWnd::cs);
  return 1;
}

void DDrawWnd::virtualCanvasCommit(Canvas *canvas, RECT *internalrect, double ra) {
  if (!allow_dd) { DDRAWWND_PARENT::commitFrameBuffer(canvas, internalrect, ra); return; }

  internalrect->left = MAX(0, (int)internalrect->left);
  internalrect->top = MAX(0, (int)internalrect->top);
  internalrect->right = MAX(w, (int)internalrect->right);
  internalrect->bottom = MAX(h, (int)internalrect->bottom);

  RECT wr;
  RECT screenrect = *internalrect;

  getWindowRect(&wr);
  screenrect.left += wr.left;
  screenrect.top += wr.top;
  screenrect.right += wr.left;
  screenrect.bottom += wr.top;

  if (ra == 1.0) {
      if (m_lpPrimSurf->Blt(&screenrect,m_lpRenderSurf,internalrect,DDBLT_WAIT,NULL) == DDERR_SURFACELOST) {
        m_lpPrimSurf->Restore();
      }
  } else {
      RECT rcr=screenrect;
      rcr.left = wr.left + (int)((double)internalrect->left*ra);
      rcr.top = wr.top + (int)((double)internalrect->top*ra);
      rcr.right = rcr.left + (int)((double)(internalrect->right-internalrect->left)*ra);
      rcr.bottom = rcr.top + (int)((double)(internalrect->bottom-internalrect->top)*ra);

      if (m_lpPrimSurf->Blt(&rcr,m_lpRenderSurf,internalrect,DDBLT_WAIT,NULL) == DDERR_SURFACELOST) 
        m_lpPrimSurf->Restore();
	}
}

void DDrawWnd::startThread() {
  DWORD id;
  quitthread=0;
  InitializeCriticalSection(&cs);
  thread =	(HANDLE)_beginthreadex(NULL,0,renderThread,0,0,(unsigned int *)&id);  
}

void DDrawWnd::stopThread() {
  quitthread = 1;
}

unsigned int WINAPI DDrawWnd::renderThread(void *) {
  while (!quitthread) {
    for (int i=0;i<ddlist.getNumItems();i++)  
      ddlist.enumItem(i)->flushPaint();
    Sleep(MIN(MAX(sleep_val,1),100));
  }
  _endthreadex(0);
  return 1;
}

void DDrawWnd::invalidate() {
  if (!allow_dd) { DDRAWWND_PARENT::invalidate(); return; }
  DDRAWWND_PARENT::deferedInvalidate();
}

void DDrawWnd::invalidateRect(RECT *r) {
  if (!allow_dd) { DDRAWWND_PARENT::invalidateRect(r); return; }
  DDRAWWND_PARENT::deferedInvalidateRect(r);
}

void DDrawWnd::invalidateRgn(api_region *rgn) {
  if (!allow_dd) { DDRAWWND_PARENT::invalidateRgn(rgn); return; }
  DDRAWWND_PARENT::deferedInvalidateRgn(rgn);
}

void DDrawWnd::validate() {
  if (!allow_dd) { DDRAWWND_PARENT::validate(); return; }
  DDRAWWND_PARENT::deferedValidate();
}

void DDrawWnd::validateRect(RECT *r) {
  if (!allow_dd) { DDRAWWND_PARENT::validateRect(r); return; }
  DDRAWWND_PARENT::deferedValidateRect(r);
}

void DDrawWnd::validateRgn(api_region *rgn) {
  if (!allow_dd) { DDRAWWND_PARENT::validateRgn(rgn); return; }
  DDRAWWND_PARENT::deferedValidateRgn(rgn);
}


CRITICAL_SECTION DDrawWnd::cs;
HANDLE DDrawWnd::thread=NULL;
int DDrawWnd::quitthread=0;
PtrList<DDrawWnd> DDrawWnd::ddlist;
int DDrawWnd::allow_dd = 1;
int DDrawWnd::sleep_val = 10;


