#include "Decoder.h"

#define sign(a)         ((a) < 0 ? -1 : 1)

/* private prototypes */
static void recon_comp (unsigned char *src, unsigned char *dst,
  int lx, int lx2, int w, int h, int x, int y, int dx, int dy);

static void rec(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
static void recc(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
static void rech(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
static void rechc(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
static void h263_recv(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
static void recvc(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
static void rec4(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
static void rec4c(unsigned char *s, unsigned char *d, int lx, int lx2, int h);

static int roundtab[16]=  {0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,2};

void Decoder::reconstruct(int bx, int by, int mode)
{
		int w, h, lx, lx2, dx, dy, xp, yp, comp, sum;
	int x, y;
	unsigned char *src[3];

	x = bx / 16 + 1;
	y = by / 16 + 1;
	lx = coded_picture_width;

		lx2 = coded_picture_width + 64;
		src[0] = edgeframe[0];
		src[1] = edgeframe[1];
		src[2] = edgeframe[2];

		/* P prediction */
		w = 8;
		h = 8;
		/* Y */
		/* 4 MV can be used without OBMC in        * deblocking filter mode                  */

		if (mode == MODE_INTER4V || mode == MODE_INTER4V_Q)
		{
			for (comp = 0; comp < 4; comp++)
			{
				dx = MV[0][comp + 1][y][x];
				dy = MV[1][comp + 1][y][x];

				xp = bx + ((comp & 1) << 3);
				yp = by + ((comp & 2) << 2);
				recon_comp(src[0], newframe[0], lx, lx2, w, h, xp, yp, dx, dy);
			}
		}
		else
		{
			dx = MV[0][0][y][x];
			dy = MV[1][0][y][x];
			recon_comp(src[0], newframe[0], lx, lx2, w << 1, h << 1, bx, by, dx, dy);
			// TODO: MC16x16
		}

		/* Chroma */
		if (mode == MODE_INTER4V || mode == MODE_INTER4V_Q)
		{
			sum = MV[0][1][y][x] + MV[0][2][y][x] + MV[0][3][y][x] + MV[0][4][y][x];
			dx = sign(sum) * (roundtab[abs(sum) % 16] + (abs(sum) / 16) * 2);

			sum = MV[1][1][y][x] + MV[1][2][y][x] + MV[1][3][y][x] + MV[1][4][y][x];
			dy = sign(sum) * (roundtab[abs(sum) % 16] + (abs(sum) / 16) * 2);

		}
		else
		{
			dx = MV[0][0][y][x];
			dy = MV[1][0][y][x];
			/* chroma rounding */
			dx = (dx % 4 == 0 ? dx >> 1 : (dx >> 1) | 1);
			dy = (dy % 4 == 0 ? dy >> 1 : (dy >> 1) | 1);
		}
		lx >>= 1;
		bx >>= 1;
		lx2 >>= 1;
		by >>= 1;
		/* Chroma */
		recon_comp(src[1], newframe[1], lx, lx2, w, h, bx, by, dx, dy);
		recon_comp(src[2], newframe[2], lx, lx2, w, h, bx, by, dx, dy);
}

static void recon_comp(unsigned char *src,unsigned char *dst,int lx,int lx2,int w,int h,int x,int y,int dx,int dy)
{
  int xint, xh, yint, yh;
  unsigned char *s, *d;

  xint = dx>>1;
  xh = dx & 1;
  yint = dy>>1;
  yh = dy & 1;

  /* origins */
  s = src + lx2*(y+yint) + x + xint;
  d = dst + lx*y + x;

  if (!xh && !yh)
    if (w!=8)
      rec(s,d,lx,lx2,h);
    else
      recc(s,d,lx,lx2,h);
  else if (!xh && yh)
    if (w!=8)
      h263_recv(s,d,lx,lx2,h);
    else 
      recvc(s,d,lx,lx2,h);
  else if (xh && !yh)
    if (w!=8)
      rech(s,d,lx,lx2,h);
    else
      rechc(s,d,lx,lx2,h);
  else /* if (xh && yh) */
    if (w!=8)
      rec4(s,d,lx,lx2,h);
    else
      rec4c(s,d,lx,lx2,h);
}

static void rec(unsigned char *s,unsigned char *d,int lx,int lx2,int h)
{
  int j;

  for (j=0; j<h; j++)
  {
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
    d[4] = s[4];
    d[5] = s[5];
    d[6] = s[6];
    d[7] = s[7];
    d[8] = s[8];
    d[9] = s[9];
    d[10] = s[10];
    d[11] = s[11];
    d[12] = s[12];
    d[13] = s[13];
    d[14] = s[14];
    d[15] = s[15];
    s+= lx2;
    d+= lx;
  }
}

static void recc(unsigned char *s,unsigned char *d,int lx,int lx2,int h)
{
  int j;

  for (j=0; j<h; j++)
  {
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
    d[4] = s[4];
    d[5] = s[5];
    d[6] = s[6];
    d[7] = s[7];
    s+= lx2;
    d+= lx;
  }
}

static void rech(unsigned char *s,unsigned char *d,int lx,int lx2,int h)
{
  unsigned char *dp,*sp;
  int j;
  unsigned int s1,s2;

  sp = s;
  dp = d;
  for (j=0; j<h; j++)
  {
    s1=sp[0];
    dp[0] = (unsigned int)(s1+(s2=sp[1])+1)>>1;
    dp[1] = (unsigned int)(s2+(s1=sp[2])+1)>>1;
    dp[2] = (unsigned int)(s1+(s2=sp[3])+1)>>1;
    dp[3] = (unsigned int)(s2+(s1=sp[4])+1)>>1;
    dp[4] = (unsigned int)(s1+(s2=sp[5])+1)>>1;
    dp[5] = (unsigned int)(s2+(s1=sp[6])+1)>>1;
    dp[6] = (unsigned int)(s1+(s2=sp[7])+1)>>1;
    dp[7] = (unsigned int)(s2+(s1=sp[8])+1)>>1;
    dp[8] = (unsigned int)(s1+(s2=sp[9])+1)>>1;
    dp[9] = (unsigned int)(s2+(s1=sp[10])+1)>>1;
    dp[10] = (unsigned int)(s1+(s2=sp[11])+1)>>1;
    dp[11] = (unsigned int)(s2+(s1=sp[12])+1)>>1;
    dp[12] = (unsigned int)(s1+(s2=sp[13])+1)>>1;
    dp[13] = (unsigned int)(s2+(s1=sp[14])+1)>>1;
    dp[14] = (unsigned int)(s1+(s2=sp[15])+1)>>1;
    dp[15] = (unsigned int)(s2+sp[16]+1)>>1;
    sp+= lx2;
    dp+= lx;
  }
}

static void rechc(unsigned char *s,unsigned char *d,int lx,int lx2,int h)
{
  unsigned char *dp,*sp;
  int j;
  unsigned int s1,s2;

  sp = s;
  dp = d;
  for (j=0; j<h; j++)
  {
    s1=sp[0];
    dp[0] = (unsigned int)(s1+(s2=sp[1])+1)>>1;
    dp[1] = (unsigned int)(s2+(s1=sp[2])+1)>>1;
    dp[2] = (unsigned int)(s1+(s2=sp[3])+1)>>1;
    dp[3] = (unsigned int)(s2+(s1=sp[4])+1)>>1;
    dp[4] = (unsigned int)(s1+(s2=sp[5])+1)>>1;
    dp[5] = (unsigned int)(s2+(s1=sp[6])+1)>>1;
    dp[6] = (unsigned int)(s1+(s2=sp[7])+1)>>1;
    dp[7] = (unsigned int)(s2+sp[8]+1)>>1;
    sp+= lx2;
    dp+= lx;
  }
}


static void h263_recv(unsigned char *s,unsigned char *d,int lx,int lx2,int h)
{
  unsigned char *dp,*sp,*sp2;
  int j;

  sp = s;
  sp2 = s+lx2;
  dp = d;
  for (j=0; j<h; j++)
  {
    dp[0] = (unsigned int)(sp[0]+sp2[0]+1)>>1;
    dp[1] = (unsigned int)(sp[1]+sp2[1]+1)>>1;
    dp[2] = (unsigned int)(sp[2]+sp2[2]+1)>>1;
    dp[3] = (unsigned int)(sp[3]+sp2[3]+1)>>1;
    dp[4] = (unsigned int)(sp[4]+sp2[4]+1)>>1;
    dp[5] = (unsigned int)(sp[5]+sp2[5]+1)>>1;
    dp[6] = (unsigned int)(sp[6]+sp2[6]+1)>>1;
    dp[7] = (unsigned int)(sp[7]+sp2[7]+1)>>1;
    dp[8] = (unsigned int)(sp[8]+sp2[8]+1)>>1;
    dp[9] = (unsigned int)(sp[9]+sp2[9]+1)>>1;
    dp[10] = (unsigned int)(sp[10]+sp2[10]+1)>>1;
    dp[11] = (unsigned int)(sp[11]+sp2[11]+1)>>1;
    dp[12] = (unsigned int)(sp[12]+sp2[12]+1)>>1;
    dp[13] = (unsigned int)(sp[13]+sp2[13]+1)>>1;
    dp[14] = (unsigned int)(sp[14]+sp2[14]+1)>>1;
    dp[15] = (unsigned int)(sp[15]+sp2[15]+1)>>1;
    sp+= lx2;
    sp2+= lx2;
    dp+= lx;
  }
}

static void recvc(unsigned char *s,unsigned char *d,int lx,int lx2,int h)
{
  unsigned char *dp,*sp,*sp2;
  int j;

  sp = s;
  sp2 = s+lx2;
  dp = d;

  for (j=0; j<h; j++)
  {
    dp[0] = (unsigned int)(sp[0]+sp2[0]+1)>>1;
    dp[1] = (unsigned int)(sp[1]+sp2[1]+1)>>1;
    dp[2] = (unsigned int)(sp[2]+sp2[2]+1)>>1;
    dp[3] = (unsigned int)(sp[3]+sp2[3]+1)>>1;
    dp[4] = (unsigned int)(sp[4]+sp2[4]+1)>>1;
    dp[5] = (unsigned int)(sp[5]+sp2[5]+1)>>1;
    dp[6] = (unsigned int)(sp[6]+sp2[6]+1)>>1;
    dp[7] = (unsigned int)(sp[7]+sp2[7]+1)>>1;
    sp+= lx2;
    sp2+= lx2;
    dp+= lx;
  }
}

static void rec4(unsigned char *s,unsigned char *d,int lx,int lx2,int h)
{
  unsigned char *dp,*sp,*sp2;
  int j;
  unsigned int s1,s2,s3,s4;

  sp = s;
  sp2 = s+lx2;
  dp = d;
  for (j=0; j<h; j++)
  {
    s1=sp[0]; s3=sp2[0];
    dp[0] = (unsigned int)(s1+(s2=sp[1])+s3+(s4=sp2[1])+2)>>2;
    dp[1] = (unsigned int)(s2+(s1=sp[2])+s4+(s3=sp2[2])+2)>>2;
    dp[2] = (unsigned int)(s1+(s2=sp[3])+s3+(s4=sp2[3])+2)>>2;
    dp[3] = (unsigned int)(s2+(s1=sp[4])+s4+(s3=sp2[4])+2)>>2;
    dp[4] = (unsigned int)(s1+(s2=sp[5])+s3+(s4=sp2[5])+2)>>2;
    dp[5] = (unsigned int)(s2+(s1=sp[6])+s4+(s3=sp2[6])+2)>>2;
    dp[6] = (unsigned int)(s1+(s2=sp[7])+s3+(s4=sp2[7])+2)>>2;
    dp[7] = (unsigned int)(s2+(s1=sp[8])+s4+(s3=sp2[8])+2)>>2;
    dp[8] = (unsigned int)(s1+(s2=sp[9])+s3+(s4=sp2[9])+2)>>2;
    dp[9] = (unsigned int)(s2+(s1=sp[10])+s4+(s3=sp2[10])+2)>>2;
    dp[10] = (unsigned int)(s1+(s2=sp[11])+s3+(s4=sp2[11])+2)>>2;
    dp[11] = (unsigned int)(s2+(s1=sp[12])+s4+(s3=sp2[12])+2)>>2;
    dp[12] = (unsigned int)(s1+(s2=sp[13])+s3+(s4=sp2[13])+2)>>2;
    dp[13] = (unsigned int)(s2+(s1=sp[14])+s4+(s3=sp2[14])+2)>>2;
    dp[14] = (unsigned int)(s1+(s2=sp[15])+s3+(s4=sp2[15])+2)>>2;
    dp[15] = (unsigned int)(s2+sp[16]+s4+sp2[16]+2)>>2;
    sp+= lx2;
    sp2+= lx2;
    dp+= lx;
  }
}

static void rec4c(unsigned char *s,unsigned char *d,int lx,int lx2,int h)
{
  unsigned char *dp,*sp,*sp2;
  int j;
  unsigned int s1,s2,s3,s4;

  sp = s;
  sp2 = s+lx2;
  dp = d;
  for (j=0; j<h; j++)
  {
    s1=sp[0]; s3=sp2[0];
    dp[0] = (unsigned int)(s1+(s2=sp[1])+s3+(s4=sp2[1])+2)>>2;
    dp[1] = (unsigned int)(s2+(s1=sp[2])+s4+(s3=sp2[2])+2)>>2;
    dp[2] = (unsigned int)(s1+(s2=sp[3])+s3+(s4=sp2[3])+2)>>2;
    dp[3] = (unsigned int)(s2+(s1=sp[4])+s4+(s3=sp2[4])+2)>>2;
    dp[4] = (unsigned int)(s1+(s2=sp[5])+s3+(s4=sp2[5])+2)>>2;
    dp[5] = (unsigned int)(s2+(s1=sp[6])+s4+(s3=sp2[6])+2)>>2;
    dp[6] = (unsigned int)(s1+(s2=sp[7])+s3+(s4=sp2[7])+2)>>2;
    dp[7] = (unsigned int)(s2+sp[8]+s4+sp2[8]+2)>>2;
    sp+= lx2;
    sp2+= lx2;
    dp+= lx;
  }
}

