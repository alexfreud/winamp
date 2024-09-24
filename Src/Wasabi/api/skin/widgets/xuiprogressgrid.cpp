#include <precomp.h>
#include "xuiprogressgrid.h"
#include <tataki/canvas/ifc_canvas.h>
#include <bfc/string/stringdict.h>
#include <api/core/api_core.h>

#define ProgressGrid_TIMER_POS 1
#define PROGRESSGRID_INTERVAL 500

#ifndef _WASABIRUNTIME

BEGIN_SERVICES(ProgressGrid_Svc);
DECLARE_SERVICE(XuiObjectCreator<ProgressGridXuiSvc>);
END_SERVICES(ProgressGrid_Svc, _ProgressGrid_Svc);

#ifdef _X86_
extern "C" { int _link_ProgressGridXuiSvc; }
#else
extern "C" { int __link_ProgressGridXuiSvc; }
#endif

#endif

BEGIN_STRINGDICTIONARY(_pgorientationvalues)
SDI(L"top", PROGRESSGRID_TOP);
SDI(L"left", PROGRESSGRID_LEFT);
SDI(L"right", PROGRESSGRID_RIGHT);
SDI(L"bottom", PROGRESSGRID_BOTTOM);
END_STRINGDICTIONARY(_pgorientationvalues, pgorientationvalues)

// -----------------------------------------------------------------------

const wchar_t ProgressGridXuiObjectStr[] = L"ProgressGrid"; // xml tag
char ProgressGridXuiSvcName[] = "ProgressGrid xui object"; 
XMLParamPair ProgressGrid::params[] = {
	{ PROGRESSGRID_SETORIENTATION, L"ORIENTATION"},
	{ PROGRESSGRID_SETINTERVAL, L"INTERVAL"},
};

// -----------------------------------------------------------------------
ProgressGrid::ProgressGrid() {
	myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);

	orientation = PROGRESSGRID_RIGHT;
	progress = 0;
	started = 0;
	update_interval = PROGRESSGRID_INTERVAL;
}

void ProgressGrid::CreateXMLParameters(int master_handle)
{
	//PROGRESSGRID_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

// -----------------------------------------------------------------------
ProgressGrid::~ProgressGrid() {
	killTimer(ProgressGrid_TIMER_POS);
	WASABI_API_MEDIACORE->core_delCallback(0, this);
}

// -----------------------------------------------------------------------
int ProgressGrid::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
	if (xuihandle != myxuihandle)
		return PROGRESSGRID_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

	switch (xmlattributeid) {
		case PROGRESSGRID_SETORIENTATION:
			setOrientation(value);
		break;

		case PROGRESSGRID_SETINTERVAL:
			if ((update_interval = WTOI(value)) <= 20)
				update_interval = PROGRESSGRID_INTERVAL;
		break;

		default:
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
void ProgressGrid::setOrientation(const wchar_t *or) {
	int a = pgorientationvalues.getId(or);
	if (orientation == a) return;
	orientation = a;
	invalidate();
}

// -----------------------------------------------------------------------
void ProgressGrid::getGridRect(RECT *r) {
	RECT cr;
	getClientRect(&cr);
	float p = started ? progress : 0.0f;
	int height = (int)((float)(cr.bottom - cr.top) * p);
	int width = (int)((float)(cr.right - cr.left) * p);

	switch (orientation) {
		case PROGRESSGRID_LEFT: cr.left = cr.right - width; break;
		case PROGRESSGRID_TOP:  cr.top = cr.bottom - height; break;
		case PROGRESSGRID_RIGHT: cr.right = cr.left + width; break;
		case PROGRESSGRID_BOTTOM: cr.bottom = cr.top + height; break;
	}
	*r = cr;
}

// -----------------------------------------------------------------------
int ProgressGrid::onInit() {
	PROGRESSGRID_PARENT::onInit();
	timerCallback(ProgressGrid_TIMER_POS);
	setTimer(ProgressGrid_TIMER_POS, update_interval);
	WASABI_API_MEDIACORE->core_addCallback(0, this);
	if (WASABI_API_MEDIACORE->core_getStatus(0) != 0) started = 1; else started = 0;
	return 1;
}

// -----------------------------------------------------------------------
int ProgressGrid::corecb_onSeeked(int newpos) {
	if(!started) corecb_onStarted();
	int len = WASABI_API_MEDIACORE->core_getLength(0);
	if (newpos == -1 || len <= 0) setProgress(0);
	else setProgress(((float)newpos/(float)len));
	return 0;
}

// -----------------------------------------------------------------------
int ProgressGrid::corecb_onStarted() {
	started = 1;
	invalidate();
	return 0;
}

// -----------------------------------------------------------------------
int ProgressGrid::corecb_onStopped() {
	started = 0;
	progress = 0.0f;
	invalidate();
	return 0;
}

void ProgressGrid::setProgress(float p) {
	if (progress == p) return;
	progress = p;
	invalidate();
}

void ProgressGrid::timerCallback(int id) {
	switch (id) {
		case ProgressGrid_TIMER_POS: {
			int playpos = WASABI_API_MEDIACORE->core_getPosition(0);
			corecb_onSeeked(playpos);
		}
		break;
		default:
			PROGRESSGRID_PARENT::timerCallback(id);
	}
}