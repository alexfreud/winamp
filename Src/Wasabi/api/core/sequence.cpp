#include "precomp.h"

#include "sequence.h"

#define CBCLASS ItemSequencerI
START_DISPATCH;
  CB(GETDEPENDENCYPTR, getDependencyPtr);
  CB(GETNEXTPLAYITEM, getNextPlayItem);
  CB(GETCURRENTPLAYBACKNUMBER, getCurrentPlaybackNumber);
  CB(GETNUMITEMS, getNumItems);
  CB(REWIND, rewind);
  CB(FORWARD, forward);
  CB(ONNOTIFY, onNotify);
END_DISPATCH;
#undef CBCLASS

int ItemSequencerI::onNotify(int msg, int param1, int param2) {
  switch (msg) {
    case SEQNOTIFY_ONREGISTER: return onRegister();
    case SEQNOTIFY_ONDEREGISTER: return onDeregister();
    case SEQNOTIFY_ONNEXTFILE: return onNextFile();
    case SEQNOTIFY_ONTITLECHANGE: return onTitleChange();
    case SEQNOTIFY_ONSTARTED: return onStarted();
    case SEQNOTIFY_ONSTOPPED: return onStopped();
    case SEQNOTIFY_ONPAUSED: return onPaused();
    case SEQNOTIFY_ONUNPAUSED: return onUnpaused();
  }
  return 0;
}

const char *ListSequencer::getNextPlayItem() {
  int pos;
  const char *ret;

  pos = getCurrent();

  if (pos < 0) return NULL;

  ret = enumItem(pos);

  setCurrent(pos);

  return ret;
}

int ListSequencer::rewind() {

  int pos;

  pos = getCurrent();

  if (pos < 0) return 0;

  pos--;

  if (pos < 0) {
    if (loop()) {
      pos = getNumEntries()-1;
    } else {
      pos++;
    }
  }

  setCurrent(pos);
  return 1;
}

int ListSequencer::forward() {

  int pos;

  pos = getCurrent();

  if (pos < 0) return 0;

  pos++;

  if (pos >= getNumEntries()) {
    if (loop()) {
      pos = 0;
    } else {
      return 0;
    }
  }

  setCurrent(pos);
  return 1;
}

int ListSequencer::getNumItems() {
  return getNumEntries();
}
