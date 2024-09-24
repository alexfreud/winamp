//PORTABLE
#ifndef _COMPON_H
#define _COMPON_H

#include <bfc/wasabi_std.h>
#include <bfc/string/bfcstring.h>
#include <bfc/ptrlist.h>

class ifc_canvas;
class CfgItem;	// see cfgitem.h
class CompWnd;
class ComponentObject;
class Container;
class WaComponent;
class ComponPostEntry;


class ComponentManager {
public:
  static void addStaticComponent(WaComponent *component);
  static void addPreloadComponent(const wchar_t *filename);
  static void loadPreloads();

  static void loadAll(const wchar_t *path);

  static void postLoad(int f=TRUE);

  static void unloadAll();

  static int checkGUID(GUID &g, int invalid_ok=FALSE);	// boolean of if we should load it

  static WaComponent *enumComponent(int component);

  static void broadcastNotify(int cmd, int param1=0, int param2=0);
  static void sendNotify(GUID guid, int cmd, int param1=0, int param2=0);
  static int sendCommand(GUID guid, const wchar_t *command, int p1=0, int p2=0, void *ptr=NULL, int ptrlen=0);
  static int postCommand(GUID guid, const wchar_t *command, int p1, int p2, void *ptr, int ptrlen, int waitforanswer);
  static void broadcastCommand(const wchar_t *command, int p1=0, int p2=0, void *ptr=NULL, int ptrlen=0);
  static int getNumComponents();
  static GUID getComponentGUID(int c);
  static const wchar_t *getComponentName(GUID g);
  static CfgItem *getCfgInterface(GUID g);
  static WaComponent *getComponentFromGuid(GUID g);

  static void load(const wchar_t *filename);

  static const wchar_t *getComponentPath(GUID g);

  static void startupDBs();
  static void shutdownDBs();

  static void mainThread_handlePostCommands();
  static PtrList<ComponPostEntry> componPostEntries;
};

#endif
