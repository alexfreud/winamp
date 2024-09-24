#include <precomp.h>
#include "wa2pledit.h"
#include <api/script/objects/PlaylistScriptObject.h>
#include <tataki/color/skinclr.h>
#include "wa2frontend.h"
#include <tataki/canvas/bltcanvas.h>
#include "../nu/AutoWide.h"
#ifndef _WASABIRUNTIME

BEGIN_SERVICES(Wa2Pledit_Svc);
DECLARE_SERVICE(XuiObjectCreator<Wa2PleditXuiSvc>);
END_SERVICES(Wa2Pledit_Svc, _Wa2Pledit_Svc);

#ifdef _X86_
extern "C" { int _link_Wa2PleditXuiSvc; }
#else
extern "C" { int __link_Wa2PleditXuiSvc; }
#endif

#endif

// -----------------------------------------------------------------------
const wchar_t Wa2PleditXuiObjectStr[] = L"PlaylistEditor"; // This is the xml tag
char Wa2PleditXuiSvcName[] = "Playlist Editor xui object";

// -----------------------------------------------------------------------

#define TRACKLENGTH_WIDTH 40
#define DC_CURRENTIDX 10
#define DC_PLAY 11
#define TIMER_REFRESHLIST 12

static SkinColor text(L"pledit.text"), rotext(L"pledit.text.readonly");
static SkinColor text_disabled(L"pledit.text.disabled");
static SkinColor bgcolor(L"pledit.bgcolor");
static SkinColor currenttext(L"wasabi.itemlist.outline.focus");
static SkinColor currentoutline(L"wasabi.itemlist.outline.current");

PtrList<Wa2PlaylistEditor> Wa2PlaylistEditor::editors;

COLORREF Wa2PlaylistEditor::getTextColor(LPARAM lParam) {
//  if (playlist == NULL) return text;
//  PlaylistEntry *entry = playlist->enumEntry(lParam);
//  if (entry && entry->getCurrent())
//    return currenttext;
//  else
  if (lParam == cur_index) 
    return currenttext;
  return text;
}

COLORREF Wa2PlaylistEditor::getBgColor(LPARAM lParam) {
  return bgcolor;
}

COLORREF Wa2PlaylistEditor::getFocusRectColor(LPARAM lParam) {
  return currentoutline;
}

int Wa2PlaylistEditor::needFocusRect(LPARAM lParam) 
{
  return lParam == cur_index;
}

Wa2PlaylistEditor::Wa2PlaylistEditor()
: curplaylist(0)
{
  curplaylist = NULL;
  cur_index = wa2.PE_getCurrentIndex()+1;
}

Wa2PlaylistEditor::~Wa2PlaylistEditor() {
  killTimer(TIMER_REFRESHLIST);
  editors.removeItem(this);
}

int Wa2PlaylistEditor::onInit()
{
  WA2PLAYLISTEDITOR_PARENT::onInit();
  setShowColumnsHeaders(FALSE);
  addColumn(L"Track #", calcTrackNumWidth());
  addColumn(L"Track Name", 200);
  addColumn(L"Length", TRACKLENGTH_WIDTH, 1, COL_RIGHTALIGN);
  editors.addItem(this);
  return 1;
}

void Wa2PlaylistEditor::onVScrollToggle(int set) {
  resizeCols();
}

int Wa2PlaylistEditor::onResize() {
  WA2PLAYLISTEDITOR_PARENT::onResize();
  resizeCols();
  return 1;
}

void Wa2PlaylistEditor::resizeCols() {
  RECT r;
  getClientRect(&r);
  ListColumn *c0 = getColumn(0);
  int tw = calcTrackNumWidth();
  c0->setWidth(tw);
  ListColumn *c1 = getColumn(1);
  c1->setWidth(r.right-r.left - tw - TRACKLENGTH_WIDTH);
}

int Wa2PlaylistEditor::calcTrackNumWidth() 
{
  WndCanvas bc(this);
	Wasabi::FontInfo fontInfo;
	fontInfo.pointSize = getFontSize();
  int cw = getFontSize();
  int dw = 8;
  bc.getTextExtent(L"W", &cw, NULL, &fontInfo);
  bc.getTextExtent(L".", &dw, NULL, &fontInfo);
  int n = getNumItems();
  if (n < 0) return dw+cw;
  float f = log10((float)n);
  int l = (int)ceil(f);
  return (l * cw) + dw + 5;
}

void *Wa2PlaylistEditor::getInterface(GUID interface_guid) {
  if (interface_guid == Wa2PlaylistEditor::getInterfaceGuid()) {
    return this;
  }
  return WA2PLAYLISTEDITOR_PARENT::getInterface(interface_guid);
}

void Wa2PlaylistEditor::setPlaylist(Wa2Playlist *playlist) {
  if (curplaylist == playlist) 
    return;
  curplaylist = playlist;
  loadList();
}

void Wa2PlaylistEditor::_loadList() {
  int y = getScrollY();

  setRedraw(FALSE);

  deleteAllItems();

  if (curplaylist == (Wa2Playlist *)-1) {

    // load the current playlist (the one in the real playlist editor)
    int n = wa2.PE_getNumItems();

    for (int i=0;i<n;i++) 
		{
      fileinfo2 *fi = wa2.PE_getFileTitle(i);
      addItem(StringPrintfW(L"%d.", i+1), i+1);
      setSubItem(i, 1, AutoWide(fi->filetitle));
      setSubItem(i, 2, AutoWide(fi->filelength));
    }
  } else {
    // load an available playlist
  }

  RECT r;
  getClientRect(&r);
  int ch = getContentsHeight();
  if (ch - y >= r.bottom-r.top) scrollToY(y);
  else if (ch - r.bottom-r.top >= 0) scrollToY(ch - r.bottom-r.top);
  else scrollToY(0);

  resizeCols();
  
  postDeferredCallback(DC_CURRENTIDX);
  setRedraw(TRUE);
}

void Wa2PlaylistEditor::_onNewCurrentIndex(int idx) 
{
  foreach(editors)
    editors.getfor()->onNewCurrentIndex(idx);
  endfor
}

void Wa2PlaylistEditor::onNewCurrentIndex(int idx) 
{
	if (idx!=cur_index)
	{
		int oldIdx = cur_index;
		cur_index = idx;
		invalidateItem(oldIdx);
		invalidateItem(idx);
	}
}

void Wa2PlaylistEditor::onSetVisible(int show) {
  WA2PLAYLISTEDITOR_PARENT::onSetVisible(show);
  if (!show) getParent()->setFocus();
}

void Wa2PlaylistEditor::onPlaylistModified() {
  if (curplaylist == (Wa2Playlist *)-1)
    loadList();
}

void Wa2PlaylistEditor::_onPlaylistModified() {
	// fist call the Pledit script object
	SPlaylist::onPleditModified();
	// than all our old pledit xml objects
	foreach(editors)
		editors.getfor()->onPlaylistModified();
	endfor
}

int Wa2PlaylistEditor::onDeferredCallback(intptr_t p1, intptr_t p2) 
{
  if (p1 == DC_CURRENTIDX) 
	{
    onNewCurrentIndex(wa2.PE_getCurrentIndex()+1);
    return 1;
  }
  return WA2PLAYLISTEDITOR_PARENT::onDeferredCallback(p1, p2);
}

void Wa2PlaylistEditor::onDoubleClick(int itemnum) 
{
  WA2PLAYLISTEDITOR_PARENT::onDoubleClick(itemnum);
  wa2.PE_setCurrentIndex(itemnum);
  wa2.userButton(WA2_USERBUTTON_PLAY, 0);
  _onNewCurrentIndex(itemnum+1);
}

void Wa2PlaylistEditor::timerCallback(int id) 
{
  if (id == TIMER_REFRESHLIST) {
    killTimer(TIMER_REFRESHLIST);
    _loadList();
  }
}

void Wa2PlaylistEditor::loadList() 
{
  killTimer(TIMER_REFRESHLIST);
  setTimer(TIMER_REFRESHLIST, 500);
}
