#ifndef __FX_DMOVE_H
#define __FX_DMOVE_H

#include "fx.h"
#include <tataki/canvas/bltcanvas.h>

class Layer;

class FxDynamicMove : public Fx {
  public:

    FxDynamicMove();
    ~FxDynamicMove();

    virtual int render(Layer *l, int _w, int _h, int *input, int tw, int th, int twpitch);
    virtual void setWrap(int i);
    virtual void setRect(int i);
    virtual void setBilinear(int i);
    virtual void setAlphaMode(int i);
    virtual void setAlphaOnce(int i);
    virtual void setCanCache(int i);
    virtual void setGridSize(int x, int y);
    virtual BltCanvas *getBltCanvas();
    virtual void prepareCanvas(int w, int h);
		virtual void flushCache();
  private:

    BltCanvas fx_canvas;
    int last_w, last_h;
		int cache_w, cache_h;

    int m_lastw,m_lasth;
    int m_lastxres, m_lastyres, m_xres, m_yres, m_lastpitch;
    int *m_wmul;
    int *m_tab;
    int *last_m_tab;
    int m_tab_size;
    int subpixel,rectcoords,blend,wrap;
    int inited;
    int need_alpha;
    int alpha_once;
    int can_cache;
		bool need_flush;
    double *alpha_table;

};

#endif