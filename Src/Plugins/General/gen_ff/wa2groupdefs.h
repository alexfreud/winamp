#ifndef __WA2GROUPDEFS_H
#define __WA2GROUPDEFS_H

#include <api/syscb/callbacks/skincb.h>

//-----------------------------------------------------------------------------------------------

class Wa2Groupdefs : public SkinCallbackI {
  public:
    Wa2Groupdefs();
    virtual ~Wa2Groupdefs();

    int skincb_onBeforeLoadingElements();

  private:
};

#endif
