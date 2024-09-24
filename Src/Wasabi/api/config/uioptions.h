#ifndef _UIOPTIONS_H
#define _UIOPTIONS_H

#include <api/config/items/cfgitemi.h>

#define UIOPTIONS_PARENT CfgItemI
class UIOptions : public UIOPTIONS_PARENT {
public:
  UIOptions(const wchar_t *name=NULL);
  static void onTimerRefreshRate(int rate);
};

#endif
