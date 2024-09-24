#ifndef __THEMESLIST_H
#define __THEMESLIST_H

#include "api/wnd/wndclass/listwnd.h"
#include "main.h"
//#include "../Components/wac_downloads/wac_downloads_download_manager.h"
#include "../../../../Components/wac_network/wac_network_http_receiver_api.h"
#include "../nu/AutoWide.h"
#include "api/script/objects/systemobj.h"

#define DOWNLOADSLIST_PARENT ListWnd

// -----------------------------------------------------------------------
class DownloadsList : public DOWNLOADSLIST_PARENT
{
  
  public:

    DownloadsList();
    virtual ~DownloadsList();

    virtual int onInit();
    virtual void onDoubleClick(int itemnum);
    virtual int onPaint(Canvas *canvas);
    virtual int onRightClick(int itemnum);
	virtual int onColumnDblClick(int col, int x, int y);
	virtual int onColumnLabelClick(int col, int x, int y);

    virtual int onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source);
    virtual int onResize();
    virtual int wantResizeCols() { return 0; }
    virtual int setXuiParam(int _xuihandle, int xmlattrid, const wchar_t *name, const wchar_t *value);
    virtual int wantHScroll() { return !nohscroll; }

 //   virtual int getTextBold(LPARAM lParam);
    virtual void onSetVisible(int show);

	// Callbacks from MediaDownloader
	static void onDownloadStart(const char *url, DownloadToken token);
	static void onDownloadTick(DownloadToken token);
	static void onDownloadEnd(DownloadToken token, const wchar_t * filename);
	static void onDownloadError(DownloadToken token, int code);
	static void onDownloadCancel(DownloadToken token);

    enum {
      CTLIST_NOHSCROLL = 0,
    };

protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:
    static XMLParamPair params[];
    int xuihandle;
    int nohscroll;
    int ensure_on_paint;

	void newItem(int status, DownloadToken token);

	static PtrList<void> activeDownloads;
	static PtrList<DownloadsList> skinObjects;

	enum DOWNLOAD_STATUS
	{
		STATUS_WAITING      = 0,
		STATUS_TRANSFERRING = 1,
		STATUS_FINISHED     = 2,
		STATUS_ERROR        = -1,
		STATUS_CANCEL       = -2,
	};
};

// -----------------------------------------------------------------------
extern const wchar_t DownloadsListXuiObjectStr[];
extern char DownloadsListXuiSvcName[];
class DownloadsListXuiSvc : public XuiObjectSvc<DownloadsList, DownloadsListXuiObjectStr, DownloadsListXuiSvcName> {};

#endif
