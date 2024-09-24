#ifndef _VIDEO_H
#define _VIDEO_H

#include <windows.h>
#include <ddraw.h>
#include <multimon.h>

#include "main.h"

#define NUM_WIDGETS 11

class VideoOutput;
class SubsItem;

class VideoOutputChild {
public:
  VideoOutputChild() { m_mon_x=m_mon_y=0; }
  virtual ~VideoOutputChild() { }
  
  virtual int create(VideoOutput *parent, int w, int h, unsigned int type, int flipit, double aspectratio)=0; //return 1 if ok
  virtual int needChange()=0; //return 1 if need to renegociate video output

  virtual int onPaint(HWND hwnd, HDC hdc) { return 0; } //return 1 if override
  virtual void displayFrame(const char *buf, int size, int time)=0;

  virtual void goFullScreen()=0;
  virtual void removeFullScreen()=0;
  virtual int showOSD() { return 0; }
  virtual void hideOSD() { }

  virtual void timerCallback() { }

  virtual void setPalette(RGBQUAD *pal) { }

  virtual void drawSubtitle(SubsItem *item) { }
  virtual void resetSubtitle() { }

  char m_szTest[512];
  
  void update_monitor_coords(VideoOutput *parent);
  int  m_mon_x;
  int  m_mon_y;
  int  m_found_devguid;
  GUID m_devguid;
  HMONITOR m_monitor_to_find;
};

#include "vid_overlay.h"
#include "vid_ddraw.h"

class VideoOutput : public IVideoOutput {
  public:
    VideoOutput(HWND parent_hwnd=NULL, int initxpos=CW_USEDEFAULT, int initypos=CW_USEDEFAULT);
    ~VideoOutput();
    int open(int w, int h, int vflip, double aspectratio, unsigned int fmt);
    void close();
    void draw(void *frame);
    void drawSubtitle(SubsItem *item);
    void showStatusMsg(const char *text);
    void notifyBufferState(int bufferstate); /* 0-255*/
    int get_latency();
    void setcallback(LRESULT (*msgcallback)(void *token, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam), void *token) { m_msgcallback_tok=token; m_msgcallback=msgcallback; }
    void setNSVDecoder(NSVDecoder *nsv_decoder) { decoder = nsv_decoder; }

    void fullscreen();
    void remove_fullscreen();
    int is_fullscreen();

    HWND getHwnd() { return video_hwnd; }

    void adjustAspect(RECT &rd);

    int vid_vsync;
    int vid_aspectadj;
    int vid_overlays;
    int vid_ddraw;

    void getViewport(RECT *r, HWND wnd, int full);
    void setOutputSize(int w, int h);
    void getOutputSize(int *w, int *h);

    int  osdShowing() { return show_osd; }
    int  osdReady()   { return ctrlrects_ready; }
    void showOSD();
    void hideOSD();
    void drawOSD(HDC hdc, RECT *r);
    int  getOSDbarHeight() { return (show_osd && ctrlrects_ready) ? (ctrlrect_all.bottom - ctrlrect_all.top) : 0; }; 

  private:


    LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static int class_refcnt;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND video_hwnd, video_parent_hwnd;
    NSVDecoder *decoder;

    double aspect;
    int width, height, flip;
    unsigned int type;
    int uyvy_output,yuy2_output;
    int i420_output;
    int is_fs;
    int ignore_mousemove_count;
    int show_osd;
    HWND oldfsparent;
    RECT oldfsrect;  // the old window rect, BEFORE fullscreen mode was entered
    RECT lastfsrect; // the most recent bounding rect when in fullscreen mode
    LONG oldfsstyle;

    int m_bufferstate;
    void resetSubtitle();
    SubsItem *curSubtitle;
    
    // ONSCREEN DISPLAY (osd):
    HFONT osdFontText;
    HFONT osdFontSymbol;
    HGDIOBJ osdProgressBrushBg;
    HGDIOBJ osdProgressBrushFg;
    HGDIOBJ osdProgressPenBg;
    HGDIOBJ osdProgressPenFg;
    HGDIOBJ osdProgressPenBgHilite;
    HGDIOBJ osdBlackBrush;
    void osdHitTest(int x, int y, int dragging);
    int osdLastClickItem;
    HDC     osdMemDC;	 // memory device context
    HBITMAP	osdMemBM;  // memory bitmap (for memDC)
    HBITMAP osdOldBM;  // old bitmap (from memDC)
    int     osdMemBMW; // width of memory bitmap
    int     osdMemBMH; // height of memory bitmap
    int     osdLastMouseX;  // for WM_MOUSEMOVE thresholding, so osd isn't spastic
    int     osdLastMouseY;  // for WM_MOUSEMOVE thresholding, so osd isn't spastic
    RECT    ctrlrect[NUM_WIDGETS]; // relative to [i.e. (0,0) is] upper left corner of the black strip @ the bottom
    RECT    ctrlrect_all;          // relative to [i.e. (0,0) is] upper left corner of the black strip @ the bottom
    int     ctrlrects_ready;

    LRESULT (*m_msgcallback)(void *token, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void *m_msgcallback_tok;

    VideoOutputChild *m_video_output;
    VideoOutputChild *createVideoOutput(int n);

    CRITICAL_SECTION m_cs;
    char *m_statusmsg;

    int m_need_change;

    HBITMAP m_logo;
    int m_logo_w, m_logo_h;

    DWORD m_lastbufinvalid;

#ifdef ACTIVEX_CONTROL
	int m_firstframe;
#endif
};

#endif