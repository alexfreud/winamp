/*
** Copyright (C) 2003 Nullsoft, Inc.
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

#ifndef _CHILDWND_H_
#define _CHILDWND_H_

typedef struct {
  int id;
  int type;			// 0xLTRB
  RECT rinfo;
} ChildWndResizeItem;

/* If you are including this file from a plugin, you can't call these functions directly.  they are here to help you create the function pointer typedefs */
void childresize_init(HWND hwndDlg, ChildWndResizeItem *list, int num);
void childresize_resize(HWND hwndDlg, ChildWndResizeItem *list, int num);
void childresize_resize2(HWND hwndDlg, ChildWndResizeItem *list, int num, BOOL fRedraw = FALSE, HRGN rgnUpdate = NULL);

//extended versions we use so we can modify the list before actually setting it
void childresize_resize_to_rectlist(HWND hwndDlg, ChildWndResizeItem *list, int num, RECT *rectout);
void childresize_resize_from_rectlist(HWND hwndDlg, ChildWndResizeItem *list, int num, RECT *rectin);


#endif//_CHILDWND_H_