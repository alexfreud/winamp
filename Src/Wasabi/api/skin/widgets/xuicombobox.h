#ifndef __XUICOMBOBOX_H
#define __XUICOMBOBOX_H

#include <api/skin/widgets/combobox.h>

extern const wchar_t ComboBoxXuiObjectStr[];
extern char ComboBoxXuiSvcName[];
class ComboBoxXuiSvc : public XuiObjectSvc<ComboBox, ComboBoxXuiObjectStr, ComboBoxXuiSvcName> {};

#endif
