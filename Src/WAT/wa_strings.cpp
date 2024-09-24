#include "WAT.h"

//
//  to const char*
//
const char *wa::strings::convert::to_char( const wchar_t *p_message )
{
	std::wstring l_message_w( p_message );
	std::string  l_message( l_message_w.begin(), l_message_w.end() );

	return _strdup( l_message.c_str() );
}

const char *wa::strings::convert::to_char( const std::wstring p_message )
{
	std::string l_message = wa::strings::convert::to_string( p_message );

	return _strdup( l_message.c_str() );
}


//
//  to const wchar_t*
//
const wchar_t *wa::strings::convert::to_wchar( const char *p_message )
{
	std::string  l_message( p_message );
	std::wstring l_message_w( l_message.begin(), l_message.end() );

	return _wcsdup( l_message_w.c_str() );
}


//
//  to std::string
//
std::string wa::strings::convert::to_string( const char *p_message )
{
	std::string l_message( p_message );

	return l_message;
}

std::string wa::strings::convert::to_string( const wchar_t *p_message )
{
	std::wstring l_message_w( p_message );
	std::string  l_message( l_message_w.begin(), l_message_w.end() );

	return l_message;
}

std::string wa::strings::convert::to_string( const std::wstring p_message )
{
	std::string l_message( p_message.begin(), p_message.end() );

	return l_message;
}


//
//  to std::wstring
//
std::wstring wa::strings::convert::to_wstring( const char *p_message )
{
	std::string  l_message_w( p_message );
	std::wstring l_message( l_message_w.begin(), l_message_w.end() );

	return l_message;
}

std::wstring wa::strings::convert::to_wstring( const wchar_t *p_message )
{
	std::wstring l_message( p_message );

	return l_message;
}

std::wstring wa::strings::convert::to_wstring( const std::string p_message )
{
	std::wstring l_message( p_message.begin(), p_message.end() );

	return l_message;
}


//
// replace
//
void wa::strings::replace( std::string &p_string, const std::string &p_from, const std::string &p_to )
{
	if ( p_from.empty() )
		return;

	size_t start_pos = p_string.find( p_from );

	if ( start_pos == std::string::npos )
		return;

	p_string.replace( start_pos, p_from.length(), p_to );

	return;
}

void wa::strings::replace( char *p_string, const char *p_from, const char *p_to )
{
	std::string l_string( p_string );

	wa::strings::replace( l_string, std::string( p_from ), std::string( p_to ) );

	p_string = (char *)l_string.c_str();
}

void wa::strings::replace( wchar_t *p_string, const wchar_t *p_from, const wchar_t *p_to )
{
	std::string l_string = wa::strings::convert::to_string( p_string );

	wa::strings::replace( l_string, wa::strings::convert::to_string( p_from ), wa::strings::convert::to_string( p_to ) );

	std::wstring l_wstring = wa::strings::convert::to_wstring( l_string );

	p_string = (wchar_t *)( l_wstring.c_str() );
}


//
// replaceAll
//
void wa::strings::replaceAll( std::string &p_string, const std::string &p_from, const std::string &p_to )
{
	if ( p_from.empty() )
		return;

	size_t start_pos = 0;
	while ( ( start_pos = p_string.find( p_from, start_pos ) ) != std::string::npos )
	{
		p_string.replace( start_pos, p_from.length(), p_to );
		start_pos += p_to.length(); // In case 'p_to' contains 'p_from', like replacing 'x' with 'yx'
	}
}

void wa::strings::replaceAll( char *p_string, const char *p_from, const char *p_to )
{
	std::string l_string( p_string );

	wa::strings::replaceAll( l_string, std::string( p_from ), std::string( p_to ) );

	p_string = (char *)l_string.c_str();
}

void wa::strings::replaceAll( wchar_t *p_string, const wchar_t *p_from, const wchar_t *p_to )
{
	std::string l_string = wa::strings::convert::to_string( p_string );

	wa::strings::replaceAll( l_string, wa::strings::convert::to_string( p_from ), wa::strings::convert::to_string( p_to ) );

	std::wstring l_wstring = wa::strings::convert::to_wstring( l_string );

	p_string = (wchar_t *)( l_wstring.c_str() );
}


//
// create_string
//

std::string wa::strings::create_string( const char *Format, ... )
{
	char _log_message_a[ DEFAULT_STR_BUFFER_SIZE ];
	va_list args;
	va_start( args, Format );
	snprintf( _log_message_a, DEFAULT_STR_BUFFER_SIZE, Format, args );

	return std::string( _log_message_a );
}

std::string wa::strings::create_string( const wchar_t *Format, ... )
{
	wchar_t     _log_message_w[ DEFAULT_STR_BUFFER_SIZE ];
	va_list args;
	va_start( args, Format );
	_vsnwprintf( _log_message_w, DEFAULT_STR_BUFFER_SIZE, Format, args );

	return wa::strings::convert::to_string( _log_message_w );
}

std::string wa::strings::create_string( const std::string Format, ... )
{
	va_list args;
	va_start( args, Format );

	return create_string( Format.c_str(), args );
}






//=====================================================================================================================
//
// wa_string::wa_string  (constructor)
//
wa::strings::wa_string::wa_string( const char *p_initial )
{
	if ( p_initial == NULL )
		_wa_string.clear();
	else
		_wa_string = wa::strings::convert::to_wstring( p_initial );	
}

wa::strings::wa_string::wa_string( const wchar_t *p_initial )
{
	if ( p_initial == NULL )
		_wa_string.clear();
	else
		_wa_string = std::wstring( p_initial );
}

wa::strings::wa_string::wa_string( const std::string &p_initial )
{
	_wa_string = wa::strings::convert::to_wstring( p_initial );
}

wa::strings::wa_string::wa_string( const std::wstring &p_initial )
{
	_wa_string = p_initial;
}


//
// wa_string::operator=
//
void wa::strings::wa_string::operator=( const char *p_value )
{
	_wa_string = wa::strings::convert::to_wstring( p_value );
}

void wa::strings::wa_string::operator=( const wchar_t *p_value )
{
	_wa_string = std::wstring( p_value );
}

void wa::strings::wa_string::operator=( const std::string &p_value )
{
	_wa_string = wa::strings::convert::to_wstring( p_value );
}

void wa::strings::wa_string::operator=( const std::wstring &p_value )
{
	_wa_string = p_value;
}


//
// wa_string::operator==
//
bool wa::strings::wa_string::operator==( const char *p_value ) const
{
	std::string l_wa_string( wa::strings::convert::to_string( _wa_string ) );

	return l_wa_string.compare( p_value ) == 0;
}

bool wa::strings::wa_string::operator==( const wchar_t *p_value ) const
{
	return _wa_string.compare( p_value ) == 0;
}

bool wa::strings::wa_string::operator==( const std::string &p_value ) const
{
	std::string l_wa_string( wa::strings::convert::to_string( _wa_string ) );

	return l_wa_string.compare( p_value ) == 0;
}

bool wa::strings::wa_string::operator==( const std::wstring &p_value ) const
{
	return _wa_string.compare( p_value ) == 0;
}


//
// wa_string::operator!=
//
bool wa::strings::wa_string::operator!=( const char *p_value ) const
{
	std::string l_wa_string( wa::strings::convert::to_string( _wa_string ) );

	return l_wa_string.compare( p_value ) != 0;
}

bool wa::strings::wa_string::operator!=( const wchar_t *p_value ) const
{
	return _wa_string.compare( p_value ) != 0;
}

bool wa::strings::wa_string::operator!=( const std::string &p_value ) const
{
	std::string l_wa_string( wa::strings::convert::to_string( _wa_string ) );

	return l_wa_string.compare( p_value ) != 0;
}

bool wa::strings::wa_string::operator!=( const int p_value ) const
{
	return std::to_wstring( p_value ).compare( _wa_string ) != 0;
}


//
// wa_string::operator+
//
wa::strings::wa_string wa::strings::wa_string::operator+( const char *p_value )
{
	_wa_string.append( wa::strings::convert::to_wstring( p_value ) );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::operator+( const wchar_t *p_value )
{
	_wa_string.append( p_value );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::operator+( const std::string &p_value )
{
	_wa_string.append( wa::strings::convert::to_wstring( p_value ) );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::operator+( const std::wstring &p_value )
{
	_wa_string.append( p_value );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::operator+( const int p_value )
{
	append( p_value );

	return *this;
}


//
// wa_string::append
//
wa::strings::wa_string wa::strings::wa_string::append( const char *p_value )
{
	_wa_string.append( wa::strings::convert::to_wstring( p_value ) );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::append( const wchar_t *p_value )
{
	_wa_string.append( p_value );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::append( const std::string &p_value )
{
	_wa_string.append( wa::strings::convert::to_wstring( p_value ) );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::append( const std::wstring &p_value )
{
	_wa_string.append( p_value );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::append( const wa_string p_value )
{
	_wa_string.append(p_value.GetW().c_str());

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::append( const int p_value )
{
#ifdef WIN32
	// max size is 4294967295 is 10 character
	wchar_t temp[ 11 ] = { 0 };
#elif WIN64
	// maxsize 9223372036854775807 is 19
	wchar_t temp[ 20 ] = { 0 };
#endif

	_itow( p_value, temp, 10 );
	_wa_string.append( temp );

	return *this;
}


// wa_string::GetA
const std::string wa::strings::wa_string::GetA() const
{
	return wa::strings::convert::to_string( _wa_string );
}

// wa_string::GetW
const std::wstring wa::strings::wa_string::GetW() const
{
	return _wa_string;
}


//
// wa_string::lengthA
//
unsigned int wa::strings::wa_string::lengthA() const
{
	return strlen( wa::strings::convert::to_char( _wa_string ) );
}

unsigned int wa::strings::wa_string::lengthW() const
{
	return wcslen( _wa_string.c_str() );
}

unsigned int wa::strings::wa_string::lengthS() const
{
	std::string l_wa_string = wa::strings::convert::to_string( _wa_string );

	return l_wa_string.length();
}

unsigned int wa::strings::wa_string::lengthWS() const
{
	return _wa_string.length();
}


//
// wa_string::replace
//
bool wa::strings::wa_string::contains( const char *p_value )
{
	std::string l_value( p_value );

	return this->contains( l_value );
}

bool wa::strings::wa_string::contains( const wchar_t *p_value )
{
	std::string l_value = wa::strings::convert::to_string( p_value );

	return this->contains( l_value );
}

bool wa::strings::wa_string::contains( const std::string &p_value )
{
	std::string l_wa_string = wa::strings::convert::to_string( _wa_string );

	return ( l_wa_string.find( p_value ) != std::string::npos );
}

bool wa::strings::wa_string::contains( const std::wstring &p_value )
{
	std::string l_value = wa::strings::convert::to_string( p_value );

	return this->contains( l_value );
}


//
// wa_string::replace
//
wa::strings::wa_string wa::strings::wa_string::replace( const char *p_from, const char *p_to )
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	wa::strings::replace( l_string, std::string( p_from ), std::string( p_to ) );

	_wa_string = wa::strings::convert::to_wstring( l_string );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::replace( const wchar_t *p_from, const wchar_t *p_to )
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	wa::strings::replace( l_string, wa::strings::convert::to_string( p_from ), wa::strings::convert::to_string( p_to ) );

	_wa_string = wa::strings::convert::to_wstring( l_string );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::replace( const std::string &p_from, const std::string &p_to )
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	wa::strings::replace( l_string, p_from, p_to );

	_wa_string = wa::strings::convert::to_wstring( l_string );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::replace( const std::wstring &p_from, const std::wstring &p_to )
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	wa::strings::replace( l_string, wa::strings::convert::to_string( p_from), wa::strings::convert::to_string( p_to ) );

	_wa_string = wa::strings::convert::to_wstring( l_string );

	return *this;
}


//
// wa_string::replaceAll
//
wa::strings::wa_string wa::strings::wa_string::replaceAll( const char *p_from, const char *p_to )
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	wa::strings::replaceAll( l_string, std::string( p_from ), std::string( p_to ) );

	_wa_string = wa::strings::convert::to_wstring( l_string );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::replaceAll( const wchar_t *p_from, const wchar_t *p_to )
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	wa::strings::replaceAll( l_string, wa::strings::convert::to_string( p_from ), wa::strings::convert::to_string( p_to ) );

	_wa_string = wa::strings::convert::to_wstring( l_string );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::replaceAll( const std::string &p_from, const std::string &p_to )
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	wa::strings::replaceAll( l_string, p_from, p_to );

	_wa_string = wa::strings::convert::to_wstring( l_string );

	return *this;
}

wa::strings::wa_string wa::strings::wa_string::replaceAll( const std::wstring &p_from, const std::wstring &p_to )
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	wa::strings::replaceAll( l_string, wa::strings::convert::to_string( p_from ), wa::strings::convert::to_string( p_to ) );

	_wa_string = wa::strings::convert::to_wstring( l_string );

	return *this;
}


//
// wa_string::startsWith
//
bool wa::strings::wa_string::startsWith( const char *l_head ) const
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	return ( l_string.substr( 0, std::strlen( l_head ) ).compare( l_head ) == 0 );
}

bool wa::strings::wa_string::startsWith( const wchar_t *l_head ) const
{
	return ( _wa_string.substr( 0, wcslen( l_head ) ).compare( l_head ) == 0 );
}

bool wa::strings::wa_string::startsWith( const std::string &l_head ) const
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	return ( l_string.substr( 0, l_head.size() ).compare( l_head ) == 0 );
}

bool wa::strings::wa_string::startsWith( const std::wstring &l_head ) const
{
	return ( _wa_string.substr( 0, l_head.size() ).compare( l_head ) == 0 );
}


//
// wa_string::endsWith
//
bool wa::strings::wa_string::endsWith( const char *l_tail ) const
{
	// TODO

	return false;
}

bool wa::strings::wa_string::endsWith( const wchar_t *l_tail ) const
{
	// TODO

	return false;
}

bool wa::strings::wa_string::endsWith( const std::string &l_tail ) const
{
	// TODO

	return false;
}

bool wa::strings::wa_string::endsWith( const std::wstring &l_tail ) const
{
	// TODO

	return false;
}


//
// wa_string::findFirst
//
int wa::strings::wa_string::findFirst( const char *l_text ) const
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	return l_string.find_first_of( l_text );
}

int wa::strings::wa_string::findFirst( const wchar_t *l_text ) const
{
	return _wa_string.find_first_of( l_text );
}

int wa::strings::wa_string::findFirst( const std::string &l_text ) const
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	return l_string.find_first_of( l_text );
}

int wa::strings::wa_string::findFirst( const std::wstring &l_text ) const
{
	return _wa_string.find_first_of( l_text );
}


//
// wa_string::findLast
//
int wa::strings::wa_string::findLast( const char *l_text ) const
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	return l_string.find_last_of( l_text );
}

int wa::strings::wa_string::findLast( const wchar_t *l_text ) const
{
	return _wa_string.find_last_of( l_text );
}

int wa::strings::wa_string::findLast( const std::string &l_text ) const
{
	std::string l_string = wa::strings::convert::to_string( _wa_string );

	return l_string.find_last_of( l_text );
}

int wa::strings::wa_string::findLast( const std::wstring &l_text ) const
{
	return _wa_string.find_last_of( l_text );
}


//
// wa_string::find
//
int wa::strings::wa_string::find( const std::wstring &l_text ) const
{
	return _wa_string.find( l_text );
}


// wa_string::mid
std::wstring wa::strings::wa_string::mid( const int p_start, const int p_length ) const
{
	return _wa_string.substr( p_start, p_length );
}


// wa_string::toUpper
wa::strings::wa_string wa::strings::wa_string::toUpper()
{
	std::transform( _wa_string.begin(), _wa_string.end(), _wa_string.begin(), ::toupper );

	return *this;
}
