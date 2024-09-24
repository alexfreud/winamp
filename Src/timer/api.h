#ifndef NULLSOFT_API_TIMER_H
#define NULLSOFT_API_TIMER_H

#include <api/config/api_config.h>
#define WASABI_API_CONFIG configApi

#include <api/syscb/api_syscb.h>
#define WASABI_API_SYSCB sysCallbackApi

#include <api/script/api_maki.h>
#define WASABI_API_MAKI makiApi

#include "timerapi.h"
extern TimerApi *timer_svc;
#define WASABI_API_TIMER timer_svc

#endif // !NULLSOFT_API_TIMER_H
