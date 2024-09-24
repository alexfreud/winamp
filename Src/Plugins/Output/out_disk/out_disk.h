#include "../Winamp/out.h"
#include "resource.h"
extern Out_Module mod;

#include "../Agave/Language/api_language.h"
#include <api/service/waServiceFactory.h>

#define PLUGIN_VERSION L"2.18"

inline void * z_malloc(UINT x)
{
	void * p=malloc(x);
	if (p) memset(p,0,x);
	return p;
}
#if 0
#include "ssrc/ssrc.h"
#endif