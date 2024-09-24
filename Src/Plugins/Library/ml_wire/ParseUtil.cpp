#include "ParseUtil.h"

bool PropertyIsTrue( const XMLNode *item, const wchar_t *property )
{
	if ( !item )
		return false;

	const wchar_t *value = item->GetProperty( property );
	if ( !value )
		return false;

	return !_wcsicmp( value, L"true" );
}

bool PropertyIsFalse( const XMLNode *item, const wchar_t *property )
{
	if ( !item )
		return false;

	const wchar_t *value = item->GetProperty( property );
	if ( !value )
		return false;

	return !_wcsicmp( value, L"false" );
}

const wchar_t *GetContent( const XMLNode *item, const wchar_t *tag )
{
	const XMLNode *curNode = item->Get( tag );
	if ( curNode )
		return curNode->GetContent();
	else
		return 0;
}

const wchar_t *GetProperty( const XMLNode *item, const wchar_t *tag, const wchar_t *property )
{
	const XMLNode *curNode = item->Get( tag );
	if ( curNode )
		return curNode->GetProperty( property );
	else
		return 0;
}