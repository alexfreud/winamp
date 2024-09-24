#include "precomp.h"
#include "wndcb.h"

using namespace WndCallback;

int WndCallbackI::syscb_notify(int msg, intptr_t param1, intptr_t param2)
{
	switch (msg)
	{
	case SHOWWINDOW:
		{
			WndInfo *i = reinterpret_cast<WndInfo *>((void *)param1);
			if (!i) return 0;
			return onShowWindow(i->c, i->guid, i->groupid);
		}
	case HIDEWINDOW:
		{
			WndInfo *i = reinterpret_cast<WndInfo *>((void *)param1);
			if (!i) return 0;
			return onHideWindow(i->c, i->guid, i->groupid);
		}
	case GROUPCHANGE:
		{
			WndInfo *i = reinterpret_cast<WndInfo *>((void *)param1);
			if (!i) return 0;
			return onGroupChange(i->groupid);
		}
	case TYPECHANGE:
		{
			WndInfo *i = reinterpret_cast<WndInfo *>((void *)param1);
			if (!i) return 0;
			return onTypeChange(i->wndtype);
		}
	}
	return 0;
}
