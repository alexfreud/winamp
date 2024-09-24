#include <precomp.h>

#include "queryline.h"

#include <api/api.h>
#include <api/db/metatags.h>
#include <bfc/string/string.h>
#include <bfc/parse/pathparse.h>

QueryLine::QueryLine(const char *query) : autoquery(FALSE), querytext(query), autofield(MT_NAME) {
}

void QueryLine::setQuery(const char *query) {
  querytext = query;
  String sp = querytext;
  if (autoquery && !querytext.isempty()) {
    sp = "";
    PathParser pp(querytext, " ");
    if (pp.getNumStrings() <= 0) return;
    for (int i = 0; i < pp.getNumStrings(); i++) {
      if (i != 0) sp += " and ";
      sp += StringPrintf("(\"%s\" has \"%s\")", autofield.getValue(), pp.enumString(i));
    }
  }
  sqs_setQuery(sp);
}

int QueryLine::setAuto(int bv) {
  autoquery = !!bv;
  setQuery(querytext);
  return 1;
}
