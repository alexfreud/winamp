#pragma once

#ifndef _WA_FILE_HEADER
#define _WA_FILE_HEADER

#include <stdio.h>

#include <sys/stat.h>
#include <string>
#include <string.h>
#include <algorithm>
#include <cstring>
#include <cstdarg>
#include <cwchar>
#include <wchar.h>
#include <fstream>
#include <io.h>
#include <iostream>
#include <streambuf>

#include <vector>
#include <map>

#include <cstddef>
#include <sys/stat.h>

#define DEFAULT_STR_BUFFER_SIZE 1024

//namespace fs = std::experimental::filesystem;

namespace wa
{
	namespace files
	{
		bool file_exists( const char *p_filename );
		bool file_exists( const std::string &p_filename );
		bool file_exists( const wchar_t *p_filename );

		int  file_size( const char *p_filename );
		int  file_size( const wchar_t *p_filename );

		bool folder_exists( const char* p_folder );

		bool getFilenamesFromFolder( std::vector<std::string> &p_result, const std::string &p_folder_path, const std::string &p_reg_ex, const size_t p_limit = 100 );
	}

	namespace strings
	{
		namespace convert
		{
			//
			//  to const char*
			//
			const char* to_char( const wchar_t *p_message );
			const char* to_char( const std::wstring p_message );


			//
			//  to const wchar_t*
			//
			const wchar_t *to_wchar( const char *p_message );


			//
			//  to std::string
			//
			std::string to_string( const char *p_message );
			std::string to_string( const wchar_t *p_message );
			std::string to_string( const std::wstring p_message );


			//
			//  to std::wstring
			//
			std::wstring to_wstring( const char *p_message );
			std::wstring to_wstring( const wchar_t *p_message );
			std::wstring to_wstring( const std::string p_message );

		}

		void replace( std::string &p_string, const std::string &p_from, const std::string &p_to );
		void replace( char *p_string, const char *p_from, const char *p_to );
		void replace( wchar_t *p_string, const wchar_t *p_from, const wchar_t *p_to );

		void replaceAll( std::string &p_string, const std::string &p_from, const std::string &p_to );
		void replaceAll( char *p_string, const char *p_from, const char *p_to );
		void replaceAll( wchar_t *p_string, const wchar_t *p_from, const wchar_t *p_to );

		std::string create_string( const char *Format, ... );
		std::string create_string( const wchar_t *Format, ... );
		std::string create_string( const std::string Format, ... );

		class wa_string
		{
		public:
			wa_string( const char *p_initial = NULL );
			wa_string( const wchar_t *p_initial = NULL );
			wa_string( const std::string &p_initial );
			wa_string( const std::wstring &p_initial );

			void operator = ( const char *p_value );
			void operator = ( const wchar_t *p_value );
			void operator = ( const std::string &p_value );
			void operator = ( const std::wstring &p_value );

			bool operator == ( const char *p_value ) const;
			bool operator == ( const wchar_t *p_value ) const;
			bool operator == ( const std::string &p_value ) const;
			bool operator == ( const std::wstring &p_value ) const;

			bool operator != ( const char *p_value ) const;
			bool operator != ( const wchar_t *p_value ) const;
			bool operator != ( const std::string &p_value ) const;
			bool operator != ( const int p_value ) const;

			wa_string operator + ( const char *p_value );
			wa_string operator + ( const wchar_t *p_value );
			wa_string operator + ( const std::string &p_value );
			wa_string operator + ( const std::wstring &p_value);
			wa_string operator + ( const int p_value );

			wa_string append( const char *p_value );
			wa_string append( const wchar_t *p_value );
			wa_string append( const std::string &p_value );
			wa_string append( const std::wstring &p_value );
			wa_string append( const wa_string p_value);
			wa_string append( const int p_value );

			const std::string   GetA() const;
			const std::wstring  GetW() const;

			void  clear()                                             { _wa_string.clear(); }

			unsigned int lengthA() const;
			unsigned int lengthW() const;
			unsigned int lengthS() const;
			unsigned int lengthWS() const;

			bool contains( const char *p_value );
			bool contains( const wchar_t *p_value );
			bool contains( const std::string &p_value );
			bool contains( const std::wstring &p_value );

			wa_string replace( const char *p_from, const char *p_to );
			wa_string replace( const wchar_t *p_from, const wchar_t *p_to );
			wa_string replace( const std::string &p_from, const std::string &p_to );
			wa_string replace( const std::wstring &p_from, const std::wstring &p_to );

			wa_string replaceAll( const char *p_from, const char *p_to );
			wa_string replaceAll( const wchar_t *p_from, const wchar_t *p_to );
			wa_string replaceAll( const std::string &p_from, const std::string &p_to );
			wa_string replaceAll( const std::wstring &p_from, const std::wstring &p_to );

			bool      startsWith( const char *l_head ) const;
			bool      startsWith( const wchar_t *l_head ) const;
			bool      startsWith( const std::string &l_head ) const;
			bool      startsWith( const std::wstring &l_head ) const;

			bool      endsWith( const char *l_tail ) const;
			bool      endsWith( const wchar_t *l_tail ) const;
			bool      endsWith( const std::string &l_tail ) const;
			bool      endsWith( const std::wstring &l_tail ) const;

			int       findFirst( const char *l_text ) const;
			int       findFirst( const wchar_t *l_text ) const;
			int       findFirst( const std::string &l_text ) const;
			int       findFirst( const std::wstring &l_text ) const;

			int       findLast( const char *l_text ) const;
			int       findLast( const wchar_t *l_text ) const;
			int       findLast( const std::string &l_text ) const;
			int       findLast( const std::wstring &l_text ) const;

			int       find( const std::wstring &l_text ) const;

			std::wstring mid( const int p_start, const int p_length ) const;

			bool      empty() const                                   { return _wa_string.empty(); }

			wa_string toUpper();


		private:
			std::wstring _wa_string;

		};
	}

	namespace bits_operation
	{
		unsigned char* GetBits(unsigned char* Source, unsigned int NbrOfBits, unsigned int* BufferSize);
		wa::strings::wa_string PrintInBinary(unsigned char* buffer, unsigned int size);
	}
}

#endif // !_WA_FILE_HEADER

