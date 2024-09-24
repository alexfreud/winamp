#ifndef _VIDEO_DDRAW_H
#define _VIDEO_DDRAW_H

#include <ddraw.h>
#include "video.h"

class SubsItem;

class DDrawVideoOutput : public VideoOutputChild {
public:
  DDrawVideoOutput();
  virtual ~DDrawVideoOutput();

  int create(VideoOutput *parent, int w, int h, unsigned int type, int flipit, double aspectratio); //return 1 if ok
  int needChange() { return needchange; }

  int onPaint(HWND hwnd, HDC hdc);
  void displayFrame(const char *buf, int size, int time);

  void goFullScreen();
  void removeFullScreen();

  void timerCallback();

  void setPalette(RGBQUAD *pal) { m_palette=pal; }

  int showOSD();
  void hideOSD();

  void drawSubtitle(SubsItem *item);
  void resetSubtitle();

private:
  int width, height, flip;
  int needchange;
  unsigned int type;
  VideoOutput *m_parent;
  LPDIRECTDRAW		   lpDD;
  LPDIRECTDRAWSURFACE lpddsOverlay, lpddsPrimary, lpddsSTTemp;
  int sttmp_w, sttmp_h;
  DDCAPS          capsDrv;
  unsigned int    uDestSizeAlign, uSrcSizeAlign;
  DWORD           dwUpdateFlags;
  RECT rs,rd;
  RECT lastresizerect;

  bool initing;
  int is_fullscreen;

  LPDIRECTDRAWCLIPPER lpddsClipper;
  DDPIXELFORMAT m_ddpf;
  int m_depth;
  RGBQUAD *m_palette;

  HFONT subFont;
  RECT subRect;
  SubsItem *m_lastsubtitle;
  int m_sub_needremeasure;
  RECT winRect;
  int m_fontsize;
};

#endif
