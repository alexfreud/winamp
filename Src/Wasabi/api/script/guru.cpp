#include <precomp.h>
#include <api.h>
#ifdef WASABI_COMPILE_WND
#include <api/wnd/basewnd.h>
#include <tataki/canvas/bltcanvas.h>
#endif
#include <api/script/guru.h>
#include <api/script/script.h>
#include <api/script/vcpu.h>
#ifdef WASABI_COMPILE_SKIN
#include <api/skin/skin.h>
#endif

extern HINSTANCE hInstance;
#ifdef WASABI_COMPILE_WND
Guru::Guru() 
{
	fcount = 0;
	txt=NULL;
	code=0;
	intinfo=0;
}

Guru::~Guru() {
}
#endif

void Guru::spawn(SystemObject *_script, int code, const wchar_t *pub, int intinfo) {
#ifdef WASABI_COMPILE_WND
	script = _script;
	if (WASABI_API_PALETTE->getSkinPartIterator() > last_iterator) {
	    mustquit = 0;
		last_iterator = WASABI_API_PALETTE->getSkinPartIterator();
	}
	else
		return;

#ifdef WASABI_COMPILE_SKIN
	int oldlock = WASABI_API_SKIN->skin_getLockUI();
	WASABI_API_SKIN->skin_setLockUI(0);
#endif

	Guru g;
	g.setCode(code);
	g.setPublicTxt(pub);
	g.setIntInfo(intinfo);
	g.setStartHidden(1);
	g.init(hInstance, INVALIDOSWINDOWHANDLE, TRUE);

	RECT r;
	Wasabi::Std::getViewport(&r, (POINT*)NULL);

	r.left = r.left + ((r.right-r.left-640)/2);
	r.right = r.left + 640;
	TextInfoCanvas c(&g);
	Wasabi::FontInfo fontInfo;
	fontInfo.pointSize = WASABI_API_APP->getScaleY(14);
  
#ifdef WIN32
	fontInfo.face = L"Lucida Console";
#else
	fontInfo.face = L"Lucida";
#endif
	fontInfo.bold = true;
  
	r.bottom = r.top + c.getTextHeight(&fontInfo)* (script != NULL ? 9 : 7);

	g.resize(&r);
	g.setVisible(1);
	g.bringToFront();

	MSG msg = {0};
	WASABI_API_WND->pushModalWnd(&g);
#ifdef WIN32
	HWND old = SetCapture(g.gethWnd());
#endif
	while (!mustquit) {
		mustquit = !GetMessage(&msg, g.gethWnd(), 0, 0);
#ifdef WIN32
		if (!msg.hwnd || !TranslateAccelerator(msg.hwnd, NULL, &msg)) {
#endif
			TranslateMessage(&msg);
			DispatchMessage(&msg);
#ifdef WIN32
		}
#endif
	}

	WASABI_API_WND->popModalWnd(&g);
#ifdef WIN32
	SetCapture(old);
#endif

#else
	StringPrintfW t(L"Guru Meditation #%04X.%04X%04X.%d%s%s", code, (intinfo & 0xFFFF), VCPU::VIP & 0xFFFF, VCPU::VSD, pub?" ":"", pub?pub:"");
	Std::messageBox(t, L"Guru Meditiation", 16);
#endif

#ifdef WASABI_COMPILE_SKIN
	WASABI_API_SKIN->skin_setLockUI(oldlock);
#endif
}

#ifdef WASABI_COMPILE_WND

int Guru::onPaint(Canvas *canvas) {

	GURU_PARENT::onPaint(canvas);

	PaintBltCanvas paintcanvas;
	if (canvas == NULL) {
		paintcanvas.beginPaint(this);
		canvas = &paintcanvas;
	}

	RECT r;
	getClientRect(&r);

	canvas->fillRect(&r, 0);

	if (fcount%2==0) 
	{
	    int w;
		Wasabi::FontInfo fontInfo;
		fontInfo.color = RGB(0xFF,0,0);
		fontInfo.pointSize = WASABI_API_APP->getScaleY(14);

#ifdef WIN32
		fontInfo.face = L"Lucida Console";
#else
		fontInfo.face = L"Lucida";
#endif
		fontInfo.bold = true;
		fontInfo.opaque = false;
		w = canvas->getTextHeight(&fontInfo);
		RECT s = {40, w*2, 560, w*3};
		canvas->textOutCentered(&s, L"Winamp Script Failure. Press the left mouse button to continue.", &fontInfo);
		StringPrintfW t(L"Guru Meditation #%04X.%04X%04X.%d%s%s", code, (intinfo & 0xFFFF), VCPU::VIP & 0xFFFF, VCPU::VSD, txt?L" ":L"", txt?txt:L"");
		s.top=w*4;
		s.bottom=s.top+w;
		canvas->textOutCentered(&s, t, &fontInfo);

		if (script != NULL) {
			s.top=w*6;
			s.bottom=s.top+w;
			canvas->textOutCentered(&s, script->getFilename(), &fontInfo);
		}

		RECT z;
		z.top = r.top + 5;
		z.bottom = r.top + min(10, w-2);
		z.left = r.left + 5;
		z.right = r.right - 5;
		canvas->fillRect(&z, RGB(0xFF,0,0));

		z.top = r.top + 5;
		z.bottom = r.bottom - 5;;
		z.left = r.right - min(10, w-2);
		z.right = r.right - 5;
		canvas->fillRect(&z, RGB(0xFF,0,0));

		z.top = r.bottom - min(10, w-2);
		z.bottom = r.bottom - 5;
		z.left = r.left + 5;
		z.right = r.right - 5;
		canvas->fillRect(&z, RGB(0xFF,0,0));

		z.top = r.top + 5;
		z.bottom = r.bottom - 5;;
		z.left = r.left + 5;
		z.right = r.left + min(10, w-2);
		canvas->fillRect(&z, RGB(0xFF,0,0));
	}
  
	return 1;
}

int Guru::onLeftButtonUp(int x, int y) {
	mustquit=1;
	return GURU_PARENT::onLeftButtonUp(x,y);
}

int Guru::onInit() {
	GURU_PARENT::onInit();
	setTimer(GURU_TIMERID, 400);
	return 1;
}

void Guru::setCode(int c) {
	code = c;
}

void Guru::setPublicTxt(const wchar_t *t) {
	txt = t;
}

void Guru::setIntInfo(int info) {
	intinfo = info;
}

void Guru::timerCallback(int id) {
	if (id == GURU_TIMERID) {
		fcount++;
		if (fcount > 7) {
			killTimer(GURU_TIMERID);
		}
		invalidate();
		return;
	}
	GURU_PARENT::timerCallback(id);
}

int Guru::mustquit = 0;
int Guru::last_iterator = -1;
SystemObject * Guru::script = NULL;
#endif
