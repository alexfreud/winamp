#ifndef __BOOKMARKLIST_H
#define __BOOKMARKLIST_H

#include <api/wnd/wndclass/guiobjwnd.h>

#define BOOKMARKLIST_PARENT GuiObjectWnd 

// -----------------------------------------------------------------------
class BookmarkList : public BOOKMARKLIST_PARENT {
  
  public:

    BookmarkList();
    virtual ~BookmarkList();

    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    void set(const wchar_t *elementname);

protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:

    enum {
      BOOKMARKLIST_SET = 0,
    };
		static XMLParamPair params[];
    int myxuihandle;
};

// -----------------------------------------------------------------------
extern const wchar_t BookmarkListXuiObjectStr[];
extern char BookmarkListXuiSvcName[];
class BookmarkListXuiSvc : public XuiObjectSvc<BookmarkList, BookmarkListXuiObjectStr, BookmarkListXuiSvcName> {};

#endif
