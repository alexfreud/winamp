#include "precomp.h"
#include <api/script/scriptmgr.h>
#include <api/script/script.h>
#include <api/skin/widgets/animlayer.h>
#include <tataki/canvas/canvas.h>

const wchar_t animLayerXuiObjectStr[] = L"AnimatedLayer"; // This is the xml tag
char animLayerXuiSvcName[] = "Animated Layer xui object"; // this is the name of the xuiservice

AnimLayerScriptController _animlayerController;
AnimLayerScriptController *animlayerController = &_animlayerController;

// -- Functions table -------------------------------------
function_descriptor_struct AnimLayerScriptController::exportedFunction[] = {
            {L"setSpeed", 1, (void*)AnimatedLayer::script_vcpu_setSpeed },
            {L"gotoFrame", 1, (void*)AnimatedLayer::script_vcpu_gotoFrame },
            {L"setStartFrame", 1, (void*)AnimatedLayer::script_vcpu_setStartFrame },
            {L"setEndFrame", 1, (void*)AnimatedLayer::script_vcpu_setEndFrame },
            {L"setAutoReplay", 1, (void*)AnimatedLayer::script_vcpu_setAutoReplay },
            {L"play", 0, (void*)AnimatedLayer::script_vcpu_play },
            {L"togglePause", 0, (void*)AnimatedLayer::script_vcpu_pause },
            {L"stop", 0, (void*)AnimatedLayer::script_vcpu_stop },
            {L"pause", 0, (void*)AnimatedLayer::script_vcpu_pause },
            {L"isPlaying", 0, (void*)AnimatedLayer::script_vcpu_isPlaying },
            {L"isPaused", 0, (void*)AnimatedLayer::script_vcpu_isPaused },
            {L"isStopped", 0, (void*)AnimatedLayer::script_vcpu_isStopped },
            {L"getStartFrame", 0, (void*)AnimatedLayer::script_vcpu_getStartFrame },
            {L"getEndFrame", 0, (void*)AnimatedLayer::script_vcpu_getEndFrame },
            {L"getLength", 0, (void*)AnimatedLayer::script_vcpu_getLength },
            {L"getDirection", 0, (void*)AnimatedLayer::script_vcpu_getDirection },
            {L"getAutoReplay", 0, (void*)AnimatedLayer::script_vcpu_getAutoReplay },
            {L"getCurFrame", 0, (void*)AnimatedLayer::script_vcpu_getCurFrame },
            {L"onPlay", 0, (void*)AnimatedLayer::script_vcpu_onPlay },
            {L"onPause", 0, (void*)AnimatedLayer::script_vcpu_onPause },
            {L"onResume", 0, (void*)AnimatedLayer::script_vcpu_onResume },
            {L"onStop", 0, (void*)AnimatedLayer::script_vcpu_onStop },
            {L"onFrame", 1, (void*)AnimatedLayer::script_vcpu_onFrame },
            {L"setRealtime", 1, (void*)AnimatedLayer::script_vcpu_setRealtime },
        };
// --------------------------------------------------------

const wchar_t *AnimLayerScriptController::getClassName()
{
	return L"AnimatedLayer";
}

const wchar_t *AnimLayerScriptController::getAncestorClassName()
{
	return L"Layer";
}

ScriptObject *AnimLayerScriptController::instantiate()
{
	AnimatedLayer *a = new AnimatedLayer;
	ASSERT(a != NULL);
	return a->getScriptObject();
}

void AnimLayerScriptController::destroy(ScriptObject *o)
{
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	ASSERT(a != NULL);
	delete a;
}

void *AnimLayerScriptController::encapsulate(ScriptObject *o)
{
	return NULL; // no encapsulation for animatedlayer yet
}

void AnimLayerScriptController::deencapsulate(void *o)
{}

int AnimLayerScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *AnimLayerScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID AnimLayerScriptController::getClassGuid()
{
	return animLayerGuid;
}
XMLParamPair AnimatedLayer::params[] =
    {
        {ANIMLAYER_AUTOPLAY, L"AUTOPLAY"},
        {ANIMLAYER_AUTOREPLAY, L"AUTOREPLAY"},
        {ANIMLAYER_DEBUG, L"DEBUG"},
        {ANIMLAYER_ELEMENTFRAMES, L"ELEMENTFRAMES"},
        {ANIMLAYER_END, L"END"},
        {ANIMLAYER_FRAMEHEIGHT, L"FRAMEHEIGHT"},
        {ANIMLAYER_FRAMEWIDTH, L"FRAMEWIDTH"},
        {ANIMLAYER_REALTIME, L"REALTIME"},
        {ANIMLAYER_SPEED, L"SPEED"},
        {ANIMLAYER_START, L"START"},
    };

AnimatedLayer::AnimatedLayer()
{
	getScriptObject()->vcpu_setInterface(animLayerGuid, (void *)static_cast<AnimatedLayer *>(this));
	getScriptObject()->vcpu_setClassName(L"AnimatedLayer");
	getScriptObject()->vcpu_setController(animlayerController);
	autoplay = 0;
	startframe = -1;
	endframe = -1;
	curframe = 0;
	autoreplay = 1;
	speed = 200;
	timerset = 0;
	status = ANIM_STOPPED;
	realtime = 0;
	debug = 0;
	style = ANIM_UNKNOWN;
	oldstyle = ANIM_UNKNOWN;
	frameHeight = AUTOWH;
	frameWidth = AUTOWH;
	multiple_elements_frames = 0;

	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);	
}

void AnimatedLayer::CreateXMLParameters(int master_handle)
{
	//ANIMLAYER_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

AnimatedLayer::~AnimatedLayer()
{
	bitmap_elements.deleteAll();
	regionlist.deleteAll();
}

int AnimatedLayer::onInit()
{
	ANIMLAYER_PARENT::onInit();

	int w, h;
	getGuiObject()->guiobject_getGuiPosition(NULL, NULL, &w, &h, NULL, NULL, NULL, NULL);
	if (frameWidth == AUTOWH && w != AUTOWH) setWidth(w, 1);
	if (frameHeight == AUTOWH && h != AUTOWH) setHeight(h, 1);
	if (style == 0)
	{
		SkinBitmap *bm = getBitmap();
		if (bm)
		{
			if (bm->getWidth() != w) style = ANIM_HORZ;
			else if (bm->getHeight() != h) style = ANIM_VERT;
		}
	}

	if (getRegionOp()) 
		makeRegion();
	reloadMultipleElements();

	if (autoplay)
	{
		if (startframe == -1)
			setStartFrame(0);
		if (endframe == -1)
			setEndFrame(getLength() - 1);
		play();
	}
	return 1;
}

int AnimatedLayer::setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *strvalue)
{
	if (xuihandle == _xuihandle)
	{
		switch (xmlattributeid)
		{
		case ANIMLAYER_AUTOREPLAY: setAutoReplay(WTOI(strvalue)); return 1;
		case ANIMLAYER_AUTOPLAY: setAutoPlay(WTOI(strvalue)); return 1;
		case ANIMLAYER_SPEED: setSpeed(WTOI(strvalue)); return 1;
		case ANIMLAYER_FRAMEHEIGHT: setHeight(WTOI(strvalue)); return 1;
		case ANIMLAYER_FRAMEWIDTH: setWidth(WTOI(strvalue)); return 1;
		case ANIMLAYER_REALTIME: setRealtime(WTOI(strvalue)); return 1;
		case ANIMLAYER_ELEMENTFRAMES: setElementFrames(WTOI(strvalue)); return 1;
		case ANIMLAYER_START: setStartFrame(WTOI(strvalue)); return 1;
		case ANIMLAYER_END: setEndFrame(WTOI(strvalue)); return 1;
		case ANIMLAYER_DEBUG: debug = WTOI(strvalue); return 1;
		}
	}
	return ANIMLAYER_PARENT::setXuiParam(_xuihandle, xmlattributeid, xmlattributename, strvalue);
}

void AnimatedLayer::_invalidate()
{
	if (realtime)
	{
		if (isVisible() && !isMinimized()) cascadeRepaint();
	}
	else
		invalidate();
}

void AnimatedLayer::setElementFrames(int n)
{
	if (multiple_elements_frames == n) return ;
	multiple_elements_frames = n;
	if (n > 0)
	{
		if (style != ANIM_MULTI)
			oldstyle = style;
		style = ANIM_MULTI;
	}
	else
	{
		style = oldstyle;
		oldstyle = ANIM_UNKNOWN;
	}
	invalidateRegionCache();
}

void AnimatedLayer::setHeight(int h, int selfset)
{
	ASSERTPR(selfset || style == ANIM_UNKNOWN, "can't set frameHeight if frameWidth has already been set");
	frameHeight = h;
	if (!selfset) style = ANIM_VERT;
}

int AnimatedLayer::getHeight()
{
	if (style == ANIM_MULTI)
	{
		SkinBitmap *bm0 = getElementBitmap(0);
		if (bm0 == NULL) return AUTOWH;
		return bm0->getHeight();
	}
	if (style == ANIM_HORZ)
		return ANIMLAYER_PARENT::getHeight();
	return frameHeight;
}

void AnimatedLayer::setWidth(int w, int selfset)
{
	ASSERTPR(selfset || style == ANIM_UNKNOWN, "can't set frameWidth if frameHeight has already been set");
	frameWidth = w;
	if (!selfset) style = ANIM_HORZ;
}

int AnimatedLayer::getWidth()
{
	if (style == ANIM_MULTI)
	{
		SkinBitmap *bm0 = getElementBitmap(0);
		if (bm0 == NULL) return AUTOWH;
		return bm0->getWidth();
	}
	if (style == ANIM_VERT)
		return ANIMLAYER_PARENT::getWidth();
	return frameWidth;
}

void AnimatedLayer::setRealtime(int r)
{
	realtime = r;
}

int AnimatedLayer::getLength()
{
	if (style == ANIM_VERT && frameHeight < 0) return 0;
	if (style == ANIM_HORZ && frameWidth < 0) return 0;
	ASSERT(getBitmap() != NULL);
	if (style == ANIM_VERT)
		return ANIMLAYER_PARENT::getHeight() / frameHeight;
	else if (style == ANIM_HORZ)
		return ANIMLAYER_PARENT::getWidth() / frameWidth;
	else if (style == ANIM_MULTI)
		return multiple_elements_frames;
	return 0;
}

void AnimatedLayer::timerCallback(int id)
{
	switch (id)
	{
	case TIMER_ANIM:
		{
			int oldframe = curframe;
			for (int i = 0;i < timerclient_getSkipped() + 1;i++)
			{
				if (curframe == getEndFrame())
				{
					if (!autoreplay)
					{
						stop();
						break;
					}
					else
						curframe = getStartFrame();
				}
				else
				{
					curframe += getDirection();
					if (curframe != oldframe)
						script_onFrame(curframe);
				}
			}
			if (curframe != oldframe)
				_invalidate();
			break;
		}
	default:
		ANIMLAYER_PARENT::timerCallback(id);
		break;
	}
}

int AnimatedLayer::getSourceOffsetY()
{
	if (style == ANIM_MULTI) return 0;
	if (style == ANIM_HORZ) return 0;
	if (curframe > getLength() - 1) return 0;
	return curframe * getHeight();
}

int AnimatedLayer::getSourceOffsetX()
{
	if (style == ANIM_MULTI) return 0;
	if (style == ANIM_VERT) return 0;
	if (curframe > getLength() - 1) return 0;
	return curframe * getWidth();
}

void AnimatedLayer::setSpeed(int s)
{
	speed = s;
	if (status == ANIM_PLAYING)
	{
		stopTimer();
		startTimer();
	}
}

void AnimatedLayer::stopTimer()
{
	if (timerset)
	{
		killTimer(TIMER_ANIM);
		timerset = 0;
	}
}

void AnimatedLayer::startTimer()
{
	if (!timerset)
	{
		setTimer(TIMER_ANIM, speed);
		timerset = 1;
	}
}

void AnimatedLayer::play()
{
	gotoFrame(startframe);
	startTimer();
	status = ANIM_PLAYING;
	script_onPlay();
}

void AnimatedLayer::stop()
{
	stopTimer();
	status = ANIM_STOPPED;
	script_onStop();
}

void AnimatedLayer::pause()
{
	if (status == ANIM_PAUSED)
	{
		startTimer();
		status = ANIM_PLAYING;
		script_onResume();
	}
	else
		if (status == ANIM_PLAYING)
		{
			stopTimer();
			status = ANIM_PAUSED;
			script_onPause();
		}
}

int AnimatedLayer::getCurFrame()
{
	return curframe;
}

void AnimatedLayer::setStartFrame(int s)
{
	if (s < 0) return ;
	startframe = s;
}

void AnimatedLayer::setEndFrame(int e)
{
	if (e < 0) return ;
	endframe = e;
}

void AnimatedLayer::setAutoReplay(int r)
{
	autoreplay = r;
}

void AnimatedLayer::setAutoPlay(int r)
{
	autoplay = r;
	// no need to trigger an event here, we can't be in a script if we
	// need to autoplay at xml loading
}

int AnimatedLayer::getStartFrame()
{
	return startframe == -1 ? 0 : startframe;
}

int AnimatedLayer::getEndFrame()
{
	return endframe == -1 ? getLength() - 1 : endframe;
}

int AnimatedLayer::getSpeed()
{
	return speed;
}

int AnimatedLayer::isPlaying()
{
	return status == ANIM_PLAYING;
}

int AnimatedLayer::isStopped()
{
	return status == ANIM_STOPPED;
}

int AnimatedLayer::isPaused()
{
	return status == ANIM_PAUSED;
}

int AnimatedLayer::getAutoReplay()
{
	return autoreplay;
}

int AnimatedLayer::getDirection()
{
	return getStartFrame() < getEndFrame() ? 1 : -1;
}

void AnimatedLayer::gotoFrame(int n)
{
	if (n != curframe)
	{
		curframe = n;
		_invalidate();
		script_onFrame(n);
	}
}

api_region *AnimatedLayer::getBitmapRegion()
{
	if (curframe > getLength() - 1) return NULL;
	return regionlist.enumItem(getCurFrame());
}

void AnimatedLayer::makeRegion()
{
	if (!isInited()) return ;
	regionlist.deleteAll();
	for (int i = 0;i < getLength();i++)
	{
		RegionI *rg;
		if (style == ANIM_VERT)
		{
			RECT g = {0, i * getHeight(), getWidth(), i * getHeight() + getHeight()};
			rg = new RegionI(getBitmap(), &g, 0, -i * getHeight(), FALSE);
		}
		else if (style == ANIM_HORZ)
		{
			RECT g = {i * getWidth(), 0, i * getWidth() + getWidth(), getHeight()};
			rg = new RegionI(getBitmap(), &g, -i * getWidth(), 0, FALSE);
		}
		else if (style == ANIM_MULTI)
		{
			RECT g = {0, 0, getWidth(), getHeight()};
			rg = new RegionI(getElementBitmap(i), &g, 0, 0, FALSE);
		}
		else
			return;
		regionlist.addItem(rg);
	}
}

void AnimatedLayer::deleteRegion()
{
	regionlist.deleteAll();
}

SkinBitmap *AnimatedLayer::getBitmap()
{
	if (style != ANIM_MULTI)
		return layer_getBitmap();
	return getElementBitmap(getCurFrame());
}

SkinBitmap *AnimatedLayer::getElementBitmap(int n)
{
	return bitmap_elements.enumItem(n);
}

void AnimatedLayer::reloadMultipleElements()
{
	bitmap_elements.deleteAll();
	if (style != ANIM_MULTI) return ;
	// basically blah$$$$.png becomes blah0000.png, blah0001.png etc
	for (int i = 0;i < multiple_elements_frames;i++)
	{
		StringW elementname(layer_getBitmapName());
		elementname.replaceNumericField(i);
		bitmap_elements.addItem(new SkinBitmap(elementname));
	}
}

void AnimatedLayer::setBitmap(const wchar_t *name)
{
	ANIMLAYER_PARENT::setBitmap(name);
	reloadMultipleElements();
}

// Script virtuals

int AnimatedLayer::script_getStartFrame()
{
	return getStartFrame();
}

int AnimatedLayer::script_getEndFrame()
{
	return getEndFrame();
}

int AnimatedLayer::script_getSpeed()
{
	return getSpeed();
}

int AnimatedLayer::script_getCurFrame()
{
	return getCurFrame();
}

int AnimatedLayer::script_getDirection()
{
	return getDirection();
}

int AnimatedLayer::script_getAutoReplay()
{
	return getAutoReplay();
}

int AnimatedLayer::script_getLength()
{
	return getLength();
}

int AnimatedLayer::script_isPlaying()
{
	return isPlaying();
}

int AnimatedLayer::script_isStopped()
{
	return isStopped();
}

int AnimatedLayer::script_isPaused()
{
	return isPaused();
}

void AnimatedLayer::script_play()
{
	play();
}

void AnimatedLayer::script_pause()
{
	pause();
}

void AnimatedLayer::script_stop()
{
	stop();
}

void AnimatedLayer::script_setStartFrame(int s)
{
	setStartFrame(s);
}

void AnimatedLayer::script_setEndFrame(int e)
{
	setEndFrame(e);
}

void AnimatedLayer::script_setRealtime(int r)
{
	setRealtime(r);
}

void AnimatedLayer::script_setAutoReplay(int r)
{
	setAutoReplay(r);
}
/*
void AnimatedLayer::script_gotoFrame(int n) {
  gotoFrame(n);
}*/

void AnimatedLayer::script_setSpeed(int n)
{
	setSpeed(n);
}

void AnimatedLayer::script_onPause()
{
	script_vcpu_onPause(SCRIPT_CALL, getScriptObject());
}

void AnimatedLayer::script_onResume()
{
	script_vcpu_onResume(SCRIPT_CALL, getScriptObject());
}

void AnimatedLayer::script_onStop()
{
	script_vcpu_onStop(SCRIPT_CALL, getScriptObject());
}

void AnimatedLayer::script_onPlay()
{
	script_vcpu_onPlay(SCRIPT_CALL, getScriptObject());
}

void AnimatedLayer::script_onFrame(int n)
{
	if (getRegionOp()) { invalidateRegionCache(); getParent()->invalidateWindowRegion(); }
	scriptVar _n = SOM::makeVar(SCRIPT_INT);
	SOM::assign(&_n, n);
	script_vcpu_onFrame(SCRIPT_CALL, getScriptObject(), _n);
}

// end virtuals

// VCPU

scriptVar AnimatedLayer::script_vcpu_gotoFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar f)
{
	SCRIPT_FUNCTION_INIT;
	ASSERT(SOM::isNumeric(&f));
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) a-> /*script_*/gotoFrame(SOM::makeInt(&f));
	RETURN_SCRIPT_VOID;
}

scriptVar AnimatedLayer::script_vcpu_getLength(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) return MAKE_SCRIPT_INT(a->script_getLength());
	RETURN_SCRIPT_ZERO;
}

scriptVar AnimatedLayer::script_vcpu_setStartFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s)
{
	SCRIPT_FUNCTION_INIT;
	ASSERT(SOM::isNumeric(&s));
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) a->script_setStartFrame(SOM::makeInt(&s));
	RETURN_SCRIPT_VOID;
}

scriptVar AnimatedLayer::script_vcpu_setEndFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar e)
{
	SCRIPT_FUNCTION_INIT;
	ASSERT(SOM::isNumeric(&e));
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) a->script_setEndFrame(SOM::makeInt(&e));
	RETURN_SCRIPT_VOID;
}

scriptVar AnimatedLayer::script_vcpu_setAutoReplay(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar rp)
{
	SCRIPT_FUNCTION_INIT;
	ASSERT(SOM::isNumeric(&rp));
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) a->script_setAutoReplay(SOM::makeBoolean(&rp));
	RETURN_SCRIPT_VOID;
}

scriptVar AnimatedLayer::script_vcpu_play(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) a->script_play();
	RETURN_SCRIPT_VOID;
}

scriptVar AnimatedLayer::script_vcpu_pause(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) a->script_pause();
	RETURN_SCRIPT_VOID;
}

scriptVar AnimatedLayer::script_vcpu_stop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) a->script_stop();
	RETURN_SCRIPT_VOID;
}

scriptVar AnimatedLayer::script_vcpu_onPlay(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, animlayerController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar AnimatedLayer::script_vcpu_onStop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, animlayerController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar AnimatedLayer::script_vcpu_onPause(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, animlayerController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar AnimatedLayer::script_vcpu_onResume(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, animlayerController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar AnimatedLayer::script_vcpu_onFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar f)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, animlayerController, f);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, f);
}

scriptVar AnimatedLayer::script_vcpu_getAutoReplay(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) return MAKE_SCRIPT_INT(a->script_getAutoReplay());
	RETURN_SCRIPT_ZERO;
}

scriptVar AnimatedLayer::script_vcpu_getDirection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) return MAKE_SCRIPT_INT(a->script_getDirection());
	RETURN_SCRIPT_ZERO;
}

scriptVar AnimatedLayer::script_vcpu_getStartFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) return MAKE_SCRIPT_INT(a->script_getStartFrame());
	RETURN_SCRIPT_ZERO;
}

scriptVar AnimatedLayer::script_vcpu_getEndFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) return MAKE_SCRIPT_INT(a->script_getEndFrame());
	RETURN_SCRIPT_ZERO;
}

scriptVar AnimatedLayer::script_vcpu_isPlaying(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) return MAKE_SCRIPT_BOOLEAN(a->script_isPlaying());
	RETURN_SCRIPT_ZERO;
}

scriptVar AnimatedLayer::script_vcpu_isPaused(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) return MAKE_SCRIPT_BOOLEAN(a->script_isPaused());
	RETURN_SCRIPT_ZERO;
}

scriptVar AnimatedLayer::script_vcpu_isStopped(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) return MAKE_SCRIPT_BOOLEAN(a->script_isStopped());
	RETURN_SCRIPT_ZERO;
}

scriptVar AnimatedLayer::script_vcpu_getSpeed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) return MAKE_SCRIPT_INT(a->script_getSpeed());
	RETURN_SCRIPT_ZERO;
}

scriptVar AnimatedLayer::script_vcpu_getCurFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) return MAKE_SCRIPT_INT(a->script_getCurFrame());
	RETURN_SCRIPT_ZERO;
}

scriptVar AnimatedLayer::script_vcpu_setSpeed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s)
{
	SCRIPT_FUNCTION_INIT;
	ASSERT(SOM::isNumeric(&s));
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) a->script_setSpeed(SOM::makeInt(&s));
	RETURN_SCRIPT_VOID;
}

scriptVar AnimatedLayer::script_vcpu_setRealtime(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r)
{
	SCRIPT_FUNCTION_INIT;
	AnimatedLayer *a = static_cast<AnimatedLayer *>(o->vcpu_getInterface(animLayerGuid));
	if (a) a->script_setRealtime(SOM::makeInt(&r));
	RETURN_SCRIPT_VOID;
}


int AnimatedLayer::onPaint(Canvas *canvas)
{
	int r = ANIMLAYER_PARENT::onPaint(canvas);
	if (debug && canvas != NULL)
	{
		Wasabi::FontInfo fontInfo;
		fontInfo.pointSize = 14;
		canvas->textOut(0, 0, StringPrintfW(L"%d", curframe), &fontInfo);
	}
	return r;
}
