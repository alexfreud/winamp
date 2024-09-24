#pragma once
#ifndef metadata_H_
#define metadata_H_

#include <map>
#include "ID3miniParsers.h"
#include "unicode/uniFile.h"

// metadata class. Mostly just a container for key/value pairs
// stored ala ID3V2. Some helper functions also to aid in
// ID3V1 mapping
/*
	metadata strings are stored in utf8 format
*/
class metadata
{
public:
	// The metadata values are polymorphic since they may contain all sorts of differnt
	// types of information depending on the specific tag. All you can do with them
	// is clone them or convert them to xml
	class metaValue_base
	{
	public:
		virtual ~metaValue_base(){}
		virtual metaValue_base* clone() const throw() = 0;
		virtual metaValue_base* clone(const std::vector<uniString::utf8> &s) const = 0;
		virtual uniString::utf8 toXML(const std::string &tag) const throw() = 0;
		virtual uniString::utf8 toString() const throw() = 0;
	};

	// templatized subclass of metaValue_base. You instantiate this with a class or
	// struct of your creation that represents the specific data for a tag.
	// The most interesting part is the constructor. You must provide
	// an overload in the ID3V2 space (see ID3miniParsers.h) of the function
	// fromStringList() which loads your struct from a list of unicode strings. A string
	// list is adequate to represent all data coming out of ID3V2. Even binary hunks which
	// get base64 encoded before being passed in a list to your constructor.
	template<typename T>
	class metaValue: public metaValue_base
	{
		T								m_value;
		std::vector<uniString::utf8>	m_originalStringList;

	public:
		metaValue() {}
		metaValue(const T &v, const std::vector<uniString::utf8> &slist) : m_value(v), m_originalStringList(slist) {}
		metaValue(const std::vector<uniString::utf8> &slist) : m_originalStringList(slist)
		{
			ID3V2::fromStringList(slist,m_value);
		}
		virtual metaValue_base* clone() const throw()
				{ return new metaValue<T>(m_originalStringList); }
		virtual metaValue_base* clone(const std::vector<uniString::utf8> &slist) const // clone with new data
				{ return new metaValue<T>(slist); }
		virtual uniString::utf8 toString() const throw()
				{ return ID3V2::toString(m_value); }
		virtual uniString::utf8 toXML(const std::string &tag) const throw()
				{ return ID3V2::toXML(uniString::utf8(tag),m_value); }
		const T& value() const throw() { return m_value; }
	};

private:
	typedef std::multimap<std::string,metaValue_base*> keyValueMap_t;
	keyValueMap_t m_keyValueMap;

	// make a copy of this object. Necessary because data is allocated on heap
	void copy(const metadata &m) throw();

public:
	metadata() { m_keyValueMap.clear(); }
	// force our private copy() to be called in all copy scenarios
	metadata(const metadata &m);
	metadata& operator=(const metadata &m);
	/////////

	~metadata() throw();
	void clear() throw();
	bool empty() const throw() { return m_keyValueMap.empty(); }

	// return true if a particular value exists at least once
	bool valueExists(const std::string &key) const throw();
	// note: setValue() takes ownership of the 'value' parameter. This will
	// add another key/value pair. Since we have a multimap, existing values of "key" are
	// not replaced.
	void setValue(const std::string &key,metaValue_base* value) throw();
	void removeValue(const std::string &key) throw(); // remove all values for key

	// only returns first one found
	const metaValue_base* getValue(const std::string &key) const throw();

	// returns a string representation of first instance found
	uniString::utf8 getValueAsString(const std::string &key) const throw();

	// V1 mappings
	static const std::string& NAME() throw()	{ static const std::string k("TIT2"); return k; }
	static const std::string& ARTIST() throw()	{ static const std::string k("TPE1"); return k; }
	static const std::string& ALBUM() throw()	{ static const std::string k("TALB"); return k; }
	static const std::string& YEAR() throw()	{ static const std::string k("TYER"); return k; }
	static const std::string& COMMENT() throw() { static const std::string k("COMM"); return k; }
	static const std::string& GENRE() throw()	{ static const std::string k("TCON"); return k; }

	// other mappings (flac/ogg/internal etc)
	static const std::string& COMPOSER() throw()		{ static const std::string k("TCOM"); return k; }
	static const std::string& PUBLISHER() throw()		{ static const std::string k("TPUB"); return k; }
	static const std::string& TRACKNUMBER() throw()		{ static const std::string k("TRCK"); return k; }
	static const std::string& DISKNUMBER() throw()		{ static const std::string k("TPOS"); return k; }
	static const std::string& ALBUMARTIST() throw()		{ static const std::string k("TPE2"); return k; }
	static const std::string& BAND() throw()			{ static const std::string k("TPE2"); return k; }
	static const std::string& PERFORMER() throw()		{ static const std::string k("TPE2"); return k; }
	static const std::string& CONDUCTOR() throw()		{ static const std::string k("TPE3"); return k; }
	static const std::string& BEATSPERMINUTE() throw()	{ static const std::string k("TBPM"); return k; }
	static const std::string& LYRICS() throw()			{ static const std::string k("USLT"); return k; }
	static const std::string& ENCODERSETTINGS() throw()	{ static const std::string k("TSSE"); return k; }
	static const std::string& RATING() throw()			{ static const std::string k("POPM"); return k; }
	static const std::string& PICTURE() throw()			{ static const std::string k("APIC"); return k; }
	static const std::string& CUSTOMTEXT() throw()		{ static const std::string k("TXXX"); return k; }
	static const std::string& VERSION() throw()			{ static const std::string k("TPE4"); return k; }
	static const std::string& COPYRIGHT() throw()		{ static const std::string k("TCOP"); return k; }
	static const std::string& LICENSE() throw()			{ static const std::string k("TOWN"); return k; }
	static const std::string& ISRC() throw()			{ static const std::string k("TSRC"); return k; }

	static const std::string& DJ() throw()	{ static const std::string k("DJ"); return k; }
	static const std::string& URL() throw()	{ static const std::string k("URL"); return k; }

	// output xml. "safe" functions deal with null value scenario without barfing
	static uniString::utf8 safeXML(const std::string &tag,const metaValue_base *m) throw();
	static uniString::utf8 safeString(const metaValue_base *m) throw();

	uniString::utf8 toXML() const throw();

	static uniString::utf8 toXML_fromFilename(const uniFile::filenameType &filename,
											  const uniFile::filenameType &url,
											  const uniString::utf8 &pattern) throw();
	static uniString::utf8 toFixedString(const uniFile::filenameType &filename) throw();

	bool get_replayGain(double &gain) const throw(); // return true if value found
	bool get_replayGain() const throw(); // same, but just the return value
	bool noMeaningfulMetadata() const throw(); // return true if metadata is empty, or stuff in there is not
												// useful for user (like replaygain).

	typedef size_t streamID_t;
	static uniString::utf8 convert_3902_to_shoutcast1(const uniString::utf8 &d, const streamID_t id) throw(std::runtime_error);
	static uniString::utf8 get_song_title_from_3902(const uniString::utf8 &d);
	static uniString::utf8 get_XX_from_3902(const uniString::utf8& node, const uniString::utf8 &d, const uniString::utf8 &old) throw(std::runtime_error);
	static std::vector<uniString::utf8> get_nextsongs_from_3902(const uniString::utf8 &d, std::vector<uniString::utf8>& oldSongList, bool first = false) throw(std::runtime_error);
};

extern const uniString::utf8 METADATA;
extern const uniString::utf8 E_METADATA;

#endif
