#ifndef _VIDEO_OVERLAY_H
#define _VIDEO_OVERLAY_H

#include <ddraw.h>
#include <multimon.h>
#include "video.h"

class SubsItem;

class OverlayVideoOutput : public VideoOutputChild {
public:
  OverlayVideoOutput();
  virtual ~OverlayVideoOutput();

  int create(VideoOutput *parent, int w, int h, unsigned int type, int flipit, double aspectratio); //return 1 if ok
  int needChange() { return needchange; }

  int onPaint(HWND hwnd, HDC hdc);
  void displayFrame(const char *buf, int size, int time);

  void goFullScreen();
  void removeFullScreen();

  void timerCallback();

  int showOSD();
  void hideOSD();

  void drawSubtitle(SubsItem *item);
  virtual void resetSubtitle();

private:
  int width, height, flip;
  int needchange;
  unsigned int type;
  VideoOutput *m_parent;
  LPDIRECTDRAW		   lpDD;
  LPDIRECTDRAWSURFACE lpddsOverlay, lpddsPrimary;
  DDCAPS          capsDrv;
  unsigned int    uDestSizeAlign, uSrcSizeAlign;
  DWORD           dwUpdateFlags;
  DDOVERLAYFX     ovfx;
  RECT rs,rd;
  RECT m_oldrd;
  RECT winRect;

  bool initing;
  int is_fullscreen, yuy2_output, uyvy_output;
  
  void getRects(RECT *drs, RECT *drd);

  HFONT subFont;
  RECT subRect;
  SubsItem *curSubtitle;
  int m_fontsize;
};

#endif