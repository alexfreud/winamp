/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#if 0 // no more minibrowser

static int listDragging=0,listSel=-1;

static WNDPROC OldBookListProc;

static BOOL CALLBACK BookListProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_LBUTTONDOWN:
			if (!listDragging) 
      {
				POINT p;
				RECT r;
				GetCursorPos(&p);
				GetWindowRect(hwndDlg,&r);
        if (p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom)
        {
				  int x=SendMessageW(hwndDlg,LB_ITEMFROMPOINT,0,MAKELPARAM(p.x-r.left,p.y-r.top-2));
				  if (!HIWORD(x))
				  {
					  listDragging=1;
					  listSel=x;
				  }
        }
			}
    break;
//    case WM_KILLFOCUS:
    case WM_LBUTTONUP:
			listDragging=0;
			listSel=-1;
    break;
  }
  return CallWindowProc(OldBookListProc,hwndDlg,uMsg,wParam,lParam);
}

static char *g_bmedit_fn, *g_bmedit_ft;

static BOOL CALLBACK BookMarkEditProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      SetDlgItemText(hwndDlg,IDC_TITLE,g_bmedit_ft);
      SetDlgItemText(hwndDlg,IDC_FILE,g_bmedit_fn);
    return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
          GetDlgItemText(hwndDlg,IDC_TITLE,g_bmedit_ft,4095);
          GetDlgItemText(hwndDlg,IDC_FILE,g_bmedit_fn,MAX_PATH);
        case IDCANCEL:
          EndDialog(hwndDlg,0);
        return 0;
      }
      return 0;
  }
  return 0;
}

static BOOL CALLBACK BookProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{	
//	hi helpinfo[]={
//	};
//	DO_HELP();
	switch (uMsg)
	{
		case WM_CTLCOLORLISTBOX:
			if(listDragging)
			{
						POINT p;
						RECT r;
            int thisp;
						GetCursorPos(&p);
						GetWindowRect(GetDlgItem(hwndDlg,IDC_SELBOX),&r);
						thisp=SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_ITEMFROMPOINT,0,MAKELPARAM(p.x-r.left,p.y-r.top-2));
						if(HIWORD(thisp)) 
						{
							// mouse pointer outside client area
							thisp=LOWORD(thisp);
						}
						if (listSel != -1 && listSel != thisp)
						{
              int len=SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCOUNT,0,0);
							// move listSel to thisp.
              if (thisp >= 0 && listSel >= 0 && thisp < len && listSel < len) {
                char fn[MAX_PATH] = {0};
                char file1[MAX_PATH] = {0}, title1[4096] = {0};
                FILE *fp,*fpo;
                Bookmark_getfn(fn);
                fp=fopen(fn,"rt");
            
                fpo=fopen(TEMP_FILE,"wt");
                if (fp&&fpo)
                {
                  char ft[4096] = {0};
                  int x=0;
                  while (1)
                  {
                    if (x == listSel)
                    {
                      fgets(file1,MAX_PATH,fp);
                      fgets(title1,4096,fp);
                    }
                    else
                    {
                      fgets(fn,MAX_PATH,fp);
                      fgets(ft,4096,fp);
                    }                    
                    if (feof(fp)) break;
                    x++;
                  }
                  fseek(fp,0,SEEK_SET);
                  x=0;
                  while (1)
                  {
                    fgets(fn,MAX_PATH,fp);
                    fgets(ft,4096,fp);
                    if (feof(fp)) break;
                    if (listSel < thisp)
                    {
                      if (x != listSel) fprintf(fpo,"%s%s",fn,ft);
                      if (x == thisp) fprintf(fpo,"%s%s",file1,title1);
                    }
                    else
                    {
                      if (x == thisp) fprintf(fpo,"%s%s",file1,title1);
                      if (x != listSel) fprintf(fpo,"%s%s",fn,ft);
                    }
                    x++;
                  }
                }
                if (fp) fclose(fp);
                if (fpo) fclose(fpo);
                if (fp && fpo)
                {
                  Bookmark_getfn(fn);
                  DeleteFile(fn);
                  MoveFile(TEMP_FILE,fn);
                  SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETTEXT,(WPARAM)listSel,(LPARAM)title1);
                  SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_DELETESTRING,(WPARAM)listSel,0);
                  SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_INSERTSTRING,(WPARAM)thisp,(LPARAM)title1);
                  listSel=thisp;
                  SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_SETCURSEL,thisp,0);
                }
              }

						}
			}
		return 0;
    case WM_USER+32:
      if (wParam == 1024 && lParam == 3213)
      {
        char fn[MAX_PATH] = {0};

        FILE *fp;
        SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_RESETCONTENT,0,0);
        Bookmark_getfn(fn);
        fp=fopen(fn,"rt");
        if (fp)
        {
          while (1)
          {
            char ft[4096] = {0};
            fgets(fn,MAX_PATH,fp);
            if (feof(fp)) break;
            fgets(ft,4096,fp);
            if (feof(fp)) break;
            if (ft[0] && fn[0])
            {
              if (fn[lstrlen(fn)-1]=='\n') fn[lstrlen(fn)-1]=0;
              if (ft[lstrlen(ft)-1]=='\n') ft[lstrlen(ft)-1]=0;
              if (ft[0] && fn[0])
              {
                SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_ADDSTRING,0,(LPARAM)ft);
              }
            }
          }
          fclose(fp);
        }
      }
    return 0;
		case WM_INITDIALOG:
      {
        char fn[MAX_PATH] = {0};

        FILE *fp;
        OldBookListProc=(WNDPROC)SetWindowLong(GetDlgItem(hwndDlg,IDC_SELBOX), GWLP_WNDPROC,(LONG)BookListProc);
        listDragging=0;
        listSel=-1;
        Bookmark_getfn(fn);
        fp=fopen(fn,"rt");
        if (fp)
        {
          while (1)
          {
            char ft[4096] = {0};
            fgets(fn,MAX_PATH,fp);
            if (feof(fp)) break;
            fgets(ft,4096,fp);
            if (feof(fp)) break;
            if (ft[0] && fn[0])
            {
              if (fn[lstrlen(fn)-1]=='\n') fn[lstrlen(fn)-1]=0;
              if (ft[lstrlen(ft)-1]=='\n') ft[lstrlen(ft)-1]=0;
              if (ft[0] && fn[0])
              {
                SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_ADDSTRING,0,(LPARAM)ft);
              }
            }
          }
          fclose(fp);
        }
      }
		return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
        case IDC_BUTTON5:
        case IDC_SELBOX:
        case IDC_BUTTON4: // open
          if (LOWORD(wParam) != IDC_SELBOX || HIWORD(wParam) == LBN_DBLCLK)
          {
            int x,len,openDir=0;
            char fn[MAX_PATH] = {0};
            FILE *fp;
            Bookmark_getfn(fn);
            fp=fopen(fn,"rt");
            
            if (fp)
            {
              len=SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCOUNT,0,0);
              for (x = 0; x < len; x ++)
              {
                char ft[4096] = {0};
                fgets(fn,MAX_PATH,fp);
                fgets(ft,4096,fp);
                if (feof(fp)) break;
                if (SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETSEL,x,0))
                {
                  if (ft[0] && fn[0])
                  {
                    if (fn[lstrlen(fn)-1]=='\n') fn[lstrlen(fn)-1]=0;
                    if (ft[lstrlen(ft)-1]=='\n') ft[lstrlen(ft)-1]=0;
                    if (ft[0] && fn[0])
                    {
											if (!strstr(fn,"http://"))
											{
												int ga=GetFileAttributes(fn);
												if ((ga!=0xffffffff) && (ga & FILE_ATTRIBUTE_DIRECTORY))
												{
													getNewFile((LOWORD(wParam) != IDC_BUTTON5),hwndDlg,fn);
													openDir=1;
												} 
											}
											if(!openDir)
											{
					              if (LOWORD(wParam) != IDC_BUTTON5) PlayList_delete();
												PlayList_appendthing(fn);
											}
                    }
                  }
                }
              }
              if (LOWORD(wParam) != IDC_BUTTON5 && !openDir) 
              {
          		  if (config_shuffle) PlayList_randpos(-BIGINT);
          		  else PlayList_setposition(0);
		            PlayList_getcurrent(FileName,FileTitle,FileTitleNum);
            	  plEditRefresh();
	              StartPlaying();
              }
              else
            	  plEditRefresh();
              fclose(fp);
            }
          }
        return 0;
        case IDC_EDITBOOK:
          {
            int sel=SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCURSEL,0,0);
            if (sel != LB_ERR)
            {
              int x;
              char fn[MAX_PATH] = {0};
              FILE *fp,*fpo;
              Bookmark_getfn(fn);
              fp=fopen(fn,"rt");
            
              fpo=fopen(TEMP_FILE,"wt");
              if (fp&&fpo)
              {
                x=0;
                while (1)
                {
                  char ft[4096] = {0};
                  fgets(fn,MAX_PATH,fp);
                  fgets(ft,4096,fp);
                  if (feof(fp)) break;
                  if (fn[lstrlen(fn)-1]=='\n') fn[lstrlen(fn)-1]=0;
                  if (ft[lstrlen(ft)-1]=='\n') ft[lstrlen(ft)-1]=0;
                  if (x==sel)
                  {
                    g_bmedit_fn=fn;
                    g_bmedit_ft=ft;
                    LPDialogBox(IDD_EDITBOOKMARK,hwndDlg,BookMarkEditProc);
                    SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_DELETESTRING,x,0);
                    SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_INSERTSTRING,x,(LPARAM)ft);
                    SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_SETCURSEL,x,0);
                  }
                  fprintf(fpo,"%s\n%s\n",fn,ft);
                  x++;
                }
              }
              if (fp) fclose(fp);
              if (fpo) fclose(fpo);
              if (fp && fpo)
              {
                Bookmark_getfn(fn);
                DeleteFile(fn);
                MoveFile(TEMP_FILE,fn);
              }
            }
          }
        return 0;
        case IDC_BUTTON1: // remove
          {
            int sel=SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_GETCURSEL,0,0);
            if (sel != LB_ERR) {
              char fn[MAX_PATH] = {0};
              FILE *fp,*fpo;
              Bookmark_getfn(fn);
              fp=fopen(fn,"rt");
            
              fpo=fopen(TEMP_FILE,"wt");
              if (fp&&fpo)
              {
                int l=0;
                int x=0;
                while (1)
                {
                  char ft[4096] = {0};
                  fgets(fn,MAX_PATH,fp);
                  fgets(ft,4096,fp);
                  if (feof(fp)) break;
                  if (x == sel)
                  {
                    l=x-1;
                    SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_DELETESTRING,x,0);
                  }
                  else 
                  {
                    fprintf(fpo,"%s%s",fn,ft);
                  }
                  x++;
                }
                SendDlgItemMessage(hwndDlg,IDC_SELBOX,LB_SETCURSEL,l,0);
              }
              if (fp) fclose(fp);
              if (fpo) fclose(fpo);
              if (fp && fpo)
              {
                Bookmark_getfn(fn);
                DeleteFile(fn);
                MoveFile(TEMP_FILE,fn);
              }
            }
          }
        break;
			}
		return FALSE;
	}
	return 0;
}

#endif
