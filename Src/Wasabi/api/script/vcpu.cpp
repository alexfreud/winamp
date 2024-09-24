#include <precomp.h>
#include <wasabicfg.h>
#include <api/skin/widgets/mb/scriptbrowser.h>
#include <api/script/scriptmgr.h>
#include <api/script/script.h>
#include "vcpu.h"
#include "opcodes.h"
#include <api/wndmgr/container.h>
#include <api/wndmgr/msgbox.h>
#include <api/script/objecttable.h>
#include <api/syscb/callbacks/consolecb.h>
#include "../nu/AutoWide.h"

ScriptObjectManager *VCPU::scriptManager = NULL;

void VCPU::shutdown()
{
  foreach(globalDlfList)
    FREE(globalDlfList.getfor()->functionName);
    delete globalDlfList.getfor();
  endfor
  globalDlfList.removeAll();
  atoms.deleteAll();
}

// -------------------------------------------------------------
void VCPU::push(VCPUscriptVar v) {
  CpuStack.push(v);
  VSP++;
}

// -------------------------------------------------------------
void VCPU::push(scriptVar v) {
  VCPUscriptVar _v;
  _v.v = v;
  CpuStack.push(_v);
  VSP++;
}

void VCPU::RemoveOldScripts()
{
	while (scriptsToRemove.getNumItems())
	{
		int id = scriptsToRemove.getFirst();
		VCPU::removeScript(id);
		scriptsToRemove.delByPos(0);
	}
}

// -------------------------------------------------------------
VCPUscriptVar VCPU::pop() {
  if (VSP <= 0) {
    Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_POPEMPTYSTACK);
    VCPUscriptVar v;
    MEMSET(&v, 0, sizeof(v));
    return v;
//    ASSERT(0);
  }
  VCPUscriptVar v;
  CpuStack.pop(&v);
  VSP--;
	if (VSP == 0)
		VCPU::RemoveOldScripts(); // benski> TODO: dunno if this is the best place for this
  return v;
}

// -------------------------------------------------------------
VCPUscriptVar VCPU::peekAt(int n) {
  if (VSP <= n) {
    Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_INVALIDPEEKSTACK);
    VCPUscriptVar v;
    MEMSET(&v, 0, sizeof(v));
    return v;
//    ASSERT(0);
  }
  VCPUscriptVar v={0,{0},0};
  CpuStack.peekAt(&v, n);
  return v;
}

// -------------------------------------------------------------
int VCPU::assignNewScriptId() {
  return numScripts++;
}

// -------------------------------------------------------------
int VCPU::oldClassToClassId(int id) {
  if (id < SCRIPT_OBJECT) return id;
  if (id >= 0x10000) return id;
  switch (id) {
    case 7 : return ObjectTable::getClassFromName(L"Object");
    case 8 : return ObjectTable::getClassFromName(L"SystemObject");
    case 9 : return ObjectTable::getClassFromName(L"Container");
    case 10: return ObjectTable::getClassFromName(L"Layout");
    case 11: return ObjectTable::getClassFromName(L"Button");
    case 12: return ObjectTable::getClassFromName(L"Slider");
    case 13: return ObjectTable::getClassFromName(L"Text");
    case 14: return ObjectTable::getClassFromName(L"Image");
    case 15: return ObjectTable::getClassFromName(L"Anim");
    case 16: return ObjectTable::getClassFromName(L"Vis");
    case 17: return ObjectTable::getClassFromName(L"Component");
    case 18: return ObjectTable::getClassFromName(L"ToggleButton");
    case 19: return ObjectTable::getClassFromName(L"Timer");
    case 20: return ObjectTable::getClassFromName(L"Layer");
    case 21: return ObjectTable::getClassFromName(L"GuiObject");
    case 22: return ObjectTable::getClassFromName(L"AnimatedLayer");
    case 23: return ObjectTable::getClassFromName(L"Browser");
    case 24: return ObjectTable::getClassFromName(L"Edit");
    case 25: return ObjectTable::getClassFromName(L"Map");
    case 26: return ObjectTable::getClassFromName(L"Popup");
    case 27: return ObjectTable::getClassFromName(L"Title");
    case 28: return ObjectTable::getClassFromName(L"ComponentBucket");
    case 29: return ObjectTable::getClassFromName(L"Status");
    case 30: return ObjectTable::getClassFromName(L"Region");
    case 31: return ObjectTable::getClassFromName(L"Wac");
    case 32: return ObjectTable::getClassFromName(L"List");
    case 33: return ObjectTable::getClassFromName(L"SBitList");
    case 34: return ObjectTable::getClassFromName(L"SEqVis");
    default: Script::guruMeditation(NULL, GURU_INVALIDOLDID, L"xlat error", id);
      break;
  }
  return SCRIPT_INT; // heh =)
}


// -------------------------------------------------------------
int VCPU::addScript(void *mem, int memsize, int id) {

  int i,j;
  int translateobjects = 0;

  char *p = (char *)mem;

  int hdr=0;
  if (!MEMCMP(p, "FG\x03\x04\x14\00\00\00\00", 8))
    hdr=1;
  else if (!MEMCMP(p, "FG\x03\x04\x15\00\00\00\00", 8))
    hdr=2;
  else if (!MEMCMP(p, "FG\x03\x04\x16\00\00\00\00", 8))
    hdr=3;
  else if (!MEMCMP(p, "FG\x03\x04\x17\00\00\00\00", 8))
    hdr=4;     
  else if (!MEMCMP(p, "FG\x03\x04", 4)) {
    if (*(p+4) > 0x17) 
      hdr = -1;
  }

  switch (hdr) {
    case -1:
      Script::guruMeditation(SOM::getSystemObjectByScriptId(id), GURU_FUTUREFORMAT, L"NEED LATEST VERSION");
      return -1;
    case 0:
      Script::guruMeditation(SOM::getSystemObjectByScriptId(id), GURU_INVALIDHEADER);
      return -1;
    case 1:
      Script::guruMeditation(SOM::getSystemObjectByScriptId(id), GURU_OLDFORMAT, L"DEPRECATED BINARY");
      return -1;
    case 2:
      translateobjects=1;
      break;
    case 3:
    case 4:
      break;
  }

  SOM::getSystemObjectByScriptId(id)->setIsOldFormat(translateobjects);

  p+=8;

  TList<int> *typetable = SOM::getSystemObjectByScriptId(id)->getTypesList();
  typetable->removeAll();

  if (!translateobjects) {
  
    int nGuids = *(int *)p;
    p+=sizeof(int);

    for (int z=0;z<nGuids;z++) {
      GUID g;
      MEMCPY(&g, p, sizeof(GUID));
      p+=sizeof(GUID);
      char zz[256] = {0};
      nsGUID::toChar(g, zz);
      int t = ObjectTable::getClassFromGuid(g);
      if (t == -1) { 
        DebugStringW(L"maki class entry %d not found : %s\n", z, zz);
//        __asm int 3; 
      }
      typetable->addItem(t);
    }
  }

  // -------------------------------------------------------------
  // Load DLF Table

  int DLFEntryBase = DLFentryTable.getNumItems();

  int nDLFentries = *(int *)p;
  p+=sizeof(int);

  for (i=0;i<nDLFentries;i++) {

    int basetype = *(int *)p;
    int pt = basetype;
    int type = basetype;
    p+=sizeof(int);

    if (translateobjects) {
      basetype = oldClassToClassId(basetype);
    } else
      if (basetype >= CLASS_ID_BASE && basetype < 0x10000)
        basetype = typetable->enumItem(basetype - CLASS_ID_BASE);

    if (basetype == -1) {
//CUT!!!! so annoying      Std::messageBox("Error while loading a script, a component is missing", "Oops", 0);
      DebugStringW(L"Tried to link DLF %d (class entry %d) but the class isn't here\n", i, pt - CLASS_ID_BASE);
      //return -1;
    }

    type = basetype;

    uint16_t stringLen = *(uint16_t *)p;
    p+=sizeof(uint16_t);
    char functionName[65536+1] = {0};
    MEMCPY(functionName, p, stringLen);
    functionName[stringLen]=0;
    p+=stringLen;

    // check if entry seems valid

    if (!*functionName) {
      Script::guruMeditation(SOM::getSystemObjectByScriptId(id), GURU_INVALIDFUNCINDLF);
//      api->messageBox("Invalid function name in DLF table", "Script Error", MSGBOX_OK, NULL, NULL);
      return -1;
    }

    // ok, register this function
    VCPUdlfEntry *e = new VCPUdlfEntry;
    e->basetype = type;
#ifdef _WIN32
				int size = MultiByteToWideChar(CP_UTF8, 0, functionName, -1, 0,0);
		if (size)
		{
			wchar_t *wide = (wchar_t *)MALLOC(size*sizeof(wchar_t));
			MultiByteToWideChar(CP_UTF8, 0, functionName, -1, wide,size);
			e->functionName = wide;
		}
		else
			e->functionName = 0;
#else
    e->functionName = WCSDUP(AutoWide(functionName));
#warning port me
#endif
    e->scriptId = id;
    // insert safe values
    e->nparams = -1;
    e->DLFid = -1;
    e->ptr = NULL;
    DLFentryTable.addItem(e);

    setupDLF(e, DLFEntryBase);

  }

  // -------------------------------------------------------------
  // Load VAR Table

  int variableBase = variablesTable.getNumItems();

  int nVariables = *(int *)p;
  p+=sizeof(int);

  for (i=0;i<nVariables;i++) {
    scriptVar e;
    MEMCPY(&e, p, sizeof(scriptVar));
    p+=sizeof(scriptVar);

    VCPUscriptVar *v = new VCPUscriptVar;
    v->isaclass = 0;

    if (e.type >= 0x10000) {
      int type = e.type;
      int id;

      do {
        id = type - 0x10000;
        VCPUscriptVar *v = variablesTable.enumItem(id+variableBase);
        v->isaclass = 1;
        type = v->v.type;
      } while (type >= 0x10000);
      
    }

    if (translateobjects) {
      e.type = oldClassToClassId(e.type);
    } else
      if (e.type >= CLASS_ID_BASE && e.type < 0x10000)
        e.type = typetable->enumItem(e.type - CLASS_ID_BASE);

    v->scriptId = id;
    v->varId = i;
    v->transcient = (*p++ == 0);
    if (hdr >= 4)
      v->isstatic = *p++;
    else
      v->isstatic = 0;

    if (hdr < 4) {
      // Autoassign system variables
      if (e.type == ObjectTable::getClassFromName(L"SystemObject")) {
        SystemObject *so = SOM::getSystemObjectByScriptId(id);
        if (so) e.data.odata = so->getScriptObject(); else e.data.odata = NULL;
      }
    } else {
      if (v->isstatic && e.type == ObjectTable::getClassFromName(L"SystemObject")) {
        SystemObject *so = SOM::getSystemObjectByScriptId(id);
        if (so) e.data.odata = so->getScriptObject(); else e.data.odata = NULL;
        v->isstatic = 0; // disable deletion
      } else if (v->isstatic) {
        // Autoassign class variables
        e.data.odata = ObjectTable::instantiate(e.type);
        if (e.data.odata)
          e.data.odata->vcpu_setScriptId(VSD);
      }
    }

    if (e.type == SCRIPT_STRING) 
		{
			wchar_t *emptyString = WMALLOC(1);
			emptyString[0]=0;
      e.data.sdata = emptyString;
    }

    if (e.type == SCRIPT_DOUBLE) 
      e.data.ddata = e.data.fdata;

    v->v = e;
    variablesTable.addItem(v);
  }

  // -------------------------------------------------------------
  // Load Strings into string vars

  int nStrings = *(int *)p;
  p+=sizeof(int);

  j=0;
  //CUT: int count=0;

	char string_buf[65536+1] = {0};
  for (i=0;i<nStrings;i++) 
	{
    int attach_id = *(int *)p;
    p+=4;
    uint16_t stringLen = *(uint16_t *)p;
    p+=2;
    //char *string;
    //string = (char *)MALLOC(stringLen+1);
    MEMCPY(string_buf, p, stringLen);
    string_buf[stringLen]=0;
    p+=stringLen;

    // find next variable in this script that needs a string attached, and attach it
    VCPUscriptVar *v = variablesTable.enumItem(attach_id+variableBase);
		if (!v)
		{
			Script::guruMeditation(SOM::getSystemObjectByScriptId(id), GURU_EXCEPTION, L"Invalid String ID");
      return -1;
		}
    FREE((wchar_t *)(v->v.data.sdata));
		// strings are stored in UTF-8, but we're using UTF-16 here
#ifdef _WIN32
		int size = MultiByteToWideChar(CP_UTF8, 0, string_buf, -1, 0,0);
		if (size)
		{
			wchar_t *wide = (wchar_t *)MALLOC(size*sizeof(wchar_t));
			MultiByteToWideChar(CP_UTF8, 0, string_buf, -1, wide,size);
			v->v.data.sdata = wide;
		}
		else
			v->v.data.sdata = 0;
		
#else
#warning port me 
		// TODO: benski> change to do one malloc
    v->v.data.sdata = WCSDUP(AutoWide(string_buf)); 
#endif
		//FREE(string);

	}


  // -------------------------------------------------------------
  // Load Events into table

  int nEvents = *(int *)p;
  p+=sizeof(int);

  for (i=0;i<nEvents;i++) {

    int varId = *(int *)p;
    p+=sizeof(int);
    int DLFentry = *(int *)p;
    p+=sizeof(int);
    int pointer = *(int *)p;
    p+=sizeof(int);

    // check if this event seems valid
    if (DLFentry >= nDLFentries || DLFentry < 0) {
//      api->messageBox("Invalid event DLF descriptor", "Script Error", MSGBOX_OK, NULL, NULL);
      Script::guruMeditation(SOM::getSystemObjectByScriptId(id), GURU_INVALIDEVENTDLF);
      return -1;
    }

    if (pointer < 0) {
//      api->messageBox("Invalid event address", "Script Error", MSGBOX_OK, NULL, NULL);
      Script::guruMeditation(SOM::getSystemObjectByScriptId(id), GURU_INVALIDEVENTADDR);
      return -1;
    }

    if (varId < 0 || varId >= nVariables) {
//      api->messageBox("Invalid event variable", "Script Error", MSGBOX_OK, NULL, NULL);
      Script::guruMeditation(SOM::getSystemObjectByScriptId(id), GURU_INVALIDEVENTVAR);
      return -1;
    }

    // insert event into table

    VCPUeventEntry *e = new VCPUeventEntry;
    //e->DLFentry = DLFentry + DLFEntryBase;
    e->DLFid = DLFentryTable.enumItem(DLFEntryBase+DLFentry)->DLFid;
    e->pointer = pointer;
    e->scriptId = id;
    e->varId = varId;

    eventsTable.addItem(e);
  }

  // -------------------------------------------------------------
  // Load Code block into code table

  int codeSize = *(int *)p;
  p+=sizeof(int);
  VCPUcodeBlock *c = new VCPUcodeBlock;
  c->codeBlock = p;
  c->dlfBase = DLFEntryBase;
  c->varBase = variableBase;
  c->scriptId = id;
  c->size = codeSize;

  codeTable.addItem(c);

  SystemObject *so = SOM::getSystemObjectByScriptId(id);
  if (so) {
    ScriptObject *sso = so->getScriptObject();
    if (sso)
      sso->vcpu_addAssignedVariable(0, id);
  }

  cacheCount++;

  c->debugsymbols = c->codeBlock+codeSize;
  c->debugsize = memsize - (c->debugsymbols-(char *)mem);

  //WASABI_API_MAKIDEBUG->debugger_createJITD(id); // fucko !!
 
  return id;
}


// -------------------------------------------------------------
int VCPU::varBase(int scriptId) {
  static int lasti=-1;;
  static int lastb=0;
  static int lastid=0;
  if (lastid == scriptId && lasti>=0 && lasti < codeTable.getNumItems()) {
    if (lastid == codeTable.enumItem(lasti)->scriptId) 
      return lastb;
  }
  for (int i=0;i<codeTable.getNumItems();i++ ){
    if (codeTable.enumItem(i)->scriptId == scriptId) {
      lasti = i;
      lastid = scriptId;
      lastb = codeTable.enumItem(i)->varBase;
      return lastb;
    }
  }
  Script::guruMeditation(SOM::getSystemObjectByScriptId(scriptId), GURU_INVALIDSCRIPTID);
  ASSERT(0);
  return 0;
}

// -------------------------------------------------------------
int VCPU::nVars(int scriptId) {
  for (int i=0;i<codeTable.getNumItems();i++ ){
    if (codeTable.enumItem(i)->scriptId == scriptId) {
      if (codeTable.getNumItems() == i+1)
        return variablesTable.getNumItems() - codeTable.enumItem(i)->varBase;
      return codeTable.enumItem(i+1)->varBase - codeTable.enumItem(i)->varBase;
    }
  }
  Script::guruMeditation(SOM::getSystemObjectByScriptId(scriptId), GURU_INVALIDSCRIPTID);
  ASSERT(0);
  return 0;
}

// -------------------------------------------------------------
int VCPU::dlfBase(int scriptId) {
  for (int i=0;i<codeTable.getNumItems();i++ ){
    if (codeTable.enumItem(i)->scriptId == scriptId)
      return codeTable.enumItem(i)->dlfBase;
  }
  Script::guruMeditation(SOM::getSystemObjectByScriptId(scriptId), GURU_INVALIDSCRIPTID);
  ASSERT(0);
  return 0;                                        
}

// -------------------------------------------------------------
void VCPU::removeScript(int id) 
{
//  ASSERTPR(VCPU::VSP==0, "Can't unload script while in script");
	if (VCPU::VSP != 0)
	{
		scriptsToRemove.addItem(id);
		return;
	}

  SystemObject *s = SOM::getSystemObjectByScriptId(id);
	if (s)
	{
		s->onUnload();
		delete s;
	}

  PtrList<ScriptObject> *l = SystemObject::getAllScriptObjects();
  int i;
  for (i=0;i<l->getNumItems();i++) {
    ScriptObject *o = l->enumItem(i);
    o->vcpu_delMembers(id);
  }

  int dlfdeleted=0;
  int vardeleted=0;

  for (i=0;i<DLFentryTable.getNumItems();i++) {
    if (DLFentryTable.enumItem(i)->scriptId == id) {
      delrefDLF(DLFentryTable.enumItem(i));
      if (DLFentryTable.enumItem(i)->functionName)
        FREE(DLFentryTable.enumItem(i)->functionName);
      delete DLFentryTable.enumItem(i);
      DLFentryTable.delByPos(i);
      dlfdeleted++;
      i--;
    }
  }

  for (i=0;i<eventsTable.getNumItems();i++) {
    if (eventsTable.enumItem(i)->scriptId == id) {
      delete eventsTable.enumItem(i);
      eventsTable.delByPos(i);
      i--;
    }
  }

  for (i=0;i<variablesTable.getNumItems();i++) {
    if (variablesTable.enumItem(i)->scriptId == id) {
      VCPUscriptVar *v = variablesTable.enumItem(i);
      if (v->isstatic && v->v.type) {
        ObjectTable::destroy(v->v.data.odata);
      }
      if (v->v.type == SCRIPT_STRING)
        if (v->v.data.sdata)
          FREE((wchar_t *)v->v.data.sdata);
      delete v;
      variablesTable.delByPos(i);
      vardeleted++;
      i--;
    }
  }

  for (i=0;i<codeTable.getNumItems();i++) {
    if (codeTable.enumItem(i)->scriptId == id) {
      VCPUcodeBlock *b = codeTable.enumItem(i);
      delete b;
      codeTable.removeByPos(i);
      for (;i<codeTable.getNumItems();i++) {
        codeTable.enumItem(i)->dlfBase-=dlfdeleted;
        codeTable.enumItem(i)->varBase-=vardeleted;
      }
    }
  }


  cacheCount++;
}

// -------------------------------------------------------------
// Find next matching object, starting from start
int VCPU::findObject(ScriptObject *o, int start, int dlfid, int vcpuid) {
/*  int stop;
  if (vcpuid != -1) {
  int b = varBase(vcpuid);
    if (start < b)
      start = b;
    stop = b + nVars(vcpuid);
  } else {
    stop = variablesTable.getNumItems();
    if (start < 0) 
      start = 0;
  }*/

/*  while (start < stop) {
    VCPUscriptVar *v = variablesTable.enumItem(start);
    if (v->v.data.odata == o && !v->transcient && (vcpuid == -1 || v->scriptId == vcpuid))
      return start;
    start++;
  }*/
  return -1;
}



// -------------------------------------------------------------
// Assign DLF functionId to class exported functions, starting from the last non initialized DLF
void VCPU::setupDLF(VCPUdlfEntry *e, int dlfEntryBase) {
  if (ObjectTable::addrefDLF(e, highestDLFId)) {
    newDlf();
  }

}

int VCPU::newDlf() {
  return highestDLFId++;
}

void VCPU::resetDlf() {
  highestDLFId = 0;
}

void VCPU::registerGlobalDlf(VCPUdlfEntry *e, int dlf) {
  ASSERT(dlf == globalDlfList.getNumItems());
  VCPUdlfEntry *_e = new VCPUdlfEntry;
  MEMCPY(_e, e, sizeof(VCPUdlfEntry));
  _e->functionName = WCSDUP(e->functionName);
  globalDlfList.addItem(_e);
}

void VCPU::delrefDLF(VCPUdlfEntry *e) {
  ObjectTable::delrefDLF(e);
}

// -------------------------------------------------------------

TList<VCPUscriptVar> VCPU::plist;

// -------------------------------------------------------------
int VCPU::runEvent(VCPUeventEntry *e, int np, int pbase) {

#ifdef WASABI_COMPILE_MAKIDEBUG
  /*if (WASABI_API_MAKIDEBUG && WASABI_API_MAKIDEBUG->debugger_isActive()) {
    if (WASABI_API_MAKIDEBUG->debugger_filterEvent(e->scriptId, e->DLFid)) {
      DebugString("Skipping event\n");
      scriptVar v;
      v.type = SCRIPT_INT;
      v.data.idata = 0;
      VCPU::push(v);
      return 1;
    }
  }*/
#endif

  for (int z=0;z<np;z++) {
    VCPU::push(plist[z+pbase]);
  }

  runCode(e->scriptId, e->pointer, np);

#ifdef WASABI_COMPILE_MAKIDEBUG
/*  if (WASABI_API_MAKIDEBUG && WASABI_API_MAKIDEBUG->debugger_isActive()) 
    WASABI_API_MAKIDEBUG->debugger_eventComplete();*/
#endif

  return 1;
}

// This is the function that actually executes the event. In the future, it will sequencially parse all loaded scripts in reversed load order and stop
// either at the end of the chain OR as soon as one of the event used "complete;" in its code

// -------------------------------------------------------------
scriptVar VCPU::executeEvent(scriptVar v, int functionId, int np, int vcpuid) {

  VCPUscriptVar retvar={0,{0},0};
  int pbase = plist.getNumItems();

  int varId=0;

  complete = 0;

  for (int z=0;z<np;z++) {
    VCPUscriptVar vcpuv = VCPU::pop();
    plist.addItem(vcpuv);
  }

  // find all variables containing this object, and run their event if it's traped

  int next = 0;

  while (!complete) {
    VCPUscriptVar *vclass=NULL;
    int inheritedevent=0;
//    varId = VCPU::findObject(v.data.odata, varId, vcpuid);
    int event;
	  ASSERT(v.data.odata != NULL);
    varId = ((ScriptObject *)v.data.odata)->vcpu_getAssignedVariable(next, vcpuid, functionId, &next, &event, &inheritedevent);

    if (varId < 0) break;

    VCPUscriptVar *vc = variablesTable.enumItem(varId);
    ScriptObject *thisobject = (ScriptObject *)v.data.odata;

    VCPUeventEntry *e = eventsTable.enumItem(event);

    int r_varId = varId;

    if (!vc->isaclass && !inheritedevent) {
      if (e && runEvent(e, np, pbase))
        retvar = pop();
      if (getComplete())
        break;
    }

    while (vc->isaclass) {
      ASSERT(r_varId < variablesTable.getNumItems());
      vclass = variablesTable.enumItem(r_varId);
      vclass->v.data.odata = thisobject;

      if (runEvent(e, np, pbase))
        retvar = pop();

      if (getComplete())
        break;

      vc = vclass;
      if (vc->varId < 0x10000) break;
      r_varId = varBase(vc->scriptId) + vc->v.type - 0x10000;
    }

    if (getComplete())
      break;

    varId++;
  }

  for (int i=0;i<np;i++)
    plist.delByPos(pbase);

  return retvar.v;
}

void VCPU::callDlfCommand(void *ptr, int nargs, maki_cmd *cmd) {
  try {

    scriptVar v={0,0};
    switch (nargs) {
      case 0:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *))ptr)(cmd, -1, NULL);
        break;
      case 1:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar))ptr)(cmd, -1, NULL, v);
        break;
      case 2:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar))ptr)(cmd, -1, NULL, v, v);
        break;
      case 3:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar))ptr)(cmd, -1, NULL, v, v, v);
        break;
      case 4:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar))ptr)(cmd, -1, NULL, v, v, v, v);
        break;
      case 5:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))ptr)(cmd, -1, NULL, v, v, v, v, v);
        break;
      case 6:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))ptr)(cmd, -1, NULL, v, v, v, v, v, v);
        break;
      case 7:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))ptr)(cmd, -1, NULL, v, v, v, v, v, v, v);
        break;
      case 8:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))ptr)(cmd, -1, NULL, v, v, v, v, v, v, v, v);
        break;
      case 9:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))ptr)(cmd, -1, NULL, v, v, v, v, v, v, v, v, v);
        break;
      case 10:
        ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))ptr)(cmd, -1, NULL, v, v, v, v, v, v, v, v, v, v);
        break;
    }
  } 

  catch(...)
  {
    Script::guruMeditation(SOM::getSystemObjectByScriptId(VCPU::VSD), GURU_EXCEPTION, L"Script Fatal Error", -1);
    #ifdef ON_FATAL_SKIN_ERROR
    ON_FATAL_SKIN_ERROR
    #endif
  }

}

int VCPU::getDLFFromPointer(void *ptr, int nargs) {
  maki_cmd cmd={MAKI_CMD_GETDLF, -1};
  callDlfCommand(ptr, nargs, &cmd);
  return cmd.id;
}

// This sends the DLFId to the function itself.
void VCPU::setupDLFFunction(void *ptr, int nargs, int DLFid, VCPUdlfEntry *e) {
  registerGlobalDlf(e, DLFid);
  maki_cmd cmd={MAKI_CMD_SETDLF, DLFid};
  callDlfCommand(ptr, nargs, &cmd);
}

// This sends the DLFId to the function itself.
void VCPU::DLF_reset(void *ptr, int nargs) {
  maki_cmd cmd={MAKI_CMD_RESETDLF, -1};
  callDlfCommand(ptr, nargs, &cmd);
}

void VCPU::DLF_addref(void *ptr, int nargs) {
  maki_cmd cmd={MAKI_CMD_ADDREF, -1};
  callDlfCommand(ptr, nargs, &cmd);
}

void VCPU::DLF_remref(void *ptr, int nargs) {
  maki_cmd cmd={MAKI_CMD_REMREF, -1};
  callDlfCommand(ptr, nargs, &cmd);
}

// -------------------------------------------------------------
scriptVar VCPU::safeDiv(VCPUscriptVar *v1, VCPUscriptVar *v2) {
  double _r=0;
  double _v1=SOM::makeDouble(&v1->v);
  double _v2=SOM::makeDouble(&v2->v);
  if (_v2 != 0.0)
    _r = _v1 / _v2;
  else
    Script::guruMeditation(SOM::getSystemObjectByScriptId(v1->scriptId), GURU_DIVBYZERO, L"Division by zero");

  scriptVar r = SOM::makeVar(SCRIPT_DOUBLE);
  SOM::assign(&r, _r);
  return r;
}

/*

Registers :

VIP : Instruction Pointer
VSP : Stack Pointer
VSD : Script Descriptor (ID of script we're in)


CALLM calls member function, pops a variable ID and a DLF entry, and pushs the result of
the function. CallC calls an address, so pushs the return address on its stack
(independant from the Push/Pop stack), and jumps to the code. Ret gets the last
address pushed and returns there. JIZ jumps to an address if the first value on the stack
is an int zero. JMP jumps unconditionnaly. 

The stack is a stack of 4 bytes integers containing scriptVar IDs.
Var IDs from binaries are being added the base ID of the current script so we
have only one variables segment for all scripts. Same for DLF entries. Events
aren't references in the bytecode other than in the event table that links
addresses in code to DLF entries, so we don't need that kind of tweaking.

*/

// -------------------------------------------------------------
char *VCPU::getCodeBlock(int id, int *size) {
  for (int i=0;i<codeTable.getNumItems();i++) {
    if (codeTable.enumItem(i)->scriptId == id) {
      if (size != NULL) *size = codeTable.enumItem(i)->size;
      return codeTable.enumItem(i)->codeBlock;
    }
  }
  return NULL;
}

// -------------------------------------------------------------
VCPUcodeBlock *VCPU::getCodeBlockEntry(int id) {
  for (int i=0;i<codeTable.getNumItems();i++) {
    if (codeTable.enumItem(i)->scriptId == id) {
      return codeTable.enumItem(i);
    }
  }
  return NULL;
}

// -------------------------------------------------------------
void VCPU::runCode(int scriptId, int pointer, int np) {
  int quit=0;
  VIP = pointer;
  VSD = scriptId;

#ifdef WASABI_COMPILE_MAKIDEBUG
  int debugger_present = debugApi ? debugApi->debugger_isActive() : 0;
#endif

  char *codeblock = (char *)getCodeBlock(VSD);
  char *p = codeblock + VIP;
  unsigned char opcode;

  int stackbase = VSP-np;
  int callcbase = CallStack.peek();
  VCC = callcbase;

  while (!quit) {
#ifdef WASABI_COMPILE_MAKIDEBUG
    if (debugger_present) {
      VIPstack.push(VIP);
      VSDstack.push(VSD);
//      VSPstack.push(VSP);
      VCCstack.push(VCC);
      debugApi->debugger_trace();
      VIPstack.pop(&VIP);
      VSDstack.pop(&VSD);
//      VSPstack.pop(&VSP);
      VCCstack.pop(&VCC);
    }
#endif
    opcode = *p;
    p+=sizeof(opcode);
    VIP+=sizeof(opcode);

    switch (opcode) {
      case OPCODE_PUSH: {
        int id; // var id
        id = *(int *)p;
        p+=sizeof(int); VIP+=sizeof(int);
        VCPUscriptVar *var = variablesTable.enumItem(id+varBase(VSD));
        push(*var);
        break;
      }
      case OPCODE_POPI: {
        pop(); // discard
        if (VSP == stackbase)
          statementStringList.freeAll();
        break;
      }
      case OPCODE_POP: {
        int id = *(int *)p; // var id
        p+=sizeof(int); VIP+=sizeof(int);
        VCPUscriptVar v = pop(); 
        VCPUassign(id, v.v, VSD);
        break;
      }
      case OPCODE_SET: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        if (v1.varId == -1) {
          Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_SETNONINTERNAL);
          ASSERT(0);
        }
        scriptVar r = VCPUassign(v1.varId, v2.v, VSD);
        push(r);
        break;
      }
      case OPCODE_RETF: {
        if (/*VSP == stackbase+1 && */CallStack.peek() == callcbase) {
          quit = 1;
          break;
        }
        CallStack.pop(&p);
        VIP = p-(char *)getCodeBlock(VSD);
        break;
      }
      case OPCODE_CALLC: {
        int shift; // jump length
        shift = *(int *)p;
        p+=sizeof(int); VIP+=sizeof(int);
        CallStack.push(p);
        VCC++;
        p+=shift;
        VIP+=shift;
        break;
      }
      case OPCODE_CALLM: {
        int id; // DLF id
        id = *(int *)p;
        p+=sizeof(int); VIP+=sizeof(int);
        VCPUdlfEntry *e = DLFentryTable.enumItem(id+dlfBase(VSD));
        int np = *(int *)p;
        // OLD stack protection - was relying on a shody test based on the fact that the compiler should not be able to generate two FF's at this offset, replaced by new opcode but remains for backward compatibility
        if ((np & 0xFFFF0000) == 0xFFFF0000) {
          p += sizeof(int);
          VIP += sizeof(int);
          np &= 0xFFFF;
        } else np = -1;
        scriptVar r = callDLF(e, np);
        VCPUscriptVar vr;
        vr.scriptId = VSD;
        vr.varId = -1;
        vr.v = r;
        push(vr);
        break;
      }
      case OPCODE_CALLM2: {
        int id; // DLF id
        id = *(int *)p;
        p+=sizeof(int); VIP+=sizeof(int);
        VCPUdlfEntry *e = DLFentryTable.enumItem(id+dlfBase(VSD));
        int np = *(unsigned char *)p; p++; VIP+=1;
        scriptVar r = callDLF(e, np);
        VCPUscriptVar vr;
        vr.scriptId = VSD;
        vr.varId = -1;
        vr.v = r;
        push(vr);
        break;
      }
      case OPCODE_UMV: 
				{
        VCPUscriptVar name = pop();
        VCPUscriptVar obj = pop();
        ASSERT(obj.v.data.odata!=NULL);
        ASSERT(name.v.data.sdata!=NULL);

        int rettype = *(int *)p;
        p+=sizeof(int); VIP+=sizeof(int);

        if (rettype >= CLASS_ID_BASE) {
          SystemObject *so = SOM::getSystemObjectByScriptId(VSD);
          TList<int> *typeslist = so->getTypesList();
          rettype = typeslist->enumItem(rettype - CLASS_ID_BASE);
        }

        int oid = ((ScriptObject *)obj.v.data.odata)->vcpu_getMember(name.v.data.sdata, VSD, rettype);
        VCPUscriptVar *v = getOrphan(oid);
        ASSERT(v != NULL);
        push(*v);
        break;
        }
      case OPCODE_CMPEQ: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        VCPUscriptVar r;
        r.v = SOM::makeVar(SCRIPT_INT);
        SOM::assign(&r.v, SOM::compEq(&v1.v, &v2.v));
        push(r);
        break;
        }
      case OPCODE_CMPNE: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        VCPUscriptVar r;
        r.v = SOM::makeVar(SCRIPT_INT);
        SOM::assign(&r.v, SOM::compNeq(&v1.v, &v2.v));
        push(r);
        break;
        }
      case OPCODE_CMPA: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        VCPUscriptVar r;
        r.v = SOM::makeVar(SCRIPT_INT);
        SOM::assign(&r.v, SOM::compA(&v1.v, &v2.v));
        push(r);
        break;
        }
      case OPCODE_CMPAE: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        VCPUscriptVar r;
        r.v = SOM::makeVar(SCRIPT_INT);
        SOM::assign(&r.v, SOM::compAe(&v1.v, &v2.v));
        push(r);
        break;
        }
      case OPCODE_CMPB: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        VCPUscriptVar r;
        r.v = SOM::makeVar(SCRIPT_INT);
        SOM::assign(&r.v, SOM::compB(&v1.v, &v2.v));
        push(r);
        break;
        }
      case OPCODE_CMPBE: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        VCPUscriptVar r;
        r.v = SOM::makeVar(SCRIPT_INT);
        SOM::assign(&r.v, SOM::compBe(&v1.v, &v2.v));
        push(r);
        break;
        }
      case OPCODE_JIZ: {
        int shift; // jump length
        shift = *(int *)p;
        p+=sizeof(int); VIP+=sizeof(int);
        VCPUscriptVar v = pop();
        if (v.v.data.idata == 0) {
          p+=shift;
          VIP+=shift;
        }
        break;
      }
      case OPCODE_JNZ: {
        int shift; // jump length
        shift = *(int *)p;
        p+=sizeof(int); VIP+=sizeof(int);
        VCPUscriptVar v = pop();
        if (v.v.data.idata != 0) {
          p+=shift;
          VIP+=shift;
        }
        break;
      }
      case OPCODE_JMP: {
        int shift; // jump length
        shift = *(int *)p;
        p+=sizeof(int); VIP+=sizeof(int);
        p+=shift;
        VIP+=shift;
        break;
      }
      case OPCODE_NOT: {
        VCPUscriptVar v = pop();
        VCPUscriptVar r;
        r.v = SOM::makeVar(SCRIPT_BOOLEAN);
        switch (v.v.type) {
          case SCRIPT_BOOLEAN:
          case SCRIPT_INT:
          case SCRIPT_FLOAT:
          case SCRIPT_DOUBLE: {
            int i = SOM::makeBoolean(&v.v);
            r.v.data.idata = i == 0 ? 1 : 0;
          }
          break;
        case SCRIPT_STRING:
          r.v.data.idata = (!v.v.data.sdata || !*v.v.data.sdata) ? 1 : 0;
          break;
        default:
          r.v.data.idata = (v.v.data.odata == NULL) ? 1 : 0;
          break;
        }
        push(r);
        break;
      }
      case OPCODE_INCS: {
        VCPUscriptVar v = pop();
        push(v);
        switch (v.v.type) {
          case SCRIPT_BOOLEAN:
            v.v.data.idata = 1;
            break;
          case SCRIPT_INT:
            v.v.data.idata++;
            break;
          case SCRIPT_FLOAT:
            v.v.data.fdata = v.v.data.fdata+1;
            break;
          case SCRIPT_DOUBLE:
            v.v.data.ddata = v.v.data.ddata+1;
            break;
          default:
            Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_INCSNONNUM);
            ASSERT(0);
            break;
        }
        if (v.varId != -1)
          VCPUassign(v.varId, v.v, VSD);
        break;
      }
      case OPCODE_DECS: {
        VCPUscriptVar v = pop();
        push(v);
        switch (v.v.type) {
          case SCRIPT_BOOLEAN:
            v.v.data.idata = 0;
            break;
          case SCRIPT_INT:
            v.v.data.idata--;
            break;
          case SCRIPT_FLOAT:
            v.v.data.fdata = v.v.data.fdata-1;
            break;
          case SCRIPT_DOUBLE:
            v.v.data.ddata = v.v.data.ddata-1;
            break;
          default:
            Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_DECSNONNUM);
            ASSERT(0);
            break;
        }
        if (v.varId != -1)
          VCPUassign(v.varId, v.v, VSD);
        break;
      }
      case OPCODE_INCP: {
        VCPUscriptVar v = pop();
        switch (v.v.type) {
          case SCRIPT_BOOLEAN:
            v.v.data.idata = 1;
            break;
          case SCRIPT_INT:
            v.v.data.idata++;
            break;
          case SCRIPT_FLOAT:
            v.v.data.fdata = v.v.data.fdata+1;
            break;
          case SCRIPT_DOUBLE:
            v.v.data.ddata = v.v.data.ddata+1;
            break;
          default:
            Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_INCPNONNUM);
            ASSERT(0);
            break;
        }
        if (v.varId != -1)
          VCPUassign(v.varId, v.v, VSD);
        push(v);
        break;
      }
      case OPCODE_DECP: {
        VCPUscriptVar v = pop();
        switch (v.v.type) {
          case SCRIPT_BOOLEAN:
            v.v.data.idata = 0;
            break;
          case SCRIPT_INT:
            v.v.data.idata--;
            break;
          case SCRIPT_FLOAT:
            v.v.data.fdata = v.v.data.fdata-1;
            break;
          case SCRIPT_DOUBLE:
            v.v.data.ddata = v.v.data.ddata-1;
            break;
          default:
            Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_DECSNONNUM);
            ASSERT(0);
            break;
        }
        if (v.varId != -1)
          VCPUassign(v.varId, v.v, VSD);
        push(v);
        break;
      }
      case OPCODE_ADD: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        ASSERT(v1.v.type == SCRIPT_STRING || SOM::isNumeric(&v1.v));
        ASSERT(v2.v.type == SCRIPT_STRING || SOM::isNumeric(&v2.v));
        if (v2.v.type == SCRIPT_STRING)
				{
          int n=0;
          if (!v2.v.data.sdata) break;
          if (v1.v.data.sdata) n+= wcslen(v1.v.data.sdata);
          n+= wcslen(v2.v.data.sdata);
          wchar_t *s = (wchar_t *)WMALLOC((n+1));
          ASSERT(s != NULL);

          if (v1.v.data.sdata) 
		  {
            wcsncpy(s, v1.v.data.sdata, n);
			wcsncat(s, (v2.v.data.sdata ? v2.v.data.sdata : L""), n);
          } else
			  wcsncpy(s, (v2.v.data.sdata ? v2.v.data.sdata : L""), n);

          v1.v = SOM::makeVar(SCRIPT_STRING);
          SOM::assign(&v1.v, s);
          FREE(s);
          push(v1);
        } else {
          scriptVar r = SOM::makeVar(SCRIPT_DOUBLE);
          SOM::assign(&r, SOM::makeDouble(&v1.v) + SOM::makeDouble(&v2.v));
          push(r);
        }
        break;
      }
      case OPCODE_SUB: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        ASSERT(SOM::isNumeric(&v1.v));
        ASSERT(SOM::isNumeric(&v2.v));
        scriptVar r = SOM::makeVar(SCRIPT_DOUBLE);
        SOM::assign(&r, SOM::makeDouble(&v1.v) - SOM::makeDouble(&v2.v));
        push(r);
        break;
      }
      case OPCODE_MUL: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        ASSERT(SOM::isNumeric(&v1.v));
        ASSERT(SOM::isNumeric(&v2.v));
        scriptVar r = SOM::makeVar(SCRIPT_DOUBLE);
        SOM::assign(&r, SOM::makeDouble(&v1.v) * SOM::makeDouble(&v2.v));
        push(r);
        break;
      }
      case OPCODE_DIV: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        scriptVar r = safeDiv(&v1, &v2);
        push(r);
        break;
      }

      case OPCODE_MOD: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        if (!SOM::isNumeric(&v2.v) || !SOM::isNumeric(&v1.v)) {
          Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_MODNONNUM);
          ASSERT(0);
        }
        SOM::assign(&v1.v, SOM::makeInt(&v1.v) % SOM::makeInt(&v2.v));
        push(v1);
        break;
      }

      case OPCODE_NEG: {
        VCPUscriptVar v = pop();
        switch (v.v.type) {
          case SCRIPT_BOOLEAN:
            break;
          case SCRIPT_INT:
            v.v.data.idata = -v.v.data.idata;
            break;
          case SCRIPT_FLOAT:
            v.v.data.fdata = -v.v.data.fdata;
            break;
          case SCRIPT_DOUBLE: 
            v.v.data.ddata = -v.v.data.ddata;
            break;
          default:
            Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_NEGNONNUM);
            ASSERT(0);
            break;
        }
        push(v);
        break;
      }

      case OPCODE_BNOT: {
        VCPUscriptVar v = pop();
        if (!SOM::isNumeric(&v.v)) {
          Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_BNOTNONNUM);
          ASSERT(0);
        }
        SOM::assign(&v.v, ~SOM::makeInt(&v.v));
        push(v);
        break;
      }

      case OPCODE_SHL: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        if (!SOM::isNumeric(&v1.v) || !SOM::isNumeric(&v2.v)) {
          Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_SHLNONNUM);
          ASSERT(0);
        }
        SOM::assign(&v1.v, SOM::makeInt(&v1.v) << SOM::makeInt(&v2.v));
        push(v1);
        break;
      }
      case OPCODE_SHR: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        if (!SOM::isNumeric(&v1.v) || !SOM::isNumeric(&v2.v)) {
          Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_SHRNONNUM);
          ASSERT(0);
        }
        SOM::assign(&v1.v, SOM::makeInt(&v1.v) >> SOM::makeInt(&v2.v));
        push(v1);
        break;
      }

      case OPCODE_XOR: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        if (!SOM::isNumeric(&v1.v) || !SOM::isNumeric(&v2.v)) {
          Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_XORNONNUM);
          ASSERT(0);
        }
        SOM::assign(&v1.v, SOM::makeInt(&v1.v) ^ SOM::makeInt(&v2.v));
        push(v1);
        break;
      }

      case OPCODE_AND: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        if (!SOM::isNumeric(&v1.v) || !SOM::isNumeric(&v2.v)) {
          Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_ANDNONNUM);
          ASSERT(0);
        }
        SOM::assign(&v1.v, SOM::makeInt(&v1.v) & SOM::makeInt(&v2.v));
        push(v1);
        break;
      }
      case OPCODE_OR: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        if (!SOM::isNumeric(&v1.v) || !SOM::isNumeric(&v2.v)) {
          Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_ANDNONNUM);
          ASSERT(0);
        }
        SOM::assign(&v1.v, SOM::makeInt(&v1.v) | SOM::makeInt(&v2.v));
        push(v1);
        break;
      }

      case OPCODE_LAND: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        VCPUscriptVar r;
        r.v = SOM::makeVar(SCRIPT_BOOLEAN);
        int a2 = SOM::makeBoolean(&v2.v);
        int a1 = SOM::makeBoolean(&v1.v);
        r.v.data.idata = (a2 && a1) ? 1 : 0;
        push(r);
        break;
      }
      case OPCODE_LOR: {
        VCPUscriptVar v2 = pop();
        VCPUscriptVar v1 = pop();
        VCPUscriptVar r;
        r.v = SOM::makeVar(SCRIPT_BOOLEAN);
        int a2 = SOM::makeBoolean(&v2.v);
        int a1 = SOM::makeBoolean(&v1.v);
        r.v.data.idata = (a2 || a1) ? 1 : 0;
        push(r);
        break;
      }

      case OPCODE_DELETE: {
        VCPUscriptVar v1 = pop();
        int id = 0;
        int type = v1.v.type;
        if (type >= 0x10000)
          do {
            id = type - 0x10000;
            VCPUscriptVar *v = variablesTable.enumItem(id+varBase(VSD));
            type = v->v.type;
          } while (type >= 0x10000);

        if (isInstantiable(type)) {
          ScriptObject *s = (ScriptObject *)v1.v.data.odata;
          scriptVar v = SOM::makeVar(v1.v.type);
          VCPUassign(v1.varId, v, v1.scriptId);
          SystemObject *so = SOM::getSystemObjectByScriptId(VSD);
          so->removeInstantiatedObject(s);
          ObjectTable::destroy(s);
        }
        VCPU::push(v1);
        break;
      }


      case OPCODE_NEW: {
        int id = *(int *)p; // class id
        p+=sizeof(int); VIP+=sizeof(int);

        SystemObject *so = SOM::getSystemObjectByScriptId(VSD);
        TList<int> *typeslist = so->getTypesList();

        int _id;
        if (id >= 0x10000)
          do {
            _id = id - 0x10000;
            VCPUscriptVar *v = variablesTable.enumItem(_id+varBase(VSD));
            id = v->v.type;
          } while (id >= 0x10000);

        if (SOM::getSystemObjectByScriptId(VSD)->isOldFormat())
          id = oldClassToClassId(id);
        else
          id = typeslist->enumItem(id);

        if (isInstantiable(id)) {
          ScriptObject *s = ObjectTable::instantiate(id);
          if (s) s->vcpu_setScriptId(VSD);

          so->addInstantiatedObject(s);

          if (s == NULL) {
            Script::guruMeditation(SOM::getSystemObjectByScriptId(VSD), GURU_NEWFAILED);
          }
          VCPUscriptVar v={{SCRIPT_OBJECT, {0}}, 0};
          SOM::assign(&v.v, s);
          push(v);
        } else {
          VCPUscriptVar n = {{SCRIPT_OBJECT,NULL}, 0};
          push(n);
        }
        break;
      }

      case OPCODE_CMPLT : {
        complete = 1;
        break;
        }

      case OPCODE_NOP : 
      {
#if defined(_WIN32) || defined(_WIN64)
		  OutputDebugStringA("Opcode 0 - NOP encountered, please check!\n");
#else
#warning port me
#endif
        break;
      }
      default: {
        ASSERTALWAYS(StringPrintf("Opcode %X not implemented", opcode));
        break;
      }
    }
  }

  ASSERT(VSP == stackbase + 1);
}

// -------------------------------------------------------------
scriptVar VCPU::VCPUassign(int id, scriptVar sv, int scriptId) {
  VCPUscriptVar *v = NULL;

  if (id & (1 << 31)) {
    id = id & ~(1 << 31);
    v = getOrphan(id);
  } else
    v = variablesTable.enumItem(id+varBase(scriptId));

  if (v->v.type != SCRIPT_STRING) {
    if (!SOM::isNumeric(&v->v)) {
      // assigning an object

      scriptVar _sv = sv;

      if (_sv.data.odata != NULL && !SystemObject::isObjectValid(_sv.data.odata))
        _sv.data.odata = NULL;

      if (v->v.data.odata != _sv.data.odata) {

        if (v->v.data.odata != NULL && !v->transcient && SystemObject::isObjectValid(v->v.data.odata))
          ((ScriptObject *)v->v.data.odata)->vcpu_removeAssignedVariable(v->varId, v->scriptId);

        if (_sv.data.odata == NULL) {
          v->v.data.odata = NULL;
        } else {
          SOM::assign(&v->v, &sv);
          if (SOM::typeCheck(v, 0)) {
            if (!v->isaclass && !v->transcient)
              ((ScriptObject *)sv.data.odata)->vcpu_addAssignedVariable(v->varId, v->scriptId);
          } else {
            int type = v->v.type;
            if (type >= 0x10000)
              do {
                id = type - 0x10000;
                VCPUscriptVar *v = variablesTable.enumItem(id+varBase(VSD));
                type = v->v.type;
              } while (type >= 0x10000);
            class_entry *e = ObjectTable::getClassEntry(type);
            ASSERT(e != NULL);
            GUID g = e->classGuid;
            ScriptObject *o = NULL;
            v->v.data.odata->vcpu_getInterfaceObject(g, &o);
            if (o != NULL) {
              v->v.data.odata = o;
              if (!v->isaclass && !v->transcient)
                o->vcpu_addAssignedVariable(v->varId, v->scriptId);
            } else {
              v->v.data.odata = NULL;
            }
          }
        }
      }

    } else {
      // assigning a number
      SOM::assign(&v->v, &sv);
    }
  } else {
    ASSERT(sv.type == SCRIPT_STRING);
    SOM::persistentstrassign(&v->v, sv.data.sdata);
  }

  return v->v;
}

// -------------------------------------------------------------
void VCPU::traceState(VCPUscriptVar object, VCPUdlfEntry *e) {
  _DebugString("vcpu[%2X]: %04X [%04X].%s", VCPU::VSD, VCPU::VIP, object.varId, e->functionName); 
//  CallbackManager::issueCallback(SysCallback::CONSOLE, ConsoleCallback::DEBUGMESSAGE, 0, reinterpret_cast<int>(t));
}

// -------------------------------------------------------------
// Calls the DLF function
scriptVar VCPU::callDLF(VCPUdlfEntry *e, int np) {

  static Stack<int> cpuidstack;

  cpuidstack.push(VSD);
  cpuidstack.push(VIP);
  cpuidstack.push(VCC);

/*  if (e->external) {
    char t[256] = {0};
    VCPUscriptVar v = VCPU::peekAt(e->nparams);
    SPRINTF(t, "vcpu: %04X [%04X].%s", VCPU::VIP, v.varId, e->functionName);
    Console::outputString(0, t);
    DebugString("%s", t);
    ((void(*)(int))e->ptr)(-1);
    scriptVar rv = pop().v; // returned val
    cpuidstack.pop(&VCC);
    cpuidstack.pop(&VIP);
    cpuidstack.pop(&VSD);
    return rv;
  }*/

  
/*  char t[256] = {0};
  SPRINTF(t, "e->nparams = %d\n", e->nparams);
  DebugString("%s", t); */

  //ASSERT(np == -1 || np == e->nparams); // fucko!!!!!!!!

  if (np == -1) {
    np = e->nparams;
  }

  for (int i=0;i<np;i++) {
    paramList[i] = pop().v;
  }

  VCPUscriptVar object = pop();
  scriptVar r = MAKE_SCRIPT_INT(0);

  //traceState(object, e);

  if (object.v.data.odata == NULL) {
    Script::guruMeditation(SOM::getSystemObjectByScriptId(VCPU::VSD), GURU_NULLCALLED, L"Null object called", object.varId);
    cpuidstack.pop(&VCC);
    cpuidstack.pop(&VIP);
    cpuidstack.pop(&VSD);
    return MAKE_SCRIPT_INT(0);
    //ASSERT(0);
  }
#ifndef _DEBUG
  try
#endif
	{
    if (object.v.data.odata) object.v.data.odata->vcpu_setScriptId(object.scriptId);

    if (e->ptr != NULL) {
      switch (np) {
        case 0:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *))e->ptr)(NULL, VCPU::VSD, object.v.data.odata);
          break;
        case 1:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar))e->ptr)(NULL, VCPU::VSD, object.v.data.odata, paramList[0]);
          break;
        case 2:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar))e->ptr)(NULL, VCPU::VSD, object.v.data.odata, paramList[0], paramList[1]);
          break;
        case 3:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar))e->ptr)(NULL, VCPU::VSD, object.v.data.odata, paramList[0], paramList[1], paramList[2]);
          break;
        case 4:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar))e->ptr)(NULL, VCPU::VSD, object.v.data.odata, paramList[0], paramList[1], paramList[2], paramList[3]);
          break;
        case 5:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))e->ptr)(NULL, VCPU::VSD, object.v.data.odata, paramList[0], paramList[1], paramList[2], paramList[3], paramList[4]);
          break;
        case 6:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))e->ptr)(NULL, VCPU::VSD, object.v.data.odata, paramList[0], paramList[1], paramList[2], paramList[3], paramList[4], paramList[5]);
          break;
        case 7:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))e->ptr)(NULL, VCPU::VSD, object.v.data.odata, paramList[0], paramList[1], paramList[2], paramList[3], paramList[4], paramList[5], paramList[6]);
          break;
        case 8:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))e->ptr)(NULL, VCPU::VSD, object.v.data.odata, paramList[0], paramList[1], paramList[2], paramList[3], paramList[4], paramList[5], paramList[6], paramList[7]);
          break;
        case 9:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))e->ptr)(NULL, VCPU::VSD, object.v.data.odata, paramList[0], paramList[1], paramList[2], paramList[3], paramList[4], paramList[5], paramList[6], paramList[7], paramList[8]);
          break;
        case 10:
          r = ((scriptVar (*)(maki_cmd *, int vsd, class ScriptObject *, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar, scriptVar))e->ptr)(NULL, VCPU::VSD, object.v.data.odata, paramList[0], paramList[1], paramList[2], paramList[3], paramList[4], paramList[5], paramList[6], paramList[7], paramList[8], paramList[9]);
          break;
      }
    }
  }
#ifndef _DEBUG
	catch(...) 
  {
    Script::guruMeditation(SOM::getSystemObjectByScriptId(VCPU::VSD), GURU_EXCEPTION, L"Script Fatal Error", object.varId);
    cpuidstack.pop(&VCC);
    cpuidstack.pop(&VIP);
    cpuidstack.pop(&VSD);
    #ifdef ON_FATAL_SKIN_ERROR
    ON_FATAL_SKIN_ERROR
    #endif
    return MAKE_SCRIPT_INT(0);
  }
#endif
  cpuidstack.pop(&VCC);
  cpuidstack.pop(&VIP);
  cpuidstack.pop(&VSD);

  return r;
}

// -------------------------------------------------------------
void VCPU::addStatementString(wchar_t *s) 
{
  statementStringList.addItem(s);
}

// -------------------------------------------------------------
int VCPU::getComplete() {
  return complete;
}

// -------------------------------------------------------------
int VCPU::isInstantiable(int id) {
  ASSERT(!SOM::isNumericType(id));
  return ObjectTable::isClassInstantiable(id);
}

// -------------------------------------------------------------
int VCPU::getDlfGlobalIndex(int dlfid, int scriptid) {
  static int lasti=-1;
  static int lastid=0;
  static int lastsid=0;
  if (lasti>=0 && lasti < DLFentryTable.getNumItems()) {
    if (lastsid == scriptid && lastid == dlfid) {
      VCPUdlfEntry *e = DLFentryTable.enumItem(lasti);
      if (e->DLFid == dlfid && e->scriptId == scriptid)
        return lasti;
    }
  }
  for (int i=0;i<DLFentryTable.getNumItems();i++ ){
    VCPUdlfEntry *e = DLFentryTable.enumItem(i);
    if (e->scriptId == scriptid && e->DLFid == dlfid) {
      lasti = i;
      lastsid = scriptid;
      lastid = dlfid;
      return lasti;
    }
  }
  Script::guruMeditation(SOM::getSystemObjectByScriptId(scriptid), GURU_INVALIDEVENTDLF);
  return -1;
}

// -------------------------------------------------------------
int VCPU::isValidScriptId(int id) {
  for (int i=0;i<codeTable.getNumItems();i++)
    if (codeTable[i]->scriptId == id) return 1;
  return 0;
}

// -------------------------------------------------------------
int VCPU::getCacheCount() {
  return cacheCount;
}

// -------------------------------------------------------------
int VCPU::getUserAncestor(int varid, int scriptid) {
  VCPUscriptVar *vc = variablesTable.enumItem(varid+varBase(scriptid)) ;
  if (vc->v.type < 0x10000) return -1;
  int r_varId = vc->v.type - 0x10000;
  ASSERT(r_varId < variablesTable.getNumItems());
  return r_varId;
}

// -------------------------------------------------------------
void VCPU::pushObject(void *o) {
  scriptVar v = SOM::makeVar(SCRIPT_OBJECT, (ScriptObject *)o);
  VCPU::push(v);
}

// -------------------------------------------------------------
void VCPU::pushInt(int i) {
  scriptVar v = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&v, i);
  VCPU::push(v);
}

// -------------------------------------------------------------
void VCPU::pushBoolean(int b) {
  scriptVar v = SOM::makeVar(SCRIPT_BOOLEAN);
  SOM::assign(&v, b);
  VCPU::push(v);
}

// -------------------------------------------------------------
void VCPU::pushFloat(float f) {
  scriptVar v = SOM::makeVar(SCRIPT_FLOAT);
  SOM::assign(&v, f);
  VCPU::push(v);
}

// -------------------------------------------------------------
void VCPU::pushDouble(double d) {
  scriptVar v = SOM::makeVar(SCRIPT_DOUBLE);
  SOM::assign(&v, d);
  VCPU::push(v);
}

// -------------------------------------------------------------
void VCPU::pushString(const wchar_t *s) 
{
  scriptVar v = SOM::makeVar(SCRIPT_STRING);
  SOM::assign(&v, s);
  VCPU::push(v);
}

// -------------------------------------------------------------
void VCPU::pushVoid() {
  scriptVar v = SOM::makeVar(SCRIPT_VOID);
  VCPU::push(v);
}

// -------------------------------------------------------------
void *VCPU::popObject() {
  return (void *)VCPU::pop().v.data.odata;
}

// -------------------------------------------------------------
int VCPU::popInt() {
  scriptVar v = VCPU::pop().v;
  ASSERT(SOM::isNumeric(&v));
  return SOM::makeInt(&v);
}

// -------------------------------------------------------------
bool VCPU::popBoolean() {
  scriptVar v = VCPU::pop().v;
  ASSERT(SOM::isNumeric(&v));
  return SOM::makeBoolean(&v);
}

// -------------------------------------------------------------
float VCPU::popFloat() {
  scriptVar v = VCPU::pop().v;
  ASSERT(SOM::isNumeric(&v));
  return SOM::makeFloat(&v);
}

// -------------------------------------------------------------
double VCPU::popDouble() {
  scriptVar v = VCPU::pop().v;
  ASSERT(SOM::isNumeric(&v));
  return SOM::makeDouble(&v);
}

// -------------------------------------------------------------
const wchar_t *VCPU::popString() 
{
  scriptVar v = VCPU::pop().v;
  ASSERT(v.type == SCRIPT_STRING);
  return v.data.sdata;
}

// -------------------------------------------------------------
void VCPU::popDiscard() {
  VCPU::pop();
}

// -------------------------------------------------------------
VCPUdlfEntry *VCPU::getGlobalDlfEntry(int dlfid) {
  return globalDlfList.enumItem(dlfid);
}

// -------------------------------------------------------------
int VCPU::createOrphan(int type) {
  orphans.addItem(new OrphanEntry(orphanid, type));
  return orphanid++;
}

// -------------------------------------------------------------
void VCPU::killOrphan(int id) {
  int pos;
  OrphanEntry *p = orphans.findItem((const wchar_t *)&id, &pos);
  ASSERT(p != NULL && pos >= 0);
  if (p->v.v.type == SCRIPT_STRING)
    FREE((void *)p->v.v.data.sdata);
  delete p;
  orphans.removeByPos(pos);
}

// -------------------------------------------------------------
VCPUscriptVar *VCPU::getOrphan(int id) {
  OrphanEntry *p = orphans.findItem((const wchar_t *)&id);
  ASSERT(p != NULL);
  return &p->v;
}

// -------------------------------------------------------------
int OrphanQuickSort::compareItem(void *p1, void *p2) {
  if ((static_cast<OrphanEntry *>(p1))->id < (static_cast<OrphanEntry *>(p2))->id) return -1;
  if ((static_cast<OrphanEntry *>(p1))->id > (static_cast<OrphanEntry *>(p2))->id) return 1;
  return 0;
}

// -------------------------------------------------------------
int OrphanQuickSort::compareAttrib(const wchar_t *attr, void *p2) 
{
  int id = *(reinterpret_cast<const int *>(attr));
  int eid = (static_cast<OrphanEntry *>(p2))->id;
  if (id < eid) return -1;
  if (id > eid) return 1;
  return 0;
}

// -------------------------------------------------------------
OrphanEntry::OrphanEntry(int _id, int type) {
  id = _id;
  MEMSET(&v, 0, sizeof(VCPUscriptVar));
  v.v.type = type;
  v.scriptId = -1;
  v.varId = id | (1 << 31); 
  v.transcient = 1; // so no event is trapped, will change later when compiler supports it
}

// -------------------------------------------------------------
void VCPU::setAtom(const wchar_t *atomname, ScriptObject *o) {
  int pos;
  ScriptAtom *sa = atoms.findItem(atomname, &pos);
  if (pos >= 0) {
    delete sa;
    atoms.removeByPos(pos);
  }
  if (o)
    atoms.addItem(new ScriptAtom(atomname, o));
}

// -------------------------------------------------------------
ScriptObject *VCPU::getAtom(const wchar_t *atomname) {
  ScriptAtom *sa = atoms.findItem(atomname);
  if (sa) {
    return sa->getAtomObject();
  }
  return NULL;
}

// -------------------------------------------------------------
const wchar_t *VCPU::getClassName(int vcpuid, int localclassid) {
  SystemObject *so = SOM::getSystemObject(vcpuid);
  if (so != NULL) {
    TList<int> *l = so->getTypesList();
    if (l != NULL) {
      int global = l->enumItem(localclassid);
      class_entry *e = ObjectTable::getClassEntry(global);
      if (e != NULL)
        return e->classname;
    }
  }
  return NULL;
}


// -------------------------------------------------------------
int VCPU::cacheCount = 0;

// segments
PtrList<VCPUscriptVar> VCPU::variablesTable;
PtrList<VCPUeventEntry> VCPU::eventsTable;
PtrList<VCPUdlfEntry> VCPU::DLFentryTable;
PtrList<VCPUdlfEntry> VCPU::globalDlfList;
PtrList<VCPUcodeBlock> VCPU::codeTable;
PtrList<wchar_t> VCPU::statementStringList;
PtrListInsertSorted<OrphanEntry, OrphanQuickSort> VCPU::orphans;
PtrListQuickSorted<ScriptAtom, ScriptAtomSort> VCPU::atoms;
int VCPU::orphanid=0;

// stacks
Stack<VCPUscriptVar> VCPU::CpuStack;
Stack<char *> VCPU::CallStack;

// registers
int VCPU::VIP=0;
int VCPU::VSP=0;
int VCPU::VSD=0;
int VCPU::VCC=0;

Stack<int> VCPU::VIPstack;
Stack<int> VCPU::VSPstack;
Stack<int> VCPU::VSDstack;
Stack<int> VCPU::VCCstack;

// misc
int VCPU::numScripts=0;
int VCPU::highestDLFId=0;
scriptVar VCPU::paramList[SCRIPT_MAXARGS];
int VCPU::complete;

TList<int> VCPU::scriptsToRemove;
// NOTES

// There is no reason why people would cast System, Layout and Container
// back to the common base class... so...
// GUI objects should descend from a GUIObject rather than ScriptObject
// GUIObject would descend from ScriptObject for the compiler and should 
// be exported as "Object" for the script, ScriptObject should then not
// be exported at all, thus preventing someone from doing "Object o = System;"
// which makes no sense since System is not a GUI object. Of course you 
// could still do "Layout l = System.getContainer("mqlksd").getLayout("lqsdkj");"
// but you won't be able to cast that to an "Object". Furthermore, to get a
// GUI object, you'll use the layout's function "getObject", so this
// will add consistency to the overall thing. 

/*
--------------------------------------------------------------------------------

  VCPU: Virtual CPU, The virtual machine processor.
        The VCPU actually takes care of some kinds of segments of variables,
        events, and so on. The VCPU's task is to run any number of scripts
        serially in reversed loading order. Last script loaded takes precedence
        over previous ones. Events and functions fall back to the the previous
        script if it also defines them, unless explicitly prevented via 'complete;'
        The VCPU links DLFs in reverse hierarchy order, allowing overriding of
        functions in objects.

  DLF : Dynamically Linked Function. Function name is used to link it to
        whatever layout of functions we have in any release of the VCPU, allowing
        us to reorder our functions in objects.

  TODO: Add versionning info so we can safely expand this format.

 Binaries format :

 <obsolete>

  Size    Desc                What
  -----------------------------------------------------------------------------
  8       Header              FG\x03\x04\x14\00\00\00\00
  -----------------------------------------------------------------------------
  4       # of DLF            int
  -----------------------------------------------------------------------------
   4       DLF base type      int
   1       Size of func name  char
   N       Function name      char[n]
   ...
  -----------------------------------------------------------------------------
  4       # of variables      int
  -----------------------------------------------------------------------------
   8      variable            scriptVar
   ...
  -----------------------------------------------------------------------------
  4       # of strings        int
  -----------------------------------------------------------------------------
   1      Size of string      char    1st string assigned to 1st string var
   N      String              char[n] 2nd string assigned to 2nd string var...
   ...
  -----------------------------------------------------------------------------
  4       # of events         int
  -----------------------------------------------------------------------------
   4      variable id         int     Matching variable table
   4      DLF entry           int     Matching DLF table
   4      Function pointer    int     Pointer in code from base of code
   ...
  -----------------------------------------------------------------------------
  4       Size of code        int
  -----------------------------------------------------------------------------
   N      Compiled code       char[n]
  -----------------------------------------------------------------------------

*/
