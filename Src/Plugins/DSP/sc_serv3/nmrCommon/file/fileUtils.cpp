#ifdef _WIN32
#include <windows.h>
#else
#include <sys/param.h>
#endif
#include "fileUtils.h"
#include "stl/stringUtils.h"
#include "services/stdServiceImpl.h"
#include <fstream>
#include "../../global.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

bool fileUtil::convertOSFilePathDelimiter(uniString::utf8 &value) throw()
{
	bool converted = false;
	if (!value.empty())
	{
		#ifdef _WIN32
		const uniString::utf8& replaceFilePathDelimiter("/");
		#else
		const uniString::utf8& replaceFilePathDelimiter("\\");
		#endif

		uniString::utf8::size_type pos = value.find(replaceFilePathDelimiter);
		if (pos != uniString::utf8::npos) converted = true;
		while (pos != uniString::utf8::npos)
		{
			value.replace(pos,1,getFilePathDelimiter());
			pos = value.find(replaceFilePathDelimiter);
		}
	}
	return converted;
}

#ifdef _WIN32

uniFile::filenameType fileUtil::getFullFilePath(const uniFile::filenameType &partial_path) throw()
{
	wchar_t resolved_path[MAX_PATH] = {0};

	uniString::utf32 u32(partial_path);
	std::wstring u16;
	u32.toUtf16(u16);

	if (_wfullpath(resolved_path, u16.c_str(), MAX_PATH))
	{
		return utf32(resolved_path).toUtf8();
	}
	return partial_path;
}

bool fileUtil::fileExists(const uniFile::filenameType &fullPath) throw()
{
	uniString::utf32 u32(fullPath);
	std::wstring u16;
	u32.toUtf16(u16);

	HANDLE hPF = ::CreateFileW(u16.c_str(), 0, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hPF == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	::CloseHandle(hPF);
	return true;
}

bool fileUtil::directoryExists(const uniFile::filenameType &fullPath) throw()
{
	WIN32_FIND_DATAW fd = {0};

	uniString::utf32 u32(fullPath);
	std::wstring u16;
	u32.toUtf16(u16);

	wstring path = u16.substr(0, u16.rfind(L"\\"));
	HANDLE	h = ::FindFirstFileW(path.c_str(), &fd);
	if (h != INVALID_HANDLE_VALUE)
	{
		::FindClose(h);
	}

	return (h != INVALID_HANDLE_VALUE);
}

vector<wstring> fileUtil::directoryFileList(const wstring &pattern, const wstring &currentPath, bool fullPaths, bool preserveCase) throw()
{
	vector<wstring>	result;
	WIN32_FIND_DATAW fd = {0};
	wstring search = pattern;

	if (search.empty())
	{
		return result;
	}

	// look at the path and see if it's been setup correctly i.e has a .\ on the front for relative searches
	if (!((search[1] == ':' && search[2] == '\\') ||
		  (search[0] == '\\' && search[1] == '\\') ||
		  (search[0] == '.' && search[1] == '\\') ||
		  (search[0] == '.' && search[1] == '.' && search[2] == '\\')))
	{
		if (!currentPath.empty())
		{
			search = currentPath + search;
		}
		else
		{
			search = gStartupDirectory.toWString() + search;
		}
	}

	wstring path = search.substr(0,search.rfind(L"\\"));
	HANDLE	h = ::FindFirstFileW(search.c_str(),&fd);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (fullPaths)
			{
				if (preserveCase)
				{
					result.push_back((path + L"\\" + fd.cFileName));
				}
				else
				{
					result.push_back(toLower(path + L"\\" + fd.cFileName));
				}
			}
			else
			{
				if (preserveCase)
				{
					result.push_back((fd.cFileName));
				}
				else
				{
					result.push_back(toLower(wstring(fd.cFileName)));
				}
			}
		}
		while(::FindNextFileW(h,&fd));
	}

	if (h != INVALID_HANDLE_VALUE)
	{
		::FindClose(h);
	}

	return result;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <glob.h>

utf8 fileUtil::getFullFilePath(const uniFile::filenameType &partial_path) throw()
{
	char resolved_path[MAXPATHLEN + 1] = {0};
	if (realpath(partial_path.hideAsString().c_str(), resolved_path))
	{
		return utf8(resolved_path);
	}
	return partial_path;
}

bool fileUtil::fileExists(const uniFile::filenameType &fullPath) throw()
{
	struct stat sbuf;
	return (::stat(fullPath.hideAsString().c_str(),&sbuf) ? false : true);
}

bool fileUtil::directoryExists(const uniFile::filenameType &fullPath) throw()
{
	glob_t gt;
	bool found = false;

	if (glob(fullPath.hideAsString().c_str(),GLOB_NOSORT,0,&gt) == 0)
	{
		found = true;
	}

	globfree(&gt);
	return found;
}

std::vector<std::string> fileUtil::directoryFileList(const std::string &pattern, const std::string &currentPath) throw()
{
	vector<string> result;

	glob_t gt;

	string search = pattern;

	if (search.empty())
	{
		return result;
	}

	// look at the path and see if it's been setup correctly i.e has a ./ on the front for relative searches
	if (!((search[0] == '\\' && search[1] == '\\') ||
		  (search[0] == '/') ||
		  (search[0] == '.' && search[1] == '/') ||
		  (search[0] == '.' && search[1] == '.' && search[2] == '/')))
	{
		if (!currentPath.empty())
		{
			search = currentPath + search;
		}
		else
		{
			search = string("./") + search;
		}
	}

	if (glob(search.c_str(),GLOB_NOSORT,0,&gt) == 0)
	{
		for (size_t x = 0; x < gt.gl_pathc; ++x)
		{
			result.push_back(gt.gl_pathv[x]);
		}
	}

	globfree(&gt);
	return result;
}

#endif
