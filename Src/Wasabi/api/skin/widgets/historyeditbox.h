#ifndef __HISTORYEDITBOX_H
#define __HISTORYEDITBOX_H

#include <api/skin/widgets/combobox.h>
#include <bfc/ptrlist.h>
#include <bfc/string/StringW.h>

#define HISTORYEDITBOX_PARENT ComboBox

class HistoryEditBox : public HISTORYEDITBOX_PARENT {

  public:

    HistoryEditBox();
    virtual ~HistoryEditBox();

    virtual const wchar_t *dropdownlist_getMainGroupId() { return L"wasabi.historyeditbox.main.group"; }
    virtual const wchar_t *dropdownlist_getListGroupId() { return L"wasabi.historyeditbox.list.group"; }
    virtual const wchar_t *dropdownlist_getButtonId() { return L"historyeditbox.button"; }
    virtual const wchar_t *dropdownlist_getListId() { return L"historyeditbox.list"; }

    virtual const wchar_t *combobox_getEditId() { return L"historyeditbox.edit"; }

    void onEditKeyDown(int vk);
    void onEditEnter(const wchar_t *txt);

    virtual int wantAutoSort() { return 0; }

    virtual int wantDownOpenList() { return history_pos == 0; }
    virtual void dropdownlist_onCloseList();
    virtual void onPreOpenList();
    virtual int onInit();
    virtual void addHistory(const wchar_t *txt);

    virtual int onAction(const wchar_t *action, const wchar_t *param=NULL, int x=-1, int y=-1, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, ifc_window *source=NULL);

    void history_back();
    void history_forward();

    enum {
      HISTORYEDITBOX_SETNAVBUTTONS=0,
    };
		
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:
static XMLParamPair params[];
#ifdef WASABI_COMPILE_CONFIG
    void saveHistory();
    void loadHistory(int refill=1);
#endif

    PtrList<StringW> history;
    int history_pos;
    int xuihandle;
};

#endif
