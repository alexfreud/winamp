#ifndef _SVC_TOOLTIPS_H
#define _SVC_TOOLTIPS_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class NOVTABLE svc_toolTipsRenderer : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::TOOLTIPSRENDERER; }

  int spawnTooltip(const wchar_t *text);

  enum {
    SPAWNTOOLTIP        =10,
  };
};

inline int svc_toolTipsRenderer::spawnTooltip(const wchar_t *text) {
  return _call(SPAWNTOOLTIP, 0, text);
}

class NOVTABLE svc_toolTipsRendererI : public svc_toolTipsRenderer {
public:
  virtual int spawnTooltip(const wchar_t *text)=0;

protected:
  RECVS_DISPATCH;
};

#endif
