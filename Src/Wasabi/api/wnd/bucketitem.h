#ifndef __BUCKETITEM_H
#define __BUCKETITEM_H

#include <bfc/common.h>
#include <api/syscb/callbacks/wndcb.h>
#include <api/wnd/wndclass/buttwnd.h>
#include <bfc/depend.h>
#include <api/wnd/notifmsg.h>

template <class T> class BucketItemT : public T {
  public:
    BucketItemT(GUID g=INVALID_GUID, const wchar_t *text=NULL) : guid_target(g), target_txt(text) {
        setBorders(0);
        setHInstanceColorGroup(L"Thinger icons");
    }

    virtual ~BucketItemT() {
    }
   
    virtual void setBucketText(const wchar_t *txt) {
      notifyParent(ChildNotify::COMPONENTBUCKET_SETTEXT, reinterpret_cast<intptr_t>(txt), 0);
    }
    virtual void onLeftPush(int x, int y) {
      T::onLeftPush(x, y);
      if (guid_target != INVALID_GUID) {
        RECT r;
        getClientRect(&r);
        clientToScreen(&r);
        int newstatus = WASABI_API_WNDMGR->skinwnd_toggleByGuid(guid_target);
        setActivatedButton(newstatus);
      }
    }

    virtual int onShowWindow(GUID g, const wchar_t *groupid) {
      if (g == guid_target) setActivatedButton(1);
      return 1;
    }

    virtual int onHideWindow(GUID g, const wchar_t *groupid) {
      if (g == guid_target) setActivatedButton(0);
      return 1;
    }

    virtual void onEnterArea() {
      T::onEnterArea();
      if (!target_txt.isempty()) setBucketText(target_txt);
    }

    virtual void onLeaveArea() {
      T::onLeaveArea();
      if (!target_txt.isempty()) setBucketText(NULL);
    }

    void setAutoOpen(GUID g) {
      guid_target = g;
    }

    void setAutoText(const wchar_t *txt) {
      target_txt = txt;
    }

  private:
    GUID guid_target;
    StringW target_txt;
};

#define BUCKETITEM_PARENT ButtonWnd
class BucketItem : public BucketItemT<BUCKETITEM_PARENT> {
  public:
    BucketItem(GUID g=INVALID_GUID, const wchar_t *text=NULL) : BucketItemT<ButtonWnd> (g, text) {}
    virtual ~BucketItem() {}
};


#endif // __BUCKETITEM_H

