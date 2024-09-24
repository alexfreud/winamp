#ifndef __XUIDROPDOWNLIST_H
#define __XUIDROPDOWNLIST_H

#include <api/skin/widgets/dropdownlist.h>

extern const wchar_t DropDownListXuiObjectStr[];
extern char DropDownListXuiSvcName[];
class DropDownListXuiSvc : public XuiObjectSvc<DropDownList, DropDownListXuiObjectStr, DropDownListXuiSvcName> {};

#endif
