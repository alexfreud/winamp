#ifndef NULLSOFT_FREETYPE_H
#define NULLSOFT_FREETYPE_H

#include <api/wac/wac.h>

#define WACNAME WACFreetype
#define WACPARENT WAComponentClient

class WACNAME : public WACPARENT 
{
public:
  WACNAME();
  virtual ~WACNAME();

  virtual GUID getGUID();

  virtual void onRegisterServices();
  virtual void onCreate();
  virtual void onDestroy();
private:
};

extern WACPARENT *the;

#endif
