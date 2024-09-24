#include <precomp.h>
#include "xuiquerydrag.h"
#include <tataki/canvas/ifc_canvas.h>
#include <api/db/multiqueryserver.h>
#include <bfc/file/filename.h>

char QueryDragXuiObjectStr[] = "QueryDrag"; // This is the xml tag
char QueryDragXuiSvcName[] = "QueryDrag xui object"; // and this is the name of the service

XMLParamPair QueryDrag::params[] = {
  {QUERYDRAG_SETIMAGE, "image"},
  {QUERYDRAG_SETSOURCE, "source"},
	};
QueryDrag::QueryDrag() {
  setVirtual(0); // fucko
  myxuihandle = newXuiHandle();
	
	
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
  fn = NULL;
}

QueryDrag::~QueryDrag() {
}

int QueryDrag::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return QUERYDRAG_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) {
    case  QUERYDRAG_SETIMAGE:
      setImage(value);
      break;
    case  QUERYDRAG_SETSOURCE:
      setSource(value);
      break;
    default:
      return 0;
  }
  return 1;
}

int QueryDrag::onPaint(Canvas *canvas) {
  QUERYDRAG_PARENT::onPaint(canvas);

  RECT r;
  getClientRect(&r);

  RenderBaseTexture(canvas, r, 255);

  if (image.getBitmap()) 
    image.getBitmap()->stretchToRectAlpha(canvas, &r, getPaintingAlpha()); 

  return 1;
}

void QueryDrag::setImage(const char *elementname) {
  image = elementname; 
  if (isInited()) invalidate(); 
}

void QueryDrag::setSource(const char *elementname) {
  source = elementname;
}

int QueryDrag::getPreferences(int what) {
  switch (what) {
    case SUGGESTED_W:
      if (image.getBitmap()) return image.getBitmap()->getWidth();
    case SUGGESTED_H:
      if (image.getBitmap()) return image.getBitmap()->getHeight();
  }
  return QUERYDRAG_PARENT::getPreferences(what);
}

int QueryDrag::onMouseMove(int x, int y) {
  QUERYDRAG_PARENT::onMouseMove(x,y);
  if (isInClick()) 
    onBeginDrag();
  return 1;
}

void QueryDrag::onBeginDrag() {
  api_window *mqsw = NULL;
  if (source.isempty()) mqsw = findWindowByInterface(multiQueryServerGuid);
  else mqsw = findWindow(source);
  if (!mqsw) return;
  MultiQueryServer *mqs = static_cast<MultiQueryServer *>(mqsw->getInterface(multiQueryServerGuid));
  
  // multiquery is now available in mqs->mqs_getMultiQuery(); using format "table guid;query;table guid;query;etc..."
  fn = new FilenameI(StringPrintf("query://%s.nsq", mqs->mqs_getMultiQuery()));
  addDragItem(Filename::dragitem_getDatatype(), static_cast<Filename*>(fn));
  handleDrag();
}

int QueryDrag::dragComplete(int success) {
  ASSERT(fn != NULL);
  delete fn; fn = NULL;
  return 1;
}
