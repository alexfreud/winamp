#ifndef __COREACTIONS_H
#define __COREACTIONS_H

#include <api/service/svcs/svc_action.h>

class CoreActions : public svc_actionI {
  public :
    CoreActions();
    virtual ~CoreActions();

    static const char *getServiceName() { return "Core Actions"; }
    virtual int onActionId(int pvtid, const wchar_t *action, const wchar_t *param=NULL, int p1=0, int p2=0, void *data=NULL, int datalen=0, ifc_window *source=NULL);
    virtual const wchar_t *getHelp(int action);

  private:

    enum {
	    ACTION_PREV = 0,
	    ACTION_PLAY,
	    ACTION_PAUSE,
	    ACTION_STOP,
	    ACTION_NEXT,
	    ACTION_EJECT,
	    ACTION_EJECTURL,
	    ACTION_EJECTDIR,
	    ACTION_SEEK,
	    ACTION_VOLUME,
      ACTION_EQ_TOGGLE,
      ACTION_EQ_PREAMP,
      ACTION_EQ_BAND,
      ACTION_VOLUME_UP,
      ACTION_VOLUME_DOWN,
      ACTION_REWIND_5S,
      ACTION_FFWD_5S,
      ACTION_PLAY_CD,
      ACTION_EQ_AUTO = ACTION_PLAY_CD+16,
      ACTION_EQ_RESET,
      ACTION_PAN,
      ACTION_MUTE,
      ACTION_TOGGLE_REPEAT,
      ACTION_TOGGLE_SHUFFLE,
      ACTION_TOGGLE_CROSSFADER,
	  ACTION_PREFS,
    };
};

#endif
