#include <precomp.h>

#include "xuiwa2slider.h"
#include <tataki/canvas/canvas.h>
#include <tataki/bitmap/bitmap.h>
#include <api/core/api_core.h>

#define WA2SLIDER_SEEK_INTERVAL 500

const wchar_t Wa2SliderXuiObjectStr[] = L"images"; // This is the xml tag
char Wa2SliderXuiSvcName[] = "Images XuiObject Service";

XMLParamPair Wa2Slider::params[] =
    {
        {WA2SLIDER_IMAGES, L"IMAGES"},
        {WA2SLIDER_IMAGESSPACING, L"IMAGESSPACING"},
        {WA2SLIDER_SOURCE, L"SOURCE"},
    };

Wa2Slider::Wa2Slider()
		: started(false)
{
	realpos = 0;
	imagesBitmap = 0;
	spacing = 0;
	action = ACT_NONE;

	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);	
}

void Wa2Slider::CreateXMLParameters(int master_handle)
{
	//WA2SLIDER_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

Wa2Slider::~Wa2Slider()
{
	killTimer(Wa2Slider_TIMER_POS);
	WASABI_API_MEDIACORE->core_delCallback(0, this);
	delete(imagesBitmap);
}

int Wa2Slider::setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value)
{
	if (xuihandle == _xuihandle)
	{
		switch (xmlattributeid)
		{
		case WA2SLIDER_IMAGES: images = value; return 1;
		case WA2SLIDER_IMAGESSPACING: spacing = WTOI(value); return 1;
		case WA2SLIDER_SOURCE:
			if (!_wcsicmp(value, L"volume")) action = ACT_VOLUME;
			if (!_wcsicmp(value, L"balance")) action = ACT_BALANCE;
			if (!_wcsicmp(value, L"seek")) action = ACT_SEEK;
			return 1;
		}
	}
	return WA2SLIDER_PARENT::setXuiParam(_xuihandle, xmlattributeid, xmlattributename, value);
}

int Wa2Slider::onInit()
{
	WA2SLIDER_PARENT::onInit();

	imagesBitmap = new SkinBitmap(images);

	if (action == ACT_VOLUME)
		corecb_onVolumeChange(WASABI_API_MEDIACORE->core_getVolume(0));
	if (action == ACT_BALANCE)
		corecb_onPanChange(WASABI_API_MEDIACORE->core_getPan(0));
	if (action == ACT_SEEK)
	{
		corecb_onSeeked(WASABI_API_MEDIACORE->core_getPosition(0));
		started = (WASABI_API_MEDIACORE->core_getStatus(0) != 0);
		setTimer(Wa2Slider_TIMER_POS, WA2SLIDER_SEEK_INTERVAL);
	}

	WASABI_API_MEDIACORE->core_addCallback(0, this);

	return 0;
}

int Wa2Slider::onPaint(Canvas *canvas)
{
	if (imagesBitmap->getHeight() && spacing)
	{
		RECT r, r2;
		getClientRect(&r2);
		int nb = (imagesBitmap->getHeight() / spacing) - 1;
		int which = 0;
		switch (action)
		{
		case ACT_BALANCE:
			{
				int p = realpos;
				int f = 32768; //FULL/2;
				if (p > f)
				{
					which = (realpos - f) * nb / f;
				}
				else if (p < f)
				{
					which = (f - realpos) * nb / f;
				}
				else which = 0;
			}
			break;
		case ACT_SEEK:
			if (!started)
				which = 0;
			else
				which = realpos * nb / 65536;
			break;
		case ACT_VOLUME:
			which = realpos * nb / 65536;
			break;
		}

		r.left = 0;
		r.top = which * spacing;
		r.bottom = which * spacing + (r2.bottom - r2.top);
		r.right = r2.right - r2.left;
		imagesBitmap->blitToRect(canvas, &r, &r2);
	}
	return WA2SLIDER_PARENT::onPaint(canvas);
}

int Wa2Slider::corecb_onVolumeChange(int newvol)
{
	if (action != ACT_VOLUME) return 0;
	realpos = (int)(((double)newvol * 65535.f) / 255.f);
	invalidate();
	return 0;
}

int Wa2Slider::corecb_onPanChange(int newpan)
{
	if (action != ACT_BALANCE) return 0;

	realpos = (int)(((double)(newpan + 127) * 65535.f) / 255.f);
invalidate();
	return 0;
}

int Wa2Slider::corecb_onSeeked(int newpos)
{
	if (action != ACT_SEEK) return 0;

	int len = WASABI_API_MEDIACORE->core_getLength(0);

	realpos = (int)(((float)(newpos) * 65535.f) / (float)len);
invalidate();
	return 0;
}

int Wa2Slider::corecb_onStarted()
{
	started = true;
	corecb_onSeeked(0);
	invalidate();
	return 0;
}

int Wa2Slider::corecb_onStopped()
{
	started = false;
	corecb_onSeeked(0);
	invalidate();
	return 0;
}
void Wa2Slider::timerCallback(int id)
{
	switch (id)
	{
	case Wa2Slider_TIMER_POS:
		corecb_onSeeked(WASABI_API_MEDIACORE->core_getPosition(0));
		break;
	default:
		WA2SLIDER_PARENT::timerCallback(id);
	}
}
