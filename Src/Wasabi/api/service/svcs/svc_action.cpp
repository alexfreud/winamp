#include <precomp.h>

#include "svc_action.h"

#define CBCLASS svc_actionI
START_DISPATCH;
CB(HASACTION, hasAction);
CB(ONACTION, onAction);
END_DISPATCH;
#undef CBCLASS

svc_actionI::~svc_actionI()
{
	actions.deleteAll();
}

void svc_actionI::registerAction(const wchar_t *actionid, int pvtid)
{
	ASSERT(actionid != NULL);
	actions.addItem(new ActionEntry(actionid, pvtid));
}

int svc_actionI::hasAction(const wchar_t *name)
{
	if (name == NULL) return FALSE;
	return (actions.findItem(name) != NULL);
}

int svc_actionI::onAction(const wchar_t *action, const wchar_t *param, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source)
{
	if (action == NULL) return 0;
	int pos = -1;
	if (actions.findItem(action, &pos))
	{
		return onActionId(actions.enumItem(pos)->getId(), action, param, p1, p2, data, datalen, source);
	}
	return 0;
}