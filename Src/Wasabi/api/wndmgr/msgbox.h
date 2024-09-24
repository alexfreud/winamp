#ifndef __MESSAGEBOX_H
#define __MESSAGEBOX_H

#include <bfc/string/bfcstring.h>
#include <bfc/string/StringW.h>
#include <bfc/ptrlist.h>

#define MSGBOX_ABORTED    0 // NOT a flag, only a return code

#define MSGBOX_OK         1
#define MSGBOX_CANCEL     2
#define MSGBOX_YES        4
#define MSGBOX_NO         8
#define MSGBOX_ALL        16
#define MSGBOX_NEXT       32
#define MSGBOX_PREVIOUS   64

class GuiObject;
class SkinWnd;


typedef struct {
  wchar_t *txt;
  int id;
} _btnstruct;

class MsgBox {
  public:
    
    MsgBox(const wchar_t *text, const wchar_t *title=L"Alert", int flags=MSGBOX_OK, const wchar_t *notanymore=NULL);
    virtual ~MsgBox();
    virtual int run();


  private:

    void createButtons();
    int reposButtons();
    void addButton(const wchar_t *text, int retcode);

    StringW text, title;
    int flags;
    PtrList<GuiObject> buttons;
    GuiObject *checkbox;
    SkinWnd *sw;
    StringW notanymore_id;
};

#endif