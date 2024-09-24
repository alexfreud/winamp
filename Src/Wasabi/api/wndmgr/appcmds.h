#ifndef _APPCMDS_H
#define _APPCMDS_H

#include <bfc/dispatch.h>
#include <bfc/depend.h>
#include <bfc/common.h>
#include <bfc/string/StringW.h>
#include <bfc/ptrlist.h>

class DragItem;

// this will be fully dispatched later on
class AppCmds : public Dispatchable
{
public:

	int appcmds_getNumCmds();

	const wchar_t *appcmds_enumCmd(int n, int *side, int *id);

	enum {
	    SIDE_LEFT = 0, SIDE_RIGHT = 1
	};

	enum {
	    LEFT_CLICK = 0,
	    RIGHT_CLICK = 1,
	};

	void appcmds_onCommand(int id, const RECT *buttonRect, int which_button); //onscreen coords

	enum {
	    APPCMDS_GETNUMCMDS = 100,
	    APPCMDS_ENUMCMD = 200,
	    APPCMDS_ONCOMMAND = 300,
	};
};

inline int AppCmds::appcmds_getNumCmds()
{
	return _call(APPCMDS_GETNUMCMDS, 0);
}

inline const wchar_t *AppCmds::appcmds_enumCmd(int n, int *side, int *id)
{
	return _call(APPCMDS_ENUMCMD, (const wchar_t *)NULL, n, side, id);
}

inline void AppCmds::appcmds_onCommand(int id, const RECT *buttonRect, int which_button)
{
	_voidcall(APPCMDS_ONCOMMAND, id, buttonRect, which_button);
}

class CmdRec
{
public:
	CmdRec(const wchar_t *name, int _id, int _side, int _autodelete = 0) : cmdname(name), id(_id), side(_side), autodelete(_autodelete) {}
	virtual ~CmdRec() {}
	StringW cmdname;
	int id, side;
	int autodelete;

	virtual void onCommand(const RECT *buttonRect, int which_button) {}}
;

class AppCmdsI : public AppCmds
{
public:
	AppCmdsI() { }
	virtual ~AppCmdsI();

protected:
	void appcmds_addCmd(CmdRec *cmdrec);
	void appcmds_addCmd(const wchar_t *name, int id, int side = SIDE_LEFT);
	void appcmds_deleteAll(); //calls delete on each one

public:
	virtual int appcmds_getNumCmds();
	virtual const wchar_t *appcmds_enumCmd(int n, int *side, int *id);

	// override this and catch your commands, otherwise it'll call the CmdRec
	virtual void appcmds_onCommand(int id, const RECT *buttonRect, int which_button);

protected:
	RECVS_DISPATCH;
	PtrList<CmdRec> cmds;
};

#endif
