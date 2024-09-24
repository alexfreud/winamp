#ifndef __XUIHISTORYEDITBOX_H
#define __XUIHISTORYEDITBOX_H

#include <api/skin/widgets/historyeditbox.h>

extern const wchar_t HistoryEditXuiObjectStr[];
extern char HistoryEditXuiSvcName[];
class HistoryEditXuiSvc : public XuiObjectSvc<HistoryEditBox, HistoryEditXuiObjectStr, HistoryEditXuiSvcName> {};

#endif
