#include <precomp.h>
#include "seqband.h"
#include <api/core/api_core.h>

#define NOTIFYMSG_EQ_HELLO 0x3003

const wchar_t eqBandXuiStr[] = L"EqBand"; // This is the xml tag
char eqBandXuiSvcName[] = "EqBand xui object"; // this is the name of the xuiservice

XMLParamPair SEQBand::params[] = {
	{SEQBAND_SETPARAM, L"BAND"},
	{SEQBAND_SETPARAM, L"PARAM"},
};

SEQBand::SEQBand() {
	band = 0;
	setDrawOnBorders(TRUE);
	setEnable(TRUE);
	setHotPosition(0);
	isactive=0;
	discard_next_event = 0;
	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);

	setLimits(-127,127);
}

void SEQBand::CreateXMLParameters(int master_handle)
{
	//SEQBAND_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

SEQBand::~SEQBand() {
	WASABI_API_MEDIACORE->core_delCallback(0, this);
}

int SEQBand::setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval) {
	if (_xuihandle != xuihandle) return SEQBAND_PARENT::setXuiParam(_xuihandle, attrid, name, strval);
	switch (attrid) {
		case SEQBAND_SETPARAM:
		case SEQBAND_SETBAND:
			setBand(WTOI(strval)-1);
			break;
		default:
			return 0;
	}
	return 1;
}

void SEQBand::setBand(int b) {
	band = b;
}

int SEQBand::onInit() {
	SEQBAND_PARENT::onInit();

	corecb_onEQBandChange(band, WASABI_API_MEDIACORE->core_getEqBand(0, band));
	WASABI_API_MEDIACORE->core_addCallback(0, this);

	return 1;
}

int SEQBand::onSetPosition() {
	setHotPosition((Std::keyDown(VK_SHIFT) ? -1 : 0));
	SEQBAND_PARENT::onSetPosition();
	int pos = getSliderPosition();	// get slider pos
	discard_next_event = 1;
	WASABI_API_MEDIACORE->core_setEqBand(0,band,pos);
	return 1;
}

int SEQBand::onResize() {
	SEQBAND_PARENT::onResize();
	invalidate();
	return 1;
}

int SEQBand::corecb_onEQBandChange(int b, int newval) {
	if(b!=band) return 0;
	if (discard_next_event) {
		discard_next_event = 0;
		return 0;
	}
	setPosition(newval,0);
	return 0;
}

int SEQBand::onLeftButtonDown(int x, int y) { 
	isactive=1; 
	return SEQBAND_PARENT::onLeftButtonDown(x,y); 
}

int SEQBand::onMouseMove(int x, int y) {
	if(isactive) {
		ifc_window *parent=getParent();
		if(parent) {
			ifc_window *wnd=parent->findRootWndChild(x,y);
			if(wnd && wnd!=this) {
				if(wnd->childNotify(this,NOTIFYMSG_EQ_HELLO,0,0)) { // will return 1 if it's another EQBand
					onLeftButtonUp(x,y);
					wnd->onLeftButtonDown(x,y);
				}
			}
		}
	}
	return SEQBAND_PARENT::onMouseMove(x,y);
}

int SEQBand::onLeftButtonUp(int x, int y) { 
	isactive=0; 
	return SEQBAND_PARENT::onLeftButtonUp(x,y); 
}

int SEQBand::childNotify(ifc_window *child, int msg, intptr_t param1, intptr_t param2) {
	if(msg==NOTIFYMSG_EQ_HELLO) return 1;
	return SEQBAND_PARENT::childNotify(child,msg,param1,param2);
}