#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_COMMANDS_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_COMMANDS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ifc_omservice;

// returns TRUE if command was handled (fSuccess will have result code if not NULL).

HRESULT Command_NavigateService(ifc_omservice *service, LPCWSTR pszUrl, BOOL fActiveOnly);
BOOL Command_ProcessService(HWND hView, ifc_omservice *service, INT commandId, BOOL *fSuccess); 
BOOL Command_ProcessView(HWND hView, INT commandId, BOOL *fSuccess); 
BOOL Command_ProcessGeneral(INT commandId, BOOL *fSuccess);

BOOL Command_ReportService(ifc_omservice *service);
BOOL Command_UnsubscribeService(ifc_omservice *service);
BOOL Command_ShowServiceInfo(ifc_omservice *service);
BOOL Command_ResetServicePolicy(ifc_omservice *service);
BOOL Command_ResetSubscription();
BOOL Command_SetServiceRating(ifc_omservice *service, INT rating);
HRESULT Command_CreateService(void);
BOOL Command_OpenServiceView(ifc_omservice *service);
BOOL Command_OpenServicePopup(ifc_omservice *service);

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_COMMANDS_HEADER