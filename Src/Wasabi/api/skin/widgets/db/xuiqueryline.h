#ifndef _XUIQUERYLINE_H
#define _XUIQUERYLINE_H
         
#include <api/skin/widgets/db/queryline.h>

#define SCRIPTQUERYLINE_PARENT QueryLine
class ScriptQueryLine : public SCRIPTQUERYLINE_PARENT {
public:
  ScriptQueryLine();
  virtual ~ScriptQueryLine();

  //virtual int onInit();

  virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);
  void setXuiQueryList(const char *v);

  //virtual int onDeferredCallback(intptr_t p1, intptr_t p2);

private:
  enum {
    QUERYLINE_SETQUERYLIST=1,
    QUERYLINE_SETQUERY,
    QUERYLINE_SETAUTO,
  };
  void ql_setQuery(const char *);
  void ensureConnected();
  int myxuihandle;
  String querylist_id;
};

extern char QueryLineXuiObjectStr[];
extern char QueryLineXuiSvcName[];
class QueryLineXuiSvc : public XuiObjectSvc<ScriptQueryLine, QueryLineXuiObjectStr, QueryLineXuiSvcName> {};

#endif
