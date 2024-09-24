#include <windows.h>
#include <math.h>

#define M_PI 3.14159265358979323846
extern int (*warand)(void);
extern void line(unsigned char *fb, int X0, int Y0, int X1, int Y1, int w, int h);

static double sc_curtime;
static unsigned int sc_starttime;

static int __inline myftol(double d)
{
  int a;
  __asm
  {
    fld d
    fistp a
    mov eax, a
  }
}

static void __doscope(int fx, int ipos, double i, double v, double &x, double &y, double &sc_tmp)
{
  double r,d;
  switch (fx)
  {
    case 0: x=2.0*(i-0.5); y=v*0.5; return;
    case 1: 
      x=cos(i*M_PI*3.0+sc_curtime)*(0.5+v*0.2); 
      y=sin(i*M_PI*6.0+sc_curtime)*(0.5+v*0.2);    
    return;
    case 2:
      r=i*M_PI*128+sc_curtime; 
      x=cos(r/(64.0+32.0*cos(sc_curtime*M_PI)))*0.4+sin(r)*0.05; 
      y=sin(r/(64.0+32.0*cos(sc_curtime*M_PI)))*0.4+cos(r)*0.05; 
    return;
    case 3:
      r=M_PI*cos(sc_curtime*0.5)*2;
      x=cos(r)*0.3+cos(i*M_PI*2)*v*0.5;
      y=sin(r)*0.3+sin(i*M_PI*2)*v*0.5;
    return;
    case 4:
      r=M_PI*cos(sc_curtime*0.5)*2;
      x=cos(r)*0.3+cos(i*M_PI*2)*(1.0+v)*0.23;
      y=sin(r*1.5)*0.3+sin(i*M_PI*2)*(1.0+v)*0.23;
    return;
    case 5:
      y=2.0*(i-0.5); 
      x=v*0.5 + 0.4*cos(sc_curtime);
    return;
    case 6:
      d=i+v*0.2; 
      r=sc_curtime+i*3.14159*4; 
      x=cos(r)*d*0.6; 
      y=sin(r)*d*0.6; 
    return;
    case 7:
      x=(i-0.5)*2.0; 
      y=0.25-0.5*fabs(sin(i*M_PI*4 + sc_curtime)) + v*0.2; 
    return;
    case 8: 
      r=i*3.14159*2; 
      d=sin(r*3)+v*0.5; 
      x=cos(sc_curtime+r)*d; 
      y=sin(sc_curtime-r)*d;
    return;
    case 9:
      x=2.0*(i-0.5); 
      y=v*v*v*0.7;
    return;
    case 10:
      x=cos(i*8)*0.9*sin(sc_curtime); 
      y=sin(i*8)*0.5*v;
    return;
    case 11:
      d=i-0.9;
      sc_tmp=sc_tmp+v*0.1;
      x=cos(sc_tmp)*d+sin(sc_tmp)*v+0.3;
      y=sin(sc_tmp)*d-cos(sc_tmp)*v; 
    return;
    case 12:
      r=i*8;
      x=cos(r)*0.3+sin(r*(3+cos(sc_curtime*0.5+i*3.0)))*0.2;
      y=sin(r)*0.3+cos(r*(2+2*cos(sc_curtime*0.5+i*3.0)))*0.2;
    return;
    case 13:
      x=v*0.3+sin(i*4*3.14159)*cos(sc_curtime)*0.4;
      y=2.0*(i-0.5);
    return;
    case 14:
      if (!(ipos&3)) x=y=0;
      else
      {
        x=v;
        y=(ipos&8)?1.0:-1.0;
        if (ipos & 4)
        {
          sc_tmp=y;
          y=x;
          x=sc_tmp;
        }
      }
    return;
    case 15:
      x=(ipos&1)?-1:1;
      y=(ipos&30)?0:v;
    return;
    case 16:
      r=ipos * 3.14159 * (0.3);
      x=(0.3+v*0.1)*cos(r);
      y=(0.3+v*0.1)*sin(r);
    return;
  }
}

//#define TEST_SCOPE 14
#define NUM_SC 17

static unsigned int start;

#define INTERVAL 5000

static int curfx[2]={0,NUM_SC},lastfx[2]={0,NUM_SC}, interval_l=INTERVAL;
static double fxblend, lastfxblend;
void drawscope(unsigned char *out, int w, int h, unsigned char *visdata)
{
  double tmp1=0.0;
  double tmp2=0.0;
  double tmp3=0.0;
  double tmp4=0.0;
  int x;
  int lx=0;
  int ly=0;
  double w2=w*65536.0/2.0;
  double h2=h*65536.0/2.0;
  if (!sc_starttime) sc_starttime=GetTickCount();
  sc_curtime=(GetTickCount()-sc_curtime)/1000.0;

  if (GetTickCount()>=start+INTERVAL)
  {
    lastfxblend=fxblend;
    lastfx[0]=curfx[0];
    lastfx[1]=curfx[1];
    start=GetTickCount();
    curfx[0]=warand()%NUM_SC;
    curfx[1]=warand()%NUM_SC;
    interval_l=2500+(warand()&8191);
    if (interval_l > INTERVAL) interval_l=INTERVAL;

    for (x = 0; x < 2; x ++)
      if (curfx[x] == 14 || curfx[x] == 15) curfx[x]=warand()%NUM_SC;
    fxblend=0.25+(warand()&511)/1023.0;
#ifdef TEST_SCOPE
    curfx[0]=TEST_SCOPE;
    lastfx[0]=curfx[0];
    curfx[1]=NUM_SC;
    lastfx[1]=NUM_SC;
    fxblend=lastfxblend=0.0;
#endif
  }
  double weight=(double)(GetTickCount()-start)*(1.0/(double)(interval_l));
  if (weight > 1.0) weight=1.0;
  for (x = 0; x < 576; x ++)
  {
    int xp;
    int yp;
    double xof=0.0, yof=0.0;
    double ix=x/576.0;
    double iy=((visdata[x]^128)-128)/127.5;

    __doscope(curfx[0],x,ix,iy,xof,yof,tmp1);
    {
      double xof2=0.0,yof2=0.0;
      __doscope(curfx[1],x,ix,iy,xof2,yof2,tmp3);
      xof=xof*(1.0-fxblend) + xof2*fxblend;
      yof=yof*(1.0-fxblend) + yof2*fxblend;
    }
    
    {
      double xof2=0.0, yof2=0.0;
      __doscope(lastfx[0],x,ix,iy,xof2,yof2,tmp2);
      {
        double xof3=0.0,yof3=0.0;
        __doscope(lastfx[1],x,ix,iy,xof3,yof3,tmp4);
        xof2=xof2*(1.0-lastfxblend) + xof3*lastfxblend;
        yof2=yof2*(1.0-lastfxblend) + yof3*lastfxblend;
      }
      xof=xof*weight + xof2*(1.0-weight);
      yof=yof*weight + yof2*(1.0-weight);
    }

    xp = myftol(xof * w2 + w2);
    yp = myftol(yof * h2 + h2);

    if (x) line(out,lx,ly,xp,yp,w,h);
    
    lx=xp;
    ly=yp;
  }
}