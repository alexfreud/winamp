//#include <precomp.h>
#include "./api.h"
#include <api/wnd/api_wnd.h>
#include "tooltip.h"

#ifdef WASABI_COMPILE_CONFIG
#include <api/config/items/attrint.h>
#include <api/config/items/cfgitem.h>
#endif
#include <api/service/svc_enum.h>

Tooltip::Tooltip(const wchar_t *txt)
{
	WASABI_API_WND->appdeactivation_push_disallow(NULL);
	svc = NULL;

	if (!txt || !*txt) return ;

#ifdef WASABI_COMPILE_CONFIG
	// {9149C445-3C30-4e04-8433-5A518ED0FDDE}
	const GUID uioptions_guid =
	    { 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };
	if (!_intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid), L"Enable tooltips"))
	{
		// tooltips disabled
		return ;
	}
#endif

	waServiceFactory *svf = WASABI_API_SVC->service_enumService(WaSvc::TOOLTIPSRENDERER, 0);
	svc = castService<svc_toolTipsRenderer>(svf);

	if (!svc)
	{
		// no tooltips available!
		return ;
	}

	svc->spawnTooltip(txt);
}

Tooltip::~Tooltip()
{
	if (svc != NULL)
		WASABI_API_SVC->service_release(svc);
	WASABI_API_WND->appdeactivation_pop_disallow(NULL);
}