#ifndef _VIEW_MB_H_
#define _VIEW_MB_H_

#include "main.h"
#include "contnr.h"
#include "evntsink.h"
#include "childwnd.h"
#include "resource.h"
#include "..\winamp\wa_dlg.h"
#include <shlobj.h>
#include "mbutil.h"
#include "config.h"
extern void gracenoteCancelRequest();
// ---------------------------------------------------------------
// class viewMBHandler is a singleton class -
// All public members are static, including
// viewMBHandler :: DialogProc.   
//  CTOR and DTOR are public (and not static)
//  the  only instance of viewMBHandler is private,
//  theVmb;
// ---------------------------------------------------------
// ---------------------------------------------------------
//  this class has the basic responsibiliity of creating (and
//  handling the messages for) an IEControl window.
//  It is a  prototype for the Advertising window,
//  which will be a similar class, but without the extraneous
//  stuff.  ie  There are currently navigation buttons for which 
//  messages must be handled.  These should disappear 
//  in the final class,  along with the gracenote stuff.
// 
//  Ben Pontius
// ---------------------------------------------------------------
class viewMBHandler
{
public:
	// ---------------------------------------------------------------
	friend BOOL view_mbDialogProc (HWND hwndDlg,  UINT uMsg,  WPARAM wParam, LPARAM lParam);
private:	
	// ---------------------------------------------------------------
	// CTOR
	// ---------------------------------------------------------------
	viewMBHandler ()
	: m_hwnd (0),
	m_pweb (0),
	m_contnr (0),
	m_event (0),
	m_eventCookie (0),
	m_defurl (1),
	m_tmpurl (0),
	loc_oldWndProc (0)
	{
	}
	// ---------------------------------------------------------------
	// DTOR  - public (because the compiler demanded it.)
	// ---------------------------------------------------------------
	~viewMBHandler ()
	{
		// ---------------------------------------------------------------
		gracenoteCancelRequest ();	
		// ---------------------------------------------------------------
		if (m_contnr != 0)
		{
			destroyIEControl ();
		}
		if (m_tmpurl)
		{
			free (m_tmpurl);
			m_tmpurl = 0;
		}
		m_hwnd  =  0;
	}
public:
	// ---------------------------------------------------------------
	static void Refresh (int defurl);
	// ---------------------------------------------------------------
	static void SetDesc (char *desc);
	// ---------------------------------------------------------------
	static void Navigate (char *s);
	// ---------------------------------------------------------------
	static void navigateGracenoteTuid ();      
	// ---------------------------------------------------------------
	//   the central (CALLBACK) function
	// ---------------------------------------------------------------
	static BOOL CALLBACK DialogProc (HWND hwndDlg,  UINT uMsg,  WPARAM wParam, LPARAM lParam);
	// ---------------------------------------------------------------
	static BOOL CALLBACK newWndProc (HWND hwndDlg,  UINT uMsg,  WPARAM wParam, LPARAM lParam);
	// ---------------------------------------------------------------
	static void ContextMenu (HWND parent);
	// ---------------------------------------------------------------
	static viewMBHandler &getInstance ()
	{
		if (0 == pVmb)
		{
			pVmb = new viewMBHandler;
		}
		return *pVmb;
	}

protected:
	// ---------------------------------------------------------------
	IConnectionPoint *GetConnectionPoint(REFIID riid);
	// ---------------------------------------------------------------
	void ConnectEvents ();
	// ---------------------------------------------------------------
	//  bp IEControl creation:
	// ---------------------------------------------------------------
	void createIEControl ();
	// ---------------------------------------------------------------
	// BP - IEControl destruction
	// ---------------------------------------------------------------
	void destroyIEControl ();
	// ---------------------------------------------------------------
	void NavigateToName (LPCTSTR pszUrl);

private:
	
	IWebBrowser2 *m_pweb;                
	HWND m_hwnd;                           
	CContainer *m_contnr;  
	CEventSink *m_event;                 
	DWORD m_eventCookie;                 
	int m_defurl;
	char *m_tmpurl;                      
	WNDPROC loc_oldWndProc;
	static viewMBHandler *pVmb;		

};

#endif // _VIEW_MB_H_
