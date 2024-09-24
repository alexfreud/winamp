#include "precomp__gen_ff.h"
#include "fsmonitor.h"

#define tag L"wa5_fsmonitorclass"

LRESULT CALLBACK fsMonitorWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
extern HINSTANCE hInstance;
//------------------------------------------------------------------------
FullScreenMonitor::FullScreenMonitor()
{
    m_go_fs_timer_set = 0;
    m_cancel_fs_timer_set = 0;
    m_fs = 0;
    WNDCLASSW wc;
    if (!GetClassInfoW( hInstance, tag, &wc ))
    {
        MEMSET( &wc, 0, sizeof( wc ) );
        wc.lpfnWndProc = fsMonitorWndProc;
        wc.hInstance = hInstance;					// hInstance of DLL
        wc.lpszClassName = tag;			// our window class name
        wc.style = 0;

        int _r = RegisterClassW( &wc );
        ASSERTPR( _r, "cannot create fsmonitor wndclass" );
    }

    hWnd = CreateWindowExW( 0, tag, L"", 0, 0, 0, 1, 1, NULL, NULL, hInstance, NULL );

    ASSERT( hWnd );

    SetWindowLongPtrW( hWnd, GWLP_USERDATA, (LONG_PTR) this );

    APPBARDATA abd;

    abd.cbSize = sizeof( APPBARDATA );
    abd.hWnd = hWnd;
    abd.uCallbackMessage = APPBAR_CALLBACK;

    SHAppBarMessage( ABM_NEW, &abd );
}

//------------------------------------------------------------------------
FullScreenMonitor::~FullScreenMonitor()
{
    APPBARDATA abd;

    abd.cbSize = sizeof( APPBARDATA );
    abd.hWnd = hWnd;

    SHAppBarMessage( ABM_REMOVE, &abd );

    if (IsWindow( hWnd ))
        DestroyWindow( hWnd );
}

//------------------------------------------------------------------------
void FullScreenMonitor::registerCallback( FSCallback *cb )
{
    if (m_callbacks.haveItem( cb )) return;
    m_callbacks.addItem( cb );
}


//------------------------------------------------------------------------
void FullScreenMonitor::unregisterCallback( FSCallback *cb )
{
    m_callbacks.removeItem( cb );
}

//------------------------------------------------------------------------
void FullScreenMonitor::onGoFullscreen()
{
    if (m_cancel_fs_timer_set)
    {
        timerclient_killTimer( 2 );
        m_cancel_fs_timer_set = 0;
    }
    else
    {
        timerclient_setTimer( 1, 250 );
        m_go_fs_timer_set = 1;
    }
}

//------------------------------------------------------------------------
void FullScreenMonitor::onCancelFullscreen()
{
    if (m_go_fs_timer_set)
    {
        timerclient_killTimer( 1 );
        m_go_fs_timer_set = 0;
    }
    else
    {
        timerclient_setTimer( 2, 250 );
        m_cancel_fs_timer_set = 1;
    }
}

//------------------------------------------------------------------------
void FullScreenMonitor::sendGoFSCallbacks()
{
    foreach( m_callbacks );
    m_callbacks.getfor()->onGoFullscreen();
    endfor;
}

//------------------------------------------------------------------------
void FullScreenMonitor::sendCancelFSCallbacks()
{
    foreach( m_callbacks );
    m_callbacks.getfor()->onCancelFullscreen();
    endfor;
}

//------------------------------------------------------------------------
void FullScreenMonitor::timerclient_timerCallback( int id )
{
    if (id == 1)
    {
        timerclient_killTimer( 1 );
        m_go_fs_timer_set = 0;
        sendGoFSCallbacks();
    }
    else if (id == 2)
    {
        timerclient_killTimer( 2 );
        m_cancel_fs_timer_set = 0;
        sendCancelFSCallbacks();
    }
}

//------------------------------------------------------------------------
int FullScreenMonitor::wndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch (uMsg)
    {
        case APPBAR_CALLBACK:
        {
            switch (wParam)
            {
                case ABN_FULLSCREENAPP:
                    //DebugString("ABN_FULLSCREENAPP\n");
                    if (lParam && !m_fs)
                    {
                        m_fs = 1;
                        onGoFullscreen();
                    }
                    else if (!lParam && m_fs)
                    {
                        m_fs = 0;
                        onCancelFullscreen();
                    }
                    break;
            }
        }
    }
    return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

//------------------------------------------------------------------------
LRESULT CALLBACK fsMonitorWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
#ifdef WIN32
    FullScreenMonitor *gThis = (FullScreenMonitor *) GetWindowLongPtrW( hwnd, GWLP_USERDATA );
    if (!gThis)
        return DefWindowProc( hwnd, uMsg, wParam, lParam );
    else
        return gThis->wndProc( hwnd, uMsg, wParam, lParam );
#else
    return 0;
#endif
}

