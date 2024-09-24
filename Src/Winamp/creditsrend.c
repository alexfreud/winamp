#include "main.h"

#define LAND_SIZE 500
#define LAND_DIV 12

extern void plTextPutStrW(pl_Cam* cam, pl_sInt x, pl_sInt y, pl_Float z, pl_uChar color, const wchar_t* string);
static void setup_materials(pl_Mat **mat, unsigned char *pal);
static pl_Obj *setup_landscape(float size, int div, pl_Mat *m);
static pl_Mat *mat[7];
static pl_Cam *cam;
static pl_Obj *land,*object[8], *lightobject,*lightobject2, *billboard, *teamobject;
static pl_Spline spline;
static pl_Float splineTime;
static pl_Light *light;
static unsigned int prevtime;
static pl_Texture *grndtex, *watex, *walogotex, *teamtex;
static char *teamtexbase;
static int displaycredits=1;
static float keys1[8*5];

static void initspline(int i)
{
	spline.keys[i*5+0] = (pl_Float)(warand()%320)-160;
	spline.keys[i*5+1] = (pl_Float)(warand()%110)+15;
	spline.keys[i*5+2] = (pl_Float)(warand()%320)-160;
	spline.keys[i*5+3] = (pl_Float)(warand()%32)/32.0f;
	spline.keys[i*5+4] = (pl_Float)(warand()%32)/32.0f;
}

static unsigned int m_creditspos;
static int m_creditspos_frames;
static int m_lastpos;
static int rpoo=1;

void render_togglecredits()
{
	displaycredits=!displaycredits;
}

void render_init(int w, int h, char *pal)
{
	int i;
	float wd;
	int ishigh=!!(GetAsyncKeyState(VK_SHIFT)&GetAsyncKeyState(VK_MENU)&0x8000);
	rpoo=1;
	splineTime=0.0;
	spline.tens = (pl_Float)-0.6;
	spline.keyWidth = 5;
	spline.numKeys = 8;

	spline.keys = keys1;
	for (i = 0; i < spline.numKeys; i ++) 
	{
		initspline(i);
	}

	cam = plCamCreate(w,h,1.0,120.0,NULL,NULL);
	cam->ScreenWidth=w;
	cam->ScreenHeight=h;
	cam->ClipTop = 1;
	cam->ClipLeft = 1;
	cam->ClipBottom = h-1;
	cam->ClipRight = w-1;
	cam->AspectRatio=1.0;
	cam->CenterX=w/2;
	cam->CenterY=h/2;

	cam->Sort = 1;
	cam->Z = -60;

	light = plLightCreate();

	setup_materials(mat,(unsigned char*)pal);
	land = setup_landscape(LAND_SIZE,ishigh?LAND_DIV*2:LAND_DIV,mat[0]);

	teamobject = plMakeBox(32,32,32,mat[5]);
	teamobject->Yp=75;
	teamobject->Xp=-LAND_SIZE/3;
	teamobject->Za=-90;
	teamobject->Ya=90;
	teamobject->BackfaceCull=0;

	billboard = plMakePlane(LAND_SIZE/3,90,1,mat[4]);
	billboard->Yp=75;
	billboard->Xp=LAND_SIZE/3;
	billboard->Za=-90;
	billboard->Ya=90;
	billboard->BackfaceCull=0;

	wd=9.8f;
	for (i = 0; i < sizeof(object)/sizeof(object[0]); i++)
	{
		object[i]=plObjCreate(0,0);
		object[i]->Xp=0;
		object[i]->Yp=90;
		object[i]->Zp=0;
		object[i]->Children[0]=plMakeTorus(wd,wd+4.0f,ishigh?(64+i*4):(12+i*2),ishigh?12:6,mat[1+(i&1)]);
		wd+=5.8f;
	}
	lightobject=plMakeSphere(17.0,ishigh?16:6,ishigh?24:8,mat[3]);
	lightobject2=plMakeSphere(17.0,ishigh?16:6,ishigh?24:8,mat[3]);
	m_creditspos=GetTickCount64();
	m_creditspos_frames=0;
	m_lastpos=-1;
	prevtime=GetTickCount64();
}

// 128 frames:
// 0-15, silence
// 16-31, fadein
// 32-111, display
// 112-127, fadeout

void render_quit(void)
{
	int x;

	m_lastpos=-1;
	m_creditspos=0;
	m_creditspos_frames=0;
	plObjDelete(land); land=0;
	plObjDelete(billboard); billboard=0;
	plObjDelete(teamobject); teamobject=0;
  
	for (x = 0; x < sizeof(object)/sizeof(object[0]); x ++)
	{
		plObjDelete(object[x]); object[x]=0;
	}
	plObjDelete(lightobject);
	plObjDelete(lightobject2);
	for (x = 0; x < sizeof(mat)/sizeof(mat[0])-1; x ++)
	{
		plMatDelete(mat[x]); mat[x]=0;
	}
	plCamDelete(cam); cam=0;
	plLightDelete(light); light=0;
	plTexDelete(grndtex); grndtex=0;
	plTexDelete(watex); watex = 0;
	if (teamtex && teamtexbase) teamtex->Data = teamtexbase; teamtexbase=0;
	plTexDelete(teamtex); teamtex=0;
	plTexDelete(walogotex); walogotex=0;
}

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
static wchar_t *creditslist[]=
{
	L"Winamp v" WIDEN(APP_VERSION) L"\n"
	L"    The Credits"
	,
	L"Winamp v" WIDEN(APP_VERSION) L" Development:\n"
	L"     Quentin Hebette\n"
	L"     Thierry Honore\n"
	L"     Lionel Peeters\n"
	L"     Hakan Danisik\n"
	L"     Eddy Richman\n"
	L"     Jef Mauguit\n"
//	L"     Mher Didaryan\n"
//	L"     Siarhei Herasiuta\n"
//	L"     Ben Allison\n"
	,
	L"QA, Engineering & Support:\n"
	L"    DJ Egg\n"
	,
	L"Freeform Skin Engine Updates:\n"
	L"    Linus Brolin"
	,
	L"Bento Skin:\n"
	L"    Martin Pohlmann\n"
	L"    Taber Buhl\n"
	L"    Ben Allison\n"
	L"    Victor Brocaz\n"
	,
	L"Language Packs:\n"
//	L"    Dutch: Paul van Garderen\n"
	L"    French: Julien Victor, Benoit Hervier\n"
	L"    German: Christoph Grether\n"
//	L"    Italian: Flocksoft, Riccardo Vianello\n"
	L"    Polish: Pawel Porwisz"
	,
	L"Language Packs:\n"
	L"    Spanish: Manuel Fernando Gutierrez, Joel Almeida,\n"
	L"                Darwin Toledo Caceres aka Niwrad\n"
//	L"    Swedish: Kenneth Chen, Amir Tehrani,\n"
//	L"                Björn-Ole Antonsen\n"
	L"    Russian: Nureev Aleksandr, Eduard Galkin\n"
	L"    Turkish: Ali Sarioglu"
	,
	L"Language Packs:\n"
	L"    Portuguese (Brazil): Anderson Silva\n"
//	L"    Romanian: Catalin, Sebastian Alexandru\n"
	L"    Japanese: Toshiya Matsuo\n"
	L"    Hungarian: Laszlo Gardonyi\n"
//	L"    Indonesian: Antony Kurniawan"
	,
	L"Winamp Hall-of-Fame:\n"
	L"    Justin Frankel\n"
	L"    Christophe Thibault\n"
	L"    Francis Gastellu\n"
	L"    Brennan Underwood\n"
	L"    Peter Pawlowski\n"
	L"    Tom Pepper\n"
	L"    Ryan Geiss\n"
	L"    Will Fisher\n"
	L"    Maksim Tyrtyshny\n"
	L"    Darren Owen\n"
	L"    Ben Allison"
	,
	L"Installer packaged with NSIS:\n"
	L"    http://nsis.sourceforge.net/\n"
	L"    thanklessly maintained by\n"
	L"    Amir Szekely\n"
	L"    Anders Kjersem\n"
	L"Unicode NSIS port\n"
	L"    by Jim Park\n"
	L"    http://www.scratchpaper.com"
	,
	L"Modern Skin:\n"
	L"    Sven Kistner\n"
	L"    http://www.metrix.de"
	,
	/*L"Online Help: Jatin Billimoria\n"
	L"    with updates from DJ Egg"
	,*/
	L"PCM EQ magic:\n"
	L"    4Front Technologies/George Yohng\n"
	L"    http://www.yohng.com/\n"
	L"\n"
	L"EQ presets: Lars Holmberg"
	/*
	,
	L"MikMod plug-in:\n"
	L"    Jake Stine\n"
	L" ...testing:\n"
	L"    Mathew Valente"
	*/
	,
	L"Intro sound: JJ McKay"
	,
	/*
	L"Nullsoft Alumni (aka fun-haters):\n"
	L"  Rob Lord, Ian Rogers, Ryan Melcher\n"
	L"  Patrick Goddard, Jason Crawford\n"
	L"  Daniel Ruben, Kyle Yamamoto\n"
	L"  Susan Becker, Bill Thompson\n"
	L"  Josh Gerrish, Steven Blumenfeld\n"
	L"  Bonnie Burton, Rolf Hanson"
	,
	L"Nullsoft Alumni (continued):\n"
	L"  Chris Amen, Keith Peters\n"
	L"  Konstantin Martynenko\n"
	L"  Justin Frankel, Christophe Thibault\n"
	L"  Steve Gedikian, David Biderman\n" 
	L"  Tom Pepper, Ghislain 'Aus' Lacroix\n"
	L"  Jonathan Ward, Michael 'Mig' Gerard\n"
	L"   ...and DENNY!"
	,
	L"Nullsoft Alumni (continued):\n"
	L"  Lloyd Given, Scott Brown\n"
	L"  Ben Sutherland, Wen Huang\n"
	L"  Ben Pontius, Brenda Chung\n"
	L"  Lauren Axelrod, Shaun Montgomery\n"
	L"  Chris Edwards, Matt Callaway\n"
	L"  Stephen 'Tag' Loomis, Jason Herskowitz"
	,
	L"Nullsoft Alumni (continued):\n"
	L"  Ben London, Ying Chen, Rob Gould, Marcian Lytwyn\n" 
	L"  Shawn Lavelle, Alex Petty, Chad Tempest\n"
	L"  Vanaja Nataraj, Ashok Bania, Venkatraman L\n"
	L"  Smita Roul, Shiva Virupaksha, Gautam Dayanidhi\n"
	L"  John Niranjan, Krishna Chaitanya, Chitra A, Rakesh G A\n"
	L"  Sumit Kumar, Vinay Sharma, Sudhindra Aithal\n"
	L"  Venkatesh Arya, Manoj Chourasia, Basavana Gowda\n"
	L"  Niharika Patro, Prasanna Revan, Mohan Balaji, Sasikumar R"
	,
	L"Nullsoft Alumni (continued):\n"
	L"  Geno Yoham, Tarik Dahir, Tejas Mistry, Billy White, Bill Hicks\n"
	L"  Jonathan Chester, Gergo Spolarics, Maksim Tyrtyshny\n"
	L"  Taber Buhl, Shashikiran Reddy, Ryan Flynn, James Cready\n"
	*/
	L"Credits rendered with Plush:\n"
	L"    http://www.cockos.com/wdl/\n"
	L"    (8bpp foreva)"
	,
	L"Thanks:\n"
	L"    NS Beta Team & Craig Freer\n"
	L"    Our lowly forum moderators\n"
	L"    Our precious skin reviewers\n"
	L"    EFnet #mpeg3\n"
	L"    4Front Technologies"
	,
	/*
	L"So long and ...\n"
	L"    thanks for all the fish"
	,
	L"We really whipped the llama's ass!"
	,
	L"Much <3 to all who have helped\n"
	L"both directly and indirectly with\n"
	L"Winamp over the many years!"
	,
	L"At the going down of the sun\n"
	L"and in the morning ...\n\n"
	L"we will remember Winamp <3"
	,
	*/
	L" Copyright © 1997-2023\n"
	L"        Winamp SA\n"
	L"     www.winamp.com"
	,
	L""
	,
	L"(you can double left click to toggle the credits\n"
	L"           for your viewing pleasure)"
	,
	L"(you can also double right click to go into\n"
	L"          crappy fullscreen mode)"
	,
	L""
};

static int creditslist_l=(sizeof(creditslist)/sizeof(creditslist[0]))*128;
static unsigned int start_time;

static int text_hW(const wchar_t *str)
{
	int nc=1;
	while (str && *str)
	{
		if (*str==L'\n') nc++;
		str++;
	}
	return ScaleY(nc*17);
}

static int text_hA(const char* str)
{
	int nc = 1;
	while (str && *str)
	{
		if (*str == '\n') nc++;
		str++;
	}
	return ScaleY(nc * 17);
}

static int text_w(const wchar_t *str)
{
	int maxc=0,nc=0;
	while (str && *str)
	{
		if (*str==L'\n') nc=0;
		else nc++;
		str++;
		if (nc > maxc) maxc=nc;
	}
	return ScaleX(maxc*9);
}

#ifndef STATICBALLTEXTURE
#define FIRE_BITMAP_W 64
#define FIRE_BITMAP_H 64
void makeBallTexture(char *tx)
{
	int y;
	unsigned char *p=(unsigned char *)tx;
	int x;
	unsigned char *t=p + FIRE_BITMAP_W*FIRE_BITMAP_H;
	for (x = 0; x < FIRE_BITMAP_W; x ++)
	{
		int a=*t - 10;
		if ((warand()&0x7) == 7) a+=130;
		if (a < 0) a=0;
		else if (a > 150) a=150;
		*t++=a;//warand()&0xf0;
	}
	for (y = 0; y < FIRE_BITMAP_H; y ++)
	{
		*p++=p[0]/4 + p[FIRE_BITMAP_W]/2 + p[FIRE_BITMAP_W+1]/4;

		for (x = 1; x < FIRE_BITMAP_W-1; x ++)
			*p++=p[0]/4 + p[FIRE_BITMAP_W]/4 + p[FIRE_BITMAP_W-1]/4 + p[FIRE_BITMAP_W+1]/4;

		*p++=p[0]/4 + p[FIRE_BITMAP_W]/2 + p[FIRE_BITMAP_W-1]/4;
	}
}

void makeBallTextPal(char *pal)
{
	unsigned char *t=(unsigned char *)pal;
	int x=255;
	t[0]=t[1]=t[2]=0;
	t+=3;
	while (x)
	{
		if (x > 128)
		{
			int a=256-x;
			a*=3;
			if (a>255)a=255;
			t[2]=0;
			t[1]=a/2;
			t[0]=a;
		}
		else
		{
			t[2]=256-x*2;
			t[1]=255/3 + ((256-x)*2)/3;
			t[0]=255;
		}

		t+=3; 
		x--;
	}
}

#endif

void render_render(unsigned char *framebuffer, HDC hdc)
{
	static float light_sc=0.2f+2*0.3f;
	pl_Float curpos[5];
	int i;
	cam->frameBuffer=framebuffer;

	{
		unsigned int now = GetTickCount64();
		unsigned int t = now - prevtime;
		if (t < 0) t=0;
		splineTime += (pl_Float)(t*(0.01/33.0));
		prevtime=now;
	}
  
	if (splineTime > spline.numKeys) 
	{
		for (i = 2; i < spline.numKeys-2; i ++) 
		{
			initspline(i);
		}
		rpoo=0;
		splineTime -= spline.numKeys;
	}
	if (!rpoo&&splineTime>3.0)
	{
		rpoo=1;
		initspline(0);
		initspline(1);
		initspline(spline.numKeys-2);
		initspline(spline.numKeys-1);
	}
	plSplineGetPoint(&spline,splineTime,curpos);

	{
		for (i = sizeof(object)/sizeof(object[0])-1; i > 0; i --)
		{
			object[i]->Xa=object[i-1]->Xa;
			object[i]->Ya=object[i-1]->Ya;
			object[i]->Za=object[i-1]->Za;
		}
		object[0]->Xa+=5*curpos[3];
		object[0]->Ya+=5*curpos[4];
		object[0]->Za-=5*(curpos[4]*curpos[3]);
	}

	cam->X = curpos[0];
	cam->Y = curpos[1];
	cam->Z = curpos[2];

	teamobject->Xp=(pl_Float)(100.0*sin((splineTime-0.1)*3.14159));
	teamobject->Yp=(pl_Float)(75+40.0*cos((splineTime-0.1)*3.14159));
	teamobject->Zp=(pl_Float)(100.0*cos((splineTime-0.1)*3.14159*1.5));
	teamobject->Ya+=1.1f;

	lightobject->Xp=(pl_Float)(100.0*sin(splineTime*3.14159));
	lightobject->Yp=(pl_Float)(75+40.0*cos(splineTime*3.14159));
	lightobject->Zp=(pl_Float)(100.0*cos(splineTime*3.14159*1.5));
	lightobject->Ya+=1.1f;
	plLightSet(light,PL_LIGHT_POINT,lightobject->Xp,lightobject->Yp,lightobject->Zp,0.7f-0.2f/2+light_sc/2,LAND_SIZE/2);//(warand()%LAND_SIZE)-LAND_SIZE/2,12,(warand()%LAND_SIZE)-LAND_SIZE/2,1.0,LAND_SIZE/4);

 	plCamSetTarget(cam,lightobject->Xp/4,lightobject->Yp/4.0f+90*0.75f,lightobject->Zp/4);
	cam->ClipBack = 1500.0;

#ifndef STATICBALLTEXTURE
	if (watex && watex->Data) { makeBallTexture(watex->Data); makeBallTexture(watex->Data);}
#endif
	plRenderBegin(cam);
	plRenderLight(light);
 	plRenderObj(land);
	plRenderEnd();
	plRenderBegin(cam);
	plRenderLight(light);

	billboard->Za+=1.0;
	billboard->Ya+=1.0;

	{
		light_sc=0.2f+2*0.3f;
		plRenderObj(lightobject);
	}

	if (sa_curmode && playing)
	{
		char sadata[75*2+8] = {0};
		unsigned char *data=(unsigned char *)sa_get(in_getouttime(),sa_curmode, sadata);
		if (data) 
		{
			if (sa_curmode == 2) data+=75;
			for (i = 0; i < sizeof(object)/sizeof(object[0]); i ++)
			{
				int t=data[(7-i)*3];
				int t2=data[(7-i)*3+1];
				int t3=data[(7-i)*3+2];
				float val;
				if (t2 > t) t=t2;
				if (t3 > t) t=t3;
				if (sa_curmode==2) { t-=128; t/=4;}
				val=t*1.3f;
				if (object[i]->Children[0]->Yp < val) object[i]->Children[0]->Yp=val;
				else object[i]->Children[0]->Yp=object[i]->Children[0]->Yp*0.9f;
				if (i)
				{
					object[i]->Xa=object[0]->Xa;
					object[i]->Ya=object[0]->Ya;
					object[i]->Za=object[0]->Za;
				}
			}
		}
	}
	else
	{
		for (i = 0; i < sizeof(object)/sizeof(object[0]); i ++)
		{
			object[i]->Children[0]->Yp=object[i]->Children[0]->Yp*0.9f;
		}
	}
	for (i = 0; i < sizeof(object)/sizeof(object[0]); i ++) plRenderObj(object[i]);

 	plRenderObj(billboard);
	plRenderObj(teamobject);

	plRenderEnd();

	if (displaycredits)
	{
		int alpha=0;
		int creditspos=((GetTickCount64()-m_creditspos)/30)%creditslist_l;
		int pos=creditspos&127;
		const wchar_t *str=creditslist[creditspos/128];
		static int g_ypos;
		static int g_xpos;
		if (creditspos/128 != m_lastpos)
	    {
			m_lastpos=creditspos/128;
			if (creditspos<128) 
			{
				m_creditspos_frames=0;
				start_time=GetTickCount64();
			}
			g_ypos=cam->ScreenHeight/4+(warand()%(cam->ScreenHeight/2-text_hW(str)/2));
			g_xpos=10+(warand()%(cam->ClipRight-text_w(str)-20));
		}
		else if (pos>=112) alpha=(127-pos);
		else if (pos >= 32) alpha=15;
		else if (pos >= 16) alpha=(pos-16);

		if (alpha&&str[0]) plTextPutStrW(cam,g_xpos,g_ypos,0, 1, str);
	}

	{
		int t=(GetTickCount64()-start_time)/1000;
		char nbuf[32] = {0};
		StringCchPrintfA(nbuf, 32, "%dfps",t?(m_creditspos_frames/t):0);
		plTextPutStr(cam,3,cam->ScreenHeight+1-text_hA(nbuf),0,1, nbuf);
	}
	m_creditspos_frames++;

	#define TEAM_IMG_W 32
	#define TEAM_IMG_H 32 //416
	if (teamtex && (!(m_creditspos_frames & 127)))
	{
		int g_regver=2;

		teamtex->Data += 32*32;

		if (g_regver < 1 && teamtex->Data-teamtexbase >= TEAM_IMG_W*(TEAM_IMG_H-32))
			teamtex->Data=teamtexbase;     
		else if (teamtex->Data-teamtexbase >= TEAM_IMG_W*TEAM_IMG_H)
		teamtex->Data=teamtexbase;     
	}
}

#define SPLASH_IMG_W 400
#define SPLASH_IMG_H 189

static pl_Texture *mkWALogoTex(int which)
{
	//pl_Texture *p=(pl_Texture*)GlobalAlloc(GPTR,sizeof(pl_Texture));
	pl_Texture* p = (pl_Texture*)malloc(sizeof(pl_Texture));
	if (p)
	{
		//char *temp=(char *)GlobalAlloc(GPTR,SPLASH_IMG_W*SPLASH_IMG_H);
		char* temp = (char*)malloc(SPLASH_IMG_W * SPLASH_IMG_H);
		//p->Data=GlobalAlloc(GPTR,which ? TEAM_IMG_H*TEAM_IMG_W : 256*256);
		p->Data = malloc(which ? TEAM_IMG_H * TEAM_IMG_W : 256 * 256);
		//p->PaletteData=GlobalAlloc(GPTR,3*256);
		p->PaletteData = malloc(3 * 256);
		if (p->Data && p->PaletteData && temp)
		{
			HBITMAP m_imgbm, m_imgoldbm;
			HDC m_imgdc;
			struct
			{
				BITMAPINFO bmi;
				RGBQUAD more_bmiColors[256];
				LPVOID data;
			} m_bitmap;

			int c;
			char *out=p->Data;

			memset(&m_bitmap, 0, sizeof(m_bitmap));

			m_imgdc = CreateCompatibleDC(NULL);
			// TODO (load from PNG)
			m_imgbm = LoadImage(hMainInstance,MAKEINTRESOURCE(which ? IDB_TEAM : IDB_SPLASH), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
			//m_imgbm = WALoadImage(hMainInstance, L"PNG", MAKEINTRESOURCEW(which ? IDR_TEAM : IDR_SPLASH), FALSE);
			m_imgoldbm=SelectObject(m_imgdc,m_imgbm);
			m_bitmap.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			m_bitmap.bmi.bmiHeader.biPlanes = 1;
			m_bitmap.bmi.bmiHeader.biBitCount = 8;
			m_bitmap.bmi.bmiHeader.biCompression = BI_RGB;
			m_bitmap.bmi.bmiHeader.biSizeImage = 0;
			m_bitmap.bmi.bmiHeader.biClrUsed = 256;
			m_bitmap.bmi.bmiHeader.biClrImportant = 256;
			m_bitmap.bmi.bmiHeader.biWidth = which ? TEAM_IMG_W : SPLASH_IMG_W;
			m_bitmap.bmi.bmiHeader.biHeight = which ? -TEAM_IMG_H : -SPLASH_IMG_H;
			m_bitmap.bmi.bmiHeader.biSizeImage = which ? TEAM_IMG_H * TEAM_IMG_W : SPLASH_IMG_W*SPLASH_IMG_H;

			GetDIBits(m_imgdc,m_imgbm,0,which ? TEAM_IMG_H : SPLASH_IMG_H,temp,(BITMAPINFO *)&m_bitmap,DIB_RGB_COLORS);
			GetDIBColorTable(m_imgdc,0,which ? 64 : 256,m_bitmap.bmi.bmiColors);

			SelectObject(m_imgdc, m_imgoldbm);
			DeleteDC(m_imgdc);
			DeleteObject(m_imgbm);

 			for (c = 0; c < (which ? 64 : 256); c ++) 
			{
				p->PaletteData[c*3] = m_bitmap.bmi.bmiColors[c].rgbRed;
				p->PaletteData[c*3+1] = m_bitmap.bmi.bmiColors[c].rgbGreen;
				p->PaletteData[c*3+2] = m_bitmap.bmi.bmiColors[c].rgbBlue;
			}

			if (which)
			{
				memcpy(out,temp,TEAM_IMG_W*TEAM_IMG_H);
				p->Width=5;
				p->Height=5;
				p->iWidth=32;
				p->iHeight=32;
				p->uScale=-32;
				p->vScale=32;
				p->NumColors=64;
			}
			else
			{
				int dxp=(SPLASH_IMG_W<<16)/256;
				int dyp=(SPLASH_IMG_H<<16)/256;
				int y, yp = 0;
				for (y = 0; y < 256; y ++)
				{
					char *in=temp+(yp>>16)*SPLASH_IMG_W;
					int x, xp = 0;
					for (x = 0; x < 256; x ++)
					{
						*out++=in[xp>>16];
						xp+=dxp;
					}
					yp+=dyp;
				}
				p->Width=8;
				p->Height=8;
				p->iWidth=256;
				p->iHeight=256;
				p->uScale=-256;
				p->vScale=256;
				p->NumColors=256;
			}
			//GlobalFree(temp);
			free(temp);
		}
		//else { if (p->Data) GlobalFree(p->Data); if (p->PaletteData) GlobalFree(p->PaletteData); GlobalFree(p); p=NULL; if (temp) GlobalFree(temp); }
		else { if (p->Data) free(p->Data); if (p->PaletteData) free(p->PaletteData); free(p); p = NULL; if (temp) free(temp); }
	}
	return p;
}

static pl_Texture *mkWATex()
{
	//pl_Texture *p=(pl_Texture*)GlobalAlloc(GPTR,sizeof(pl_Texture));
	pl_Texture* p = (pl_Texture*)malloc(sizeof(pl_Texture));
	if (p)
	{
		//p->Data=GlobalAlloc(GPTR,64*65+2);
		p->Data = malloc(64 * 65 + 2);
		//p->PaletteData=GlobalAlloc(GPTR,3*256);
		p->PaletteData = malloc(3 * 256);
		if (p->Data && p->PaletteData)
		{
			makeBallTextPal(p->PaletteData);
			p->Width=6;
			p->Height=6;
			p->iWidth=64;
			p->iHeight=64;
			p->uScale=128;
			p->vScale=64;
			p->NumColors=150;
		}
		//else { if (p->Data) GlobalFree(p->Data); if (p->PaletteData) GlobalFree(p->PaletteData); GlobalFree(p); p=NULL; }
		else { if (p->Data) free(p->Data); if (p->PaletteData) free(p->PaletteData); free(p); p = NULL; }
	}
	return p;
}

static pl_Texture *mkGroundTex()
{ 
	//pl_Texture *p=(pl_Texture*)GlobalAlloc(GPTR,sizeof(pl_Texture));
	pl_Texture* p = (pl_Texture*)malloc(sizeof(pl_Texture));
	if (p)
	{
		//p->Data=GlobalAlloc(GPTR,16*16);
		p->Data = malloc(16 * 16);
		//p->PaletteData=GlobalAlloc(GPTR,3*16);
		p->PaletteData = malloc(3 * 16);
	    if (p->Data && p->PaletteData)
		{
			int x,y;
			p->Width=4;
			p->Height=4;
			p->iWidth=16;
			p->iHeight=16;
			p->uScale=16*3;
			p->vScale=16*3;
			p->NumColors=16;
  
			for (y = 0; y < 16; y ++) for (x = 0; x < 16; x ++) p->Data[y*16+x]=x^y;

			for (x = 0; x < 16; x ++) 
			{
				p->PaletteData[x*3+0]=43+((93-43)*x)/16;
				p->PaletteData[x*3+1]=25+((52-25)*x)/16;
				p->PaletteData[x*3+2]=9+((23-9)*x)/16;
			}
		}
		//else { if (p->Data) GlobalFree(p->Data); if (p->PaletteData) GlobalFree(p->PaletteData); GlobalFree(p); p=NULL; }
		else { if (p->Data) free(p->Data); if (p->PaletteData) free(p->PaletteData); free(p); p = NULL; }
	}
	return p;
}

static void setup_materials(pl_Mat **mat, unsigned char *pal) 
{
	int fuckomode=!(GetAsyncKeyState(VK_CONTROL) & GetAsyncKeyState(VK_SHIFT) & 0x8000 );
	mat[0] = plMatCreate();
	mat[1] = plMatCreate();
	mat[2] = plMatCreate();
	mat[3] = plMatCreate();
	mat[4] = plMatCreate();
	mat[5] = plMatCreate();
	mat[6]=0;

	watex = mkWATex();
	grndtex=mkGroundTex();

	mat[0]->ShadeType = PL_SHADE_GOURAUD;
	mat[0]->Shininess = 16;
	mat[0]->NumGradients = 1500;
	mat[0]->Ambient[0] = -128;
	mat[0]->Ambient[1] = -128;
	mat[0]->Ambient[2] = -128; 
	mat[0]->Diffuse[0] = 170;
	mat[0]->Diffuse[1] = 140;
	mat[0]->Diffuse[2] = 140;
	mat[0]->Specular[0] = 140;
	mat[0]->Specular[1] = 90;
	mat[0]->Specular[2] = 0;
	mat[0]->FadeDist = 5000.0;
	mat[0]->Texture = fuckomode ? watex : grndtex;
	mat[0]->TexScaling = 8.0;
	mat[0]->PerspectiveCorrect = 16;

	mat[1]->ShadeType = PL_SHADE_GOURAUD;
	mat[1]->Shininess = 8;
	mat[1]->NumGradients = 150;
	mat[1]->Ambient[0] = 0;
	mat[1]->Ambient[1] = 0;
	mat[1]->Ambient[2] = 0; 
	mat[1]->Diffuse[0] = 0;
	mat[1]->Diffuse[1] = 0;
	mat[1]->Diffuse[2] = 0;
	mat[1]->Specular[0] = 450;
	mat[1]->Specular[1] = 264;
	mat[1]->Specular[2] = 150;

	mat[2]->ShadeType = PL_SHADE_GOURAUD;
	mat[2]->Shininess = 8;
	mat[2]->NumGradients = 150;
	mat[2]->Ambient[0] = 32;
	mat[2]->Ambient[1] = 32;
	mat[2]->Ambient[2] = 64; 
	mat[2]->Diffuse[0] = 120;
	mat[2]->Diffuse[1] = 60;
	mat[2]->Diffuse[2] = 60;
	mat[2]->Specular[0] = 200;
	mat[2]->Specular[1] = 80;
	mat[2]->Specular[2] = 80;

	mat[3]->ShadeType = PL_SHADE_GOURAUD;
	mat[3]->Shininess = 1;
	mat[3]->NumGradients = 1;
	mat[3]->Ambient[0] = 0;
	mat[3]->Ambient[1] = 0;
	mat[3]->Ambient[2] = 0; 
	mat[3]->Diffuse[0] = 0;
	mat[3]->Diffuse[1] = 0;
	mat[3]->Diffuse[2] = 0;
	mat[3]->Specular[0] = 0;
	mat[3]->Specular[1] = 0;
	mat[3]->Specular[2] = 0;
	mat[3]->Texture = fuckomode ? grndtex : watex;
	mat[3]->TexScaling = 1.4f;
	mat[3]->PerspectiveCorrect = 16;

	mat[4]->ShadeType = PL_SHADE_GOURAUD;
	mat[4]->Shininess = 1;
	mat[4]->NumGradients = 1;
	mat[4]->Ambient[0] = 0;
	mat[4]->Ambient[1] = 0;
	mat[4]->Ambient[2] = 0; 
	mat[4]->Diffuse[0] = 0;
	mat[4]->Diffuse[1] = 0;
	mat[4]->Diffuse[2] = 0;
	mat[4]->Specular[0] = 0;
	mat[4]->Specular[1] = 0;
	mat[4]->Specular[2] = 0;
	mat[4]->Texture = walogotex = mkWALogoTex(0);
	mat[4]->TexScaling = 1.0;
	mat[4]->PerspectiveCorrect = 16;

	mat[5]->ShadeType = PL_SHADE_GOURAUD;
	mat[5]->Shininess = 1;
	mat[5]->NumGradients = 1;
	mat[5]->Ambient[0] = 0;
	mat[5]->Ambient[1] = 0;
	mat[5]->Ambient[2] = 0; 
	mat[5]->Diffuse[0] = 0;
	mat[5]->Diffuse[1] = 0;
	mat[5]->Diffuse[2] = 0;
	mat[5]->Specular[0] = 0;
	mat[5]->Specular[1] = 0;
	mat[5]->Specular[2] = 0;
	mat[5]->Texture = teamtex = mkWALogoTex(1);
	mat[5]->TexScaling = 1.0;
	mat[5]->PerspectiveCorrect = 16;
	if (teamtex) teamtexbase=teamtex->Data;

	plMatInit(mat[0]);
	plMatInit(mat[1]);
	plMatInit(mat[2]);
	plMatInit(mat[3]);
	plMatInit(mat[4]);
	plMatInit(mat[5]);

	memset(pal,0,768);
	plMatMakeOptPal(pal,2,255,mat,1);

	pal[0] = pal[1] = pal[2] = 0;
	pal[3] = pal[4] = pal[5] = 255;

	plMatMapToPal(mat[0],pal,0,255);
	plMatMapToPal(mat[1],pal,0,255);
	plMatMapToPal(mat[2],pal,0,255);
	plMatMapToPal(mat[3],pal,0,255);
	plMatMapToPal(mat[4],pal,0,255);
	plMatMapToPal(mat[5],pal,0,255);
}

static void adjustmapping(pl_Obj *obj)
{
	int nf=obj->NumFaces;
	int x;
	pl_Face *f=obj->Faces;
	for (x = 0; x < nf; x ++)
	{
		f->MappingV[0]=MulDiv(f->MappingV[0],(150<<16)/500,(1<<16));
		f->MappingV[1]=MulDiv(f->MappingV[1],(150<<16)/500,(1<<16));
		f->MappingV[2]=MulDiv(f->MappingV[2],(150<<16)/500,(1<<16));
		f++;
	}
}

static pl_Obj *setup_landscape(float size, int div, pl_Mat *m) 
{
	
	pl_Obj *o = plMakePlane(size,size,div,m); 
	o->Children[0]= plMakePlane(size,size,div,m); 
	o->Children[0]->Yp=150;
	o->Children[0]->Xa=-180;
	div/=3;

	o->Children[1]= plMakePlane(size,150,div,m); 
	o->Children[1]->Yp=75;
	o->Children[1]->Zp=size/2;
	o->Children[1]->Xa=90;
	adjustmapping(o->Children[1]);
	o->Children[2]= plMakePlane(size,150,div,m); 
	o->Children[2]->Yp=75;
	o->Children[2]->Zp=-size/2;
	o->Children[2]->Xa=-90;
	adjustmapping(o->Children[2]);
	o->Children[3]= plMakePlane(size,150,div,m); 
	o->Children[3]->Yp=75;
	o->Children[3]->Xp=size/2;
	o->Children[3]->Za=-90;
	o->Children[3]->Ya=90;
	adjustmapping(o->Children[3]);
	o->Children[4]= plMakePlane(size,150,div,m); 
	o->Children[4]->Yp=75;
	o->Children[4]->Xp=-size/2;
	o->Children[4]->Za=90;
	o->Children[4]->Ya=90;
	adjustmapping(o->Children[4]);

 	o->Yp = 0;
	return (o);
}