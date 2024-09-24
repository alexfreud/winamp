#ifndef _SVC_ACTION_H
#define _SVC_ACTION_H

#include <bfc/dispatch.h>
#include <bfc/string/bfcstring.h>
#include <bfc/ptrlist.h>

#include <api/service/services.h>

class ifc_window;

class NOVTABLE svc_action : public Dispatchable {
protected:
  svc_action() { }

public:
  static FOURCC getServiceType() { return WaSvc::ACTION; }
  
  int hasAction(const wchar_t *name);
  int onAction(const wchar_t *action, const wchar_t *param=NULL, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, ifc_window *source=NULL);

  enum {
    HASACTION=10,
    ONACTION=20,
  };

};

inline int svc_action::hasAction(const wchar_t *name) {
  return _call(HASACTION, 0, name);
}

inline int svc_action::onAction(const wchar_t *action, const wchar_t *param, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) {
  return _call(ONACTION, 0, action, param, p1, p2, data, datalen, source);
}

class ActionEntry {
  public:
    ActionEntry(const wchar_t *_action, int _id) : action(_action), id(_id) {}
    virtual ~ActionEntry() { }

    const wchar_t *getAction() { return action; }
    int getId() { return id; }

  private:
    StringW action;
    int id;
};

class SortActions {
public:
  static int compareItem(ActionEntry *p1, ActionEntry *p2) {
    return WCSICMP(p1->getAction(), p2->getAction());
  }
  static int compareAttrib(const wchar_t *attrib, ActionEntry *item) {
    return WCSICMP(attrib, item->getAction());
  }
};

class NOVTABLE svc_actionI : public svc_action {
public:
    virtual ~svc_actionI();
    void registerAction(const wchar_t *actionid, int pvtid);
    virtual int onActionId(int pvtid, const wchar_t *action, const wchar_t *param=NULL, int p1=0, int p2=0, void *data=NULL, int datalen=0, ifc_window *source=NULL)=0;

protected:
  virtual int hasAction(const wchar_t *name);
  virtual int onAction(const wchar_t *action, const wchar_t *param=NULL, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, ifc_window *source=NULL);

  PtrListQuickSorted<ActionEntry, SortActions> actions;

  RECVS_DISPATCH;
};
#include <api/service/servicei.h>
template <class T>
class ActionCreator : public waServiceFactoryT<svc_action, T> {};
template <class T>
class ActionCreatorSingle : public waServiceFactoryTSingle<svc_action, T> {
public:
  svc_action *getHandler() {
    return waServiceFactoryT<svc_action, T>::getSingleService();
  }
};

#include <api/service/svc_enum.h>
#include <bfc/string/StringW.h>

class ActionEnum : public SvcEnumT<svc_action> {
public:
  ActionEnum(const wchar_t *_action) : action(_action) { }
protected:
  virtual int testService(svc_action *svc) {
    return (!action.isempty() && svc->hasAction(action));
  }
private:
  StringW action;
};

class FireAction {
public:
  enum {
    ACTION_NOT_HANDLED = 0x80000000
  };
  /**
    Fire a named action out into the system with the given parameters.

    This method will only send the action to the first registered handler for that action.

    This prevents the action from being overridden or handled by newer wacs.

    The content and syntax of the generalized params are defined by the handler of the action string.

    Read: Using Wasabi: General Development: Actions
      
    @see                svc_actionI
    @param action       The action string.
    @param param        A string parameter to the action.
    @param p1           The first integer parameter to the action.
    @param p2           The second integer parameter to the action.
    @param data         An untyped data buffer parameter to the action.
    @param datalen      The size in bytes of the data buffer parameter.
    @param source       A window object that can be given as the source object, if the action handler is expecting one.  Actions bound to guiobjects use that guiobject's rootwnd pointer as the source.
    @param apply_to_all Send the action to everyone.  (If false only sends to first registered)
  */
  FireAction(const wchar_t *action, const wchar_t *param = NULL, intptr_t p1 = 0, intptr_t p2 = 0, void *data = NULL, size_t datalen = 0, ifc_window *source = NULL, int apply_to_all = TRUE) {
    lastretval = ACTION_NOT_HANDLED;
    ActionEnum ae(action);
    svc_action *act;
    while ((act = ae.getNext()) != NULL) {
      lastretval = act->onAction(action, param, p1, p2, data, datalen, source);
      ae.release(act);
      if (!apply_to_all) break;
    }
  }
  /**
    More robust retval handling is needed.

    I ought to be grabbing all of the return values into a list an exposing that.
    
    Later.

    Read: Using Wasabi: General Development: Actions
      
    @see                svc_actionI
    @ret                The return code of the action sent.
  */
  int getLastReturnValue() {
    return lastretval;
  }
private:
  int lastretval;
};

#endif
