#ifdef WASABI_COMPILE_WNDMGR
#ifndef __CONTAINER_H
#define __CONTAINER_H

class Container;
class Layout;

#include <bfc/ptrlist.h>
#include <api/skin/xmlobject.h>

#ifdef WASABI_WIDGETS_GUIOBJECT
#include <api/script/objects/guiobj.h>
#endif
#include <api/script/script.h>
#include <api/script/scriptobj.h>
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>


#define CONTAINER_UNDEFINED_NAME L"undefined container name"

enum {
    CONTAINERPARAM_NAME = 0,
    CONTAINERPARAM_ID,
    CONTAINERPARAM_DYNAMIC,
    CONTAINERPARAM_DEFAULTX,
    CONTAINERPARAM_DEFAULTY,
    CONTAINERPARAM_DEFAULTVISIBLE,
    CONTAINERPARAM_CANCLOSE,
    CONTAINERPARAM_NOMENU,
    CONTAINERPARAM_NOFOCUSAPPONCLOSE,
		CONTAINERPARAM_CONTENT,
};

class ContainerScriptController: public ScriptObjectControllerI
{
public:

	virtual const wchar_t *getClassName();
	virtual const wchar_t *getAncestorClassName();
	virtual ScriptObjectController *getAncestorController();
	virtual int getNumFunctions();
	virtual const function_descriptor_struct *getExportedFunctions();
	virtual GUID getClassGuid();
	virtual ScriptObject *instantiate();
	virtual void destroy(ScriptObject *o);
	virtual void *encapsulate(ScriptObject *o);
	virtual void deencapsulate(void *o);
	virtual int getInstantiable();

private:

	static function_descriptor_struct exportedFunction[];

};

class ContentEntry
{
public:
	ContentEntry(const wchar_t *gid, GUID g, ifc_window *w) : groupid(gid), guid(g), wnd(w) { }

	GUID guid;
	StringW groupid;
	ifc_window *wnd;
};

extern ContainerScriptController *containerController;

#define CONTAINER_SCRIPTPARENT RootObjectInstance

class Container : public CONTAINER_SCRIPTPARENT, public XmlObjectI, public DependentI
{

public:
	Container(int scriptid = -1);
	virtual ~Container();

	virtual void setName(const wchar_t *name);
	virtual void setId(const wchar_t *id);

	virtual const wchar_t *getName();
	virtual const wchar_t *getId();

	virtual int setXmlParam(const wchar_t *p, const wchar_t *v);

	virtual void onInit(int noshow = 0);

	virtual void addLayout(Layout *layout);
	virtual void resetLayouts();

	virtual void setVisible(int sh);
	virtual int isVisible();

	virtual void switchToLayout(const wchar_t *name, int moveit = 1);
	virtual void toggle();
	virtual void close();
	virtual void sysMenu();
	virtual int getNumLayouts();
	virtual Layout *enumLayout(int n);
	virtual Layout *getLayout(const wchar_t *name, int *pos = NULL);
	virtual int getScriptId();
	virtual void savePositions();
	void setDefaultLayout(const wchar_t *name);
	void loadFromDefaults(int noshow = 0);
	int isTranscient() { return transcient; }
	void setTranscient(int is) { transcient = is; }

	// player callbacks to notify that container/component
	// has been set visible/invisible
	virtual void otherContainerToggled(const wchar_t *id, int visible);
	virtual void componentToggled(GUID *g, int visible);

	virtual int isMainContainer();
	virtual int isDynamic();

	virtual void onSwitchToLayout(Layout *l);
	virtual void onBeforeSwitchToLayout(Layout *oldl, Layout *newl);
	virtual void onHideLayout(Layout *l);
	virtual void onShowLayout(Layout *l);
	virtual Layout *getCurrentLayout();
	virtual int getDefaultPositionX(void);
	virtual int getDefaultPositionY(void);
	virtual void onChildSetLayoutVisible(Layout *l, int v);

	virtual void notifyAddContent(ifc_window *w, const wchar_t *groupid, GUID guid = INVALID_GUID);
	virtual void notifyRemoveContent(ifc_window *w);
	virtual int hasContent(GUID g);
	virtual GUID getDefaultContent();

	const wchar_t *getDescriptor(void);

	virtual void getWindowRect(RECT *r);

	virtual void sendNotifyToAllLayouts(int notifymsg, int param1, int param2);
	void setDynamic(int i);

	void updateDefaultVisible();
	void setDeleting() { deleting = 1; }
	int isDeleting() { return deleting; }
	int isInited() { return inited; }

	int canClose() { return canclose; }
	void setCanClose(int c) { canclose = c; }
	GUID getGUID() { return myGUID; }

	int getNoMenu() { return nomenu; }

	void setPreventSaveVisibility(int p) { prevent_save_visibility = p; }
	int wantRefocusApp() { return refocusapponclose; }
	void setWantRefocusApp(int rf) { refocusapponclose = rf; }

	public: 
		// ifc_dependent stuff

	static const GUID *depend_getClassGuid() {
		// {C439D9F4-34F5-453c-B947-448ED20203FC}
		static const GUID ret = 
		{ 0xc439d9f4, 0x34f5, 0x453c, { 0xb9, 0x47, 0x44, 0x8e, 0xd2, 0x2, 0x3, 0xfc } };

		return &ret;
	}
	enum
	{
		Event_NAMECHANGE = 0,
	};


private:
	void containerCallback(int msg);
	StringW containerName;
	StringW containerId;
	PtrList<Layout> layouts;
	int currentLayout;
	StringW last_layout;
	int canclose;
	int lastLayout;
	int default_visible, loaded_default_visible;
	int ismain;
	int dynamic;
	int default_x = -1;
	int default_y = -1;
	int default_w = -1;
	int default_h = -1;
	int scriptid;
	int inited;
	int deleting;
	int transcient;
	PtrList<ContentEntry> contents;
	int switching_layout;
	int nomenu;
	int prevent_save_visibility;
	int refocusapponclose;
	GUID contentGuid;
	bool hasContentGuid;

private:
	GUID myGUID;
	void showDefaultLayout(int noshow = 0);

public:
	static scriptVar script_vcpu_getId(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_onSwitchToLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l);
	static scriptVar script_onBeforeSwitchToLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar oldl, scriptVar newl);
	static scriptVar script_onHideLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l);
	static scriptVar script_onShowLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l);
	static scriptVar script_getLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj);
	static scriptVar script_switchToLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s);
	static scriptVar script_getCurrentLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_show(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_hide(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_close(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_toggle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_getNumLayouts(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_enumLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n);
	static scriptVar script_isDynamic(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_setName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name);
	static scriptVar script_getName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_getGuid(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_setXmlParam(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar p, scriptVar v);
	static scriptVar script_onAddContent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar window, scriptVar id, scriptVar g);

};

enum {
    CONT_CB_NONE,
    CONT_CB_VISIBLE,
    CONT_CB_HIDDEN
};

#endif
#endif
