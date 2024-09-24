#ifndef __QUERYDRAG_H
#define __QUERYDRAG_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <tataki/bitmap/autobitmap.h>

class FilenameI;

#define  QUERYDRAG_PARENT GuiObjectWnd 

// -----------------------------------------------------------------------
// Your wnd object class

class QueryDrag : public  QUERYDRAG_PARENT {
  
  public:

    QueryDrag();
    virtual ~QueryDrag();

    virtual int onPaint(Canvas *c); 
    virtual int getPreferences(int what); 
    virtual int onMouseMove(int x, int y);

    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    void setImage(const char *elementname);
    void setSource(const char *elementname);
    void onBeginDrag();
    virtual int dragComplete(int success);

  private:

    AutoSkinBitmap image;
    String source;

    FilenameI *fn;

    enum {
       QUERYDRAG_SETIMAGE = 0,
       QUERYDRAG_SETSOURCE,
    };
		static XMLParamPair params[];
    int myxuihandle;
};


// -----------------------------------------------------------------------
extern char QueryDragXuiObjectStr[];
extern char QueryDragXuiSvcName[];
class QueryDragXuiSvc : public XuiObjectSvc<QueryDrag, QueryDragXuiObjectStr, QueryDragXuiSvcName> {};

#endif
