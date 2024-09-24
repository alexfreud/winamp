#ifndef _METRICSCB_H
#define _METRICSCB_H

#include <bfc/dispatch.h>

// metrics codes
namespace Metric {
  enum {
    TEXTDELTA=10,
  };
};

class MetricsCallback : public Dispatchable {
public:
  int setMetric(int metricid, int param1=0, int param2=0);

  // class Dispatchable codes
  enum {
    SETMETRIC=100,
  };
};

inline int MetricsCallback::setMetric(int metricid, int param1, int param2) {
  return _call(SETMETRIC, 0, metricid, param1, param2);
}

class MetricsCallbackI : public MetricsCallback {
public:
  virtual int metricscb_setTextDelta(int delta) { return 0; }

protected:
  int mcb_setMetric(int metricid, int param1, int param2);

  RECVS_DISPATCH;
};

#endif
