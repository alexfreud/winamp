#ifndef _SEQBAND_H
#define _SEQBAND_H

#include "pslider.h"
#include <api/syscb/callbacks/corecbi.h>

#define SEQBAND_PARENT PSliderWnd
#define SEQBAND_XMLPARENT PSliderWnd

class SEQBand : public SEQBAND_PARENT, public CoreCallbackI {
public:
	SEQBand();  // band=0-9
	virtual ~SEQBand();

	virtual int onInit();
	virtual int onResize();
	virtual int setXuiParam(int xuihandle, int attrid, const wchar_t *name, const wchar_t *strval);
	virtual void setBand(int b);

	virtual int onLeftButtonDown(int x, int y);
	virtual int onMouseMove(int x, int y);	// only called when mouse captured
	virtual int onLeftButtonUp(int x, int y);
	virtual int childNotify(ifc_window *child, int msg, intptr_t param1=0, intptr_t param2=0);

	enum {
		SEQBAND_SETPARAM=0,
		SEQBAND_SETBAND,
	};
  
protected:
	/*static */void CreateXMLParameters(int master_handle);
	virtual int onSetPosition();

	virtual int corecb_onEQBandChange(int band, int newval);

	int band, isactive;
	int discard_next_event;
private:
	static XMLParamPair params[];
	int xuihandle;
};

extern const wchar_t eqBandXuiStr[];
extern char eqBandXuiSvcName[];
class EqBandXuiSvc : public XuiObjectSvc<SEQBand, eqBandXuiStr, eqBandXuiSvcName> {};

#endif