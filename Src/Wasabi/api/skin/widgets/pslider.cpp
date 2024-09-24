#include <precomp.h>
#include <wasabicfg.h>
#include "pslider.h"
#include <api/script/scriptmgr.h>
#include <api/script/vcpu.h>
#include <api/skin/skinparse.h>

#ifdef WASABI_WIDGETS_MEDIASLIDERS
#include "seqband.h"
#include "seqpreamp.h"
#include "svolbar.h"
#include "sseeker.h"
#include "spanbar.h"
#endif

char sliderObjectStr[] = "Slider"; // This is the xml tag
char sliderXuiSvcName[] = "Slider xui object"; // this is the name of the xuiservice
XMLParamPair PSliderWnd::params[] = 
{
                                        {PSLIDER_SETBARLEFT, L"BARLEFT"},
                                        {PSLIDER_SETBARMIDDLE, L"BARMIDDLE"},
                                        {PSLIDER_SETBARRIGHT, L"BARRIGHT"},
                                        {PSLIDER_SETDOWNTHUMB, L"DOWNTHUMB"},
                                        {PSLIDER_SETHIGH, L"HIGH"},
                                        {PSLIDER_SETHOTPOS, L"HOTPOS"},
                                        {PSLIDER_SETHOTRANGE, L"HOTRANGE"},
                                        {PSLIDER_SETHOVERTHUMB, L"HOVERTHUMB"},
                                        {PSLIDER_SETLOW, L"LOW"},
                                        {PSLIDER_SETORIENTATION, L"ORIENTATION"},
                                        {PSLIDER_SETTHUMB, L"THUMB"},
										{PSLIDER_SETSTRETCHTHUMB, L"STRETCHTHUMB"},
                                    };

PSliderWnd::PSliderWnd()
{
	setLimits(0, 255);
	getScriptObject()->vcpu_setInterface(sliderGuid, (void *)static_cast<PSliderWnd *>(this));
	getScriptObject()->vcpu_setClassName(L"Slider");
	getScriptObject()->vcpu_setController(sliderController);

	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);	
}

void PSliderWnd::CreateXMLParameters(int master_handle)
{
	//PSLIDER_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

PSliderWnd::~PSliderWnd()
{}

int PSliderWnd::setXuiParam(int _xuihandle, int attrid, const wchar_t *paramname, const wchar_t *strvalue)
{
	if (xuihandle != _xuihandle) return PSLIDER_PARENT::setXuiParam(_xuihandle, attrid, paramname, strvalue);
	switch (attrid)
	{
	case PSLIDER_SETBARLEFT:
		setLeftBmp(strvalue);
		break;
	case PSLIDER_SETBARMIDDLE:
		setMiddleBmp(strvalue);
		break;
	case PSLIDER_SETBARRIGHT:
		setRightBmp(strvalue);
		break;
	case PSLIDER_SETTHUMB:
		setThumbBmp(strvalue);
		break;
	case PSLIDER_SETDOWNTHUMB:
		setThumbDownBmp(strvalue);
		break;
	case PSLIDER_SETHOVERTHUMB:
		setThumbHiliteBmp(strvalue);
		break;
	case PSLIDER_SETSTRETCHTHUMB:
		setThumbStretched(_wtoi(strvalue));
		break;
	case PSLIDER_SETORIENTATION:
		setOrientation(SkinParser::getOrientation(strvalue));
		break;
	case PSLIDER_SETLOW:
		{
			int mx = getMaxLimit();
			setLimits(WTOI(strvalue), mx);
#ifdef WASABI_COMPILE_CONFIG
			reloadConfig();
#endif
			break;
		}
	case PSLIDER_SETHIGH:
		{
			int mn = getMinLimit();
			setLimits(mn, WTOI(strvalue));
#ifdef WASABI_COMPILE_CONFIG
			reloadConfig();
#endif
			break;
		}
	case PSLIDER_SETHOTPOS:
		{
			int a = WTOI(strvalue);
			setHotPosition(a);
			break;
		}
	case PSLIDER_SETHOTRANGE:
		{
			int a = WTOI(strvalue);
			setHotPosRange(a);
			break;
		}
	default:
		return 0;
	}
	return 1;
}

int PSliderWnd::onInit()
{
	setNoDefaultBackground(1);
	PSLIDER_PARENT::onInit();
	return 1;
}

#ifdef WASABI_COMPILE_CONFIG
void PSliderWnd::reloadConfig()
{
	if (getGuiObject()->guiobject_hasCfgAttrib())
		onReloadConfig();
}
#endif

#ifdef WASABI_COMPILE_CONFIG
int PSliderWnd::onReloadConfig()
{
	int newVal = getGuiObject()->guiobject_getCfgInt();
	setPosition(newVal, 0);

	return PSLIDER_PARENT::onReloadConfig();
}
#endif

int PSliderWnd::onSetPosition()
{
	int r = PSLIDER_PARENT::onSetPosition();
	scriptVar p = SOM::makeVar(SCRIPT_INT);
	int intVal = getSliderPosition();
#ifdef WASABI_COMPILE_CONFIG
	getGuiObject()->guiobject_setCfgInt(intVal);
#endif
	SOM::assign(&p, intVal / scriptDivisor());
	script_onSetPosition(SCRIPT_CALL, getScriptObject(), p);
	return r;
}

int PSliderWnd::onPostedPosition(int pp)
{
	scriptVar p = SOM::makeVar(SCRIPT_INT);
	int intVal = getSliderPosition();
#ifdef WASABI_COMPILE_CONFIG
	getGuiObject()->guiobject_setCfgInt(intVal);
#endif
	SOM::assign(&p, intVal / scriptDivisor());
	script_onPostedPosition(SCRIPT_CALL, getScriptObject(), p);
	return 1;
}

int PSliderWnd::onSetFinalPosition()
{
	int r = PSLIDER_PARENT::onSetPosition();
	scriptVar p = SOM::makeVar(SCRIPT_INT);
	int intVal = getSliderPosition();
#ifdef WASABI_COMPILE_CONFIG
	getGuiObject()->guiobject_setCfgInt(intVal);
#endif
	SOM::assign(&p, intVal / scriptDivisor());
	script_onSetFinalPosition(SCRIPT_CALL, getScriptObject(), p);
	return r;
}

SliderScriptController _sliderController;
SliderScriptController *sliderController = &_sliderController;

// -- Functions table -------------------------------------
function_descriptor_struct SliderScriptController::exportedFunction[] = {
            {L"setPosition", 1, (void*)PSliderWnd::script_setPosition },
            {L"getPosition", 0, (void*)PSliderWnd::script_getPosition },
            {L"onSetPosition", 1, (void*)PSliderWnd::script_onSetPosition },
            {L"onPostedPosition", 1, (void*)PSliderWnd::script_onPostedPosition },
            {L"onSetFinalPosition", 1, (void*)PSliderWnd::script_onSetFinalPosition },
            {L"lock", 0, (void*)PSliderWnd::script_lock},
            {L"unlock", 0, (void*)PSliderWnd::script_unlock},
        };
// --------------------------------------------------------

const wchar_t *SliderScriptController::getClassName()
{
	return L"Slider";
}

const wchar_t *SliderScriptController::getAncestorClassName()
{
	return L"GuiObject";
}

ScriptObject *SliderScriptController::instantiate()
{
	PSliderWnd *s = new PSliderWnd;
	ASSERT(s != NULL);
	return s->getScriptObject();
}

void SliderScriptController::destroy(ScriptObject *o)
{
	PSliderWnd *s = static_cast<PSliderWnd *>(o->vcpu_getInterface(sliderGuid));
	ASSERT(s != NULL);
	delete s;
}

void *SliderScriptController::encapsulate(ScriptObject *o)
{
	return NULL; // no encapsulation for sliders yet
}

void SliderScriptController::deencapsulate(void *o)
{}

int SliderScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *SliderScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID SliderScriptController::getClassGuid()
{
	return sliderGuid;
}


const wchar_t *PSliderWnd::vcpu_getClassName()
{
	return L"Slider";
}

void PSliderWnd::lock ()
{}

void PSliderWnd::unlock()
{}

//------------------------------------------------------------------------

scriptVar PSliderWnd::script_onSetPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar p)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, sliderController, p);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, p);
}

scriptVar PSliderWnd::script_onPostedPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar p)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, sliderController, p);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, p);
}

scriptVar PSliderWnd::script_onSetFinalPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar p)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, sliderController, p);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, p);
}

scriptVar PSliderWnd::script_setPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&v));
	PSliderWnd *s = static_cast<PSliderWnd *>(o->vcpu_getInterface(sliderGuid));
	if (s) s->setPosition(GET_SCRIPT_INT(v) * s->scriptDivisor());
	RETURN_SCRIPT_VOID;
}

scriptVar PSliderWnd::script_getPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	PSliderWnd *s = static_cast<PSliderWnd *>(o->vcpu_getInterface(sliderGuid));
	if (s) return MAKE_SCRIPT_INT(s->getSliderPosition() / s->scriptDivisor());
	return MAKE_SCRIPT_FLOAT(0);
}

scriptVar PSliderWnd::script_lock(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	PSliderWnd *s = static_cast<PSliderWnd *>(o->vcpu_getInterface(sliderGuid));
	if (s) s->lock ();
	RETURN_SCRIPT_VOID;
}

scriptVar PSliderWnd::script_unlock(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	PSliderWnd *s = static_cast<PSliderWnd *>(o->vcpu_getInterface(sliderGuid));
	if (s) s->unlock();
	RETURN_SCRIPT_VOID;
}

GuiObject *SliderXuiSvc::instantiate(const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
#ifdef WASABI_WIDGETS_MEDIASLIDERS
	if (!params)
	{
		PSliderWnd *r = new PSliderWnd;
		return r->getGuiObject();
	}
	const wchar_t *action = params->getItemValue(L"action");
	const wchar_t *param = params->getItemValue(L"param");
	if (!action)
		action=L"";
	PSliderWnd *r = NULL;
#ifdef WASABI_WIDGETS_MEDIASLIDERS
	if (!_wcsicmp(action, L"seek"))
		r = new SSeeker;
	else if (!_wcsicmp(action, L"volume"))
		r = new SVolBar;
	else if (!_wcsicmp(action, L"pan"))
		r = new SPanBar;
	else if (!_wcsicmp(action, L"eq_band"))
	{
		if (!_wcsicmp(param, L"preamp"))
			r = new SEQPreamp;
		else
			r = new SEQBand;
	}
	else if (!_wcsicmp(action, L"eq_preamp"))
	{
		r = new SEQPreamp;
	}
	else
	{
#endif

		r = new PSliderWnd;

#ifdef WASABI_WIDGETS_MEDIASLIDERS

	}
#endif

	return r->getGuiObject();
#else
	PSliderWnd *r = new PSliderWnd;
	return r->getGuiObject();
#endif
}

void SliderXuiSvc::destroy(GuiObject *g)
{
	PSliderWnd *obj = static_cast<PSliderWnd *>(g->guiobject_getRootWnd());
	delete obj;
}
