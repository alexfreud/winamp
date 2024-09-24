//#include "main.h"
//
//BOOL MyDialogExProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
//{
//    static CefRefPtr<MyCefClient> cef_client;
//
//    switch ( msg )
//    {
//        case WM_INITDIALOG:
//        {
//            CefMainArgs main_args( GetModuleHandle( nullptr ) );
//            CefSettings settings;
//            CefRefPtr<CefApp> app;
//            CefInitialize( main_args, settings, app, nullptr );
//
//            HWND cef_container_handle = GetDlgItem( hwnd, IDC_CEF_CONTAINER );
//
//            RECT rect;
//            GetClientRect( cef_container_handle, &rect );
//            CefRect cef_rect( rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top );
//
//            CefWindowInfo window_info;
//            window_info.SetAsChild( cef_container_handle, cef_rect );
//
//            cef_client = new MyCefClient( cef_container_handle );
//
//            // Create the browser window in the container control.
//            CefBrowserSettings browser_settings;
//            CefRefPtr<CefRequestContext> request_context;
//            CefRefPtr<CefDictionaryValue> extra_info;
//
//            CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync( window_info, cef_client.get(), FANZONE_BASE_URL, browser_settings, extra_info, request_context );
//
//
//            return TRUE;
//        }
//
//        case WM_DESTROY:
//        {
//            CefShutdown();
//            break;
//        }
//
//        default:
//            break;
//    }
//
//    return FALSE;
//}
