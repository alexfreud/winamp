#ifndef _SEQPREAMP_H
#define _SEQPREAMP_H

#include "pslider.h"
#include <api/syscb/callbacks/corecbi.h>

#define SEQPREAMP_PARENT PSliderWnd
#define SEQPREAMP_XMLPARENT PSliderWnd

class SEQPreamp : public SEQPREAMP_PARENT, public CoreCallbackI {
public:
	SEQPreamp();
	virtual ~SEQPreamp();

	virtual int onInit();

protected:
	virtual int onSetPosition();

	virtual int corecb_onEQPreampChange(int newval);
	int discard_next_event;
};

extern const wchar_t eqPreAmpXuiStr[];
extern char eqPreAmpXuiSvcName[];
class EqPreAmpXuiSvc : public XuiObjectSvc<SEQPreamp, eqPreAmpXuiStr, eqPreAmpXuiSvcName> {};

#endif