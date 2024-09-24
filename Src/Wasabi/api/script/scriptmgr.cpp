#include <precomp.h>
#include <bfc/wasabi_std.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/objects/systemobj.h>
#include <api/script/vcpu.h>
#include <api/skin/skinparse.h>
#include <api/script/objecttable.h>
#ifdef WASABI_COMPILE_WND
#include <api/wnd/wndtrack.h>
#endif

#ifdef WASABI_COMPILE_COMPONENTS
PtrList<WACObject> comps;
#endif

extern GUID baseGUID;

ScriptObjectManager::ScriptObjectManager() {
	ASSERTPR(!inited, "don't create 2 scriptobjectmanager, you dumbass");
	inited=1;
#ifdef WASABI_COMPILE_COMPONENTS
	for (int i=0;i<api->getNumComponents();i++) {
		GUID g = api->getComponentGUID(i);
		WACObject *c = new WACObject();
		c->setGUID(g);
		comps.addItem(c);
	}
	WACObject *m = new WACObject();
	m->setGUID(baseGUID);
	comps.addItem(m);
#endif
}

ScriptObjectManager::~ScriptObjectManager() {
	inited=0;
#ifdef WASABI_COMPILE_COMPONENTS
	comps.deleteAll();
#endif
}

// Create a variable of specified type
scriptVar ScriptObjectManager::makeVar(int type) 
{
	scriptVar v;
	v.type = type;
	v.data.ddata = 0;
	return v;
}

// Create a variable of specified type and assigns an object to it
scriptVar ScriptObjectManager::makeVar(int type, ScriptObject *o) 
{
	scriptVar v = makeVar(type);
	v.data.odata = o;
	return v;
}

// Assigns a char* to a String variable. Frees old value if necessary
void ScriptObjectManager::assign(scriptVar *v, const wchar_t *str) 
{
	ASSERT(v != NULL);
	ASSERT(v->type == SCRIPT_STRING); // Compiler discarded
	if (v->data.sdata) FREE((wchar_t *)v->data.sdata);
	if ((int)str > 65536) {
	    v->data.sdata = WCSDUP(str);
		VCPU::addStatementString(const_cast<wchar_t *>(v->data.sdata));
	} else
		v->data.sdata = NULL;
}

// Assigns an int to an int, float or double variable. 
void ScriptObjectManager::assign(scriptVar *v, int i) {
	ASSERT(v != NULL);
	switch (v->type) {
		case SCRIPT_FLOAT:
			assign(v, (float)i);
			return;
		case SCRIPT_DOUBLE:
			assign(v, (double)i);
			return;
		case SCRIPT_INT:
		case SCRIPT_BOOLEAN:
			v->data.idata = i;
			return;
		case SCRIPT_STRING:
			assign(v, StringPrintfW(L"%d", i));
			return;
		default:
			assign(v, (ScriptObject*)NULL);
			return;
	}
}

// Assigns a float to an int, float or double variable. 
void ScriptObjectManager::assign(scriptVar *v, float f) {
	ASSERT(v != NULL);
	switch (v->type) {
		case SCRIPT_INT:
		case SCRIPT_BOOLEAN:
			assign(v, (int)f);
			return;
		case SCRIPT_DOUBLE:
			assign(v, (double)f);
			return;
		case SCRIPT_FLOAT:
			v->data.fdata = f;
			return;
		case SCRIPT_STRING:
		{
			wchar_t t[96] = {0};
			WCSNPRINTF(t, 96, L"%f", f);
			assign(v, t);
			return;
		}
		default:
			assign(v, (ScriptObject*)NULL);
			return;
	}
}

// Assigns a double to an int, float or double variable. 
void ScriptObjectManager::assign(scriptVar *v, double d) {
	ASSERT(v != NULL);
	switch (v->type) {
		case SCRIPT_INT:
		case SCRIPT_BOOLEAN:
			assign(v, (int)d);
			return;
		case SCRIPT_FLOAT:
			assign(v, (float)d);
			return;
		case SCRIPT_DOUBLE:
			v->data.ddata = d;
			return;
		case SCRIPT_STRING:
		{
			wchar_t t[96] = {0};
			WCSNPRINTF(t, 96, L"%e", d);
			assign(v, t);
			return;
		}
		default:
			assign(v, (ScriptObject*)NULL);
			return;
	}
}

// Assigns a object to an object variable, handles hierarchy.
void ScriptObjectManager::assign(scriptVar *v, ScriptObject *o) {
	ASSERT(v != NULL);
	// TODO: temporarily assert descendancy
	v->data.odata = o;
}


// Assigns a numerical scriptVar to another numerical scriptVar
// or an object to another object
// Autocasts
void ScriptObjectManager::assign(scriptVar *v1, scriptVar *v2) {
	ASSERT(v1 != NULL);
	ASSERT(v2 != NULL);
	switch (v1->type) {
		case SCRIPT_INT:
			assign(v1, SOM::makeInt(v2));
			break;
		case SCRIPT_FLOAT:
			assign(v1, SOM::makeFloat(v2));
			break;
		case SCRIPT_DOUBLE:
			assign(v1, SOM::makeDouble(v2));
			break;
		case SCRIPT_STRING:
			assign(v1, v2->data.sdata);
			break;
		case SCRIPT_BOOLEAN:
			assign(v1, SOM::makeBoolean(v2));
			break;
		default:
			assign(v1, v2->data.odata);
			break;
	}
}

void ScriptObjectManager::assignPersistent(scriptVar *v1, scriptVar *v2) {
	ASSERT(v1 != NULL);
	ASSERT(v2 != NULL);
	switch (v1->type) {
		case SCRIPT_INT:
			assign(v1, SOM::makeInt(v2));
			break;
		case SCRIPT_FLOAT:
			assign(v1, SOM::makeFloat(v2));
			break;
		case SCRIPT_DOUBLE:
			assign(v1, SOM::makeDouble(v2));
			break;
		case SCRIPT_STRING:
			persistentstrassign(v1, v2->data.sdata);
			break;
		case SCRIPT_BOOLEAN:
			assign(v1, SOM::makeBoolean(v2));
			break;
		default:
			assign(v1, v2->data.odata);
			break;
	}
}

void ScriptObjectManager::strflatassign(scriptVar *v, const wchar_t *str) 
{
	ASSERT(v != NULL);
	ASSERT(v->type == SCRIPT_STRING); // Compiler discarded
	if (v->data.sdata) FREE((wchar_t *)v->data.sdata);
	if ((int)str > 65536)
		v->data.sdata = (wchar_t *)str;
	else
		v->data.sdata = NULL;
}

void ScriptObjectManager::persistentstrassign(scriptVar *v, const wchar_t *str) {
	ASSERT(v != NULL);
	ASSERT(v->type == SCRIPT_STRING); // Compiler discarded
	if (v->data.sdata) FREE((wchar_t *)v->data.sdata);
	if ((int)str > 65536)
		v->data.sdata = WCSDUP(str);
	else
		v->data.sdata = NULL;
}

// comparision functions
int ScriptObjectManager::compEq(scriptVar *v1, scriptVar *v2) {
	int r;
	switch (v1->type) {
		case SCRIPT_INT:
			r = v1->data.idata == makeInt(v2);
			break;
		case SCRIPT_FLOAT:
			r = v1->data.fdata == makeFloat(v2);
			break;
		case SCRIPT_DOUBLE:
			r = v1->data.ddata == makeDouble(v2);
			break;
		case SCRIPT_BOOLEAN:
			r = ((v1->data.idata == 0) ? 0 : 1) == ((makeBoolean(v2) == 0) ? 0 : 1);
			break;
		case SCRIPT_STRING:
			if (v1->data.sdata && v2->data.sdata)
			{
				r = !wcscmp(v1->data.sdata, v2->data.sdata);
				break;
			}
			// pass through
		default: // any object, reference has to match
			r = v1->data.odata == v2->data.odata;
		break;
	}
	return r;
}

int ScriptObjectManager::compNeq(scriptVar *v1, scriptVar *v2) {
	int r;
	switch (v1->type) {
		case SCRIPT_INT:
			r = v1->data.idata != makeInt(v2);
			break;
		case SCRIPT_FLOAT:
			r = v1->data.fdata != makeFloat(v2);
			break;
		case SCRIPT_DOUBLE:
			r = v1->data.ddata != makeDouble(v2);
			break;
		case SCRIPT_BOOLEAN:
			r = ((v1->data.idata == 0) ? 0 : 1) != ((makeBoolean(v2) == 0) ? 0 : 1);
			break;
		case SCRIPT_STRING:
			if (v1->data.sdata && v2->data.sdata)
				r = (wcscmp(v1->data.sdata, v2->data.sdata) != 0) ? 1 : 0;
			else
				r = 0;
			break;
		default:
			r = v1->data.odata != v2->data.odata;
			break;
	}
	return r;
}

int ScriptObjectManager::compA(scriptVar *v1, scriptVar *v2) {
	int r;
	switch (v1->type) {
		case SCRIPT_BOOLEAN:
			r = ((v1->data.idata == 0) ? 0 : 1) > ((makeBoolean(v2) == 0) ? 0 : 1);
			break;
		case SCRIPT_INT:
			r = v1->data.idata > makeInt(v2);
			break;
		case SCRIPT_FLOAT:
			r = v1->data.fdata > makeFloat(v2);
			break;
		case SCRIPT_DOUBLE:
			r = v1->data.ddata > makeDouble(v2);
			break;
		case SCRIPT_STRING:
			r = (wcscmp(v1->data.sdata, v2->data.sdata) > 0) ? 1 : 0;
			break;
		default:
			r = 0;
			break;
	}
	return r;
}

int ScriptObjectManager::compAe(scriptVar *v1, scriptVar *v2) {
	int r;
	switch (v1->type) {
		case SCRIPT_BOOLEAN:
			r = ((v1->data.idata == 0) ? 0 : 1) >= ((makeBoolean(v2) == 0) ? 0 : 1);
			break;
		case SCRIPT_INT:
			r = v1->data.idata >= makeInt(v2);
			break;
		case SCRIPT_FLOAT:
			r = v1->data.fdata >= makeFloat(v2);
			break;
		case SCRIPT_DOUBLE:
			r = v1->data.ddata >= makeDouble(v2);
			break;
		case SCRIPT_STRING:
			r = (wcscmp(v1->data.sdata, v2->data.sdata) >= 0) ? 1 : 0;
			break;
		default:
			r = 0;
		break;
	}
	return r;
}

int ScriptObjectManager::compB(scriptVar *v1, scriptVar *v2) {
	int r;
	switch (v1->type) {
		case SCRIPT_BOOLEAN:
			r = ((v1->data.idata == 0) ? 0 : 1) < ((makeBoolean(v2) == 0) ? 0 : 1);
			break;
		case SCRIPT_INT:
			r = v1->data.idata < makeInt(v2);
			break;
		case SCRIPT_FLOAT:
			r = v1->data.fdata < makeFloat(v2);
			break;
		case SCRIPT_DOUBLE:
			r = v1->data.ddata < makeDouble(v2);
			break;
		case SCRIPT_STRING:
			r = (wcscmp(v1->data.sdata, v2->data.sdata) < 0) ? 1 : 0;
			break;
		default:
			r = 0;
			break;
	}
	return r;
}

int ScriptObjectManager::compBe(scriptVar *v1, scriptVar *v2) {
	int r;
	switch (v1->type) {
		case SCRIPT_BOOLEAN:
			r = ((v1->data.idata == 0) ? 0 : 1) <= ((makeBoolean(v2) == 0) ? 0 : 1);
			break;
		case SCRIPT_INT:
			r = v1->data.idata <= makeInt(v2);
			break;
		case SCRIPT_FLOAT:
			r = v1->data.fdata <= makeFloat(v2);
			break;
		case SCRIPT_DOUBLE:
			r = v1->data.ddata <= makeDouble(v2);
			break;
		case SCRIPT_STRING:
			r = (wcscmp(v1->data.sdata, v2->data.sdata) <= 0) ? 1 : 0;
			break;
		default:
			r = 0;
			break;
	}
	return r;
}

void ScriptObjectManager::mid(wchar_t *dest, const wchar_t *str, int s, int l) {
	if (str == NULL) return;
	int rl = wcslen(str);
	if (l == 0) return;
	if (s >= rl) return;
	if (s+l > rl || l == -1) l = rl-s;
	WCSCPYN(dest, str+s, l+1);
}

int ScriptObjectManager::makeInt(scriptVar *v) {
	switch (v->type) {
		case SCRIPT_INT:
			return v->data.idata;
		case SCRIPT_FLOAT:
			return (int)v->data.fdata;
		case SCRIPT_DOUBLE:
			return (int)v->data.ddata;
		case SCRIPT_BOOLEAN:
			return (v->data.idata == 0) ? 0 : 1;
		case SCRIPT_STRING:
			return WTOI(v->data.sdata);
		default:
		return 0;
	}
}

float ScriptObjectManager::makeFloat(scriptVar *v) {
	switch (v->type) {
		case SCRIPT_INT:
			return (float)v->data.idata;
		case SCRIPT_FLOAT:
			return v->data.fdata;
		case SCRIPT_DOUBLE:
			return (float)v->data.ddata;
		case SCRIPT_BOOLEAN:
			return (float)((v->data.idata == 0) ? 0 : 1);
		case SCRIPT_STRING:
			return (float)WTOF(v->data.sdata);
		default:
		return 0.0f;
	}
}

double ScriptObjectManager::makeDouble(scriptVar *v) {
	switch (v->type) {
		case SCRIPT_VOID:
			return 0;
		case SCRIPT_INT:
			return (double)v->data.idata;
		case SCRIPT_FLOAT:
			return (double)v->data.fdata;
		case SCRIPT_DOUBLE:
			return v->data.ddata;
		case SCRIPT_BOOLEAN:
			return (double)((v->data.idata == 0) ? 0 : 1);
		case SCRIPT_STRING:
			return WTOF(v->data.sdata);
		default:
			return 0.0;
	}
}

bool ScriptObjectManager::makeBoolean(scriptVar *v) {
	switch (v->type) {
		case SCRIPT_INT:
		case SCRIPT_BOOLEAN:
			return (v->data.idata == 0) ? 0 : 1;
		case SCRIPT_FLOAT:
			return (v->data.fdata == 0) ? 0 : 1;
		case SCRIPT_DOUBLE:
			return (v->data.ddata == 0) ? 0 : 1;
		case SCRIPT_STRING: {
			StringW s = v->data.sdata;
			if (s.iscaseequal(L"false") || s.iscaseequal(L"f"))
				return 0;
			if (s.iscaseequal(L"true") || s.iscaseequal(L"t"))
				return 1;
			return (WTOI(s.getValue()) != 0);
		}
		default:
			return (v->data.odata != NULL) ? 1 : 0;
	}
}

int ScriptObjectManager::isNumeric(scriptVar *s) {
	return isNumericType(s->type);
}

int ScriptObjectManager::isString(scriptVar *s) {
	return (s->type == SCRIPT_STRING);
}

int ScriptObjectManager::isVoid(scriptVar *s) {
	return (s->type == SCRIPT_VOID);
}

int ScriptObjectManager::isObject(scriptVar *s) {
	return (s->type == SCRIPT_OBJECT);
}

int ScriptObjectManager::isNumericType(int t) {
	return (t == SCRIPT_INT || t == SCRIPT_BOOLEAN || t == SCRIPT_FLOAT || t == SCRIPT_DOUBLE);
}

#ifdef WASABI_COMPILE_COMPONENTS
WACObject *ScriptObjectManager::getWACObject(const char *guid) {
	GUID cg;
	//BU: let me use guid:avs please :)
	cg = *SkinParser::getComponentGuid(guid);
	return getWACObject(cg);
}

WACObject *ScriptObjectManager::getWACObject(GUID cg) {
	for (int i=0;i<comps.getNumItems();i++) {
	    GUID dg = comps[i]->getGUID();
	    if (!MEMCMP(&dg,&cg,sizeof(GUID)))
			return comps[i];
	}
	return NULL;
}
#endif

SystemObject *ScriptObjectManager::getSystemObject(int n) {
	return syslist.enumItem(n);
}

SystemObject *ScriptObjectManager::getSystemObjectByScriptId(int id) {
	static int lasti = -1;
	static SystemObject *lasto = NULL;
	if (lasti == id && syslist.haveItem(lasto)) return lasto;
	for (int i=0;i<syslist.getNumItems();i++) {
		if (syslist.enumItem(i)->getScriptId() == id) {
			lasto = syslist.enumItem(i);
			lasti = id;
			return lasto;
		}
	}
	return NULL;
}

void ScriptObjectManager::registerSystemObject(SystemObject *o) {
	syslist.addItem(o);
}

void ScriptObjectManager::unregisterSystemObject(SystemObject *o) {
	syslist.removeItem(o);
}

int SOM::getNumSystemObjects() {
	return syslist.getNumItems();
}

SystemObject *SOM::enumSystemObject(int n) {
	return syslist.enumItem(n);
}

int ScriptObjectManager::typeCheck(VCPUscriptVar *v, int fail) {
	ASSERT(v);
	ASSERT(v->v.data.odata);
	int type = v->v.type;
	while (type >= 0x10000) {
		int id = VCPU::varBase(v->scriptId) + (type - 0x10000);
		VCPUscriptVar *vc = VCPU::variablesTable.enumItem(id);
		type = vc->v.type;
	}
	int otype = ObjectTable::getClassFromName(v->v.data.odata->vcpu_getClassName());
	ASSERT(otype >= 0);
	if (ObjectTable::isDescendant(type, otype)) return 1;
	if (fail)
		Script::guruMeditation(getSystemObject(v->scriptId), GURU_INCOMPATIBLEOBJECT, L"VAR/OBJECT CLASS MISMATCH", v->varId);
	return 0;
}

#ifdef WASABI_COMPILE_WNDMGR
WindowHolder *ScriptObjectManager::getSuitableWindowHolderFromScript(GUID g) {
	for (int i=0;i<getNumSystemObjects();i++) {
		SystemObject *o = enumSystemObject(i);
		WindowHolder *so = o->getSuitableWindowHolderByGuid(g);
		if (so) return so;
	}
	return NULL;
}

int ScriptObjectManager::checkAbortShowHideWindow(GUID g, int visible) {
	for (int i=0;i<getNumSystemObjects();i++) {
		SystemObject *o = enumSystemObject(i);
		int r = o->onGetCancelComponent(g, visible);
		if (r) 
			return r;
	}
	return 0;
}
#endif

#ifdef WASABI_COMPILE_WND
ScriptObject *ScriptObjectManager::findObject(const wchar_t *name) 
{
	for (int i=0;i<windowTracker->getNumAllWindows();i++) 
	{
		ScriptObject *so = static_cast<ScriptObject *>(windowTracker->enumAllWindows(i)->getInterface(scriptObjectGuid));
		if (!so) continue;
		GuiObject *go = static_cast<GuiObject *>(so->vcpu_getInterface(guiObjectGuid));
		if (!go) continue;
		if (WCSCASEEQLSAFE(go->guiobject_getId(), name)) 
		return so;
	}
	return NULL;
}
#endif

SystemObject * ScriptObjectManager::system;
PtrList < SystemObject > ScriptObjectManager::syslist;

int ScriptObjectManager::inited = 0;
