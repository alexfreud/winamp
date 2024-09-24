#ifndef _SVC_EVAL_H
#define _SVC_EVAL_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class NOVTABLE svc_evaluator : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::EVALUATOR; }

  const char *getEvalType();	// "php", "perl", "math", "xmlgen", etc.

// these are for future optimization -- BU
  //  void assignVar(const char *name, const char *value);
  //  const char *getVarValue(const char *name);
  //  int getVarIndex(const char *name);
  //  const char *getVarValueByIndex(int pos);

  int setEvalString(const char *string, int length, BOOL isBinary);
  // free the returned memory with api->sysFree()
  const char *evaluate(int *length, BOOL *isBinary);

protected:
  enum {
    GETEVALTYPE, ASSIGNVAR, GETVARVALUE, GETVARINDEX, GETVARVALUEBYINDEX,
    SETEVALSTRING, EVALUATE
  };
};

inline
const char *svc_evaluator::getEvalType() {
  return _call(GETEVALTYPE, (const char *)NULL);
}

inline
int svc_evaluator::setEvalString(const char *string, int length, BOOL isBinary){
  return _call(SETEVALSTRING, FALSE, string, length, isBinary);
}

inline
const char *svc_evaluator::evaluate(int *length, BOOL *isBinary) {
  return _call(EVALUATE, (const char *)NULL, length, isBinary);
}

// implementor derives from this one
class NOVTABLE svc_evaluatorI : public svc_evaluator {
public:
  virtual const char *getEvalType()=0;

//  void assignVar(const char *name, const char *value);
//  const char *getVarValue(const char *name);
//  int getVarIndex(const char *name);
//  const char *getVarValueByIndex(int pos);

  // implementor should make a copy of the string (if needed)
  virtual int setEvalString(const char *string, int length, BOOL isBinary)=0;
  // implementor should alloc returned mem w/ api->sysMalloc()
  virtual const char *evaluate(int *length, BOOL *isBinary)=0;

protected:
  RECVS_DISPATCH;
};

#endif
