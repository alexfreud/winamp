#include "precomp.h"
#include "mbsvc.h"
#include "svc.h"
#include "../studio/genwnd.h"

static WACNAME wac;
WAComponentClient *the = &wac;
WACNAME *wacmb = &wac;

#include "../studio/services/servicei.h"

// {181BE599-2249-4a1c-8283-4EE85FE8EC86}
static const GUID guid = 
{ 0x181be599, 0x2249, 0x4a1c, { 0x82, 0x83, 0x4e, 0xe8, 0x5f, 0xe8, 0xec, 0x86 } };

WACNAME::WACNAME() {
  registerService(new waServiceFactoryT<svc_miniBrowser, MbSvc>);
}

WACNAME::~WACNAME() {
}

GUID WACNAME::getGUID() {
  return guid;
}

