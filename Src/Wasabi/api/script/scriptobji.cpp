#include "api.h"
#include <api/script/scriptobji.h>
#include <api/script/objcontroller.h>
#include <api/script/scriptguid.h>

ScriptObjectI::ScriptObjectI(const wchar_t *class_name, ScriptObjectController *object_controller)
{
	classname = class_name;
	controller = object_controller;
	cache_count = -1;
	membercachegid = -1;
	membercachesid = -1;
	ingetinterface = 0;
	vcpu_init();
}

ScriptObjectI::~ScriptObjectI()
{
	assignedVariables.deleteAll();
	memberVariables.deleteAll();
	interfaceslist.deleteAll();
	WASABI_API_MAKI->vcpu_removeScriptObject(this);
}

void *ScriptObjectI::vcpu_getInterface(GUID g, int *interfacetype)
{
	if (g == scriptObjectGuid) return this;
	InterfaceEntry *entry = 0;
	int n=0;
	while (entry = interfaceslist.enumItem(n++))
	{
		if (entry->getGuid() == g)
		{
		if (interfacetype != NULL)
			*interfacetype = entry->getType();
		return entry->getInterface();
		}
	}

	if (ingetinterface) return NULL;
	ingetinterface = 1;

	void *i = NULL;
	ScriptObjectController *c = controller;
	//CUT: ScriptObject *no = NULL;
	while (i == NULL && c != NULL)
	{
		i = c->cast(this, g);
		if (i != NULL) break;
		c = c->getAncestorController();
	}

	if (interfacetype != NULL)
		*interfacetype = INTERFACE_SCRIPTOBJECT;
	ingetinterface = 0;
	return i;
}

void *ScriptObjectI::vcpu_getInterfaceObject(GUID g, ScriptObject **o)
{
	if (g == scriptObjectGuid) return this;
	InterfaceEntry *entry = 0;
	int n=0;
	while (entry = interfaceslist.enumItem(n++))
	{
		if (entry && entry->getGuid() == g)
		{
			*o = NULL;
			return entry->getInterface();
		}
	}
	if (ingetinterface) return NULL;
	ingetinterface = 1;

	void *i = NULL;
	ScriptObjectController *c = controller;
	//CUT: ScriptObject *no = NULL;
	while (i == NULL && c != NULL)
	{
		i = c->cast(this, g);
		if (i != NULL)
		{
			if (o != NULL)
				*o = (ScriptObject *)i;
			break;
		}
		c = c->getAncestorController();
	}

	ingetinterface = 0;
	return i;
}

int ScriptObjectI::vcpu_getAssignedVariable(int start, int scriptid, int functionId, int *next, int *globalevententry, int *inheritedevent)
{
	if (start < 0) start = 0;
	if (start >= assignedVariables.getNumItems()) return -1;
	for (int i = start;i < assignedVariables.getNumItems();i++)
	{
		assvar *v = assignedVariables.enumItem(i);
		if (WASABI_API_MAKI->vcpu_getCacheCount() != cache_count)
		{
			if (!WASABI_API_MAKI->vcpu_isValidScriptId(v->scriptid))
			{
				vcpu_removeAssignedVariable(v->varid, v->scriptid);
				i--;
				continue;
			}
		}
		if (scriptid == -1 || v->scriptid == scriptid)
		{
			int r = getEventForVar(v, functionId, inheritedevent);
			if (r == -1) continue;
			if (next) *next = i + 1;
			if (globalevententry) *globalevententry = r;
			return WASABI_API_MAKI->vcpu_mapVarId(v->varid, v->scriptid);
		}
	}
	return -1;
}

void ScriptObjectI::vcpu_removeAssignedVariable(int var, int id)
{
	for (int i = 0;i < assignedVariables.getNumItems();i++)
	{
		assvar *v = assignedVariables.enumItem(i);
		if (v->varid == var && v->scriptid == id)
		{
			delete v;
			assignedVariables.removeItem(v);
			return ;
		}
	}
}

void ScriptObjectI::vcpu_addAssignedVariable(int var, int scriptid)
{
	do
	{
		assvar *v = new assvar;
		v->scriptid = scriptid;
		v->varid = var;
		assignedVariables.addItem(v);
		computeEventList(v);
		var = WASABI_API_MAKI->vcpu_getUserAncestorId(var, scriptid);
	}
	while (var != -1);
}

const wchar_t *ScriptObjectI::vcpu_getClassName()
{
	return classname;
}

ScriptObjectController *ScriptObjectI::vcpu_getController()
{
	return controller;
}

int ScriptObjectI::vcpu_getScriptId()
{
	return id;
}

void ScriptObjectI::vcpu_setScriptId(int i)
{
	id = i;
}

int ScriptObjectI::vcpu_getMember(const wchar_t *id, int scriptid, int rettype)
{
	if (membercachesid == scriptid && !WCSICMP(membercacheid, id))
		return membercachegid;
	membercacheid = id;
	membercachesid = scriptid;
	for (int i = 0;i < memberVariables.getNumItems();i++)
	{
		MemberVar *m = memberVariables.enumItem(i);
		if (m->getScriptId() == scriptid && !WCSICMP(m->getName(), id))
		{
			membercachegid = m->getGlobalId();
			return membercachegid;
		}
	}
	MemberVar *m = new MemberVar(id, scriptid, rettype);
	memberVariables.addItem(m);
	membercachegid = m->getGlobalId();
	return membercachegid;
}

void ScriptObjectI::vcpu_delMembers(int scriptid)
{
	for (int i = 0;i < memberVariables.getNumItems();i++)
		if (memberVariables.enumItem(i)->getScriptId() == scriptid)
		{
			delete memberVariables.enumItem(i);
			memberVariables.removeByPos(i--);
		}
}

void ScriptObjectI::vcpu_setInterface(GUID g, void *v, int interfacetype)
{
	for (int i = 0;i < interfaceslist.getNumItems();i++)
		if (interfaceslist.enumItem(i)->getGuid() == g)
		{
			InterfaceEntry *p = interfaceslist.enumItem(i);
			delete p;
			interfaceslist.removeByPos(i);
			i--;
		}
	interfaceslist.addItem(new InterfaceEntry(g, v, interfacetype));
}

void ScriptObjectI::vcpu_setClassName(const wchar_t *name)
{
	classname = name;
}

void ScriptObjectI::vcpu_setController(ScriptObjectController *c)
{
	controller = c;
}

void ScriptObjectI::vcpu_init()
{
	WASABI_API_MAKI->vcpu_addScriptObject(this);
}

int ScriptObjectI::getEventForVar(assvar *var, int funcid, int *inheritedevent)
{
	if (WASABI_API_MAKI->vcpu_getCacheCount() != cache_count)
	{
		for (int i = 0;i < assignedVariables.getNumItems();i++)
		{
			assvar* ass = assignedVariables.enumItem(i);
			// Martin> We need to ensure here that a valid script is called
			// There are a few circumstances where the script is already deleted from SOM but we want to call it.
			// Example: onMouseWheelDown() in a script embedded in a customobject and another script.
			// another script can hide the customobject and thus the custom object's embedded script gets unloaded.
			// the old scriptID is still cached and wants to be invoked! this will lead to an guru but the guru cannot be thrown
			// since the ScriptID isn't valid anymore. this leads to a nullpointer assert crash.
			if (!WASABI_API_MAKI->vcpu_isValidScriptId(ass->scriptid))
			{
				continue;
			}
			computeEventList(ass);
		}
		cache_count = WASABI_API_MAKI->vcpu_getCacheCount();
	}
	TList<int> *list = &var->dlfs;
	for (int i = 0;i < list->getNumItems();i += 4)
		if (list->enumItem(i) == funcid && list->enumItem(i + 1) == var->varid)
		{
			*inheritedevent = list->enumItem(i + 3);
			return list->enumItem(i + 2);
		}

	return -1;
}

void ScriptObjectI::computeEventList(assvar *a)
{
	a->dlfs.removeAll();

	int dlfid;
	int scriptid;
	int varid;

	int var = a->varid;
	int inheritedevent = 0;

	do
	{
		for (int i = 0;i < WASABI_API_MAKI->vcpu_getNumEvents();i++)
		{
			WASABI_API_MAKI->vcpu_getEvent(i, &dlfid, &scriptid, &varid);
			if (scriptid == a->scriptid && varid == var)
			{
				a->dlfs.addItem(dlfid);
				a->dlfs.addItem(varid);
				a->dlfs.addItem(i);
				a->dlfs.addItem(inheritedevent);
			}
		}
		var = WASABI_API_MAKI->vcpu_getUserAncestorId(var, a->scriptid);
		inheritedevent = 1;
	}
	while (var != -1);
}



ScriptObjectI::MemberVar::MemberVar(const wchar_t *_name, int _scriptid, int _rettype)
{
	name = _name;
	rettype = _rettype;
	scriptid = _scriptid;
	globalid = WASABI_API_MAKI->maki_createOrphan(rettype);
}

ScriptObjectI::MemberVar::~MemberVar()
{
	WASABI_API_MAKI->maki_killOrphan(globalid); // heh :)
}

