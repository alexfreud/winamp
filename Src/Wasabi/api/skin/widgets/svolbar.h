#ifndef _SVOLBAR_H
#define _SVOLBAR_H

#include "pslider.h"
#include <api/syscb/callbacks/corecbi.h>

#define SVOLBAR_PARENT PSliderWnd
#define SVOLBAR_XMLPARENT PSliderWnd

class SVolBar : public SVOLBAR_PARENT, public CoreCallbackI {
public:
	SVolBar();
	virtual ~SVolBar();

	virtual int onInit();
	virtual void lock();
	virtual void unlock();

protected:
	int locked;
	virtual int onSetPosition();

	virtual int corecb_onVolumeChange(int newvol);
};

extern const wchar_t volBarXuiStr[];
extern char volBarXuiSvcName[];
class VolBarXuiSvc : public XuiObjectSvc<SVolBar, volBarXuiStr, volBarXuiSvcName> {};

#endif