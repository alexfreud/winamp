#ifndef __PROGRESSGRID_H
#define __PROGRESSGRID_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include "xuigrid.h"
#include <api/syscb/callbacks/corecbi.h>

#define PROGRESSGRID_PARENT Grid

enum {
  PROGRESSGRID_TOP = 0,
  PROGRESSGRID_LEFT = 1,
  PROGRESSGRID_RIGHT = 2,
  PROGRESSGRID_BOTTOM = 3,
};

// -----------------------------------------------------------------------
class ProgressGrid : public PROGRESSGRID_PARENT, public CoreCallbackI {
  
  public:

    ProgressGrid();
    virtual ~ProgressGrid();

    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    virtual void setOrientation(const wchar_t *or);
    virtual void getGridRect(RECT *r);

    virtual void setProgress(float p); // 0..1

    virtual int onInit();
    virtual int corecb_onSeeked(int newpos);
    virtual int corecb_onStarted();
    virtual int corecb_onStopped();
    virtual void timerCallback(int id);
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:

    enum {
      PROGRESSGRID_SETORIENTATION = 0,
	  PROGRESSGRID_SETINTERVAL = 1,
    };
		static XMLParamPair params[];
    int orientation;
    int myxuihandle;
    float progress;
    int started;
	int update_interval;
};


// -----------------------------------------------------------------------
extern const wchar_t ProgressGridXuiObjectStr[];
extern char ProgressGridXuiSvcName[];
class ProgressGridXuiSvc : public XuiObjectSvc<ProgressGrid, ProgressGridXuiObjectStr, ProgressGridXuiSvcName> {};

#endif
