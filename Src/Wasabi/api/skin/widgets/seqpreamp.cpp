#include <precomp.h>
#include "seqpreamp.h"

#include <api/core/api_core.h>

const wchar_t eqPreAmpXuiStr[] = L"EQPreAmp"; // This is the xml tag
char eqPreAmpXuiSvcName[] = "EQPreAmp xui object"; // this is the name of the xuiservice

SEQPreamp::SEQPreamp() {
	setDrawOnBorders(TRUE);
	setEnable(TRUE);
	setHotPosition(0);
	setLimits(-127,127);
	discard_next_event = 0;
}

SEQPreamp::~SEQPreamp() {
	WASABI_API_MEDIACORE->core_delCallback(0, this);
}

int SEQPreamp::onInit() {
	SEQPREAMP_PARENT::onInit();

	corecb_onEQPreampChange(WASABI_API_MEDIACORE->core_getEqPreamp(0));
	WASABI_API_MEDIACORE->core_addCallback(0, this);

	return 1;
}

int SEQPreamp::onSetPosition() {
	setHotPosition((Std::keyDown(VK_SHIFT) ? -1 : 0));
	SEQPREAMP_PARENT::onSetPosition();
	int pos = getSliderPosition();	// get slider pos
	discard_next_event = 1;
	WASABI_API_MEDIACORE->core_setEqPreamp(0,pos);
	discard_next_event = 0;
	return 1;
}

int SEQPreamp::corecb_onEQPreampChange(int newval) {
	if (discard_next_event) return 0;
	setPosition(newval,0);
	return 0;
}