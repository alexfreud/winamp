#include <precomp.h>
#include <api.h>
#include <api/skin/widgets/mb/scriptbrowser.h>
#include <api/skin/skinparse.h>
#include <api/script/scriptmgr.h>
//#include <api/wac/main.h>//CUT!!
#include <api/skin/skinfont.h>
#include <api/skin/skin.h>
#include <api/skin/skinelem.h>
#include <api/font/font.h>
#include <api/wndmgr/snappnt.h>
#include <bfc/parse/pathparse.h>
#include <api/skin/guitree.h>
#ifdef WASABI_COMPILE_COMPONENTS
#include <api/wac/compon.h>
#endif
#include <api/service/svc_enum.h>
#include <api/script/objects/guiobject.h>
#ifdef WASABI_COMPILE_WNDMGR
#include <api/wndmgr/autopopup.h>
#endif
#include <bfc/parse/paramparser.h>
#include <api/skin/gammamgr.h>
#include <bfc/util/profiler.h>
#ifdef WASABI_COMPILE_LOCALES
#include <api/locales/xlatstr.h>
#include <api/locales/localesmgr.h>
#else
#define _
#endif
#include <bfc/string/stringdict.h>
#include <api/skin/widgets.h>

#ifdef _WIN32
extern HINSTANCE hInstance;
#endif

#define COLOR_WHITE (0xffffff)
#define COLOR_BLACK (0x000000)
#define COLOR_ERROR (0xff00ff)

// with alpha
#define COLOR_WHITEA (0xffffffff)
#define COLOR_BLACKA (0xff000000)
#define COLOR_ERRORA (0xffff00ff)

xml_tag taglist[] = {
                        {L"groupdef", XML_TAG_GROUPDEF, 1},
                        {L"group", XML_TAG_GROUP, 1},
                        {L"cfggroup", XML_TAG_CFGGROUP, 1},
                        {L"elements", XML_TAG_ELEMENTS, 1},
                        {L"snappoint", XML_TAG_SNAPPOINT, 0},
                        {L"script", XML_TAG_SCRIPT, 0},
                        {L"container", XML_TAG_CONTAINER, 1},
                        {L"layout", XML_TAG_LAYOUT, 1},
						{L"elements", XML_ELEMENTTAG_ELEMENTS, 1},
						{L"accelerators", XML_TAG_ACCELERATORS, 1},
						{L"accelerator", XML_TAG_ACCELERATOR, 1},
						{L"stringtable", XML_TAG_STRINGTABLE, 1},
						{L"stringentry", XML_TAG_STRINGENTRY, 1},
                    };

BEGIN_STRINGDICTIONARY(_resizevalues)
SDI(L"top", RESIZE_TOP);
SDI(L"left", RESIZE_LEFT);
SDI(L"right", RESIZE_RIGHT);
SDI(L"bottom", RESIZE_BOTTOM);
SDI(L"topleft", RESIZE_TOPLEFT);
SDI(L"topright", RESIZE_TOPRIGHT);
SDI(L"bottomleft", RESIZE_BOTTOMLEFT);
SDI(L"bottomright", RESIZE_BOTTOMRIGHT);
END_STRINGDICTIONARY(_resizevalues, resizevalues)

BEGIN_STRINGDICTIONARY(_parsetypes)
SDI(L"resize", PARSETYPE_RESIZE);
SDI(L"color", PARSETYPE_COLOR);
SDI(L"coloralpha", PARSETYPE_COLORALPHA);
SDI(L"regionop", PARSETYPE_REGIONOP);
SDI(L"internal_action", PARSETYPE_INTERNALACTION);
SDI(L"group_inheritance", PARSETYPE_GROUPINHERITANCE);
END_STRINGDICTIONARY(_parsetypes, parsetypes)

BEGIN_STRINGDICTIONARY(_actionlist)
SDI(L"none", ACTION_NONE);
#ifdef WA3COMPATIBILITY
SDI(L"about", ACTION_ABOUT);
SDI(L"mb_forward", ACTION_MB_FORWARD);
SDI(L"mb_back", ACTION_MB_BACK);
SDI(L"mb_url", ACTION_MB_URL);
SDI(L"mb_home", ACTION_MB_HOME);
SDI(L"mb_stop", ACTION_MB_STOP);
SDI(L"mb_refresh", ACTION_MB_REFRESH);
SDI(L"text_larger", ACTION_TEXT_LARGER);
SDI(L"text_smaller", ACTION_TEXT_SMALLER);
SDI(L"preferences", ACTION_PREFERENCES);
SDI(L"view_file_info", ACTION_VIEW_FILE_INFO);
SDI(L"doublesize", ACTION_DOUBLESIZE);
SDI(L"add_bookmark", ACTION_ADD_BOOKMARK);
SDI(L"menu", ACTION_MENU);
SDI(L"sysmenu", ACTION_SYSMENU);
SDI(L"windowmenu", ACTION_WINDOWMENU);
SDI(L"controlmenu", ACTION_CONTROLMENU);
#endif // wa3compatibility
#ifdef WASABI_WIDGETS_COMPBUCK
SDI(L"cb_next", ACTION_CB_NEXT);
SDI(L"cb_prev", ACTION_CB_PREV);
SDI(L"cb_prevpage", ACTION_CB_PREVPAGE);
SDI(L"cb_nextpage", ACTION_CB_NEXTPAGE);
#endif
#ifdef WASABI_COMPILE_WNDMGR
SDI(L"endmodal", ACTION_ENDMODAL);
SDI(L"minimize", ACTION_MINIMIZE);
SDI(L"maximize", ACTION_MAXIMIZE);
SDI(L"close", ACTION_CLOSE);
SDI(L"close_window", ACTION_CLOSE_WINDOW);
SDI(L"switch", ACTION_SWITCH);
SDI(L"toggle", ACTION_TOGGLE);
SDI(L"reload_skin", ACTION_RELOAD_SKIN);
SDI(L"enforce_minmax", ACTION_ENFORCEMINMAX);
SDI(L"toggle_always_on_top", ACTION_TOGGLE_ALWAYS_ON_TOP);
#endif // wndmgr
END_STRINGDICTIONARY(_actionlist, actionlist)

#ifdef WASABI_COMPILE_MEDIACORE
BEGIN_STRINGDICTIONARY(_displaylist)
SDI(L"songname", DISPLAY_SONGNAME);
SDI(L"songinfo", DISPLAY_SONGINFO);
SDI(L"songartist", DISPLAY_SONGARTIST);
SDI(L"songtitle", DISPLAY_SONGTITLE);
SDI(L"songalbum", DISPLAY_SONGALBUM);
SDI(L"songlength", DISPLAY_SONGLENGTH);
SDI(L"time", DISPLAY_TIME);
SDI(L"timeelapsed", DISPLAY_TIME);
SDI(L"timeremaining", DISPLAY_TIME);
SDI(L"componentbucket", DISPLAY_CB);
SDI(L"songbitrate", DISPLAY_SONGBITRATE);
SDI(L"songsamplerate", DISPLAY_SONGSAMPLERATE);
SDI(L"songinfo_localise", DISPLAY_SONGINFO_TRANSLATED);
END_STRINGDICTIONARY(_displaylist, displaylist)
#endif // mediacore

static GUID staticguid;

void SkinParser::initialize()
{
	if (!quickxmltaglist.getNumItems())
	{
		for (int i = 0;i < sizeof(taglist) / sizeof(xml_tag);i++)
			quickxmltaglist.addItem(&taglist[i]);
	}

	// first two are for back compatibility
	skinXML.registerCallback(L"WinampAbstractionLayer", xmlReaderCallback);
	skinXML.registerCallback(L"WinampAbstractionLayer\f*", xmlReaderCallback);
	skinXML.registerCallback(L"WasabiXML", xmlReaderCallback);
	skinXML.registerCallback(L"WasabiXML\f*", xmlReaderCallback);

	guiTree = new GuiTree();

	xuiCache = new SvcCacheT<svc_xuiObject>;
}

void SkinParser::shutdown()
{
	skinXML.unregisterCallback((void*)xmlReaderCallback);
	delete guiTree; guiTree = NULL;
	delete xuiCache; xuiCache = NULL;
}

#ifdef WASABI_COMPILE_WNDMGR
void SkinParser::setInitialFocus()
{
	for (int i = 0;i < containers.getNumItems();i++)
	{
		if (containers[i]->isVisible())
		{
			Layout *l = containers[i]->getCurrentLayout();
			if (l)
			{
				l->setFocus();
				return ;
			}
		}
	}
#ifdef WIN32
#ifdef WA3COMPATIBILITY
	SetFocus(Main::gethWnd());
#endif //WA3COMPATIBILITY
#else
	DebugString( "portme -- SkinParser::setInitialFocus\n" );
#endif  //WIN32
}
#endif

#ifdef WASABI_COMPILE_WNDMGR 
// do not forget to popParserState(); before returning
int SkinParser::loadContainers(const wchar_t *skin)
{
	wchar_t olddir[WA_MAX_PATH] = {0};
	Wasabi::Std::getCurDir(olddir, WA_MAX_PATH);
	Wasabi::Std::setCurDir(WASABI_API_APP->path_getAppPath());
	int oldncontains = getNumContainers();
	pushParserState();
	allowscripts = 1;
	centerskin = 1;
	staticloading = 1;
	instantiatinggroup = 0;
	transcientcontainer = 0;
	inContainer = inLayout = 0;
	curGroup = NULL;
	recording_container = 0;
	recording_groupdef = 0;
	inElements = inGroup = inGroupDef = inAccelerators = inStringTable = 0;
	includepath = WASABI_API_SKIN->getSkinPath();
	loading_main_skinfile = 0;
	SkinElementsMgr::onBeforeLoadingSkinElements(includepath);
	GammaMgr::onBeforeLoadingGammaGroups();
	scriptId = WASABI_API_PALETTE->getSkinPartIterator();
	int skinType = Skin::checkSkin(skin);
	int retcode = 0;
	loading_main_skinfile = 1;
	switch (skinType)
	{
	case Skin::CHKSKIN_UNKNOWN:
		popParserState();
		break;
#ifdef WA3COMPATIBILITY
	case Skin::CHKSKIN_ISWA2:
		retcode = XmlReader::loadFile("svc:wa2skinxml", includepath);
		break;
#endif
	default:
		{
			retcode = skinXML.loadFile(StringPathCombine(includepath, L"skin.xml"), includepath);
			break;
		}
	}

	int n = guiTree->getNumObject(XML_TAG_CONTAINER);
	for (int i = 0;i < n;i++)
	{
		SkinItem *item = guiTree->getObject(XML_TAG_CONTAINER, i);
		if (item && item->getParams())
		{
			if (item->getParams()->getItemValueInt(L"dynamic"))
			{
				if (item->getParams()->getItemValueInt(L"default_visible"))
				{
					const wchar_t *name = item->getParams()->getItemValue(L"name");
#ifdef ON_TWEAK_CONTAINER_NAMEW
					ON_TWEAK_CONTAINER_NAMEW(name);
#endif
					wchar_t c[512]=L"-";
#ifdef WASABI_COMPILE_CONFIG
					WASABI_API_CONFIG->getStringPrivate(StringPrintfW(L"everloaded/%s", name), c, 511, L"-");
#endif
					c[510] = 0;
					if (c[0] == '-')
					{
						// never been created, create it now since it has default_visible
						staticloading = 0;
						/*Container *c = */instantiateDynamicContainer(item);
						staticloading = 1;
					}
				}
			}
		}
	}

	loading_main_skinfile = 0;

	Wasabi::Std::setCurDir(olddir);

	int ncontainersloaded = getNumContainers() - oldncontains;
	if (retcode == 0 || ncontainersloaded == 0)
	{
		return 0;
	}

#ifdef WASABI_COMPILE_CONFIG
	WASABI_API_CONFIG->setStringPrivate(L"last_skin", Skin::getSkinName());
#endif

	ASSERT(tha != NULL);
	SkinElementsMgr::onAfterLoadingSkinElements();
	GammaMgr::onAfterLoadingGammaGroups();

#ifdef WASABI_COMPILE_COMPONENTS
	ComponentManager::broadcastNotify(WAC_NOTIFY_SKINGUILOADED, WASABI_API_PALETTE->getSkinPartIterator());
#endif

	Skin::sendGuiLoadedCallback();
	popParserState();
	return ncontainersloaded;
}

void SkinParser::centerSkin()
{
	RECT sr;
	if (centerskin && getSkinRect(&sr))
	{
		int l = getNumContainers();
		int w = (Wasabi::Std::getScreenWidth() - (sr.right - sr.left)) / 2;
		int h = (Wasabi::Std::getScreenHeight() - (sr.bottom - sr.top)) / 2;
		for (int i = 0;i < l;i++)
		{
			Container *c = enumContainer(i);
			if (!c->isVisible()) continue;
			Layout *l = c->getCurrentLayout();
			RECT r;
			l->getWindowRect(&r);
			r.left += w;
			r.right += w;
			r.bottom += h;
			r.top += h;
			l->move(r.left, r.top);
		}
	}
	foreach(containers)
	containers.getfor()->savePositions();
	endfor;
}

int SkinParser::getSkinRect(RECT *r, ifc_window *exclude)
{
	if (!r) return 0;
	ZERO(*r);
	Container *cexcluded = NULL;
	if (exclude != NULL)
	{
		Layout *l = static_cast<Layout *>(exclude->getDesktopParent());
		if (l != NULL) cexcluded = l->getParentContainer();
	}
	int x = 99999, y = 99999, x2 = -1, y2 = -1;
	int l = getNumContainers();
	for (int i = 0;i < l;i++)
	{
		Container *c = enumContainer(i);
		if (c == cexcluded) continue;
		if (!c->isInited()) c->onInit();
		if (c->isDeleting() || !c->getCurrentLayout()) continue;
		int cx = c->getDefaultPositionX();
		int cy = c->getDefaultPositionY();
		if (cx == -1) cx = 0;
		if (cy == -1) cy = 0;
		RECT r;
		c->getWindowRect(&r);
		int cw = r.right - r.left;
		int ch = r.bottom - r.top;
		if (cx < x) x = cx;
		if (cy < y) y = cy;
		if ((cx + cw) > x2) x2 = cx + cw;
		if ((cy + ch) > y2) y2 = cy + ch;
	}
	if (x2 > 0 && y2 > 0 && x != 99999 && y != 99999)
	{
		Wasabi::Std::setRect(r, x, y, x2, y2);
		return 1;
	}
	return 0;
}
#endif

// do not forget to popParserState(); before returning
void SkinParser::loadScriptXml(const wchar_t *filename, int scriptid)
{
	pushParserState();
	allowscripts = 1;
	instantiatinggroup = 0;
#ifdef WASABI_COMPILE_WNDMGR
	transcientcontainer = 0;
	inContainer = inLayout = 0;
#endif
	staticloading = 1;
	recording_container = 0;
	recording_groupdef = 0;
	curGroup = NULL;
	inElements = inGroup = inGroupDef = inAccelerators = inStringTable = 0;
	scriptId = scriptid;

	//CUT  char file[WA_MAX_PATH];
	//CUT  char drive[WA_MAX_PATH];
	//CUT  char dir[WA_MAX_PATH];
	//CUT  char fname[WA_MAX_PATH];
	//CUT  char ext[WA_MAX_PATH];


	includepath.setValue(L"");

	wchar_t olddir[WA_MAX_PATH] = {0};
	Wasabi::Std::getCurDir(olddir, WA_MAX_PATH);
	Wasabi::Std::setCurDir(WASABI_API_APP->path_getAppPath());

	if (!WCSNICMP(filename, L"buf:", 4))
	{
		skinXML.loadFile(filename, includepath);
	}
	else
	{
		//CUT DebugString("filename is %s\n", filename);
		includepath = filename;
		includepath.RemovePath();

		skinXML.loadFile(filename /*file*/, includepath);
	}

	Wasabi::Std::setCurDir(olddir);

	popParserState();
}


#ifdef WASABI_COMPILE_WNDMGR 
// do not forget to popParserState(); before returning
Container *SkinParser::loadContainerForWindowHolder(const wchar_t *groupid, GUID g, int initit, int transcient, const wchar_t *containerid, int container_flag)
{
	ASSERTPR((g == INVALID_GUID || groupid == NULL) && (g != INVALID_GUID || groupid != NULL), "sorry, one or the other, indulge aristotle");
	pushParserState();
	allowscripts = 1;
	instantiatinggroup = 0;
	transcientcontainer = transcient;
	staticloading = 0;
	recording_container = 0;
	recording_groupdef = 0;
	curContainer = NULL;
	lastCreatedContainer = NULL;
	curGroup = NULL;
	inContainer = inLayout = inElements = inGroup = inGroupDef = inAccelerators = inStringTable = 0;
	scriptId = -1; //WASABI_API_PALETTE->getSkinPartIterator();
	SkinItem *found = NULL;
	SkinItem *generic = NULL;

	for (int i = guiTree->getNumObject(XML_TAG_CONTAINER) - 1;i >= 0 && found == NULL;i--)
	{
		SkinItem *item = guiTree->getObject(XML_TAG_CONTAINER, i);
		if (item == NULL) continue;
		ifc_xmlreaderparams *par = item->getParams();
		if (!par) continue;

		if (g != INVALID_GUID)
		{
			for (size_t j = 0;found == NULL && j != par->getNbItems();j++)
			{
				const wchar_t *p = par->getItemName(j);
				if (!WCSICMP(p, L"component") || !WCSICMP(p, L"hold"))
				{
					ParamParser pp(par->getItemValue(j));
					if (pp.hasGuid(g) && found == NULL)
					{
						found = item;
						break;
					}
					if (generic == NULL && (pp.hasGuid(GENERIC_GUID) || pp.hasString(L"@ALL@")))
					{
						generic = item;
					}
				}
			}
		}
		else if (groupid != NULL)
		{
			for (size_t j = 0;j != par->getNbItems() && found == NULL;j++)
			{
				const wchar_t *p = par->getItemName(j);
				if (!WCSICMP(p, L"hold"))
				{
					ParamParser pp(par->getItemValue(j));
					if (pp.hasString(groupid))
					{
						found = item;
						break;
					}
					if (pp.hasString(L"@ALL@"))
					{
						generic = item;
					}
				}
			}
		}
	}

	if (found == NULL && generic == NULL)
	{
		popParserState();
		return NULL;
	}

	if (!found)
	{
		if (containerid != NULL)
		{
			SkinItem *item = guiTree->getContainerById(containerid);
			if (item != NULL)
			{
				Container *c = instantiateDynamicContainer(item, initit);
				popParserState();
				return c;
			}
		}
		else
		{
			if (container_flag != 0) return NULL;
		}
	}

	Container *c = instantiateDynamicContainer(found != NULL ? found : generic, initit);
	popParserState();
	return c;
}

Container *SkinParser::instantiateDynamicContainer(SkinItem *containeritem, int initit)
{

	int quit = 0;
	int guitreeid = guiTree->getObjectIdx(containeritem);
	for (int i = guitreeid;i < guiTree->getNumObject() && !quit;i++)
	{
		SkinItem *ii = guiTree->getList()->enumItem(i);
		ifc_xmlreaderparams *params = ii->getParams();
		const wchar_t *path = ii->getXmlRootPath();
		if (path)
			includepath = path;
		int object_type = guiTree->getObjectType(ii);
		const wchar_t *name = ii->getName();
		if (!params)
		{
			if (object_type == XML_TAG_CONTAINER)
				quit = 1;
			_onXmlEndElement(object_type, name);
		}
		else
		{
			_onXmlStartElement(object_type, name, params);
		}
	}
	return lastCreatedContainer;
}

// do not forget to popParserState(); before returning
Container *SkinParser::newDynamicContainer(const wchar_t *containerid, int transcient)
{

	pushParserState();

	allowscripts = 1;
	instantiatinggroup = 0;
	transcientcontainer = transcient;
	staticloading = 0;
	recording_container = 0;
	recording_groupdef = 0;
	curContainer = NULL;
	lastCreatedContainer = NULL;
	curGroup = NULL;
	inContainer = inLayout = inElements = inGroup = inGroupDef = inAccelerators = inStringTable = 0;
	scriptId = WASABI_API_PALETTE->getSkinPartIterator();
	SkinItem *found = NULL;

	for (int i = guiTree->getNumObject(XML_TAG_CONTAINER) - 1;i >= 0 && found == NULL;i--)
	{
		SkinItem *item = guiTree->getObject(XML_TAG_CONTAINER, i);
		ifc_xmlreaderparams *par = item->getParams();
		if (!par) continue;
		const wchar_t *p = par->getItemValue(L"id");
		if (!WCSICMP(p, containerid))
		{
			found = item;
			break;
		}
	}

	Container *c = NULL;
	if (found != NULL)
		c = instantiateDynamicContainer(found);

	popParserState();

	return c;
}

#endif

// do not forget to popParserState(); before returning
void SkinParser::fillGroup(Group *group, const wchar_t *groupid, SkinItem *specific_item, int params_only, int no_params, int scripts_enabled)
{
	ASSERT(group != NULL);
	pushParserState();

	instantiatinggroup = 1;

#ifdef WASABI_COMPILE_WNDMGR
	transcientcontainer = 0;
#endif

	allowscripts = scripts_enabled;
	staticloading = 0;
	recording_container = 0;
	recording_groupdef = 0;
	lastCreatedGroup = NULL;
	scriptId = group->getSkinPartId();
	SkinItem *found = NULL;

	PtrList<ifc_xmlreaderparams> ancestor_param_list;

	found = specific_item == NULL ? guiTree->getGroupDef(groupid) : specific_item;

	if (found == NULL)
	{
		popParserState();
		return ;
	}

	curGroup = group;
	inGroup = 1;
	parseGroup(found, &ancestor_param_list, params_only);

	if (!no_params)
	{
		XmlObject *xo = static_cast<XmlObject *>(curGroup->getScriptObject()->vcpu_getInterface(xmlObjectGuid));
		for (int i = ancestor_param_list.getNumItems() - 1;i >= 0;i--)
			initXmlObject(xo, ancestor_param_list.enumItem(i), 1);
	}

	popParserState();
}

GuiObject *SkinParser::newDynamicGroup(const wchar_t *groupid, int grouptype, SkinItem *specific_item, int specific_scriptid, int scripts_enabled)
{
#ifdef WASABI_COMPILE_CONFIG
	int iscfggroup = (grouptype == GROUP_CFGGROUP);
#endif

#ifdef WASABI_COMPILE_WNDMGR
	int islayoutgroup = (grouptype == GROUP_LAYOUTGROUP);
#endif

	Group *r = NULL;
#ifdef WASABI_COMPILE_CONFIG
	if (!iscfggroup)
	{
#endif
#ifdef WASABI_COMPILE_WNDMGR
		if (!islayoutgroup)
			r = new Group;
		else
		{
			Layout *l = new Layout;
			r = l;
			l->setParentContainer(NULL);
		}
#else // wndmgr
		r = new Group;
#endif // wndmgr
#ifdef WASABI_COMPILE_CONFIG

	}
	else
		r = new CfgGroup;
#endif

	r->setSkinPartId(specific_scriptid > -1 ? specific_scriptid : WASABI_API_PALETTE->getSkinPartIterator());

	if (r != NULL)
	{
		r->setXmlParam(L"id", groupid);
		r->setGroupContent(groupid, specific_item, scripts_enabled);
		fillGroup(r, groupid, specific_item, 1, 0, scripts_enabled);
		return r->getGuiObject();
	}
	return NULL;
}

void SkinParser::pushParserState()
{
	parser_status *p = new parser_status;
#ifdef WASABI_COMPILE_WNDMGR
	p->curContainer = curContainer;
	p->curLayout = curLayout;
	p->inContainer = inContainer;
	p->inLayout = inLayout;
	p->transcientcontainer = transcientcontainer;
#endif
	p->staticloading = staticloading;
	p->curGroup = curGroup;
	p->includepath = includepath;
	p->inElements = inElements;
	p->inGroup = inGroup;
	p->inGroupDef = inGroupDef;
	p->instantiatinggroup = instantiatinggroup;
	p->scriptid = scriptId;
	p->allowscripts = allowscripts;
	p->inAccelerators = inAccelerators;
	p->inStringTable = inStringTable;
	statusstack.addItem(p);
}

void SkinParser::popParserState()
{
	ASSERT(statusstack.getNumItems() > 0);
	parser_status *p = statusstack.enumItem(statusstack.getNumItems() - 1);
	statusstack.removeByPos(statusstack.getNumItems() - 1);
	ASSERT(p != NULL);
#ifdef WASABI_COMPILE_WNDMGR
	curContainer = p->curContainer;
	curLayout = p->curLayout;
	inContainer = p->inContainer;
	inLayout = p->inLayout;
	transcientcontainer = p->transcientcontainer;
#endif
	curGroup = p->curGroup;
	includepath = p->includepath;
	inElements = p->inElements;
	inAccelerators = p->inAccelerators;
	inStringTable = p->inStringTable;
	inGroup = p->inGroup;
	inGroupDef = p->inGroupDef;
	staticloading = p->staticloading;
	instantiatinggroup = p->instantiatinggroup;
	scriptId = p->scriptid;
	allowscripts = p->allowscripts;
	delete p;
}

#ifdef WASABI_COMPILE_WNDMGR

Container *SkinParser::getContainer(const wchar_t *id)
{
	for (int i = 0;i < containers.getNumItems();i++)
		if (!WCSICMP(id, containers.enumItem(i)->getId()))
			return containers.enumItem(i);
	return NULL;
}

Layout *SkinParser::getLayout(const wchar_t *contlay)
{
	PathParserW pp(contlay, L",");
	if (pp.getNumStrings() == 2)
	{
		Container *c = SkinParser::getContainer(pp.enumString(0));
		if (c)
		{
			Layout *l = c->getLayout(pp.enumString(1));
			if (l)
			{
				return l;
			}
		}
	}
	return NULL;
}

int SkinParser::script_getNumContainers()
{
	return containers.getNumItems();
}

Container *SkinParser::script_enumContainer(int n)
{
	return containers.enumItem(n);
}

int SkinParser::isContainer(Container *c)
{
	return containers.haveItem(c);
}

Container *SkinParser::script_getContainer(const wchar_t *id)
{
	for (int i = 0;i < containers.getNumItems();i++)
	{
		Container *c = containers.enumItem(i);
		if (c)
		{
			const wchar_t *c_id = containers.enumItem(i)->getId();
			if (c_id && !WCSICMP(id, c_id))
				return containers.enumItem(i);
		}
	}
	return NULL;
}

void SkinParser::componentToggled(GUID *g, int visible)
{
	for (int i = 0;i < containers.getNumItems();i++)
		containers[i]->componentToggled(g, visible);
}

void SkinParser::sendNotifyToAllContainers(int notifymsg, int param1, int param2)
{
	for (int i = 0;i < containers.getNumItems();i++)
		containers[i]->sendNotifyToAllLayouts(notifymsg, param1, param2);
}


void SkinParser::toggleContainer(int num)
{
	if (num < 0) return ;
	if (num > containers.getNumItems()) return ;

	Container *c = containers[num];
	if (!c) return ;
	c->toggle();
}

void SkinParser::startupContainers(int scriptid)
{
	for (int i = 0;i < containers.getNumItems();i++)
	{
		if (scriptid == -1 || containers[i]->getScriptId() == scriptid && !containers[i]->isDynamic())
			containers[i]->onInit();
	}
}

void SkinParser::showContainer(int num, int show)
{
	if (num < 0) return ;
	if (num > containers.getNumItems()) return ;

	Container *c = containers[num];
	c->setVisible(show);
}

#endif // wndmgr

int SkinParser::getHex(const wchar_t *p, int size)
{
	int v = 0, i = 0;
	while (*p != 0 && *p != '-' && *p != '}')
	{
		unsigned int a = *p;
		if (a >= '0' && a <= '9') a -= '0';
		if (a >= 'a' && a <= 'f') a -= 'a' -10;
		if (a >= 'A' && a <= 'F') a -= 'A' -10;
		v = (v * 16) + a;
		p++;
		i++; if (size != -1 && i == size) return v;
	}
	return v;
}

#ifdef WASABI_COMPILE_WNDMGR
int SkinParser::getComponentGuid(GUID *g, const wchar_t *p)
{
	g->Data1 = getHex(p);
	while (*p != 0 && *p != '-') p++;
	if (*p == '-')
	{
		p++;
		g->Data2 = getHex(p);
		while (*p != 0 && *p != '-') p++;
		if (*p == '-')
		{
			p++;
			g->Data3 = getHex(p);
			while (*p != 0 && *p != '-') p++;
			if (*p == '-')
			{
				p++;
				g->Data4[0] = getHex(p, 2); p += 2;
				g->Data4[1] = getHex(p, 2); p += 3;
				g->Data4[2] = getHex(p, 2); p += 2;
				g->Data4[3] = getHex(p, 2); p += 2;
				g->Data4[4] = getHex(p, 2); p += 2;
				g->Data4[5] = getHex(p, 2); p += 2;
				g->Data4[6] = getHex(p, 2); p += 2;
				g->Data4[7] = getHex(p, 2);
				return 1;
			}
		}
	}
	return 0;
}

GUID *SkinParser::getComponentGuid(const wchar_t *id)
{
	static GUID g;
	g = nsGUID::fromCharW(id);
	if (g == INVALID_GUID) return NULL;
	return &g;
}

#endif

int SkinParser::parse(const wchar_t *str, const wchar_t *what)
{
	int a = parsetypes.getId(what);
	if (a < 0) return WTOI(str);
	switch (a)
	{
	case PARSETYPE_RESIZE: return parseResize(str);
	case PARSETYPE_COLOR: return parseColor(str);
	case PARSETYPE_COLORALPHA: return parseColorAlpha(str);
	case PARSETYPE_REGIONOP: return parseRegionOp(str);
	case PARSETYPE_INTERNALACTION: return getAction(str);
	case PARSETYPE_GROUPINHERITANCE: return parseGroupInheritance(str);
	}
	// todo: add svc_intParser
	return 0;
}

int SkinParser::parseGroupInheritance(const wchar_t *str)
{
	if (WCSCASEEQLSAFE(str, L"1")) return GROUP_INHERIT_ALL;
	if (WCSCASEEQLSAFE(str, L"0")) return GROUP_INHERIT_NOTHING;
	ParamParser pp(str);
	int v = 0;
	for (int i = 0;i < pp.getNumItems();i++)
	{
		const wchar_t *s = pp.enumItem(i);
		if (WCSCASEEQLSAFE(s, L"xui")) v |= GROUP_INHERIT_XUIOBJECTS;
		if (WCSCASEEQLSAFE(s, L"scripts")) v |= GROUP_INHERIT_SCRIPTS;
		if (WCSCASEEQLSAFE(s, L"params")) v |= GROUP_INHERIT_PARAMS;
	}
	return v;
}

ARGB32 SkinParser::parseColor(const wchar_t *color, int *error)
{
	if (color == NULL || *color == '\0') { if (error) *error = 1; return COLOR_ERROR; }
	if (!WCSICMP(color, L"white")) return COLOR_WHITE;
	if (!WCSICMP(color, L"black")) return COLOR_BLACK;
	if (wcschr(color, ','))
	{
		int r = 0, g = 0, b = 0;
		if (swscanf(color, L"%d,%d,%d", &r, &g, &b) != 3) return COLOR_ERROR;
		return RGB(r, g, b); // our colors are reversed internally
	}
	if (*color == '#')
	{
		int r = 0, g = 0, b = 0;
		if (swscanf(color, L"#%02x%02x%02x", &r, &g, &b) != 3) return COLOR_ERROR;
		return RGB(r, g, b);
	}
	if (error) *error = 1;
	return COLOR_ERROR;
}

ARGB32 SkinParser::parseColorAlpha(const wchar_t *color)
{
	if (color == NULL || *color == '\0') return COLOR_BLACKA;
	if (!WCSICMP(color, L"white")) return COLOR_WHITEA;
	if (!WCSICMP(color, L"black")) return COLOR_BLACKA;
	if (wcschr(color, ','))
	{
		int r = 0, g = 0, b = 0, a = 255;
		// note that 3 params is ok
		if (swscanf(color, L"%d,%d,%d,%d", &r, &g, &b, &a) < 3) return COLOR_ERRORA;
		ARGB32 ret = RGB(r, g, b); // our colors are reversed internally
		ret |= ((a & 0xff) << 24);
		return ret;
	}
	if (*color == '#')
	{
		int r = 0, g = 0, b = 0, a = 255;
		if (swscanf(color, L"#%02x%02x%02x%02x", &r, &g, &b, &a) < 3) return COLOR_ERRORA;
		ARGB32 ret = RGB(r, g, b); // our colors are reversed internally
		ret |= ((a & 0xff) << 24);
		return ret;
	}
	return COLOR_ERRORA;
}

#ifdef WASABI_COMPILE_WNDMGR

void SkinParser::toggleContainer(const wchar_t *id)
{
	// component toggling
	GUID *g;
	if (g = getComponentGuid(id))
	{
		GUID g2;
		MEMCPY(&g2, g, sizeof(GUID));
		WASABI_API_WNDMGR->skinwnd_toggleByGuid(g2);
		return ;
	}
	for (int i = 0;i < containers.getNumItems();i++)
		if (!WCSICMP(id, containers[i]->getId())) toggleContainer(i);
}

void SkinParser::showContainer(const wchar_t *id, int show)
{
	// component guid
	/*  GUID *g;
	  if (g = getComponentGuid(id)) {
	    WASABI_API_WNDMGR->setComponentVisible(*g, show);
	    return;
	  }*/

	foreach(containers)
	if (!WCSICMP(id, containers.getfor()->getId()))
		showContainer(foreach_index, show);
	endfor
}

#endif // wndmgr

#pragma warning( disable : 4065 )
int SkinParser::getAction(const wchar_t *action, const wchar_t **name)
{
	//CT> this should be a binary search for more efficiency
	if (name != NULL) *name = NULL;
	int a = actionlist.getId(action);
	if (a == -1) return ACTION_NONE;

	// these strings are for accessibility when no text has been assigned to the control that triggers these actions
	if (name != NULL)
	{
		switch (a)
		{
#ifdef WASABI_COMPILE_WNDMGR
		case ACTION_RELOAD_SKIN: *name = _(L"Reload skin"); break;
		case ACTION_MINIMIZE: *name = _(L"Minimize window"); break;
		case ACTION_MAXIMIZE: *name = _(L"Maximize window"); break;
		case ACTION_CLOSE: *name = _(L"Close"); break;
		case ACTION_SWITCH: *name = _(L"Switch to"); break;
		case ACTION_TOGGLE: *name = _(L"Toggle"); break;
		case ACTION_CLOSE_WINDOW: *name = _(L"Close window"); break;
#endif

#ifdef WA3COMPATIBILITY
		case ACTION_ABOUT: *name = _(L"About"); break;
		case ACTION_SYSMENU: *name = _(L"Open system menu"); break;
		case ACTION_CONTROLMENU: *name = _(L"Open control menu"); break;
		case ACTION_MENU: *name = _(L"Open menu"); break;
		case ACTION_WINDOWMENU: *name = _(L"Window menu"); break;
		case ACTION_MB_FORWARD: *name = _(L"Forward"); break;
		case ACTION_MB_BACK: *name = _(L"Back"); break;
		case ACTION_MB_URL: *name = _(L"Url"); break;
		case ACTION_MB_HOME: *name = _(L"Home"); break;
		case ACTION_MB_STOP: *name = _(L"Stop loading"); break;
		case ACTION_MB_REFRESH: *name = _(L"Refresh"); break;
		case ACTION_TEXT_LARGER: *name = _(L"Increase text size"); break;
		case ACTION_TEXT_SMALLER: *name = _(L"Decrease text size"); break;
		case ACTION_PREFERENCES: *name = _(L"Preferences"); break;
		case ACTION_TOGGLE_ALWAYS_ON_TOP: *name = _(L"Toggle Always on top"); break;
		case ACTION_VIEW_FILE_INFO: *name = _(L"View file info"); break;
		case ACTION_ADD_BOOKMARK: *name = _(L"Add bookmark"); break;
		case ACTION_DOUBLESIZE: *name = _(L"Toggle double size mode"); break;
#endif
#ifdef WASABI_WIDGETS_COMPBUCK
		case ACTION_CB_NEXT: *name = _(L"More"); break;
		case ACTION_CB_PREV: *name = _(L"More"); break;
#endif
		default: break;
		}
	}
	return a;
}
#pragma warning( default : 4065 )

#ifdef WASABI_COMPILE_MEDIACORE
int SkinParser::getDisplay(const wchar_t *display)
{
	int a = displaylist.getId(display);
	if (a == -1) return DISPLAY_SERVICE;
	return a;
}
#endif

int SkinParser::getAlign(const wchar_t *align)
{
#ifdef _WIN32
	if (!WCSICMP(align, L"LEFT")) return ALIGN_LEFT;
	if (!WCSICMP(align, L"CENTER")) return ALIGN_CENTER;
	if (!WCSICMP(align, L"RIGHT")) return ALIGN_RIGHT;
	if (!WCSICMP(align, L"TOP")) return ALIGN_TOP;
	if (!WCSICMP(align, L"BOTTOM")) return ALIGN_BOTTOM;
#else
#warning port me
#endif
	return DISPLAY_NONE;
}

int SkinParser::getOrientation(const wchar_t *orient)
{
	if (!WCSICMP(orient, L"V") || !WCSICMP(orient, L"VERTICAL"))
		return ORIENTATION_VERTICAL;
	return ORIENTATION_HORIZONTAL;
}

// link guiobject to guiobject
void SkinParser::initGuiObject(GuiObject *g, Group *pgroup)
{
	ASSERT(pgroup);
	pgroup->addObject(g);
}

// This sends the params to the script object through its XmlObject
// interface. Try not to add code here, but instead in setXmlParam/XmlInit
// in the object itself
void SkinParser::initXmlObject(XmlObject *o, ifc_xmlreaderparams *params, int no_id)
{
	ASSERT(o);
	if (params)
		for (size_t i = 0;i != params->getNbItems();i++)
			if (!no_id || WCSICMP(params->getItemName(i), L"id")) // don't pass along id="blah" stuff
				o->setXmlParam(params->getItemName(i), params->getItemValue(i));
	//  o->XmlInit(); //fg> now defered
}

#ifdef WASABI_COMPILE_WNDMGR

// This sends the params to the script object through its XmlObject
// interface. Try not to add code here, but instead in setXmlParam/XmlInit
// in the object itself
void SkinParser::initLayout(Layout *l, Container *pcont)
{
	ASSERT(pcont);
	l->setParentContainer(pcont);
	pcont->addLayout(l);
}

#endif

void SkinParser::xmlReaderCallback(int start, const wchar_t *xmltag, skin_xmlreaderparams *params)
{
	if (start) onXmlStartElement(xmltag, params);
	else onXmlEndElement(xmltag);
}

void SkinParser::onXmlStartElement(const wchar_t *name, skin_xmlreaderparams *params)
{
	xml_tag *i = quickxmltaglist.findItem(name);
	if (i) _onXmlStartElement(i->id, name, params);
	else _onXmlStartElement(XML_TAG_UNKNOWN, name, params);
}

void SkinParser::onXmlEndElement(const wchar_t *name)
{
	xml_tag *i = quickxmltaglist.findItem(name);
	if (i)
	{
		if (i->needclosetag)
			_onXmlEndElement(i->id, name);
	} /*else
		    _onXmlEndElement(XML_TAG_UNKNOWN, name);*/ // not needed yet
}



void SkinParser::parseGroup(SkinItem *groupitem, PtrList<ifc_xmlreaderparams> *ancestor_param_list, int params_only, int overriding_inheritance_flags)
{
	ifc_xmlreaderparams *par = groupitem->getParams();
	const wchar_t *groupid = par->getItemValue(L"id");
	const wchar_t *ic = par->getItemValue(L"inherit_content");
	const wchar_t *og = par->getItemValue(L"inherit_group");
	const wchar_t *ip = par->getItemValue(L"inherit_params");

	int inheritance_flags = parseGroupInheritance(ic);
	int inherit_params = 1;
	if (ip != NULL && *ip != 0) inherit_params = WTOI(ip);
	if ((og && *og) && (!ic || !*ic)) inheritance_flags = GROUP_INHERIT_ALLCONTENT;

	if (inherit_params) inheritance_flags |= GROUP_INHERIT_PARAMS;

	if (WCSCASEEQLSAFE(og, groupid)) og = NULL;


	if (inheritance_flags != GROUP_INHERIT_NOTHING)
	{
		SkinItem *prior_item = NULL;
		if (og != NULL && *og)
			prior_item = guiTree->getGroupDef(og);
		else
			prior_item = guiTree->getGroupDefAncestor(groupitem);
		if (prior_item != NULL)
			parseGroup(prior_item, ancestor_param_list, params_only, inheritance_flags);
	}

	if (overriding_inheritance_flags & GROUP_INHERIT_PARAMS)
		ancestor_param_list->addItem(groupitem->getParams());

	if (!params_only)
	{
		int guitreeid = guiTree->getObjectIdx(groupitem);
		for (int i = guitreeid + 1;i < guiTree->getNumObject();i++)
		{
			SkinItem *item = guiTree->getList()->enumItem(i);
			ifc_xmlreaderparams *params = item->getParams();;
			const wchar_t *path = item->getXmlRootPath();
			if (path)
				includepath = path;
			int object_type = guiTree->getObjectType(item);
			if (object_type == XML_TAG_GROUPDEF && !params) break;
			if (object_type == XML_TAG_SCRIPT && !(overriding_inheritance_flags & GROUP_INHERIT_SCRIPTS)) continue;
			if (object_type != XML_TAG_SCRIPT && !(overriding_inheritance_flags & GROUP_INHERIT_XUIOBJECTS)) continue;
			const wchar_t *name = item->getName();
			if (!params)
				_onXmlEndElement(object_type, name);
			else
				_onXmlStartElement(object_type, name, params);
		}
	}

}

void SkinParser::_onXmlStartElement(int object_type, const wchar_t *object_name, ifc_xmlreaderparams *params)
{
	GuiObject *g = NULL;                                // We'll need to build a GUI object
	XmlObject *x = NULL;
	Group *g_group = NULL;

	if (object_type == XML_TAG_UNKNOWN)
	{
		if (loading_main_skinfile && (!WCSICMP(object_name, L"WinampAbstractionLayer") || !WCSICMP(object_name, L"WasabiXML")))
		{
			skinversion = WTOF(params->getItemValue(L"version"));
		}
	}

/*#ifdef WASABI_COMPILE_WNDMGR
	int isacontainer = 0;
#endif // wndmgr*/

	if (object_type == XML_TAG_GROUPDEF)
	{
		if (staticloading)
		{
			recording_groupdef++;
		}
		inGroupDef++;
	}

	if (object_type == XML_TAG_ACCELERATORS)
	{
		const wchar_t *section = params->getItemValue(L"section");
		if (!section)
			LocalesManager::setAcceleratorSection(L"general");
		else
			LocalesManager::setAcceleratorSection(section);

		inAccelerators = 1;
	}

	if (object_type == XML_TAG_STRINGTABLE)
	{
		const wchar_t *section = params->getItemValue(L"id");
		if (!section)
			LocalesManager::SetStringTable(L"nullsoft.wasabi");
		else
			LocalesManager::SetStringTable(section);

		inStringTable = 1;
	}

	if (inStringTable && object_type == XML_TAG_STRINGENTRY)
	{
		const wchar_t *b = params->getItemValue(L"id");
		const wchar_t *a = params->getItemValue(L"string");
		if (b && a)
		{
			LocalesManager::AddString(WTOI(b), a);
		}
	}

	if (inAccelerators && object_type == XML_TAG_ACCELERATOR)
	{
		const wchar_t *b = params->getItemValue(L"bind");
		const wchar_t *a = params->getItemValue(L"action");
		if (b && a)
		{
			//LocalesManager::addAccelerator(b, a);
			//Martin> this is a temporary fix to protect skin.xml from overriding the language pack accels
			LocalesManager::addAcceleratorFromSkin(b, a);
		}
	}

	if ((!recording_container && !recording_groupdef) && !inGroupDef)
	{
		if (object_type == XML_TAG_SCRIPT)
		{
			//      const char *id = params->getItemValue(L"id");
			if (1)
			{
				if (allowscripts)
				{
					int vcpuid = Script::addScript(includepath, params->getItemValue(L"file"), params->getItemValue(L"id"));
					if (vcpuid != -1)
					{
						Script::setScriptParam(vcpuid, params->getItemValue(L"param"));
						Script::setParentGroup(vcpuid, curGroup);
						Script::setSkinPartId(vcpuid, scriptId);
						if (curGroup != NULL)
							curGroup->addScript(vcpuid);
						else // todo: schedule this for the end of skinparse, after all layouts are inited
							SOM::getSystemObjectByScriptId(vcpuid)->onLoad();
					}
				}
			}
		}
	}

#ifdef WASABI_COMPILE_WNDMGR
	if ((!recording_groupdef && !recording_container) && (inContainer || inGroup) && !inGroupDef)
	{     // Am I in a container definition ?
		if (inLayout || inGroup)
		{ // Am I in a layout or in a group ?
#else // wndmgr
	if ((!recording_groupdef && !recording_container) && inGroup && !inGroupDef)
	{
		{    // Am I in definition ?
#endif // wndmgr


			// Create appropriate GuiObject descendant
			if (object_type == XML_TAG_GROUP || object_type == XML_TAG_CFGGROUP)
			{
				Group *old = curGroup;
				GuiObject *newgrp = newDynamicGroup(params->getItemValue(L"id"), (object_type == XML_TAG_CFGGROUP) ? GROUP_CFGGROUP : GROUP_GROUP);
				if (newgrp)
				{
					x = static_cast<XmlObject *>(newgrp->guiobject_getScriptObject()->vcpu_getInterface(xmlObjectGuid));
					g = newgrp;
				}
				curGroup = old;
			}
#ifdef WASABI_COMPILE_WNDMGR
			else if (object_type == XML_TAG_SNAPPOINT)
			{
				x = new SnapPoint(curLayout, curContainer);
			}
#endif
			else if (object_type != XML_TAG_UNKNOWN)
			{
				g = NULL;
			}
			else
			{
				SkinItem *item = guiTree->getXuiGroupDef(object_name);
				if (item != NULL)
				{
					Group *old = curGroup;
					const wchar_t *grpid = NULL;
					if (item->getParams() != NULL)
						grpid = item->getParams()->getItemValue(L"id");
					GuiObject *newgrp = NULL;
					if (grpid == NULL)
						newgrp = newDynamicGroup(params->getItemValue(L"id"), GROUP_GROUP, item);
					else
						newgrp = newDynamicGroup(grpid, GROUP_GROUP, NULL, -1, allowscripts);
					if (newgrp)
					{
						x = static_cast<XmlObject *>(newgrp->guiobject_getScriptObject()->vcpu_getInterface(xmlObjectGuid));
						g = newgrp;
					}
					curGroup = old;
				}
				else
				{
					g = createExternalGuiObject(object_name, &x, params);
				}
			}
		}
#ifdef WASABI_COMPILE_WNDMGR
		else
		{ // if inlayout

			if (object_type == XML_TAG_LAYOUT)
			{                   // Now enters a new layout
				curLayout = new Layout;
				curGroup = curLayout;
				inLayout = 1;
				initLayout(curLayout, curContainer);
				x = static_cast<XmlObject *>(curLayout->getGuiObject()->guiobject_getScriptObject()->vcpu_getInterface(xmlObjectGuid));
			}
		}
#endif // wndmgr

	}
#ifdef WASABI_COMPILE_WNDMGR
	else
	{ // if inContainer

		if (inElements)
		{
			// do nothing
		}
		else if (object_type == XML_TAG_CONTAINER)
		{
			//isacontainer = 1;
			const wchar_t *d = params->getItemValue(L"dynamic");
			int dyn = WASABI_WNDMGR_ALLCONTAINERSDYNAMIC ? 1 : (d ? WTOI(d) : 0);

			if (dyn && staticloading)
			{
				recording_container = 1;
			}
			else
			{
				inContainer = 1;
				curContainer = new Container(scriptId);
				curContainer->setId(params->getItemValue(L"id"));
				containers.addItem(curContainer);
#ifdef _DEBUG
				DebugStringW(L"new Container - skinpartid = %d\n", scriptId);
#endif
				if (transcientcontainer) curContainer->setTranscient(1);
				x = curContainer;
			}
		}
		else
		{
			if (object_type == XML_TAG_SCRIPTS)
				inScripts = 1;
			else if (object_type == XML_TAG_ELEMENTS)
				inElements = 1;
		}

	} // if container else
#endif // wndmgr

	if (g_group)
	{
		curGroup = g_group;
	}
	else if (g)
		initGuiObject(g, curGroup);

	if (x)
		initXmlObject(x, params);

	if (recording_container || recording_groupdef)
		guiTree->addItem(object_type, object_name, params, scriptId, includepath.v());
}

void SkinParser::_onXmlEndElement(int object_type, const wchar_t *name)
{
	if (recording_container || recording_groupdef)
		guiTree->addItem(object_type, name, NULL, scriptId, includepath);

	if (object_type == XML_TAG_GROUPDEF)
	{
		lastCreatedGroup = curGroup;
		if (staticloading)
			recording_groupdef--;
		if (recording_groupdef < 0) recording_groupdef = 0;
		inGroup = 0;
		inGroupDef--;
		curGroup = NULL;
	}

#ifdef WASABI_COMPILE_WNDMGR
	if (object_type == XML_TAG_CONTAINER)
	{
		if (inContainer)
		{
			//if (!curContainer->isDynamic()) containers.addItem(curContainer); //FG>script
			//containers.addItem(curContainer);
			lastCreatedContainer = curContainer;
			curContainer = NULL;
			inContainer = 0;
		}
		recording_container = 0;
		if (recording_groupdef)
		{
			WASABI_API_WNDMGR->messageBox(L"container closed but group still open, closing group", L"error in xml data", 0, NULL, NULL);
			recording_groupdef = 0;
		}
	}

	if (inLayout && object_type == XML_TAG_LAYOUT)
	{
#ifdef WA3COMPATIBILITY
		curLayout->setForwardMsgWnd(WASABI_API_WND->main_getRootWnd()->gethWnd());
#endif
		curLayout->setAutoResizeAfterInit(1);
#ifndef WA3COMPATIBILITY
#ifdef _WIN32
		curLayout->init(hInstance, curLayout->getCustomOwner() ? curLayout->getCustomOwner()->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd(), TRUE);
#else
#warning port me
		curLayout->init(0, curLayout->getCustomOwner() ? curLayout->getCustomOwner()->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd(), TRUE);
#endif
#else
		curLayout->init(hInstance, curLayout->getCustomOwner() ? curLayout->getCustomOwner()->gethWnd() : Main::gethWnd(), TRUE);
#endif
		curLayout->getGuiObject()->guiobject_onStartup();
		curLayout = NULL;
		inLayout = 0;
		curGroup = NULL;
	}
#endif

	if (inScripts && object_type == XML_TAG_SCRIPTS)
	{
		inScripts = 0;
	}

	if (inElements && object_type == XML_TAG_ELEMENTS)
	{
		inElements = 0;
	}

	if (inAccelerators && object_type == XML_TAG_ACCELERATORS)
	{
		LocalesManager::setAcceleratorSection(L"");
		inAccelerators = 0;
	}

	if (inStringTable && object_type == XML_TAG_STRINGTABLE)
	{
		LocalesManager::SetStringTable(L"");
		inStringTable = 0;
	}
}

#ifdef WASABI_COMPILE_WNDMGR
int SkinParser::verifyContainer(Container *c)
{
	for (int i = 0;i < containers.getNumItems();i++)
	{
		if (containers.enumItem(i) == c)
			return 1;
	}
	return 0;
}
#endif

int SkinParser::parseResize(const wchar_t *r)
{
	int a = resizevalues.getId(r);
	if (a < 0) return WTOI(r);
	return a;
}

int SkinParser::parseRegionOp(const wchar_t *r)
{
	if (!WCSICMP(r, L"or")) return REGIONOP_OR;
	if (!WCSICMP(r, L"and")) return REGIONOP_AND;
	if (!WCSICMP(r, L"sub")) return REGIONOP_SUB;
	if (!WCSICMP(r, L"sub2")) return REGIONOP_SUB2;
	return WTOI(r);
}

void SkinParser::cleanupScript(int scriptid)
{
	if (scriptid == -1) scriptid = WASABI_API_PALETTE->getSkinPartIterator();
	int i;
	for (i = 0;i < SOM::getNumSystemObjects();i++)
	{
		if (SOM::getSystemObject(i)->getSkinPartId() == scriptid)
		{
			int vcpu = SOM::getSystemObject(i)->getScriptId();
			Script::unloadScript(vcpu);
			i--;
		}
	}
#ifdef WASABI_COMPILE_WNDMGR
	for (i = 0;i < containers.getNumItems();i++)
	{
		Container *c = containers[i];
		if (c->getScriptId() == scriptid)
		{
			c->setDeleting();
			delete c; // c autodeletes from containers
			i--;
		}
	}
#endif
	guiTree->removeSkinPart(scriptid);
#ifdef WASABI_COMPILE_WNDMGR
	AutoPopup::removeSkinPart(scriptid);
#endif
}

#ifdef WASABI_COMPILE_WNDMGR
void SkinParser::unloadAllContainers()
{
	foreach(containers)
	containers.getfor()->setDeleting();
	endfor;
	containers.deleteAllSafe();
	//script_containers.removeAll();
}
#endif

void SkinParser::cleanUp()
{
#ifdef WASABI_COMPILE_WNDMGR
	AutoPopup::removeAllAddons();
#endif
	Script::unloadAllScripts();
#ifdef WASABI_COMPILE_WNDMGR
	unloadAllContainers();
#endif
	guiTree->reset();
}

#ifdef WASABI_COMPILE_WNDMGR
int SkinParser::getNumContainers()
{
	return containers.getNumItems();
}

Container *SkinParser::enumContainer(int n)
{
	return containers.enumItem(n);
}
#endif

const wchar_t *SkinParser::getXmlRootPath()
{
	return includepath;
}

#ifdef WASABI_COMPILE_WNDMGR
const wchar_t *SkinParser::getCurrentContainerId()
{
	if (curContainer) return curContainer->getId();
	return NULL;
}

const wchar_t *SkinParser::getCurrentGroupId()
{
	if (curGroup) return curGroup->getGuiObject()->guiobject_getId();
	return NULL;
}
#endif

GuiObject *SkinParser::createExternalGuiObject(const wchar_t *object_name, XmlObject **x, ifc_xmlreaderparams *params)
{
	svc_xuiObject *svc = NULL;
	waServiceFactory *sf = xuiCache->findServiceFactory(object_name);
	if (sf != NULL)
		svc = castService<svc_xuiObject>(sf, FALSE);
	else
	{
		XuiObjectSvcEnum xose(object_name);
		svc = xose.getNext(FALSE);
		sf = xose.getLastFactory();
	}
	if (svc != NULL)
	{
		GuiObject *go = svc->instantiate(object_name, params);
		if (!go) return NULL;
		go->guiobject_setXuiService(svc);
		go->guiobject_setXuiServiceFactory(sf);
		ScriptObject *so = go->guiobject_getScriptObject();
		ASSERTPR(so != NULL, "tell francis to fix scriptobjectless xuiobjects");
		if (x) *x = static_cast<XmlObject *>(so->vcpu_getInterface(xmlObjectGuid));
		return go;
	}
	return NULL;
}

void SkinParser::destroyGuiObject(GuiObject *o)
{
	svc_xuiObject *svc = o->guiobject_getXuiService();
	if (!svc)
	{
		ScriptObject *so = o->guiobject_getScriptObject();
		ASSERT(so != NULL);
		GuiObjectWnd *go = static_cast<GuiObjectWnd *>(so->vcpu_getInterface(guiObjectWndGuid));
		ASSERT(go != NULL);
		delete go;
	}
	else
	{
		waServiceFactory *sf = o->guiobject_getXuiServiceFactory();
		svc->destroy(o);
		sf->releaseInterface(svc);
	}
}

#ifdef WASABI_COMPILE_WNDMGR
void SkinParser::focusFirst()
{
	foreach(containers)
	for (int j = 0;j < containers.getfor()->getNumLayouts();j++)
	{
		Layout *l = containers.getfor()->enumLayout(j);
		if (l != NULL && l->isVisible())
		{
			l->setFocus();
			return ;
		}
	}
	endfor;
}

#ifdef WA3COMPATIBILITY 
// non portable, the skin might be missing/buggy as hell, we need to use the os' msgbox
void SkinParser::emmergencyReloadDefaultSkin()
{
	if (!Main::revert_on_error)
	{
		if (!STRCASEEQLSAFE("Default", WASABI_API_SKIN->getSkinName()))
		{
			WASABI_API_WND->appdeactivation_setbypass(1);
			Std::messageBox(StringPrintfW(L"Failed to load the skin (%s). Did you remove it ?\nThis could also be due to missing components (ie: wa2skin.wac for winamp 2 skins), please check the skin's documentation.\nReverting to default skin.", WASABI_API_SKIN->getSkinName()), "Error", 0);
			WASABI_API_WND->appdeactivation_setbypass(0);
			Skin::toggleSkin("Default");
		}
		else
		{
			WASABI_API_WND->appdeactivation_setbypass(1);
			Std::messageBox("The default skin did not load any user interface! Ooch! What should I do ? Oh well, good luck...", "Danger Danger Will Robinson!", 0);
			WASABI_API_WND->appdeactivation_setbypass(0);
		}
	}
}
#endif
#endif

GuiObject *SkinParser::xui_new(const wchar_t *classname)
{
	SkinItem *item = guiTree->getXuiGroupDef(classname);
	if (item != NULL)
	{
		Group *old = curGroup;
		const wchar_t *grpid = NULL;
		if (item->getParams() != NULL)
			grpid = item->getParams()->getItemValue(L"id");
		GuiObject *newgrp = NULL;
		if (grpid != NULL)
			newgrp = newDynamicGroup(grpid, GROUP_GROUP);
		curGroup = old;
		if (newgrp != NULL) return newgrp;
	}
	return createExternalGuiObject(classname, NULL, NULL);
}

void SkinParser::xui_delete(GuiObject *o)
{
	destroyGuiObject(o);
}

double SkinParser::getSkinVersion()
{
	return skinversion;
}

void SkinParser::setAllLayoutsRatio(double ra)
{
	foreach(containers)
	Container *c = containers.getfor();
	int n = c->getNumLayouts();
	for (int i = 0;i < n;i++)
	{
		Layout *l = c->enumLayout(i);
		if (l->getNoParent() != 1)
			l->setRenderRatio(ra);
	}
	endfor;
}

void SkinParser::setAllLayoutsTransparency(int v)
{
	foreach(containers)
	Container *c = containers.getfor();
	int n = c->getNumLayouts();
	for (int i = 0;i < n;i++)
	{
		Layout *l = c->enumLayout(i);
		if (l->getNoParent() != 1)
			l->setAlpha(v);
	}
	endfor;
}

Layout *SkinParser::getMainLayout()
{
	foreach(containers)
	Container *c = containers.getfor();
	if (!c->isMainContainer()) continue;
	return c->enumLayout(0);
	endfor;
	return NULL;
}

/*
void SkinParser::setAllLayoutsAutoOpacify(int ao, int force) {
  foreach(containers)
    Container *c = containers.getfor();
    int n = c->getNumLayouts();
    for (int i=0;i<n;i++) {
      Layout *l = c->enumLayout(i);
      if (l->getNoParent() != 1)
        l->setAutoOpacify(ao, force);
    }
  endfor;
}
 
void SkinParser::setAllLayoutsOverrideAlpha(int oa) {
  foreach(containers)
    Container *c = containers.getfor();
    int n = c->getNumLayouts();
    for (int i=0;i<n;i++) {
      Layout *l = c->enumLayout(i);
      if (l->isInited() && l->isTransparencySafe() && l->getTransparencyOverride() == -1) {
        if (l->getNoParent() != 1)
          l->setTransparency(oa);
      }
    }
  endfor;
}
*/

Group *SkinParser::curGroup, *SkinParser::lastCreatedGroup;
int SkinParser::inScripts = 0, SkinParser::inElements = 0, SkinParser::inGroupDef = 0, SkinParser::inGroup = 0, SkinParser::inAccelerators = 0, SkinParser::inStringTable = 0;
int SkinParser::scriptId = 0;
int SkinParser::recording_container = 0;
int SkinParser::recording_groupdef = 0;
int SkinParser::staticloading = 0;
PtrList<parser_status> SkinParser::statusstack;
int SkinParser::instantiatinggroup = 0;
int SkinParser::allowscripts = 0;
skin_xmlreaderparams *SkinParser::groupparams = NULL;
PtrListQuickSorted<xml_tag, XmlTagComp> SkinParser::quickxmltaglist;
double SkinParser::skinversion = 0.0;

#ifdef WASABI_COMPILE_WNDMGR
Container *SkinParser::curContainer, *SkinParser::lastCreatedContainer;
Layout *SkinParser::curLayout;
int SkinParser::inContainer, SkinParser::inLayout;
//PtrList<Container> SkinParser::script_containers;
PtrList<Container> SkinParser::containers;
int SkinParser::centerskin;
int SkinParser::transcientcontainer;
SvcCacheT<svc_xuiObject> *SkinParser::xuiCache;
int SkinParser::loading_main_skinfile = 0;
StringW SkinParser::includepath;
#endif
