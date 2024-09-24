#error this file is going away
#if 0
#ifndef __TEXTFEEDCB_H
#define __TEXTFEEDCB_H

#include "dispatch.h"
#include "../common/common.h"

class TextFeedCallback : public Dispatchable {
  public:

    void textfeed_onReceiveText(const wchar_t *text);

  enum {
    TEXTFEEDCB_ONRECEIVETEXT = 10,
  };
};

inline void TextFeedCallback::textfeed_onReceiveText(const wchar_t *text) {
  _voidcall(TEXTFEEDCB_ONRECEIVETEXT, text);
}

class TextFeedCallbackI : public TextFeedCallback {
  public:
    TextFeedCallbackI() {}
    virtual ~TextFeedCallbackI() {}

    virtual void textfeed_onReceiveText(const wchar_t *text)=0;

  protected:
    RECVS_DISPATCH;
};
#endif
#endif
