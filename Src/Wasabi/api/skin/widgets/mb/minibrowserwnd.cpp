#include <precomp.h>
#include <api/skin/widgets/mb/minibrowserwnd.h>
#include <api/skin/widgets/mb/minibrowser.h>
#include <api/service/svcs/svc_minibrowser.h>
#include <api/service/svc_enum.h>

MiniBrowserWnd::MiniBrowserWnd(GUID mb_provider) {
  mb = NULL;
  mbsvc = MiniBrowserSvcEnum(mb_provider).getFirst();
}

MiniBrowserWnd::~MiniBrowserWnd() {
  if (mbsvc) {
    if (mb) mbsvc->destroyMiniBrowser(mb);
    WASABI_API_SVC->service_release(mbsvc);  
  }
}

int MiniBrowserWnd::onInit() {
  int r = MBWND_PARENT::onInit();
  if (mb) {
    mb->minibrowser_getRootWnd()->setStartHidden(1);
    mb->minibrowser_getRootWnd()->setParent(this);
    r &= mb->minibrowser_getRootWnd()->init(this);
  }
  onSetVisible(1);
  return r;
}

void MiniBrowserWnd::onSetVisible(int i) {
  MBWND_PARENT::onSetVisible(i);
  if (i) {
    if (!mb && mbsvc) {
      mb = mbsvc->createMiniBrowser();
      if (mb) {
        mb->minibrowser_addCB(this);
        mb->minibrowser_getRootWnd()->setStartHidden(1);
        mb->minibrowser_getRootWnd()->setParent(this);
        mb->minibrowser_getRootWnd()->init(this);
        if (isPostOnInit())
          onResize();
      }
    }
  } else {
    if (mb) {
      mbsvc->destroyMiniBrowser(mb);
      mb = NULL;
    }
  }
  if (mb && mb->minibrowser_getRootWnd()) {
    mb->minibrowser_getRootWnd()->setVisible(i);
  }
}

int MiniBrowserWnd::onResize() {
  int r = MBWND_PARENT::onResize();
  if (mb && mb->minibrowser_getRootWnd()) {
    RECT r;
    getClientRect(&r);
    mb->minibrowser_getRootWnd()->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
  }
  return r;
}


int MiniBrowserWnd::handleDesktopAlpha() {
  if (mb && mb->minibrowser_getRootWnd()) return mb->minibrowser_getRootWnd()->handleDesktopAlpha();
  return 0;
}

int MiniBrowserWnd::handleRatio() { 
  if (mb && mb->minibrowser_getRootWnd()) return mb->minibrowser_getRootWnd()->handleRatio();
  return 0; 
}

int MiniBrowserWnd::navigateUrl(const wchar_t *url) {
  if (mb) return mb->minibrowser_navigateUrl(url);
  return 0;
}

int MiniBrowserWnd::back() {
  if (mb) return mb->minibrowser_back();
  return 0;
}

int MiniBrowserWnd::forward() {
  if (mb) return mb->minibrowser_forward();
  return 0;
}

int MiniBrowserWnd::home() {
  if (mb) return mb->minibrowser_home();
  return 0;
}

int MiniBrowserWnd::refresh() {
  if (mb) return mb->minibrowser_refresh();
  return 0;
}

int MiniBrowserWnd::stop() {
  if (mb) return mb->minibrowser_stop();
  return 0;
}

void MiniBrowserWnd::setTargetName(const wchar_t *name) {
  if (mb) mb->minibrowser_setTargetName(name);
}

const wchar_t *MiniBrowserWnd::getTargetName() {
  if (mb) return mb->minibrowser_getTargetName();
  return NULL;
}

const wchar_t *MiniBrowserWnd::getCurrentUrl() {
  if (mb) return mb->minibrowser_getCurrentUrl();
  return NULL;
}

int MiniBrowserWnd::onBeforeNavigate(const wchar_t *url, int flags, const wchar_t *frame) {
  return 0; // return 1 to cancel navigation
}

void MiniBrowserWnd::onDocumentComplete(const wchar_t *url) {
}

void MiniBrowserWnd::onDocumentReady(const wchar_t *url) {
}

void MiniBrowserWnd::onNavigateError(const wchar_t *url, int status) {
}

void MiniBrowserWnd::onMediaLink(const wchar_t *url) {
}

const wchar_t* MiniBrowserWnd::messageToMaki(wchar_t* str1, wchar_t* str2, int i1, int i2, int i3)
{
	return 0;
}

int MiniBrowserWnd::minibrowsercb_onBeforeNavigate(const wchar_t *url, int flags, const wchar_t *frame) {
  return onBeforeNavigate(url, flags, frame);
}

void MiniBrowserWnd::minibrowsercb_onDocumentComplete(const wchar_t *url) {
  onDocumentComplete(url);
}

void MiniBrowserWnd::minibrowsercb_onDocumentReady(const wchar_t *url) {
  onDocumentReady(url);
}

void MiniBrowserWnd::minibrowsercb_onMediaLink(const wchar_t *url) {
  onMediaLink(url);
}

void MiniBrowserWnd::minibrowsercb_onNavigateError(const wchar_t *url, int status) {
  onNavigateError(url, status);
}

const wchar_t* MiniBrowserWnd::minibrowsercb_messageToMaki(wchar_t* str1, wchar_t* str2, int i1, int i2, int i3)
{
	return messageToMaki(str1, str2, i1, i2, i3);
}

void MiniBrowserWnd::setScrollbarsFlag(int a) {
  if (mb) mb->minibrowser_setScrollbarsFlag(a);
}

MiniBrowser *MiniBrowserWnd::getBrowser() {
  return mb;
}

