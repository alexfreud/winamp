#include "main.h"
#include "api.h"
#include <stdio.h>

void Bookmark_additem(char *fn, char *ft)
{
  FILE *fp;
  
  fp=fopen(BOOKMARKFILE,"a+t");
  if (fp)
  {
    char *buf=(char*)malloc(lstrlen(fn)+lstrlen(ft)+3);
    fprintf(fp,"%s\n%s\n",fn,ft);
    fclose(fp);
    if (buf)
    {
      lstrcpy(buf,fn);
      lstrcpy(buf+lstrlen(buf)+1,ft);
      SendMessage(hMainWindow,WM_WA_IPC,(WPARAM)buf,IPC_ADDBOOKMARK);
      free(buf);
			AGAVE_API_STATS->IncrementStat(api_stats::BOOKMARK_COUNT);
    }

  }
}