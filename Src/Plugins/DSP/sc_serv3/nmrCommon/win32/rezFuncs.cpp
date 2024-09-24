#include <windows.h>
#include <tchar.h>
#include "rezFuncs.h"
#include "stl/stringUtils.h"

using namespace std;
using namespace stringUtil;
using namespace uniString;

void getVersionInfo(utf8 &version) throw()
{
	version = "";
	
	TCHAR filename[_MAX_PATH + 1] = {0};
	DWORD junk = 0;
	char *versionData = 0;

	// version information from the application resources
	::GetModuleFileName(0,filename,_MAX_PATH);
	DWORD versionInfoSize = ::GetFileVersionInfoSize(filename,&junk);
	versionData = new char[versionInfoSize];
	if (::GetFileVersionInfoW(filename,0,versionInfoSize,versionData))
	{
		void *valPtr = 0;
		UINT valSize = 0;
		if (::VerQueryValue(versionData,_T("\\"), &valPtr, &valSize))
		{
			const VS_FIXEDFILEINFO *ffi = reinterpret_cast<const VS_FIXEDFILEINFO *>(valPtr);
			version =  
				tos(((ffi->dwProductVersionMS & 0xffff0000) >> 16)) + "." + 
				tos(((ffi->dwProductVersionMS & 0x0000ffff))) + "." + 
				tos(((ffi->dwProductVersionLS & 0xffff0000) >> 16)) + "." + 
				tos(((ffi->dwProductVersionLS & 0x0000ffff)));
		}
	}
	delete [] versionData;
}
