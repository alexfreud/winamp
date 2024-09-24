#include <precomp.h>
#include "historyeditbox.h"

XMLParamPair HistoryEditBox::params[] = {
	  {HISTORYEDITBOX_SETNAVBUTTONS, L"NAVBUTTONS"}, // param is implemented by script
};
HistoryEditBox::HistoryEditBox() {
  history_pos = 0;
  xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);

  setXmlParam(L"navbuttons", L"1"); // so we need to set a default value in the xml param list
}

void HistoryEditBox::CreateXMLParameters(int master_handle)
{
	//HISTORYEDITBOX_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

HistoryEditBox::~HistoryEditBox() {
  history.deleteAll();
}

void HistoryEditBox::history_forward() {
  if (history_pos > 0 && !isListOpen()) {
    history_pos--;
    if (history_pos > 0)
      setText(history.enumItem(history.getNumItems()-history_pos)->getValue(), 1);
  }
}

void HistoryEditBox::history_back() {
  if (!isListOpen() && history_pos < history.getNumItems()) {
    history_pos++;
    setText(history.enumItem(history.getNumItems()-history_pos)->getValue(), 1);
  }
}

void HistoryEditBox::onEditKeyDown(int vk) {
  HISTORYEDITBOX_PARENT::onEditKeyDown(vk);
  if (Std::keyDown(VK_CONTROL)) return;
  if (vk == VK_DOWN) {
    history_forward();
  } else if (vk == VK_UP) {
    history_back();
  }
}

int HistoryEditBox::onInit() {
  int r = HISTORYEDITBOX_PARENT::onInit();
#ifdef WASABI_COMPILE_CONFIG
  loadHistory();
#endif
  return r;
}

void HistoryEditBox::dropdownlist_onCloseList() {
  HISTORYEDITBOX_PARENT::dropdownlist_onCloseList();
  history_pos = 0;
}

void HistoryEditBox::onPreOpenList() 
{
  HISTORYEDITBOX_PARENT::onPreOpenList();
  addHistory(getText());
}

void HistoryEditBox::onEditEnter(const wchar_t *txt) 
{
  HISTORYEDITBOX_PARENT::onEditEnter(txt);
  if (Std::keyDown(VK_CONTROL)) return;
  addHistory(txt);
}

void HistoryEditBox::addHistory(const wchar_t *txt) 
{
  HISTORYEDITBOX_PARENT::onEditEnter(txt);
  history_pos = 0;
  
  if (!txt || !*txt) return;

  // yay multi-instances on unique history
#ifdef WASABI_COMPILE_CONFIG
  loadHistory(0);
#endif

  foreach(history) 
    StringW *s = history.getfor();
    if (!_wcsicmp(s->getValue(), txt)) {
      delete s;
      history.removeByPos(foreach_index);
      break;
    }
  endfor;

  history.addItem(new StringW(txt));

  while (history.getNumItems() > 64)
	{
    StringW *s = history.enumItem(0);
    delete s;
    history.removeByPos(1);
  }
#ifdef WASABI_COMPILE_CONFIG
  saveHistory();
  loadHistory(1);
#endif
}

#ifdef WASABI_COMPILE_CONFIG
void HistoryEditBox::loadHistory(int refill) {
  history.deleteAll();
  wchar_t d[256] = {0};
  wchar_t c[WA_MAX_PATH] = {0};
  int i;
  for (i=0;;i++) {
    StringCbPrintfW(d,sizeof(d),  L"%s_history_%d", getId(), i);
    WASABI_API_CONFIG->getStringPrivate(d, c, WA_MAX_PATH, L"");
    if (!*c)
      break;
    history.addItem(new StringW(c));
  }
  if (refill) {
    deleteAllItems();
    for (i=history.getNumItems()-1;i>=0;i--) {
      addItem(history.enumItem(i)->getValue());
    }
  }
}

void HistoryEditBox::saveHistory() {
  wchar_t d[256] = {0};
	int i;
  for (i=0;i<history.getNumItems();i++) {
    StringCbPrintfW(d, sizeof(d), L"%s_history_%d", getId(), i);
    WASABI_API_CONFIG->setStringPrivate(d, history.enumItem(i)->getValue());
  }
  StringCbPrintfW(d, sizeof(d), L"%s_history_%d", getId(), i);
  WASABI_API_CONFIG->setStringPrivate(d, L"");
}
#endif

int HistoryEditBox::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) {
  int r = HISTORYEDITBOX_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
  if (WCSCASEEQLSAFE(action, L"back")) history_back();
  if (WCSCASEEQLSAFE(action, L"forward")) history_forward();
  return r;
}