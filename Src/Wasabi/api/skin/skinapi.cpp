#include <precomp.h>
#include <api.h>
#include "skinapi.h"
#include <api/skin/skin.h>
#include <api/wndmgr/skinembed.h>
#include <api/skin/skinelem.h>
#include <api/imgldr/imgldr.h>
#include <api/skin/gammamgr.h>
#include <api/skin/groupmgr.h>
#include <api/skin/cursormgr.h>
#include <api/skin/guitree.h>
#include <api/skin/groupwndcreate.h>
#include <api/wndmgr/autopopup.h>
#include <tataki/canvas/bltcanvas.h>

api_skin *skinApi;
static waServiceTSingle<svc_windowCreate, GroupWndCreateSvc> groupWndCreate;

SkinApi::SkinApi()
{
	lockui = 0;
	tha = new Skin();
	SkinParser::initialize();

	WASABI_API_SVC->service_register(&groupWndCreate);

	SkinElementsMgr::init();
	GammaMgr::init();

	// fixed this for 5.58+ so it'll use the correct skins directory
	// and not the winamp.exe folder + "skins" - fixes @SKINSPATH@
	// when the skins directory has been altered - from Bento notifier.xml
	skinspath = WASABI_API_APP->path_getSkinSettingsPath();
}

SkinApi::~SkinApi()
{
	delete tha; tha = NULL;
#ifdef WASABI_COMPILE_WNDMGR
	AutoPopup::reset();
#endif
	SkinElementsMgr::deinit();
	GammaMgr::deinit();
	WASABI_API_SVC->service_deregister(&groupWndCreate);
	SkinParser::shutdown();
}

void SkinApi::preShutdown()
{
	Skin::unloadSkin();
	SkinElementsMgr::resetSkinElements();
}

ARGB32 SkinApi::skin_getColorElement(const wchar_t *type, const wchar_t **color_group)
{
	return WASABI_API_PALETTE->getColorElement(type, color_group);
}

const ARGB32 *SkinApi::skin_getColorElementRef(const wchar_t *type, const wchar_t **color_group)
{
	return WASABI_API_PALETTE->getColorElementRef(type, color_group);
}

const int *SkinApi::skin_getIterator()
{
	return WASABI_API_PALETTE->getSkinPartIteratorPtr();
}

void SkinApi::skin_switchSkin(const wchar_t *skin_name, const wchar_t *skin_path)
{
	if (skin_name && *skin_name) Skin::toggleSkin(skin_name, skin_path, 1);
}

void SkinApi::skin_unloadSkin()
{
	Skin::unloadSkin();
}

const wchar_t *SkinApi::getSkinName()
{
	return Skin::getSkinName();
}

const wchar_t *SkinApi::getSkinPath()
{
	return Skin::getSkinPath();
}

const wchar_t *SkinApi::getSkinsPath()
{
	return skinspath;
}

const wchar_t *SkinApi::getDefaultSkinPath()
{
	return Skin::getDefaultSkinPath();
}

ARGB32 *SkinApi::imgldr_requestSkinBitmap(const wchar_t *file, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached)
{
	if (file == NULL)
	{
		DebugStringW(L"illegal param : file == NULL\n");
		return NULL;
	}
	return imageLoader::requestSkinBitmap(file, has_alpha, x, y, subw, subh, w, h, cached);
}

void SkinApi::imgldr_releaseSkinBitmap(ARGB32 *bmpbits)
{
	if (bmpbits == NULL)
	{
		DebugStringW(L"illegal param : bmpbits == NULL\n");
		return ;
	}
	imageLoader::releaseSkinBitmap(bmpbits);
}

ARGB32 SkinApi::filterSkinColor(ARGB32 color, const wchar_t *elementid, const wchar_t *groupname)
{
	return imageLoader::filterSkinColor(color, elementid, groupname);
}

void SkinApi::reapplySkinFilters()
{
	imageLoader::applySkinFilters();
}

/* ---------------------------------------- */
int SkinApi::colortheme_getNumColorSets()
{
	return WASABI_API_COLORTHEMES->getNumGammaSets();
}

const wchar_t *SkinApi::colortheme_enumColorSet(int n)
{
	return WASABI_API_COLORTHEMES->enumGammaSet(n);
}

int SkinApi::colortheme_getNumColorGroups(const wchar_t *colorset)
{
	return WASABI_API_COLORTHEMES->getNumGammaGroups(colorset);
}

const wchar_t *SkinApi::colortheme_enumColorGroupName(const wchar_t *colorset, int n)
{
	return WASABI_API_COLORTHEMES->enumGammaGroup(colorset, n);
}

ColorThemeGroup *SkinApi::colortheme_enumColorGroup(int colorset, int colorgroup)
{
	return WASABI_API_COLORTHEMES->enumColorThemeGroup(colorset, colorgroup);
}

ColorThemeGroup *SkinApi::colortheme_getColorGroup(const wchar_t *colorset, const wchar_t *colorgroup)
{
	return WASABI_API_COLORTHEMES->getColorThemeGroup(colorset, colorgroup);
}

void SkinApi::colortheme_setColorSet(const wchar_t *colorset)
{
	WASABI_API_COLORTHEMES->setGammaSet(colorset);
	// TODO: benski> move this to a syscallback: SysCallback::SKINCB, SkinCallback::COLORTHEMECHANGED
	WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"Color Themes/%s", getSkinName()), colorset);
}

const wchar_t *SkinApi::colortheme_getColorSet()
{
	return WASABI_API_COLORTHEMES->getGammaSet();
}

void SkinApi::colortheme_newColorSet(const wchar_t *set)
{
	WASABI_API_COLORTHEMES->newGammaSet(set);
}

void SkinApi::colortheme_updateColorSet(const wchar_t *set)
{
	WASABI_API_COLORTHEMES->updateGammaSet(set);
}

void SkinApi::colortheme_renameColorSet(const wchar_t *set, const wchar_t *newname)
{
	WASABI_API_COLORTHEMES->renameGammaSet(set, newname);
}

void SkinApi::colortheme_deleteColorSet(const wchar_t *set)
{
	WASABI_API_COLORTHEMES->deleteGammaSet(set);
}

 /* -------------------------------------------- */

int SkinApi::loadSkinFile(const wchar_t *xmlfile)
{
	return Skin::loadSkinPart(xmlfile);
}

void SkinApi::unloadSkinPart(int skinpartid)
{
	Skin::unloadSkinPart(skinpartid);
}

ifc_window *SkinApi::group_create(const wchar_t *groupid, int scripts_enabled)
{
	return GroupMgr::instantiate(groupid, GROUP_GROUP, NULL, scripts_enabled);
}

int SkinApi::group_destroy(ifc_window *group)
{
	return GroupMgr::destroy(static_cast<Group*>(group));
}

int SkinApi::group_exists(const wchar_t *groupid)
{
	return GroupMgr::exists(groupid);
}

#ifdef WASABI_COMPILE_CONFIG
ifc_window *SkinApi::group_create_cfg(const wchar_t *groupid, CfgItem *cfgitem, const wchar_t *attributename, int scripts_enabled)
{
	return GroupMgr::instantiate(groupid, cfgitem, attributename, scripts_enabled);
}
#endif // WASABI_COMPILE_CONFIG

#ifdef WASABI_COMPILE_WNDMGR
ifc_window *SkinApi::group_create_layout(const wchar_t *groupid, int scripts_enabled)
{
	return GroupMgr::instantiate(groupid, GROUP_LAYOUTGROUP, NULL, scripts_enabled);
}
#endif //WASABI_COMPILE_WNDMGR

OSCURSOR SkinApi::cursor_request(const wchar_t *id)
{
	return CursorMgr::requestCursor(id);
}

int SkinApi::parse(const wchar_t *str, const wchar_t *how)
{
	return SkinParser::parse(str, how);
}

GuiObject *SkinApi::xui_new(const wchar_t *classname)
{
	return SkinParser::xui_new(classname);
}

void SkinApi::xui_delete(GuiObject *o)
{
	SkinParser::xui_delete(o);
}

int SkinApi::getNumGroupDefs()
{
	return guiTree->getNumGroupDefs();
}

SkinItem *SkinApi::enumGroupDef(int n)
{
	return guiTree->enumGroupDef(n);
}

ifc_window *SkinApi::group_createBySkinItem(SkinItem *item, int scripts_enabled)
{
	return GroupMgr::instantiate(NULL, GROUP_GROUP, item, scripts_enabled);
}

SkinItem *SkinApi::getGroupDefAncestor(SkinItem *item)
{
	return guiTree->getGroupDefAncestor(item);
}

int SkinApi::groupdef_getNumObjects(SkinItem *_item)
{
	GuiTreeItem *item = static_cast<GuiTreeItem *>(_item);
	int idx = item->getIdx();
	idx++;
	int n = 0;
	while (1)
	{
		GuiTreeItem *it = guiTree->getList()->enumItem(idx);
		if (it->getType() == XML_TAG_GROUPDEF && it->getParams() == NULL) break;
		idx++; n++;
	}
	return n;
}

SkinItem *SkinApi::groupdef_enumObject(SkinItem *_item, int n)
{
	GuiTreeItem *item = static_cast<GuiTreeItem *>(_item);
	int idx = item->getIdx();
	idx++;
	int _n = 0;
	GuiTreeItem *it = NULL;
	while (1)
	{
		it = guiTree->getList()->enumItem(idx);
		if (it->getType() == XML_TAG_GROUPDEF && it->getParams() == NULL) break;
		if (n == _n) break;
		idx++; _n++;
	}
	return it;
}

int SkinApi::loadGroupDefData(const wchar_t *groupdef, SkinItem **lastgroupdef)
{
	StringW s;
	s = L"buf:";
  
	s += L"<?xml version=\"1.0\" encoding=\"UTF-16\" standalone=\"yes\"?>"
	     L"<WinampAbstractionLayer version=\"0.8\">";

	s += groupdef;

	s += L"</WinampAbstractionLayer>";

	int r = Skin::loadSkinPart(s);
	if (lastgroupdef != NULL)
		*lastgroupdef = guiTree->getLastDefinedGroup();
	return r;
}

double SkinApi::skin_getVersion()
{
	return SkinParser::getSkinVersion();
}
#ifdef WASABI_COMPILE_IMGLDR
ARGB32 SkinApi::skin_getBitmapColor(const wchar_t *id)
{
	SkinBitmap bitmap(id);
	BltCanvas c(bitmap.getWidth() + 1, bitmap.getHeight() + 1, 0); // TODO: this won't work on the mac i don't think
	bitmap.blit(&c, 0, 0);
	int x = bitmap.getWidth() / 2;
	int y = bitmap.getHeight() / 2;
	int *bits = (int *)c.getBits();
	if (bits != NULL)
	{
		return bits[x + y*bitmap.getWidth() + 1];
	}
	return 0xFFFF00FF;
}
#endif

bool SkinApi::skin_isLoaded()
{
	return Skin::isLoaded();
}