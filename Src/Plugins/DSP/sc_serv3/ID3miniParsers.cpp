#include "ID3miniParsers.h"
#include "stl/stringUtils.h"
#include <locale>

using namespace ID3V2;
using namespace uniString;
using namespace std;

static const __uint8 E_LATIN1(0);
static const __uint8 E_UTF16(1); // with COM
static const __uint8 E_UTF16BE(2); // big endian no BOM
static const __uint8 E_UTF8(3); // utf8

void ID3V2::base64encode(const char *in,size_t siz,string &out) throw()
{
	char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int shift = 0;
	int accum = 0;

	while (siz)
	{
		accum <<= 8;
		shift += 8;
		accum |= *in++;
		siz--;
		while ( shift >= 6 )
		{
			shift -= 6;
			out.push_back(alphabet[(accum >> shift) & 0x3F]);
		}
	}
	if (shift == 4)
	{
		out.push_back(alphabet[(accum & 0xF)<<2]);
		out.push_back('=');
	}
	else if (shift == 2)
	{
	    out.push_back(alphabet[(accum & 0x3)<<4]);
		out.push_back('=');
		out.push_back('=');
	}
}

// way cool function. Given a hunk of data from an ID3V2 entry, break it into a collection
// of unicode strings based on a format string (sort of like sprintf for ID3V2 data)
vector<utf8> ID3V2::extractor(const vector<__uint8> &data,const char *format) throw(exception)
{
	vector<utf8> result;

	__uint8 encoding = E_LATIN1;

	int bytesRemaining = (int)data.size();
	vector<__uint8>::size_type offset = 0;

	const char *f = format;
	while((*f) && (bytesRemaining > 0))
	{
		switch(*f)
		{
			case ' ': break; // do nothing, just move to next
			case 'e': // encoding
				encoding = data[offset++];
				--bytesRemaining;
			break;

			case 'c': // single digit code
			{
				__uint8 code = data[offset++];
				--bytesRemaining;
				result.push_back(stringUtil::tos((int)code));
			}
			break;

			case 'y': // dynamic counter
			{
				__uint64 counter = 0;
				while(bytesRemaining > 0)
				{
					counter = counter << 8;
					counter += data[offset];
					++offset;
					--bytesRemaining;
				}
				result.push_back(stringUtil::tos(counter));
			}
			break;

			case 'u': // non-encoding string
			{
				size_t slen = uniString::strlen(&(data[offset]),bytesRemaining);
				utf32 u32;
				u32.assignFromLatinExtended(&(data[offset]),slen);
				offset += (slen+1);
				bytesRemaining -= (slen+1);
				result.push_back(u32.toUtf8());
			}
			break;

			case 's': // encoding string
			{
				if (encoding == E_LATIN1)
				{
					size_t slen = uniString::strlen(&(data[offset]),bytesRemaining);
					utf32 u32;
					u32.assignFromLatinExtended(&(data[offset]),slen);
					offset += (slen+1);
					bytesRemaining -= (slen+1);
					result.push_back(u32.toUtf8());
				}
				else if (encoding == E_UTF16)
				{
					const utf16::value_type *v16 = reinterpret_cast<const utf16::value_type*>(&(data[offset]));
					size_t slen = uniString::strlen(v16,bytesRemaining / 2);
					utf32 u32(v16,slen);
					offset += ((slen * 2) + 2);
					bytesRemaining -= ((slen * 2) + 2);
					result.push_back(u32.toUtf8());
				}
				else if (encoding == E_UTF16BE)
				{
					const utf16::value_type *v16 = reinterpret_cast<const utf16::value_type*>(&(data[offset]));
					size_t slen = uniString::strlen(v16,bytesRemaining / 2);
					utf32 u32(v16,slen,false); // assume big endian
					offset += ((slen * 2) + 2);
					bytesRemaining -= ((slen * 2) + 2);
					result.push_back(u32.toUtf8());
				}
				else if (encoding == E_UTF8)
				{
					size_t slen = uniString::strlen(&(data[offset]),bytesRemaining);
					utf32 u32(&(data[offset]),slen);
					offset += (slen+1);
					bytesRemaining -= (slen+1);
					result.push_back(u32.toUtf8());
				}
				else throw runtime_error("unknown encoding " + stringUtil::tos((int)encoding));
			}
			break;

			case 'b': // binary data
			{
				string b64;
				base64encode((const char *)&(data[offset]),bytesRemaining,b64);
				result.push_back(b64);
				offset += bytesRemaining;
				bytesRemaining = 0;
			}
			break;

			case 'l': // three char language code
			{
				if (bytesRemaining < 3) throw runtime_error("not enough data");
				utf8 l;
				bool bad = 
					(data[offset] < '0' || data[offset] > 'z' ||
					data[offset+1] < '0' || data[offset+1] > 'z' ||
					data[offset+2] < '0' || data[offset+2] > 'z');
					
				l.push_back(data[offset++]); l.push_back(data[offset++]); l.push_back(data[offset++]);
				bytesRemaining -= 3;
				if (bad)
					result.push_back(utf8());
				else
					result.push_back(l);				
			}
			break;

			case '+':
				while(f > format && ((*f) == '+' || (*f) == ' ')) --f;
				--f;
				break;
		}
	++f;
	}

	while((!result.empty()) && (result.back().empty())) result.pop_back();
	return result;
}

static const utf8 LT("<");
static const utf8 GT(">");
static const utf8 LTSLASH("</");
static const utf8 EMPTYSTRING;
static const utf8::value_type LPAREN('(');
static const utf8::value_type RPAREN(')');

// genres can be a list of codes and subgenres of high complexity. Parse it all out
// (see ID3V2 specification for genre tag)
//
// some examples
// (21)
// (42) Eurodisco
// (16) (18) Trance
void ID3V2::fromStringList(const vector<utf8> &slist,genreList_t &gl) throw(runtime_error)
{
	gl.clear();

	static const int state_Initial = 0;
	static const int state_LeadingParen = 1;
	static const int state_Code = 2;
	static const int state_Subgenre = 3;
	int state = state_Initial;

	genreEntry e;

	for(vector<utf8>::const_iterator li = slist.begin(); li != slist.end(); ++li)
	{
		for(utf8::const_iterator i = (*li).begin(); i != (*li).end(); ++i)
		{
			switch(state)
			{
				case state_Initial:
					e.m_genreCode = EMPTYSTRING;
					e.m_refinement = EMPTYSTRING;
					if (stringUtil::safe_is_space(*i)) {}
					else if ((*i) == LPAREN)
					{
						state = state_LeadingParen;
					}
					else
					{
						state = state_Subgenre;
						e.m_refinement += (*i);
					}
				break;

				case state_LeadingParen:
					if ((*i) == LPAREN)
					{
						state = state_Subgenre;
						e.m_refinement += (*i);
					}
					else if ((*i) == RPAREN)
					{
						if (!e.m_genreCode.empty() || !e.m_refinement.empty())
							gl.push_back(e);
						e.m_genreCode = EMPTYSTRING;
						e.m_refinement = EMPTYSTRING;
						state = state_Initial;
					}
					else
					{
						if (!e.m_genreCode.empty() || !e.m_refinement.empty())
							gl.push_back(e);
						e.m_genreCode = EMPTYSTRING;
						e.m_refinement = EMPTYSTRING;
						e.m_genreCode += (*i);
						state = state_Code;
					}
				break;

				case state_Code:
					if ((*i) == RPAREN)
					{
						state = state_Subgenre;
					}
					else
					{
						e.m_genreCode += *i;
					}
				break;

				case state_Subgenre:
					if ((*i) == LPAREN)
					{
						state = state_LeadingParen;
					}
					else
					{
						e.m_refinement += (*i);
					}
				break;

				default:
					throw logic_error(string(__FUNCTION__) + " internal error. Bad state");
				break;
			}
		}

		switch(state)
		{
			case state_Initial:
			break;

			case state_LeadingParen:
				throw runtime_error(string(__FUNCTION__) + " badly formed TCON data " + (*li).hideAsString());
			break;

			case state_Code:
				throw runtime_error(string(__FUNCTION__) + " badly formed TCON data " + (*li).hideAsString());
			break;

			case state_Subgenre:
				if (!e.m_genreCode.empty() || !e.m_refinement.empty())
					gl.push_back(e);
			break;

			default:
				throw logic_error(string(__FUNCTION__) + " internal error. Bad state");
			break;
		} // character iteration
	} // end string list iteration
}

uniString::utf8 ID3V2::toXML(const utf8 &tag,const genreList_t &l) throw()
{
	utf8 result;
	utf8 endTag = utf8("</") + tag + utf8(">");

	for(genreList_t::const_iterator i = l.begin(); i != l.end(); ++i)
	{
		result += LT + tag;
		if (!(*i).m_genreCode.empty())
			result += utf8(" v1=\"") + (*i).m_genreCode.escapeXML() + utf8("\"");
		result += GT + (*i).m_refinement.escapeXML() + endTag;
	} 
	return result;
}

uniString::utf8 ID3V2::toString(const genreList_t &l) throw()
{
	for(genreList_t::const_iterator i = l.begin(); i != l.end(); ++i)
	{
		if (!(*i).m_genreCode.empty()) return (*i).m_genreCode;
	}
	return uniString::utf8();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void ID3V2::fromStringList(const vector<uniString::utf8> &s,stringList_t &l) throw()
{
	//l = stringUtil::tokenizer(s,(utf8::value_type)'/');
	l = s;
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const stringList_t &l) throw()
{
	utf8 result;
	utf8 startTag = LT + tag + GT;
	utf8 endTag = LTSLASH + tag + GT;
	for(stringList_t::const_iterator i = l.begin(); i != l.end(); ++i)
	{
		result += startTag + (*i).escapeXML() + endTag;
	} 
	return result;
}

uniString::utf8 ID3V2::toString(const stringList_t &l) throw()
{
	return (l.empty() ? uniString::utf8() : l.front());
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,comment_t &c) throw()
{
	if (s.size() >= 3)
	{
		c.m_languageCode = s[0];
		c.m_id = s[1];
		c.m_comment = s[2];
	}
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const comment_t &c) throw()
{
	utf8 result;
	result += LT + tag;
	if (!c.m_languageCode.empty())
		result += utf8(" language=\"") + c.m_languageCode.escapeXML() + utf8("\"");
	if (!c.m_id.empty())
		result += utf8(" id=\"") + c.m_id.escapeXML() + utf8("\"");
	result += GT + c.m_comment.escapeXML() + LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const comment_t &c) throw()
{
	return c.m_comment;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,userUrl_t &c) throw()
{
	if (!s.empty())
		c.m_id = s[0];
	if (s.size() > 1)
		c.m_url = s[1];
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const userUrl_t &c) throw()
{
	if (c.m_id.empty() && c.m_url.empty()) return utf8();

	utf8 result = LT + tag;
	if (!c.m_id.empty())
		result += utf8(" id=\"") + c.m_id.escapeXML() + utf8("\"");
	result += GT + c.m_url.escapeXML() + LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const userUrl_t &c) throw()
{ return c.m_url; }

/////////////////////////////////////////////////////////////////////////////////////////////////////

void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,userText_t &t) throw()
{
	if (!s.empty())
		t.m_id = s[0];
	if (s.size() > 1)
		t.m_text = s[1];
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const userText_t &t) throw()
{
	if (t.m_id.empty() && t.m_text.empty()) return utf8();

	utf8 result = LT + tag;
	if (!t.m_id.empty())
		result += utf8(" id=\"") + t.m_id.escapeXML() + utf8("\"");
	result += GT + t.m_text.escapeXML() + LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const userText_t &t) throw()
{ return t.m_text; }

/////////////////////////////////////////////////////////////////////////////////////////////

void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,popularimeter_t &p) throw()
{
	vector<utf8>::size_type siz = s.size();

	if (siz > 0)
		p.m_email = s[0];
	if (siz > 1)
		p.m_rating = s[1];
	if (siz > 2)
		p.m_counter = s[2];
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const popularimeter_t &p) throw()
{
	if (p.m_counter.empty() && p.m_email.empty() && p.m_rating.empty()) return utf8();

	utf8 result = LT + tag + GT;

	result += LT + utf8("email") + GT;
	result += p.m_email.escapeXML();
	result += LTSLASH + utf8("email") + GT;

	result += LT + utf8("rating") + GT;
	result += p.m_rating.escapeXML();
	result += LTSLASH + utf8("rating") + GT;

	result += LT + utf8("counter") + GT;
	result += p.m_counter.escapeXML();
	result += LTSLASH + utf8("counter") + GT;

	result += LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const popularimeter_t &p) throw()
{ return p.m_rating; }

/////////////////////////////////////////////////////////////////////////////////////////////////
void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,unsyncLyrics_t &c) throw()
{
	if (s.size() >= 3)
	{
		c.m_languageCode = s[0];
		c.m_id = s[1];
		c.m_lyrics = s[2];
	}
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const unsyncLyrics_t &c) throw()
{
	if (c.m_lyrics.empty()) return utf8();

	utf8 result;
	result += LT + tag;
	if (!c.m_languageCode.empty())
		result += utf8(" language=\"") + c.m_languageCode.escapeXML() + utf8("\"");
	if (!c.m_id.empty())
		result += utf8(" id=\"") + c.m_id.escapeXML() + utf8("\"");
	result += GT + c.m_lyrics.escapeXML() + LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const unsyncLyrics_t &c) throw()
{
	return c.m_lyrics;
}

/////////////////////////////////////////////////////////////////////////////////////////
void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,picture_t &p) throw()
{
	vector<uniString::utf8>::size_type siz = s.size();
	if (siz > 0)	p.m_mimeType = s[0];
	if (siz > 1)	p.m_pictureType = s[1];
	if (siz > 2)	p.m_id = s[2];
	if (siz > 3)	p.m_pictureData = s[3];
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const picture_t &p) throw()
{
	utf8 result;
	result += LT + tag;
	if (!p.m_mimeType.empty())
		result += utf8(" mime=\"") + p.m_mimeType.escapeXML() + utf8("\"");
	if (!p.m_id.empty())
		result += utf8(" id=\"") + p.m_id.escapeXML() + utf8("\"");
	if (!p.m_pictureType.empty())
		result += utf8(" type=\"") + p.m_pictureType.escapeXML() + utf8("\"");
	result += GT + p.m_pictureData.escapeXML() + LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const picture_t &p) throw()
{
	return p.m_pictureData;
}

////////////////////////////////////////////////////////////////////////////////////
void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,mcdi_t &m) throw()
{
	if (!s.empty()) m.m_cdTOC = s[0];
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const mcdi_t &m) throw()
{
	utf8 result;
	result += LT + tag;
	result += GT + m.m_cdTOC.escapeXML() + LTSLASH + tag + GT;
	return result;	
}

uniString::utf8 ID3V2::toString(const mcdi_t &m) throw()
{ return m.m_cdTOC; }
	
////////////////////////////////////////////////////////////////////////////////////

void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,ufid_t &u) throw()
{
	vector<utf8>::size_type siz = s.size();
	if (siz > 0) u.m_id = s[0];
	if (siz > 1) u.m_data = s[1];
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const ufid_t &u) throw()
{
	utf8 result;
	result += LT + tag;
	if (!u.m_id.empty())
		result += utf8(" id=\"") + u.m_id.escapeXML() + utf8("\"");
	result += GT + u.m_data.escapeXML() + LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const ufid_t &u) throw()
{ return u.m_data; }

////////////////////////////////////////////////////////////////////////////////////

void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,part_t &p) throw()
{
	if (!s.empty())
	{
		vector<utf8> l = stringUtil::tokenizer(s[0],(utf8::value_type)'/');
		if (!l.empty()) p.m_part = l[0];
		if (l.size() > 1) p.m_total = l[1];
	}
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const part_t &p) throw()
{
	utf8 result;
	result += LT + tag;
	if (!p.m_total.empty())
		result += utf8(" total=\"") + p.m_total.escapeXML() + utf8("\"");
	result += GT + p.m_part.escapeXML() + LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const part_t &p) throw()
{ return p.m_part; }

////////////////////////////////////////////////////////////////////////////////////

void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,geob_t &g) throw()
{
	vector<utf8>::size_type siz = s.size();
	if (siz > 0) g.m_mimeType = s[0];
	if (siz > 1) g.m_filename = s[1];
	if (siz > 2) g.m_id = s[2];
	if (siz > 3) g.m_data = s[3];
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const geob_t &g) throw()
{
	utf8 result;
	result += LT + tag;
	if (!g.m_mimeType.empty())
		result += utf8(" mime=\"") + g.m_mimeType.escapeXML() + utf8("\"");
	if (!g.m_id.empty())
		result += utf8(" id=\"") + g.m_id.escapeXML() + utf8("\"");
	if (!g.m_filename.empty())
		result += utf8(" filename=\"") + g.m_filename.escapeXML() + utf8("\"");
	result += GT + g.m_data.escapeXML() + LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const geob_t &g) throw()
{ return g.m_data; }

////////////////////////////////////////////////////////////////////////////////////

void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,priv_t &p) throw()
{
	vector<utf8>::size_type siz = s.size();
	if (siz > 0) p.m_id = s[0];
	if (siz > 1) p.m_data = s[1];
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const priv_t &p) throw()
{
	utf8 result;
	result += LT + tag;
	if (!p.m_id.empty())
		result += utf8(" id=\"") + p.m_id.escapeXML() + utf8("\"");
	result += GT + p.m_data.escapeXML() + LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const priv_t &p) throw()
{ return p.m_data; }

////////////////////////////////////////////////////////////////////////////////////

void ID3V2::fromStringList(const std::vector<uniString::utf8> &s,timestamp_t &t) throw(runtime_error)
{
	if (s.empty()) return;

	static const int state_year = 0;
	static const int state_month = 1;
	static const int state_day = 2;
	static const int state_hour = 3;
	static const int state_minute = 4;
	static const int state_second = 5;

	int state = state_year;
	utf8 value;
	for(utf8::const_iterator i = s[0].begin(); i != s[0].end(); ++i)
	{
		switch(state)
		{
			case state_year:
				if (uniString::is_a_number(*i))
				{
					value += *i;
					if (value.size() > 4) throw runtime_error("Year has too many digits");
				}
				else if ((*i) == '-')
				{
					if (value.size() != 4) throw runtime_error("Year has the wrong digit count");
					t.m_year = value;
					value.clear();
					state = state_month;
				}
				else throw runtime_error("Unexpected character while processing year");
			break;

			case state_month:
				if (uniString::is_a_number(*i))
				{
					value += *i;
					if (value.size() > 2) throw runtime_error("Month has too many digits");
				}
				else if ((*i) == '-')
				{
					if (value.empty()) throw runtime_error("No digits for month");
					t.m_month = value;
					value.clear();
					state = state_day;
				}
				else throw runtime_error("Unexpected character while processing month");
			break;

			case state_day:
				if (uniString::is_a_number(*i))
				{
					value += *i;
					if (value.size() > 2) throw runtime_error("Day has too many digits");
				}
				else if ((*i) == 'T')
				{
					if (value.empty()) throw runtime_error("No digits for day");
					t.m_day = value;
					value.clear();
					state = state_hour;
				}
				else throw runtime_error("Unexpected character while processing day");
			break;

			case state_hour:
				if (uniString::is_a_number(*i))
				{
					value += *i;
					if (value.size() > 2) throw runtime_error("Hour has too many digits");
				}
				else if ((*i) == ':')
				{
					if (value.empty()) throw runtime_error("No digits for hour");
					t.m_hour = value;
					value.clear();
					state = state_minute;
				}
				else throw runtime_error("Unexpected character while processing hour");
			break;

			case state_minute:
				if (uniString::is_a_number(*i))
				{
					value += *i;
					if (value.size() > 2) throw runtime_error("Minute has too many digits");
				}
				else if ((*i) == ':')
				{
					if (value.empty()) throw runtime_error("No digits for minute");
					t.m_minute = value;
					value.clear();
					state = state_second;
				}
				else throw runtime_error("Unexpected character while processing minute");
			break;

			case state_second:
				if (uniString::is_a_number(*i))
				{
					value += *i;
					if (value.size() > 2) throw runtime_error("Second has too many digits");
				}
				else throw runtime_error("Unexpected character while processing second");
			break;

			default:
				throw runtime_error(string(__FUNCTION__) + " internal error. Unknown state");
			break;
		}
	}

	if (!value.empty())
	{
		switch(state)
		{
			case state_year:
				if (value.size() != 4) throw runtime_error("Wrong digit count for year");
				t.m_year = value;
			break;

			case state_month:
				if (value.size() > 2) throw runtime_error("Month has too many digits");
				t.m_month = value;
			break;

			case state_day:
				if (value.size() > 2) throw runtime_error("Day has too many digits");
				t.m_day = value;
			break;

			case state_hour:
				if (value.size() > 2) throw runtime_error("Hour has too many digits");
				t.m_hour = value;
			break;

			case state_minute:
				if (value.size() > 2) throw runtime_error("Minute has too many digits");
				t.m_minute = value;
			break;

			case state_second:
				if (value.size() > 2) throw runtime_error("Second has too many digits");
				t.m_second = value;
			break;

			default:
				throw runtime_error(string(__FUNCTION__) + " internal error. Unknown state");
			break;
		}
	}
}

uniString::utf8 ID3V2::toXML(const uniString::utf8 &tag,const timestamp_t &t) throw()
{
	utf8 result;
	if (t.m_year.empty()) return result;
	result += LT + tag + GT;
	if (!t.m_year.empty())	 result += LT + utf8("year")	+ GT + t.m_year.escapeXML()		+ LTSLASH + utf8("year")	+ GT;
	if (!t.m_month.empty())  result += LT + utf8("month")	+ GT + t.m_month.escapeXML()	+ LTSLASH + utf8("month")	+ GT;
	if (!t.m_day.empty())	 result += LT + utf8("day")		+ GT + t.m_day.escapeXML()		+ LTSLASH + utf8("day")		+ GT;
	if (!t.m_hour.empty())	 result += LT + utf8("hour")	+ GT + t.m_hour.escapeXML()		+ LTSLASH + utf8("hour")	+ GT;
	if (!t.m_minute.empty()) result += LT + utf8("minute")	+ GT + t.m_minute.escapeXML()	+ LTSLASH + utf8("minute")	+ GT;
	if (!t.m_second.empty()) result += LT + utf8("second")	+ GT + t.m_second.escapeXML()	+ LTSLASH + utf8("second")	+ GT;
	result += LTSLASH + tag + GT;
	return result;
}

uniString::utf8 ID3V2::toString(const timestamp_t &t) throw()
{ 
	utf8 result;
	if (t.m_year.empty()) return result;
	result += t.m_year;
	if (t.m_month.empty()) return result;
	result += utf8("-") + t.m_month;
	if (t.m_day.empty()) return result;
	result += utf8("-") + t.m_day;
	if (t.m_hour.empty()) return result;
	result += utf8("T") + t.m_hour;
	if (t.m_minute.empty()) return result;
	result += utf8(":") + t.m_minute;
	if (t.m_second.empty()) return result;
	result += utf8(":") + t.m_second;
	return result;
}
