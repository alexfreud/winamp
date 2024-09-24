#ifndef __STATSWND_H
#define __STATSWND_H

#include <api/syscb/callbacks/skincb.h>

class StatsWnd : public SkinCallbackI {
	public:
		StatsWnd();
		virtual ~StatsWnd();

    virtual int skincb_onBeforeLoadingElements();

  private:
    void registerXml();

};

#endif
