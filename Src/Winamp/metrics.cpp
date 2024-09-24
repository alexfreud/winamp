/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "Main.h"
#include <windows.h>
#include "../nu/AutoUrl.h"
#include <io.h>

char metric_plugin_list[512] = {0};

typedef struct 
{
	char country[64];
	char email[128];
	char objects[256];
	unsigned char sendemail;
	unsigned char sex; //1=m, 2=f, otherwise undefined
	unsigned char agegroup; // 1=under 13, 2=13-18, 3=18-24, 4=25-34, 5=35-44, 6=45-54, 7=55+
	unsigned char pad[1];
} reg_data;

void makeurlcodes(char *in, char *out)
{
	char *i=in,*p=out;
	while (i && *i)
	{
    int v=*((unsigned char *)i++);
    if (IsCharAlpha(v) || (v>='0' && v <='9') || v == ' ' || v == '.' || v == '-' || v == '_') *p++ = v;
		else
		{
			int a,b;
			*p++ = '%';
			a=v>>4;
			b=v&15;
			a += '0';
			if (a > '9') a += 'A'-'9'-1;
			b += '0';
			if (b > '9') b += 'A'-'9'-1;
			*p++ = a;
			*p++ = b;		
		}
	}
	if (p) *p=0;
}

static int in_objlist(char *objlist, char *obj)
{
	char *b=objlist-1;
	while ((b=strstr(b+1,obj)))
	{
		if (b==objlist||b[-1]==':')
		{
			if (b[lstrlenA(obj)]==':' || !b[lstrlenA(obj)])
				return 1;
		}
	}
	return 0;
}

/* benski> this could be made faster using StringCchCatEx, but this isn't called enough to warrant
spending the time right this instant */
static void diff_objs(char *oldobj, char *newobj, char out[512])
{
	char *n=newobj;
	*out=0;
	while (n && *n)
	{
		char buf[128],*b=buf;
		while (n && *n && *n != ':') *b++=*n++;
		if (n && *n) n++;
		*b++=0;
		if (!in_objlist(oldobj,buf))
		{
			if (!*out) StringCchCatA(out, 512, ":");
			StringCchCatA(out, 512, buf);
		}
	}
	n=oldobj;
	while (n && *n)
	{
		char buf[128],*b=buf;
		while (n && *n && *n != ':') *b++=*n++;
		if (n && *n) n++;
		*b++=0;
		if (!in_objlist(newobj,buf))
		{
			if (!*out) StringCchCatA(out, 512, ":");
			StringCchCatA(out, 512, "-");
			StringCchCatA(out, 512, buf);
		}
	}
}

static void get_objlist(char l[256])
{
	char browserbrand[128]="";
	GetPrivateProfileStringA("winamp","browserbrand","",browserbrand+1,sizeof(browserbrand)-1,INI_FILEA);
	if (browserbrand[1])
		browserbrand[0]='.';
	else browserbrand[0]=0;
		StringCchPrintfA(l, 256, "wa%s%s%s",app_version,browserbrand,metric_plugin_list);
}

#define METRICS_EMAIL			0x0001
#define METRICS_COUNTRY			0x0002
#define METRICS_ANNOUNCEMENTS	0x0003
#define METRICS_GENDER			0x0004

INT GetMetricsValueW(const char *data, const char *pszType, void *pDest, int cbDest)
{
	reg_data *prd = (reg_data*)data;
	char *str = NULL;
	unsigned char *val = NULL; 
	if (!prd || !pszType || !pDest) return 0;

	if (IS_INTRESOURCE(pszType))
	{
		switch(((INT)(INT_PTR)pszType))
		{
			case METRICS_EMAIL:			str = prd->email; break;
			case METRICS_COUNTRY:		str = prd->country; break;
			case METRICS_ANNOUNCEMENTS:	val = &prd->sendemail; break;
			case METRICS_GENDER:			val = &prd->sex; break;
		}
	}

	if (str) return MultiByteToWideChar(CP_ACP, 0, str, -1, (wchar_t*)pDest, cbDest/sizeof(wchar_t));
	if (val) 
	{
		ZeroMemory(pDest, cbDest);
		*((unsigned char*)pDest) = *val;
		return sizeof(*val);
	}
	return 0;
}

BOOL SetMetricsValueW(const char *data, const char *pszType, const void *pVal, int cbVal)
{
	reg_data *prd = (reg_data*)data;
	char *str = NULL;
	unsigned char *val = NULL; 
	int maxLen;
	if (!prd || !pszType) return FALSE;

	maxLen = 1;

	if (IS_INTRESOURCE(pszType))
	{
		switch(((INT)(INT_PTR)pszType))
		{
			case METRICS_EMAIL:			str = prd->email; maxLen = sizeof(prd->email)/sizeof(char); break;
			case METRICS_COUNTRY:		str = prd->country; maxLen = sizeof(prd->country)/sizeof(char); break;
			case METRICS_ANNOUNCEMENTS:	val = &prd->sendemail; break;
			case METRICS_GENDER:			val = &prd->sex; break;
		}
	}
 
	if (str)
	{
		int len;
		const wchar_t *val = (const wchar_t*)pVal;
		if (!*val) { *str = 0x00; return TRUE; }
		len = WideCharToMultiByte(CP_ACP, 0, val, (-1 == cbVal) ? -1 : cbVal/sizeof(wchar_t), str, maxLen, NULL, NULL);
		if (len && len < maxLen) str[len] = 0x00;
		return (len);
	}
	if (val) 
	{
		*val = *((unsigned char*)pVal);
		return TRUE;
	}

	return FALSE;
}

INT GetMetricsSize(const char *data)
{
	reg_data *prd = (reg_data*)data;
	if (!prd || (!*prd->email && !*prd->country && !prd->sex && !prd->agegroup && !prd->sendemail)) return 0;
	return sizeof(reg_data);
}

BOOL SendMetrics(const char *data, HWND hwndParent)
{
	BOOL result;
	reg_data *prd = (reg_data*)data;
	char buf1[512] = {0}, buf2[512] = {0}, _old_obj[256] = {0};
	char urlbuf[8192] = {0};
	size_t urlsize = sizeof(urlbuf)/sizeof(*urlbuf);
	char *url = urlbuf;
	int is_upgrade=!GetPrivateProfileIntW(L"WinampReg",L"IsFirstInst",0,INI_FILE);

	if (!prd) return 0;

	// fucko: set is_upgrade
	StringCchCopyExA(url,urlsize,"http://client.winamp.com/update/do_im.php?", &url, &urlsize, 0);
	makeurlcodes(prd->country,buf1);
	stats_getuidstr(buf2);
	StringCchPrintfExA(url, urlsize, &url, &urlsize, 0, "ID=%s&ZIP=%s",buf2,buf1);
	makeurlcodes(prd->email,buf1);
	StringCchPrintfExA(url, urlsize, &url, &urlsize, 0, "&EMAIL=%s&ML=%c" /*"&AGE=%d" */ "&SEX=%c&ISUP=%d",buf1,prd->sendemail?'y':'n',
//				prd->agegroup,
				prd->sex==1 ? 'M' : (prd->sex==2 ? 'F' : 'N' ),
				is_upgrade);
	
	CopyMemory(_old_obj, prd->objects, sizeof(_old_obj));
	get_objlist(prd->objects);
	diff_objs(_old_obj,prd->objects,buf1);
	makeurlcodes(buf1,buf2);
	StringCchPrintfExA(url, urlsize, &url, &urlsize, 0,"&OBJS=%s",buf2);
	StringCchPrintfExA(url, urlsize, &url, &urlsize, 0,"&SKIN=%s",(char *)AutoUrl(config_skin));

	wchar_t mlfilename[MAX_PATH] = {0};
	PathCombineW(mlfilename, PLUGINDIR, L"gen_ml.dll");
	StringCchPrintfExA(url, urlsize, &url, &urlsize, 0,"&MLIB=%d",!_waccess(mlfilename, 0));

	StringCchPrintfExA(url, urlsize, &url, &urlsize, 0,"&V=%s",app_version);

	DeleteFileW(TEMP_FILE);
	result = !httpRetrieveFileW(hwndParent, urlbuf, TEMP_FILE, getStringW(IDS_INST_SENDINGIN,NULL,0)); 
	if (result) WritePrivateProfileStringW(L"WinampReg", L"IsFirstInst", L"0", INI_FILE);
	return result;
}