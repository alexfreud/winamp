#ifndef NULLSOFT_WEBDEV_PLUGIN_COMMANDS_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_COMMANDS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef LPVOID HNAVITEM;
class ifc_omservice;

HRESULT Command_NavigateService(ifc_omservice *service, LPCWSTR pszUrl, BOOL fActiveOnly);
HRESULT Command_PostNavigateSvc(ifc_omservice *service, LPCWSTR pszUrl, BOOL fActiveOnly);
HRESULT Command_EditService(ifc_omservice *service);
HRESULT Command_ReloadService(ifc_omservice *service);
HRESULT Command_ResetPermissions(ifc_omservice *service);
HRESULT Command_LocateService(ifc_omservice *service);
HRESULT Command_EditServiceExternal(ifc_omservice *service);
HRESULT Command_DeleteItem(HNAVITEM hItem);
HRESULT Command_DeleteAll();
HRESULT Command_CreateService(void);
HRESULT Command_OpenView(HNAVITEM hItem);
HRESULT Command_NewWindow(HNAVITEM hItem);
HRESULT Command_ShowBrowserOptions(void);

BOOL CommandManager_Process(HNAVITEM hItem, ifc_omservice *service, UINT commandId);

#endif //NULLSOFT_WEBDEV_PLUGIN_COMMANDS_HEADER