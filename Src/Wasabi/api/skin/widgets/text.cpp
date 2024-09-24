#include <precomp.h>
#include "text.h"
#include <api.h>
#include <api/wndmgr/layout.h>
#ifdef WASABI_WIDGETS_COMPBUCK
#include <api/skin/widgets/compbuck2.h>
#endif
#include <api/skin/skinparse.h>
#if defined(WA3COMPATIBILITY) || defined(WASABI_STATICVARMGR)
#include <api/util/varmgr.h>
#endif
#include <api/core/sequence.h>
#include <api/script/vcpu.h>
#ifdef WA3COMPATIBILITY
#include <core/corehandle.h>
#endif
#include <api/wnd/notifmsg.h>
#include <api/locales/xlatstr.h>
#include <api/skin/feeds/TextFeedEnum.h>
#include <bfc/parse/pathparse.h>
#include <bfc/util/timefmt.h>
#ifdef WASABI_COMPILE_MEDIACORE
#include <api/core/api_core.h>
#endif
#include <api/service/svcs/svc_font.h>
#include <api/config/items/attribs.h>
#include <api/skin/skinelem.h>
#include <api/service/svcs/svc_action.h>
#include <tataki/blending/blending.h>
#include <tataki/canvas/bltcanvas.h>


const wchar_t textXuiObjectStr[] = L"Text"; // This is the xml tag
char textXuiSvcName[] = "Text xui object"; // this is the name of the xuiservice

#define TTS_DELAY 4000

#define TICKER_TIMER_POS 1
#define TICKER_RESET_ALTNAME 2
#define TIMER_SKIPCFG 0x987

#define COLORMODE_RGB 0
#define COLORMODE_SKINCOLOR 1

XMLParamPair Text::params[] =
{
	{TEXT_SETALTSHADOWCOLOR, L"ALTSHADOWCOLOR"},
	{TEXT_SETALTSHADOWX, L"ALTSHADOWX"},
	{TEXT_SETALTSHADOWY, L"ALTSHADOWY"},
	{TEXT_SETALTVALIGN, L"ALTVALIGN"},
	{TEXT_SETCBSOURCE, L"CBSOURCE"},
	{TEXT_SETTEXT, L"DEFAULT"},
	{TEXT_SETDISPLAY, L"DISPLAY"},
	{TEXT_SETFORCEFIXED, L"FORCEFIXED"},
	{TEXT_SETFORCELOCASE, L"FORCELOWERCASE"},
	{TEXT_SETFORCELOCASE, L"FORCELOCASE"},
	{TEXT_SETFORCEUPCASE, L"FORCEUPCASE"},
	{TEXT_SETFORCEUPCASE, L"FORCEUPPERCASE"},
	
	{TEXT_SETNOGRAB, L"NOGRAB"},
	{TEXT_SETOFFSETX, L"OFFSETX"},
	{TEXT_SETOFFSETY, L"OFFSETY"},
	
	{TEXT_SETSHADOWCOLOR, L"SHADOWCOLOR"},
	{TEXT_SETSHADOWX, L"SHADOWX"},
	{TEXT_SETSHADOWY, L"SHADOWY"},
	{TEXT_SETSHOWLEN, L"SHOWLEN"},
	{TEXT_SETTEXT, L"TEXT"},
	{TEXT_SETTICKER, L"TICKER"},
	{TEXT_SETTICKERSTEP, L"TICKERSTEP"},
	{TEXT_SETTIMECOLONWIDTH, L"TIMECOLONWIDTH"},
	{TEXT_SETTIMERHOURS, L"TIMERHOURS"},
	{TEXT_SETTIMEROFFSTYLE, L"TIMEROFFSTYLE"},
	{TEXT_SETVALIGN, L"VALIGN"},
	{TEXT_SETWRAPPED, L"WRAP"},
	{TEXT_SETTIMERHOURSROLLOVER, L"TIMERHOURSROLLOVER"},
};

Text::Text()
{
	getScriptObject()->vcpu_setInterface(textGuid, (void *)static_cast<Text *>(this));
	getScriptObject()->vcpu_setClassName(L"Text");
	getScriptObject()->vcpu_setController(textController);

	//isbitmapfont = iswinfontrender = 0;
	bufferinvalid = 1;
	cachedsizew = 0;
	size[0] = size[1] = 0;
	textpos = 0;
	time_tts = 20;
	tts = time_tts;
	sens = 0;
	grab_x = 0;
	cur_len = 0;
	ticker = 0;
	timerhours = 0;
	timerhoursRollover = 0;
	display = DISPLAY_NONE;
	elapsed = 1;
	fixedTimerStyle = 0;

	shadowcolor[0].setColorGroup(L"Text backgrounds");
	shadowcolor[0].setColor(RGB(0, 0, 0));
	shadowcolor[1].setColorGroup(L"Text backgrounds");
	shadowcolor[1].setColor(RGB(0, 0, 0));

	shadowcolor_mode[0] = COLORMODE_RGB;
	shadowcolor_mode[1] = COLORMODE_RGB;
	shadowx[0] = shadowx[1] = shadowy[0] = shadowy[1] = 0;
	timecolonw = -1;

	timeroffstyle = 0;
	
	nograb = 0;
	showlen = 0;
	forcefixed = 0;

	forceupcase = 0;
	forcelocase = 0;
	lastautowidth = 32;
	textfeed = NULL;
	wrapped = 0;
	valign[0] = ALIGN_CENTER;
	valign[1] = ALIGN_CENTER;
	offsetx = 0;
	offsety = 0;
	tickerstep = 1;
	const GUID uioptions_guid =
	  { 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };
	CfgItem *item = WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid);
	if (item != NULL)
	{
		float f = (float)item->getDataAsFloat(L"Text Ticker Speed", 1.0f / 2.0f);
		skipn = (int)((1.0f / f) - 1 + 0.5f);
	}
	skip = 0;
	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);
	registered_syscb = 0;
}

void Text::CreateXMLParameters(int master_handle)
{
	//TEXT_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

Text::~Text()
{
	killTimer(TICKER_TIMER_POS);
	killTimer(TICKER_RESET_ALTNAME);
	killTimer(TIMER_SKIPCFG);

	if (registered_syscb) WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SvcCallbackI*>(this));

#ifdef WASABI_WIDGETS_COMPBUCK
	if (display == DISPLAY_CB)
		if (mycbid.getNumItems() == 0)
			ComponentBucket2::unRegisterText(this);
		else
			for (int i = 0;i < mycbid.getNumItems();i++)
				ComponentBucket2::unRegisterText(this, mycbid.enumItem(i)->getValue());
#endif

#ifdef WASABI_COMPILE_MEDIACORE
	WASABI_API_MEDIACORE->core_delCallback(0, this);
#endif
	if (textfeed)
	{
		viewer_delViewItem(textfeed->getDependencyPtr());
		SvcEnum::release(textfeed);
		textfeed = NULL;
	}
	mycbid.deleteAll();
}

int Text::setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval)
{
	if (xuihandle != _xuihandle) return TEXT_PARENT::setXuiParam(_xuihandle, attrid, name, strval);
	switch (attrid)
	{
#ifdef WASABI_COMPILE_MEDIACORE
	case TEXT_SETDISPLAY:
		displaystr = strval;
		setDisplay(SkinParser::getDisplay(strval));
		if (!_wcsicmp(strval, L"TIMEREMAINING"))
		{
			fixedTimerStyle = 1;
			elapsed = 0;
		}
		else if (!_wcsicmp(strval, L"TIMEELAPSED"))
		{
			fixedTimerStyle = 1;
			elapsed = 1;
		}
		break;
#endif
	case TEXT_SETTICKER:
		setTickering(WTOI(strval));
		break;

	case TEXT_SETTEXT:
	{
		StringW old = getPrintedText();
		deftext = parseText(strval);
		if (!WCSCASEEQLSAFE(old, getPrintedText()))
		{
			
			if (WCSCASEEQLSAFE(L":componentname", deftext) || WCSCASEEQLSAFE(L"@COMPONENTNAME@", deftext))
			{
				Container *container = getGuiObject()->guiobject_getParentGroup()->getParentContainer();
				viewer_addViewItem(container);
			}

			StringW str = getPrintedText();
			onTextChanged(str);
		}
		break;
	}

	case TEXT_SETSHADOWCOLOR:
		if (WASABI_API_PALETTE->getColorElementRef(strval))
		{
			shadowcolor_mode[0] = COLORMODE_SKINCOLOR;
			sshadowcolor[0] = strval;
			shadowcolor_mode[1] = COLORMODE_SKINCOLOR;
			sshadowcolor[1] = strval;
		}
		else
			setShadowColor(SkinParser::parseColor(strval), 0);
		break;

	case TEXT_SETALTSHADOWCOLOR:
		if (WASABI_API_PALETTE->getColorElementRef(strval))
		{
			shadowcolor_mode[1] = COLORMODE_SKINCOLOR;
			sshadowcolor[1] = strval;
		}
		else
			setShadowColor(SkinParser::parseColor(strval), 1);
		break;

	case TEXT_SETSHADOWX:
		setShadowX(WTOI(strval));
		break;

	case TEXT_SETALTSHADOWX:
		setShadowX(WTOI(strval), 1);
		break;

	case TEXT_SETSHADOWY:
		setShadowY(WTOI(strval));
		break;

	case TEXT_SETALTSHADOWY:
		setShadowY(WTOI(strval), 0);
		break;

	case TEXT_SETTIMEROFFSTYLE:
		setTimerOffStyle(WTOI(strval));
		break;

	case TEXT_SETTIMERHOURS:
		setTimerHours(WTOI(strval));
		break;

	case TEXT_SETTIMERHOURSROLLOVER:
		setTimerHoursRollover(WTOI(strval));
		break;

	case TEXT_SETTIMECOLONWIDTH:
		setTimeColonWidth(WTOI(strval));
		break;

	case TEXT_SETNOGRAB:
		nograb = WTOI(strval);
		break;

	case TEXT_SETSHOWLEN:
		showlen = WTOI(strval);
		break;

	case TEXT_SETFORCEFIXED:
		forcefixed = WTOI(strval);
		break;

	case TEXT_SETFORCEUPCASE:
		forceupcase = WTOI(strval);
		break;

	case TEXT_SETFORCELOCASE:
		forcelocase = WTOI(strval);
		break;

	case TEXT_SETCBSOURCE:
		addCBSource(strval);
		break;

	case TEXT_SETWRAPPED:
		wrapped = WTOI(strval);
		if (isPostOnInit())
			invalidateTextBuffer();
		break;

	case TEXT_SETVALIGN:
		valign[0] = SkinParser::getAlign(strval);
		valign[1] = valign[0];
		if (isPostOnInit())
			invalidateTextBuffer();
		break;

	case TEXT_SETALTVALIGN:
		valign[1] = SkinParser::getAlign(strval);
		if (isPostOnInit())
			invalidateTextBuffer();
		break;

	case TEXT_SETOFFSETX:
		offsetx = WTOI(strval);
		if (isPostOnInit())
			invalidateTextBuffer();
		break;

	case TEXT_SETTICKERSTEP:
		tickerstep = WTOI(strval);
		break;

	case TEXT_SETOFFSETY:
		offsety = WTOI(strval);
		if (isPostOnInit())
			invalidateTextBuffer();
		break;

	default:
		return 0;
	}
	return 1;
}

int Text::getPreferences(int what)
{
	StringW thaname = getPrintedText();
	if (thaname.isempty())
	{
		return 32;
	}
	switch(wantTranslation())
	{
	case 1:
		thaname = _(thaname);
		break;
	case 2:
		thaname = __(thaname);
		break;
	}

	int alt = 0;
#ifdef WASABI_COMPILE_CONFIG
	// {280876CF-48C0-40bc-8E86-73CE6BB462E5}
	const GUID options_guid =
	  { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
	CfgItem *item = WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid);
	if (item != NULL)
	{
		alt = item->getDataAsInt(L"Alternate Fonts", 0);
		if (alt < 0 || alt > 1) alt = 0;
	}
	if (alt)
	{
		if (item && item->getDataAsInt(L"No 7-bit TTF AltFonts", 1))
		{
			const wchar_t *p = (const wchar_t *)thaname.getValue();
			while (p && *p)
			{
				if (*p > 127) break;
				p++;
			}
			if (p && !*p) alt = 0;
		}
	}
#endif
	switch (what)
	{
	case SUGGESTED_W:
	{
		int min_w = 0;
		if (forceupcase)
			thaname.toupper();
		if (forcelocase)
			thaname.tolower();
		TextInfoCanvas canvas(this);
		Wasabi::FontInfo fontInfo;
		GetFontInfo(&fontInfo, alt);

		const wchar_t *p = wcschr(thaname, ':');
		if (display == DISPLAY_TIME && p)
		{
			wchar_t secs[256] = {0};
			wchar_t mins[256] = {0};
			WCSCPYN(mins, thaname, p - thaname);

			wcsncpy(secs, p + 1, 256);
			int fixw = canvas.getTextWidth(L"0", &fontInfo);
			int _ws = forcefixed ? fixw * wcslen(secs) : canvas.getTextWidth(secs, &fontInfo);
			int _wm = forcefixed ? fixw * wcslen(mins) : canvas.getTextWidth(mins, &fontInfo);
			int _wc = forcefixed ? fixw * wcslen(L":") : canvas.getTextWidth(L":", &fontInfo);
			min_w = _ws + _wm + getTimeColonWidth(_wc);
		}
		else
		{
			PathParserW ppg(thaname, L"\n");
			for (int i = 0;i < ppg.getNumStrings();i++)
			{
				PathParserW pp(ppg.enumString(i), L"\t");
				int w = 0;
				for (int j = 0; j < pp.getNumStrings(); j++)
				{
					w += canvas.getTextWidth(pp.enumString(j), &fontInfo) + 4;
				}
				min_w = MAX(min_w, w);
			}
		}

		return min_w + lpadding + rpadding;
	}
	case SUGGESTED_H:
		PathParserW pp(thaname, L"\n");
		return fontsize[alt] * pp.getNumStrings();
	}
	return TEXT_PARENT::getPreferences(what);
}

// supermegafucko! corehandle should mirror bitrate/samplerate/channels functions instead of text having to know about gen_ff ! -- will do that real soon
#if defined(GEN_FF) & defined(WA5)
#include "../../../../Plugins/General/gen_ff/wa2frontend.h"
#endif

int Text::onInit()
{
	TEXT_PARENT::onInit();
	registered_syscb++;
	initDisplay();
	return 1;
}

void Text::initDisplay()
{
#ifdef WASABI_COMPILE_CONFIG
	setTimer(TIMER_SKIPCFG, 1000);
#endif
	switch (display)
	{
#ifdef WASABI_COMPILE_MEDIACORE
	case DISPLAY_SONGNAME:
		setName(WASABI_API_APP->main_getVersionString());
	case DISPLAY_SONGARTIST:
	case DISPLAY_SONGALBUM:
	case DISPLAY_SONGLENGTH:
	case DISPLAY_SONGTITLE:
#ifndef WASABI_COMPILE_METADB
	case DISPLAY_SONGINFO:
	case DISPLAY_SONGINFO_TRANSLATED:
#endif
		setTimer(TICKER_TIMER_POS, 25);
		setTimeTTS(TTS_DELAY / 25);
		WASABI_API_MEDIACORE->core_addCallback(0, this);
		timerCallback(TICKER_TIMER_POS);
#ifdef GEN_FF // supermegafucko!
		if(WASABI_API_MEDIACORE->core_getStatus(0) != 0){
			if (display == DISPLAY_SONGINFO)
			{
				StringW txt;
				GET_SONG_INFO_TEXT(txt);
				corecb_onInfoChange(txt);
			}
			else if (display == DISPLAY_SONGINFO_TRANSLATED)
			{
				StringW txt;
				GET_SONG_INFO_TEXT_TRANSLATED(txt);
				corecb_onInfoChange(txt);
			}
		}
#endif
		break;

	case DISPLAY_SONGBITRATE:
		WASABI_API_MEDIACORE->core_addCallback(0, this);
		corecb_onBitrateChange(wa2.getBitrate());
		break;

	case DISPLAY_SONGSAMPLERATE:
		WASABI_API_MEDIACORE->core_addCallback(0, this);
		corecb_onSampleRateChange(wa2.getSamplerate());
		break;

	case DISPLAY_TIME:
#ifdef WASABI_COMPILE_CONFIG

		if (getGuiObject())
		{
			Layout *l = getGuiObject()->guiobject_getParentLayout();
			if (l && l->getId()) elapsed = WASABI_API_CONFIG->getIntPrivate(StringPrintfW(L"%s/timer_elapsed%s", l->getId(), (fixedTimerStyle ? StringPrintfW(L".%s", this->getId()) : L"")), elapsed);
			else elapsed = WASABI_API_CONFIG->getIntPrivate(L"timer_elapsed", elapsed);
		}
		else elapsed = WASABI_API_CONFIG->getIntPrivate(L"timer_elapsed", elapsed);

#endif
		setTimer(TICKER_TIMER_POS, 250);
		setTimeTTS(TTS_DELAY / 250);
		WASABI_API_MEDIACORE->core_addCallback(0, this);
		timerCallback(TICKER_TIMER_POS);
		break;
#endif
#ifdef WASABI_WIDGETS_COMPBUCK
	case DISPLAY_CB:
		setTimer(TICKER_TIMER_POS, 50);
		setTimeTTS(TTS_DELAY / 50);
		postDeferredCallback(DISPLAY_CB, 0);
		break;
#endif
	case DISPLAY_SERVICE:
		registerToTextFeedService();
		break;
		break;
	}
}

int Text::onDeferredCallback(intptr_t p1, intptr_t p2)
{
#ifdef WASABI_WIDGETS_COMPBUCK
	switch (p1)
	{
	case DISPLAY_CB:
		if (mycbid.getNumItems() == 0)
			ComponentBucket2::registerText(this);
		else
			for (int i = 0;i < mycbid.getNumItems();i++)
				ComponentBucket2::registerText(this, mycbid.enumItem(i)->getValue());
		return 0;
	}
#endif
	return TEXT_PARENT::onDeferredCallback(p1, p2);
}

void Text::setShadowColor(COLORREF c, int alt)
{
	if (alt < 0 || alt > 1) alt = 0;
	shadowcolor_mode[alt] = COLORMODE_RGB;
	shadowcolor[alt].setColor(c);
	invalidateTextBuffer();
	if (alt == 0) setShadowColor(c, 1);
}

void Text::setShadowX(int x, int alt)
{
	if (alt < 0 || alt > 1) alt = 0;
	shadowx[alt] = x;
	invalidateTextBuffer();
	if (alt == 0) setShadowX(x, 1);
}

void Text::setShadowY(int y, int alt)
{
	if (alt < 0 || alt > 1) alt = 0;
	shadowy[alt] = y;
	invalidateTextBuffer();
	if (alt == 0) setShadowY(y, 1);
}

void Text::getBufferPaintSize(int *w, int *h)
{
	RECT r;
	getClientRect(&r);
	int _w = r.right - r.left;
	int _h = r.bottom - r.top;

	if (bufferinvalid)
	{
		cachedsizew = getPreferences(SUGGESTED_W);
	}

	if (w) *w = MAX(_w, cachedsizew);
	if (h) *h = _h;
}

void Text::getBufferPaintSource(RECT *r)
{
	if (r)
	{
		RECT cr;
		getClientRect(&cr);
		r->left = textpos;
		r->right = cr.right - cr.left + textpos;
		r->top = 0;
		r->bottom = cr.bottom - cr.top;
	}
}

#include <bfc/util/profiler.h>

// this is a temporary buffer, it should not be painted over with the painting alpha value, since it is going
// to be hanled in the actual blit by our ancestor
int Text::onBufferPaint(BltCanvas *canvas, int _w, int _h)
{
	int h, x, y=0;

	TEXT_PARENT::onBufferPaint(canvas, _w, _h);

	if (bufferinvalid)
	{
		cachedsizew = getPreferences(SUGGESTED_W);

		StringW thaname = getPrintedText();

		if (thaname.isempty())
		{
			RECT r = {0, 0, _w, _h};
			canvas->fillRect(&r, RGB(0, 0, 0));
		}
		onTextChanged(thaname); // don't remove, skipped if unnecessary

		switch(wantTranslation())
		{
		case 1:
			thaname = _(thaname);
			break;
		case 2:
			thaname = __(thaname);
			break;
		}

		int alt = 0;
#ifdef WASABI_COMPILE_CONFIG
		// {280876CF-48C0-40bc-8E86-73CE6BB462E5}
		const GUID options_guid =
		  { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
		CfgItem *item = WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid);
		if (item != NULL)
		{
			alt = item->getDataAsInt(L"Alternate Fonts", 0);
			if (alt < 0 || alt > 1) alt = 0;
		}
		if (alt)
		{
			if (item && item->getDataAsInt(L"No 7-bit TTF AltFonts", 1))
			{
				const wchar_t *p = (const wchar_t *)thaname.getValue();
				while (p && *p)
				{
					if (*p > 127) break;
					p++;
				}
				if (p && !*p) alt = 0;
			}
		}
#endif

		// canvas may have changed because of onTextChanged !
		canvas = render_canvas;
		canvas->getDim(&_w, &_h, NULL);

		RECT r = {0, 0, _w, _h};

		canvas->fillRect(&r, RGB(0, 0, 0));

		if (forceupcase)
			CharUpperW(thaname.getNonConstVal());
		if (forcelocase)
			CharLowerW(thaname.getNonConstVal());

		wchar_t secs[256] = {0};
		wchar_t mins[256] = {0};
		wchar_t hours[256] = {0};

		Wasabi::FontInfo fontInfo;
		GetFontInfo(&fontInfo, alt);

		const wchar_t *p = wcschr(thaname, L':');

		if (display != DISPLAY_TIME || !p)
		{
			int wantpadding = 0;
			int w = canvas->getTextWidth(thaname, &fontInfo);
			if (w <= r.right - r.left - 2 - lpadding - rpadding)
			{
				// if text is wider than area, don't try to align it or it'll screw up the scroll
				if (fontInfo.alignFlags != DT_CENTER) wantpadding = 1;
			}
			else
			{
				fontInfo.alignFlags = STDFONT_LEFT;
				wantpadding = 1;
			}
			h = canvas->getTextHeight(thaname, &fontInfo);
			x = r.left + 2 /*-textpos*/;
			if (wantpadding)
				x += lpadding;
			switch (valign[alt])
			{
			case ALIGN_CENTER:
				y = r.top + ((r.bottom - r.top - h) / 2);
				break;
			case ALIGN_TOP:
				y = r.top;
				break;
			}
			x += offsetx;
			y += offsety;
			cur_len = 0;
			PathParserW pp(thaname, L"\t");
			for (int i = 0; i < pp.getNumStrings(); i++)
			{
				if (i > 0)
					fontInfo.alignFlags = ALIGN_RIGHT;

				if (shadowx[alt] != 0 || shadowy[alt] != 0)
				{
					fontInfo.color = getShadowColor(alt);
					if (wrapped)
						canvas->textOutWrapped(x + shadowx[alt], y + shadowy[alt], r.right - r.left - 2 /*+textpos*/, r.bottom - r.top, pp.enumString(i), &fontInfo);
					else
						canvas->textOut(x + shadowx[alt], y + shadowy[alt], r.right - r.left - 2 /*+textpos*/, r.bottom - r.top, pp.enumString(i), &fontInfo);
					fontInfo.color = GetColor(alt);
				}
				if (wrapped)
					canvas->textOutWrapped(x, y, r.right - r.left - 2 /*+textpos*/, r.bottom - r.top, pp.enumString(i), &fontInfo);
				else
					canvas->textOut(x, y, r.right - r.left - 2 /*+textpos*/, r.bottom - r.top, pp.enumString(i), &fontInfo);
				cur_len = canvas->getTextWidth(pp.enumString(i), &fontInfo) + (wantpadding ? (lpadding + rpadding) : 0);
			}
		}
		else
		{
			if(timerhours)
			{
				WCSCPYN(hours, thaname, (p - thaname)+1);
				const wchar_t* p2 = wcschr(p + 1, L':');
				WCSCPYN(mins, p + 1, (p2 - p));
				if(p2 && *(p2 + 1))
					wcsncpy(secs, p2 + 1, 256);
				else
				{
					wcsncpy(secs, mins, 256);
					wcsncpy(mins, hours, 256);
					hours[0] = 0;
				}
			}
			else
			{
				WCSCPYN(mins, thaname, (p - thaname)+1);
				wcsncpy(secs, p + 1, 256);
			}

			h = canvas->getTextHeight(thaname, &fontInfo);
			int fixw = canvas->getTextWidth(L"0", &fontInfo);
			int _ws = forcefixed ? fixw * wcslen(secs) : canvas->getTextWidth(secs, &fontInfo);
			int _wm = forcefixed ? fixw * wcslen(mins) : canvas->getTextWidth(mins, &fontInfo);
			int _wh = forcefixed ? fixw * wcslen(hours) : canvas->getTextWidth(hours, &fontInfo);
			int _wc = forcefixed ? fixw * wcslen(L":") : canvas->getTextWidth(L":", &fontInfo);
			wchar_t widthchar = forcefixed ? '0' : 0;
			if (fontInfo.alignFlags == ALIGN_RIGHT)
			{
				x = (r.right - 2) - shadowx[alt] - rpadding;
				switch (valign[alt])
				{
				case ALIGN_CENTER:
					y = r.top + ((r.bottom - r.top - h) / 2);
					break;
				case ALIGN_TOP:
					y = r.top;
					break;
				}
				x += offsetx;
				y += offsety;
				if (shadowx[alt] != 0 || shadowy[alt] != 0)
				{
					fontInfo.color = getShadowColor(alt);
					textOut(canvas, x - _ws, y, secs, widthchar, &fontInfo);
					textOut(canvas, x - _ws - getTimeColonWidth(_wc), y, L":", widthchar, &fontInfo);
					textOut(canvas, x - _ws - getTimeColonWidth(_wc) - _wm, y, mins, widthchar, &fontInfo);
					if(timerhours && hours[0])
					{
						textOut(canvas, x - _ws - getTimeColonWidth(_wc) - _wm - getTimeColonWidth(_wc), y, L":", widthchar, &fontInfo);
						textOut(canvas, x - _ws - getTimeColonWidth(_wc) - _wm - getTimeColonWidth(_wc) - _wh, y, hours, widthchar, &fontInfo);
					}
					fontInfo.color = GetColor(alt);
				}

				x += shadowx[alt]; y += shadowy[alt];
				textOut(canvas, x - _ws, y, secs, widthchar, &fontInfo);
				textOut(canvas, x - _ws - getTimeColonWidth(_wc), y, L":", widthchar, &fontInfo);
				textOut(canvas, x - _ws - getTimeColonWidth(_wc) - _wm, y, mins, widthchar, &fontInfo);
				if(timerhours && hours[0])
				{
					textOut(canvas, x - _ws - getTimeColonWidth(_wc) - _wm - getTimeColonWidth(_wc), y, L":", widthchar, &fontInfo);
					textOut(canvas, x - _ws - getTimeColonWidth(_wc) - _wm - getTimeColonWidth(_wc) - _wh, y, hours, widthchar, &fontInfo);
				}
			}
			else if (fontInfo.alignFlags == ALIGN_LEFT)
			{
				x = (r.left + 2) - shadowx[alt] + lpadding;
				switch (valign[alt])
				{
				case ALIGN_CENTER:
					y = r.top + ((r.bottom - r.top - h) / 2);
					break;
				case ALIGN_TOP:
					y = r.top;
					break;
				}
				x += offsetx;
				y += offsety;
				if (shadowx != 0 || shadowy != 0)
				{
					fontInfo.color = getShadowColor(alt);
					if(timerhours && hours[0])
					{
						textOut(canvas, x, y, hours, widthchar, &fontInfo);
						textOut(canvas, x + _wh, y, L":", widthchar, &fontInfo);
						textOut(canvas, x + _wh + getTimeColonWidth(_wc), y, mins, widthchar, &fontInfo);
						textOut(canvas, x + _wh + getTimeColonWidth(_wc) + _wm, y, L":", widthchar, &fontInfo);
						textOut(canvas, x + _wh + getTimeColonWidth(_wc) + _wm + getTimeColonWidth(_wc), y, secs, widthchar, &fontInfo);
					}
					else
					{
						textOut(canvas, x, y, mins, widthchar, &fontInfo);
						textOut(canvas, x + _wm, y, L":", widthchar, &fontInfo);
						textOut(canvas, x + _wm + getTimeColonWidth(_wc), y, secs, widthchar, &fontInfo);
					}
					fontInfo.color = GetColor(alt);

				}
				x += shadowx[alt]; y += shadowy[alt];
				if(timerhours && hours[0])
				{
					textOut(canvas, x, y, hours, widthchar, &fontInfo);
					textOut(canvas, x + _wh, y, L":", widthchar, &fontInfo);
					textOut(canvas, x + _wh + getTimeColonWidth(_wc), y, mins, widthchar, &fontInfo);
					textOut(canvas, x + _wh + getTimeColonWidth(_wc) + _wm, y, L":", widthchar, &fontInfo);
					textOut(canvas, x + _wh + getTimeColonWidth(_wc) + _wm + getTimeColonWidth(_wc), y, secs, widthchar, &fontInfo);
				}
				else{
					textOut(canvas, x, y, mins, widthchar, &fontInfo);
					textOut(canvas, x + _wm, y, L":", widthchar, &fontInfo);
					textOut(canvas, x + _wm + getTimeColonWidth(_wc), y, secs, widthchar, &fontInfo);
				}
			}
			else if (fontInfo.alignFlags == ALIGN_CENTER)
			{
				if(timerhours && hours[0])
					x = (r.left + ((r.right - r.left - _ws - _wm - _wh - getTimeColonWidth(_wc)) / 3)) - shadowx[alt];
				else
					x = (r.left + ((r.right - r.left - _ws - _wm - getTimeColonWidth(_wc)) / 2)) - shadowx[alt];
				switch (valign[alt])
				{
				case ALIGN_CENTER:
					y = r.top + ((r.bottom - r.top - h) / 2);
					break;
				case ALIGN_TOP:
					y = r.top;
					break;
				}
				x += offsetx;
				y += offsety;
				if (shadowx[alt] != 0 || shadowy[alt] != 0)
				{
					fontInfo.color = getShadowColor(alt);
					if(timerhours && hours[0])
					{
						textOut(canvas, x, y, hours, widthchar, &fontInfo);
						textOut(canvas, x + _wh, y, L":", widthchar, &fontInfo);
						textOut(canvas, x + _wh + getTimeColonWidth(_wc), y, mins, widthchar, &fontInfo);
						textOut(canvas, x + _wh + getTimeColonWidth(_wc) + _wm, y, L":", widthchar, &fontInfo);
						textOut(canvas, x + _wh + getTimeColonWidth(_wc) + _wm + getTimeColonWidth(_wc), y, secs, widthchar, &fontInfo);
					}
					else{
						textOut(canvas, x, y, mins, widthchar, &fontInfo);
						textOut(canvas, x + _wm, y, L":", widthchar, &fontInfo);
						textOut(canvas, x + _wm + getTimeColonWidth(_wc), y, secs, widthchar, &fontInfo);
					}
					fontInfo.color = GetColor(alt);
				}
				x += shadowx[alt]; y += shadowy[alt];
				if(timerhours && hours[0])
				{
					textOut(canvas, x, y, hours, widthchar, &fontInfo);
					textOut(canvas, x + _wh, y, L":", widthchar, &fontInfo);
					textOut(canvas, x + _wh + getTimeColonWidth(_wc), y, mins, widthchar, &fontInfo);
					textOut(canvas, x + _wh + getTimeColonWidth(_wc) + _wm, y, L":", widthchar, &fontInfo);
					textOut(canvas, x + _wh + getTimeColonWidth(_wc) + _wm + getTimeColonWidth(_wc), y, secs, widthchar, &fontInfo);
				}
				else{
					textOut(canvas, x, y, mins, widthchar, &fontInfo);
					textOut(canvas, x + _wm, y, L":", widthchar, &fontInfo);
					textOut(canvas, x + _wm + getTimeColonWidth(_wc), y, secs, widthchar, &fontInfo);
				}
			}
			cur_len = _ws + _wm + getTimeColonWidth(_wc);
		}

		bufferinvalid = 0;
	}

	// alpha is taken care of in our bufferpaintwnd
	return 1;
}

void Text::timerCallback(int id)
{
	int upd = 0;
	if (id == TIMER_SKIPCFG)
	{
#ifdef WASABI_COMPILE_CONFIG
		const GUID uioptions_guid =
		  { 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };
		CfgItem *item = WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid);
		if (item != NULL)
		{
			float f = (float)item->getDataAsFloat(L"Text Ticker Speed", 1.0f / 2.0f);
			skipn = (int)((1.0f / f) - 1 + 0.5f);
		}
#endif

	}
	if (id == TICKER_RESET_ALTNAME)
	{
		killTimer(id);
		setAlternateName(NULL);
	}

	if (getAlternateName() == NULL || !*getAlternateName())
	{

		if (id == TICKER_TIMER_POS)
		{

#ifdef WASABI_COMPILE_MEDIACORE

			wchar_t txt[4096] = {0};

			// TODO: Change the way to get the current status text
			switch (display)
			{
#ifdef WASABI_COMPILE_METADB
			case DISPLAY_SONGALBUM:
			{
				const char *cur = WASABI_API_CORE->core_getCurrent(0);
				if (cur && (WASABI_API_METADB->metadb_getMetaData(cur, MT_ALBUM, txt, 4095, MDT_STRINGZ)))
				{
					if (!lastText.getValue() || STRCMP(txt, lastText.getValue()))
					{
						upd = 1;
						setName(txt);
					}
				}
				if (upd)
				{
					lastText = txt;
					resetTicker();
				}
			}
			break;
#endif
			case DISPLAY_SONGLENGTH:
			{
				int len = -1;
#ifdef WASABI_COMPILE_METADB
				const char *cur = WASABI_API_CORE->core_getCurrent(0);
				if (cur && (WASABI_API_METADB->metadb_getMetaData(cur, MT_LENGTH, (char *)&len, 4, MDT_TIME)) && len != -1)
				{
#else
				len = WASABI_API_MEDIACORE->core_getLength(0);
				if (len != -1)
				{
#endif
					if (timerhours)
						TimeFmt::printHourMinSec(len / 1000, txt, 4096, timerhoursRollover);
					else
						TimeFmt::printMinSec(len / 1000, txt, 4096);

					if (!lastText.getValue() || wcscmp(txt, lastText.getValue()))
					{
						upd = 1;
						setName(txt);
					}
				}
				if (upd)
				{
					lastText = txt;
					resetTicker();
				}
			}
			break;
#ifdef WASABI_COMPILE_METADB
			case DISPLAY_SONGARTIST:
			{
				const char *cur = WASABI_API_CORE->core_getCurrent(0);
				if (cur && (WASABI_API_METADB->metadb_getMetaData(cur, MT_ARTIST, txt, 4095, MDT_STRINGZ)))
				{
					if (!lastText.getValue() || STRCMP(txt, lastText.getValue()))
					{
						upd = 1;
						setName(txt);
					}
				}
				if (upd)
				{
					lastText = txt;
					resetTicker();
				}
			}
			break;
#endif


			case DISPLAY_SONGNAME:

			case DISPLAY_SONGTITLE:
			{
				WCSCPYN(txt, WASABI_API_MEDIACORE->core_getTitle(0), 4096);
				{
					if (showlen)
					{
						int length = wa2.getLength();
						if (length != 0 && length != -1)
						{
							length /= 1000;
							if (wcslen(txt) < 4095 - 25)
								wcscat(txt, StringPrintfW(L" (%d:%02d)", length / 60, length % 60));
						}
					}

					if (!lastText.getValue() || wcscmp(txt, lastText.getValue()))
					{
						upd = 1;
						setName(txt);
					}
				}
				if (upd)
				{
					lastText = txt;
					resetTicker();
				}
			}
			break;
			case DISPLAY_TIME:
			{
				int cp = WASABI_API_MEDIACORE->core_getPosition(0);
				if (cp < 0)
				{
					switch (timeroffstyle)
					{
					case 0:
						wcsncpy(txt, L"  :  ", 4096);
						break;
					case 1:
						StringCbPrintfW(txt, sizeof(txt),  L"%s00:00", elapsed ? L"" : L"-");
						break;
					case 2:
						*txt = 0;
						break;
					case 3:
						StringCbPrintfW(txt, sizeof(txt), L"%s0:00:00", elapsed ? L"" : L"-");
						break;
					case 4:
						wcsncpy(txt, L" :  :  ", 4096);
						break;
					}
				}
				else
				{
					int p;
					int len = WASABI_API_MEDIACORE->core_getLength(0);
					int el = elapsed;
					if (len == -1000 || el == -1)
						el = 1; // no remaining time on http streams, etc...
					if (el)
						p = cp / 1000;
					else
						p = (len - cp) / 1000;
					if (!el) p = -p;
					if (timerhours)
						TimeFmt::printHourMinSec(p, txt, 4096, timerhoursRollover);
					else
						TimeFmt::printMinSec(p, txt, 4096);
				}
				if (!lastText.getValue() || wcscmp(txt, lastText.getValue()))
				{
					setName(txt);
					upd = 1;
					lastText = txt;
				}
			}
			break;
		}
		int u = 0;
		advanceTicker(&u);
		if (u) invalidateBuffer();
#endif

		}
		else
		{
			TEXT_PARENT::timerCallback(id);
		}
	}

	if (upd)
	{
		invalidateTextBuffer();
	}
}

void Text::advanceTicker(int *upd)
{
	// update tts stuff
	if (ticker && !grab && isVisible())
	{
		int oldpos = textpos;
		RECT re = clientRect();
		if (cur_len < (re.right - re.left - 2)) textpos = 0;
		else if (tts > 0) tts -= (timerclient_getSkipped() + 1);
		else
		{
			if (skip < skipn) skip++;
			else
			{
				skip = 0;
				if (!sens) textpos += tickerstep * (timerclient_getSkipped() + 1); else textpos -= tickerstep * (timerclient_getSkipped() + 1);
				if (textpos < 0) textpos = 0;
				if (textpos > cur_len - (re.right - re.left - 2)) textpos = cur_len - (re.right - re.left - 2);
				if (cur_len <= (textpos + re.right - re.left - 2))
				{
					sens = 1;
					tts = time_tts;
				}
				if (textpos <= 0)
				{
					sens = 0;
					textpos = 0;
					tts = time_tts;
				}
			}
		}
		if (textpos != oldpos && upd != NULL) *upd = 1;
	}
}

void Text::setTimeDisplayMode(int remaining)
{
	if (fixedTimerStyle)
		return;

	elapsed = !remaining;
	Layout *l = getGuiObject()->guiobject_getParentLayout();
	if (l && l->getId()) 
		WASABI_API_CONFIG->setIntPrivate(StringPrintfW(L"%s/timer_elapsed", l->getId()), elapsed);
	else
		WASABI_API_CONFIG->setIntPrivate(L"timer_elapsed", elapsed);
	invalidateTextBuffer();
}

int Text::onLeftButtonDown(int x, int y)
{
	if (!TEXT_PARENT::onLeftButtonDown(x, y))
	{
		grab = 0;
		return 0;
	}
	if (nograb || wrapped) return 0;
	if (display == DISPLAY_TIME)
	{
		elapsed = !elapsed;
#ifdef WIN32
		// HACK! lone needs to make that a cfg attrib, but no time, a build needs to go out :D
#define WINAMP_OPTIONS_ELAPSED          40037
#define WINAMP_OPTIONS_REMAINING        40038
		if (!fixedTimerStyle) SendMessageW(WASABI_API_WND->main_getRootWnd()->gethWnd(), WM_COMMAND, elapsed ? WINAMP_OPTIONS_ELAPSED : WINAMP_OPTIONS_REMAINING, 0);
#endif
#ifdef WASABI_COMPILE_WNDMGR
#ifdef WASABI_COMPILE_CONFIG
		if (getGuiObject())
		{
			Layout *l = getGuiObject()->guiobject_getParentLayout();
			if (l && l->getId()) WASABI_API_CONFIG->setIntPrivate(StringPrintfW(L"%s/timer_elapsed%s", l->getId(), (fixedTimerStyle ? StringPrintfW(L".%s", this->getId()) : L"")), elapsed);
			else WASABI_API_CONFIG->setIntPrivate(L"timer_elapsed", elapsed);
		}
		else WASABI_API_CONFIG->setIntPrivate(L"timer_elapsed", elapsed);
#endif
#endif
		timerCallback(TICKER_TIMER_POS);
		return 1;
	}

	grab = 1;
	grab_x = x + textpos;
	//  onMouseMove(x,y);
	return 1;
}

int Text::onMouseMove(int x, int y)
{

	if (!TEXT_PARENT::onMouseMove(x, y))
	{
		grab = 0;
	}

	//POINT pos = {x, y};
	//clientToScreen(&pos);

	if (!grab) return 1;

	textpos = grab_x - x;

	RECT re;
	getClientRect(&re);
	textpos = min(textpos, cur_len - ((re.right - re.left) - 2) - 1);
	if (textpos < 0) textpos = 0;
	invalidateBuffer();
	return 1;
}

int Text::onLeftButtonUp(int x, int y)
{
	if (!TEXT_PARENT::onLeftButtonUp(x, y))
	{
		grab = 0;
		return 0;
	}
	if (nograb) return 0;
	grab = 0;
	tts = time_tts;
	return 1;
}


#ifdef WASABI_COMPILE_MEDIACORE
int Text::corecb_onStatusMsg(const wchar_t *text)
{
	if (display == DISPLAY_SONGNAME)
		setAlternateName(text);
	return 0;
}

int Text::corecb_onBitrateChange(int kbps)
{
	if (display == DISPLAY_SONGBITRATE)
	{
			if (kbps)
			{
					wchar_t bitrate[64] = {0};
					WCSNPRINTF(bitrate, 64, L"%d", kbps);
					setName(bitrate);
			}
			else
				setName(L"");
		
	}
	return 0;
}

int Text::corecb_onSampleRateChange(int hz)
{
	if (display == DISPLAY_SONGSAMPLERATE)
	{
			if (hz)
			{
					wchar_t sampleRate[64] = {0};
					WCSNPRINTF(sampleRate, 64, L"%d", hz);
					setName(sampleRate);
			}
			else
				setName(L"");
		
	}
	return 0;
}

int Text::corecb_onInfoChange(const wchar_t *text)
{
	switch (display)
	{
	case DISPLAY_SONGINFO:
	case DISPLAY_SONGINFO_TRANSLATED:
		setName(text);
		break;
	case DISPLAY_TIME:
		timerCallback(TICKER_TIMER_POS);
		break;
	}
	return 0;
}

int Text::corecb_onStopped()
{
	switch (display)
	{
	case DISPLAY_SONGINFO:
	case DISPLAY_SONGINFO_TRANSLATED:
	case DISPLAY_SONGBITRATE:
	case DISPLAY_SONGSAMPLERATE:
		setName(L"");
		break;
	case DISPLAY_TIME:
		timerCallback(TICKER_TIMER_POS);
		break;
	}
	return 0;
}

int Text::corecb_onStarted()
{
	/*  if (display == DISPLAY_TIME) {
	    timerCallback(TICKER_TIMER_POS);
	  }*/
	return 0;
}

int Text::corecb_onSeeked(int newpos)
{
	if (display == DISPLAY_TIME)
		timerCallback(TICKER_TIMER_POS);
	return 0;
}

#endif //mediacore

void Text::invalidateTextBuffer()
{
	bufferinvalid = 1;
	invalidateBuffer();
}

int Text::setTextSize(int newsize, int alt)
{
	if (alt < 0 || alt > 1) alt = 0;
	if (newsize < 1 || newsize > 72) return 0;
	size[alt] = newsize;
	invalidateTextBuffer();
	return 1;
}

void Text::setTickering(int enable)
{
	ticker = enable;
	if (!enable) textpos = 0;
	invalidateTextBuffer();
}

void Text::setDisplay(int disp)
{
	if (disp == display) return ;
	if (textfeed)
	{
		viewer_delViewItem(textfeed->getDependencyPtr());
		feed_id = L"";
		SvcEnum::release(textfeed);
		textfeed = NULL;
	}
	display = disp;
	if (isPostOnInit()) initDisplay();
	if (disp == DISPLAY_SERVICE && isPostOnInit())
		registerToTextFeedService();
	invalidateTextBuffer();
}

void Text::setAlternateName(const wchar_t *s)
{
	if (((!s || !*s) && alternatename.isempty()) || WCSCASEEQLSAFE(alternatename, s)) return ;
	killTimer(TICKER_RESET_ALTNAME);
	alternatename = parseText(s);
	onTextChanged(getPrintedText());
	resetTicker();
	invalidate();
	setTimer(TICKER_RESET_ALTNAME, 1000);
}

void Text::setText(const wchar_t *t)
{
	deftext = parseText(t);
	invalidate();
	onTextChanged(getPrintedText());
}

const wchar_t *Text::parseText(const wchar_t *s)
{
	static wchar_t t[4096];
	if (!s) return NULL;
	WCSCPYN(t, s, 4096);
	wchar_t *p = t;
	while (p && *p && *(p + 1))
	{
		if (*p == '\\' && *(p + 1) == 'n')
		{
			// TODO check
			wcscpy(p, p + 1);
			t[wcslen(t)] = 0;
			*p = '\n';
		}
		p++;
	}
	return t;
}

void Text::onTextChanged(const wchar_t *txt)
{
	if (WCSCASEEQLSAFE(lasttxt, txt)) return ;
	lasttxt = txt;
	invalidate();
	int w = getTextWidth();
	if (w != lastautowidth)
		notifyParent(ChildNotify::AUTOWHCHANGED);
	lastautowidth = w;
	script_vcpu_onTextChanged(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(lasttxt));
	invalidateTextBuffer();
}

const wchar_t *Text::getPrintedText()
{
	const wchar_t *name = getAlternateName();
	if (!name || !*name)
		name = deftext.getValue();
	if (!name || !*name)
		name = getName();
	if (name == NULL)
		return L"";

#if defined(WASABI_STATICVARMGR) || !defined(WASABINOMAINAPI)

	if (wantTranslation())
	{
		StringW *s = PublicVarManager::translate(name, getGuiObject());
		if (s != NULL)
		{
			printedtxt.swap(s);
			delete s;
			return printedtxt.getValueSafe();
		}
	}

	return name;
#else
	return name;

#endif
}

void Text::onSetName()
{
	invalidateTextBuffer();
	onTextChanged(getPrintedText());
}

const wchar_t *Text::getAlternateName(void)
{
	if (alternatename.isempty()) return NULL;
	return alternatename;
}

void Text::setTimerOffStyle(int o)
{
	if (timeroffstyle == o) return ;
	timeroffstyle = o;
	invalidateTextBuffer();
}

void Text::setTimerHours(int o)
{
	if (timerhours == o) return ;
	timerhours = o;
	invalidateTextBuffer();
}

void Text::setTimerHoursRollover(int o)
{
	if (timerhoursRollover == o) return ;
	timerhoursRollover = o;
	invalidateTextBuffer();
}

void Text::setTimeTTS(int tts)
{
	time_tts = tts;
	invalidateTextBuffer();
}

void Text::resetTicker()
{
	sens = 0;
	textpos = 0;
	tts = time_tts;
	invalidateBuffer();
}

void Text::setTimeColonWidth(int w)
{
	timecolonw = w;
	invalidateTextBuffer();
}

int Text::getTimeColonWidth(int def)
{
	return timecolonw == -1 ? def : timecolonw;
}

void Text::textOut(Canvas *canvas, int x, int y, const wchar_t *txt, wchar_t widthchar, const Wasabi::FontInfo *fontInfo)
{
	if (widthchar == 0)
	{
		canvas->textOut(x, y, txt, fontInfo);
		return ;
	}
	wchar_t wc[2] = { widthchar, 0 };
	int cwidth = canvas->getTextWidth(wc, fontInfo);
	int slen = wcslen(txt);

	for (int i = 0; i < slen; i++)
	{
		wc[0] = txt[i];
		int dw = cwidth - canvas->getTextWidth(wc, fontInfo);	// get difference
		canvas->textOut(x + dw / 2, y, wc, fontInfo);
		x += cwidth;
	}
}

void Text::addCBSource(const wchar_t *cbsource)
{
	StringW *s = new StringW(cbsource);
	mycbid.addItem(s);
}

int Text::getTextWidth()
{
	const wchar_t *txt = getAlternateName() ? getAlternateName() : isInited() ? getName() : NULL;
	if (!txt) txt = deftext.getValue();
	if (!txt) return 0;
#ifdef WA3COMPATIBILITY
	/*
	String *translate = PublicVarManager::translate(thaname, getGuiObject());
	if (translate)
		thanme = translate->getValueSafe();
		*/
#endif
	//txt = _(txt);
	int alt = 0;
#ifdef WASABI_COMPILE_CONFIG
	// {280876CF-48C0-40bc-8E86-73CE6BB462E5}
	const GUID options_guid =
	  { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
	CfgItem *item = WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid);
	if (item != NULL)
	{
		alt = item->getDataAsInt(L"Alternate Fonts", 0);
		if (alt < 0 || alt > 1) alt = 0;
	}
	if (alt)
	{
		if (item && item->getDataAsInt(L"No 7-bit TTF AltFonts", 1))
		{
			const wchar_t *p = (const wchar_t *)txt;
			while (p && *p)
			{
				if (*p > 127) break;
				p++;
			}
			if (p && !*p) alt = 0;
		}
	}
#endif
	TextInfoCanvas canvas(this);
	Wasabi::FontInfo fontInfo;
	GetFontInfo(&fontInfo, alt);
	int w = canvas.getTextWidth(txt, &fontInfo) + 4;
//	delete txt;
	return w;
}

void Text::registerToTextFeedService()
{
	if (!registered_syscb++) WASABI_API_SYSCB->syscb_registerCallback(static_cast<SvcCallbackI*>(this));

	if (textfeed)
	{
		viewer_delViewItem(textfeed->getDependencyPtr());
		feed_id = L"";
		SvcEnum::release(textfeed);
		textfeed = NULL;
	}

	if (!displaystr.isempty()) textfeed = TextFeedEnum(displaystr).getFirst();

	if (textfeed != NULL)
	{
		feed_id = displaystr;
		viewer_addViewItem(textfeed->getDependencyPtr());
	}
}

void Text::svccb_onSvcRegister(FOURCC type, waServiceFactory *svc)
{
	if (type == WaSvc::TEXTFEED)
	{
		//CUTif (!displaystr.isempty()) {
		//CUTDebugString("RERERERER %s", displaystr.v());
		//CUT}
		registerToTextFeedService();
	}
}

int Text::viewer_onEvent(api_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen)
{
	if (textfeed && item == textfeed->getDependencyPtr())
	{
		if (event == svc_textFeed::Event_TEXTCHANGE && WCSCASEEQLSAFE((const wchar_t *)param, feed_id))
		{
			//CUTDebugString("got feed '%s'", (const char *)ptr);
			setName((const wchar_t *)ptr);
			return 1;
		}
	}
	
	else if (classguid && *classguid == *Container::depend_getClassGuid())
	{
		onSetName();
		return 1;
	}
	return 0;
}

int Text::triggerEvent(int event, intptr_t p1, intptr_t p2)
{
	int r = TEXT_PARENT::triggerEvent(event, p1, p2);
	if (event == TRIGGER_ONRESIZE)
		notifyParent(ChildNotify::AUTOWHCHANGED);
	if (event == TRIGGER_INVALIDATE)
		invalidateTextBuffer();
	return r;
}

COLORREF Text::getShadowColor(int alt)
{
	if (alt < 0 || alt > 1) alt = 0;
	if (shadowcolor_mode[alt] == COLORMODE_SKINCOLOR) return sshadowcolor[alt];
	return shadowcolor[alt].getColor();
}

TextScriptController _textController;
TextScriptController *textController = &_textController;

// -- Functions table -------------------------------------
function_descriptor_struct TextScriptController::exportedFunction[] =
{
	{L"setText", 1, (void*)Text::script_vcpu_setText },
	{L"setAlternateText", 1, (void*)Text::script_vcpu_setAlternateText },
	{L"getText", 0, (void*)Text::script_vcpu_getText },
	{L"getTextWidth", 0, (void*)Text::script_vcpu_getTextWidth },
	{L"onTextChanged", 1, (void*)Text::script_vcpu_onTextChanged },
};
// --------------------------------------------------------

const wchar_t *TextScriptController::getClassName()
{
	return L"Text";
}

const wchar_t *TextScriptController::getAncestorClassName()
{
	return L"GuiObject";
}

ScriptObject *TextScriptController::instantiate()
{
	Text *t = new Text;
	ASSERT(t != NULL);
	return t->getScriptObject();
}

void TextScriptController::destroy(ScriptObject *o)
{
	Text *t = static_cast<Text *>(o->vcpu_getInterface(textGuid));
	ASSERT(t != NULL);
	delete t;
}

void *TextScriptController::encapsulate(ScriptObject *o)
{
	return NULL; // no encapsulation for text yet
}

void TextScriptController::deencapsulate(void *o)
{}

int TextScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *TextScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID TextScriptController::getClassGuid()
{
	return textGuid;
}

const wchar_t *Text::vcpu_getClassName()
{
	return L"Text";
}

scriptVar Text::script_vcpu_setText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(t.type == SCRIPT_STRING);
	Text *tx = static_cast<Text *>(o->vcpu_getInterface(textGuid));
	if (tx) tx->setText(GET_SCRIPT_STRING(t));
	RETURN_SCRIPT_VOID;
}

scriptVar Text::script_vcpu_setAlternateText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(t.type == SCRIPT_STRING);
	Text *tx = static_cast<Text *>(o->vcpu_getInterface(textGuid));
	if (tx) tx->setAlternateName(GET_SCRIPT_STRING(t));
	RETURN_SCRIPT_VOID;
}

scriptVar Text::script_vcpu_getText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Text *t = static_cast<Text *>(o->vcpu_getInterface(textGuid));
	if (t)
	{
		const wchar_t *from = t->getPrintedText();
		// BU rewrote in response to talkback for 489
		if (from == NULL || *from == '\0') from = t->getLastText();
		if (from == NULL || *from == '\0') from = L"";
		WCSCPYN(s_txt, from, 4096);
		return MAKE_SCRIPT_STRING(s_txt);
	}
	return MAKE_SCRIPT_STRING(L"");
}

scriptVar Text::script_vcpu_getTextWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Text *t = static_cast<Text *>(o->vcpu_getInterface(textGuid));
	if (t) return MAKE_SCRIPT_INT(t->getTextWidth());
	return MAKE_SCRIPT_INT(0);
}

scriptVar Text::script_vcpu_onTextChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar newtxt)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, textController, newtxt);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, newtxt);
}

wchar_t Text::s_txt[WA_MAX_PATH] = {0};