#include <precomp.h>
#include "nakedobject.h"

NakedObject::NakedObject() {
  reentry_onresize = 0;
  reentry_onsetvisible = 0;
}

int NakedObject::getPreferences(int what) {
  return 0;
}

int NakedObject::onResize() {
  int rt = NAKEDOBJECT_PARENT::onResize();
  RECT r;
  getClientRect(&r);
  if (!reentry_onresize && r.left != r.right || r.top != r.bottom) {
    reentry_onresize = 1;
    resize(r.left, r.top, 0, 0);
    reentry_onresize = 0;
  }
  return rt;
}

void NakedObject::onSetVisible(int i) {
  NAKEDOBJECT_PARENT::onSetVisible(i);
  if (!i) return;
  if (!reentry_onsetvisible) {
    reentry_onsetvisible = 1;
    setVisible(0);
    reentry_onsetvisible = 0;
  }
}

