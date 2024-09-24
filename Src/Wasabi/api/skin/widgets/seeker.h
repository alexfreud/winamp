#ifndef _SEEKER_H
#define _SEEKER_H

#include "../common/slider.h"

#define SEEKER_PARENT SliderWnd
class Seeker : public SliderWnd {
public:
  Seeker();
  virtual ~Seeker();

  virtual int onInit();
  virtual int onResize();

protected:
  virtual int onSetFinalPosition();

  // from BaseWnd
  virtual void timerCallback(int id);
};

#endif
