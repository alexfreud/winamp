#include <precomp.h>

#include "xuiqueryline.h"
#include <api/skin/widgets/db/xuiquerylist.h>

#define CB_SETQUERYLIST 0x1978

char QueryLineXuiObjectStr[] = "QueryLine"; // This is the xml tag
char QueryLineXuiSvcName[] = "QueryLine xui object";

ScriptQueryLine::ScriptQueryLine() {
  myxuihandle = newXuiHandle();
  addParam(myxuihandle, "querylist", QUERYLINE_SETQUERYLIST, XUI_ATTRIBUTE_IMPLIED);
  addParam(myxuihandle, "query", QUERYLINE_SETQUERY, XUI_ATTRIBUTE_IMPLIED);
  addParam(myxuihandle, "auto", QUERYLINE_SETAUTO, XUI_ATTRIBUTE_IMPLIED);
}

ScriptQueryLine::~ScriptQueryLine() { }

/*int ScriptQueryLine::onInit() {
  SCRIPTQUERYLINE_PARENT::onInit();
  if (!querylist_id.isempty())
    postDeferredCallback(CB_SETQUERYLIST, 0, 500);
  return 1;
}*/

int ScriptQueryLine::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return SCRIPTQUERYLINE_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) {
    case QUERYLINE_SETQUERYLIST:
      setXuiQueryList(value);
    break;
    case QUERYLINE_SETQUERY:
      ql_setQuery(value);
    break;
    case QUERYLINE_SETAUTO:
      setAuto(WTOI(value));
    break;
    default:
      return 0;
  }
  return 1;
}

void ScriptQueryLine::ql_setQuery(const char *q) {
  ensureConnected();
  setQuery(q);
}

void ScriptQueryLine::ensureConnected() {
  api_window *o = findWindow(querylist_id);
  if (!o) return;
  ScriptQueryList *querylist = static_cast<ScriptQueryList *>(o->getInterface(queryListGuid));
  if (!querylist) return;
  sqs_setMultiQueryServer(querylist);
}

void ScriptQueryLine::setXuiQueryList(const char *v) {
  querylist_id = v;
}

/*int ScriptQueryLine::onDeferredCallback(intptr_t p1, intptr_t p2) {
  switch (p1) {
    case CB_SETQUERYLIST:
      break;
    default:
      return SCRIPTQUERYLINE_PARENT::onDeferredCallback(p1, p2);
  }
  return 1;
}
*/