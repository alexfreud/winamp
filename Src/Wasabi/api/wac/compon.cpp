#include <precomp.h>

#include <bfc/wasabi_std.h>

#include "compon.h"
//#include <api/wac/main.h> // CUT!

#ifndef WASABINOMAINAPI
#include <api/metadb/metadb.h>
#include <api/wac/papi.h>
#endif
#include <api/script/objects/compoobj.h>
#include <api/wndmgr/container.h>
#include <api/skin/skinparse.h>
#include <api/wac/wac.h>
#include <api/script/objects/wacobj.h>
#include <api/wnd/wndtrack.h>
#include <api/script/objecttable.h>

#include <api/config/items/cfgitemi.h>

#include <bfc/loadlib.h>
//#include <bfc/util/profiler.h>
#include <api/locales/xlatstr.h>
#include <bfc/file/recursedir.h>

#include <api/service/svc_enum.h>
#include <api/service/services.h>
#include <api/service/servicei.h>
#include <api/skin/skin.h>
#include <api/script/scriptmgr.h>
#include <bfc/parse/pathparse.h>

#ifndef WASABINOMAINAPI
#include <api/api1.h>
#endif

#include <api/application/wkc.h>
#include <api/wndmgr/skinembed.h>
#include <api/script/objects/compoobj.h>

#include <api/wnd/usermsg.h>
#include <bfc/util/inifile.h>

class CShutdownCallback {
  public:
  virtual void rl_onShutdown()=0;
};


extern GUID baseGUID;

static TList<GUID> loadlist, banlist;

// compon.cpp : maintains the list of installed components, and installs
// each one

// keep a list of component pointers as well as instances
class component_slot 
{
public:
#ifndef WASABINOMAINAPI
   component_slot(Library *_dll, WaComponent *_wac, ComponentAPI *_api, GUID g, const wchar_t *orig_path) 
     : dll(_dll), wac(_wac), path(orig_path), guid(g), componentapi(_api) {
#else
   component_slot(Library *_dll, WaComponent *_wac, GUID g, const wchar_t *orig_path) 
     : dll(_dll), wac(_wac), path(orig_path), guid(g) {
#endif
    int fnlen = wcslen(Wasabi::Std::filename(orig_path));
    path.trunc(-fnlen);
    postonregsvcs = 0;
    postoncreate = 0;
    loadearly = 0;	// will be filled in later
  }
  ~component_slot() {
#ifndef WASABINOMAINAPI
    if (dll != NULL) PAPI::destroyAPI(componentapi);
#endif
    delete dll;
  }
  void registerServices() {
#ifndef WASABINOMAINAPI
    if (!postonregsvcs) wac->registerServices(componentapi);
#else
    if (!postonregsvcs) wac->registerServices(WASABI_API_SVC);
#endif
    postonregsvcs = 1;
  }
  void onCreate() {
    if (!postoncreate) wac->onCreate();
    postoncreate = 1;
  }

  Library *dll;
  WaComponent *wac; 	// we don't delete this
  StringW path;
  GUID guid;		// prevent spoofing
#ifndef WASABINOMAINAPI
  ComponentAPI *componentapi;
#endif
  int postoncreate;
  int postonregsvcs;
  int loadearly;
};

static PtrList<component_slot> components;

/*static PtrList<cd_entry> cd_list;
static PtrList<ComponentObject> co_list;*/

// the minimum SDK version # we accept
const int WA_COMPONENT_VER_MIN = WA_COMPONENT_VERSION;

  static int postComponentCommand(GUID guid, const wchar_t *command, int p1, int p2, void *ptr, int ptrlen, int waitforanswer);

class ComponPostEntry {
public:
  ComponPostEntry(GUID pguid, const wchar_t *pcommand, int pp1, int pp2, void *pptr, int pptrlen, int pwait) :
    guid(pguid), command(pcommand), p1(pp1), p2(pp2), ptr(pptr), ptrlen(pptrlen), waitforanswer(pwait),
      posted(0), result(0) { }
  GUID guid;
  StringW command;
  int p1, p2;
  void *ptr;
  int ptrlen;
  int waitforanswer;
  int posted;
  int result;
};

static PtrList<StringW> preloads;

void ComponentManager::addStaticComponent(WaComponent *component) {
  GUID guid = component->getGUID();
  if (!checkGUID(guid)) return;	// guid check (banlist etc.)

#ifndef WASABINOMAINAPI
  // reuse main api *
  components.addItem(new component_slot(NULL, component, api, guid, NULL));
#else
  components.addItem(new component_slot(NULL, component, guid, NULL));
#endif
}

void ComponentManager::addPreloadComponent(const wchar_t *filename)
{
  foreach(preloads)
    if (PATHEQL(filename, preloads.getfor()->getValue())) return;// no dups
  endfor
  preloads.addItem(new StringW(filename));
}

void ComponentManager::loadPreloads() {
  foreach(preloads)
    load(preloads.getfor()->getValue());
  endfor
  preloads.deleteAll();
}

void ComponentManager::load(const wchar_t *filename) {
  // ensure no duplicate filenames
  foreach(components)
    if (PATHEQL(filename, components.getfor()->dll->getName())) return;
  endfor

  #ifdef WA3COMPATIBILITY
  // let kernel controller test the file
  WasabiKernelController *wkc = Main::getKernelController();
  if (wkc && !wkc->testComponent(filename)) return;
  #endif

  // check if they have an ini (to get guid faster)
  StringW inifile = filename;
  const wchar_t *ext = Wasabi::Std::extension(inifile);
	int len = wcslen(ext);
  inifile.trunc(-len);
  inifile.cat(L"ini");
  IniFile ini(inifile);
  GUID ini_guid = ini.getGuid(L"component", L"guid");
  if (!checkGUID(ini_guid, TRUE)) return;

//  PR_ENTER2("load component", filename);

  // attach the DLL
  Library *dll = new Library(filename);
  if (!dll->load()) {
    delete dll;
    return;
  }

  // check the version of SDK it was compiled with
  WACGETVERSION wac_get_version = (WACGETVERSION)dll->getProcAddress("WAC_getVersion");
  if (wac_get_version == NULL) {
    delete dll;
    return;
  }

  int version = (*wac_get_version)();
  if (version < WA_COMPONENT_VER_MIN ||	// defined above
      version > WA_COMPONENT_VERSION) {	// from wac.h
    delete dll;
    return;
  }

  // init the dll itself
  WACINIT wacinit = (WACINIT)dll->getProcAddress("WAC_init");
  if (wacinit != NULL) (*wacinit)(dll->getHandle());

  WACENUMCOMPONENT wec = (WACENUMCOMPONENT)dll->getProcAddress("WAC_enumComponent");
  if (wec == NULL) {
    delete dll;
    return;
  }

  // fetch the pointer
  WaComponent *wac = (*wec)(0);

  GUID guid = wac->getGUID();

  if (ini_guid != INVALID_GUID && guid != ini_guid) {
    delete dll;
    DebugString("guids didn't match! %s", filename);
    return;
  }

  // check if we want to load this GUID
  if (!checkGUID(guid)) {
    delete dll;
    return;
  }

#ifndef WASABINOMAINAPI
  // allocate an api pointer bound to their GUID
  ComponentAPI *newapi = PAPI::createAPI(wac, guid);
  if (newapi == NULL) {
    delete dll;
    return;
  }
#endif

  PathParserW pp(filename);
  StringW path;
  for (int i=0;i<pp.getNumStrings()-1;i++) 
	{
		path.AppendFolder(pp.enumString(i));
  }

  wac->setComponentPath(path);

  // keep track of dll handles for shutdown
  components.addItem(new component_slot(dll, wac, 
#ifndef WASABINOMAINAPI
  newapi, 
#endif
  guid, filename));

//  PR_LEAVE();
}

const wchar_t *ComponentManager::getComponentPath(GUID g) {
  foreach(components)
    if (g == components.getfor()->guid) {
      return components.getfor()->path;
    }
  endfor
  return NULL;
}

void ComponentManager::startupDBs() {
/*  for (int i=0;i<components.getNumItems();i++)
    MetaDB::addComponentDB(components.enumItem(i)->wac);*/
}

void ComponentManager::shutdownDBs() {
#ifndef WASABINOMAINAPI
  for (int i = 0; i < components.getNumItems(); i++) {
    //MetaDB::getBaseDB()->removeComponentDB(components[i]->wac);
    (static_cast<ComponentAPI1 *>(components[i]->componentapi))->shutdownDB();
  }
#else
//MULTIAPI-FIXME: solve the shutdownDB puzzle
#endif
}

void ComponentManager::unloadAll() {
  // cable out! let 'er go!
//  deleteAllCD(); // deletes compwnds
  foreach(components)
    components.getfor()->wac->deregisterServices();
  endfor
  foreach(components)
    components.getfor()->wac->onDestroy();
  endfor
  components.deleteAll();	// free the DLLs, and kill their API *'s
}

int ComponentManager::checkGUID(GUID &g, int invalid_ok) {
  // no invalid guid
  if (!invalid_ok && g == INVALID_GUID) return FALSE;

  // check against banlist
  if (banlist.haveItem(g)) return FALSE;

  // check against load-only list
  if (loadlist.getNumItems() && !loadlist.haveItem(g)) return FALSE;

  // ensure no duplicate GUIDs
  foreach(components)
    if (g == components.getfor()->guid) {
//CUT      StringPrintf s("%s and %s", components.getfor()->dll->getName(), filename);
//CUT      Std::messageBox(s, "Duplicate component guid", MB_OK);
      return FALSE;
    }
  endfor

  // ok
  return TRUE;
}

WaComponent *ComponentManager::enumComponent(int component) {
  if (component < 0 || component >= components.getNumItems()) return NULL;
  return components[component]->wac;
}

void ComponentManager::loadAll(const wchar_t *path) {

#if 0//CUT
  static const char *loadorder[] = {
    "wasabi.system/pngload.wac",
    "wasabi.player/core.wac",
    "metrics.wac", // so metrics dialog appears before the splash screen
    "winamp/winamp.wac", // so splash screen displays right after startup
    "winamp/pledit.wac",
    "winamp/library.wac",
    "preferences.wac", // so prefs has the system groups at the top
    "skinswitch.wac", // so skinswitch is the first non internal prefs screen, ignored if not present. fucko: need to modify prefs system so we don't need to load in any particular order
    NULL
  };

  for (int i = 0; loadorder[i] != NULL; i++) {
    StringPrintf fn("%s%s%s", WACDIR, DIRCHARSTR, loadorder[i]);
    ComponentManager::load(fn);
  }
#endif
	RecurseDir dir(path, L"*.*");// have to do *.* to get subdirs
  while (dir.next()) {
		StringPathCombine fn(dir.getPath(), dir.getFilename());
    const wchar_t *ext = Wasabi::Std::extension(fn);
    if (!WCSICMP(ext, L"wac"))
      ComponentManager::load(fn);
  }
}

void ComponentManager::postLoad(int f) { // two-level startup procedure
  // note we're calling the slot, not the component directly

  // allow punk-ass bitches to load early if need be
  foreach(components)
    if ((components.getfor()->loadearly = !!components.getfor()->wac->onNotify(WAC_NOTIFY_LOADEARLY))) {
      components.getfor()->registerServices();
    }
  endfor

  foreach(components)
    if (!components.getfor()->loadearly)
      components.getfor()->registerServices();
  endfor
  foreach(components)
    components.getfor()->onCreate();
  endfor
  if (f) ObjectTable::loadExternalClasses();
}

void ComponentManager::broadcastNotify(int cmd, int param1, int param2) {
  foreach(components)
    if ((components.getfor()->loadearly = !!components.getfor()->wac->onNotify(WAC_NOTIFY_LOADEARLY, cmd, param1, param2)))
      components.getfor()->wac->onNotify(cmd, param1, param2);
  endfor
  foreach(components)
    if (!components.getfor()->loadearly)
      components.getfor()->wac->onNotify(cmd, param1, param2);
  endfor
}

void ComponentManager::sendNotify(GUID guid, int cmd, int param1, int param2) {
  WaComponent *wac = getComponentFromGuid(guid);
  if (wac)
    wac->onNotify(cmd, param1, param2);
}

int ComponentManager::sendCommand(GUID guid, const wchar_t *command, int p1, int p2, void *ptr, int ptrlen) {
  if (command == NULL) return 0;
  WaComponent *wac = getComponentFromGuid(guid);
  if (wac) return wac->onCommand(command, p1, p2, ptr, ptrlen);
  return 0;
}

int ComponentManager::postCommand(GUID guid, const wchar_t *command, int p1, int p2, void *ptr, int ptrlen, int waitforanswer) {
#ifdef WA3COMPATIBILITY // todo: make thread id part of application api
  if(Std::getCurrentThreadId()==Main::getThreadId() && waitforanswer) {
    // if it is already the main thread calling, just pass the command to sendCommand
    return sendCommand(guid,command,p1,p2,ptr,ptrlen);
  }
#endif
  ComponPostEntry *cpe=new ComponPostEntry(guid,command,p1,p2,ptr,ptrlen,waitforanswer);
  componPostEntries.addItem(cpe);
#ifdef WIN32
#ifdef WA3COMPATIBILITY // todo: make this a call to application api
  PostMessage(Main::gethWnd(),UMSG_COMPON_POSTMESSAGE,0,0); // ask the main thread to call mainthreadpostCommands();
#endif
#else
  PostMessage(None,UMSG_COMPON_POSTMESSAGE,0,0); // ask the main thread to call mainthreadpostCommands();
#endif
  if(waitforanswer) {
    while(!cpe->posted) Sleep(1);
    int res=cpe->result;
    componPostEntries.removeItem(cpe);
    delete(cpe);
    return res;
  }
  return 0; // cpe will get deleted by mainthreadpostCommands();
}


void ComponentManager::broadcastCommand(const wchar_t *command, int p1, int p2, void *ptr, int ptrlen) {
  if (command == NULL) return;
  for (int i = 0; i < components.getNumItems(); i++) {
    components[i]->wac->onCommand(command, p1, p2, ptr, ptrlen);
  }
}

int ComponentManager::getNumComponents() {
  return components.getNumItems();
}

GUID ComponentManager::getComponentGUID(int c) {
  if (c >= components.getNumItems()) return INVALID_GUID;
  return components[c]->guid;
}

const wchar_t *ComponentManager::getComponentName(GUID g)
{
  WaComponent *wac = getComponentFromGuid(g);
  if (wac)
    return wac->getName();
  return NULL;
}

CfgItem *ComponentManager::getCfgInterface(GUID g) {
  WaComponent *wac = getComponentFromGuid(g);
  if (wac == NULL) return NULL;
  return wac->getCfgInterface(0);
}

WaComponent *ComponentManager::getComponentFromGuid(GUID g) {
  if (g == INVALID_GUID) return NULL;
  for (int i=0;i<components.getNumItems();i++) {
    if (g == components[i]->guid)
      return components[i]->wac;
  }
  return NULL;
}

void ComponentManager::mainThread_handlePostCommands() {
  // critical section
  for(int i=0;i<componPostEntries.getNumItems();i++) {
    ComponPostEntry *cpe=componPostEntries[i];
    if(!cpe->posted) {
      sendCommand(cpe->guid,cpe->command.getValue(),cpe->p1,cpe->p2,cpe->ptr,cpe->ptrlen);
      if(cpe->waitforanswer) cpe->posted=1;
      else {
        delete cpe;
        componPostEntries.removeByPos(i);
        i--;
      }
    }
  }
}

PtrList<ComponPostEntry> ComponentManager::componPostEntries;
