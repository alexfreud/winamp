#ifndef _GRAD_H
#define _GRAD_H

#include <api/service/svcs/svc_imggen.h>
#include <bfc/draw/gradient.h>

class GradientImage : public svc_imageGeneratorI, public Gradient 
{
public:
  static const char *getServiceName() { return "Gradient image generator"; }
  virtual int testDesc(const wchar_t *desc);
  virtual ARGB32 *genImage(const wchar_t *desc, int *has_alpha, int *w, int *h, ifc_xmlreaderparams *params=NULL);
};

#endif
