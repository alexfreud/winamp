#ifndef __EDITBOX_H
#define __EDITBOX_H

#include <api/wnd/wndclass/embeddedxui.h>

#define  EDITBOX_PARENT EmbeddedXuiObject

// -----------------------------------------------------------------------
class EditBox : public EDITBOX_PARENT {
  public:

    virtual const wchar_t *embeddedxui_getContentId() { return "wasabi.edit"; }
    virtual const wchar_t *embeddedxui_getEmbeddedObjectId() { return "wasabi.edit.box"; }
};

// -----------------------------------------------------------------------
extern char EditBoxXuiObjectStr[];
extern char EditBoxXuiSvcName[];
class EditBoxXuiSvc : public XuiObjectSvc<EditBox, EditBoxXuiObjectStr, EditBoxXuiSvcName> {};

#endif
