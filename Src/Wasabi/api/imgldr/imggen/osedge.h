#ifndef _OSEDGE_H
#define _OSEDGE_H

#include <api/service/svcs/svc_imggen.h>

class OsEdgeImage : public svc_imageGeneratorI {
public:
  static const char *getServiceName() { return "OS Edge image generator"; }
  virtual int testDesc(const wchar_t *desc);
  virtual ARGB32 *genImage(const wchar_t *desc, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params=NULL);
};

#endif
