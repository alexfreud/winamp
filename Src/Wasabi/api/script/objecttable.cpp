#include <precomp.h>
#include <api/script/objecttable.h>

#ifndef _NOSTUDIO
#include <api/service/svcs/svc_scriptobji.h>
#include <api/script/scriptobj.h>
#include <api/script/vcputypes.h>
#include <api/script/objects/systemobj.h>
#include <api/script/scriptmgr.h>
#include <api/script/objects/rootobject.h>
#include <api/script/objects/guiobj.h>
#include <api/skin/widgets/group.h>

#ifdef WASABI_WIDGETS_LAYER
#include <api/skin/widgets/layer.h>
#endif

#ifdef WASABI_WIDGETS_ANIMLAYER
#include <api/skin/widgets/animlayer.h>
#endif

#include <AlbumArt.h>

#ifdef WASABI_WIDGETS_BUTTON
#include <api/skin/widgets/button.h>
#endif

#ifdef WASABI_COMPILE_WNDMGR
#ifdef WASABI_WIDGETS_WNDHOLDER
#include <api/wndmgr/container.h>
#endif
#include <api/wndmgr/layout.h>
#endif // wndmgr

#ifdef WASABI_WIDGETS_EDIT
#include <api/skin/widgets/edit.h>
#endif

#ifdef WASABI_WIDGETS_SLIDER
#include <api/skin/widgets/pslider.h>
#endif

#ifdef WASABI_WIDGETS_MEDIAVIS
#include <api/skin/widgets/sa.h>
#endif

#ifdef WASABI_COMPILE_MEDIACORE

#ifdef WASABI_WIDGETS_MEDIAEQCURVE
#include <api/skin/widgets/seqvis.h>
#endif

#ifdef WASABI_WIDGETS_MEDIASTATUS
#include <api/skin/widgets/sstatus.h>
#endif

#endif // mediacore

#ifdef WASABI_WIDGETS_TEXT
#include <api/skin/widgets/text.h>
#endif

#ifdef WASABI_WIDGETS_TGBUTTON
#include <api/skin/widgets/tgbutton.h>
#endif

#ifdef WASABI_WIDGETS_TITLEBAR
#include <api/skin/widgets/title.h>
#endif

#ifdef WASABI_SCRIPTOBJECTS_POPUP
#include <api/script/objects/spopup.h>
#endif

#ifdef WASABI_SCRIPTOBJECTS_MAP
#include <api/script/objects/smap.h>
#endif

#ifdef WASABI_WIDGETS_GROUPLIST
#include <api/skin/widgets/grouplist.h>
#endif

#ifdef WASABI_WIDGETS_COMPBUCK
#include <api/skin/widgets/compbuck2.h>
#endif

#ifdef WASABI_SCRIPTOBJECTS_WAC
#include <api/script/objects/wacobj.h>
#endif

#ifdef WASABI_SCRIPTOBJECTS_LIST
#include <api/script/objects/slist.h>
#endif


#ifdef WASABI_SCRIPTOBJECTS_BITLIST
#include <api/script/objects/sbitlist.h>
#endif

#ifdef WASABI_SCRIPTOBJECTS_REGION
#include <api/script/objects/sregion.h>
#endif

#ifdef WASABI_WIDGETS_MOUSEREDIR
#include <api/skin/widgets/mouseredir.h>
#endif

#ifdef WASABI_WIDGETS_BROWSER
#include <api/skin/widgets/mb/xuibrowser.h>
#endif

#ifdef WASABI_WIDGETS_QUERYLIST
#include <api/skin/widgets/db/xuiquerylist.h>
#endif

#ifdef WASABI_WIDGETS_FILTERLIST
#include <api/skin/widgets/db/xuifilterlist.h>
#endif

#ifdef WASABI_WIDGETS_WNDHOLDER
#include <api/skin/widgets/xuiwndholder.h>
#endif

#ifdef WASABI_WIDGETS_DROPDOWNLIST
#include <api/skin/widgets/dropdownlist.h>
#endif

#ifdef WASABI_SCRIPTOBJECTS_EMBEDDEDXUI
#include <api/wnd/wndclass/embeddedxui.h>
#endif

#ifdef WASABI_WIDGETS_LAYOUTSTATUS
#include <api/skin/widgets/xuistatus.h>
#endif

#ifdef WASABI_WIDGETS_TABSHEET
#include <api/skin/widgets/xuitabsheet.h>
#endif

#ifdef WASABI_WIDGETS_LIST
#include <api/skin/widgets/xuilist.h>
#endif

#ifdef WASABI_WIDGETS_TREE
#include <api/skin/widgets/xuitree.h>
#endif

#ifdef WASABI_WIDGETS_CHECKBOX
#include <api/skin/widgets/xuicheckbox.h>
#endif

#include <api/skin/widgets/xuiframe.h>

//Martin> This fixes Wa5 Menu Xml Object is not instanciable via maki - silly bug
#include <api/skin/widgets/xuimenuso.h>

//fileIO
#include <api/script/objects/sfile.h>
#include <api/script/objects/sxmldoc.h>

//ColorMgr
#include <api/script/objects/scolormgr.h>
#include <api/script/objects/scolor.h>
#include <api/script/objects/sgammaset.h>
#include <api/script/objects/sgammagroup.h>

#include <api/script/objects/sapplication.h>
#include <api/script/objects/sprivate.h>

#include <api/script/vcpu.h>

#include <bfc/ptrlist.h>

#include <api/service/svc_enum.h>
#include <api/service/service.h>
#include <api/service/services.h>

void ObjectTable::start() {
  registerClass(rootScriptObjectController);
  registerClass(systemController);
#ifdef WASABI_COMPILE_SKIN
  registerClass(guiController);
  registerClass(groupController);
#ifdef WASABI_SCRIPTOBJECTS_EMBEDDEDXUI
  registerClass(embeddedXuiController);
#endif
#ifdef WASABI_WIDGETS_LAYER
  registerClass(layerController);
#endif
#ifdef WASABI_WIDGETS_ANIMLAYER
  registerClass(animlayerController);
#endif
  registerClass(albumartController);
#ifdef WASABI_WIDGETS_BUTTON
  registerClass(buttonController);
#endif
#ifdef WASABI_WIDGETS_TGBUTTON
  registerClass(tgbuttonController);
#endif
#ifdef WASABI_WIDGETS_COMPBUCK
  registerClass(cbucketController);
#endif
#ifdef WASABI_COMPILE_WNDMGR
  registerClass(containerController);
  registerClass(layoutController);
#endif
#ifdef WASABI_WIDGETS_EDIT
  registerClass(editController);
#endif
#ifdef WASABI_WIDGETS_SLIDER
  registerClass(sliderController);
#endif
#ifdef WASABI_WIDGETS_MEDIAVIS
  registerClass(visController);
#endif
#ifdef WASABI_COMPILE_MEDIACORE
#ifdef WASABI_WIDGETS_MEDIAEQCURVE
  registerClass(eqvisController);
#endif
#ifdef WASABI_WIDGETS_MEDIASTATUS
  registerClass(statusController);
#endif
#endif // mediacore
#ifdef WASABI_WIDGETS_TEXT
  registerClass(textController);
#endif
	registerClass(frameController);
#ifdef WASABI_WIDGETS_TITLEBAR
  registerClass(titleController);
#endif
#ifdef WASABI_SCRIPTOBJECTS_POPUP
  registerClass(popupController);
#endif
#ifdef WASABI_SCRIPTOBJECTS_WAC
  registerClass(wacController);
#endif
#endif
#ifdef WASABI_SCRIPTOBJECTS_LIST
  registerClass(listController);
#endif
#ifdef WASABI_SCRIPTOBJECTS_BITLIST
  registerClass(bitlistController);
#endif
#ifdef WASABI_SCRIPTOBJECTS_REGION
  registerClass(regionController);
#endif
#ifdef WASABI_SCRIPTOBJECTS_MAP
  registerClass(mapController);
#endif
#ifdef WASABI_COMPILE_SKIN
#ifdef WASABI_WIDGETS_GROUPLIST
  registerClass(grouplistController);
#endif
#ifdef WASABI_WIDGETS_MOUSEREDIR
  registerClass(mouseredirController);
#endif
#ifdef WASABI_COMPILE_CONFIG
  registerClass(cfgGroupController);
#endif
#ifdef WASABI_WIDGETS_BROWSER
  registerClass(browserController);
#endif
#ifdef WASABI_WIDGETS_QUERYLIST
  registerClass(queryListController);
#endif
#ifdef WASABI_WIDGETS_FILTERLIST
  registerClass(filterListController);
#endif
#ifdef WASABI_COMPILE_WNDMGR
#ifdef WASABI_WIDGETS_WNDHOLDER
  registerClass(windowHolderController);
#endif
#endif // wndmgr
#ifdef WASABI_WIDGETS_DROPDOWNLIST
  registerClass(dropDownListController);
#endif
#ifdef WASABI_WIDGETS_LAYOUTSTATUS
  registerClass(layoutStatusController);
#endif
#ifdef WASABI_WIDGETS_TABSHEET
  registerClass(tabsheetController);
#endif
#ifdef WASABI_WIDGETS_LIST
  registerClass(guiListController);
#endif
#ifdef WASABI_WIDGETS_TREE
  registerClass(guiTreeController);
  registerClass(treeItemController);
#endif
#ifdef WASABI_WIDGETS_CHECKBOX
  registerClass(checkBoxController);
#endif
#endif

	registerClass(fileController);
	registerClass(xmlDocController);
	registerClass(applicationController);
	registerClass(SPrivateController);
	registerClass(xuiMenuScriptController);
	registerClass(colorMgrController);
	registerClass(colorController);
	registerClass(gammasetController);
	registerClass(gammagroupController);
}

void ObjectTable::shutdown() {
  for (int i=0;i<classes.getNumItems();i++) {
    class_entry *e = classes[i];
    unlinkClass(e);
    FREE((char *)e->classname);
  }
  classes.deleteAll();    
  VCPU::resetDlf();
  externalloaded = 0;
}

// unload external classes
void ObjectTable::unloadExternalClasses() {
  for (int i=0;i<classes.getNumItems();i++) {
    if (classes.enumItem(i)->external) {
      class_entry *ce = classes.enumItem(i);
      unlinkClass(ce);
      classes.removeByPos(i);
      i--;
    }
  }
  classidx = classes.getNumItems();
  externalloaded = 0;
}

void ObjectTable::unlinkClass(class_entry *e) {
  if (e->sf != NULL && !WASABI_API_SVC->service_isvalid(WaSvc::SCRIPTOBJECT, e->sf)) return;
  ScriptObjectController *c = e->controller;
  const function_descriptor_struct *ds = c->getExportedFunctions();
  for (int j=0;j<c->getNumFunctions();j++) {
    VCPU::DLF_reset(ds[j].physical_ptr, ds[j].nparams);
  }
}

void ObjectTable::loadExternalClasses() {
  if (externalloaded) return;
  externalloaded = 1;
  ExternalScriptObjectEnum soe ;
  svc_scriptObject *obj = soe.getFirst();
  while (obj) {
    obj->onRegisterClasses(rootScriptObjectController);
    int g=0;
    while (1) {
      ScriptObjectController *o = obj->getController(g);
      if (!o) break;
      //DebugString(StringPrintf("Registering script class %s\n", o->getClassName()));
      ObjectTable::registerClass(o, soe.getLastFactory());
      g++;
    }
    WASABI_API_SVC->service_release(obj);
    obj = soe.getNext();
  }
}


// returns classid. ancestorclass = 0 = Object
int ObjectTable::registerClass(ScriptObjectController *c, waServiceFactory *sf) {
  ASSERT(c != NULL);

  c->onRegisterClass(rootScriptObjectController);

  const wchar_t *classname = c->getClassName();
  const wchar_t *ancestorclassname = c->getAncestorClassName();
  GUID g = c->getClassGuid();

  if (getClassFromName(classname) > -1) {
    ASSERTPR(0, StringPrintf("duplicate script class name %S", classname));
#ifdef _WIN32
    ExitProcess(0);
#else
    exit(0);
#endif
  }
  if (getClassFromGuid(g) > -1) {
    ASSERTPR(0, "duplicate script class guid");
#ifdef _WIN32
    ExitProcess(0);
#else
    exit(0);
#endif
  }
  
  int ancestorclassid = -1;
  if (ancestorclassname != NULL)
     ancestorclassid = getClassFromName(ancestorclassname);

  class_entry * en = new class_entry;
  en->classid = CLASS_ID_BASE + classidx++;
  c->setClassId(en->classid);
  c->setAncestorClassId(ancestorclassid);
  en->classname = WCSDUP(classname);
  en->controller = c;
  en->classGuid = g;
  en->ancestorclassid = ancestorclassid;
  en->instantiable = c->getInstantiable();
  en->referenceable = c->getReferenceable();
  en->external = sf != NULL;
  en->sf = sf;
  classes.addItem(en);
  dlfAddClassRef(c, NULL); // FG> fucko
  return classes.getNumItems()-1;
}

int ObjectTable::addrefDLF(VCPUdlfEntry *dlf, int id) {
  
  int classid = dlf->basetype;

  while (classid) {

    class_entry *e = getClassEntry(classid);
    if (!e) return 0;
    function_descriptor_struct *s = (function_descriptor_struct *)e->controller->getExportedFunctions();

    for (int i=0;i<e->controller->getNumFunctions();i++, s++) {
      if (!WCSICMP(s->function_name, dlf->functionName)) {
        int xid = VCPU::getDLFFromPointer(s->physical_ptr, s->nparams);
        if (xid != -1) {
          dlf->DLFid = xid;
          dlf->ptr = s->physical_ptr;
          dlf->nparams = s->nparams;
          VCPU::DLF_addref(s->physical_ptr, s->nparams);
          return 0;
        }
        dlf->DLFid = id;
        dlf->ptr = s->physical_ptr;
        dlf->nparams = s->nparams;
        VCPU::setupDLFFunction(s->physical_ptr, s->nparams, id, dlf);
        VCPU::DLF_addref(s->physical_ptr, s->nparams);
        return 1;
      }
    }

  classid = e->controller->getAncestorClassId();
  }
  return 0;
}

void ObjectTable::delrefDLF(VCPUdlfEntry *dlf) {
  int classid = dlf->basetype;

  while (classid) {

    class_entry *e = getClassEntry(classid);
    if (!e) return;
    function_descriptor_struct *s = (function_descriptor_struct *)e->controller->getExportedFunctions();

    for (int i=0;i<e->controller->getNumFunctions();i++, s++) {
      if (!WCSICMP(s->function_name, dlf->functionName)) {
        VCPU::DLF_remref(s->physical_ptr, s->nparams);
        return;
      }
    }
  classid = e->controller->getAncestorClassId();
  }
}

void ObjectTable::resetDLF(VCPUdlfEntry *dlf) {
  int classid = dlf->basetype;

  while (classid) {

    class_entry *e = getClassEntry(classid);
    if (!e) return;
    function_descriptor_struct *s = (function_descriptor_struct *)e->controller->getExportedFunctions();

    for (int i=0;i<e->controller->getNumFunctions();i++, s++) {
      if (!WCSICMP(s->function_name, dlf->functionName)) {
        VCPU::DLF_reset(s->physical_ptr, s->nparams);
        return;
      }
    }
  classid = e->controller->getAncestorClassId();
  }
}

int ObjectTable::getClassFromName(const wchar_t *classname) {
  for (int i=0;i<classes.getNumItems();i++) {
    if (classes[i] && !WCSICMP(classname, classes[i]->classname)) {
      return classes.enumItem(i)->classid;
    }
  }
  return -1;
}

int ObjectTable::getClassFromGuid(GUID g) {
  GUID t;
  for (int i=0;i<classes.getNumItems();i++) {
    t = classes.enumItem(i)->classGuid;
    if (g == t)
      return classes.enumItem(i)->classid;
  }
  return -1;
}

const wchar_t *ObjectTable::getClassName(int classid) {
  class_entry *e =getClassEntry(classid);
  if (!e) return NULL;
  return e->classname;
}

int ObjectTable::isExternal(int classid) {
  for (int i=0;i<classes.getNumItems();i++)
    if (classes.enumItem(i)->classid == classid) 
      return 1;
  return 0;
}

int ObjectTable::getNumGuids() {
  return classes.getNumItems();
}

GUID ObjectTable::enumGuid(int i) {
  return classes.enumItem(i)->classGuid;
}

const wchar_t *ObjectTable::enumClassName(int n) {
  return classes.enumItem(n)->classname;
}

int ObjectTable::getClassEntryIdx(int classid) {
  for (int i=0;i<classes.getNumItems();i++)  
    if (classes[i]->classid == classid)
      return i;
  return -1;
}

int ObjectTable::isDescendant(int class1, int classid) {

  if (classid < CLASS_ID_BASE) return 0;
  if (class1 < CLASS_ID_BASE) return 0;

  class_entry *e = getClassEntry(classid);
  //CUT: class_entry *_e = getClassEntry(class1);

  while (e) {
    if (class1 == classid) return 1;
    e = getClassEntry(e->ancestorclassid);
    if (e) classid = e->classid;
  }
  return 0;
}

int ObjectTable::isClassInstantiable(int classid) {
  class_entry *e =getClassEntry(classid);
  if (!e) return 0;
  return e->instantiable;
}

int ObjectTable::isClassReferenceable(int classid) {
  class_entry *e =getClassEntry(classid);
  if (!e) return 0;
  return e->referenceable;
}

ScriptObject *ObjectTable::instantiate(int classid) {
  class_entry *e = getClassEntry(classid);
  if (!e) return NULL;
  ScriptObject *o = e->controller->instantiate();
  return o;
}

void *ObjectTable::encapsulate(int classid, ScriptObject *o) {
  class_entry *e = getClassEntry(classid);
  if (!e) return NULL;
  void *itf = e->controller->encapsulate(o);
  if (itf) 
    o->vcpu_setInterface(e->classGuid, itf);
  return itf;
}

void ObjectTable::destroy(ScriptObject *o) {
  if (!o) return;
  class_entry *e = getClassEntry(getClassFromName(o->vcpu_getClassName()));
  if (!e) return;
  e->controller->destroy(o);
}

void ObjectTable::deencapsulate(int classid, void *o) {
  class_entry *e = getClassEntry(classid);
  if (!e) return;
  e->controller->deencapsulate(o);
}

const wchar_t *ObjectTable::getFunction(int dlfid, int *n, ScriptObjectController **p) 
{
  VCPUdlfEntry *e = VCPU::getGlobalDlfEntry(dlfid);
  if (p) *p = getClassEntry(e->basetype)->controller;
  if (n) *n = e->nparams;
  return e->functionName;
}

scriptVar ObjectTable::callFunction(ScriptObject *obj, int dlfid, scriptVar **params) {
  VCPUdlfEntry *e = VCPU::getGlobalDlfEntry(dlfid);
  VCPU::push(MAKE_SCRIPT_OBJECT(obj));
  for (int i=e->nparams-1;i>=0;i--)
    VCPU::push(*params[i]);
  return VCPU::callDLF(e, -1);
}

int ObjectTable::dlfAddRef(ScriptObjectController *o, const wchar_t *function_name, void *host) 
{
  ScriptObjectController *_o = o;
  while (_o) {
    const function_descriptor_struct *s = _o->getExportedFunctions();
    for (int i=0;i<_o->getNumFunctions();i++) {
      if (!WCSICMP(function_name, s[i].function_name)) {
        return dlfAddRef(_o, i, host);
      }
    }
    // function not found, see if ancestor has it
    _o = _o->getAncestorController();
  }
  ASSERTALWAYS(StringPrintf("Function %s not found in %s class object hierarchy", function_name, o->getClassName()));
  return -1;
}

int ObjectTable::dlfAddRef(ScriptObjectController *o, int i, void *host) {
  const function_descriptor_struct *s = o->getExportedFunctions();
  int id = VCPU::getDLFFromPointer(s[i].physical_ptr, s[i].nparams);
  if (id < 0) { // not yet set
    // allocate new vcpudlfentry and insert it in vcpu
    id = VCPU::newDlf();
    VCPUdlfEntry e;
    e.basetype = o->getClassId();
    e.DLFid = id;
//    DebugString("  s = %08X\n", s);
//    DebugString("  s[i] = %08X\n", s[i]);
    ASSERT(s != NULL);
    e.functionName = const_cast<wchar_t *>(s[i].function_name);
//    DebugString("  %s\n", e.functionName);
    e.nparams = s[i].nparams;
    e.ptr = s[i].physical_ptr;
    e.scriptId = -1;
    VCPU::setupDLFFunction(e.ptr, e.nparams, id, &e);
  } 
  VCPU::DLF_addref(s[i].physical_ptr, s[i].nparams);
  if (host != NULL) {
    hostrefstruct *r = new hostrefstruct;
    r->host = host;
    r->nargs = s[i].nparams;
    r->ptr = s[i].physical_ptr;
    hostrefs.addItem(r);
  }
  return id;
}

void ObjectTable::dlfAddClassRef(ScriptObjectController *o, void *host) {
//  while (o) {
    //CUT: const function_descriptor_struct *s = o->getExportedFunctions();
    for (int i=0;i<o->getNumFunctions();i++) {
      dlfAddRef(o, i, host);
    }
//    o = o->getAncestorController();
//  }
}


void ObjectTable::dlfRemRef(void *host) {
  for (int i=0;i<hostrefs.getNumItems();i++) {
    hostrefstruct *r = hostrefs.enumItem(i);
    if (r->host == host) {
      //VCPU::DLF_remref(r->ptr, r->nargs); // TODO: re-enable after fixup
      delete r;
      hostrefs.removeByPos(i);
      i--;
    }
  }
}

int ObjectTable::checkScript(SystemObject *o) {
  TList<int> *l = o->getTypesList();
  if (!l) return 0;
  for (int i=0;i<l->getNumItems();i++) {
    if (l->enumItem(i) == -1) {
//#ifdef WASABI_COMPILE_WNDMGR
//      WASABI_API_WNDMGR->messageBox("Error while loading a script, a component is missing", "Oops", 0, "", NULL);
//#else
//      MessageBox(NULL, "Error while loading a script, a component is missing", "Oops", 0);
//#endif
//      return 0;
    } else {

    }
  }
  return 1;
}

ScriptObjectController *ObjectTable::getController(GUID g) {
  int i = getClassFromGuid(g);
  if (i == -1) return NULL;
  class_entry *e = getClassEntry(i);
  if (e)
    return e->controller;  
  return NULL;
}

class_entry *ObjectTable::getClassEntry(int classid) {
  for (int i=0;i<classes.getNumItems();i++)  
    if (classes[i]->classid == classid)
      return classes[i];
  return NULL;
}

#ifdef _NOSTUDIO

int ObjectTable::validateMember(int classid, char *member, ControlBlock *parms, int *ret) {
  ControlBlock *p = parms;
  int r=0;
  for (int i=0;i<classes.getNumItems();i++) 
    if (classes.enumItem(i)->classid == classid) {
      const function_descriptor_struct *s = classes.enumItem(i)->controller->getExportedFunctions();
      for (int j=0;j<classes.enumItem(i)->controller->getNumFunctions();j++) {
        if (STRCASEEQL(s->function_name, member)) {
          if (s->nparams == 0 && parms)
            wrongParametersNumber(member, s->nparams);
          for (int k=0;k<s->nparams&&!r;k++) { 
            int b = s->param[k];
            if (p == NULL) { 
              wrongParametersNumber(member, s->nparams);
            }
            if (!::isDescendant(p, b)) { 
              wrongParameterType(k, member, (char *)getClassName(p->getBaseClass()), (char *)getClassName(classid)); 
            }
          p = p->getNext(); 
          } 
        *ret = tempExternalToInternal(s->return_type);
        return 1;
        }
      s++;
      }
    }
  return 0;
}

#endif

//-----

PtrList < class_entry > ObjectTable::classes;
PtrList < hostrefstruct > ObjectTable::hostrefs;
int ObjectTable::classidx = 0;
int ObjectTable::externalloaded = 0;
#endif
