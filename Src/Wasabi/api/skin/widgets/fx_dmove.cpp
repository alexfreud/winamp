#include <precomp.h>

#include <tataki/blending/blending.h>
#include "fx_dmove.h"

#include <api/skin/widgets/layer.h>
#include <math.h>

#define M_PI 3.14159265358979323846


FxDynamicMove::FxDynamicMove() 
: fx_canvas(4,4)
{
	need_flush=false;
	cache_w=cache_h=4;
  m_wmul=0;
  m_tab=0;
  last_m_tab=0;
  m_lastxres=m_lastyres=m_lastpitch=0;
  m_xres=16;
  m_yres=16;
  last_w=last_h=-1;
	inited=0;
  subpixel=1;
  rectcoords=0;
  blend=0;
  wrap=0;
  need_alpha = 1;
  alpha_table = (double *)MALLOC((m_xres+1)*(m_yres+1)*sizeof(double));
  alpha_once= 0;
  can_cache = 1;
  m_lastw = 0;
  m_lasth = 0;
  m_tab_size = 0;
}

FxDynamicMove::~FxDynamicMove() 
{
  FREE(m_wmul);
  FREE(m_tab);
  FREE(last_m_tab);
  FREE(alpha_table);
}

int FxDynamicMove::render(Layer *l, int _w, int _h, int *input, int tw, int th, int twpitch) 
{
  double var_x;
  double var_y;
  double var_d;
  double var_r;


/*if (fx_canvas) {
  HDC dc;
  dc = GetDC(NULL);
  BitBlt(dc, 0, 0, w, h, fx_canvas->getHDC(),0, 0, SRCCOPY);
  ReleaseDC(NULL, dc);
  }*/

  prepareCanvas(_w, _h);

  int w_adj=(tw-2)<<16;
  int h_adj=(th-2)<<16;
  int dowrap=wrap;
  int XRES=m_xres+1;
  int YRES=m_yres+1;
  int ignore_last_compare=0;

  if (XRES > _w) XRES=_w;
  if (YRES > _h) YRES=_h;
  if (XRES < 2) XRES=2;
  if (XRES > 256) XRES=256;
  if (YRES < 2) YRES=2;
  if (YRES > 256) YRES=256;
  if (need_flush || m_lasth != th || m_lastpitch != twpitch || m_lastw != tw || !m_tab || !m_wmul || m_lastxres != XRES || m_lastyres != YRES)
  {
    int y;
    m_lastxres = XRES;
    m_lastyres = YRES;
    m_lastw=tw;
    m_lasth=th;
    m_lastpitch=twpitch;
    FREE(m_wmul);
    m_wmul=(int*)MALLOC(sizeof(int)*th);
    for (y = 0; y < th; y ++) m_wmul[y]=y*twpitch;
    FREE(m_tab);
    FREE(last_m_tab);
    m_tab_size = (XRES*YRES*3 + XRES*6 + 6)*sizeof(int);
    m_tab=(int*)MALLOC(m_tab_size);
    last_m_tab=(int*)MALLOC(m_tab_size);
    ignore_last_compare=1;
  }
	need_flush=false;
  int isblend=blend;

  int issub=subpixel;
  if (!issub)
  {
    w_adj=(tw-1)<<16;
    h_adj=(th-1)<<16;
  }
  if (w_adj<0) w_adj=0;
  if (h_adj<0) h_adj=0;

  {
    int x;
    int y;
    int *tabptr=m_tab;

    double xsc=2.0/tw,ysc=2.0/th;
    double dw2=((double)tw*32768.0);
    double dh2=((double)th*32768.0);
    double max_screen_d=sqrt((double)(tw*tw+th*th))*0.5;
    
    double divmax_d=1.0/max_screen_d;

    max_screen_d *= 65536.0;

    double _var_alpha = 0.50;
    int yc_pos, yc_dpos, xc_pos, xc_dpos;
    yc_pos=0;
    xc_dpos = (tw<<16)/(XRES-1);
    yc_dpos = (th<<16)/(YRES-1);
    for (y = 0; y < YRES; y ++)
    {
      xc_pos=0;
      for (x = 0; x < XRES; x ++)
      {
        double xd,yd;
        
        xd=((double)xc_pos-dw2)*(1.0/65536.0);
        yd=((double)yc_pos-dh2)*(1.0/65536.0);

        xc_pos+=xc_dpos;

        var_x=xd*xsc;
        var_y=yd*ysc;
        var_d=sqrt(xd*xd+yd*yd)*divmax_d;
        var_r=atan2(yd,xd) + M_PI*0.5;

        double _var_r=var_r, _var_d=var_d;
        if (isblend && need_alpha) {
          _var_alpha = l->fx_onGetPixelA(var_r, var_d, var_x, var_y);
          alpha_table[y*YRES+x] = _var_alpha;
        } else if (isblend && !need_alpha) {
          _var_alpha = alpha_table[y*YRES+x];
        }
        if (!rectcoords) {
          double t_var_r = l->fx_onGetPixelR(var_r, var_d, var_x, var_y);
          _var_d = l->fx_onGetPixelD(var_r, var_d, var_x, var_y);
          _var_r = t_var_r;
        }
        double _var_x=var_x, _var_y=var_y;
        if (rectcoords) {
          double t_var_x = l->fx_onGetPixelX(var_r, var_d, var_x, var_y);
          _var_y = l->fx_onGetPixelY(var_r, var_d, var_x, var_y);
          _var_x = t_var_x;
        }

        int tmp1,tmp2,tmp3;
        if (!rectcoords)
        {
          _var_d *= max_screen_d;
          _var_r -= M_PI*0.5;
          tmp1=(int) (dw2 + cos(_var_r) * _var_d);
          tmp2=(int) (dh2 + sin(_var_r) * _var_d);
        }
        else
        {
          tmp1=(int) ((_var_x+1.0)*dw2);
          tmp2=(int) ((_var_y+1.0)*dh2);
        }
        if (!dowrap)
        {
          if (tmp1 < 0) tmp1=0;
          if (tmp1 > w_adj) tmp1=w_adj;
          if (tmp2 < 0) tmp2=0;
          if (tmp2 > h_adj) tmp2=h_adj;
        }
        *tabptr++ = tmp1;
        *tabptr++ = tmp2;
        tmp3 = (int)(_var_alpha*65536.0*256.0);
        if (tmp3 < 0) tmp3=0;
        if (tmp3 > 0xff0000) tmp3=0xff0000;
        *tabptr++=tmp3;
      }
      yc_pos+=yc_dpos;
    }
  if (alpha_once)
    need_alpha=0;
  }

  if (can_cache && !ignore_last_compare && !MEMCMP(m_tab, last_m_tab, m_tab_size)) {
    // cached, do nothing
    // DebugString("cache hit!\n");
  } else {
    MEMCPY(last_m_tab, m_tab, m_tab_size);

    // yay, the table is generated. now we do a fixed point 
    // interpolation of the whole thing and pray.
    {
      int *interptab=m_tab+XRES*YRES*3;
      int *rdtab=m_tab;
      unsigned int *in=(unsigned int *)input;
      unsigned int *blendin=(unsigned int *)input;
      unsigned int *out=(unsigned int *)fx_canvas.getBits();
      int yseek=1;
      int xc_dpos, yc_pos=0, yc_dpos;
      xc_dpos=(tw<<16)/(XRES-1);
      yc_dpos=(th<<16)/(YRES-1);
      int lypos=0;
      int yl=_h;
      while (yl>0)
      {
        yc_pos+=yc_dpos;   
        yseek=(yc_pos>>16)-lypos;
        if (!yseek) return 0;
        lypos=yc_pos>>16;
        int l=XRES;
        int *stab=interptab;
        int xr3=XRES*3;
        while (l--)
        {
          int tmp1, tmp2,tmp3;
          tmp1=rdtab[0];
          tmp2=rdtab[1];
          tmp3=rdtab[2];
          stab[0]=tmp1;
          stab[1]=tmp2;
          stab[2]=(rdtab[xr3]-tmp1)/yseek;
          stab[3]=(rdtab[xr3+1]-tmp2)/yseek;
          stab[4]=tmp3;
          stab[5]=(rdtab[xr3+2]-tmp3)/yseek;
          rdtab+=3;
          stab+=6;
        }

        if (yseek > yl) yseek=yl;
        yl-=yseek;

        if (yseek > 0) while (yseek--)
        {
          int d_x;
          int d_y;
          int d_a;
          int ap;
          int seek;
          int *seektab=interptab;
          int xp,yp;
          int l=_w;
          int lpos=0;
          int xc_pos=0;
          while (l>0)
          {
            xc_pos+=xc_dpos;
            seek=(xc_pos>>16)-lpos;
            if (!seek) 
            {
              #ifndef NO_MMX
                Blenders::BLEND_MMX_END();
              #endif
              return 0;
            }
            lpos=xc_pos>>16;
            xp=seektab[0];
            yp=seektab[1];
            ap=seektab[4];
            d_a=(seektab[10]-ap)/(seek);
            d_x=(seektab[6]-xp)/(seek);
            d_y=(seektab[7]-yp)/(seek);
            seektab[0] += seektab[2];
            seektab[1] += seektab[3];
            seektab[4] += seektab[5];
            seektab+=6;
        
            if (seek>l) seek=l;
            l-=seek;
            if (seek>0)
            {
  // normal loop
  #define NORMAL_LOOP(Z) while (seek--) { Z; xp+=d_x; yp+=d_y; }

  // wrapping loop
  #define WRAPPING_LOOPS(Z) \
    if (d_x <= 0 && d_y <= 0) NORMAL_LOOP(if (xp < 0) xp += w_adj; if (yp < 0) yp += h_adj; Z) \
    else if (d_x <= 0) NORMAL_LOOP(if (xp < 0) xp += w_adj; if (yp >= h_adj) yp-=h_adj; Z) \
    else if (d_y <= 0) NORMAL_LOOP(if (xp >= w_adj) xp-=w_adj; if (yp < 0) yp += h_adj; Z) \
    else NORMAL_LOOP(if (xp >= w_adj) xp-=w_adj; if (yp >= h_adj) yp-=h_adj; Z)


              // this is uber gay. J1, D4, L or J1_MMX, D4_MMX, L_MMX
  #define LOOPS(DO,J1,D4,L)  \
                if ((isblend&2) && issub) DO(*out++=Blenders::BLEND_AD##J1(Blenders::BLEN##D4(in+(xp>>16)+m_wmul[yp>>16],twpitch,xp,yp),*blendin++,ap>>16); ap+=d_a) \
                else if (isblend&2) DO(*out++=Blenders::BLEND_AD##J1(in[(xp>>16)+m_wmul[yp>>16]],*blendin++,ap>>16); ap+=d_a) \
                else if (isblend && issub) DO(*out++=Blenders::BLEND_MU##L(Blenders::BLEN##D4(in+(xp>>16)+m_wmul[yp>>16],twpitch,xp,yp),ap>>16); ap+=d_a) \
                else if (isblend) DO(*out++=Blenders::BLEND_MU##L(in[(xp>>16)+m_wmul[yp>>16]],ap>>16); ap+=d_a) \
                else if (issub) DO(*out++=Blenders::BLEN##D4(in+(xp>>16)+m_wmul[yp>>16],twpitch,xp,yp)) \
                else DO(*out++=in[(xp>>16)+m_wmul[yp>>16]])

              if (!dowrap)
              {
                #ifndef NO_MMX
                  if (Blenders::MMX_AVAILABLE())
                  {
                    LOOPS(NORMAL_LOOP,J1_MMX,D4_MMX,L_MMX)
                  }
                  else
                #endif
                  {
                    LOOPS(NORMAL_LOOP,J1,D4,L)
                  }
              }
              else // dowrap
              {
                xp %= w_adj+1; yp %= h_adj+1;
                if (xp < 0) xp+=w_adj; if (yp < 0) yp+=h_adj;

                if (d_x < -w_adj || d_x > w_adj) d_x=0; if (d_y < -h_adj || d_y > h_adj) d_y=0;

                #ifndef NO_MMX
                  if (Blenders::MMX_AVAILABLE())
                  {
                    LOOPS(WRAPPING_LOOPS,J1_MMX,D4_MMX,L_MMX)
                  }
                  else
                #endif
                  {
                    LOOPS(WRAPPING_LOOPS,J1,D4,L)
                  }
              }
            }
          }
          blendin+=twpitch-_w;

          // adjust final (rightmost elem) part of seektab
          seektab[0] += seektab[2];
          seektab[1] += seektab[3];
          seektab[4] += seektab[5];
        }
      }
    }
  }

/*HDC dc = GetDC(NULL);
BitBlt(dc, 0, 0, w, h, fx_canvas->getHDC(),0, 0, SRCCOPY);
ReleaseDC(NULL, dc);*/

  #ifndef NO_MMX
    Blenders::BLEND_MMX_END();
  #endif
  return 0;
};

void FxDynamicMove::setWrap(int i) {
  wrap = i;
}

void FxDynamicMove::setRect(int i) {
  rectcoords = i;
  
}
void FxDynamicMove::setBilinear(int i) {
  subpixel = i;
}

void FxDynamicMove::setAlphaMode(int i) {
  blend = i;
}

void FxDynamicMove::setAlphaOnce(int i) {
  alpha_once = i;
}

void FxDynamicMove::setGridSize(int x, int y) {
  m_xres = x;
  m_yres = y;
  FREE(alpha_table);
  alpha_table = (double *)MALLOC((x+1)*(y+1)*sizeof(double));
  need_alpha = 1;
}

BltCanvas *FxDynamicMove::getBltCanvas() 
{
  return &fx_canvas;
}

void FxDynamicMove::prepareCanvas(int w, int h) {
  if (w != last_w || h != last_h) 
	{
		if (cache_w < w || cache_h < h)
		{
			cache_w = max(cache_w, w);
			cache_h = max(cache_h, h);
			fx_canvas.DestructiveResize(cache_w, cache_h, 32);
		}
    last_w = w;
    last_h = h;
  }
}

void FxDynamicMove::setCanCache(int i) {
  can_cache = i;
}

void FxDynamicMove::flushCache()
{
	need_flush=true;
}