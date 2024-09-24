#ifndef NULLSOFT_GEN_FF_WA2SONGTICKER_H
#define NULLSOFT_GEN_FF_WA2SONGTICKER_H

//#include <api/wnd/wndclass/bufferpaintwnd.h>
#include <api/skin/widgets/textbase.h>
#include <api/syscb/callbacks/corecbi.h>
#include <tataki/color/skinclr.h>
#include <bfc/depend.h>

#define SONGTICKER_PARENT TextBase

class SongTicker 
	: public SONGTICKER_PARENT, /* provides basic wasabi windowing functinoality */
	public CoreCallbackI, /* to get title change updates */
	public DependentViewerI /* for config callbacks*/
{

public:
	SongTicker();
	~SongTicker(); 

	enum TickerMode
	{
		TICKER_OFF,
		TICKER_SCROLL,
		TICKER_BOUNCE,
	};

	int onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source);

protected:
/* BaseWnd */
	virtual int onInit();
	virtual int onBufferPaint(BltCanvas *canvas, int w, int h); // draw the song ticker here

		/* TimerClient */
	virtual void timerCallback(int id); // scroll the song ticker every # seconds

		/* media core callbacks */
	/* TODO: benski> some thoughts... this differs from current behaviour but might be an interesting idea
		virtual int corecb_onStarted(); // start the ticker
		virtual int corecb_onStopped(); // go back to showing Winamp version string
		virtual int corecb_onPaused(); // pause the ticker
		virtual int corecb_onUnpaused(); // unpause the ticker

		// benski> currently unused in Winamp 5... might be worth implementing so we can draw text here
		virtual int corecb_onStatusMsg(const wchar_t *text);
		virtual int corecb_onWarningMsg(const wchar_t *text);
		virtual int corecb_onErrorMsg(const wchar_t *text);
	*/
	
	virtual int corecb_onTitleChange(const wchar_t *title);
	// benski> not sure what the difference here is - virtual int corecb_onTitle2Change(const wchar_t *title2);
	 virtual int corecb_onLengthChange(int newlength);
	void getBufferPaintSource(RECT *r);
	virtual void getBufferPaintSize(int *w, int *h); 

	/* TextBase */
	void invalidateTextBuffer()
	{
		invalidateBuffer();
	}

	int viewer_onEvent(api_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen);
		int onLeftButtonDown(int x, int y);
  int onMouseMove(int x, int y);
  int onLeftButtonUp(int x, int y);

	/*static */void CreateXMLParameters(int master_handle);
	 virtual int onResize();
private:
	int grab_x;

	/* Title */
	void BuildTitle();
	wchar_t song_title[1024];
	int song_length;
	int position;
	StringW display, rotatingDisplay;
	int width_of_str, width_of_str_padded;

	/* */
	int textW;
	bool buffer_hw_valid;

	/* scroll timer */
	TimerToken scroll_timer_id;
	int ticker_direction;
	TickerMode tickerMode;
	int skipTimers;
	uint32_t last_tick;
	
	/* XML Parameters */
	enum
	{
		SONGTICKER_TICKER,
	};
	static XMLParamPair params[];
	int xuihandle;
	int setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval);

};

extern const wchar_t songtickerXuiObjectStr[];
extern char songtickerXuiSvcName[];
class SongTickerXuiSvc : public XuiObjectSvc<SongTicker, songtickerXuiObjectStr, songtickerXuiSvcName> {};

#endif
