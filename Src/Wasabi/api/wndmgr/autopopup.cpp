#include <precomp.h>
#include "autopopup.h"

static PtrListQuickSorted<AutoPopupEntry, AutoPopupEntrySort> entries;
static int nid=0;

int AutoPopup::registerGuid(int skinpartid, GUID g, const wchar_t *desc, const wchar_t *prefered_container, int required) 
{
  if (desc == NULL || !*desc) desc = L"[????]";
  AutoPopupEntry *ape = new AutoPopupEntry(skinpartid, g, NULL, desc, prefered_container, required);
  entries.addItem(ape);
  return ape->getNid();
}

int AutoPopup::registerGroupId(int skinpartid, const wchar_t *groupid, const wchar_t *desc, const wchar_t *prefered_container, int required) {
  if (desc == NULL || !*desc) desc = L"[????]";
  AutoPopupEntry *ape = new AutoPopupEntry(skinpartid, INVALID_GUID, groupid, desc, prefered_container, required);
  entries.addItem(ape);
  return ape->getNid();
}

int AutoPopup::getNumItems() { return entries.getNumItems(); }
AutoPopupEntry *AutoPopup::enumItem(int n) { return entries.enumItem(n); }

int AutoPopup::getNumGroups() {
  int n = 0;
  foreach(entries)
    AutoPopupEntry *e = entries.getfor();
    if (e->getGroupId() != NULL && e->getGuid() == INVALID_GUID) n++;
  endfor;
  return n;
}

int AutoPopup::getNumGuids() {
  int n = 0;
  foreach(entries)
    AutoPopupEntry *e = entries.getfor();
    if (e->getGroupId() == NULL && e->getGuid() != INVALID_GUID) n++;
  endfor;
  return n;
}

const wchar_t *AutoPopup::enumGroup(int n) {
  int c = 0;
  foreach(entries)
    AutoPopupEntry *e = entries.getfor();
    if (e->getGroupId() != NULL && e->getGuid() == INVALID_GUID) {
      if (c == n) return e->getGroupId();
      c++;
    }
  endfor;
  return NULL;
}

GUID AutoPopup::enumGuid(int n) {
  int c = 0;
  foreach(entries)
    AutoPopupEntry *e = entries.getfor();
    if (e->getGroupId() == NULL && e->getGuid() != INVALID_GUID) {
      if (c == n) return e->getGuid();
      c++;
    }
  endfor;
  return INVALID_GUID;
}

const wchar_t *AutoPopup::enumGroupDescription(int n) {
  int c = 0;
  foreach(entries)
    AutoPopupEntry *e = entries.getfor();
    if (e->getGroupId() != NULL && e->getGuid() == INVALID_GUID) {
      if (c == n) return e->getDescription();
      c++;
    }
  endfor;
  return NULL;
}

const wchar_t *AutoPopup::enumGuidDescription(int n) {
  int c = 0;
  foreach(entries)
    AutoPopupEntry *e = entries.getfor();
    if (e->getGroupId() == NULL && e->getGuid() != INVALID_GUID) {
      if (c == n) return e->getDescription();
      c++;
    }
  endfor;
  return NULL;
}


AutoPopupEntry *AutoPopup::getByDesc(const wchar_t *desc) {
  foreach(entries)
    if (!WCSICMP(desc, entries.getfor()->getDescription()))
      return entries.getfor();
  endfor;
  return NULL;
}

void AutoPopup::reset() { entries.deleteAll(); nid = 0; }

const wchar_t *AutoPopup::getDefaultContainerParams(const wchar_t *groupid, GUID g, int *flag) {
  if (groupid == NULL) { // guid
    for (int i=entries.getNumItems()-1;i>=0;i--) {
      if (entries.enumItem(i)->getGuid() == g) {
        if (flag != NULL) *flag = entries.enumItem(i)->getContainerHow();
        return entries.enumItem(i)->getPreferedContainer();
      }
    }
  } else { // groupid
    for (int i=entries.getNumItems()-1;i>=0;i--) 
		{
      if (WCSCASEEQLSAFE(entries.enumItem(i)->getGroupId(), groupid)) 
			{
        if (flag != NULL) *flag = entries.enumItem(i)->getContainerHow();
        return entries.enumItem(i)->getPreferedContainer();
      }
    }
  }
  return NULL;
}

void AutoPopup::unregister(int id) {
  for (int i=0;i<entries.getNumItems();i++) {
    AutoPopupEntry *ape = entries.enumItem(i);
    if (ape->getNid() == id) {
      delete ape;
      entries.removeByPos(i);
      i--;
      continue;
    }
  }
}

void AutoPopup::removeSkinPart(int id) {
  for (int i=0;i<entries.getNumItems();i++) {
    AutoPopupEntry *ape = entries.enumItem(i);
    if (ape->getSkinpart() == id) {
      delete ape;
      entries.removeByPos(i);
      i--;
    }
  }
}

int AutoPopup::allocNid() { return nid++; }

void AutoPopup::removeAllAddons() {
  for (int i=0;i<entries.getNumItems();i++) {
    AutoPopupEntry *ape = entries.enumItem(i);
    if (ape->getSkinpart() != SKINPARTID_NONE) {
      delete ape;
      entries.removeByPos(i);
      i--;
    }
  }
}
