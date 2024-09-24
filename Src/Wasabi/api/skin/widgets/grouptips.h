#ifndef __GROUPTIPS_H
#define __GROUPTIPS_H

#include <api/service/svcs/svc_tooltips.h>
#include <bfc/ptrlist.h>
#include <api/timer/timerclient.h>

class ifc_window;

int groupTipsTimerProc(int id, void *data1, void *data2);

class GroupTips : public svc_toolTipsRendererI/*, public TimerClientI*/ {
public:
  GroupTips();
  virtual ~GroupTips();
  static const char *getServiceName() { return "Group tooltip renderer"; }
  
  virtual int spawnTooltip(const wchar_t *text);
private:
  ifc_window *tipwnd;
};

#endif