#include <precomp.h>

#include "cursor.h"

#define CBCLASS CursorI
START_DISPATCH;
  CB(CURSOR_GETOSHANDLE, getOSHandle);
END_DISPATCH;

#ifdef WASABI_COMPILE_SKIN

SkinCursor::SkinCursor(const wchar_t *elementid) {
  name = elementid;
  cursor = NULL;
  WASABI_API_SYSCB->syscb_registerCallback(static_cast<SkinCallbackI *>(this));
}

SkinCursor::SkinCursor() {
  WASABI_API_SYSCB->syscb_registerCallback(static_cast<SkinCallbackI *>(this));
  cursor = NULL;
}

SkinCursor::~SkinCursor() {
  WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SkinCallbackI *>(this));
}

OSCURSORHANDLE SkinCursor::getOSHandle() {
  if (cursor == NULL && !name.isempty()) {
    cursor = WASABI_API_SKIN->cursor_request(name);
  }
  return cursor;
}                  

int SkinCursor::skincb_onReset() {
  reset();
  return 1;
}

void SkinCursor::reset() {
  cursor = NULL;
}

void SkinCursor::setCursorElementId(const wchar_t *id) {
  name = id;
  reset();
}

#endif
