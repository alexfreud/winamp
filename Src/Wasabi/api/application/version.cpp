#include <precomp.h>
#include "version.h"
#include <bfc/parse/pathparse.h>

static int buildno = -1;
static const wchar_t *WASABI_VERSION = L"";

#define STRFILEVER     "1, 0, 0, 502\0"

static StringW  version_string;

void WasabiVersion::setAppName(const wchar_t *name) 
{
  appname = name;
}

const wchar_t *WasabiVersion::getAppName() 
{
  return appname;
}

const wchar_t *WasabiVersion::getVersionString() 
{
  if (appname.isempty()) 
		appname=L"";
  if (version_string.isempty()) 
	{
    if (!appname.isempty())
      version_string = appname.getValue();
    if (wcslen(WASABI_VERSION)) 
		{
      version_string.cat(L" ");
      version_string.cat(WASABI_VERSION);
    }
  }
  return version_string;
}

unsigned int WasabiVersion::getBuildNumber() 
{
  if (buildno == -1) 
	{
    PathParser pp(STRFILEVER, ",");
    buildno = ATOI(pp.enumString(3));
  }
  return buildno;
}
