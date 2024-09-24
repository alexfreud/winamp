#ifndef __SKINAPI_H
#define __SKINAPI_H

#include <api/skin/api_skin.h>
#include <bfc/string/StringW.h>
#include <api/skin/widgets.h>
#include <api/xml/xmlparams.h>

class SkinApi : public api_skinI 
{
  public:

     SkinApi();
     virtual ~SkinApi();
     virtual void preShutdown();

     virtual ARGB32 skin_getColorElement(const wchar_t *type, const wchar_t **color_group = NULL);
     virtual const ARGB32 *skin_getColorElementRef(const wchar_t *type, const wchar_t **color_group = NULL);
     virtual const int *skin_getIterator();
     virtual void skin_switchSkin(const wchar_t *skin_name, const wchar_t *skin_path);
     virtual void skin_unloadSkin();
     virtual const wchar_t *getSkinName();
     virtual const wchar_t *getSkinPath();
     virtual const wchar_t *getSkinsPath();
     virtual const wchar_t *getDefaultSkinPath();
     virtual ARGB32 *imgldr_requestSkinBitmap(const wchar_t *file, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached);
     virtual void imgldr_releaseSkinBitmap(ARGB32 *bmpbits);
     virtual ARGB32 filterSkinColor(ARGB32 color, const wchar_t *elementid, const wchar_t *groupname);
     virtual void reapplySkinFilters();
     virtual int colortheme_getNumColorSets();
     virtual const wchar_t *colortheme_enumColorSet(int n);
     virtual int colortheme_getNumColorGroups(const wchar_t *colorset);
     virtual const wchar_t *colortheme_enumColorGroupName(const wchar_t *colorset, int n);
     virtual ColorThemeGroup *colortheme_enumColorGroup(int colorset, int n);
     virtual ColorThemeGroup *colortheme_getColorGroup(const wchar_t *colorset, const wchar_t *group);
     virtual void colortheme_setColorSet(const wchar_t *colorset);
     virtual const wchar_t *colortheme_getColorSet();
     virtual void colortheme_newColorSet(const wchar_t *set);
     virtual void colortheme_updateColorSet(const wchar_t *set);
     virtual void colortheme_renameColorSet(const wchar_t *set, const wchar_t *newname);
     virtual void colortheme_deleteColorSet(const wchar_t *set);
     virtual int loadSkinFile(const wchar_t *xmlfile);
     virtual int loadGroupDefData(const wchar_t *groupdef, SkinItem **lastgroupitem);
     virtual void unloadSkinPart(int skinpartid);
     virtual ifc_window *group_create(const wchar_t *groupid, int scripts_enabled=1);
     virtual int group_exists(const wchar_t *groupid);
#ifdef WASABI_COMPILE_CONFIG
     virtual ifc_window *group_create_cfg(const wchar_t *groupid, CfgItem *cfgitem, const wchar_t *attributename, int scripts_enabled=1);
#endif // WASABI_COMPILE_CONFIG
#ifdef WASABI_COMPILE_WNDMGR
     virtual ifc_window *group_create_layout(const wchar_t *groupid, int scripts_enabled=1);
#endif //WASABI_COMPILE_WNDMGR
     virtual int group_destroy(ifc_window *group);
     virtual int parse(const wchar_t *str, const wchar_t *how);
     virtual GuiObject *xui_new(const wchar_t *classname);
     virtual void xui_delete(GuiObject *o);
     virtual OSCURSOR cursor_request(const wchar_t *id);

     virtual int getNumGroupDefs();
     virtual SkinItem *enumGroupDef(int n);
     virtual ifc_window *group_createBySkinItem(SkinItem *item, int scripts_enabled=1);
     virtual SkinItem *getGroupDefAncestor(SkinItem *item);
     virtual int groupdef_getNumObjects(SkinItem *item);
     virtual SkinItem *groupdef_enumObject(SkinItem *groupitem, int n);
     virtual void skin_setLockUI(int l) { if (l) lockui++; else if (lockui) lockui--; }
     virtual int skin_getLockUI() { return lockui; }
     virtual double skin_getVersion();
#ifdef WASABI_COMPILE_IMGLDR
     virtual ARGB32 skin_getBitmapColor(const wchar_t *id);
#endif
		 bool skin_isLoaded();

  private:
    
     StringW skinspath;
     int lockui;
};

#endif
