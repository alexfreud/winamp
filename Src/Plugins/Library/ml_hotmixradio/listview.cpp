/*
** Copyright  (C) 2003 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
*/

#include <windows.h>
#include <commctrl.h>
#include "listview.h"

#ifdef GEN_ML_EXPORTS
#include "main.h" // for getting the font
#include "config.h"
#endif
//  bp Comment:  all the calls beginning   "ListView_"  are 
//  MACROs defined in commctrl.h 

void W_ListView :: AddCol (char *text, int w)
{
  LVCOLUMN lvc={0,};
  lvc.mask = LVCF_TEXT|LVCF_WIDTH;
  lvc.pszText = text;
  if  (w) lvc.cx=w;
  ListView_InsertColumn (m_hwnd, m_col, &lvc);   
  m_col++;
}

int W_ListView::GetColumnWidth (int col)
{
  if  (col < 0 || col >= m_col) return 0;
  return ListView_GetColumnWidth (m_hwnd, col);
}


int W_ListView::GetParam (int p)
{
  LVITEM lvi={0,};
  lvi.mask = LVIF_PARAM;
  lvi.iItem = p;
  ListView_GetItem (m_hwnd, &lvi);
  return lvi.lParam;
}

int W_ListView::InsertItem (int p, char *text, int param)
{
  LVITEM lvi={0,};
  lvi.mask = LVIF_TEXT | LVIF_PARAM;
  lvi.iItem = p;
  lvi.pszText = text;
  lvi.cchTextMax=strlen (text);
  lvi.lParam = param;
  return ListView_InsertItem (m_hwnd, &lvi);
}


void W_ListView::SetItemText (int p, int si, char *text)
{
  LVITEM lvi={0,};
  lvi.iItem = p;
  lvi.iSubItem = si;
  lvi.mask = LVIF_TEXT;
  lvi.pszText = text;
  lvi.cchTextMax = strlen (text);
  ListView_SetItem (m_hwnd, &lvi);
}

void W_ListView::SetItemParam (int p, int param)
{
  LVITEM lvi={0,};
  lvi.iItem = p;
  lvi.mask=LVIF_PARAM;
  lvi.lParam=param;
  ListView_SetItem (m_hwnd, &lvi);
}

void W_ListView::refreshFont ()
{
  if  (m_font) 
  {
    DeleteFont (m_font); 
    SetWindowFont (m_hwnd, NULL, FALSE);
  }
  m_font = NULL;

  HWND h;
#ifdef GEN_ML_EXPORTS
  h=g_hwnd;
#else
  h=m_libraryparent;
#endif
  if  (h && m_allowfonts) 
  {
    int a=SendMessage (h, WM_USER+0x1000 /*WM_ML_IPC*/,66, 0x0600 /*ML_IPC_SKIN_WADLG_GETFUNC*/);
    if  (a)
    {
      m_font= (HFONT)a;
      SetWindowFont (m_hwnd, m_font, FALSE);
    }
  } 
  InvalidateRect (m_hwnd, NULL, TRUE);
}

void W_ListView::setallowfonts (int allow)
{
  m_allowfonts=allow;
}

void W_ListView::setwnd (HWND hwnd) 
{ 
  m_hwnd = hwnd;
  if  (hwnd)
  {
    ListView_SetExtendedListViewStyle  (hwnd, LVS_EX_FULLROWSELECT|LVS_EX_UNDERLINEHOT ); 
    refreshFont ();
  }
}
