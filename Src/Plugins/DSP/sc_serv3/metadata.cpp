#ifdef _WIN32
#include <winsock2.h>
#endif
#include "metadata.h"
#include "aolxml/aolxml.h"
#include "filenameMetadata.h"
#include "file/fileUtils.h"
#include "stl/stringUtils.h"
#include "services/stdServiceImpl.h"
#include "streamData.h"

using namespace std;
using namespace stringUtil;
using namespace uniString;

metadata::metadata(const metadata &m)
{
	copy(m);
}

metadata& metadata::operator=(const metadata &m)
{
	copy(m);
	return *this;
}

void metadata::clear() throw()
{
	for (keyValueMap_t::const_iterator i = m_keyValueMap.begin(); i != m_keyValueMap.end(); ++i)
	{
		delete (*i).second;
	}
	m_keyValueMap.clear();
}

void metadata::copy(const metadata &m) throw()
{
	clear();
	for (keyValueMap_t::const_iterator i = m.m_keyValueMap.begin(); i != m.m_keyValueMap.end(); ++i)
	{
		if ((*i).second)
		{
			m_keyValueMap.insert(make_pair((*i).first,(*i).second->clone()));
		}
	}
}

metadata::~metadata() throw()
{
	clear();
}

// remove all values for key
void metadata::removeValue(const string &key) throw()
{
	keyValueMap_t::iterator i = m_keyValueMap.find(key);
	while (i != m_keyValueMap.end())
	{
		delete (*i).second;
		m_keyValueMap.erase(i);
		i = m_keyValueMap.find(key);
	}
}

bool metadata::valueExists(const string &key) const throw()
{
	return (m_keyValueMap.find(key) != m_keyValueMap.end());
}

//// only returns first one found
const metadata::metaValue_base* metadata::getValue(const string &key) const throw()
{
	keyValueMap_t::const_iterator i = m_keyValueMap.find(key);
	return (i == m_keyValueMap.end() ? 0 : (*i).second);
}

//// only returns first one found
utf8 metadata::getValueAsString(const std::string &key) const throw()
{
	static utf8 empty;

	const metaValue_base *mv = getValue(key);
	return (mv ? mv->toString() : empty);
}

void metadata::setValue(const string &key,metadata::metaValue_base *value) throw()
{
	m_keyValueMap.insert(make_pair(key,value));
}

utf8 metadata::safeXML(const string &tag,const metaValue_base *m) throw()
{
	static utf8 empty;
	if (!m) return empty;
	return m->toXML(tag);
}

utf8 metadata::safeString(const metaValue_base *m) throw()
{
	static utf8 empty;
	if (!m) return empty;
	return m->toString();
}

bool metadata::noMeaningfulMetadata() const throw()
{
	return (m_keyValueMap.find(NAME()) == m_keyValueMap.end());
}

bool metadata::get_replayGain(double &gain) const throw()
{
	pair<keyValueMap_t::const_iterator,keyValueMap_t::const_iterator> mdrange = m_keyValueMap.equal_range("TXXX");
	for (keyValueMap_t::const_iterator i = mdrange.first; i != mdrange.second; ++i)
	{
		metadata::metaValue<ID3V2::userText_t> *md = dynamic_cast<metadata::metaValue<ID3V2::userText_t> *>((*i).second);
		if (md)
		{
			ID3V2::userText_t ut = md->value();
			if (ut.m_id == "replaygain_track_gain" || ut.m_id == "REPLAYGAIN_TRACK_GAIN")
			{
				gain = atof((const char *)ut.m_text.c_str());
				return true;
			}
		}
	}
	return false;
}

bool metadata::get_replayGain() const throw()
{
	double g;
	return get_replayGain(g);
}

const utf8 METADATA("<metadata>");
const utf8 E_METADATA("</metadata>");
#ifdef XML_DEBUG
static const utf8 EOL(eol());
#else
static const utf8 EOL("");
#endif

// new xml for shoutcast		
utf8 metadata::toXML() const throw()
{
	utf8 o;
	o += "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" + EOL;
	o += METADATA + EOL;
	for (keyValueMap_t::const_iterator i = m_keyValueMap.begin(); i != m_keyValueMap.end(); ++i)
	{
		o += safeXML((*i).first,(*i).second) + EOL;
	}
	o += E_METADATA + EOL;
	return o;
}

utf8 metadata::toXML_fromFilename(const uniFile::filenameType &filename,const uniFile::filenameType &url,const utf8 &pattern) throw()
{
	utf8 o;

	o += "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" + EOL;
	o += METADATA + EOL;

	bool patternWorked(false);
	try
	{
		filenameMetadata fm;
		fm.setPattern(pattern);
		fm.parse(filename);
		const std::map<uniString::utf8,uniString::utf8>&m = fm.getTokens();
		for (map<utf8,utf8>::const_iterator i = m.begin(); i != m.end(); ++i)
		{
			o += "<" + (*i).first + ">" + (*i).second.escapeXML() + "</" + (*i).first + ">" + EOL;
		}
		patternWorked = true;
	}
	catch(.../*const exception &ex*/)
	{
	}

	if (!patternWorked)
	{
		try
		{
			string str = string(filename.toANSI(true));
			utf8 song = asciiToUtf8(str);

			filenameMetadata fm;
			fm.setPattern(pattern);
			fm.parse(song);
			const std::map<uniString::utf8,uniString::utf8>&m = fm.getTokens();
			for (map<utf8,utf8>::const_iterator i = m.begin(); i != m.end(); ++i)
			{
				o += "<" + (*i).first + ">" + (*i).second.escapeXML() + "</" + (*i).first + ">" + EOL;
			}
			patternWorked = true;
		}
		catch(.../*const exception &ex*/)
		{
			//ELOG(string("[METADATA] Failure converting filename to metadata ") + ex.what());
		}
	}

	if (!patternWorked)
	{
		o += "<TIT2>";

		utf8 us = fileUtil::stripSuffix(filename);
		// remove path based on delimiter for Unix (/) Win32 (\) or MacOS (:)
		us = fileUtil::stripPath(us,utf8("/"));
		us = fileUtil::stripPath(us,utf8("\\"));
		us = fileUtil::stripPath(us,utf8(":"));
		o += us.escapeXML();
		o += "</TIT2>" + EOL;
	}

	if (!url.empty())
	{
		utf8 u = url;
		if ((u.find(utf8("://")) == utf8::npos) &&
			(u.find(utf8("&")) != 0) &&
			u.find(utf8("DNAS/streamart?sid=")) == utf8::npos &&
			u.find(utf8("DNAS/playingart?sid=")) == utf8::npos)
		{
			u = "http://" + u;
		}
		o += "<URL>" + u.escapeXML() + "</URL>" + EOL;
	}

	o += E_METADATA + EOL;
	return o;
}

utf8 metadata::toFixedString(const uniFile::filenameType &filename) throw()
{
	utf8 o;

	bool patternWorked(false);
	try
	{
		filenameMetadata fm;
		fm.setPattern("%N");
		fm.parse(filename);
		const std::map<uniString::utf8,uniString::utf8>&m = fm.getTokens();
		for (map<utf8,utf8>::const_iterator i = m.begin(); i != m.end(); ++i)
		{
			o = (*i).second;
		}
		patternWorked = true;
	}
	catch(.../*const exception &ex*/)
	{
	}

	if (!patternWorked)
	{
		try
		{
			string str = string(filename.toANSI(true));
			utf8 song = asciiToUtf8(str);

			filenameMetadata fm;
			fm.setPattern("%N");
			fm.parse(song);
			const std::map<uniString::utf8,uniString::utf8>&m = fm.getTokens();
			for (map<utf8,utf8>::const_iterator i = m.begin(); i != m.end(); ++i)
			{
				o = (*i).second;
			}
			patternWorked = true;
		}
		catch(.../*const exception &ex*/)
		{
			//ELOG(string("[METADATA] Failure converting filename to metadata ") + ex.what());
		}
	}

	if (!patternWorked)
	{
		o = filename;
	}

	return o;
}

uniString::utf8 metadata::convert_3902_to_shoutcast1(const uniString::utf8 &d, const streamID_t id) throw (std::runtime_error)
{
	utf8 o;
	aolxml::node *n = 0;
	try
	{
		n = aolxml::node::parse(d.hideAsString());
		utf8 artist = aolxml::subNodeText(n, "/metadata/" + ARTIST(), (utf8)"");
		utf8 name = aolxml::subNodeText(n, "/metadata/" + NAME(), (utf8)"");

		if (!artist.empty())
		{
			if (!o.empty())
			{
				o += " - ";
			}
			o += artist;
		}
		if (!name.empty())
		{
			if (!o.empty())
			{
				o += " - ";
			}
			o += name;
		}

		if (!streamData::validateTitle(o))
		{
			WLOG("[ADMINCGI sid=" + tos(id) + "] Title update rejected - value not allowed: " + o);
			o.clear();
		}

		o = "StreamTitle='" + o + "';";

		aolxml::node::nodeList_t nodes = aolxml::node::findNodes(n,"/metadata/extension/title");
		if (!nodes.empty())
		{
			for (aolxml::node::nodeList_t::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
			{
				if (*i)
				{
					// skip the first element as that is the current song and not what we want to get
					int seq = atoi((*i)->findAttributeString("seq").c_str());
					if (seq == 2)
					{
						utf8 next = ((*i)->pcdata());
						if (streamData::validateTitle(next))
						{
							o += "StreamNext='" + next + "';";
						}
						break;
					}
				}
			}
		}

		utf8 url = aolxml::subNodeText(n, "/metadata/" + URL(), (utf8)"");
		if (!url.empty())
		{
			utf8::size_type pos = url.find(utf8("://"));
			if (pos == utf8::npos)
			{
				url = "http://" + url;
			}

			o += "StreamUrl='" + url + "';";
		}
	}
	catch(const exception &ex)
	{
		forget(n);
		throw std::runtime_error(ex.what());
	}
	forget(n);
	return o;
}

uniString::utf8 metadata::get_song_title_from_3902(const uniString::utf8 &d)
{
	utf8 o;
	utf8 result;
	aolxml::node *n = 0;
	try
	{
		n = aolxml::node::parse(d.hideAsString());
		utf8 artist = aolxml::subNodeText(n, "/metadata/" + ARTIST(), (utf8)"");
		utf8 name = aolxml::subNodeText(n, "/metadata/" + NAME(), (utf8)"");

		if (!artist.empty())
		{
			if (!result.empty()) result += " - ";
			result += artist;
		}
		if (!name.empty())
		{
			if (!result.empty()) result += " - ";
			result += name;
		}
	}
	catch(const exception &ex)
	{
		forget(n);
		throw std::runtime_error(ex.what());
	}
	forget(n);
	return result;
}

uniString::utf8 metadata::get_XX_from_3902(const uniString::utf8& node, const uniString::utf8 &d,
										   const uniString::utf8 &old) throw (std::runtime_error)
{
	utf8 result;
	aolxml::node *n = 0;
	try
	{
		n = aolxml::node::parse(d.hideAsString());
		result = aolxml::subNodeText(n, utf8("/metadata/" + node).hideAsString(), old);
	}
	catch(const exception &ex)
	{
		forget(n);
		throw std::runtime_error(ex.what());
	}
	forget(n);
	return result;
}

std::vector<uniString::utf8> metadata::get_nextsongs_from_3902(const uniString::utf8 &d,
															   std::vector<uniString::utf8>& oldSongList,
															   const bool first) throw (std::runtime_error)
{
	aolxml::node *n = 0;
	std::vector<uniString::utf8> nextSongList;

	try
	{
		n = aolxml::node::parse(d.hideAsString());
		aolxml::node::nodeList_t nodes = aolxml::node::findNodes(n,"/metadata/extension/title");
		if(!nodes.empty())
		{
			for (aolxml::node::nodeList_t::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
			{
				if (*i)
				{
					// skip the first element as that is the current song and not what we want to hold
					int seq = atoi((*i)->findAttributeString("seq").c_str());
					if ((seq > 1 && !first) || (first && seq == 2))
					{
						nextSongList.push_back((*i)->pcdata());
					}
					if (first)
					{
						break;
					}
				}
			}
		}
		else
		{
			// if there are no nodes then as this could be from a stream-specific metadata
			// update then we need to preserve the existing list so it's not cleared out.
			forget(n);
			return oldSongList;
		}
	}
	catch(const exception &ex)
	{
		forget(n);
		throw std::runtime_error(ex.what());
	}
	forget(n);
	return nextSongList;
}
