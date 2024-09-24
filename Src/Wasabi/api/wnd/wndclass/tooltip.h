#ifndef __TOOLTIP_H
#define __TOOLTIP_H

#include <api/service/svcs/svc_tooltips.h>

class Tooltip {

  public:

    Tooltip(const wchar_t *txt);
    virtual ~Tooltip();

  private:

    svc_toolTipsRenderer *svc;
};

#endif
