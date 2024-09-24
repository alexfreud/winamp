#include <precomp.h>
#include "appcmds.h"

#define CBCLASS AppCmdsI
START_DISPATCH;
CB(APPCMDS_GETNUMCMDS, appcmds_getNumCmds);
CB(APPCMDS_ENUMCMD, appcmds_enumCmd);
VCB(APPCMDS_ONCOMMAND, appcmds_onCommand);
END_DISPATCH;
#undef CBCLASS

AppCmdsI::~AppCmdsI()
{
	foreach(cmds)
	if (cmds.getfor()->autodelete)
		delete cmds.getfor();
	endfor
}

void AppCmdsI::appcmds_addCmd(CmdRec *cmdrec)
{
	cmds.addItem(cmdrec);
}

void AppCmdsI::appcmds_addCmd(const wchar_t *name, int id, int side)
{
	cmds.addItem(new CmdRec(name, id, side, TRUE));
}

void AppCmdsI::appcmds_deleteAll()
{
	foreach(cmds)
	if (cmds.getfor()->autodelete) delete cmds.getfor();
	endfor
	cmds.removeAll();
}

int AppCmdsI::appcmds_getNumCmds()
{
	return cmds.getNumItems();
}

const wchar_t *AppCmdsI::appcmds_enumCmd(int n, int *side, int *id)
{
	CmdRec *cr = cmds[n];
	if (cr == NULL) return NULL;
	if (side != NULL) *side = cr->side;
	if (id != NULL) *id = cr->id;
	return cr->cmdname;
}

void AppCmdsI::appcmds_onCommand(int id, const RECT *buttonRect, int which_button)
{
	foreach(cmds)
	if (cmds.getfor()->id == id)
	{
		cmds.getfor()->onCommand(buttonRect, which_button);
		break;
	}
	endfor
}
