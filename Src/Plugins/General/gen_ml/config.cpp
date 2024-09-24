#include <windows.h>
#include <strsafe.h>

#include "config.h"
#include "../nu/AutoCharFn.h"



C_Config::C_Config( wchar_t *ini )
{
	memset( m_strbuf, 0, sizeof( m_strbuf ) );
	memset( m_strbufA, 0, sizeof( m_strbufA ) );

	m_inifile  = _wcsdup( ini );
	m_inifileA = _strdup( AutoCharFn( m_inifile ) );
	m_section  = 0;
}

C_Config::~C_Config()
{
	if ( m_inifile )
		free( m_inifile );

	if ( m_inifileA )
		free( m_inifileA );

	if ( m_section )
		free( m_section );
}


void C_Config::WriteInt( wchar_t *name, int value )
{
	wchar_t buf[ 32 ] = { 0 };
	StringCchPrintfW( buf, 32, L"%d", value );
	WriteString( name, buf );
}

int C_Config::ReadInt( wchar_t *name, int defvalue )
{
	return GetPrivateProfileIntW( L"gen_ml_config", name, defvalue, m_inifile );
}


wchar_t *C_Config::WriteString( wchar_t *name, wchar_t *string )
{
	WritePrivateProfileStringW( L"gen_ml_config", name, string, m_inifile );

	return name;
}

char *C_Config::WriteString( char *name, char *string )
{
	WritePrivateProfileStringA( "gen_ml_config", name, string, m_inifileA );

	return name;
}

wchar_t *C_Config::ReadString( wchar_t *name, wchar_t *defstr )
{
	static wchar_t foobuf[] = L"___________gen_ml_lameness___________";
	m_strbuf[ 0 ] = 0;
	GetPrivateProfileStringW( L"gen_ml_config", name, foobuf, m_strbuf, ARRAYSIZE( m_strbuf ), m_inifile );
	if ( !wcscmp( foobuf, m_strbuf ) )
		return defstr;

	m_strbuf[ ARRAYSIZE( m_strbuf ) - 1 ] = 0;

	return m_strbuf;
}

bool C_Config::ReadString( const wchar_t *name, const wchar_t *defvalue, wchar_t *storage, size_t len )
{
	static wchar_t foobuf[] = L"___________gen_ml_lameness___________";
	storage[ 0 ] = 0;
	GetPrivateProfileStringW( L"gen_ml_config", name, foobuf, storage, (DWORD)len, m_inifile );
	if ( !wcscmp( foobuf, storage ) )
	{
		if ( defvalue )
			lstrcpynW( storage, defvalue, (int)len );

		return false;
	}

	return true;
}

char *C_Config::ReadString( const char *name, char *defstr )
{
	static char foobuf[] = "___________gen_ml_lameness___________";
	m_strbufA[ 0 ] = 0;
	GetPrivateProfileStringA( "gen_ml_config", name, foobuf, m_strbufA, ARRAYSIZE( m_strbufA ), m_inifileA );
	if ( !strcmp( foobuf, m_strbufA ) )
		return defstr;

	m_strbufA[ ARRAYSIZE( m_strbufA ) - 1 ] = 0;

	return m_strbufA;
}

char *C_Config::ReadString( const char *section_name, const char *key_name, char *defvalue )
{
	static char foobuf[] = "___________lameness___________";
	m_strbufA[ 0 ] = 0;
	GetPrivateProfileStringA( section_name, key_name, foobuf, m_strbufA, ARRAYSIZE( m_strbufA ), m_inifileA );
	if ( !strcmp( foobuf, m_strbufA ) )
		return defvalue;

	m_strbufA[ ARRAYSIZE( m_strbufA ) - 1 ] = 0;

	return m_strbufA;
}

bool C_Config::ReadString( const char *name, const char *defvalue, char *storage, size_t len )
{
	static char foobuf[] = "___________gen_ml_lameness___________";
	storage[ 0 ] = 0;
	GetPrivateProfileStringA( "gen_ml_config", name, foobuf, storage, (DWORD)len, m_inifileA );
	if ( !strcmp( foobuf, storage ) )
	{
		if ( defvalue )
			lstrcpynA( storage, defvalue, (int)len );

		return false;
	}

	return true;
}