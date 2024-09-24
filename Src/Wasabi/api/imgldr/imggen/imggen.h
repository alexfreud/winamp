#ifndef _IMGGEN_H
#define _IMGGEN_H

#include "../studio/wac.h"

#define WACNAME WACimggen
class WACNAME : public WAComponentClient {
public:
  WACNAME();

  virtual const char *getName() { return "Standard Image Generators"; };
  virtual GUID getGUID();
};
#endif
