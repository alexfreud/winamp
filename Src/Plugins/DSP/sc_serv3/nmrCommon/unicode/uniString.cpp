#include "uniString.h"
#include <sstream>
#include <map>
#include <stdlib.h>

using namespace std;
using namespace uniString;

/*****************************************************************/
//////////////// various helping funcs /////////////////////////

// helper function that throws exceptions
static void throwBadUTF8Code(utf8::value_type v, int position) throw(badUnicodeData)
{
	ostringstream o;
	o << "Bad UTF-8 code (" << hex << (int)v << ") at position " << position;
	throw uniString::badUnicodeData(o.str());
}

static void throwBadUTF16Code(utf16::value_type v, int position) throw(badUnicodeData)
{
	ostringstream o;
	o << "Bad UTF-16 code (" << hex << (int)v << ") at position " << position;
	throw uniString::badUnicodeData(o.str());
}

bool utf8::isValid(bool allowIncompleteEndingSequence) const throw()
{
	int position = 0;
	for (utf8::const_iterator i = begin(); i != end(); ++i, ++position)
	{
		utf8::value_type v8 = (*i);
		if (v8 & 0x80)
		{
			// count number of follow up bytes
			utf8::value_type follow_up_mask = 0xc0;
			utf8::value_type leading_value_mask = 0x3f;
			int follow_up_bytes = 0;

			while ((v8 & follow_up_mask) == follow_up_mask)
			{
				if (follow_up_mask == 0xff)
				{
					return false;
				}

				++follow_up_bytes;
				follow_up_mask = (follow_up_mask >> 1) | 0x80;
				leading_value_mask = leading_value_mask >> 1;
			}

			// we should always have follow up bytes since 0x80 is illegal
			if (!follow_up_bytes)
			{
				return false;
			}

			utf32::value_type v = v8 & leading_value_mask;
			while(follow_up_bytes--)
			{
				++i;
				++position;
				if (i == end())
				{
					if (allowIncompleteEndingSequence)
					{
						break;
					}
					else
					{
						return false;
					}
				}
				v8 = *i;
				if ((v8 & 0xc0) != 0x80) // follow ups must begin with 10xxxxxx
				{
					return false;
				}
				v = (v << 6) | (v8 & 0x3f);
			}
		}
	}
	return true;
}

// convert utf8 to utf32 
/*
	u32 - utf32 string to set
	ibegin,iend - template iterators for the beginning and end of the UTF-8 bitstream
	allowIncompleteEndingSequence - if true then we just ignore missing values at the very end
			of the bitstream. Otherwise we throw an exception.
*/

template<typename ITER>
static void Utf8ToUtf32(utf32 &u32, ITER ibegin, ITER iend, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	// use temp so an exception leaves this string in tact
	utf32 newValue;
	int position = 0;

	for (ITER i = ibegin; i != iend; ++i,++position)
	{
		utf8::value_type v8 = (*i);
		if (!(v8 & 0x80))
		{
			newValue.push_back(v8);
		}
		else
		{
			// count number of follow up bytes
			utf8::value_type follow_up_mask = 0xc0;
			utf8::value_type leading_value_mask = 0x3f;
			int follow_up_bytes = 0;

			while ((v8 & follow_up_mask) == follow_up_mask)
			{
				if (follow_up_mask == 0xff)
				{
					throwBadUTF8Code(v8, position);
				}
				++follow_up_bytes;
				follow_up_mask = (follow_up_mask >> 1) | 0x80;
				leading_value_mask = leading_value_mask >> 1;
			}

			// we should always have follow up bytes since 0x80 is illegal
			if (!follow_up_bytes)
			{
				throwBadUTF8Code(v8, position);
			}

			utf32::value_type v = v8 & leading_value_mask;
			while (follow_up_bytes--)
			{
				++i;
				++position;
				if (i == iend)
				{
					if (allowIncompleteEndingSequence)
					{
						break;
					}
					else
					{
						throw badUnicodeData("Bad UTF-8 data. Ending sequence is incomplete");
					}
				}
				v8 = *i;
				if ((v8 & 0xc0) != 0x80) // follow ups must begin with 10xxxxxx
				{
					throwBadUTF8Code(v8, position);
				}
				v = (v << 6) | (v8 & 0x3f);
			}
			if (v != Utf16BOM)
			{
				newValue.push_back(v);
			}
		}
	}

	u32.clear();
	u32 = newValue;
}

///////////////////// byte swap stuff for UTF-16 //////////////////////////////////////////////////////
// endian swap of 16 bit value
static inline utf16::value_type byteSwap(utf16::value_type nValue) throw()
{
	return (((nValue>> 8)) | (nValue << 8));
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
// push a value into a UTF-16 encoding string based on the endian of the value and what
// the machine natively stores. On entry "v" is in the native format of the machine since
// it was just converted from the 32 bit code point.
template<typename T>
static void Utf16EndianPush(T &u16, utf16::value_type v, bool littleEndian) throw()
{
	if (littleEndian)
	{
		u16.push_back(v);
	}
	else
	{
		u16.push_back(byteSwap(v));
	}
}

// convert a UTF-16 value to machine native value
static inline utf16::value_type unswap(utf16::value_type v, bool littleEndianData) throw()
{ 
	return (littleEndianData ? v : byteSwap(v));
}
#else
template<typename T>
static void Utf16EndianPush(T &u16, utf16::value_type v, bool littleEndian) throw()
{
	if (!littleEndian)
	{
		u16.push_back(v);
	}
	else
	{
		u16.push_back(byteSwap(v));
	}
}

static inline utf16::value_type unswap(utf16::value_type v,bool littleEndianData) throw()
{ 
	return (!littleEndianData ? v : byteSwap(v));
}
#endif
///////////////////////////////////

// assign UTF-32 from UTF-16 encoding
/*
	u32 - utf32 string to set
	ibegin,iend - iterators for UTF-16 encoding source
	assumeLittleEndian - assume the UTF-16 encoding is little endian unless a BOM is detected
	allowIncompleteEndingSequence - if true ignore final code point if UTF-16 sequence is incomplete, otherwise
									throw an exception
*/

template<typename ITER>
static void Utf16ToUtf32(utf32 &u32, ITER ibegin, ITER iend, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	bool littleEndianData = assumeLittleEndian;

	utf32 newValue;

	int position = 0;

	for (ITER i = ibegin; i != iend; ++i,++position)
	{
		utf16::value_type w1 = (utf16::value_type)(*i); // yes, use utf16 value even for wstring since we know it's good
		if (w1 == 0xfeff)
		{
			littleEndianData = leSystem;
			continue;
		}
		else if (w1 == 0xfffe)
		{
			littleEndianData = !leSystem;
			continue;
		}

		w1 = unswap(w1,littleEndianData);
		if (w1 < 0xd800 || w1 > 0xdfff)
		{
			newValue.push_back(w1);
		}
		else if (w1 > 0xdbff)
		{
			throwBadUTF16Code(w1,position);
		}
		else
		{
			++i;
			++position;
			if (i == iend)
			{
				if (allowIncompleteEndingSequence)
				{
					break;
				}
				else
				{
					throw badUnicodeData("Bad UTF-16 data. Ending sequence is incomplete");
				}
			}
			utf16::value_type w2 = (*i);
			w2 = unswap(w2, littleEndianData);
			if (w2 < 0xdc00 || w2 > 0xdfff)
			{
				throwBadUTF16Code(w2, position);
			}
			utf32::value_type v1 = w1 & 0x03ff;
			utf32::value_type v2 = w2 & 0x03ff;
			newValue.push_back((v1 << 10) | v2);
		}
	}

	u32.clear();
	u32 = newValue;
}

template<typename U8>
static void Utf32CodeToUtf8(utf32::value_type v, U8 &u8) throw()
{
	if (v < 0x00000080)
	{
		// only allow \t, \r, \n if in 0-31 ranage
		// otherwise expat in the DNAS will refuse
		if (v <= 31)
		{
			if (v == 9 || v == 10 || v == 13)
			{
				u8.push_back((utf8::value_type)v);
			}
		}
		else
		{
			u8.push_back((utf8::value_type)v);
		}
	}
	else
	{
		utf32::value_type maxTopValue = 0x0000003f;
		utf8::value_type  topValueBitPattern = 0x80;
		vector<utf8::value_type> buf;

		// filter out the extended control characters just incase
		if (v >= 0x80 && v <= 0x9F)
		{
			u8.push_back((utf8::value_type)0x3F);
			return;
		}
		// and also filter this so we don't insert BOMs
		else if (v == 0xFFFE || v == 0xFFFF)
		{
			return;
		}

		while (v > maxTopValue)
		{
			buf.push_back(0x00000080 | (v & 0x0000003f));
			v = v >> 6;
			maxTopValue = maxTopValue >> 1;
			topValueBitPattern = ((topValueBitPattern >> 1) | 0x80);
		}

		buf.push_back(topValueBitPattern | v);
		u8.insert(u8.end(), buf.rbegin(), buf.rend());
	}
}

template<typename U8>		
static void Utf32ToUtf8(const utf32 &u32, U8 &u8, bool leadingBOM) throw()
{
	u8.clear();
	if (leadingBOM) 
	{
		// we rarely want a BOM in utf-8. But I've run into template bugs where
		// the utf8 constructor is accidentally getting called with true for the BOM
		// this compile time flag checks for that
		Utf32CodeToUtf8(Utf16BOM,u8);
	}
	for (utf32::const_iterator i = u32.begin(); i != u32.end(); ++i)
	{
		Utf32CodeToUtf8(*i, u8);
	}
}

// create UTF-16 from unicode (according to rfc2781)
template<typename U16>
static void Utf32ToUtf16(const utf32 &u32, U16 &u16, bool leadingBOM, bool littleEndian) throw()
{
	u16.clear();
	if (leadingBOM)
	{
		Utf16EndianPush(u16, Utf16BOM, littleEndian);
	}
	for (utf32::const_iterator i = u32.begin(); i != u32.end(); ++i)
	{
		utf32::value_type v = *i;
		if (v < 0x00010000)
		{
			Utf16EndianPush(u16, v, littleEndian);
		}
		else
		{
			utf32::value_type vp = v - 0x00010000;
			utf16::value_type w1 = 0xd800;
			utf16::value_type w2 = 0xdc00;
			w1 = w1 | ((vp & 0x000ffc00) >> 10);
			w2 = w2 | (vp & 0x000003ff);
			Utf16EndianPush(u16,w1,littleEndian);
			Utf16EndianPush(u16,w2,littleEndian);
		}
	}
}

utf32::utf32(const __int8 *s, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{ 
	Utf8ToUtf32(*this, s, s + strlen((const char *)s), allowIncompleteEndingSequence);
}

utf32::utf32(const __int8 *s, size_t len, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf8ToUtf32(*this, s, s + len, allowIncompleteEndingSequence);
}

utf32::utf32(const std::string &s, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf8ToUtf32(*this, s.begin(), s.end(), allowIncompleteEndingSequence);
}

utf32::utf32(const utf8 &u8, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf8ToUtf32(*this, u8.begin(), u8.end(), allowIncompleteEndingSequence);	
}

utf32::utf32(const utf8::value_type *u8, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf8ToUtf32(*this, u8, u8 + strlen((const char *)u8), allowIncompleteEndingSequence);
}

utf32::utf32(const utf8::value_type *u8,size_t len, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf8ToUtf32(*this, u8, u8 + len, allowIncompleteEndingSequence);
}

utf32::utf32(const utf16 &u16,bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf16ToUtf32(*this, u16.begin(), u16.end(), assumeLittleEndian, allowIncompleteEndingSequence);
}

utf32::utf32(const utf16::value_type *u16, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	utf16::size_type len = 0;
	const utf16::value_type *tmp = u16;
	if (tmp && *tmp)
	{
		while(*(tmp++))
		{
			++len;
		}
	}
	Utf16ToUtf32(*this, u16, u16 + len, assumeLittleEndian, allowIncompleteEndingSequence);
}

utf32::utf32(const utf16::value_type *u16, size_t len, bool assumeLittleEndian ,bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf16ToUtf32(*this, u16, u16 + len, assumeLittleEndian, allowIncompleteEndingSequence);
}

#ifdef _WIN32
utf32::utf32(const std::wstring &w, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf16ToUtf32(*this, w.begin(), w.end(), assumeLittleEndian, allowIncompleteEndingSequence);
}

utf32::utf32(const wchar_t *u16, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	size_t len = 0;
	const wchar_t *tmp = u16;
	if (tmp && *tmp)
	{
		while(*(tmp++))
		{
			++len;
		}
	}
	Utf16ToUtf32(*this,u16,u16+len,assumeLittleEndian,allowIncompleteEndingSequence);
}

utf32::utf32(const wchar_t *u16, size_t len, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf16ToUtf32(*this, u16, u16 + len, assumeLittleEndian, allowIncompleteEndingSequence);
}
#endif		

void utf32::assign(const __int8 *s, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{ 
	Utf8ToUtf32(*this, s, s + strlen((const char *)s), allowIncompleteEndingSequence);
}

void utf32::assign(const __int8 *s, size_t len, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf8ToUtf32(*this, s, s + len, allowIncompleteEndingSequence);
}

void utf32::assign(const std::string &s, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf8ToUtf32(*this, s.begin(), s.end(), allowIncompleteEndingSequence);
}

void utf32::assign(const utf8 &u8, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf8ToUtf32(*this, u8.begin(), u8.end(), allowIncompleteEndingSequence);	
}

void utf32::assign(const utf8::value_type *u8, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf8ToUtf32(*this, u8, u8 + strlen((const char *)u8), allowIncompleteEndingSequence);
}

void utf32::assignAsHighBitANSI(const utf8::value_type *u8) throw()
{
	if (u8)
	{
		while (*u8)
		{
			push_back(*u8);
			++u8;
		}
	}
}

void utf32::assign(const utf8::value_type *u8, size_t len, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf8ToUtf32(*this, u8, u8 + len, allowIncompleteEndingSequence);
}

void utf32::assign(const utf16 &u16, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf16ToUtf32(*this, u16.begin(), u16.end(), assumeLittleEndian, allowIncompleteEndingSequence);
}

void utf32::assign(const utf16::value_type *u16, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	utf16::size_type len = 0;
	const utf16::value_type *tmp = u16;
	if (tmp && *tmp)
	{
		while(*(tmp++))
		{
			++len;
		}
	}
	Utf16ToUtf32(*this, u16, u16 + len, assumeLittleEndian, allowIncompleteEndingSequence);
}

void utf32::assign(const utf16::value_type *u16, size_t len, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf16ToUtf32(*this, u16, u16 + len, assumeLittleEndian, allowIncompleteEndingSequence);
}

#ifdef _WIN32
void utf32::assign(const std::wstring &w, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf16ToUtf32(*this, w.begin(), w.end(), assumeLittleEndian, allowIncompleteEndingSequence);
}

void utf32::assign(const wchar_t *u16, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	size_t len = 0;
	const wchar_t *tmp = u16;
	if (tmp && *tmp)
	{
		while(*(tmp++))
		{
			++len;
		}
	}
	Utf16ToUtf32(*this, u16, u16 + len, assumeLittleEndian, allowIncompleteEndingSequence);
}	

void utf32::assign(const wchar_t *u16, size_t len, bool assumeLittleEndian, bool allowIncompleteEndingSequence) throw(badUnicodeData)
{
	Utf16ToUtf32(*this, u16, u16 + len, assumeLittleEndian, allowIncompleteEndingSequence);
}
#endif		

void utf32::assignFromLatinExtended(const std::string &s) throw()
{
	clear();
	for (string::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		push_back((utf32::value_type)(*i));
	}
}

void utf32::assignFromLatinExtended(const __uint8 *s) throw()
{
	clear();
	if (s)
	{
		while (*s)
		{
			push_back(*(s++));
		}
	}
}

void utf32::assignFromLatinExtended(const __uint8 *s, size_t len) throw()
{
	clear();
	if (s && len > 0)
	{
		while (len--)
		{
			push_back(*(s++));
		}
	}
}

utf8 utf32::toUtf8(bool leadingBOM) const throw()
{
	utf8 u8;
	Utf32ToUtf8(*this, u8, leadingBOM);
	return u8;
}

void utf32::toUtf8(utf8 &u8, bool leadingBOM) const throw()
{
	Utf32ToUtf8(*this, u8, leadingBOM);
}

void utf32::toUtf8(std::string &s, bool leadingBOM) const throw()
{
	Utf32ToUtf8(*this, s, leadingBOM);
}

utf16 utf32::toUtf16(bool leadingBOM, bool littleEndian) const throw()
{
	utf16 u16;
	Utf32ToUtf16(*this, u16, leadingBOM, littleEndian);
	return u16;
}

void utf32::toUtf16(utf16 &u16, bool leadingBOM, bool littleEndian) const throw()
{
	Utf32ToUtf16(*this, u16, leadingBOM, littleEndian);
}

#ifdef _WIN32
void utf32::toUtf16(std::wstring &w, bool leadingBOM, bool littleEndian) const throw()
{
	Utf32ToUtf16(*this, w, leadingBOM, littleEndian);
}
#endif	

namespace uniString
{
	template<typename T>
	class xmlEscapes: public map<typename T::value_type,T>
	{
	public:
		xmlEscapes()
		{  		
			static const typename T::value_type lessthan[] = 
				{(typename T::value_type)'&',(typename T::value_type)'l',(typename T::value_type)'t',(typename T::value_type)';',(typename T::value_type)0};
			static const typename T::value_type greaterthan[] = 
				{(typename T::value_type)'&',(typename T::value_type)'g',(typename T::value_type)'t',(typename T::value_type)';',(typename T::value_type)0};
			static const typename T::value_type ampersand[] = 
				{(typename T::value_type)'&',(typename T::value_type)'a',(typename T::value_type)'m',(typename T::value_type)'p',(typename T::value_type)';',(typename T::value_type)0};
			static const typename T::value_type apostrophe[] = 
				{(typename T::value_type)'&',(typename T::value_type)'a',(typename T::value_type)'p',(typename T::value_type)'o',(typename T::value_type)'s',(typename T::value_type)';',(typename T::value_type)0};
			static const typename T::value_type quote[] = 
				{(typename T::value_type)'&',(typename T::value_type)'q',(typename T::value_type)'u',(typename T::value_type)'o',(typename T::value_type)'t',(typename T::value_type)';',(typename T::value_type)0};
			
			(*this)['<'] = T(lessthan);
			(*this)['>'] = T(greaterthan);
			(*this)['&'] = T(ampersand);
			(*this)['\''] = T(apostrophe);
			(*this)['"'] = T(quote);
		}
	};
}

static const uniString::xmlEscapes<utf32> gUtf32XmlEscapes;
static const uniString::xmlEscapes<utf16> gUtf16XmlEscapes;
static const uniString::xmlEscapes<utf8> gUtf8XmlEscapes;

template<typename T>
static T xml_escape(const T &t, const uniString::xmlEscapes<T> &m) throw()
{
	T result;

	for (typename T::const_iterator i = t.begin(); i != t.end(); ++i)
	{
		typename uniString::xmlEscapes<T>::const_iterator e = m.find(*i);
		if (e != m.end())
		{
			result.insert(result.end(),(*e).second.begin(),(*e).second.end());
		}
		else
		{
			result.push_back(*i);
		}
	}
	return result;
}

utf32 utf32::escapeXML() const throw()
{
	return xml_escape(*this, gUtf32XmlEscapes);
}

utf16 utf16::escapeXML() const throw()
{
	return xml_escape(*this, gUtf16XmlEscapes);
}

utf8 utf8::escapeXML() const throw()
{
	return xml_escape(*this, gUtf8XmlEscapes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void utf8::assign(const utf32 &u32, bool leadingBOM) throw()
{
	u32.toUtf8(*this, leadingBOM);
}

void utf16::assign(const utf32 &u32, bool leadingBOM, bool littleEndian) throw()
{
	u32.toUtf16(*this, leadingBOM, littleEndian);
}

////////////////////////////////////////////////////////////////////////////////////////////
string utf8::toANSI(bool allowHighBitCodePoints) const throw()
{
	string result;
	bool utf32MethodWorked = false;

	try
	{
		// convert to utf32 so we can easily remove code points
		const utf32 u32(*this);
		if (allowHighBitCodePoints)
		{
			for (utf32::const_iterator i = u32.begin(); i != u32.end(); ++i)
			{
				result.push_back((char)*i);
			}
		}
		else
		{
			for (utf32::const_iterator i = u32.begin(); i != u32.end(); ++i)
			{
				if ((*i) <= 0x7f)
				{
					result.push_back((char)*i);
				}
				else
				{
					result.push_back((char)'?');
				}
			}
		}
		utf32MethodWorked = true;
	}
	catch(...)
	{
	}

	// if the string actually has high bit ANSI values (for instance, from a badly
	// formed playlist), we should still do something sensible.
	if (!utf32MethodWorked)
	{
		result.clear();
		if (allowHighBitCodePoints)
		{
			for (utf8::const_iterator i = begin(); i != end(); ++i)
			{
				result.push_back((char)*i);
			}
		}
		else
		{
			for (utf8::const_iterator i = begin(); i != end(); ++i)
			{
				if ((*i) <= 0x7f)
				{
					result.push_back((char)*i);
				}
				else
				{
					result.push_back((char)'?');
				}
			}
		}
	}
	return result;
}

#ifdef _WIN32
wstring utf8::toWString() const throw(badUnicodeData)
{
	utf32 u32(*this);
	wstring result;
	u32.toUtf16(result);
	return result;
}
#endif

int utf8::toInt() const throw()
{
	return ::atoi((*this).hideAsString().c_str());
}
