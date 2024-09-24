#ifndef _GRADIENT_H
#define _GRADIENT_H

#include <bfc/wasabi_std.h>
#include <bfc/string/StringW.h>
#include <tataki/color/filteredcolor.h>

class GradientPoint 
{
public:
  GradientPoint(float p, ARGB32 c, const wchar_t *group=L"") : pos(p), dist(0), color(c, group), x(0), y(0) { }
  float pos;
  double dist;
  FilteredColor color;
  float x, y;
  static int compareItem(GradientPoint *p1, GradientPoint* p2) {
    int r = CMP3(p1->pos, p2->pos);
    if (r == 0) return CMP3(p1, p2);
    else return r;
  }
};


class Gradient 
{
public:
  Gradient();
  virtual ~Gradient();

  void setX1(float x1);
  void setY1(float y1);
  void setX2(float x2);
  void setY2(float y2);
  
  void clearPoints();
  void addPoint(float pos, ARGB32 color);

  // "pos=color;pos=color" "0.25=34,45,111"
  void setPoints(const wchar_t *str);

  void setReverseColors(int c);

  void setAntialias(int c);

  void setMode(const wchar_t *mode);

  void setGammaGroup(const wchar_t *group);

  // note: this will automatically premultiply against alpha
  void renderGradient(ARGB32 *bits, int width, int height, int pitch=0);

protected:
  virtual void onParamChange() { }

  ARGB32 getPixelCirc(double x, double y);

private:
  float gradient_x1, gradient_y1, gradient_x2, gradient_y2;
	class GradientList : public PtrListQuickSorted<GradientPoint, GradientPoint> { };
  GradientList list;
  void renderGrad(ARGB32 *bits, int len, int *positions);
  int reverse_colors;
  int antialias;
  StringW mode, gammagroup;
};

#endif
