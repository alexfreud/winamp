#include <precomp.h>

//<?#include "<class data="implementationheader"/>"
#include "svcenum.h"
//?>


#include <api/service/services.h>
#include <api/service/waservicefactory.h>
#include <bfc/bfc_assert.h>

SvcEnum::SvcEnum() : type(WaSvc::NONE), factory(NULL) {
  reset();
}

void *SvcEnum::_getNext(int global_lock) {
	if (WASABI_API_SVC == NULL) return NULL;
	for (;;) {
		factory = WASABI_API_SVC->service_enumService(type, pos++);
		if (factory == NULL) return NULL;
		void *s = factory->getInterface(FALSE);// get but don't lock
		if (s)
		{
			if (_testService(s)) {
				if (global_lock)
					WASABI_API_SVC->service_lock(factory, s);	// lock in sys tables
				return s;
			}
			factory->releaseInterface(s);
		}
	}
}

void SvcEnum::reset() {
	pos = 0;
	factory = NULL;
}

int SvcEnum::release(void *ptr) {
  return WASABI_API_SVC->service_release(ptr);
}

waServiceFactory *SvcEnum::getLastFactory() {
  return factory;
}
