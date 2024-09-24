#ifndef __API_SKIN_H
#define __API_SKIN_H

#include <wasabicfg.h>
#include <bfc/dispatch.h>

class ifc_window;
class GuiObject;
class ColorThemeGroup;
class ifc_xmlreaderparams;
class SkinItem;

#ifdef WASABI_COMPILE_CONFIG
class CfgItem;
#endif //WASABI_COMPILE_CONFIG

#ifdef WASABI_COMPILE_WNDMGR
class CfgItem;
#endif //WASABI_COMPILE_WNDMGR

#ifdef _WIN32
#ifndef OSCURSOR
#define OSCURSOR HICON
#endif
#elif defined(__APPLE__)
#define OSCURSOR CGImageRef
#endif

class NOVTABLE api_skin : public Dispatchable
{
  public:
    // skin colors
    ARGB32 skin_getColorElement(const wchar_t *type, const wchar_t **color_group = NULL);
    const ARGB32 *skin_getColorElementRef(const wchar_t *type, const wchar_t **color_group = NULL);
    const int *skin_getIterator();

    // possibly make a svc_skinloader
    void skin_switchSkin(const wchar_t *skin_name, const wchar_t *skin_path=NULL);
    void skin_unloadSkin();
    int loadSkinFile(const wchar_t *xmlfile);
    int loadGroupDefData(const wchar_t *groupdef, SkinItem **lastgroupitem);
    void unloadSkinPart(int skinpartid);
    const wchar_t *getSkinName();
    const wchar_t *getSkinPath();
    const wchar_t *getSkinsPath();
    const wchar_t *getDefaultSkinPath();
    
    // skin bitmaps
    ARGB32 *imgldr_requestSkinBitmap(const wchar_t *file, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached);
    void imgldr_releaseSkinBitmap(ARGB32 *bmpbits);
#ifdef WASABI_COMPILE_IMGLDR
    ARGB32 skin_getBitmapColor(const wchar_t *bitmapid);
#endif
    
    // skin filters
    ARGB32 filterSkinColor(ARGB32 color, const wchar_t *elementid, const wchar_t *groupname);
    void reapplySkinFilters();
    int colortheme_getNumColorSets();
    const wchar_t *colortheme_enumColorSet(int n);
    int colortheme_getNumColorGroups(const wchar_t *colorset);
    const wchar_t *colortheme_enumColorGroupName(const wchar_t *colorset, int n);
    ColorThemeGroup *colortheme_enumColorGroup(int colorset, int n);
    ColorThemeGroup *colortheme_getColorGroup(const wchar_t *colorset, const wchar_t *colorgroup);
    void colortheme_setColorSet(const wchar_t *colorset);
    const wchar_t *colortheme_getColorSet();
    void colortheme_newColorSet(const wchar_t *set);
    void colortheme_updateColorSet(const wchar_t *set);
    void colortheme_renameColorSet(const wchar_t *set, const wchar_t *newname);
    void colortheme_deleteColorSet(const wchar_t *set);
    
    // groups
    ifc_window *group_create(const wchar_t *groupid, int scripts_enabled=1);
    int group_exists(const wchar_t *groupid);
#ifdef WASABI_COMPILE_CONFIG
    ifc_window *group_create_cfg(const wchar_t *groupid, CfgItem *cfgitem, const wchar_t *attributename, int scripts_enabled=1);
#endif // WASABI_COMPILE_CONFIG
#ifdef WASABI_COMPILE_WNDMGR
    ifc_window *group_create_layout(const wchar_t *groupid, int scripts_enabled=1);
#endif //WASABI_COMPILE_WNDMGR
    int group_destroy(ifc_window *group);
    
    int parse(const wchar_t *str, const wchar_t *how);
    GuiObject *xui_new(const wchar_t *classname);
    void xui_delete(GuiObject *o);
    OSCURSOR cursor_request(const wchar_t *id);
    int getNumGroupDefs();
    SkinItem *enumGroupDef(int n);
    ifc_window *group_createBySkinItem(SkinItem *groupitem, int scripts_enabled=1);
    SkinItem *getGroupDefAncestor(SkinItem *groupitem);
    int groupdef_getNumObjects(SkinItem *groupitem);
    SkinItem *groupdef_enumObject(SkinItem *item, int n);
    void skin_setLockUI(int l);
    int skin_getLockUI();
    double skin_getVersion(); 
		bool skin_isLoaded();

    
  enum {
    API_SKIN_SKIN_GETCOLORELEMENT = 0,
    API_SKIN_SKIN_GETCOLORELEMENTREF = 10,
    API_SKIN_SKIN_GETITERATOR = 20,
    API_SKIN_SKIN_SWITCHSKIN = 30,
    API_SKIN_SKIN_UNLOADSKIN = 35,
    API_SKIN_GETSKINNAME = 40,
    API_SKIN_GETSKINPATH = 50,
    API_SKIN_GETSKINSPATH = 60,
    API_SKIN_GETDEFAULTSKINPATH = 70,
    API_SKIN_IMGLDR_REQUESTSKINBITMAP = 80,
    API_SKIN_IMGLDR_RELEASESKINBITMAP = 90,
    API_SKIN_FILTERSKINCOLOR = 100,
    API_SKIN_REAPPLYSKINFILTERS = 110,
    API_SKIN_COLORTHEME_GETNUMCOLORSETS = 120,
    API_SKIN_COLORTHEME_ENUMCOLORSET = 130,
    API_SKIN_COLORTHEME_GETNUMCOLORGROUPS = 140,
    API_SKIN_COLORTHEME_ENUMCOLORGROUPNAME = 150,
    API_SKIN_COLORTHEME_ENUMCOLORGROUP = 160,
    API_SKIN_COLORTHEME_GETCOLORGROUP = 165,
    API_SKIN_COLORTHEME_SETCOLORSET = 170,
    API_SKIN_COLORTHEME_GETCOLORSET = 180,
    API_SKIN_COLORTHEME_NEWCOLORSET = 190,
    API_SKIN_COLORTHEME_RENAMESET = 191,
    API_SKIN_COLORTHEME_DELETE = 192,
    API_SKIN_COLORTHEME_UPDATECOLORSET = 193,
    API_SKIN_LOADSKINFILE = 200,
    API_SKIN_UNLOADSKINPART = 210,
    API_SKIN_GROUP_CREATE = 220,
    API_SKIN_GROUP_EXISTS = 225,
#ifdef WASABI_COMPILE_CONFIG
    API_SKIN_GROUP_CREATE_CFG = 230,
#endif // WASABI_COMPILE_CONFIG
#ifdef WASABI_COMPILE_WNDMGR
    API_SKIN_GROUP_CREATE_LAYOUT = 240,
#endif //WASABI_COMPILE_WNDMGR
    API_SKIN_GROUP_DESTROY = 250,
    API_SKIN_PARSE = 260,
    API_SKIN_XUI_NEW = 270,
    API_SKIN_XUI_DELETE = 280,
    API_SKIN_CURSOR_REQUEST = 290,
    API_SKIN_GETNUMGROUPS = 300,
    API_SKIN_ENUMGROUP = 310,
    API_SKIN_GROUP_CREATEBYITEM = 320,
    API_SKIN_GETGROUPANCESTOR = 330,
    API_SKIN_GROUPDEF_GETNUMOBJECTS = 340,
    API_SKIN_GROUPDEF_ENUMOBJECT = 350,
    API_SKIN_LOADGROUPDEFDATA = 360,
    API_SKIN_SETLOCKUI = 370,
    API_SKIN_GETLOCKUI = 380,
    API_SKIN_GETVERSION = 390,
    API_SKIN_GETBITMAPCOLOR = 400,
		API_SKIN_ISLOADED = 410,
  };
};

inline ARGB32 api_skin::skin_getColorElement(const wchar_t *type, const wchar_t **color_group) {
  return _call(API_SKIN_SKIN_GETCOLORELEMENT, (ARGB32)NULL, type, color_group);
}

inline const ARGB32 *api_skin::skin_getColorElementRef(const wchar_t *type, const wchar_t **color_group) {
  return _call(API_SKIN_SKIN_GETCOLORELEMENTREF, (ARGB32 *)NULL, type, color_group);
}

inline const int *api_skin::skin_getIterator() {
  return _call(API_SKIN_SKIN_GETITERATOR, (const int *)0);
}

inline void api_skin::skin_switchSkin(const wchar_t *skin_name, const wchar_t *skin_path) {
  _voidcall(API_SKIN_SKIN_SWITCHSKIN, skin_name, skin_path);
}

inline void api_skin::skin_unloadSkin() {
  _voidcall(API_SKIN_SKIN_UNLOADSKIN);
}

inline const wchar_t *api_skin::getSkinName() {
  return _call(API_SKIN_GETSKINNAME, (const wchar_t *)0);
}

inline const wchar_t *api_skin::getSkinPath() {
  return _call(API_SKIN_GETSKINPATH, (const wchar_t *)0);
}

inline const wchar_t *api_skin::getSkinsPath() {
  return _call(API_SKIN_GETSKINSPATH, (const wchar_t *)0);
}

inline const wchar_t *api_skin::getDefaultSkinPath() {
  return _call(API_SKIN_GETDEFAULTSKINPATH, (const wchar_t *)0);
}

inline ARGB32 *api_skin::imgldr_requestSkinBitmap(const wchar_t *file, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached) {
  return _call(API_SKIN_IMGLDR_REQUESTSKINBITMAP, (ARGB32 *)NULL, file, has_alpha, x, y, subw, subh, w, h, cached);
}

inline void api_skin::imgldr_releaseSkinBitmap(ARGB32 *bmpbits) {
  _voidcall(API_SKIN_IMGLDR_RELEASESKINBITMAP, bmpbits);
}

inline ARGB32 api_skin::filterSkinColor(ARGB32 color, const wchar_t *elementid, const wchar_t *groupname) {
  return _call(API_SKIN_FILTERSKINCOLOR, (ARGB32)NULL, color, elementid, groupname);
}

inline void api_skin::reapplySkinFilters() {
  _voidcall(API_SKIN_REAPPLYSKINFILTERS);
}

inline int api_skin::colortheme_getNumColorSets() {
  return _call(API_SKIN_COLORTHEME_GETNUMCOLORSETS, (int)0);
}

inline const wchar_t *api_skin::colortheme_enumColorSet(int n) {
  return _call(API_SKIN_COLORTHEME_ENUMCOLORSET, (const wchar_t *)0, n);
}

inline int api_skin::colortheme_getNumColorGroups(const wchar_t *colorset) {
  return _call(API_SKIN_COLORTHEME_GETNUMCOLORGROUPS, (int)0, colorset);
}

inline const wchar_t *api_skin::colortheme_enumColorGroupName(const wchar_t *colorset, int n) {
  return _call(API_SKIN_COLORTHEME_ENUMCOLORGROUPNAME, (const wchar_t *)0, colorset, n);
}

inline ColorThemeGroup *api_skin::colortheme_enumColorGroup(int colorset, int n) {
  return _call(API_SKIN_COLORTHEME_ENUMCOLORGROUP, (ColorThemeGroup *)NULL, colorset, n);
}

inline ColorThemeGroup *api_skin::colortheme_getColorGroup(const wchar_t *colorset, const wchar_t *group) {
  return _call(API_SKIN_COLORTHEME_GETCOLORGROUP, (ColorThemeGroup *)NULL, colorset, group);
}

inline void api_skin::colortheme_setColorSet(const wchar_t *colorset) 
{
  _voidcall(API_SKIN_COLORTHEME_SETCOLORSET, colorset);
}

inline const wchar_t *api_skin::colortheme_getColorSet() 
{
  return _call(API_SKIN_COLORTHEME_GETCOLORSET, (const wchar_t *)0);
}

inline void api_skin::colortheme_newColorSet(const wchar_t *set) {
  _voidcall(API_SKIN_COLORTHEME_NEWCOLORSET, set);
}

inline void api_skin::colortheme_updateColorSet(const wchar_t *set) {
  _voidcall(API_SKIN_COLORTHEME_UPDATECOLORSET, set);
}

inline void api_skin::colortheme_renameColorSet(const wchar_t *set, const wchar_t *newname) {
  _voidcall(API_SKIN_COLORTHEME_RENAMESET, set, newname);
}

inline void api_skin::colortheme_deleteColorSet(const wchar_t *set) {
  _voidcall(API_SKIN_COLORTHEME_DELETE, set);
}


inline int api_skin::loadSkinFile(const wchar_t *xmlfile) {
  return _call(API_SKIN_LOADSKINFILE, (int)0, xmlfile);
}

inline void api_skin::unloadSkinPart(int skinpartid) {
  _voidcall(API_SKIN_UNLOADSKINPART, skinpartid);
}

inline ifc_window *api_skin::group_create(const wchar_t *groupid, int scripts_enabled) {
  return _call(API_SKIN_GROUP_CREATE, (ifc_window *)NULL, groupid, scripts_enabled);
}

inline int api_skin::group_exists(const wchar_t *groupid) {
  return _call(API_SKIN_GROUP_EXISTS, 1, groupid);
}

#ifdef WASABI_COMPILE_CONFIG
inline ifc_window *api_skin::group_create_cfg(const wchar_t *groupid, CfgItem *cfgitem, const wchar_t *attributename, int scripts_enabled) {
  return _call(API_SKIN_GROUP_CREATE_CFG, (ifc_window *)NULL, groupid, cfgitem, attributename, scripts_enabled);
}
#endif // WASABI_COMPILE_CONFIG

#ifdef WASABI_COMPILE_WNDMGR
inline ifc_window *api_skin::group_create_layout(const wchar_t *groupid, int scripts_enabled) {
  return _call(API_SKIN_GROUP_CREATE_LAYOUT, (ifc_window *)NULL, groupid, scripts_enabled);
}
#endif //WASABI_COMPILE_WNDMGR

inline int api_skin::group_destroy(ifc_window *group) {
  return _call(API_SKIN_GROUP_DESTROY, (int)0, group);
}

inline int api_skin::parse(const wchar_t *str, const wchar_t *how) {
  return _call(API_SKIN_PARSE, (int)0, str, how);
}

inline GuiObject *api_skin::xui_new(const wchar_t *classname) {
  return _call(API_SKIN_XUI_NEW, (GuiObject *)NULL, classname);
}

inline void api_skin::xui_delete(GuiObject *o) {
  _voidcall(API_SKIN_XUI_DELETE, o);
}

inline OSCURSOR api_skin::cursor_request(const wchar_t *id) 
{
  return _call(API_SKIN_CURSOR_REQUEST, (OSCURSOR)NULL, id);
}

inline int api_skin::getNumGroupDefs() {
  return _call(API_SKIN_GETNUMGROUPS, (int)0);
}

inline SkinItem *api_skin::enumGroupDef(int n) {
  return _call(API_SKIN_ENUMGROUP, (SkinItem *)NULL, n);
}

inline ifc_window *api_skin::group_createBySkinItem(SkinItem *item, int scripts_enabled) {
  return _call(API_SKIN_GROUP_CREATEBYITEM, (ifc_window *)NULL, item, scripts_enabled);
}

inline SkinItem *api_skin::getGroupDefAncestor(SkinItem *item) {
  return _call(API_SKIN_GETGROUPANCESTOR, (SkinItem *)NULL, item);
}

inline int api_skin::groupdef_getNumObjects(SkinItem *groupitem) {
  return _call(API_SKIN_GROUPDEF_GETNUMOBJECTS, 0, groupitem);
}

inline SkinItem *api_skin::groupdef_enumObject(SkinItem *item, int n) {
  return _call(API_SKIN_GROUPDEF_ENUMOBJECT, (SkinItem *)NULL, item, n);
}

inline int api_skin::loadGroupDefData(const wchar_t *groupdef, SkinItem **lastgroupitem) {
  return _call(API_SKIN_LOADGROUPDEFDATA, -1, groupdef, lastgroupitem);
}

inline void api_skin::skin_setLockUI(int l) {
  _voidcall(API_SKIN_SETLOCKUI, l);
}

inline int api_skin::skin_getLockUI() {
  return _call(API_SKIN_GETLOCKUI, 0);
}

inline double api_skin::skin_getVersion() {
  return _call(API_SKIN_GETVERSION, 0.8);
}

#ifdef WASABI_COMPILE_IMGLDR
inline ARGB32 api_skin::skin_getBitmapColor(const wchar_t *bitmapid) {
  return _call(API_SKIN_GETBITMAPCOLOR, 0xFFFF00FF, bitmapid);
}
#endif

inline bool api_skin::skin_isLoaded()
{
	return _call(API_SKIN_ISLOADED, false);
}

class api_skinI : public api_skin 
{
  protected:
    api_skinI() {}
    virtual ~api_skinI() {}

  public:
    virtual ARGB32 skin_getColorElement(const wchar_t *type, const wchar_t **color_group = NULL)=0;
    virtual const ARGB32 *skin_getColorElementRef(const wchar_t *type, const wchar_t **color_group = NULL)=0;
    virtual const int *skin_getIterator()=0;
    virtual void skin_switchSkin(const wchar_t *skin_name, const wchar_t *skin_path=NULL)=0;
    virtual void skin_unloadSkin()=0;
    virtual const wchar_t *getSkinName()=0;
    virtual const wchar_t *getSkinPath()=0;
    virtual const wchar_t *getSkinsPath()=0;
    virtual const wchar_t *getDefaultSkinPath()=0;
    virtual ARGB32 *imgldr_requestSkinBitmap(const wchar_t *file, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached)=0;
    virtual void imgldr_releaseSkinBitmap(ARGB32 *bmpbits)=0;
    virtual ARGB32 filterSkinColor(ARGB32 color, const wchar_t *elementid, const wchar_t *groupname)=0;
    virtual void reapplySkinFilters()=0;
    virtual int colortheme_getNumColorSets()=0;
    virtual const wchar_t *colortheme_enumColorSet(int n)=0;
    virtual int colortheme_getNumColorGroups(const wchar_t *colorset)=0;
    virtual const wchar_t *colortheme_enumColorGroupName(const wchar_t *colorset, int n)=0;
    virtual ColorThemeGroup *colortheme_enumColorGroup(int colorset, int n)=0;
    virtual ColorThemeGroup *colortheme_getColorGroup(const wchar_t *colorset, const wchar_t *group)=0;
    virtual void colortheme_setColorSet(const wchar_t *colorset)=0;
    virtual const wchar_t *colortheme_getColorSet()=0;
    virtual void colortheme_newColorSet(const wchar_t *set)=0;
    virtual void colortheme_updateColorSet(const wchar_t *set)=0;
    virtual void colortheme_renameColorSet(const wchar_t *set, const wchar_t *newname)=0;
    virtual void colortheme_deleteColorSet(const wchar_t *set)=0;
    virtual int loadSkinFile(const wchar_t *xmlfile)=0;
    virtual int loadGroupDefData(const wchar_t *groupdef, SkinItem **lastgroupitem)=0;
    virtual void unloadSkinPart(int skinpartid)=0;
    virtual ifc_window *group_create(const wchar_t *groupid, int scripts_enabled=1)=0;
    virtual int group_exists(const wchar_t *groupid)=0;
#ifdef WASABI_COMPILE_CONFIG
    virtual ifc_window *group_create_cfg(const wchar_t *groupid, CfgItem *cfgitem, const wchar_t *attributename, int scripts_enabled=1)=0;
#endif // WASABI_COMPILE_CONFIG
#ifdef WASABI_COMPILE_WNDMGR
    virtual ifc_window *group_create_layout(const wchar_t *groupid, int scripts_enabled=1)=0;
#endif //WASABI_COMPILE_WNDMGR
    virtual int group_destroy(ifc_window *group)=0;
    virtual int parse(const wchar_t *str, const wchar_t *how)=0;
    virtual GuiObject *xui_new(const wchar_t *classname)=0;
    virtual void xui_delete(GuiObject *o)=0;
    virtual OSCURSOR cursor_request(const wchar_t *id)=0;
    virtual int getNumGroupDefs()=0;
    virtual SkinItem *enumGroupDef(int n)=0;
    virtual ifc_window *group_createBySkinItem(SkinItem *item, int scripts_enabled=1)=0;
    virtual SkinItem *getGroupDefAncestor(SkinItem *groupitem)=0;
    virtual int groupdef_getNumObjects(SkinItem *groupitem)=0;
    virtual SkinItem *groupdef_enumObject(SkinItem *groupitem, int n)=0;
    virtual void skin_setLockUI(int l)=0;
    virtual int skin_getLockUI()=0;
    virtual double skin_getVersion()=0;
#ifdef WASABI_COMPILE_IMGLDR
    virtual ARGB32 skin_getBitmapColor(const wchar_t *id)=0;
#endif
		virtual bool skin_isLoaded()=0;
  protected:
    RECVS_DISPATCH;
};

// {F2398F09-63B0-4442-86C9-F8BC473F6DA7}
static const GUID skinApiServiceGuid = 
{ 0xf2398f09, 0x63b0, 0x4442, { 0x86, 0xc9, 0xf8, 0xbc, 0x47, 0x3f, 0x6d, 0xa7 } };

extern api_skin *skinApi;


#endif
