#include "baseOptions.h"
#include "stl/stringUtils.h"
#ifdef _WIN32
#include "win32/rezFuncs.h"
#else
#include "unixversion.h"
#endif

using namespace std;
using namespace uniString;
using namespace stringUtil;

bool baseOptions::toBool(const utf8 &s) throw()
{
	if (s.empty()) return true;
	utf8 ss = toLower(s);
	return (s[0] == 't' || s[0] == '1' || s[0] == 'y');
}

const vector<utf8> baseOptions::fromArgs(const vector<utf8> &cl) throw()
{
	vector<utf8> unused;

	for (vector<utf8>::const_iterator i = cl.begin(); i != cl.end(); ++i)
	{
		utf8::size_type colon_pos = (*i).find(utf8(":"));
		if (colon_pos != utf8::npos)
		{
			utf8 key = (*i).substr(0,colon_pos);
			utf8 value = (*i).substr(colon_pos+1);
			if (key == "flog")
			{
				m_fileLog = value;
			}
			else if (key == "clog")
			{
				m_consoleLogging = toBool(value);
			}
			else
			{
				unused.push_back(*i);
			}
		}
		else
		{
			unused.push_back(*i);
		}
	}

	return unused;
}

utf8 baseOptions::getVersionBuildStrings() throw()
{
#ifdef _WIN32
	static utf8 version;
	if (version.empty())
	{
		getVersionInfo(version);
	}
#else
	static utf8 version;
	if (version.empty())
	{
		for (int x = 0; x < VENT; ++x)
		{
			if (x)
			{
				version += ".";
			}
			version += tos(PRODUCTVERSION[x]);
		}
	}
#endif	
	return version;
}
