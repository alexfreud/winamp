#include "precomp.h"
#include "imggen.h"

#include "solid.h"
#include "grad.h"
#include "osedge.h"
#include "poly.h"
#include "shadowwnd.h"

#include "../studio/services/servicei.h"

static WACNAME wac;
WAComponentClient *the = &wac;

// {9C9CB15E-2904-4df2-B8CE-FFBC6CD230DC}
static const GUID guid = 
{ 0x9c9cb15e, 0x2904, 0x4df2, { 0xb8, 0xce, 0xff, 0xbc, 0x6c, 0xd2, 0x30, 0xdc } };

WACNAME::WACNAME() {
  registerService(new waServiceFactoryTSingle<svc_imageGenerator, SolidImage>);
  registerService(new waServiceFactoryTSingle<svc_imageGenerator, GradientImage>);
  registerService(new waServiceFactoryTSingle<svc_imageGenerator, OsEdgeImage>);
  registerService(new waServiceFactoryTSingle<svc_imageGenerator, PolyImage>);
  registerService(new XuiObjectCreator<XuiShadowWndSvc>);
}

GUID WACNAME::getGUID() {
  return guid;
}
