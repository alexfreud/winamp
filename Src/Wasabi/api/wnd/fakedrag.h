#ifndef _FAKEDRAG_H
#define _FAKEDRAG_H

#include <api/wnd/basewnd.h>

class FakeDragWnd : public BaseWnd {
public:
  FakeDragWnd() { dragging = 1; }
  ~FakeDragWnd() { dragging = 0; }
};

#endif
