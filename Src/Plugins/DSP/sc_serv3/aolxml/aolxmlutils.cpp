#include "aolxml.h"
#include "stl/stringUtils.h"
#include <map>

using namespace std;
using namespace uniString;
using namespace stringUtil;

class xmlEscapes: public map<char,string>
{
public:
	xmlEscapes()
	{  		
		(*this)['<'] = "&lt;";
		(*this)['>'] = "&gt;";
		(*this)['&'] = "&amp;";
		(*this)['\''] = "&apos;";
		(*this)['"'] = "&quot;";
	}
};

static const xmlEscapes gsXmlEscapes;

string aolxml::escapeXML(const string &s) throw()
{
	string result;
	string::size_type siz = s.size();
	for(string::size_type x = 0; x < siz; ++x)
	{
		unsigned char uc = s[x];
		if (((uc > 0x7f) || (uc >= 1 && uc <= 8) || (uc >= 0x0b && uc <= 0x0c) || (uc >= 0x0e && uc <= 0x1f)))
		{
			result += "&#" + tos((unsigned int)uc) + ";";
		}
		else 
		{
			xmlEscapes::const_iterator i = gsXmlEscapes.find(s[x]);
			if (i != gsXmlEscapes.end())
				result += (*i).second;
			else
				result += s[x];
		}
	}
	return result;
}

utf8 aolxml::escapeXML(const utf8 &s) throw()
{
	string result;
	string::size_type siz = s.size();
	for(string::size_type x = 0; x < siz; ++x)
	{
		//unsigned char uc = s[x];
		xmlEscapes::const_iterator i = gsXmlEscapes.find(s[x]);
		if (i != gsXmlEscapes.end())
			result += (*i).second;
		else
			result += s[x];
	}
	return result;
}
