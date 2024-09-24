#include <windows.h>
#include <math.h>

extern int (*warand)(void);
static double rsc[3], rv[3];

static double drand(void)
{
	return (warand()%4096)/(double)4096;
}

static void startgen(void)
{
	for (int x = 0; x < 3; x ++)
	{
		rsc[x]=drand()*256.0+256.0;
		rv[x]=drand()*0.25+0.25;
	}

	if ((warand()&0xf)==5) rv[0]+=drand()*4.0;
	if ((warand()&0xf)==2) rv[1]+=drand()*4.0;
	if ((warand()&0xf)==3) rv[2]+=drand()*4.0;
}

static int getv(int i, int w)
{
	double d=sin((double)i*3.14159/256.0*rv[w])*rsc[w];
	int v;
	__asm
	{
		fld d
		fistp v
	}
	if (v<0)v=0;
	if (v>0xff)v=0xff;
	return v;
}

static int lreversed;

unsigned char *getnewpalette()
{
	static unsigned char thispal[256][3];
	int x;
	startgen();

	for (x = 0; x < 768; x ++)
	{
		thispal[x/3][x%3]=getv(x/3,x%3);
	}

	int lr=warand()%21;

	if (lreversed) 
	{
		if (warand()&1) lreversed=0;
		lr=4;
	}
	else if (lr == 4)
	{
		lreversed=1;
	}

	switch (lr)
	{
		case 3:
			for (x = 0; x < 128; x ++)
			{
				thispal[x][0]=thispal[x*2][0];
				thispal[x][1]=thispal[x*2][1];
				thispal[x][2]=thispal[x*2][2];
			}
			for (; x < 256; x ++)
			{
				thispal[x][0]=thispal[255-x][0];
				thispal[x][1]=thispal[255-x][1];
				thispal[x][2]=thispal[255-x][2];
			}
		break;
		case 4:
			for (x = 0; x < 128; x ++)
			{
				int q;
				for (q = 0; q < 3; q ++)
				{
					unsigned char t;
					t=thispal[x][q];
					thispal[x][q]=thispal[255-x][q];
					thispal[255-x][q]=t;
				}
			}
		break;
	}

	return &thispal[0][0];
}