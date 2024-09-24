#include <lib/std.mi>

System.onScriptLoaded() {
  group sg = getScriptGroup();
  string params = getParam();
  int offset = 0;

  for (int a = 0; getToken(params,",",a) != ""; a++ )
  {
    guiobject tmp = sg.getObject(getToken(params,",",a));
    tmp.setXMLparam("x",integertostring(offset));
    offset += tmp.getAutoWidth();
  }
}