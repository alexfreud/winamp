#ifndef ID3miniParsers_H_
#define ID3miniParsers_H_

#include "unicode/uniString.h"
#include <vector>
#include <stdexcept>

// parsers for complex ID3V2 fields (like genre)

namespace ID3V2
{
	void base64encode(const char *in,size_t siz,std::string &out) throw();

	////////////////////
	/* Extracts strings based on a coded formatter
	
		e - single byte encoding. Will be used for all subsequence encoding based strings
		u - non-encoded latin-1 string 
		s - encoded string
		b - binary coded data to end of block
		l - three char language code
		c - single byte code
		y - dynamic counter (value in popularimeter)
		
		modifiers:
		
			+ - repeats to end of block
	*/

	std::vector<uniString::utf8> extractor(const std::vector<__uint8> &data,const char *format) throw(std::exception);
	//////////////////////
	//// structs for various tag types
	struct genreEntry
	{
		uniString::utf8 m_genreCode;
		uniString::utf8 m_refinement;
	};

	struct comment_t
	{
		uniString::utf8 m_languageCode;
		uniString::utf8 m_id;
		uniString::utf8 m_comment;
	};

	struct userUrl_t
	{
		uniString::utf8 m_id;
		uniString::utf8 m_url;
	};

	struct userText_t
	{
		uniString::utf8 m_id;
		uniString::utf8 m_text;
	};

	struct popularimeter_t
	{
		uniString::utf8 m_email;
		uniString::utf8 m_rating;
		uniString::utf8 m_counter;
	};

	struct unsyncLyrics_t
	{
		uniString::utf8 m_languageCode;
		uniString::utf8 m_id;
		uniString::utf8 m_lyrics;
	};

	struct picture_t
	{
		uniString::utf8 m_mimeType;
		uniString::utf8 m_pictureType;
		uniString::utf8 m_id;
		uniString::utf8 m_pictureData;
	};

	struct mcdi_t // CD TOC
	{
		uniString::utf8 m_cdTOC;
	};

	struct ufid_t // universal file id
	{
		uniString::utf8 m_id;
		uniString::utf8 m_data;
	};

	struct part_t // TRCK or TPOS (ie 3/7)
	{
		uniString::utf8 m_part;
		uniString::utf8 m_total;
	};

	struct geob_t	// general encapsulated object
	{
		uniString::utf8 m_mimeType;
		uniString::utf8 m_filename;
		uniString::utf8 m_id;
		uniString::utf8 m_data;
	};

	struct priv_t	// private frame
	{
		uniString::utf8 m_id;
		uniString::utf8 m_data;
	};

	struct timestamp_t
	{
		uniString::utf8 m_year;
		uniString::utf8 m_month;
		uniString::utf8 m_day;
		uniString::utf8 m_hour;
		uniString::utf8 m_minute;
		uniString::utf8 m_second;
	};

	typedef std::vector<ID3V2::genreEntry> genreList_t;
	typedef std::vector<uniString::utf8> stringList_t;
	typedef uniString::utf8 string_t;

	// parser and extractor overloads for the various data types
	/////
	void fromStringList(const std::vector<uniString::utf8> &s,genreList_t &l) throw(std::runtime_error);
	uniString::utf8 toXML(const uniString::utf8 &tag,const genreList_t &l) throw();
	uniString::utf8 toString(const genreList_t &l) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,stringList_t &l) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const stringList_t &l) throw();
	uniString::utf8 toString(const stringList_t &l) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,comment_t &c) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const comment_t &c) throw();
	uniString::utf8 toString(const comment_t &c) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,userUrl_t &c) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const userUrl_t &c) throw();
	uniString::utf8 toString(const userUrl_t &c) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,userText_t &t) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const userText_t &t) throw();
	uniString::utf8 toString(const userText_t &t) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,popularimeter_t &p) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const popularimeter_t &p) throw();
	uniString::utf8 toString(const popularimeter_t &p) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,unsyncLyrics_t &c) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const unsyncLyrics_t &c) throw();
	uniString::utf8 toString(const unsyncLyrics_t &c) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,picture_t &c) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const picture_t &c) throw();
	uniString::utf8 toString(const picture_t &c) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,mcdi_t &m) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const mcdi_t &m) throw();
	uniString::utf8 toString(const mcdi_t &m) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,ufid_t &u) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const ufid_t &u) throw();
	uniString::utf8 toString(const ufid_t &u) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,part_t &u) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const part_t &u) throw();
	uniString::utf8 toString(const part_t &u) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,geob_t &g) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const geob_t &g) throw();
	uniString::utf8 toString(const geob_t &g) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,priv_t &p) throw();
	uniString::utf8 toXML(const uniString::utf8 &tag,const priv_t &p) throw();
	uniString::utf8 toString(const priv_t &p) throw();

	void fromStringList(const std::vector<uniString::utf8> &s,timestamp_t &t) throw(std::runtime_error);
	uniString::utf8 toXML(const uniString::utf8 &tag,const timestamp_t &t) throw();
	uniString::utf8 toString(const timestamp_t &t) throw();
}

#endif
