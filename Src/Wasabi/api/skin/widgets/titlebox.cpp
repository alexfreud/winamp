#include <precomp.h>
#include "titlebox.h"
#include <api/script/objects/guiobject.h>
#include <api/script/scriptobj.h>
#include <api/script/objects/c_script/c_text.h>
#include <api/skin/widgets/customobject.h>

TitleBox::TitleBox() {
  titleleft = NULL;
  titleright = NULL;
  titlecenter = NULL;
  content = NULL;
  centered = 0;
}

TitleBox::~TitleBox() {
  delete titleleft;
  delete titleright;
  delete titlecenter;
  delete content;
}

int TitleBox::onResize() {
  int rt = TITLEBOX_PARENT::onResize();
  if (!isInited()) return 0;

  RECT r, d;
  getClientRect(&r);

  int textwidth = titlecenter ? titlecenter->getPreferences(SUGGESTED_W) : 0;

  d = r;

  if (titleleft) {
    if (!getCentered()) 
      d.right = d.left + titleleft->getPreferences(SUGGESTED_W);
    else
      d.right = d.left + ((r.right-r.left) - textwidth) / 2;
    d.bottom = d.top + titleleft->getPreferences(SUGGESTED_H);
    titleleft->resize(d.left, d.top, d.right-d.left, d.bottom-d.top);
  }

  if (titlecenter) {
    d.left = d.right;
    d.right = d.left + titlecenter->getPreferences(SUGGESTED_W);
    d.bottom = d.top + titlecenter->getPreferences(SUGGESTED_H);
    titlecenter->resize(d.left, d.top, d.right-d.left, d.bottom-d.top);
  }

  if (titleright) {
    d.left = d.right;
    d.right = r.right;
    d.bottom = d.top + titleright->getPreferences(SUGGESTED_H);
    titleright->resize(d.left, d.top, d.right-d.left, d.bottom-d.top);
  }

  return rt;
}

int TitleBox::onInit() {
  int rt = TITLEBOX_PARENT::onInit();

  titleleft = new GuiObjectWnd();
  titleleft->setContent(L"wasabi.titlebox.left.group");
  titlecenter = new GuiObjectWnd();
  titlecenter->setContent(L"wasabi.titlebox.center.group");
  titleright = new GuiObjectWnd();
  titleright->setContent(L"wasabi.titlebox.right.group");

  titleleft->setParent(this);
  titlecenter->setParent(this);
  titleright->setParent(this);

  titleleft->init(this);
  titlecenter->init(this);
  titleright->init(this);

  setContent(L"wasabi.titlebox.main.group");
  return rt;
}

void TitleBox::setSubContent(int setcontent) {
  if (setcontent) {
    delete content;
    content = NULL;
  }

  GuiObject *myself = getContent(); 

  if (myself != NULL) 
	{
    if (setcontent) 
		{
      GuiObject *holder = findObject(L"titlebox.content"); // get the <CustomObject />
      if (holder != NULL) {
        CustomObject *obj = static_cast<CustomObject*>(holder->guiobject_getScriptObject()->vcpu_getInterface(customObjectGuid));
        if (obj != NULL) {
          content = new GuiObjectWnd();
          content->setContent(content_id);
          obj->customobject_setRootWnd(content); // and give it our content, it'll init it as needed
        }
      }
    }
    GuiObject *text = titlecenter->getContent()->guiobject_findObject(L"titlebox.text");
    if (text != NULL) {
      StringW name = getTitle();
      if (name.isempty()) {
        ifc_window *cr = content->getContentRootWnd();
        if (cr != NULL)
          name = cr->getRootWndName();
      }
      if (name.isempty()) 
        name = myself->guiobject_getRootWnd()->getRootWndName();
      if (!name.isempty()) 
			{
        name += suffix;
        C_Text ctext(*text);
        ctext.setText(name);
      }
    }
  }
  if (isPostOnInit())
    onResize();
}

void TitleBox::onNewContent() {
  TITLEBOX_PARENT::onNewContent();
  setSubContent();
}

void TitleBox::setCentered(int _centered) {
  centered = _centered;
  if (isInited())
    onResize();
}

void TitleBox::setTitle(const wchar_t *t) {
  title = t;
  setName(title);
  if (isInited())
    setSubContent(0);
}

void TitleBox::setSuffix(const wchar_t *_suffix) {
  if (!WCSICMPSAFE(suffix, _suffix)) return;
  suffix = _suffix;
  if (isInited())
    setSubContent(0);
}

void TitleBox::setChildGroup(const wchar_t *grp) {
  content_id = grp;
  if (isInited())
    setSubContent();
}

int TitleBox::getPreferences(int what) {
/*  GuiObject *contentGuiObj = findObject("titlebox.content");
  if (contentGuiObj != NULL) {
    // FUCKO, this is a huge hack but I don't have time to make autowidth/heightsource be a GuiObject thing instead of a group thing right now
    int x, y, rx, ry, w, rw, h, rh;
    contentGuiObj->guiobject_getGuiPosition(&x, &y, &w, &h, &rx, &ry, &rw, &rh);
    if (w == AUTOWH) { w = contentGuiObj->guiobject_getAutoWidth(); rw = 0; }
    if (h == AUTOWH) { h = contentGuiObj->guiobject_getAutoHeight(); rh = 0; }
    if (what == SUGGESTED_W) {
      int p = contentGuiObj->guiobject_getRootWnd()->getPreferences(SUGGESTED_W);
      if (rx == 0 && rw == 1) 
        return p - w; 
      else if (rx == 0 && rw == 0) 
        return p + x + w; 
    } else if (what == SUGGESTED_H) {
      int p = contentGuiObj->guiobject_getRootWnd()->getPreferences(SUGGESTED_H);
      if (ry == 0 && rh == 1) 
        return p + y + ((-h)-y); 
      else if (ry == 0 && rh == 0) 
        return p + y + h; 
    }
    ifc_window *contentGuiRootWnd = contentGuiObj->guiobject_getRootWnd();
    return contentGuiRootWnd->getPreferences(what);
  }*/
  return TITLEBOX_PARENT::getPreferences(what);
}

