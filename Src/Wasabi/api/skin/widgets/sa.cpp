#include <precomp.h>
#include <api/wnd/popup.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <math.h>
#include <api/skin/skinparse.h>
#include "sa.h"
#include <api/core/api_core.h>
#include <tataki/canvas/bltcanvas.h>
#include "resource.h"
#include "../Agave/Language/api_language.h"

const wchar_t visXuiStr[] = L"Vis"; // This is the xml tag
char visXuiSvcName[] = "Vis xui object"; // this is the name of the xuiservice


// {9149C445-3C30-4e04-8433-5A518ED0FDDE}
const GUID uioptions_guid = 
	{ 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };

unsigned char ppal[] = { 
#ifdef CLASSIC
	0,0,0, // color 0 = black
	75,72,80, // color 1 = grey for dots
	255,55,16, // color 2 = top of spec
	255,55,16, // 3
	255,80,0, // 4
	255,80,0, // 5
	239,112,0, // 6
	239,112,0, // 7
	255,168,32, // 8
	255,168,32, // 9
	176,255,47, // 10
	176,255,47, // 11
	47,239,0, // 12
	47,239,0, // 13
	48,160,0, // 14
	48,160,0,  // 15
	32,128,0,  // 16
	32,128,0,   // 17 = bottom of spec
	255,255,255, // 18 = osc 1
	214,214,222, // 19 = osc 2 (slightly dimmer)
	181,189,189, // 20 = osc 3
	160,170,175,  // 21 = osc 4
	148,156,165,  // 22 = osc 4
	150, 150, 150, // 23 = analyzer peak dots
#else
	0,0,0, // color 0 = black
	24,33,41, // color 1 = grey for dots
	239,49,16, // color 2 = top of spec
	206,41,16, // 3
	214,90,0, // 4
	214,102,0, // 5
	214,115,0, // 6
	198,123,8, // 7
	222,165,24, // 8
	214,181,33, // 9
	189,222,41, // 10
	148,222,33, // 11
	41,206,16, // 12
	50,190,16, // 13
	57,181,16, // 14
	49,156,8,  // 15
	41,148,0,  // 16
	24,132,8,   // 17
	255,255,255, // 18 = osc 1
	214,214,222, // 19 = osc 2 (slightly dimmer)
	181,189,189, // 20 = osc 3
	160,170,175,  // 21 = osc 4
	148,156,165,  // 22 = osc 4
	150, 150, 150, // 23 = analyzer peak
#endif
};

#define CHANNEL_LEFT  1
#define CHANNEL_RIGHT 2

XMLParamPair SAWnd::params[] = {
	{SA_SETCOLORALLBANDS, L"COLORALLBANDS"},
	{SA_SETCOLORBAND1, L"COLORBAND1"},
	{SA_SETCOLORBAND2, L"COLORBAND2"},
	{SA_SETCOLORBAND3, L"COLORBAND3"},
	{SA_SETCOLORBAND4, L"COLORBAND4"},
	{SA_SETCOLORBAND5, L"COLORBAND5"},
	{SA_SETCOLORBAND6, L"COLORBAND6"},
	{SA_SETCOLORBAND7, L"COLORBAND7"},
	{SA_SETCOLORBAND8, L"COLORBAND8"},
	{SA_SETCOLORBAND9, L"COLORBAND9"},
	{SA_SETCOLORBAND10, L"COLORBAND10"},
	{SA_SETCOLORBAND11, L"COLORBAND11"},
	{SA_SETCOLORBAND12, L"COLORBAND12"},
	{SA_SETCOLORBAND13, L"COLORBAND13"},
	{SA_SETCOLORBAND14, L"COLORBAND14"},
	{SA_SETCOLORBAND15, L"COLORBAND15"},
	{SA_SETCOLORBAND16, L"COLORBAND16"},
	{SA_SETCOLORBANDPEAK, L"COLORBANDPEAK"},
	{SA_SETCOLORALLOSC, L"COLORALLOSC"},
	{SA_SETCOLOROSC1, L"COLOROSC1"},
	{SA_SETCOLOROSC2, L"COLOROSC2"},
	{SA_SETCOLOROSC3, L"COLOROSC3"},
	{SA_SETCOLOROSC4, L"COLOROSC4"},
	{SA_SETCOLOROSC5, L"COLOROSC5"},
	{SA_SETCHANNEL, L"CHANNEL"},
	{SA_SETFLIPH, L"FLIPH"},
	{SA_SETFLIPV, L"FLIPV"},
	{SA_SETMODE, L"MODE"},
	{SA_SETGAMMA, L"GAMMAGROUP"},
	{SA_SETFALLOFF, L"FALLOFF"},
	{SA_SETPEAKFALLOFF, L"PEAKFALLOFF"},
	{SA_SETBANDWIDTH, L"BANDWIDTH"},
	{SA_FPS, L"FPS"},
	{SA_COLORING, L"COLORING"},
	{SA_PEAKS, L"PEAKS"},
	{SA_OSCDRAWSTYLE, L"OSCSTYLE"},
};

SAWnd::SAWnd() 
{
	filtergroup = SA_PARENT::getFiltersGroup();

	getScriptObject()->vcpu_setInterface(visGuid, (void *)static_cast<SAWnd *>(this));
	getScriptObject()->vcpu_setClassName(L"Vis");
	getScriptObject()->vcpu_setController(visController);

	char *p = (char *)&ppal;
	for (int i=0;i<72;i++) {
		palette[i] = ((*p)<<16) + ((*(p+1))<<8) + *(p+2);
		p += 3;
	}

	setRealtime(1);
	startQuickPaint();

	config_sa=-1; // default value
#ifdef WASABI_COMPILE_CONFIG
	{
	    CfgItem *ci=WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid);
		if(ci) {
			config_sa=ci->getDataAsInt(L"Spectrum analyzer mode");
			viewer_addViewItem(ci->getDependencyPtr());
		}
	}
	saveconfsa=0;
#endif

	flip_v = 0;
	flip_h = 0;
	channel = CHANNEL_LEFT | CHANNEL_RIGHT;
	off = 0;

	config_safalloff=2;
	config_sa_peak_falloff=1;
	config_safire=4;
	config_sa_peaks=1;

	memset(bx, 0, sizeof(bx));
	memset(t_bx, 0, sizeof(t_bx));
	memset(t_vx, 0, sizeof(t_vx));

	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);
}

void SAWnd::CreateXMLParameters(int master_handle)
{
	//SA_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

int SAWnd::onInit() {
	SA_PARENT::onInit();
#ifdef WASABI_COMPILE_CONFIG
	if(config_sa==-1) {
		Layout *pl=getGuiObject()->guiobject_getParentLayout();
		if (pl->getParentContainer()) {
			confsaname.printf(L"%s/%s/%s/config_sa",WASABI_API_SKIN->getSkinName(),pl->getParentContainer()->getName(),pl->getName());
			config_sa=WASABI_API_CONFIG->getIntPrivate(confsaname,1);
			saveconfsa=1;
		}
	}
#endif
	return 1;
}

SAWnd::~SAWnd()
{
#ifdef WASABI_COMPILE_CONFIG
	if(saveconfsa) 
		WASABI_API_CONFIG->setIntPrivate(confsaname,config_sa);
#endif
}

int SAWnd::setXuiParam(int _xuihandle, int attrid, const wchar_t *pname, const wchar_t *str) {
	if (_xuihandle != xuihandle) return SA_PARENT::setXuiParam(_xuihandle, attrid, pname, str);
	switch (attrid) {
		case SA_SETCOLORALLBANDS:
			setBandColor(-1, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND1:
			setBandColor(15, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND2:
			setBandColor(14, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND3:
			setBandColor(13, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND4:
			setBandColor(12, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND5:
			setBandColor(11, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND6:
			setBandColor(10, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND7:
			setBandColor(9, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND8:
			setBandColor(8, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND9:
			setBandColor(7, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND10:
			setBandColor(6, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND11:
			setBandColor(5, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND12:
			setBandColor(4, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND13:
			setBandColor(3, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND14:
			setBandColor(2, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND15:
			setBandColor(1, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBAND16:
			setBandColor(0, SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORBANDPEAK:
			setPeakColor(SkinParser::parseColor(str)); 
			break;
		case SA_SETCOLORALLOSC:
			setOscColor(-1, SkinParser::parseColor(str));
			break;
		case SA_SETCOLOROSC1:
			setOscColor(0, SkinParser::parseColor(str));
			break;
		case SA_SETCOLOROSC2:
			setOscColor(1, SkinParser::parseColor(str));
			break;
		case SA_SETCOLOROSC3:
			setOscColor(2, SkinParser::parseColor(str));
			break;
		case SA_SETCOLOROSC4:
			setOscColor(3, SkinParser::parseColor(str));
			break;
		case SA_SETCOLOROSC5:
			setOscColor(4, SkinParser::parseColor(str));
			break;
		case SA_SETCHANNEL:
			setChannel(WTOI(str)); 
			break;
		case SA_SETFLIPH:
			setFlipH(WTOI(str)); 
			break;
		case SA_SETFLIPV:
			setFlipV(WTOI(str)); 
			break;
		case SA_SETMODE:
			setMode(WTOI(str)); 
			break;
		case SA_SETGAMMA:
			filtergroup = str;
			break;
		case SA_SETFALLOFF:
			config_safalloff = WTOI(str);
			break;
		case SA_SETPEAKFALLOFF:
			config_sa_peak_falloff=WTOI(str);
			break;
		case SA_SETBANDWIDTH:
			if (WCSCASEEQLSAFE(str, L"wide"))
				config_safire &= (~32);
			else if (WCSCASEEQLSAFE(str, L"thin"))
				config_safire |= 32;
			break;
		case SA_FPS:
			{
				int fps = WTOI(str);
				if (fps)
					setSpeed(1000/fps);
			}
			break;
		case SA_COLORING:
			if (WCSCASEEQLSAFE(str, L"fire"))
				config_safire = (config_safire&~3) | 1;
			else if (WCSCASEEQLSAFE(str, L"normal"))
				config_safire = (config_safire&~3) | 0;
			else if (WCSCASEEQLSAFE(str, L"line"))
				config_safire = (config_safire&~3) | 2;
			break;
		case SA_PEAKS:
			if (str && WTOI(str))
				config_sa_peaks=1;
			else
				config_sa_peaks=0;
			break;
		case SA_OSCDRAWSTYLE:
			if (WCSCASEEQLSAFE(str, L"dots"))
				config_safire = (config_safire & ~(3<<2)) | 0;
			else if (WCSCASEEQLSAFE(str, L"solid"))
				config_safire = (config_safire & ~(3<<2)) | 4;
			else if (WCSCASEEQLSAFE(str, L"lines"))
				config_safire = (config_safire & ~(3<<2)) | 8;
			break;
		default:
			return 0;
	}
	return 1;
}

void SAWnd::setChannel(int c) {
	channel = c;
}

void SAWnd::setFlipH(int v) {
	if (v == flip_h) return;
	flip_h = v;
	invalidate();
}

void SAWnd::setFlipV(int v) {
	if (v == flip_v) return;
	flip_v = v;
	invalidate();
}

void SAWnd::setBandColor(int band, ARGB32 col) {
	if (band == -1) {
		for (int i=0;i<16;i++)
			palette[i+2] = RGBTOBGR(col);
	} else
		palette[band+2] = RGBTOBGR(col);
}

void SAWnd::setOscColor(int n, ARGB32 col) {
	if (n == -1) {
		for (int i=0;i<4;i++)
			palette[i+18] = RGBTOBGR(col);
	} else
		palette[n+18] = RGBTOBGR(col);
}

void SAWnd::setPeakColor(ARGB32 col) {
	palette[23] = RGBTOBGR(col);
}

#define SA_BLEND(c) (palette[c] | 0xFF000000) //(alpha << 24))

int SAWnd::onQuickPaint(BltCanvas *bc, int w, int h, int newone) {
	if(!isVisible()) return 0;

#ifdef WASABI_COMPILE_MEDIACORE
	int x;
	int fo[5] = {3, 6, 12, 16, 32 };
	float pfo[5]={1.05f,1.1f,1.2f,1.4f,1.6f};

	specData=(int *)bc->getBits();
	if (newone || !config_sa)
    MEMSET(specData,0,76*16*4*4);

	if(!config_sa) {
		if (!off) {
			off = 1;
			return 1;
		}
		return 0;
	}

	off = 0;

	char visdata[576*2*2] = {0};
	unsigned char *values=(unsigned char *)visdata;
	int ret=WASABI_API_MEDIACORE->core_getVisData(0,visdata,sizeof(visdata));
	if (!ret) {
		MEMSET(visdata,0,sizeof(visdata));
	} else if (ret == 75*2) {
		if (config_sa==2) values+=75;
	} else {
  		if(config_sa==1) {
			register int v;
			for(int x=0;x<75;x++) {
				v=values[x]+values[576+x];
				v>>=4;
				values[x]=v;
			}
		}
		if (config_sa==2) {
			values+=576*2;
			register int v;
			register char *blah=(char *)values;
			for(int x=0;x<75;x++) {
				v=blah[x*4]+blah[576+(x*4)];
				v>>=4;
  				blah[x]=v;
			}
  		}
	}

//	int ws=(config_windowshade&&config_mw_open);
//	int s = (config_dsize&&config_mw_open)?1:0;

	int dbx = fo[max(min(config_safalloff,4),0)];
	float spfo=pfo[max(min(config_sa_peak_falloff,4),0)];

	MEMSET(specData,0,76*16*4*4);

	{
		{
			if (config_sa == 2)
			{
				int *gmem = specData;
				{
					int lv=-1;
					if (((config_safire>>2)&3)==0) for (x = 0; x < 75; x ++)
					{
						register int v; register char c;
						v = (((int) ((signed char *)values)[x])) + 8;
						if (v < 0) v = 0 ; if (v > 15) v = 15; c = v/2-4; if (c < 0) c = -c; c += 18;
						gmem[v*76*2] = SA_BLEND(c);
						gmem++;
					}
					else if (((config_safire>>2)&3)==1) for (x = 0; x < 75; x ++)
					{
						register int v,t; register char c;
						v = (((int) ((signed char *)values)[x])) + 8;
						if (v < 0) v = 0 ; if (v > 15) v = 15; c = v/2-4; if (c < 0) c = -c; c += 18;
						if (lv == -1) lv=v;
						t=lv;
						lv=v;
						if (v >= t) while (v >= t) gmem[v--*76*2] = SA_BLEND(c);
						else while (v < t) gmem[v++*76*2] = SA_BLEND(c);
						gmem++;
					}
					else if (((config_safire>>2)&3)==2) for (x = 0; x < 75; x ++) // solid
					{
						register int v; register char c;
						v = (((int) ((signed char *)values)[x])) + 8;
						if (v < 0) v = 0 ; if (v > 15) v = 15; c = v/2-4; if (c < 0) c = -c; c += 18;
						if (v > 7) while (v > 7) gmem[v--*76*2] = SA_BLEND(c);
						else while (v <= 7) gmem[v++*76*2] = SA_BLEND(c);
						gmem++;
					}
				}
			} 
			else
			{
				for (x = 0; x < 75; x ++)
				{
					register int y,v,t;
#ifndef CLASSIC
					t=x&~3;
#else
					t=x-(x%6);
#endif
					if (!(config_safire&32)) 
					{
						int a=values[t],b=values[t+1],c=values[t+2],d=values[t+3];
#ifndef CLASSIC
						v = a+b+c+d;//-min(a,min(b,min(c,d)));
						v/=4;
#else
						v = a+b+c+d+(int)values[t+4]+(int)values[t+5];//-min(a,min(b,min(c,d)));
						v/=6;
#endif
					}
					else v = (((int)values[x]));
					if (v > 15) v = 15;
					if ((v<<4) < bx[x]) v = (bx[x]-=dbx)>>4;
					else bx[x] = v<<4;
	 				if (bx[x] < 0) bx[x] = 0;
					if (v < 0) v = 0;
					int *gmem = specData + 76*2*15 + x;
					if ((config_safire&3)==1) t = v+2;
					else if ((config_safire&3)==2) t=17-(v);
					else t = 17;

					if (t_bx[x] <= v*256) {
						t_bx[x]=v*256;
						t_vx[x]=3.0f;
					}
#ifndef CLASSIC
					if ((config_safire&32 || (x&3)!=3)) 
					{
						if ((config_safire&3)!=2) for (y = 0; y < v; y ++)
						{
							*gmem = SA_BLEND(t-y);
							gmem -= 76*2;
						}
						else for (y = 0; y < v; y ++)
						{
							*gmem = SA_BLEND(t);
							gmem -= 76*2;
						}
#else
					if ((config_safire&32 || (!(x&1) && (x%6) < 4))) 
					{
						if ((config_safire&3)!=2) for (y = 0; y < v/2; y ++)
						{
							*gmem = SA_BLEND(t-y*2);
							gmem -= 76*2*2;
						}
						else for (y = 0; y < v/2; y ++)
						{
							*gmem = SA_BLEND(t);
							gmem -= 76*2*2;
						}
#endif
#ifndef CLASSIC
						if (config_sa_peaks && t_bx[x]/256 >= 0 && t_bx[x]/256 <= 15)
						{
							specData[76*2*15 - (t_bx[x]/256)*76*2 + x]=SA_BLEND(23);
						}
#endif
					}
					t_bx[x] -= (int)t_vx[x];
					t_vx[x] *= spfo;
					if (t_bx[x] < 0) t_bx[x]=0;
				}
			}
		}
	}

	if (flip_v)
		bc->vflip(2);
	if (flip_h)
	    bc->hflip(2);

	invalidated = 1; // rerun filter

#endif //mediacore

	return 1;
}

int SAWnd::onLeftButtonDown(int x, int y) {
	SA_PARENT::onLeftButtonDown(x, y);
	if (!WASABI_API_MAKI->vcpu_getComplete()) {
		nextMode();
#ifdef WASABI_COMPILE_CONFIG
		CfgItem *ci=WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid);
		if(ci) ci->setDataAsInt(L"Spectrum analyzer mode",config_sa);
#endif
	}
	return 1;
}

void SAWnd::nextMode() {
	config_sa++;
	if(config_sa>2) config_sa=0;
}

void SAWnd::setMode(int mode) {
	config_sa=mode;
	if(config_sa>2) config_sa=0;
}

int SAWnd::getMode() {
	return config_sa;
}

int SAWnd::onRightButtonUp(int x, int y)
{
	SA_PARENT::onRightButtonUp(x, y);

	PopupMenu menu (this);
	menu.addCommand(WASABI_API_LNGSTRINGW(IDS_NO_VISUALISATION)/*"No visualization"*/, 0, config_sa==0, FALSE);
	menu.addCommand(WASABI_API_LNGSTRINGW(IDS_SPECTRUM_ANALYZER)/*L"Spectrum analyzer"*/, 1, config_sa==1, FALSE);
	menu.addCommand(WASABI_API_LNGSTRINGW(IDS_OSCILLOSCOPE)/*L"Oscilloscope"*/, 2, config_sa==2, FALSE);

	clientToScreen(&x, &y);

	int ret = menu.popAtXY(x,y);
#ifdef WASABI_COMPILE_CONFIG
	if(ret>=0)
	{
		config_sa=ret;
		CfgItem *ci=WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid);
		if(ci) ci->setDataAsInt(L"Spectrum analyzer mode",config_sa);
	}
#endif

	WASABI_API_MAKI->vcpu_setComplete();
	return 1;
}

int SAWnd::getPreferences(int what) {
	switch (what) {
		case SUGGESTED_W: return 76;
		case SUGGESTED_H: return 16;
	}
	return SA_PARENT::getPreferences(what);
}

void SAWnd::getQuickPaintSize(int *w, int *h) {
	if (w) *w = 76*2;
	if (h) *h = 16*2;
}

void SAWnd::getQuickPaintSource(RECT *r) {
	ASSERT(r != NULL);
	r->left = 0;
	r->top = 0;
	r->right = 72;
	r->bottom = 16;
}

int SAWnd::viewer_onEvent(api_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen) { 
#ifdef WASABI_COMPILE_CONFIG
	if(event==CfgItem::Event_ATTRIBUTE_CHANGED && ptr && STRCASEEQLSAFE((const char *)ptr, "Spectrum analyzer mode")) {
		CfgItem *ci=WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid);
		if(ci) config_sa=ci->getDataAsInt((const wchar_t *)ptr);
	}
#endif
	return 1; 
}

VisScriptController _visController;
VisScriptController *visController = &_visController;

// -- Functions table -------------------------------------
function_descriptor_struct VisScriptController::exportedFunction[] = {
	{L"onFrame",     0, (void*)SAWnd::script_onFrame },
	{L"setRealtime", 1, (void*)SAWnd::script_setRealtime },
	{L"getRealtime", 0, (void*)SAWnd::script_getRealtime },
	{L"setMode",    1, (void*)SAWnd::script_vcpu_setMode},
	{L"getMode",    0, (void*)SAWnd::script_vcpu_getMode},
	{L"nextMode",    0, (void*)SAWnd::script_vcpu_nextMode},
};

// --------------------------------------------------------
const wchar_t *VisScriptController::getClassName() {
	return L"Vis";
}

const wchar_t *VisScriptController::getAncestorClassName() {
	return L"GuiObject";
}

ScriptObject *VisScriptController::instantiate() {
	SAWnd *sa = new SAWnd;
	ASSERT(sa != NULL);
	return sa->getScriptObject();
}

void VisScriptController::destroy(ScriptObject *o) {
	SAWnd *sa = static_cast<SAWnd *>(o->vcpu_getInterface(visGuid));
	ASSERT(sa != NULL);
	delete sa;
}

void *VisScriptController::encapsulate(ScriptObject *o) {
	return NULL; // no encapsulation for vis yet
}

void VisScriptController::deencapsulate(void *o) {
}

int VisScriptController::getNumFunctions() {
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *VisScriptController::getExportedFunctions() {
	return exportedFunction;                                                        
}

GUID VisScriptController::getClassGuid() {
	return visGuid;
}

// -----------------------------------------------------------------------
scriptVar SAWnd::script_onFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT; 
	PROCESS_HOOKS0(o, visController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar SAWnd::script_setRealtime(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s) {
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&s));
	SAWnd *sa = static_cast<SAWnd*>(o->vcpu_getInterface(visGuid));
	if (sa) sa->setRealtime(SOM::makeInt(&s));
	RETURN_SCRIPT_VOID;
}

scriptVar SAWnd::script_getRealtime(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT
	SAWnd *sa = static_cast<SAWnd*>(o->vcpu_getInterface(visGuid));
	if (sa) return MAKE_SCRIPT_INT(sa->getRealtime());
	return MAKE_SCRIPT_INT(0);
}

scriptVar SAWnd::script_vcpu_setMode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a) {
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&a));
	SAWnd *sa = static_cast<SAWnd*>(o->vcpu_getInterface(visGuid));
	if (sa) sa->setMode(SOM::makeInt(&a));
	RETURN_SCRIPT_VOID;
}

scriptVar SAWnd::script_vcpu_getMode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT
	SAWnd *sa = static_cast<SAWnd*>(o->vcpu_getInterface(visGuid));
	if (sa) return MAKE_SCRIPT_INT(sa->getMode());
	return MAKE_SCRIPT_INT(0);
}

scriptVar SAWnd::script_vcpu_nextMode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
	SCRIPT_FUNCTION_INIT
	SAWnd *sa = static_cast<SAWnd*>(o->vcpu_getInterface(visGuid));
	if (sa) sa->nextMode();
	RETURN_SCRIPT_VOID;
}
