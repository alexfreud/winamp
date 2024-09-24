#include <precomp.h>
#include "autoquerylist.h"
#include <api/db/subqueryserver.h>
#include <api/db/multiqueryclient.h>
#include <api/db/sharedscanner.h>
#include <bfc/string/playstring.h>
#include <api/db/metatags.h>
#include <api/wnd/fakedrag.h>
#include <bfc/util/profiler.h>
#include <bfc/file/filename.h>
#include <api/api.h>
#include <api/service/svcs/svc_droptarget.h>
#include <api/service/svc_enum.h>

#include "../../pledit/svc_pldir.h"
#include "../../pledit/playlist.h"
#include "../../pledit/editor.h"

#define TIMER_SCANNERDEL 0x6879
#define DC_REFRESH 0x6880

AutoQueryList::AutoQueryList() :
  playlist(NULL)
{
  lastpc = -1;
  last_status_update = 0;
  pldir = NULL;
  nFound = 0;
}

AutoQueryList::~AutoQueryList() {
  getGuiObject()->guiobject_removeAppCmds(this);
  SvcEnum::release(pldir);
}

int AutoQueryList::onInit() {
  AUTOQUERYLIST_PARENT::onInit();

  pldir = SvcEnumByGuid<svc_plDir>();
  if (pldir != NULL) {
    PlaylistHandle hand = pldir->insertPlaylist(NULL, "Media library query results", NULL, TRUE);
    pldir->setAutoSave(hand, FALSE);
    playlist = pldir->getPlaylist(hand);
    playlist->lock(TRUE);
  }

  appcmds_addCmd("Reset", 0, AppCmds::SIDE_RIGHT);
  getGuiObject()->guiobject_addAppCmds(this);

  postDeferredCallback(DC_REFRESH, 0);

  return 1;
}

int AutoQueryList::onDeferredCallback(intptr_t p1, intptr_t p2) {
  if (p1 == DC_REFRESH) {
    mqs_refresh();
    return 0;
  }
  return AUTOQUERYLIST_PARENT::onDeferredCallback(p1, p2);
}

void AutoQueryList::mqs_onAddPlaystring(const char *playstring, int nitems, int thispos) {
  nfound++;
  stdtimevalms now = Std::getTimeStampMS();
//  if (n > lastpc) {
  if (now - last_status_update > 0.100) {
  int n = (int)(thispos / (float)nitems * 100.0f);
    getGuiObject()->guiobject_setCompleted(n);
    getGuiObject()->guiobject_setStatusText(StringPrintf("%d%c, %d item%s found", n, '%', nfound, (nfound > 1) ? "s" : ""), TRUE);
//    lastpc = n;
//  }
    last_status_update = now;
  }
  if (playlist != NULL)
    playlist->addPlaylistItem(playstring, Playlist::APPEND, FALSE);
}

void AutoQueryList::mqs_onCompleteMultiQuery() {
  getGuiObject()->guiobject_setStatusText(StringPrintf("100%c, %d item%s found", '%', nfound, (nfound > 1) ? "s" : ""), TRUE);
  lastpc = -1;
  getGuiObject()->guiobject_popCompleted();
}

void AutoQueryList::mqs_onNewMultiQuery() {
  nfound = 0;
  getGuiObject()->guiobject_setStatusText("0%", TRUE);
  getGuiObject()->guiobject_pushCompleted();
  if (playlist != NULL) {
    playlist->deleteAll();

    GuiObject *ed = getGuiObject()->guiobject_findObjectByInterface(Editor::getInterfaceGuid());
    if (ed != NULL) {
      Editor *e = static_cast<Editor*>(ed->guiobject_getRootWnd()->getInterface(Editor::getInterfaceGuid()));
      e->setPlaylistByHandle(playlist->getHandle());
    }
  }
}

void AutoQueryList::appcmds_onCommand(int id, const RECT *buttonRect, int which_btn) {
  switch (id) {
    case 0:
      resetSubQueries();
    break;
  }
}

void AutoQueryList::onSetVisible(int v) {
  AUTOQUERYLIST_PARENT::onSetVisible(v);
  if (!v) mqs_abort();
}
