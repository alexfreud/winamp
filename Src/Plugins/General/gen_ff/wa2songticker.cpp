#include <precomp.h>
#include "wa2songticker.h"
#include <api.h>
#include <tataki/color/skinclr.h>
#include <api/core/api_core.h>
#include <api/application/api_application.h>
#include <wasabicfg.h>
#include "wa2frontend.h"
#include <api/skin/skinelem.h>
#include <api/skin/skinparse.h>
#include <api/config/items/attribs.h>
#include <api/config/api_config.h>

// {9149C445-3C30-4e04-8433-5A518ED0FDDE}
static const GUID uioptions_guid =
  { 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };

// {280876CF-48C0-40bc-8E86-73CE6BB462E5}
static const GUID options_guid =
  { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };

const wchar_t songtickerXuiObjectStr[] = L"SongTicker"; // This is the xml tag
char songtickerXuiSvcName[] = "Song Ticker XUI object"; // this is the name of the xuiservice

BEGIN_SERVICES(wa2SongTicker_Svcs);
DECLARE_SERVICE(XuiObjectCreator<SongTickerXuiSvc>);
END_SERVICES(wa2SongTicker_Svcs, _wa2SongTicker_Svcs);

#ifdef _X86_
extern "C"
{
	int _link_wa2SongTicker_Svcs;
}
#else
extern "C"
{
	int __link_wa2SongTicker_Svcs;
}
#endif

extern _float cfg_uioptions_textspeed;
extern _int cfg_uioptions_textincrement;

#define TIMER_SONGTICKER_SCROLL 0x777
//#define SONGTICKER_SCROLL_MS 200 // TODO: make adjustable (api_config, xml param, maki script object)
#define SONGTICKER_INCREMENT_PIXELS (cfg_uioptions_textincrement) // TODO: make adjustable?
#define SONGTICKER_SCROLL_MS ((13.0f*(float)cfg_uioptions_textincrement)/cfg_uioptions_textspeed)
#define SONGTICKER_SCROLL_ONE_PIXEL_MS (13.0f/cfg_uioptions_textspeed)
#define SONGTICKER_SKIP_MS 2000 // TODO: make adjustable

XMLParamPair SongTicker::params[] =
{
	{SONGTICKER_TICKER, L"TICKER"},
};

SongTicker::SongTicker()
{
	WASABI_API_MEDIACORE->core_addCallback(0, this);
	song_title[0]=0;
	song_length=-1;
	position=0;
	tickerMode=TICKER_SCROLL;
	ticker_direction=1;
	skipTimers=0;
	grab_x = 0;
	last_tick = Wasabi::Std::getTickCount();
	buffer_hw_valid=false;
	textW=0;

	/* register XML parameters */
	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);

	CfgItem *ci=WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid);
	if (ci)
	{
		viewer_addViewItem(ci->getDependencyPtr());
	}

	ci=WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid);
	if (ci)
	{
		viewer_addViewItem(ci->getDependencyPtr());
	}
}

void SongTicker::CreateXMLParameters(int master_handle)
{
	//SONGTICKER_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

SongTicker::~SongTicker()
{
	WASABI_API_MEDIACORE->core_delCallback(0, this);
// TODO: benski> is this the right place for this?
	killTimer(TIMER_SONGTICKER_SCROLL);
}

int SongTicker::onResize()
{
	killTimer(TIMER_SONGTICKER_SCROLL);
	buffer_hw_valid=false;
	invalidateBuffer();
	return SONGTICKER_PARENT::onResize();
}

int SongTicker::onInit()
{
	int r = SONGTICKER_PARENT::onInit();
	BuildTitle();
	return r;
}

int SongTicker::corecb_onTitleChange(const wchar_t *title)
{
	BuildTitle();
	return 1;
}

int SongTicker::corecb_onLengthChange(int newlength)
{
	song_length = newlength;
	return 1;
}

void SongTicker::BuildTitle()
{
	killTimer(TIMER_SONGTICKER_SCROLL);
	const wchar_t *curFilename = wa2.GetCurrentFile();
	if (curFilename && *curFilename)
	{
		wa2.GetFileInfo(curFilename, song_title, 1024, &song_length);
		WASABI_API_MEDIACORE->core_setTitle(song_title);
	}
	else
	{
		song_title[0]=0;
		song_length=-1;
	}

	if (!song_title[0] || ((unsigned long)song_title < 65536))
	{
		display = WASABI_API_APP->main_getVersionString();
	}
	else
	{
		// TODO: use TimeFmt:: to format the time
		if (song_length >= 0)
			display.printf(L"%s (%d:%02d)",song_title,song_length/60,song_length%60);
		else
			display.printf(L"%s",song_title);
	}

	rotatingDisplay.printf(L"%s *** %s", display, display);
	TextInfoCanvas c(this);
	Wasabi::FontInfo fontInfo;
	GetFontInfo(&fontInfo);
	width_of_str_padded = c.getTextWidth(display, &fontInfo) + lpadding - rpadding;
	width_of_str = c.getTextWidth(rotatingDisplay, &fontInfo) - c.getTextWidth(display, &fontInfo);
	buffer_hw_valid=false;
	invalidateBuffer();
}

int SongTicker::onBufferPaint(BltCanvas *canvas, int w, int h)
{
	SONGTICKER_PARENT::onBufferPaint(canvas, w, h);

	// TODO: benski> need to optimize this :)
	RECT r = {0,0,w,h};
	canvas->fillRect(&r, RGB(0, 0, 0));

	getClientRect(&r);

	Wasabi::FontInfo fontInfo;
	GetFontInfo(&fontInfo);
	/*
		if (tickerMode == TICKER_OFF)
		{
			lpadding=min(lpadding, w);
			w=max(w-lpadding+rpadding, 0);
			canvas->textOut(lpadding, 0, w, h, display, &fontInfo);
			return 1;
		}
	*/
	const int textAreaWidth = r.right - r.left;
	StringW *whichString = &display;
	if (width_of_str_padded > textAreaWidth) // too big to fit?
	{
		switch (tickerMode)
		{
		case TICKER_OFF:
		{
			int extra = width_of_str_padded - textAreaWidth;
			if (position>extra)
				position=extra;
			if (position<0)
				position=0;
		}
		break;

		case TICKER_SCROLL:
		{
			// make sure our position isn't out of bounds
			while (position < 0)
				position += width_of_str;

			position %= (width_of_str);

			whichString = &rotatingDisplay;
			skipTimers=0;
			setTimer(TIMER_SONGTICKER_SCROLL, (int)SONGTICKER_SCROLL_MS);
		}
		break;

		case TICKER_BOUNCE:
		{
			int extra = width_of_str_padded- textAreaWidth;
			if (position < 0)
			{
				position=0;
				ticker_direction=1;
				skipTimers=(int)(SONGTICKER_SKIP_MS/SONGTICKER_SCROLL_MS);
			}

			if (position>=extra)
			{
				position=extra-1;
				ticker_direction=-1;
				skipTimers=(int)(SONGTICKER_SKIP_MS/SONGTICKER_SCROLL_MS);
			}
			if (position < 0)
				position=0;
			setTimer(TIMER_SONGTICKER_SCROLL, (int)SONGTICKER_SCROLL_MS);
		}
		break;
		}
	}
	else // if there's enough room, just draw the string as-is
	{
		position=0;
	}
	int x =min(lpadding, w);
	w=max(w-x+rpadding, 0);
	canvas->textOut(lpadding, 0, w, h, whichString->getValueSafe(), &fontInfo);

	return 1;
}

void SongTicker::timerCallback(int id)
{
	if (id == TIMER_SONGTICKER_SCROLL && !grab)
	{
		uint32_t this_tick = Wasabi::Std::getTickCount();
		if (TICKER_OFF == tickerMode)
		{
			killTimer(TIMER_SONGTICKER_SCROLL);
			return;
		}
		if (skipTimers)
		{
			last_tick=this_tick;
			skipTimers--;
			return;
		}
		int numTicks=SONGTICKER_INCREMENT_PIXELS;
		if (this_tick > last_tick) // make sure we havn't wrapped around
		{
			numTicks = (int)((this_tick - last_tick) / SONGTICKER_SCROLL_ONE_PIXEL_MS);
			last_tick += (uint32_t)(numTicks * SONGTICKER_SCROLL_ONE_PIXEL_MS); // we do this instead of last_tick = this_tick, so we get some error shaping
		}
		else
			last_tick=this_tick;

		// move the ticker
		position+=(ticker_direction*numTicks);
		// ask to be redrawn
		if (numTicks)
			invalidate();
	}
	else
		SONGTICKER_PARENT::timerCallback(id);
}

void SongTicker::getBufferPaintSource(RECT *r)
{
	if (r)
	{
		RECT cr;
		getClientRect(&cr);
		const int textAreaWidth = cr.right - cr.left;
		if (width_of_str_padded > textAreaWidth) // too big to fit?
		{
			switch (tickerMode)
			{
			case TICKER_OFF:
			{
				int extra = width_of_str_padded- textAreaWidth;
				if (position>extra)
					position=extra;
				if (position<0)
					position=0;
			}
			break;

			case TICKER_SCROLL:
			{
				// make sure our position isn't out of bounds
				while (position < 0)
					position += width_of_str;

				position %= (width_of_str);
			}
			break;

			case TICKER_BOUNCE:
			{
				int extra = width_of_str_padded - textAreaWidth;

				if (position>=extra)
				{
					position=extra-1;
					ticker_direction=-1;
					skipTimers=(int)(SONGTICKER_SKIP_MS/SONGTICKER_SCROLL_MS);
				}

				if (position < 0)
				{
					position=0;
					ticker_direction=1;
					skipTimers=(int)(SONGTICKER_SKIP_MS/SONGTICKER_SCROLL_MS);
				}
			}
			break;
			}
		}
		else
		{
			position=0;
		}
		r->left = position;
		r->right = cr.right - cr.left + position;
		r->top = 0;
		r->bottom = cr.bottom - cr.top;
	}
}

void SongTicker::getBufferPaintSize(int *w, int *h)
{
	RECT r;
	getClientRect(&r);
	int _w = r.right - r.left;
	int _h = r.bottom - r.top;

	if (!buffer_hw_valid)
	{
		const int textAreaWidth = r.right - r.left;
		StringW *whichString = &display;
		if (width_of_str_padded > textAreaWidth && tickerMode == TICKER_SCROLL) // too big to fit?
		{
			whichString = &rotatingDisplay;
			buffer_hw_valid=false;
		}

		TextInfoCanvas canvas(this);
		Wasabi::FontInfo fontInfo;
		GetFontInfo(&fontInfo);
		textW = canvas.getTextWidth(whichString->getValueSafe(), &fontInfo);
		//int textH = canvas.getTextHeight(whichString->getValueSafe(), &fontInfo);
		textW = textW + lpadding - rpadding;
		buffer_hw_valid=true;
	}

	*w = max(_w, textW);
	*h = _h;//max(_h, textH);
}

int SongTicker::setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval)
{
	if (xuihandle != _xuihandle) return SONGTICKER_PARENT::setXuiParam(_xuihandle, attrid, name, strval);
	switch (attrid)
	{
	case SONGTICKER_TICKER:
		if (!WCSICMP(strval, L"bounce"))
			tickerMode=TICKER_BOUNCE;
		else if (!WCSICMP(strval, L"scroll"))
		{
			tickerMode=TICKER_SCROLL;
			ticker_direction=1;
		}
		else if (!WCSICMP(strval, L"off"))
		{
			tickerMode=TICKER_OFF;
			position=0;
			killTimer(TIMER_SONGTICKER_SCROLL);
		}
		buffer_hw_valid=false;
		invalidateBuffer();
		break;
	default:
		return 0;
	}
	return 1;
}

int SongTicker::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source)
{
	int r = SONGTICKER_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
	if(!WCSICMP(action, L"rebuildtitle"))
		BuildTitle();

	return r;
}

int SongTicker::viewer_onEvent(api_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen)
{
	if (*classguid == *CfgItem::depend_getClassGuid())
	{
		if (event==CfgItem::Event_ATTRIBUTE_CHANGED)
		{
			CfgItem *ci = (CfgItem *)item->dependent_getInterface(CfgItem::depend_getClassGuid());
			if (ci->getGuid() == uioptions_guid
			    && ptr && (WCSCASEEQLSAFE((const wchar_t *)ptr, L"Text Ticker Speed") || WCSCASEEQLSAFE((const wchar_t *)ptr, L"Text Ticker Increment")))
		{
			BuildTitle();
			}
		}
	}

	return 1;
}

int SongTicker::onLeftButtonDown(int x, int y)
{
	if (!SONGTICKER_PARENT::onLeftButtonDown(x, y))
	{
		grab = 0;
		return 0;
	}

	grab = 1;
	grab_x = x + position;
	//  onMouseMove(x,y);
	return 1;
}

int SongTicker::onMouseMove(int x, int y)
{

	if (!SONGTICKER_PARENT::onMouseMove(x, y))
	{
		grab = 0;
	}

	//POINT pos = {x, y};
	//clientToScreen(&pos);

	if (!grab) return 1;

	position = grab_x - x;

	// this forces us to calculate wraparound for position
	RECT dummy;
	getBufferPaintSource(&dummy);

	invalidate();
	return 1;
}

int SongTicker::onLeftButtonUp(int x, int y)
{
	if (!SONGTICKER_PARENT::onLeftButtonUp(x, y))
	{
		grab = 0;
		return 0;
	}

	grab = 0;
	return 1;
}
