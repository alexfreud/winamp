#ifndef __SKINPARSER_H
#define __SKINPARSER_H

#include <bfc/wasabi_std.h>
#include <api/wnd/basewnd.h>
#include <api/service/svccache.h>

#ifdef WASABI_COMPILE_WNDMGR
#include <api/wndmgr/layout.h>
#include <api/wndmgr/container.h>
#else
class Container;
class Layout;
#endif

#ifdef WASABI_COMPILE_FONTS
#include <api/font/skinfont.h>
#endif

#ifdef WASABI_COMPILE_XMLPARSER
#include <api/xml/xmlreader.h>
#else
class skin_xmlreaderparams;
#endif

#ifdef WASABI_COMPILE_SKIN
#include <api/skin/group.h>
#else
class Group;
class SkinItem;
#endif

class XmlObject;

enum {
	PARSETYPE_RESIZE=0,
	PARSETYPE_COLOR,
	PARSETYPE_COLORALPHA,
	PARSETYPE_REGIONOP,
	PARSETYPE_INTERNALACTION,
	PARSETYPE_GROUPINHERITANCE,
} ;

enum {
	XML_TAG_CONTAINER,
	XML_TAG_GROUP,
	XML_TAG_CFGGROUP,
	XML_TAG_GROUPDEF,
	XML_TAG_LAYOUT,
	XML_TAG_ACCELERATORS,
	XML_TAG_ACCELERATOR,
	XML_TAG_ELEMENTS,
	XML_TAG_STRINGTABLE,
	XML_TAG_STRINGENTRY,
	XML_TAG_SCRIPTS,
	XML_TAG_SNAPPOINT,
	XML_TAG_TRUETYPEFONT,
	XML_TAG_BITMAPFONT,
	XML_TAG_SCRIPT,
	XML_TAG_UNKNOWN,
};

#define GROUP_INHERIT_NOTHING     0
#define GROUP_INHERIT_XUIOBJECTS  1
#define GROUP_INHERIT_SCRIPTS     2 
#define GROUP_INHERIT_PARAMS      4 
#define GROUP_INHERIT_ALL        GROUP_INHERIT_XUIOBJECTS | GROUP_INHERIT_SCRIPTS | GROUP_INHERIT_PARAMS
#define GROUP_INHERIT_ALLCONTENT GROUP_INHERIT_XUIOBJECTS | GROUP_INHERIT_SCRIPTS 

#define GROUP_GROUP       0
#define GROUP_CFGGROUP    1

#ifdef WASABI_COMPILE_WNDMGR
#define GROUP_LAYOUTGROUP 2
#endif

enum {
	ACTION_NONE,
	ACTION_UNIMPLEMENTED=0x1000,
	ACTION_MINIMIZE,
	ACTION_MAXIMIZE,
	ACTION_CLOSE,
	ACTION_ABOUT,
	ACTION_SWITCH,
	ACTION_TOGGLE,
	ACTION_SYSMENU,
	ACTION_CONTROLMENU,
	ACTION_REPORT_BUGS,
	ACTION_MB_BACK, //FG
	ACTION_MB_FORWARD, //FG
	ACTION_MB_URL, //FG
	ACTION_MB_STOP, //FG
	ACTION_MB_REFRESH, //FG
	ACTION_MB_HOME, //FG
	ACTION_CB_NEXT, //FG
	ACTION_CB_PREV, //FG
	ACTION_CB_NEXTPAGE,
	ACTION_CB_PREVPAGE,
	ACTION_SCALE_50, //FG
	ACTION_SCALE_75, //FG
	ACTION_SCALE_100, //FG
	ACTION_SCALE_125, //FG
	ACTION_SCALE_150, //FG
	ACTION_SCALE_200, //FG
	ACTION_SCALE_400, //BU :)
	ACTION_RELOAD_SKIN,
	ACTION_TEXT_LARGER,
	ACTION_TEXT_SMALLER,
	ACTION_PREFERENCES,
	ACTION_REGISTRY,
	ACTION_ALPHA_10, //FG
	ACTION_ALPHA_20, //FG
	ACTION_ALPHA_30, //FG
	ACTION_ALPHA_40, //FG
	ACTION_ALPHA_50, //FG
	ACTION_ALPHA_60, //FG
	ACTION_ALPHA_70, //FG
	ACTION_ALPHA_80, //FG
	ACTION_ALPHA_90, //FG
	ACTION_ALPHA_100, //FG
	ACTION_AOT,	//BU always-on-top for this window only
	ACTION_TOGGLE_ALWAYS_ON_TOP,
	ACTION_MENU,
	ACTION_VIEW_FILE_INFO,
	ACTION_ADD_BOOKMARK,
	ACTION_EDIT_BOOKMARKS,
	ACTION_ENDMODAL,
	ACTION_ENFORCEMINMAX,
	ACTION_DOUBLESIZE,
	ACTION_CLOSE_WINDOW,
	ACTION_WINDOWMENU,
	ACTION_EQ_TOGGLE,
	ACTION_AUTOOPACIFY,
};

enum {
	DISPLAY_NONE,
	DISPLAY_SONGNAME,
	DISPLAY_SONGINFO,
	DISPLAY_SONGTITLE,	//BU
	DISPLAY_SONGARTIST,
	DISPLAY_SONGALBUM,
	DISPLAY_SONGLENGTH,
	DISPLAY_TIME,
	DISPLAY_CB, //FG
	DISPLAY_SONGBITRATE,
	DISPLAY_SONGSAMPLERATE,
	DISPLAY_SERVICE,
	DISPLAY_SONGINFO_TRANSLATED,
};

enum {
	ORIENTATION_HORIZONTAL,
	ORIENTATION_VERTICAL
};

enum {
	ALIGN_TOP,
	ALIGN_BOTTOM,
};

typedef struct 
{
	const wchar_t *tagname;
	int id;
	int needclosetag;
} xml_tag;

class XmlTagComp 
{
public:
	static int compareItem(void *p1, void *p2) 
	{
		return WCSICMP(((xml_tag *)p1)->tagname, ((xml_tag *)p2)->tagname);
	}
	static int compareAttrib(const wchar_t *attrib, void *item) 
	{
		return WCSICMP(attrib, ((xml_tag *)item)->tagname);
	}
};

typedef struct {
	int staticloading;
	int recording_container;
	int recording_groupdef;
	Group *curGroup;
	int inElements;
	int inAccelerators;
	int inStringTable;
	int inGroupDef;
	int inGroup;
	StringW includepath;
	skin_xmlreaderparams *groupparams;
	int instantiatinggroup;
	int scriptid;
	int allowscripts;
#ifdef WASABI_COMPILE_WNDMGR
	Container *curContainer;
	Layout *curLayout;
	int inContainer;
	int inLayout;
	int transcientcontainer;
#endif
} parser_status;

class SkinParser {

public:
	static void initialize();
	static void shutdown();

	static void setInitialFocus();

	static GuiObject *newDynamicGroup(const wchar_t *groupid, int grouptype=GROUP_GROUP, SkinItem *item=NULL, int specific_scriptid=-1, int scripts_enabled=1);
	static void parseGroup(SkinItem *groupitem, PtrList<ifc_xmlreaderparams> *ancestor_param_list, int params_only=0, int content_flags=GROUP_INHERIT_ALL);
	static void loadScriptXml(const wchar_t *filename, int scriptid);

	static void xmlReaderCallback(int start,  const wchar_t *xmltag, skin_xmlreaderparams *params);
	static void onXmlStartElement(const wchar_t *name, skin_xmlreaderparams *params);
	static void onXmlEndElement(const wchar_t *name);

	static void _onXmlStartElement(int object_type, const wchar_t *object_name, ifc_xmlreaderparams *params);
	static void _onXmlEndElement(int object_type, const wchar_t *object_name);

#ifdef WASABI_COMPILE_WNDMGR
	static GUID *getComponentGuid(const wchar_t *id);
	static int getComponentGuid(GUID *g, const wchar_t *p);
#endif

#ifdef WASABI_COMPILE_WNDMGR
	static int loadContainers(const wchar_t *skin); // todo: change name
	static void startupContainers(int scriptid=-1); // todo: change name
	static Container *loadContainerForWindowHolder(const wchar_t *groupid=NULL, GUID g=INVALID_GUID, int initit=1, int transcient=0, const wchar_t *containerid=NULL/*any*/, int container_flag=0/*dontcare*/);
	static Container *newDynamicContainer(const wchar_t *containerid, int transcient=0);
	static Container *getContainer(const wchar_t *name);
	static Layout *getLayout(const wchar_t *container_layout_pair);
	static Container *script_getContainer(const wchar_t *name);
	static Container *instantiateDynamicContainer(SkinItem *containeritem, int initit=1);
	static void componentToggled(GUID *g, int visible);
	static void sendNotifyToAllContainers(int notifymsg, int param1=0, int param2=0);
	static void toggleContainer(const wchar_t *);
	static void toggleContainer(int);
	static void showContainer(int num, int show);
	static void showContainer(const wchar_t *, int show);
	static PtrList<Container> containers;
	//static PtrList<Container> script_containers;
	static int getNumContainers();
	static Container *enumContainer(int n);
	static int script_getNumContainers();
	static Container *script_enumContainer(int id);
	static int isContainer(Container *c);
	static int verifyContainer(Container *);
	static void unloadAllContainers();
	static const wchar_t *getCurrentContainerId();
#endif

	static void cleanupScript(int scriptid);
	static void cleanUp();
#ifdef WA3COMPATIBILITY
	static void emmergencyReloadDefaultSkin();
#endif
	static void initGuiObject(GuiObject *o, Group *pgroup);
	static void initXmlObject(XmlObject *x, ifc_xmlreaderparams *params, int no_id=0);
	static void initLayout(Layout *l, Container *c);

	static int getAction(const wchar_t *action, const wchar_t **name=NULL);
	static int getDisplay(const wchar_t *display);
	static int getAlign(const wchar_t *align);
	static int getOrientation(const wchar_t *orient);
	static int parse(const wchar_t *str, const wchar_t *how);
	static int parseResize(const wchar_t *r);
	static int parseRegionOp(const wchar_t *r);
	static int parseGroupInheritance(const wchar_t *str);
	static ARGB32 parseColor(const wchar_t *color, int *error=NULL); // 1 for bitmap colors please
	static ARGB32 parseColorAlpha(const wchar_t *color); // 1 for bitmap colors please
	static const wchar_t *getXmlRootPath();
	static void pushParserState();
	static void popParserState();

#ifdef WASABI_COMPILE_WNDMGR
	static void noCenterSkin() { centerskin=0; }
#endif
	static const wchar_t *getCurrentGroupId();
	static void destroyGuiObject(GuiObject *o);
	static void fillGroup(Group *group, const wchar_t *groupid, SkinItem *specific_item=NULL, int params_only=0, int no_params=0, int scripts_enabled=1);

	static int getSkinRect(RECT *r, ifc_window *exclude=NULL);
	static void centerSkin();
	static void focusFirst();

	static GuiObject *xui_new(const wchar_t *classname);
	static void xui_delete(GuiObject *o);

	static double getSkinVersion();

	static void setAllLayoutsRatio(double ra);
	static void setAllLayoutsTransparency(int v);
//  static void setAllLayoutsAutoOpacify(int v, int force=0);
//  static void setAllLayoutsOverrideAlpha(int oa);
	static Layout *getMainLayout();

private:
	static GuiObject *createExternalGuiObject(const wchar_t *object_name, XmlObject **x, ifc_xmlreaderparams *params);

	static int getHex(const wchar_t *p,int size=-1);

	// xml parser handlers
	static Group *curGroup, *lastCreatedGroup;
	static int inScripts;
	static int inElements, inAccelerators, inStringTable;
	static int inGroupDef, inGroup; 
	static StringW includepath;  
	static int recording_container;
	static int recording_groupdef;
	static int staticloading;
	static PtrList<parser_status> statusstack;
	static int instantiatinggroup;
	static int scriptId;
	static int allowscripts;
	static skin_xmlreaderparams *groupparams;

#ifdef WASABI_COMPILE_WNDMGR
	static Container *curContainer, *lastCreatedContainer;
	static Layout *curLayout;
	static int centerskin;
	static int transcientcontainer;
	static int inContainer;
	static int inLayout;
#endif
	static double skinversion;
	static int loading_main_skinfile;

	static PtrListQuickSorted<xml_tag,XmlTagComp> quickxmltaglist;
	static SvcCacheT<svc_xuiObject> *xuiCache;
};

#endif