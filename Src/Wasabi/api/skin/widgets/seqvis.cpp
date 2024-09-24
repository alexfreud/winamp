#include <precomp.h>

#include <api/wnd/popup.h>
#include <math.h>
#include <api/skin/skinparse.h>
#include <api/service/svc_enum.h>
#include <api/service/svcs/svc_skinfilter.h>
#include <api/skin/widgets/seqvis.h>
#include <api/core/api_core.h>
#include <tataki/canvas/bltcanvas.h>
const wchar_t eqVisXuiStr[] = L"EQVis"; // This is the xml tag
char eqVisXuiSvcName[] = "EQVis xui object"; // this is the name of the xuiservice
XMLParamPair SEQVis::params[] =
  {
    {
      SEQVIS_SETCOLORBOTTOM, L"COLORBOTTOM"
    },
    {SEQVIS_SETCOLORMIDDLE, L"COLORMIDDLE"},
    {SEQVIS_SETCOLORPREAMP, L"COLORPREAMP"},
    {SEQVIS_SETCOLORTOP, L"COLORTOP"},
    {SEQVIS_SETALPHA, L"GAMMA"},  // BACKWARD COMPAT
  };
SEQVis::SEQVis()
{
	getScriptObject()->vcpu_setInterface(eqvisGuid, (void *)static_cast<SEQVis *>(this));
	getScriptObject()->vcpu_setClassName(L"EqVis");
	getScriptObject()->vcpu_setController(eqvisController);
	colortop = colormid = colorbottom = 0xffffff;
	colorpreamp = 0x888888;
	shadedColors = NULL;
	bc = NULL;
	sfe = new SkinFilterEnum();

	while (1)
	{
		svc_skinFilter *obj = sfe->getNext();
		if (!obj) break;
		filters.addItem(obj);
	}
	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);
	
}

void SEQVis::CreateXMLParameters(int master_handle)
{
	//SEQVIS_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

int SEQVis::onInit()
{
	SEQVIS_PARENT::onInit();
	WASABI_API_MEDIACORE->core_addCallback(0, this);
	return 1;
}

SEQVis::~SEQVis()
{
	foreach(filters)
	sfe->release(filters.getfor());
	endfor;
	delete sfe;
	if (shadedColors) FREE(shadedColors);
	delete(bc);
	WASABI_API_MEDIACORE->core_delCallback(0, this);
}

int SEQVis::setXuiParam(int _xuihandle, int attrid, const wchar_t *paramname, const wchar_t *strvalue)
{
	if (_xuihandle != xuihandle) return SEQVIS_PARENT::setXuiParam(_xuihandle, attrid, paramname, strvalue);
	switch (attrid)
	{
		case SEQVIS_SETALPHA:
			getGuiObject()->guiobject_setAlpha(WTOI(strvalue));
			break;
		case SEQVIS_SETCOLORTOP:
			colortop = RGBTOBGR(SkinParser::parseColor(strvalue));
			break;
		case SEQVIS_SETCOLORMIDDLE:
			colormid = RGBTOBGR(SkinParser::parseColor(strvalue));
			break;
		case SEQVIS_SETCOLORBOTTOM:
			colorbottom = RGBTOBGR(SkinParser::parseColor(strvalue));
			break;
		case SEQVIS_SETCOLORPREAMP:
			colorpreamp = RGBTOBGR(SkinParser::parseColor(strvalue));
			break;
		default:
			return 0;
	}
	return 1;
}

void SEQVis::splineGetPoint(spline_struct *s, float frame, float *out)
{
	int i, i_1, i0, i1, i2;
	float time1, time2, time3;
	float t1, t2, t3, t4, u1, u2, u3, u4, v1, v2, v3;
	float a, b, c, d;

	float *keys = s->keys;

	a = (1 - s->tens) * (1 + s->cont) * (1 + s->bias);
	b = (1 - s->tens) * (1 - s->cont) * (1 - s->bias);
	c = (1 - s->tens) * (1 - s->cont) * (1 + s->bias);
	d = (1 - s->tens) * (1 + s->cont) * (1 - s->bias);
	v1 = t1 = -a / 2.0f; u1 = a;
	u2 = (-6 - 2 * a + 2 * b + c) / 2.0f; v2 = (a - b) / 2.0f; t2 = (4 + a - b - c) / 2.0f;
	t3 = (-4 + b + c - d) / 2.0f;
	u3 = (6 - 2 * b - c + d) / 2.0f;
	v3 = b / 2.0f;
	t4 = d / 2.0f; u4 = -t4;

	i0 = (int) frame;
	i_1 = i0 - 1;
	while (i_1 < 0) i_1 += s->numKeys;
	i1 = i0 + 1;
	while (i1 >= s->numKeys) i1 -= s->numKeys;
	i2 = i0 + 2;
	while (i2 >= s->numKeys) i2 -= s->numKeys;
	time1 = frame - (float)((int) frame);
	time2 = time1 * time1;
	time3 = time2 * time1;
	i0 *= s->keyWidth;
	i1 *= s->keyWidth;
	i2 *= s->keyWidth;
	i_1 *= s->keyWidth;
	for (i = 0; i < s->keyWidth; i ++)
	{
		a = t1 * keys[i + i_1] + t2 * keys[i + i0] + t3 * keys[i + i1] + t4 * keys[i + i2];
		b = u1 * keys[i + i_1] + u2 * keys[i + i0] + u3 * keys[i + i1] + u4 * keys[i + i2];
		c = v1 * keys[i + i_1] + v2 * keys[i + i0] + v3 * keys[i + i1];
		*out++ = a * time3 + b * time2 + c * time1 + keys[i + i0];
	}
}

void SEQVis::DrawEQVis()
{

	if (!shadedColors) return ;

	float keys[12] = {0};
	spline_struct spline = {keys, 1, 12, 0.0f, 0.0f, 0.1f};

	MEMSET(specData, 0, cur_w*cur_h*4);

	int ph = (int)((127 + WASABI_API_MEDIACORE->core_getEqPreamp(0)) * ((float)cur_h) / 256.0f);
	ph *= cur_w;
	for (int j = 0;j < cur_w;j++)
		specData[j + ph] = colorpreamp | 0xFF000000; // alpha :)

	{
		int x;
		int last_p = -1;
		for (x = 0; x < 10; x ++)
			keys[x + 1] = (127 - WASABI_API_MEDIACORE->core_getEqBand(0, x)) * ((float)cur_h) / 256.0f;
		keys[0] = keys[1];
		keys[11] = keys[10];

		for (x = 0; x < cur_w; x ++)
		{
			float p;
			int this_p;
			splineGetPoint(&spline, 1.0f + x / (cur_w*11.0f / 100.0f), &p);
			this_p = (int)p;
			if (this_p < 0) this_p = 0;
			if (this_p >= cur_h) this_p = cur_h - 1;
			if (last_p == -1 || this_p == last_p)
				specData[x + this_p*cur_w] = shadedColors[this_p];
			else
			{
				if (this_p < last_p)
					for (int j = 0;j < last_p - this_p + 1;j++)
						specData[x + (this_p + j)*cur_w] = shadedColors[this_p + j];
				else if (this_p > last_p)
					for (int j = 0;j < this_p - last_p + 1;j++)
						specData[x + (last_p + j)*cur_w] = shadedColors[last_p + j];
			}
			last_p = this_p;
		}
	}
	invalidate();
	invalidated = 1; // rerun filter
}

int SEQVis::onPaint(Canvas *canvas)
{
	PaintCanvas paintcanvas;
	if (canvas == NULL)
	{
		if (!paintcanvas.beginPaint(this)) return 0;
		canvas = &paintcanvas;
	}

	SEQVIS_PARENT::onPaint(canvas);

	RECT s, r;
	getClientRect(&r);
	s.left = 0;
	s.top = 0;
	s.right = cur_w;
	s.bottom = cur_h;

	if (invalidated)
	{
		SkinBitmap *b = bc->getSkinBitmap();
		foreach(filters)
		filters.getfor()->filterBitmap((unsigned char *)bc->getBits(), b->getWidth(), b->getHeight(), 32, NULL, L"Vis/Eq");
		endfor;
		invalidated = 0;
		b->stretchToRectAlpha(canvas, &s, &r, getPaintingAlpha());
	}
	else
	{
		bc->stretchToRectAlpha(canvas, &s, &r, getPaintingAlpha());
		//SkinBitmap *b = bc->getSkinBitmap();
		//b->stretchToRectAlpha(canvas, &s, &r, getPaintingAlpha());
	}


	return 1;
}

int SEQVis::corecb_onEQBandChange(int b, int newval)
{
	DrawEQVis();
	return 0;
}

int SEQVis::corecb_onEQPreampChange(int newval)
{
	DrawEQVis();
	return 0;
}

void SEQVis::reloadResources()
{
	SEQVIS_PARENT::reloadResources();
	DrawEQVis();
	invalidate();
}

int SEQVis::onResize()
{
	SEQVIS_PARENT::onResize();
	RECT r;
	getClientRect(&r);

	cur_h = r.bottom - r.top;
	if (cur_h <= 0) cur_h = 1;
	cur_w = r.right - r.left;
	if (cur_w <= 0) cur_w = 1;

	delete bc;
	bc = new BltCanvas(cur_w, -cur_h);
	specData = (int *)bc->getBits();
	MEMSET(specData, 0, cur_w*cur_h*4);

	if (shadedColors) FREE(shadedColors);
	shadedColors = (int *)MALLOC(sizeof(int) * cur_h);
	int r1 = colortop & 0xff0000;
	int g1 = (colortop & 0x00ff00) << 8;
	int b1 = (colortop & 0x0000ff) << 16;
	int r2 = colormid & 0xff0000;
	int g2 = (colormid & 0x00ff00) << 8;
	int b2 = (colormid & 0x0000ff) << 16;
	int r3 = colorbottom & 0xff0000;
	int g3 = (colorbottom & 0x00ff00) << 8;
	int b3 = (colorbottom & 0x0000ff) << 16;
	int l = cur_h / 2;
	if (!l) l = 1;
	int i;
	for (i = 0;i < l;i++)
	{
		int r = r1 + ((r2 - r1) * i / l);
		int g = g1 + ((g2 - g1) * i / l);
		int b = b1 + ((b2 - b1) * i / l);
		shadedColors[i] = (r & 0xff0000);
		shadedColors[i] += ((g & 0xff0000) >> 8);
		shadedColors[i] += ((b & 0xff0000) >> 16);
		shadedColors[i] += 0xff000000; // alpha?
	}
	for (i = l;i < cur_h;i++)
	{
		int r = r2 + ((r3 - r2) * (i - l) / l);
		int g = g2 + ((g3 - g2) * (i - l) / l);
		int b = b2 + ((b3 - b2) * (i - l) / l);
		shadedColors[i] = (r & 0xff0000);
		shadedColors[i] += ((g & 0xff0000) >> 8);
		shadedColors[i] += ((b & 0xff0000) >> 16);
		shadedColors[i] += 0xff000000; // alpha?
	}
	DrawEQVis();

	return 1;
}

EqVisScriptController _eqvisController;
EqVisScriptController *eqvisController = &_eqvisController;

// -- Functions table -------------------------------------
function_descriptor_struct EqVisScriptController::exportedFunction[] = {
      {L"fake", 0, (void*)SEQVis::script_vcpu_fake },
    };

// --------------------------------------------------------

const wchar_t *EqVisScriptController::getClassName()
{
	return L"EqVis";
}

const wchar_t *EqVisScriptController::getAncestorClassName()
{
	return L"GuiObject";
}

ScriptObject *EqVisScriptController::instantiate()
{
	SEQVis *eqv = new SEQVis;
	ASSERT(eqv != NULL);
	return eqv->getScriptObject();
}

void EqVisScriptController::destroy(ScriptObject *o)
{
	SEQVis *eqv = static_cast<SEQVis *>(o->vcpu_getInterface(eqvisGuid));
	ASSERT(eqv != NULL);
	delete eqv;
}

void *EqVisScriptController::encapsulate(ScriptObject *o)
{
	return NULL; // no encapsulation for eqvis yet
}

void EqVisScriptController::deencapsulate(void *o)
{}

int EqVisScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *EqVisScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID EqVisScriptController::getClassGuid()
{
	return eqvisGuid;
}

// -----------------------------------------------------------------------


scriptVar SEQVis::script_vcpu_fake(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	RETURN_SCRIPT_VOID;
}

