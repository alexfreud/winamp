#ifndef _PAPI_H
#define _PAPI_H

class ComponentAPI;
class WaComponent;

namespace PAPI {
  ComponentAPI *createAPI(WaComponent *, GUID owner, GUID*config=NULL);
  void destroyAPI(ComponentAPI *);
};

#endif
