#ifndef _DRAWPOLY_H
#define _DRAWPOLY_H

#include <bfc/wasabi_std.h>

class Draw {
public:
  static void beginPolygon(ARGB32 *bits, int w, int h, ARGB32 color);
  static void addPoint(int x, int y);
  static void endPolygon();
  static void drawPointList(ARGB32 *bits, int w, int h, const wchar_t *pointlist);
};

// x,y;x,y;x,y;x,y=R,G,B|x,y;x,y;x,y;=R,G,B

#endif
