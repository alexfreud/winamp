#include <precomp.h>
#include "varmgr.h"
#include <api/script/objects/guiobj.h>
#include <api/skin/widgets/group.h>
#include <api/skin/skinparse.h>
#include <api/wac/wac.h>
#include <bfc/parse/pathparse.h>

// for now only translates special text like :componentname etc but in the future
// this will translate xml variables

StringW *PublicVarManager::translate_nocontext(const wchar_t *str) 
{
  return translate(str, NULL, NULL);
}

//Martin> TODO: Split this method for <include/>, <bitmap/>,... and <text/> objects
StringW *PublicVarManager::translate(const wchar_t *str, GuiObject *o, Container *c) 
{
  if (str == NULL) return NULL;
  //test if @ or : is present
  {
    const wchar_t *p=str;
    int found=0;
    while(p && *p && !found) {
      wchar_t a=*p++;
      if(a==':' || a=='@') found=1;
    }
    if(!found) 
			return NULL;
  }

  // lone> this needs to be a service, but it may slow shit down if we enum too often since this is called for all xml values, so carfull when you add this, lone
  #ifdef CUSTOM_VARS
  const wchar_t *rpl=NULL;
  CUSTOM_VARS(str, rpl);
  if (rpl != NULL) return new StringW(rpl);
  #endif

  const wchar_t *skinname = NULL;
  if (WASABI_API_SKIN != NULL) 
    skinname=WASABI_API_SKIN->getSkinName();

  static int in=0;
  if (in)
		return NULL;
  in = 1;

  StringW *ret = new StringW(str);

  if (skinname) 
	{
		StringW colorThemePath = WASABI_API_APP->path_getAppPath();
		colorThemePath.AppendPath(L"ColorThemes");
		colorThemePath.AppendPath(skinname);

    ret->replace(L"@COLORTHEMESPATH@", colorThemePath);
    ret->replace(L"@SKINPATH@", WASABI_API_SKIN->getSkinPath());
    ret->replace(L"@SKINSPATH@", WASABI_API_SKIN->getSkinsPath());
	ret->replace(L"@APPDATAPATH@", WASABI_API_APP->path_getUserSettingsPath());
    //ret->replace(L"@DEFAULTSKINPATH@", StringPathCombine(WASABI_API_SKIN->getSkinsPath(), L"Default")); //Martin> doesn't exist in winamp5, so cut to speed loading
	ret->replace(L"@WINAMPPATH@", StringPrintfW(L"%s\\", WASABI_API_APP->path_getAppPath()));
  }
#ifdef WASABI_COMPILE_COMPONENTS
  ret->replace("@WACDATAPATH@", WASABI_API_APP->path_getComponentDataPath());
#endif

  if (!o && !c) { in = 0; return ret; }

//CUT wtf?  if (!str) return NULL;

  const wchar_t *containerid = NULL;
  const wchar_t *groupid = NULL;
  const wchar_t *componentName = NULL;

  if (o && o->guiobject_getParentGroup()) 
	{
    groupid = o->guiobject_getParentGroup()->getGuiObject()->guiobject_getId();
    if (o->guiobject_getParentGroup()->getParentContainer()) 
		{
      containerid = o->guiobject_getParentGroup()->getParentContainer()->getId();
      componentName = o->guiobject_getParentGroup()->getParentContainer()->getName();
    }
  } 
	else {
    groupid = SkinParser::getCurrentGroupId();
    if (c) {
      containerid = c->getId();
      componentName = /*c->hasComponent() ? api->getComponentName(c->getGUID()) : */c->getName();
    } else {
      containerid = SkinParser::getCurrentContainerId();
      componentName = NULL;
    }
  }

  if (componentName != NULL) {
    ret->replace(L":componentname", componentName);  // DEPRECATED
    ret->replace(L"@COMPONENTNAME@", componentName);
  }
  if (containerid != NULL) {
    ret->replace(L":containerid", containerid);      // DEPRECATED
    ret->replace(L"@CONTAINERID@", containerid);
  }
  if (groupid != NULL) {
    ret->replace(L":groupid", groupid);              // DEPRECATED
    ret->replace(L"@GROUPID@",groupid);
  }

  in = 0;
  return ret;
}
