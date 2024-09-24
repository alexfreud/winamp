#include <precomp.h>
#include "wasabicfg.h"
#include <api/wndmgr/container.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/config/items/attrbool.h>
#include <api/config/items/cfgitem.h>
#include <api/skin/skinparse.h>
#include <api/wac/compon.h>
#include <api/wnd/wndtrack.h>
#include <api/skin/skin.h>
#include <api/wndmgr/skinembed.h>
#include <api/syscb/callbacks/wndcb.h>
#include <bfc/string/stringdict.h>
#include <api/skin/widgets/xuiwndholder.h> // TODO: cut, but we need XuiWindowHolder::parseGUID for now
#include <bfc/platform/guid.h>

#ifdef WIN32
#include "resource.h"
#include "../Agave/Language/api_language.h"
#endif

BEGIN_STRINGDICTIONARY(_containerparams)
SDI(L"name", CONTAINERPARAM_NAME);
SDI(L"id", CONTAINERPARAM_ID);
SDI(L"dynamic", CONTAINERPARAM_DYNAMIC);
SDI(L"default_x", CONTAINERPARAM_DEFAULTX);
SDI(L"default_y", CONTAINERPARAM_DEFAULTY);
SDI(L"default_visible", CONTAINERPARAM_DEFAULTVISIBLE);
SDI(L"canclose", CONTAINERPARAM_CANCLOSE);
SDI(L"nomenu", CONTAINERPARAM_NOMENU);
SDI(L"nofocusapponclose", CONTAINERPARAM_NOFOCUSAPPONCLOSE);
SDI(L"primarycomponent", CONTAINERPARAM_CONTENT);
END_STRINGDICTIONARY(_containerparams, containerparams);

Container::Container(int script_id)
{
	getScriptObject()->vcpu_setInterface(containerGuid, (void *)static_cast<Container *>(this));
	getScriptObject()->vcpu_setClassName(L"Container");
	getScriptObject()->vcpu_setController(containerController);
	scriptid = script_id;
	currentLayout = -1;
	default_visible = TRUE;
	loaded_default_visible = TRUE;
	dynamic = 0;
	MEMCPY(&myGUID, &INVALID_GUID, sizeof(GUID));
	lastLayout = -1;
	refocusapponclose = 1;
	canclose = 1;
	inited = 0;
	deleting = 0;
	transcient = 0;
	switching_layout = 0;
	nomenu = 0;
	prevent_save_visibility = 0;
	contentGuid = INVALID_GUID;
	hasContentGuid=false;
}


void Container::setName(const wchar_t *name)
{
#ifdef ON_TWEAK_CONTAINER_NAMEW
	ON_TWEAK_CONTAINER_NAMEW(name);
#endif
	containerName = name ? name : CONTAINER_UNDEFINED_NAME;
	foreach(layouts)
	Layout *l = layouts.getfor();
	if (l->getRootWndName() == NULL)
	{
		l->setOSWndName(name);
	}
	endfor;
	updateDefaultVisible();

	dependent_sendEvent(Container::depend_getClassGuid(), Event_NAMECHANGE, 0, this);
}

void Container::resetLayouts()
{
	updateDefaultVisible();

	foreach(layouts)
	layouts.getfor()->loadSavedState();
	endfor;
}

void Container::setId(const wchar_t *id)
{
	containerId = id ? id : L"undefined";
	ismain = !WCSICMP(id, L"main");
}

int Container::setXmlParam(const wchar_t *paramname, const wchar_t *strvalue)
{
	switch (containerparams.getId(paramname))
	{
	case CONTAINERPARAM_NAME:
		setName(strvalue);
		return 1;
	case CONTAINERPARAM_ID:
		setId(strvalue);
		return 1;
	case CONTAINERPARAM_DYNAMIC:
		setDynamic(WTOI(strvalue));
		return 1;
	case CONTAINERPARAM_DEFAULTX:
		default_x = WTOI(strvalue);
		return 1;
	case CONTAINERPARAM_DEFAULTY:
		default_y = WTOI(strvalue);
		return 1;
	case CONTAINERPARAM_DEFAULTVISIBLE:
		default_visible = WTOI(strvalue);
		updateDefaultVisible();
		return 1;
	case CONTAINERPARAM_CANCLOSE:
		canclose = WTOI(strvalue);
		return 1;
	case CONTAINERPARAM_NOMENU:
		nomenu = WTOI(strvalue);
		return 1;
	case CONTAINERPARAM_NOFOCUSAPPONCLOSE:
		refocusapponclose = !WTOI(strvalue);
		break;
	case CONTAINERPARAM_CONTENT:
		{
			// TODO: move this out of XuiWindowHolder
			GUID *g = XuiWindowHolder::parseGUID(strvalue);
			contentGuid = *g;
			hasContentGuid=true;
			
			break;
		}

	}
	return 0;
}

Container::~Container()
{
	containerCallback(CONT_CB_HIDDEN);
	layouts.deleteAll();
	contents.deleteAll();
	SkinParser::containers.removeItem(this);
}

void Container::getWindowRect(RECT *r)
{
	Layout *l = getCurrentLayout();
	if (l)
		l->getWindowRect(r);
}

void Container::addLayout(Layout *layout)
{
	layouts.addItem(layout);
}

const wchar_t *Container::getName()
{
	return containerName;
}

const wchar_t *Container::getId()
{
	return containerId;
}

int Container::getDefaultPositionX(void)
{
	return default_x;
}

int Container::getDefaultPositionY(void)
{
	return default_y;
}

void Container::onInit(int noshow)
{
	if (inited) return ;
	inited++;
	loadFromDefaults(noshow);
}

void Container::loadFromDefaults(int noshow)
{
#ifdef WASABI_COMPILE_CONFIG
	if (last_layout.isempty())
	{
		StringPrintfW tmp(L"container_%s|active", getName());
		wchar_t c[512] = {0};
		WASABI_API_CONFIG->getStringPrivate(tmp, c, 511, L"");
		c[510] = 0;
		last_layout = c;
	}
#endif

	if (loaded_default_visible)
	{
		showDefaultLayout(noshow);
	}
}

void Container::setDefaultLayout(const wchar_t *name)
{
	last_layout = name;
}

void Container::showDefaultLayout(int noshow)
{
	Layout *l = getLayout(last_layout);

	if (!l)
		l = layouts.enumItem(0);

	if (l)
	{
		currentLayout = layouts.searchItem(l);

		if (!noshow)
			l->setVisible(1);

		containerCallback(CONT_CB_VISIBLE); // since we set currentLayout prior to showing the layout, the callback will not be processed in onChildSetLayoutVisible
	}
}

void Container::setVisible(int sh)
{
	if (!sh && !canclose)
		return ;

	int is = isVisible();

	if (!!sh == !!is)
		return ;

	Layout *l = getCurrentLayout();

	if (!l && lastLayout != -1)
		l = layouts[lastLayout];

	if (sh)
	{
		if (!l)
			showDefaultLayout();
		else
		{
			l->setVisible(1);
		}
	}
	else
	{
		if (l)
			l->setVisible(0);
	}
}

int Container::isVisible()
{
	if (switching_layout) return 1;
	Layout *l = getCurrentLayout();
	if (!l) return 0;
	return l->isVisible();
}

void Container::onChildSetLayoutVisible(Layout *l, int v)
{
	for (int i = 0;i < layouts.getNumItems();i++)
	{
		if (layouts[i] == l)
		{
			if (v)
			{
				if (currentLayout != i)
				{
					Layout *l = NULL;
					if (currentLayout >= 0)
						l = layouts[currentLayout];
					if (l) l->setVisible(0);
					containerCallback(CONT_CB_VISIBLE);
					currentLayout = i;
					l = layouts[currentLayout];
#ifdef WASABI_COMPILE_CONFIG
					if (!isTranscient() && !prevent_save_visibility)
						WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"container_%s|active", getName()), l->getGuiObject()->guiobject_getId());
#endif
#ifdef WA3COMPATIBILITY
					l->setForwardMsgWnd(WASABI_API_WND->main_getRootWnd()->gethWnd());
#endif
					if (l->wantActivation())
					{
						l->bringToFront();
						l->setFocus();
					}
					l->invalidate();
				}
#ifdef WASABI_COMPILE_CONFIG
				if (!isTranscient() && !isDynamic() && !prevent_save_visibility)
					WASABI_API_CONFIG->setIntPrivate(StringPrintfW(L"activated/%s", getName()), v);
				WASABI_API_CONFIG->setIntPrivate(StringPrintfW(L"everloaded/%s", getName()), 1);
#endif

			}
			else
			{
				if (i == currentLayout)
				{
					if (!isDeleting() && !isTranscient() && !isDynamic())
					{
#ifdef WASABI_COMPILE_CONFIG
						WASABI_API_CONFIG->setIntPrivate(StringPrintfW(L"activated/%s", getName()), v);
#endif
						lastLayout = currentLayout;
						currentLayout = -1;
					}
					containerCallback(CONT_CB_HIDDEN);
				}
			}
			return ;
		}
	}
}

void Container::containerCallback(int msg)
{
	switch (msg)
	{
	case CONT_CB_VISIBLE:
		{
			foreach(contents)
			ContentEntry *e = contents.getfor();
			WndInfo i;
			i.groupid = e->groupid;
			i.guid = e->guid;
			i.c = this;
			WASABI_API_SYSCB->syscb_issueCallback(SysCallback::WINDOW, WndCallback::SHOWWINDOW, reinterpret_cast<intptr_t>(&i));
			endfor
			WndInfo i;
			i.groupid = getId();
			i.guid = INVALID_GUID;
			i.c = this;
			WASABI_API_SYSCB->syscb_issueCallback(SysCallback::WINDOW, WndCallback::SHOWWINDOW, reinterpret_cast<intptr_t>(&i));
		}
		break;
	case CONT_CB_HIDDEN:
		{
			foreach(contents)
			ContentEntry *e = contents.getfor();
			WndInfo i;
			i.groupid = e->groupid;
			i.guid = e->guid;
			i.c = this;
			WASABI_API_SYSCB->syscb_issueCallback(SysCallback::WINDOW, WndCallback::HIDEWINDOW, reinterpret_cast<intptr_t>(&i));
			endfor
			WndInfo i;
			i.groupid = getId();
			i.guid = INVALID_GUID;
			i.c = this;
			WASABI_API_SYSCB->syscb_issueCallback(SysCallback::WINDOW, WndCallback::HIDEWINDOW, reinterpret_cast<intptr_t>(&i));
		}
		break;
	}
}

void Container::close()
{
	int norefocs = !wantRefocusApp();
	if (norefocs) WASABI_API_WND->appdeactivation_push_disallow(NULL);
	if (!canclose) return ;
	if (isDynamic())
	{
		setVisible(0);
		skinEmbedder->destroyContainer(this); // deferred
	}
	else
		setVisible(0);
	if (norefocs) WASABI_API_WND->appdeactivation_pop_disallow(NULL);
}

void Container::toggle()
{
	if (isVisible()) close(); else setVisible(1); // close has special function hide/destroy depending on dynamic status
}

void Container::switchToLayout(const wchar_t *name, int moveit)
{
	int l = -1;
	getLayout(name, &l);

	if (l == -1)
	{
		// Layout not found, reverting to first in the list
		if (layouts.getNumItems() > 0) l = 0;
		else
			return ; // none found, abort
	}

	switching_layout = 1;
	if (l != currentLayout)
	{
		int old = currentLayout;
		RECT r = {0};
		Layout *oldlayout = old >= 0 ? layouts[old] : NULL;
		Layout *newlayout = layouts[l];
		onBeforeSwitchToLayout(oldlayout, newlayout);
		if (oldlayout)
		{
			oldlayout->getWindowRect(&r);
			oldlayout->endCapture();
		}
		int unlinked = layouts[l]->isUnlinked();
		unlinked |= oldlayout ? oldlayout->isUnlinked() : 0;
		if (moveit && !unlinked && oldlayout)
			layouts[l]->move(r.left, r.top);
#ifdef WASABI_COMPILE_CONFIG
		// {9149C445-3C30-4e04-8433-5A518ED0FDDE}
		const GUID uioptions_guid =
		    { 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };
		if (_intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid), L"Link layouts scale"))
		{
#else
		if (WASABI_WNDMGR_LINKLAYOUTSCALES)
		{
#endif
			if (oldlayout)
			{
				double _r = oldlayout->getRenderRatio();
				newlayout->setRenderRatio(_r);
			}
		}
#ifdef WASABI_COMPILE_CONFIG
		if (_intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid), L"Link layouts alpha"))
		{
#else
		if (WASABI_WNDMGR_LINKLAYOUTSALPHA)
		{
#endif
			if (oldlayout)
			{
				int a = layouts[old]->getAlpha();
				newlayout->setAlpha(a);
				int autoopacify = layouts[old]->getAutoOpacify();
				newlayout->setAutoOpacify(autoopacify);
			}
		}

		WindowTracker::layoutChanged(oldlayout, newlayout);
#ifdef ON_LAYOUT_CHANGED
		ON_LAYOUT_CHANGED;
#endif
		layouts[l]->setVisible(1);
		foreach(SkinParser::containers)
		SkinParser::containers.getfor()->savePositions();
		endfor
		onSwitchToLayout(newlayout);
	}
	switching_layout = 0;
}

void Container::savePositions()
{
	foreach (layouts)
	layouts.getfor()->savePosition();
	endfor
}

Layout *Container::getLayout(const wchar_t *name, int *pos)
{
	for ( int i = 0; i < getNumLayouts(); i++ )
	{
		if ( WCSCASEEQLSAFE( enumLayout( i )->getGuiObject()->guiobject_getId(), name ) )
		{
			if (pos)
			{
				*pos = i;
			}
			
			return enumLayout( i );
			
		}
	}

	return NULL;
}

Layout *Container::getCurrentLayout()
{
	if (currentLayout < 0)
		return NULL;

	return layouts.enumItem(currentLayout);
}

void Container::otherContainerToggled(const wchar_t *id, int visible)
{
	for (int i = 0;i < layouts.getNumItems();i++)
		layouts[i]->containerToggled(id, visible);
}

void Container::componentToggled(GUID *g, int visible)
{
	for (int i = 0;i < layouts.getNumItems();i++)
		layouts[i]->componentToggled(g, visible);
}

void Container::sendNotifyToAllLayouts(int notifymsg, int param1, int param2)
{
	for (int i = 0;i < layouts.getNumItems();i++)
		layouts[i]->sendNotifyToAllChildren(notifymsg, param1, param2);
}

int Container::isMainContainer()
{
	return ismain;
}

void Container::sysMenu()
{
	POINT p;
	Wasabi::Std::getMousePos(&p);
	layouts.enumItem(currentLayout)->onRightButtonDown(p.x, p.y);
}

void Container::setDynamic(int i)
{
	dynamic = i;
	if (!containerId.isempty())
		setId(containerId);
}

int Container::isDynamic()
{
	return dynamic;
}

void Container::onSwitchToLayout(Layout *l)
{
	ScriptObject *ls = l ? l->getGuiObject()->guiobject_getScriptObject() : NULL;
	script_onSwitchToLayout(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_OBJECT(ls));
}

void Container::onBeforeSwitchToLayout(Layout *oldl, Layout *newl)
{
	ScriptObject *olds = oldl ? oldl->getGuiObject()->guiobject_getScriptObject() : NULL;
	ScriptObject *news = newl ? newl->getGuiObject()->guiobject_getScriptObject() : NULL;
	script_onBeforeSwitchToLayout(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_OBJECT(olds), MAKE_SCRIPT_OBJECT(news));
}


void Container::onHideLayout(Layout *l)
{
	script_onHideLayout(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_OBJECT(l->getGuiObject()->guiobject_getScriptObject()));
}

void Container::onShowLayout(Layout *l)
{
	script_onShowLayout(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_OBJECT(l->getGuiObject()->guiobject_getScriptObject()));
}

int Container::getNumLayouts()
{
	return layouts.getNumItems();
}

Layout *Container::enumLayout(int n)
{
	return layouts.enumItem(n);
}

const wchar_t *Container::getDescriptor()
{
	static wchar_t d[256];
	// if we've tweaked the container names then when saving out we can see and attempt to 'untweak'
	// so that things like Winamp Modern's ML will be correctly positioned irrespective of language
	int untweak = 0;
	wchar_t tweaked[96] = {0};
	// Martin> We need to check the containerName against null - dunno why some skins cause this string to be null
	if(containerName.v() != NULL && !_wcsicmp(containerName.v(), WASABI_API_LNGSTRINGW_BUF(IDS_MEDIA_LIBRARY,tweaked,96))){
		untweak = 1;
	}
	WCSNPRINTF(d, 256,L"%s/%s", getId(), (!untweak ? containerName.v() : L"Media Library"));
	return d;
}

void Container::updateDefaultVisible()
{
	if (!canclose)
	{
		loaded_default_visible = 1;
		return ;
	}
#ifdef WASABI_COMPILE_CONFIG
	loaded_default_visible = WASABI_API_CONFIG->getIntPrivate(StringPrintfW(L"activated/%s", getName()), default_visible);
#else
	loaded_default_visible = default_visible;
#endif
}

int Container::getScriptId()
{
	return scriptid;
}

void Container::notifyAddContent(ifc_window *w, const wchar_t *id, GUID g)
{
	contents.addItem(new ContentEntry(id, g, w));
	ScriptObject *_w = w ? w->getGuiObject()->guiobject_getScriptObject() : NULL;
	wchar_t guidstr[256] = {0};
	nsGUID::toCharW(g, guidstr);
	script_onAddContent(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_OBJECT(_w), MAKE_SCRIPT_STRING(id), MAKE_SCRIPT_STRING(guidstr));
}

void Container::notifyRemoveContent(ifc_window *w)
{
	foreach(contents)
	ContentEntry *e = contents.getfor();
	if (e->wnd == w)
	{
		contents.removeItem(e);
		delete e;
		return ;
	}
	endfor;
}

int Container::hasContent(GUID g)
{
	foreach(contents)
	ContentEntry *e = contents.getfor();
	if (e->guid == g)
	{
		return 1;
	}
	endfor;
	return 0;
}

GUID Container::getDefaultContent()
{
	if (hasContentGuid)
		return contentGuid;
	if (contents.getNumItems() > 0) return contents.enumItem(0)->guid;
	return INVALID_GUID;
}

ContainerScriptController _containerController;
ContainerScriptController *containerController = &_containerController;


// -- Functions table -------------------------------------
function_descriptor_struct ContainerScriptController::exportedFunction[] = {
            {L"onSwitchToLayout", 1, (void*)Container::script_onSwitchToLayout },
            {L"onBeforeSwitchToLayout", 2, (void*)Container::script_onBeforeSwitchToLayout },
            {L"onHideLayout", 1, (void*)Container::script_onHideLayout },
            {L"onShowLayout", 1, (void*)Container::script_onShowLayout },
            {L"getLayout", 1, (void*)Container::script_getLayout },
            {L"getNumLayouts", 0, (void*)Container::script_getNumLayouts },
            {L"enumLayout", 1, (void*)Container::script_enumLayout },
            {L"getCurLayout", 0, (void*)Container::script_getCurrentLayout},
            {L"switchToLayout", 1, (void*)Container::script_switchToLayout },
            {L"isDynamic", 0, (void*)Container::script_isDynamic },
            {L"show", 0, (void*)Container::script_show },
            {L"hide", 0, (void*)Container::script_hide },
            {L"close", 0, (void*)Container::script_close},
            {L"toggle", 0, (void*)Container::script_toggle },
            {L"setName", 1, (void*)Container::script_setName },
			{L"getName", 0, (void*)Container::script_getName },
			{L"getGuid", 0, (void*)Container::script_getGuid },
            {L"setXmlParam", 2, (void*)Container::script_setXmlParam},
						{L"onAddContent", 3, (void*)Container::script_onAddContent},
        };
// --------------------------------------------------------

/*SET_HIERARCHY(Container, SCRIPT_CONTAINER);
SET_HIERARCHY2(Container, SCRIPT_CONTAINER, CONTAINER_SCRIPTPARENT);*/

const wchar_t *ContainerScriptController::getClassName()
{
	return L"Container";
}

const wchar_t *ContainerScriptController::getAncestorClassName()
{
	return L"Object";
}

ScriptObjectController *ContainerScriptController::getAncestorController()
{
	return rootScriptObjectController;
}

int ContainerScriptController::getInstantiable()
{
	return 1;
}

ScriptObject *ContainerScriptController::instantiate()
{
	Container *c = new Container;
	return c->getScriptObject();
}

void ContainerScriptController::destroy(ScriptObject *o)
{
	//ASSERTALWAYS("be nice, don't do that");
	Container *c = (Container *)o->vcpu_getInterface(containerGuid);
	delete c;
}

void *ContainerScriptController::encapsulate(ScriptObject *o)
{
	return NULL;
}

void ContainerScriptController::deencapsulate(void *o)
{}


int ContainerScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *ContainerScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID ContainerScriptController::getClassGuid()
{
	return containerGuid;
}

//-----------------------------------------------------------------------

scriptVar Container::script_onSwitchToLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, containerController, l);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, l);
}

scriptVar Container::script_onBeforeSwitchToLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar oldl, scriptVar newl)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS2(o, containerController, oldl, newl);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT2(o, oldl, newl);
}

scriptVar Container::script_onHideLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, containerController, l);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, l);
}

scriptVar Container::script_onShowLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, containerController, l);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, l);
}

// Get an layout from its ID
scriptVar Container::script_getLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(obj.type == SCRIPT_STRING); // compiler discarded
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c)
	{
		for (int i = 0;i < c->layouts.getNumItems();i++)
			if (!WCSICMP(obj.data.sdata, c->layouts.enumItem(i)->getGuiObject()->guiobject_getId()))
			{
				return MAKE_SCRIPT_OBJECT(c->layouts.enumItem(i)->getGuiObject()->guiobject_getScriptObject());
			}
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar Container::script_show(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c) c->setVisible(1);
	RETURN_SCRIPT_VOID;
}
scriptVar Container::script_hide(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c) c->setVisible(0);
	RETURN_SCRIPT_VOID;
}

scriptVar Container::script_close(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c) c->close();
	RETURN_SCRIPT_VOID;
}

scriptVar Container::script_toggle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c) c->toggle();
	RETURN_SCRIPT_VOID;
}

// Switch to another layout
scriptVar Container::script_switchToLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar l)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c)
		c->switchToLayout(GET_SCRIPT_STRING(l));
	RETURN_SCRIPT_VOID;
}

scriptVar Container::script_getNumLayouts(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c) return MAKE_SCRIPT_INT(c->getNumLayouts());
	RETURN_SCRIPT_ZERO;
}

scriptVar Container::script_enumLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&n)); // compiler discarded
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	Layout *l = NULL;
	if (c) l = c->enumLayout(SOM::makeInt(&n));
	return MAKE_SCRIPT_OBJECT(l ? l->getScriptObject() : NULL);
}

scriptVar Container::script_getCurrentLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	Layout *l = c->getCurrentLayout();
	return MAKE_SCRIPT_OBJECT(l ? l->getScriptObject() : NULL);
}

// Switch to another layout
scriptVar Container::script_vcpu_getId(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	
	if (c)
		return MAKE_SCRIPT_STRING(c->getId());

	return MAKE_SCRIPT_STRING(L"");
}

scriptVar Container::script_isDynamic(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c) return MAKE_SCRIPT_INT(c->isDynamic());
	RETURN_SCRIPT_ZERO;
}

scriptVar Container::script_setName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c) c->setName(GET_SCRIPT_STRING(name));
	RETURN_SCRIPT_VOID;
}

scriptVar Container::script_getName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c) return MAKE_SCRIPT_STRING(c->getName());
	return MAKE_SCRIPT_STRING(L"");  
}

scriptVar Container::script_getGuid(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c)
	{
		static wchar_t guidstr[256];
		nsGUID::toCharW(c->getGUID(), guidstr);
		return MAKE_SCRIPT_STRING(guidstr);
	}
  return MAKE_SCRIPT_STRING(L"");  
}

scriptVar Container::script_setXmlParam(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar p, scriptVar v)
{
	SCRIPT_FUNCTION_INIT
	Container *c = static_cast<Container *>(o->vcpu_getInterface(containerGuid));
	if (c) c->setXmlParam(GET_SCRIPT_STRING(p), GET_SCRIPT_STRING(v));
	RETURN_SCRIPT_VOID;
}

//void Container::notifyAddContent(ifc_window *w, const wchar_t *id, GUID g)
scriptVar Container::script_onAddContent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar window, scriptVar id, scriptVar g)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS3(o, containerController, window, id, g);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT3(o, window, id, g);
}

