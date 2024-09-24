#include <windows.h>

#include <math.h>

#define M_PI 3.14159265358979323846
extern int (*warand)(void);
static int XRES=24;
static int YRES=16;
static int g_w,g_h;
static int *m_wmul;
static int *m_tab;

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

static double getvis(unsigned char *visdata, int bc, int bw, int ch, int xorv)
{
	int x = 0;
	int accum = 0;
	if (ch && ch != 1 && ch != 2) return 0.0;

	if (bw < 1) bw=1;
	bc-=bw/2;
	if (bc < 0) 
	{
		bw+=bc;
		bc=0;
	}
	if (bc > 575) bc=575;
	if (bc+bw > 576) bw=576-bc;


	if (!ch)
	{
		for (x = 0; x < bw; x ++) 
		{
			accum+=(visdata[bc]^xorv)-xorv;
			accum+=(visdata[bc+576]^xorv)-xorv;
			bc++;
		}
		return (double)accum / ((double)bw*255.0);
	}
	else 
	{
		if (ch == 2) visdata+=576;
		for (x = 0; x < bw; x ++) accum+=(visdata[bc++]^xorv)-xorv;
		return (double)accum / ((double)bw*127.5);
	}
}

static unsigned char *m_visdata;

static double getosc(double band, double bandw)
{
  return getvis((unsigned char *)m_visdata,myftol(band*576.0),
    myftol(bandw*576.0),0,128);
}

static double fx_gstarttime,fx_curtime;
static double ef10_sc;

static __inline double sign(double p)
{
	if (p < 0.0) return -1.0;
	return 1.0;
}

static __inline double my_asin(double v)
{
  double tmp;
  __asm {
    fld qword ptr [v]
    fmul qword ptr [v]
    fld  st(0)           //Duplicate X**2 on tos.
    fld1                    //Compute 1-X**2.
    fsubr
    fdiv                    //Compute X**2/(1-X**2).
    fsqrt                  //Compute sqrt(x**2/(1-X**2)).
    fld1                    //To compute full arctangent.
    fpatan                 //Compute atan of the above.
    fstp qword ptr [tmp]
  }
  return tmp;
}

//#define TEST_FX 24

#define RME(n,x) case n: x break;

static void __fxsfunc(int which, double &d, double &r)
{
	double t = 0.0;
	switch (which) 
	{
		RME(0, r+=(0.1-0.2*d)*(cos(fx_curtime)); d*=0.96;)
		RME(1, d*=0.99*(1.0-sin(r*3+fx_curtime*3)/32.0); r+=0.03*sin(d * M_PI * 4.0 +fx_curtime*0.5);)
		RME(2, d*=0.94+(cos(r*32.0)*0.06);)
		RME(3, d*=1.01+(cos(r*4.0)*0.04); r+=0.03*sin(d * M_PI * 4);)
		RME(4, r+=0.1*sin(d*M_PI*5);)
		RME(5, t=sin(d*M_PI); d-=8*t*t*t*t*t*0.01; )
		RME(6, d*=0.95+(cos(r*5.0 - M_PI/2.50)*0.03); )
		RME(7, r+=0.1*cos(fx_curtime); d*=0.96+cos(d*M_PI)*0.05; )
		RME(8, t=tan(fx_curtime*0.2);
		if (t < -20.0) t=-20.0; 
		if (t > 20.0) t=20.0; 
		r+=0.07*cos(d*M_PI)*t; 
	)
	RME(9, t=d; d=d+0.05*cos(r*ef10_sc)+getosc(r,0.2)*0.5; r=r+cos(t*3.14159*ef10_sc)*0.1; )
	RME(10, d=atan(d); )
	RME(11, d=sin(d); )
	RME(12, r=r+sin(d*3.14159*4)*0.1; d=(0.99+0.04*cos(r*32))*d; )

	RME(13,d=d-0.01*(fabs(sin(d*3.14159*8))+0.1);)
	RME(14,d=d*(1.0+0.5*getosc(d,0.4)); )
	RME(15,d=0.3; )
	RME(16,d=0.1*cos(r*4.0+fx_curtime*1.3)+d; )

	RME(17,r=cos(cos(fx_curtime)*d*M_PI*17)*0.1+r; d=d*cos(r*5.0+fx_curtime)*0.1+d; )

	RME(18,r+=sin(r*2.0+cos(fx_curtime*0.2)*8)*0.15; d=d*0.98; )

	RME(19, t=sin(r-M_PI*0.5 + fx_curtime*0.3); if (t < 0.1) t=0.1; d=d-0.3*d*t; )

	RME(20, d=0.92*d*(1.0+0.09*sin(r)); r=r+0.1*(1.0-d)*(1.0-d)*(1.0-d); )

	RME(21, d=my_asin(d*0.75)*0.95/0.75;)

	RME(22,r=r+(0.2*sin(r*8+fx_curtime)); d=d*0.99;)

	RME(23,
		r=r+sin(r*4+cos(fx_curtime)*3)*sin(d*3.14159*7+cos(fx_curtime*0.2)*7)*0.3;
		d=d*(0.97+cos(r*32.7)*0.03);
		)

	RME(24,
		r=r+cos(fx_curtime*0.3)*0.1;
		t=r;
		if (t < 0.0) t+=3.14159;
		if (t > 3.14159) t-=3.14159;
		d=atan2(d,t/2)*0.9;
		)
	}
}

#define NUM_FX 25

#define COMBINE_FX 4

static double fx_weight;
static int cur_fx[COMBINE_FX], last_fx[COMBINE_FX];
static double fx_offs[2][2];
static unsigned int fx_start;
static unsigned int fx_end,fx_end2;

static void fx_init(void)
{
	if (GetTickCount()>=fx_end)
	{
		memcpy(last_fx,cur_fx,sizeof(last_fx));
		fx_start=GetTickCount();
		fx_end2=fx_start+400+(warand()&16383);
		fx_end=fx_start + 4*(fx_end2-fx_start);
		if (fx_end > fx_start+8000) fx_end=fx_start+8000;
		if (fx_end2 > fx_end) fx_end2=fx_end;

		int x;
		for (x = 0; x < COMBINE_FX; x ++)
		{
			cur_fx[x]=warand()%((NUM_FX-x) + x*x*(NUM_FX/3));
			if (cur_fx[x] == 15) cur_fx[x]=warand()%((NUM_FX-x) + x*x*(NUM_FX/3));
		}

		for (x = 1; x < COMBINE_FX; x ++)
		{
			if (cur_fx[x] >= NUM_FX) break;
		}

		for (; x < COMBINE_FX; x ++) cur_fx[x]=NUM_FX;

	#if 0
		static char buf[1024] = {0};
		wsprintf(buf,"picked: ");
		for (x = 0; x < COMBINE_FX; x ++)
		{
			if (cur_fx[x]>=NUM_FX) break;
			wsprintf(buf+strlen(buf),"%d,",cur_fx[x]);
		}
		wsprintf(buf+strlen(buf),"\n");
		OutputDebugString(buf);
	#endif

		// for testing
	#ifdef TEST_FX
		for (x = 1; x < COMBINE_FX; x++)
		{
			cur_fx[x]=NUM_FX;
		}
		cur_fx[0]=TEST_FX;
	#endif

		for (x = 0; x < 2; x ++)
		{
			fx_offs[1][x]=fx_offs[0][x];
			if (!(warand()&7)) 
				fx_offs[0][x]=((warand()%101) - 50)/750.0;
			else fx_offs[0][x]=0;
		}

		static int ff;
		if (!ff)
		{
			last_fx[0]=warand()%NUM_FX;
			for (x = 1; x < COMBINE_FX; x ++)
			{
				last_fx[x]=NUM_FX;
			}
			ff++;
		}
	}

	fx_curtime=(double)(GetTickCount()-fx_gstarttime)/1000.0;
	fx_weight=(double)(GetTickCount()-fx_start)/(double)(fx_end2-fx_start);

	if (fx_weight > 1.0) fx_weight=1.0;
	ef10_sc=cos(fx_curtime)*2 + 3;
}

static void fx_apply(double &d, double &r) // 1 if rect
{
	double d2=d, r2=r;
	int x;
	for (x = 0; x < COMBINE_FX; x ++)
	{
		if (last_fx[x] < NUM_FX) 
		{
			__fxsfunc(last_fx[x],d2,r2);
		}
		if (cur_fx[x] < NUM_FX) 
		{
			__fxsfunc(cur_fx[x],d,r);
		}
	}

	d=d*fx_weight + d2*(1.0-fx_weight);
	r=r*fx_weight + r2*(1.0-fx_weight);
}

static int mmx_fadeval[2]={1,1};
static unsigned int const mmx_blend4_revn[2]={0xff00ff,0xff00ff};
static int const mmx_blend4_zero;

////// NEW FASTER (HOPEFULLY) - THANKS FOR THE IDEA RYAN! :)
static int mask1[2]={0x0000ffff,0};
static int mask2[2]={0xffff0000,0};
static int revy[2]={0,0xff00ff};
static int mask3[2]={0xffffffff,0};
static int mask4[2]={0,0xffffffff};
static int subma=0x000000FF;

#ifdef CLOOP
static __inline unsigned char FASTMMXBLEND(unsigned char *i, unsigned int w, int xp, int yp)
{
	__asm
	{
		movd mm1, [xp]
		mov eax, i

		psrlw mm1, 8 // mm1 = -0XP
		mov esi, w     

		movd mm3, [yp]
		punpcklwd mm1,mm1 // mm1=00XP-00XP

		psrlw mm3, 8 // mm3 = -0YP
		sub ecx, ecx

		movd mm2, [subma] // mm2=0000-00FF
		mov cl, [eax]
	    
		psubw mm2, mm1    // mm2=00??-00XI
		mov ch, [eax+1]

		punpcklwd mm3,mm3 // mm3=00YP-00YP
		pand mm1, [mask2] // mm1=00XP-0000

		punpckldq mm3, mm3 //mm3=00YP-00YP-00YP-00YP
		shl ecx, 16

		movq mm4, [revy]   // mm4=0000-0000-00FF-00FF
		pand mm2, [mask1] // mm2=0000-00XI

		psubw mm4, mm3    // mm4=00YI-00YI-00??-00??
		mov cl, [eax+esi]

		pand mm3, [mask3] // mm3=0000-0000-00YP-00YP
		por mm1, mm2      // mm1=00XP-00XI

		pand mm4, [mask4] // mm4=00YI-00YI-0000-0000
		mov ch, [eax+esi+1]

		por mm3, mm4 // mm3=00YP-00YP-00YI-00YI
		punpckldq mm1, mm1 //mm1=00XP-00XI-00XP-00XI

		pmullw mm1, mm3

		movd mm0, ecx

		punpcklbw mm0, [mmx_blend4_zero]
		psrlw mm1, 8
	    
		Pmaddwd mm0, mm1
		// empty

		// stall
	    
		// stall

		psrld mm0, 8
		// empty

		movq mm1, mm0
		// empty

		psrl mm1, 32
		// empty

		paddusb mm0, mm1
		// empty

		psubusb mm0, [mmx_fadeval]
		// empty

		movd eax, mm0
	}
}
#endif

void moveframe_init(int w, int h, int divx, int divy, int fadeval)
{
	int x = 0;
	XRES=divx+1;
	YRES=divy+1;
	if (XRES&1) XRES&=~1;
	if (YRES&1) YRES&=~1;
	if (XRES<2) XRES=2;
	if (YRES<2) YRES=2;
	if (XRES>128) XRES=128;
	if (YRES>128) YRES=128;
	fx_gstarttime=(double)GetTickCount();
	m_wmul = (int*)GlobalAlloc(GPTR,h*sizeof(int)+(XRES*YRES*2 + XRES*4 + 4)*sizeof(int));
	m_tab=m_wmul + h;
	for(x = 0; x < h; x ++)
		m_wmul[x]=x*w;
	g_w=w;
	g_h=h;
	mmx_fadeval[0]=fadeval;
	mmx_fadeval[1]=fadeval;
}

void moveframe_quit()
{
	if (m_wmul) GlobalFree(m_wmul);
	m_wmul=NULL;
}

void moveframe(unsigned char *inptr, unsigned char *outptr, unsigned char *visdata)
{
	m_visdata=visdata;
	int w=g_w;
	int h=g_h;
	int w_adj=(w-2)<<16;
	int h_adj=(h-2)<<16;

	fx_init();
	int x = 0;
	int y = 0;
	int *tabptr=m_tab;

	double xsc=2.0/w,ysc=2.0/h;
	double dw2=((double)w*32768.0);
	double dh2=((double)h*32768.0);
	double max_screen_d=sqrt((double)(w*w+h*h))*0.5;
	    
	double divmax_d=1.0/max_screen_d;

	max_screen_d *= 65536.0;
	double xo=fx_offs[0][0]*fx_weight + fx_offs[1][0]*(1.0-fx_weight);
	double yo=fx_offs[0][1]*fx_weight + fx_offs[1][1]*(1.0-fx_weight);

	int yc_pos, yc_dpos, xc_pos, xc_dpos;
	yc_pos=0;
	xc_dpos = (w<<16)/(XRES-1);
	yc_dpos = (h<<16)/(YRES-1);
	for (y = 0; y < YRES; y ++)
	{
	    xc_pos=0;
		for (x = 0; x < XRES; x ++)
		{
			double xd = 0, yd = 0;
      
			xd=((double)xc_pos-dw2)*(1.0/65536.0);
			yd=((double)yc_pos-dh2)*(1.0/65536.0);
			xc_pos+=xc_dpos;

			double var_d=sqrt(xd*xd+yd*yd)*divmax_d;
			double var_r=atan2(yd,xd) + M_PI*0.5;

			int tmp1 = 0, tmp2 = 0;
			fx_apply(var_d,var_r);
			var_d *= max_screen_d;
			var_r -= M_PI*0.5;

			tmp1=myftol(dw2*(1.0+xo) + cos(var_r) * var_d);
			tmp2=myftol(dh2*(1.0+yo) + sin(var_r) * var_d);
			if (tmp1 < 0) tmp1=0;
			if (tmp1 > w_adj) tmp1=w_adj;
			if (tmp2 < 0) tmp2=0;
			if (tmp2 > h_adj) tmp2=h_adj;
			*tabptr++ = tmp1;
			*tabptr++ = tmp2;
		}
		yc_pos+=yc_dpos;
	}
	// yay, the table is generated. now we do a fixed point 
	// interpolation of the whole thing and pray.

	int *interptab=m_tab+XRES*YRES*2;
	int *rdtab=m_tab;
	int yseek=1;
	yc_pos=0;
	xc_dpos=(w<<16)/(XRES-1);
	yc_dpos=(h<<16)/(YRES-1);
	int lypos=0;
	int yl=h;
	while (yl>0)
	{
		yc_pos+=yc_dpos;   
		yseek=(yc_pos>>16)-lypos;
		if (!yseek) goto done;
		lypos=yc_pos>>16;
		int l=XRES;
		int *stab=interptab;
		int xr3=XRES*2;
		while (l--)
		{
			int tmp1, tmp2;
			tmp1=rdtab[0];
			tmp2=rdtab[1];
			stab[0]=tmp1;
			stab[1]=tmp2;
			stab[2]=(rdtab[XRES*2]-tmp1)/yseek;
			stab[3]=(rdtab[XRES*2+1]-tmp2)/yseek;
			rdtab+=2;
			stab+=4;
		}

		if (yseek > yl) yseek=yl;
		yl-=yseek;

		if (yseek > 0) while (yseek--)
		{
			int d_x;
			int d_y;
			int seek;
			int *seektab=interptab;
			int xp,yp;
			int l=w;
			int lpos=0;
			int xc_pos=0;
			while (l>0)
			{
				xc_pos+=xc_dpos;
				seek=(xc_pos>>16)-lpos;
				if (!seek) goto done;
				lpos=xc_pos>>16;
				xp=seektab[0];
				yp=seektab[1];
				d_x=(seektab[4]-xp)/(seek);
				d_y=(seektab[5]-yp)/(seek);
				seektab[0] += seektab[2];
				seektab[1] += seektab[3];
				seektab+=4;
      
				if (seek>l) seek=l;
				l-=seek;
				if (seek>0)
				{
					// normal loop
#ifdef CLOOP
					while (seek--)
					{ 
						*outptr++=FASTMMXBLEND(inptr+(xp>>16)+m_wmul[yp>>16],w,xp,yp);
						xp+=d_x; yp+=d_y; 
					}
#else
					__asm
					{
						mov edx, seek
						mov edi, outptr
						mov esi, w

						align 16
					myLoop1:
						mov eax, m_wmul
						mov ebx, [yp]

						movd mm3, ebx
						mov ecx, [xp]

						shr ebx, 16
						movd mm1, ecx

						mov eax, [eax+ebx*4];
						shr ecx, 16

						psrlw mm1, 8 // mm1 = -0XP
						add eax, ecx

						punpcklwd mm1,mm1 // mm1=00XP-00XP
						add eax, [inptr]

						psrlw mm3, 8 // mm3 = -0YP
						movd mm2, [subma] // mm2=0000-00FF
                
						psubw mm2, mm1    // mm2=00??-00XI
						mov cx, [eax]

			            punpcklwd mm3,mm3 // mm3=00YP-00YP
			            pand mm1, [mask2] // mm1=00XP-0000

			            punpckldq mm3, mm3 //mm3=00YP-00YP-00YP-00YP
			            shl ecx, 16

			            movq mm4, [revy]   // mm4=0000-0000-00FF-00FF
			            pand mm2, [mask1] // mm2=0000-00XI

						mov cx, [eax+esi]

			            por mm1, mm2      // mm1=00XP-00XI
						psubw mm4, mm3    // mm4=00YI-00YI-00??-00??

						pand mm3, [mask3] // mm3=0000-0000-00YP-00YP
						pand mm4, [mask4] // mm4=00YI-00YI-0000-0000

						por mm3, mm4 // mm3=00YP-00YP-00YI-00YI
						punpckldq mm1, mm1 //mm1=00XP-00XI-00XP-00XI

						pmullw mm1, mm3

			            movd mm0, ecx
						// empty

			            // stall

						punpcklbw mm0, [mmx_blend4_zero]
						psrlw mm1, 8
    
						Pmaddwd mm0, mm1
						mov eax, [xp]

			            add eax, [d_x]
			            mov ebx, [yp]
    
						mov [xp], eax
						add ebx, [d_y]

			            psrld mm0, 8
						mov [yp], ebx

			            movq mm1, mm0
						// empty

			            psrl mm1, 32
						// empty

						paddusb mm0, mm1
						// empty

						psubusb mm0, [mmx_fadeval]
						// empty

						movd ecx, mm0

						mov [edi], cl

						inc edi
						dec edx
						jnz myLoop1
						mov outptr, edi
					}
#endif
				}
			}
			// adjust final (rightmost elem) part of seektab
			seektab[0] += seektab[2];
			seektab[1] += seektab[3];
		}
	}
done:
	__asm emms;
}