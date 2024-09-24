/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "main.h"



#if 0
void ll_update(HWND hwndDlg)
{
	char TEMPFILE2[MAX_PATH] = {0};
	char buf[1024] = {0};
	int success=0;
	lstrcpyn(TEMPFILE2,TEMP_FILE, MAX_PATH-1); // -1 because we strcat a "0"
	
	lstrcat(TEMPFILE2,"0");
	{
		char url[512] = {0};
		int c='y',r='n';
		{
			FILE *fp = fopen(LINKFILE,"rt");
			if (fp)
			{
				fclose(fp);
				c='n';
			}
		}
		wsprintf(url,"http://client.winamp.com/update/updatelinks.jhtml?i=%c&v=%s&r=%c",
			c,
			app_version,r);
		if (!httpRetrieveFile(hwndDlg,url,TEMPFILE2,getString(IDS_DLINK_GETTING,NULL,0)))
		{
			int st=0;
			FILE *fp = fopen(TEMPFILE2,"rt");
      char bnav[1024]="";
		  char d_bloc[128]="DefBrowseLoc";
		  if (*config_browserbrand)
		  {
			  lstrcat(d_bloc,"_"); 
			  lstrcat(d_bloc,config_browserbrand);
		  }
		  lstrcat(d_bloc," ");
			if (fp) 
			{
				while (!feof(fp))
				{
					fgets(buf,sizeof(buf),fp);
					if (!strcmp(buf,"Winamp Links File v1.0\n")) st|=1;
					if (!strncmp(buf,"EndFile",7)) st|=2;
          if (!_strnicmp(buf,d_bloc,lstrlen(d_bloc)))
          {
            lstrcpyn(bnav, buf+lstrlen(d_bloc), 1024);
          }
				}
				fclose(fp);
			}
			if (st==3)
			{
				DeleteFile(LINKFILE);
				MoveFile(TEMPFILE2,LINKFILE);
        success=1;
			}
			if (success)
			{
				char str[256]="";
				char *s="http://client.winamp.com/update/mb.html";
				GetPrivateProfileString("Winamp","MBDefLoc",s,str,sizeof(str),INI_FILE);
        if (_strnicmp(str,"http://",7)) lstrcpy(str,s);
				httpRetrieveFile(hwndDlg,str,MBFILE,getString(IDS_DLINK_GETTING,NULL,0));
          
        if (strcmp(str,s)) mbctl_navigate(str,str);
        else if (isInetAvailable()) 
        {
          char *t;
          if (bnav[0] && (t=strstr(bnav," ")))
          {
            *t++=0;
            mbctl_navigate(bnav,t);
          }
          else mbctl_navigate("http://client.winamp.com/browser/","Winamp Minibrowser");
        }
        else mbctl_navigate(MBFILE,"Winamp Minibrowser");
			}
		}
		DeleteFile(TEMPFILE2);
	}
}

#endif