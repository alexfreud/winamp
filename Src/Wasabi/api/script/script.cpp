#include <precomp.h>
#include <time.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/objects/systemobj.h>
#include <api/script/vcpu.h>
#include <api/wndmgr/msgbox.h>
#include <api/script/guru.h>
#ifdef WASABI_COMPILE_SKIN
#include <api/skin/skin.h>
#endif
#include <bfc/string/bfcstring.h>
#ifdef WASABI_COMPILE_COMPONENTS
#include <api/wac/main.h>//CUT!!
#endif
#include <api/skin/widgets/group.h>
#include <api/script/objecttable.h>
#include <api/application/wkc.h>

extern StringW g_resourcepath;

// insert the given script in the virtual machine
int Script::addScript(const wchar_t *path, const wchar_t *filename, const wchar_t *id) 
{
	static int reentering = 0;
	static int redisplay = 0;

	StringPathCombine fpn(path, filename);

	wchar_t olddir[WA_MAX_PATH] = {0};
	Wasabi::Std::getCurDir(olddir, WA_MAX_PATH);
	Wasabi::Std::setCurDir(WASABI_API_APP->path_getAppPath());
	OSFILETYPE fp = WFOPEN(fpn, WF_READONLY_BINARY, NO_FILEREADERS);
	Wasabi::Std::setCurDir(olddir);
#ifdef WASABI_COMPILE_SKIN
	if (fp == OPEN_FAILED)
	{
		fpn = StringPathCombine(Skin::getDefaultSkinPath(), filename);
		Wasabi::Std::getCurDir(olddir, WA_MAX_PATH);
		Wasabi::Std::setCurDir(WASABI_API_APP->path_getAppPath());
		fp = WFOPEN(fpn, WF_READONLY_BINARY, NO_FILEREADERS);
		Wasabi::Std::setCurDir(olddir);
#endif
		if (fp == OPEN_FAILED)
		{
			if (reentering == 1)
			{
				redisplay = 1;
				return -1;
			}
			reentering = 1;
#ifdef WANT_WASABI_API_WNDMGR
			WASABI_API_SKIN->messageBox(StringPrintf("Could not load script \'%s%s\'\r\n\r\nPress \'OK\' to continue.", path, filename), "Skin Warning", MSGBOX_OK, NULL, NULL);
#else
			Wasabi::Std::messageBox(StringPrintfW(L"Could not load script \'%s%s\'\r\n\r\nPress \'OK\' to continue.", path, filename), L"Skin Warning", 0);
#endif
			if (redisplay)
			{
#ifdef WANT_WASABI_API_WNDMGR
				WASABI_API_WND->appdeactivation_setbypass(1);
#endif
				Wasabi::Std::messageBox(StringPrintfW(L"Could not load script \'%s%s\'\r\n\r\nPress \'OK\' to continue.", path, filename), L"Skin Warning", 0);
#ifdef WANT_WASABI_API_WNDMGR
				WASABI_API_WND->appdeactivation_setbypass(0);
#endif
				redisplay = 0;
			}
			reentering = 0;
			return -1;
		}
#ifdef WASABI_COMPILE_SKIN
	}
#endif

	int size = (int)FGETSIZE(fp);

	void *scriptBlock = MALLOC(size);
	FREAD(scriptBlock, size, 1, fp);
	FCLOSE(fp);

#ifdef WASABI_COMPILE_COMPONENTS
	WasabiKernelController *wkc = Main::getKernelController();
	if (wkc && !wkc->testScript(fpn, scriptBlock, size)) return -1;
#endif

	int vcpuid = VCPU::assignNewScriptId();

	SystemObject *system = new SystemObject(); 
	system->setScriptId(vcpuid);
	system->setFilename(filename);

	int r = VCPU::addScript(scriptBlock, size, vcpuid);

	if (r != vcpuid)
	{
		FREE(scriptBlock);
		delete system;
		return -1;
	}

	scriptEntry *e = new scriptEntry;
	e->scriptBlock = scriptBlock;
	e->vcpuId = vcpuid;

	scriptslist.addItem(e);

	if (!ObjectTable::checkScript(system))
	{
		unloadScript(vcpuid);
		return -1;
	}

	return vcpuid;
}

// unload the given script
void Script::unloadScript(int id)
{
	for (int i=0;i<scriptslist.getNumItems();i++) {
		if (scriptslist.enumItem(i)->vcpuId == id) {
			VCPU::removeScript(id);
			FREE(scriptslist.enumItem(i)->scriptBlock);
			delete scriptslist.enumItem(i);
			scriptslist.removeByPos(i);
		}
	}
	if (scriptslist.getNumItems() == 0) scriptslist.removeAll(); // disable fortify warning
}

// unload ALL scripts
void Script::unloadAllScripts()
{
	while (scriptslist.getNumItems()>0)
	{
		int n = scriptslist.getNumItems()-1;
		VCPU::removeScript(scriptslist.enumItem(n)->vcpuId);
		FREE(scriptslist.enumItem(n)->scriptBlock);
		delete scriptslist.enumItem(n);
		scriptslist.removeByPos(n);
	}
	scriptslist.removeAll(); // disable fortify warning
}

PtrList <scriptEntry> Script::scriptslist;

int Script::codeToSeverity(int code, wchar_t *t, int len)
{
	switch (code)
	{
		case GURU_INVALIDHEADER:
		case GURU_OLDFORMAT:
			wcsncpy(t, L"Ignoring script", len);
			return 5;
		case GURU_DIVBYZERO:
			wcsncpy(t, L"Returning 0", len);
			return 4;
		case GURU_NEWFAILED:
			wcsncpy(t, L"Returning NULL", len);
			return 4;
		case GURU_NULLCALLED:
			wcsncpy(t, L"Ignoring call", len);
			return 4;
		case GURU_INCOMPATIBLEOBJECT:
			wcsncpy(t, L"Assigning NULL", len);
			return 4;
		default:
			wcsncpy(t, L"Internal error", len);
			return 9;
	}
}

void Script::guruMeditation(SystemObject *script, int code, const wchar_t *pub, int intinfo) {

	wchar_t t[256] = {0}, u[256] = {0};
	codeToSeverity(code, u, 256);
	if (pub)
		WCSNPRINTF(t, 256, L"guru: %s - #%04X.%04X%04X", pub, code, (intinfo & 0xFFFF), VCPU::VIP & 0xFFFF);
	else
		WCSNPRINTF(t, 256,  L"guru: #%04X.%04X%04X", pub, code, (intinfo & 0xFFFF), VCPU::VIP & 0xFFFF);
	if (*u) {
		wcscat(t, L" - ");
	    wcscat(t, u);
	}
	DebugStringW(L"%s\n", t);

	time_t now;
	time(&now);
	struct tm *lt = localtime(&now);
#ifdef _WIN32
	wchar_t *p = _wasctime(lt);
#else
#warning port me
	wchar_t *p = L"port me";
#endif
	if (p && *p) p[wcslen(p)-1]=0;

	FILE *fout = _wfopen(StringPathCombine(WASABI_API_APP->path_getUserSettingsPath(), L"guru.log"), WF_APPEND_RW);
	if (fout)
	{
		StringPrintfW z(L"%s (%s/%s) - ", p, WASABI_API_SKIN->getSkinName(),script->getFilename());
		if (*u) 
		{
			z.cat(u);
			z.cat(L" - ");
		}
		StringPrintfW log(L"%s#%04X.%04X%04X.%d%s%s\n", z.getValue(), code, (intinfo & 0xFFFF), VCPU::VIP & 0xFFFF, VCPU::VSD, pub?L" ":L"", pub?pub:L"");
		fputws(log.getValue(), fout);
		fclose(fout);
	}

	Guru::spawn(script, code, pub, intinfo);

#ifdef WASABI_COMPILE_MAKIDEBUG
//	WASABI_API_MAKIDEBUG->debugger_createJITD(script->getScriptId(), 1);
//	debugApi->debugger_trace();
#endif
}

void Script::setScriptParam(int id, const wchar_t *p)
{
	SystemObject *s = SOM::getSystemObjectByScriptId(id);
	if (s) s->setParam(p);
}

void Script::setParentGroup(int id, Group *g)
{
	SystemObject *s = SOM::getSystemObjectByScriptId(id);
	if (s) s->setParentGroup(g);
}

void Script::setSkinPartId(int id, int skinpartid)
{
	SystemObject *s = SOM::getSystemObjectByScriptId(id);
	if (s) s->setSkinPartId(skinpartid);
}

int Script::varIdToGlobal(int id, int script)
{
	return id + VCPU::varBase(script);
}

int Script::getNumEventsLinked()
{
	return VCPU::eventsTable.getNumItems();
}

int Script::getLinkedEventParams(int num, int *dlfid, int *scriptid, int *varid)
{
	VCPUeventEntry *e = VCPU::eventsTable.enumItem(num);
	if (dlfid) *dlfid = e->DLFid;
	if (scriptid) *scriptid = e->scriptId;
	if (varid) *varid = e->varId;
	return num;
}

int Script::getCacheCount()
{
	return VCPU::getCacheCount();
}

int Script::getUserAncestor(int varid, int scriptid)
{
	return VCPU::getUserAncestor(varid, scriptid);
}

int Script::isValidScriptId(int id)
{
	return VCPU::isValidScriptId(id);
}