#ifndef _XUIWA2SLIDER_H
#define _XUIWA2SLIDER_H

#include <api/script/objects/guiobj.h>
#include <api/skin/widgets.h>
#include <api/syscb/callbacks/corecbi.h>

#define WA2SLIDER_PARENT GuiObjectWnd

class Wa2Slider : public WA2SLIDER_PARENT, public CoreCallbackI
{
public:
	Wa2Slider();
	virtual ~Wa2Slider();

	virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

	virtual int onInit();
	virtual int onPaint(Canvas *canvas);

	virtual int corecb_onVolumeChange(int newvol);
	virtual int corecb_onPanChange(int newpan);
	virtual int corecb_onSeeked(int newpos);
	virtual int corecb_onStarted();
	virtual int corecb_onStopped();
	virtual void timerCallback(int id);

protected:
	/*static */void CreateXMLParameters(int master_handle);
private:
	int realpos;
	StringW images;
	SkinBitmap *imagesBitmap;
	int spacing;
	int action;
	static XMLParamPair params[];
	int xuihandle;
	bool started;

	enum {
	    ACT_NONE = 0,
	    ACT_VOLUME,
	    ACT_BALANCE,
	    ACT_SEEK,
	};

	enum {
	    Wa2Slider_TIMER_POS = 1,
	};
	enum {
	    WA2SLIDER_IMAGES,
	    WA2SLIDER_IMAGESSPACING,
	    WA2SLIDER_SOURCE,
	    WA2SLIDER_NUMPARAMS,
	};
};

// -----------------------------------------------------------------------
extern const wchar_t Wa2SliderXuiObjectStr[];
extern char Wa2SliderXuiSvcName[];
class Wa2SliderXuiSvc : public XuiObjectSvc<Wa2Slider, Wa2SliderXuiObjectStr, Wa2SliderXuiSvcName> {};

#endif
