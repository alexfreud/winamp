#pragma once
#ifndef uniString_H_
#define uniString_H_

#include "intTypes.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace uniString
{
	// exception for bad utf8/utf16 conversions
	class badUnicodeData: public std::runtime_error
	{
	public:
		explicit badUnicodeData(const std::string &msg) : std::runtime_error(msg){}
	};

	#if __BYTE_ORDER == __LITTLE_ENDIAN
		static const bool leSystem = true;
	#else
		static const bool leSystem = false;
	#endif

	class utf8;
	class utf16;
	class utf32;

	template<typename S>
	size_t strlen(const S *s) throw()
	{
		if (!s) return 0;
		size_t len(0);
		while (*(s++))
		{
			++len;
		}
		return len;
	}

	// string length calculator with a limit (in case the string is badly formed)
	template<typename S>
	size_t strlen(const S *s, size_t maxamt) throw()
	{
		if (!s) return 0;
		size_t len(0);
		while (maxamt && (*s))
		{
			++s;
			++len;
			--maxamt;
		}
		return len;
	}

	template<typename T>
	bool is_a_number(T t)
	{
		return (((int)t) >= ((int)'0') && ((int)t) <= ((int)'9'));
	}

	template <typename I>
	bool is_a_number(I ibegin,I iend)
	{
		for (I i = ibegin; i != iend; ++i)
		{
			if (!is_a_number(*i))
			{
				return false;
			}
		}
		return true;
	}

	class utf16: public std::basic_string<__uint16>
	{
	public:
		typedef std::basic_string<__uint16> base_t;

		utf16(){}
		utf16(const utf16 &u16) : base_t(u16){}
		explicit utf16(const utf16::value_type *u16) : base_t(u16){}
		explicit utf16(const utf16::value_type *u16, utf16::size_type len) : base_t(u16, len){}
	#ifdef _WIN32
		explicit utf16(const std::wstring &w) : base_t(w.begin(), w.end()){}
	#endif
		explicit utf16(utf16::size_type Cnt,utf16::value_type Val) : base_t(Cnt, Val){}
		explicit utf16(const base_t &u16) : base_t(u16){}

		void assign(const utf32 &u32,bool leadingBOM = true, bool littleEndian = leSystem) throw(); 
		utf16 escapeXML() const throw();
	};

	class utf8: public std::basic_string<__uint8>
	{
	public:
		typedef std::basic_string<__uint8> base_t;
		utf8(){}

		utf8(const utf8 &u8) : base_t(u8) {}

		utf8(const utf8::value_type *u8) : base_t(u8) {}

		explicit utf8(const utf8::value_type *u8, utf8::size_type len) : base_t(u8, len) {}

		utf8(const char *u8) : base_t((const utf8::value_type *)u8) {}

		explicit utf8(const char *u8,utf8::size_type len) : base_t((const utf8::value_type *)u8, len) {}

		utf8(const std::string &s) : base_t(s.begin(), s.end()) {}

		explicit utf8(utf8::size_type Cnt, utf8::value_type Val) : base_t(Cnt, Val) {}

		utf8(const base_t &u8) : base_t(u8) {}

		template<typename T>
		explicit utf8(T ibegin, T iend) : base_t(ibegin, iend) {}

		explicit utf8(const utf32 &u32, bool leadingBOM = false)
		{
			assign(u32, leadingBOM);
		}

		void assign(const utf32 &u32, bool leadingBOM = false) throw();
		bool isValid(bool allowIncompleteEndingSequence = false) const throw();
		utf8 escapeXML() const throw();
		int toInt() const throw();

		// sometimes we need to stuff this in a string. For example if we want to throw
		// a runtime_error exception with the utf8 data.
		std::string hideAsString() const throw() 
		{
			std::string s(begin(), end());
			return s;
		}

		// remove code points above 0x7f
		std::string toANSI(bool allowHighBitCodePoints = false) const throw();
		// convenience function for windows
	#ifdef _WIN32
		// converts to utf16
		std::wstring toWString() const throw(badUnicodeData);
	#endif

		utf8 operator+(const utf8 &u8) const throw()
		{
			utf8 result(*this);
			result.insert(result.end(), u8.begin(), u8.end());
			return result;
		}

		utf8 operator+(const char *s) const throw()
		{
			utf8 result(*this);
			result.insert(result.end(), s, s + strlen(s));
			return result;
		}

		utf8 operator+(const std::string &s) const throw()
		{
			utf8 result(*this);
			result.insert(result.end(), s.begin(), s.end());
			return result;
		}

		template<typename T>
		utf8 operator+(T v) const throw()
		{
			utf8 result(*this);
			result += v;
			return result;
		}

		template<typename T>
		utf8& operator+=(T v) throw()
		{
			base_t::operator+=(v);
			return *this;
		}

		utf8& operator+=(const std::string &s) throw()
		{
			utf8 result = (*this) + s;
			*this = result;
			return *this;
		}

		utf8& operator+=(const char *s) throw()
		{
			utf8 result = (*this) + s;
			*this = result;
			return *this;
		}

		template<typename ITER>
		bool equals(ITER ibegin, ITER iend) const throw()
		{
			const_iterator i1 = begin();
			ITER i2 = ibegin;
			while ((i1 != end()) && (i2 != iend))
			{
				if (((utf8::value_type)*i1) != ((utf8::value_type)*i2))
				{
					return false;
				}
				++i1;
				++i2;
			}
			return ((i1 == end()) && (i2 == iend));
		}

		bool operator==(const char *s) const throw()
		{
			return equals(s, s + strlen(s));
		}

		bool operator==(const std::string &s) const throw()
		{
			return equals(s.begin(), s.end());
		}
	};

	inline utf8 operator+(const char *s, const utf8 &u8) throw()
	{
		utf8 r(s);
		r.insert(r.end(), u8.begin(), u8.end());
		return r;
	}

	inline utf8 operator+(const std::string &s, const utf8 &u8) throw()
	{
		utf8 r(s);
		r.insert(r.end(), u8.begin(), u8.end());
		return r;
	}

	class utf32: public std::basic_string<__uint32>
	{
	public:
		typedef std::basic_string<__uint32> base_t;

		utf32(){}
		utf32(const utf32 &us): base_t(us){}
		utf32(const utf32::value_type *u32) : base_t(u32){}
		explicit utf32(const utf32::value_type *u32, utf32::size_type len) : base_t(u32, len){}
		explicit utf32(utf32::size_type Cnt, utf32::value_type Val) : base_t(Cnt, Val){}
		utf32(const base_t &s) : base_t(s){}
		explicit utf32(utf32::const_iterator b, utf32::const_iterator e) : base_t(b, e){}
		explicit utf32(utf32::const_reverse_iterator b, utf32::const_reverse_iterator e) : base_t(b, e){}

		explicit utf32(const __int8 *s, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		explicit utf32(const __int8 *s, size_t len, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		explicit utf32(const std::string &s, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		explicit utf32(const utf8 &u8, bool allowIncompleteEndingSequence = false) throw(badUnicodeData); 
		explicit utf32(const utf8::value_type *u8, bool allowIncompleteEndingSequence = false) throw(badUnicodeData); 
		explicit utf32(const utf8::value_type *u8, size_t len, bool allowIncompleteEndingSequence = false) throw(badUnicodeData); 
		void assignAsHighBitANSI(const utf8::value_type *u8) throw();
		void assign(const __int8 *s, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		void assign(const __int8 *s, size_t len, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		void assign(const std::string &s, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		void assign(const utf8 &u8, bool allowIncompleteEndingSequence = false) throw(badUnicodeData); 
		void assign(const utf8::value_type *u8, bool allowIncompleteEndingSequence = false) throw(badUnicodeData); 
		void assign(const utf8::value_type *u8, size_t len, bool allowIncompleteEndingSequence = false) throw(badUnicodeData); 

		// use this one if the source is LATIN-1 and it may contain high bit characters
		void assignFromLatinExtended(const std::string &s) throw();
		void assignFromLatinExtended(const __uint8 *s) throw(); // null terminated
		void assignFromLatinExtended(const __uint8 *s, size_t len) throw(); // no termination
		//////////////////

		explicit utf32(const utf16 &u16, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		explicit utf32(const utf16::value_type *u16, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		explicit utf32(const utf16::value_type *u16, size_t len, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
	#ifdef _WIN32
		explicit utf32(const std::wstring &w, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		explicit utf32(const wchar_t *u16, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		explicit utf32(const wchar_t *u16, size_t len, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
	#endif
		void assign(const utf16 &u16, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		void assign(const utf16::value_type *u16, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		void assign(const utf16::value_type *u16, size_t len, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
	#ifdef _WIN32
		void assign(const std::wstring &w, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		void assign(const wchar_t *u16, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
		void assign(const wchar_t *u16, size_t len, bool assumeLittleEndian = leSystem, bool allowIncompleteEndingSequence = false) throw(badUnicodeData);
	#endif
		utf8 toUtf8(bool leadingBOM = false) const throw();
		void toUtf8(utf8 &u8, bool leadingBOM = false) const throw();
		void toUtf8(std::string &s, bool leadingBOM = false) const throw();
		utf16 toUtf16(bool leadingBOM = false, bool littleEndian = leSystem) const throw();
		void toUtf16(utf16 &u16, bool leadingBOM = false, bool littleEndian = leSystem) const throw();
	#ifdef _WIN32
		void toUtf16(std::wstring &w, bool leadingBOM = false, bool littleEndian = leSystem) const throw();
	#endif

		utf32 escapeXML() const throw();

		utf32& operator+=(const utf32 &u32)
		{
			insert(end(), u32.begin(), u32.end());
			return *this;
		}

		utf32 operator+(const utf32 &u32)
		{
			utf32 result(*this);
			result += u32;
			return result;
		}
	};

	static const utf16::value_type Utf16BOM = 0xfeff;
}

inline uniString::utf8 asciiToUtf8(std::string s)
{
	uniString::utf8 result;
	const size_t siz = s.size();
	for (size_t i = 0; i < siz; i++)
	{
		int c = s[i];

		// nothing specicial to do if 0-127
		if (c < 128 && c >= 0)
		{
			if (c >= 0 && c <= 31)
			{
				if (c == 9 || c == 10 || c == 13)
				{
					result.push_back((__uint8)c);
				}
			}
			else
			{
				result.push_back((__uint8)c);
			}
		}
		// otherwise we need to attempt to convert
		else
		{
			// bump back to +ve
			if (c < 128)
			{
				c += 256;
			}
			result.push_back((__uint8)(c >> 6) | 0xC0);
			result.push_back((__uint8)(c & 0x3F) | 0x80);
		}
	}

	return result;
}

#endif
