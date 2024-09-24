#include "Main.h"
#include "DownloadsParse.h"
#include "Downloaded.h"
#include "Defaults.h"
#include "ParseUtil.h"
#include <wchar.h>
#include <locale.h>

static __time64_t filetime( const wchar_t *file )
{
	if ( !file || !*file )
		return 0;

	WIN32_FIND_DATA f = { 0 };

	HANDLE h = FindFirstFile( file, &f );
	if ( h == INVALID_HANDLE_VALUE )
		return 0;

	FindClose( h );
	SYSTEMTIME s = { 0 };
	FileTimeToSystemTime( &f.ftCreationTime, &s );

	tm t = { 0 };
	t.tm_year = s.wYear - 1900;
	t.tm_mon  = s.wMonth - 1;
	t.tm_mday = s.wDay;
	t.tm_hour = s.wHour;
	t.tm_min  = s.wMinute;
	t.tm_sec  = s.wMinute;

	return _mktime64( &t );
}

static void ReadDownload( const XMLNode *item, bool addToLib = false )
{
	DownloadedFile newDownloaded;

	const wchar_t *channel = GetContent( item, L"channel" );
	newDownloaded.SetChannel( channel );

	const wchar_t *item_str = GetContent( item, L"item" );
	newDownloaded.SetItem( item_str );

	const wchar_t *url = GetContent( item, L"url" );
	newDownloaded.SetURL( url );

	const wchar_t *path = GetContent( item, L"path" );
	newDownloaded.SetPath( path );

	const wchar_t *publishDate = GetContent( item, L"publishDate" );

	if ( publishDate && publishDate[ 0 ] )
		newDownloaded.publishDate = _wtoi( publishDate );
	else
		newDownloaded.publishDate = filetime( newDownloaded.path );

	if ( addToLib )
		addToLibrary( newDownloaded );

	downloadedFiles.downloadList.push_back( newDownloaded );
}

static void ReadPreferences( const XMLNode *item )
{
	const XMLNode *curNode;

	curNode = item->Get(L"download");
	if ( curNode )
	{
		const wchar_t *prop = curNode->GetProperty( L"downloadpath" );
		if ( prop )
			lstrcpyn( defaultDownloadPath, prop, MAX_PATH );

		autoDownload = PropertyIsTrue( curNode, L"autodownload" );

		prop = curNode->GetProperty( L"autoDownloadEpisodes" );
		if ( prop )
			autoDownloadEpisodes = _wtoi( prop );
		needToMakePodcastsView = PropertyIsTrue( curNode, L"needToMakePodcastsView" );
	}

	curNode = item->Get(L"update");
	if ( curNode )
	{
		const wchar_t *prop = curNode->GetProperty( L"updatetime" );
		if ( prop )
			updateTime = _wtoi64( prop );

		autoUpdate = PropertyIsTrue( curNode, L"autoupdate" );
		updateOnLaunch = PropertyIsTrue( curNode, L"updateonlaunch" );
	}

	curNode = item->Get(L"subscriptions");
	if ( curNode )
	{
		_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();
		const wchar_t *prop = curNode->GetProperty( L"htmldivider" );
		if ( prop )
			htmlDividerPercent = (float)_wtof_l( prop, C_locale );

		prop = curNode->GetProperty( L"channeldivider" );
		if ( prop && prop[ 0 ] )
			channelDividerPercent = (float)_wtof_l( prop, C_locale );

		prop = curNode->GetProperty( L"itemtitlewidth" );
		if ( prop && prop[ 0 ] )
			itemTitleWidth = _wtoi( prop );

		prop = curNode->GetProperty( L"itemdatewidth" );
		if ( prop && prop[ 0 ] )
			itemDateWidth = _wtoi( prop );

		prop = curNode->GetProperty( L"itemmediawidth" );
		if ( prop && prop[ 0 ] )
			itemMediaWidth = _wtoi( prop );

		prop = curNode->GetProperty( L"itemsizewidth" );
		if ( prop && prop[ 0 ] )
			itemSizeWidth = _wtoi( prop );

		prop = curNode->GetProperty( L"currentitemsort" );
		if ( prop && prop[ 0 ] )
			currentItemSort = _wtoi( prop );

		itemSortAscending = !PropertyIsFalse( curNode, L"itemsortascending" );

		channelSortAscending = !PropertyIsFalse( curNode, L"channelsortascending" );

		prop = curNode->GetProperty( L"channelLastSelection" );
		if ( prop && prop[ 0 ] )
			channelLastSelection = _wtoi( prop );
	}

	curNode = item->Get(L"downloadsView");
	if ( curNode )
	{
		const wchar_t *prop = curNode->GetProperty( L"downloadsChannelWidth" );
		if ( prop && prop[ 0 ] )
			downloadsChannelWidth = _wtoi( prop );
		if ( downloadsChannelWidth <= 0 )
			downloadsChannelWidth = DOWNLOADSCHANNELWIDTHDEFAULT;

		prop = curNode->GetProperty( L"downloadsItemWidth" );
		if ( prop && prop[ 0 ] )
			downloadsItemWidth = _wtoi( prop );
		if ( downloadsItemWidth <= 0 )
			downloadsItemWidth = DOWNLOADSITEMWIDTHDEFAULT;

		prop = curNode->GetProperty( L"downloadsProgressWidth" );
		if ( prop && prop[ 0 ] )
			downloadsProgressWidth = _wtoi( prop );
		if ( downloadsProgressWidth <= 0 )
			downloadsProgressWidth = DOWNLOADSPROGRESSWIDTHDEFAULT;

		prop = curNode->GetProperty( L"downloadsPathWidth" );
		if ( prop && prop[ 0 ] )
			downloadsPathWidth = _wtoi( prop );
		if ( downloadsPathWidth <= 0 )
			downloadsPathWidth = DOWNLOADSPATHWIDTHDEFAULTS;

		prop = curNode->GetProperty( L"downloadsItemSort" );
		if ( prop && prop[ 0 ] )
			downloadsItemSort = _wtoi( prop );

		downloadsSortAscending = !PropertyIsFalse( curNode, L"downloadsSortAscending" );
	}

	curNode = item->Get(L"service");
	if ( curNode )
	{
		const wchar_t *prop = curNode->GetProperty( L"url" );
		if ( prop )
			lstrcpyn( serviceUrl, prop, MAX_PATH );
	}
}

void DownloadsParse::ReadNodes( const wchar_t *url )
{
	XMLNode::NodeList::const_iterator itr;
	const XMLNode *curNode = xmlDOM.GetRoot();

	curNode = curNode->Get( L"winamp:preferences" );
	if ( curNode )
	{
		int version = 1;
		const wchar_t *prop = curNode->GetProperty( L"version" );
		if ( prop && prop[ 0 ] )
			version = _wtoi( prop );

		ReadPreferences( curNode );

		curNode = curNode->Get( L"downloads" );
		if ( curNode )
		{
			const XMLNode::NodeList *downloadsList = curNode->GetList( L"download" );
			if ( downloadsList )
			{
				for ( itr = downloadsList->begin(); itr != downloadsList->end(); itr++ )
					ReadDownload( *itr, version < 2 );
			}
		}
	}
}
