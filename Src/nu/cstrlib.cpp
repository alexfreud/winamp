/* Utility library for C strings */

#include <windows.h>

extern "C" 
char *scanstr_back(char *str, char *toscan, char *defval)
{
	char *s=str+strlen(str)-1;
	if (strlen(str) < 1) return defval;
	if (strlen(toscan) < 1) return defval;
	while (1)
	{
		char *t=toscan;
		while (*t)
			if (*t++ == *s) return s;
		t=CharPrev(str,s);
		if (t==s) return defval;
		s=t;
	}
}

extern "C" 
char *extension(char *fn) 
{
  char *s = scanstr_back(fn,".\\",fn-1);
  if (s < fn) return "";
  if (*s == '\\') return "";
  return (s+1);
}

void CleanDirectory(char *str)
{
	if (!str)
		return;
	int l = strlen(str);

	while (l--)
	{
		if (str[l] == ' '
			|| str[l] == '.')
			str[l]=0;
		else
			break;
	}
}


void FormatSizeStr64(char *out, __int64 size)
{
	if (size < 1024*1024) wsprintf(out, "%u KB", (DWORD)(size >> 10));
	else if (size < 1024*1024*1024)
	{
		wsprintf(out, "%u.%02u MB", (DWORD)(size >> 20), ((((DWORD)(size >> 10))&1023)*100) >> 10);
	}
	else
	{
		wsprintf(out, "%u.%02u GB", (DWORD)(size >> 30), ((((DWORD)(size >> 20))&1023)*100) >> 10);
	}
}