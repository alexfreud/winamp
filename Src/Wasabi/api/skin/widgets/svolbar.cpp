#include <precomp.h>
#include "svolbar.h"
#include <api/core/api_core.h>

const wchar_t volBarXuiStr[] = L"VolBar"; // This is the xml tag
char volBarXuiSvcName[] = "VolBar xui object"; // this is the name of the xuiservice


#define SVOLBAR_NOTIFY_MSG NUM_NOTIFY_MESSAGES+0x1000

SVolBar::SVolBar() {
	setDrawOnBorders(TRUE);
	setEnable(TRUE);
	locked = 0;
	setLimits(0,255);
}

SVolBar::~SVolBar() {
	WASABI_API_MEDIACORE->core_delCallback(0, this);
}

int SVolBar::onInit() {
	SVOLBAR_PARENT::onInit();

	corecb_onVolumeChange(WASABI_API_MEDIACORE->core_getVolume(0));
	WASABI_API_MEDIACORE->core_addCallback(0, this);

	return 1;
}

int SVolBar::onSetPosition() {
	SVOLBAR_PARENT::onSetPosition();
	int pos = getSliderPosition();	// get slider pos
	WASABI_API_MEDIACORE->core_setVolume(0,pos);
	return 1;
}

int SVolBar::corecb_onVolumeChange(int newvol) {
	if (getSeekStatus()) return 0;
	setPosition(newvol, 0);
	onPostedPosition(newvol);
	return 0;
}

void SVolBar::lock() {
	if (locked) return;
	locked = 1;
	WASABI_API_MEDIACORE->core_delCallback(0, this);
}

void SVolBar::unlock() {
	if (!locked) return;
	locked = 0;
	WASABI_API_MEDIACORE->core_addCallback(0, this);
}