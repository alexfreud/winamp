#include <precomp.h>

#include "svc_eval.h"

#define CBCLASS svc_evaluatorI
START_DISPATCH
  CB(GETEVALTYPE, getEvalType);
  CB(SETEVALSTRING, setEvalString);
  CB(EVALUATE, evaluate);
END_DISPATCH
#undef CBCLASS
