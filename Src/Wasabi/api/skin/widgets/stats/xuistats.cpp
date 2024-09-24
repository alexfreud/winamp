#include <precomp.h>
#include "xuistats.h"
#include <tataki/canvas/ifc_canvas.h>
#include <tataki/color/skinclr.h>
#include <api.h>

#include <api/imgldr/imgldr.h>
#include <api/skin/skinparse.h>
#include <api/skin/gammamgr.h>
#include <api/skin/skinelem.h>
#include <api/skin/regioncache.h>
#include <api/wnd/wndtrack.h>
#include <api/font/font.h>
#include <api/wnd/wndapi.h>
#include <api/skin/guitree.h>
#include <api/xml/xmlreader.h>
#include <api/skin/groupwndcreate.h>
#include <api/skin/groupmgr.h>
#include <api/script/script.h>
#include "bfc/ptrlist.h"
#include "bfc/memblock.h"

// -----------------------------------------------------------------------
const wchar_t XuiStatsXuiObjectStr[] = L"Wasabi:Stats"; // This is the xml tag
char XuiStatsXuiSvcName[] = "Wasabi:Stats xui object";


// -----------------------------------------------------------------------
XuiStats::XuiStats() {
  hastimer = 0;
  line = 0;
  col = 0;
  curcanvas = NULL;
}

// -----------------------------------------------------------------------
XuiStats::~XuiStats() {
  if (hastimer)
    killTimer(0x10);
}

// -----------------------------------------------------------------------
int XuiStats::onInit() {
  XUISTATS_PARENT::onInit();
  return 1;
}

// -----------------------------------------------------------------------
#define MARGIN 10
#define FONTSIZE 15
#define LINEMUL 15
#define COLMUL 200
void XuiStats::doTextOut(Canvas *canvas, const wchar_t *text, int line, int col, const Wasabi::FontInfo *fontInfo)
{
  RECT r;
  getClientRect(&r);
  if (!canvas || !text || !*text) return;
  canvas->textOutEllipsed(r.left+MARGIN+col*COLMUL, r.top+MARGIN+line*LINEMUL, COLMUL-MARGIN/2, LINEMUL, text, fontInfo);
}

// -----------------------------------------------------------------------
void XuiStats::addLine(const wchar_t *txt, const Wasabi::FontInfo *fontInfo) 
{
  if (!curcanvas) return;
  RECT r;
  getClientRect(&r);
  if (line * LINEMUL + MARGIN + FONTSIZE + r.top > r.bottom)
    { col++; line = 0; if (*txt == 0) return; }
  doTextOut(curcanvas, txt, line++, col, fontInfo);
}

// -----------------------------------------------------------------------
int XuiStats::onPaint(Canvas *canvas) 
{
  XUISTATS_PARENT::onPaint(canvas);
  curcanvas = canvas;
  line = 0;
  col = 0;

	Wasabi::FontInfo fontInfo;
	fontInfo.face = wasabi_default_fontnameW;
	fontInfo.pointSize = FONTSIZE;
	fontInfo.color = SkinColor(L"wasabi.list.text");

  addLine(L"---------------------------- System ------", &fontInfo);
  //addLine( StringPrintfW(L"entries in ptrlists : %d", ptrlist_totalnitems) );
#ifdef _DEBUG
  addLine( StringPrintfW(L"total memblocks size : %d", memblocks_totalsize) , &fontInfo);
#endif

  // TODO: add to api_timer - addLine( StringPrintfW(L"timers : %d/%d", mainmultiplex->getNumTimers(), mainmultiplex->getNumTimersLP()) , &fontInfo);

  addLine(L"", &fontInfo);
  addLine(L"----------------------------- Wnds -------", &fontInfo);
  addLine( StringPrintfW(L"rootwnds : %d", windowTracker->getNumAllWindows()) , &fontInfo);
  addLine( StringPrintfW(L"desktop rootwnds : %d", windowTracker->getNumWindows()) , &fontInfo);

  addLine(L"", &fontInfo);
  addLine(L"--------------------------- ImgLdr -------", &fontInfo);
  addLine( StringPrintfW(L"bytes in imgldr : %d", imageLoader::getMemUsage()) , &fontInfo);
  addLine( StringPrintfW(L"cached imgldr entries : %d", imageLoader::getNumCached()) , &fontInfo);
  addLine( StringPrintfW(L"region caches : %d", RegionCache::getNumCaches()) , &fontInfo);

  addLine(L"", &fontInfo);
  addLine(L"----------------------------- Skin -------", &fontInfo);
  addLine( StringPrintfW(L"skin bitmap elements : %d", WASABI_API_PALETTE->getNumBitmapElement()) , &fontInfo);
  addLine( StringPrintfW(L"skin color elements : %d", WASABI_API_PALETTE->getNumColorElements()) , &fontInfo);
  addLine( StringPrintfW(L"containers loaded : %d", SkinParser::getNumContainers()) , &fontInfo);
  addLine( StringPrintfW(L"gamma sets : %d", WASABI_API_COLORTHEMES->getNumGammaSets()) , &fontInfo);
  addLine( StringPrintfW(L"fonts : %d", Font::getNumFonts()) , &fontInfo);
  addLine( StringPrintfW(L"base textures : %d", WndApi::getNumBaseTextures()) , &fontInfo);
  addLine( StringPrintfW(L"guitree entries : %d", guiTree->getNumObject()) , &fontInfo);
  addLine( StringPrintfW(L"wndtype groups : %d", GroupWndCreateSvc::num_group_list) , &fontInfo);
  addLine( StringPrintfW(L"hosted groups : %d", GroupMgr::getNumGroups()) , &fontInfo);
  addLine( StringPrintfW(L"scripts : %d", Script::getNumScripts()) , &fontInfo);

  addLine(L"", &fontInfo);
  addLine(L"----------------------------- Misc -------", &fontInfo);
  addLine( StringPrintfW(L"registered cfgitems : %d", WASABI_API_CONFIG->config_getNumCfgItems()) , &fontInfo);

	if (WASABI_API_THREADPOOL)
	{
	addLine(L"", &fontInfo);
  addLine(L"----------------------------- ThreadPool -------", &fontInfo);
  addLine( StringPrintfW(L"active threads : %d", WASABI_API_THREADPOOL->GetNumberOfActiveThreads()) , &fontInfo);
	addLine( StringPrintfW(L"threads in pool : %d", WASABI_API_THREADPOOL->GetNumberOfThreads()) , &fontInfo);
	}

  curcanvas = NULL;
  return 1;
}

// -----------------------------------------------------------------------
void XuiStats::onSetVisible(int show) {
  XUISTATS_PARENT::onSetVisible(show);
  if (show) {
    setTimer(0x10, 250);
  } else {
    killTimer(0x10);
  }
  hastimer = show;
}

// -----------------------------------------------------------------------
void XuiStats::timerCallback(int p1) {
  if (p1 == 0x10) invalidate();
  else XUISTATS_PARENT::timerCallback(p1);
}

