#ifndef __WIDGETS_H
#define __WIDGETS_H

#include <wasabicfg.h>
#include <bfc/ptrlist.h>
#include <api/syscb/callbacks/skincb.h>

class waServiceFactoryI;

#ifdef WASABI_COMPILE_STATSWND
class StatsWnd;
#endif

class Widgets : public SkinCallbackI {
  public:
    Widgets();
    virtual ~Widgets();

    void registerService(waServiceFactoryI *f);
    int skincb_onBeforeLoadingElements();

		void loadResources();
  private:
    

    PtrList<waServiceFactoryI> factories;
    int count;
#ifdef WASABI_COMPILE_STATSWND
    StatsWnd *statswnd;
#endif   
};



#endif
