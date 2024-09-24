#include "main.h"
#include "api__ml_iso.h"
#include <api/service/waservicefactory.h>
#include "../burner/obj_isocreator.h"
#include "../playlist/ifc_playlistloadercallback.h"
#include <shlwapi.h>
#include <strsafe.h>

/**
** Playlist Loader callback class
** Used when this plugin loads a playlist file
** the playlist loader will call the OnFile() function
** for each playlist item
*/
class ISOPlaylistLoader : public ifc_playlistloadercallback
{
public:
	ISOPlaylistLoader(obj_isocreator *_creator);

protected:
	RECVS_DISPATCH;

private:
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info);
	obj_isocreator *creator;
};

/**
** helper function
** for getting a filename location
** of where to save the ISO file
**/
static bool PromptForFilename(wchar_t *filename, size_t filenameCch)
{
	wchar_t oldCurPath[MAX_PATH], newCurPath[MAX_PATH];
	OPENFILENAMEW openfilename;

	// null terminate the string or else we'll get a garbage filename as the 'default' filename
	filename[0]=0;

	// GetSaveFileName changes Window's working directory
	// which locks that folder from being deleted until Winamp closes
	// so we save the old working directory name
	// and restore it on complete
	// Winamp maintains its own concept of a working directory
	// to help us avoid this problem
	GetCurrentDirectoryW(MAX_PATH, oldCurPath);


	// initialize the open file name struct
	openfilename.lStructSize = sizeof(openfilename);
	openfilename.hwndOwner = plugin.hwndLibraryParent;
	openfilename.hInstance = plugin.hDllInstance;
	openfilename.lpstrFilter = L"ISO Files\0*.iso\0";
	openfilename.lpstrCustomFilter = 0;
	openfilename.nMaxCustFilter = 0;
	openfilename.nFilterIndex = 0;
	openfilename.lpstrFile = filename;
	openfilename.nMaxFile = filenameCch;
	openfilename.lpstrFileTitle = 0;
	openfilename.nMaxFileTitle = 0;
	// we set the initial directory based on winamp's working path
	openfilename.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();
	openfilename.lpstrTitle = 0;
	// despite the big note about working directory
	// we don't want to use OFN_NOCHANGEDIR
	// because we're going to manually sync Winamp's working path
	openfilename.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_LONGNAMES;
	openfilename.nFileOffset = 0;
	openfilename.nFileExtension = 0;
	openfilename.lpstrDefExt = L".iso";
	openfilename.lCustData = 0;
	openfilename.lpfnHook = 0;
	openfilename.lpTemplateName = 0;

	if (GetSaveFileNameW(&openfilename))
	{
		// let's re-synch Winamp's working directory
		GetCurrentDirectoryW(MAX_PATH, newCurPath);
		WASABI_API_APP->path_setWorkingPath(newCurPath);

		// set the old path back
		SetCurrentDirectoryW(oldCurPath);
		return true; // success!
	}
	else
	{
		// set the old path back
		SetCurrentDirectoryW(oldCurPath);
		return false; // user hit cancel or something else happened
	}

}

/**
** helper functions
** for creating and deleting
** an iso creator object
** through the wasabi service manager
**/
static obj_isocreator *CreateISOCreator()
{
	waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(obj_isocreatorGUID);
	if (factory)
		return (obj_isocreator *)factory->getInterface();
	else
		return 0;
}

static void ReleaseISOCreator(obj_isocreator *creator)
{
	if (creator)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(obj_isocreatorGUID);
		if (factory)
			factory->releaseInterface(creator);
	}
}

void ConvertItemRecordListToISO(const itemRecordList *list)
{
	obj_isocreator *isocreator = CreateISOCreator();
	if (isocreator)
	{
		wchar_t destination[MAX_PATH];

		if (PromptForFilename(destination, MAX_PATH))
		{
			// these values are hardcoded for this example.
			isocreator->Open(L"WinampISO", obj_isocreator::FORMAT_JOLIET, obj_isocreator::MEDIA_CD);

			char destinationPath[MAX_PATH];

			// loop through the files and add them
			for (int i=0;i<list->Size;i++)
			{
				itemRecord &item = list->Items[i];

				// since we have metadata, we're going to auto-generate folders based on the album name
				const char *album = item.album;
				if (!album || !*album)
					album = "Unknown Album";

				// isocreator requires a preceding backslash
				StringCbPrintfA(destinationPath, sizeof(destinationPath), "\\%s\\%s", album, PathFindFileNameA(item.filename));

				// convert to unicode since that's what obj_isocreator requires
				wchar_t unicodeSource[MAX_PATH];
				wchar_t unicodeDest[MAX_PATH];
				MultiByteToWideChar(CP_ACP, 0, item.filename, -1, unicodeSource, MAX_PATH);
				MultiByteToWideChar(CP_ACP, 0, destinationPath, -1, unicodeDest, MAX_PATH);
				isocreator->AddFile(unicodeSource, unicodeDest);
			}

			isocreator->Write(destination, 0);
		}
		ReleaseISOCreator(isocreator);
	}
}

void ConvertFilenamesToISO(const char *filenames)
{
	obj_isocreator *isocreator = CreateISOCreator();
	if (isocreator)
	{
		wchar_t destination[MAX_PATH];

		if (PromptForFilename(destination, MAX_PATH))
		{
			// these values are hardcoded for this example.
			isocreator->Open(L"WinampISO", obj_isocreator::FORMAT_JOLIET, obj_isocreator::MEDIA_CD);

			wchar_t destinationPath[MAX_PATH];

			// loop through the files and add them
			while (*filenames)
			{
				/**
				** both playlist loader and iso creator want unicode filenames
				** so we'll convert it first
				*/
				wchar_t unicodeFilename[MAX_PATH];
				MultiByteToWideChar(CP_ACP, 0, filenames, -1, unicodeFilename, MAX_PATH);

				/**
				** see if this file is a playlist file
				** we'll do that by trying to load it
				** the Load() function will fail gracefully if it's not a playlist file
				** if it succeeds, it will call loader.OnFile() which adds it to the iso file
				**/
				ISOPlaylistLoader loader(isocreator); // make a callback object for playlist loading
				if (AGAVE_API_PLAYLISTMANAGER->Load(unicodeFilename, &loader) == PLAYLISTMANAGER_LOAD_NO_LOADER)
				{
					// not a playlist file, so load it normally

					// isocreator requires a preceding backslash
					StringCbPrintfW(destinationPath, sizeof(destinationPath), L"\\%s", PathFindFileNameW(unicodeFilename));

					isocreator->AddFile(unicodeFilename, destinationPath);
				}
				filenames+=strlen(filenames)+1;
			}
			isocreator->Write(destination, 0);
		}
		ReleaseISOCreator(isocreator);
	}
}


/**
** Load Playlist and write it to the ISO file
** this function is a bit complex, since we have to load the playlist
**	through api_playlistmanager.  This involves creating an playlist loader callback
**	(ifc_playlistloadercallback) which gets called for each playlist item
**/
void ConvertPlaylistToISO(const mlPlaylist *playlist)
{
	obj_isocreator *isocreator = CreateISOCreator();
	if (isocreator)
	{
		wchar_t destination[MAX_PATH];

		if (PromptForFilename(destination, MAX_PATH))
		{
			// these values are hardcoded for this example.
			const wchar_t *title=L"WinampISO";
			if (playlist->title) // if there's a playlist title, use it as the volume name
				title = playlist->title;

			isocreator->Open(title, obj_isocreator::FORMAT_JOLIET, obj_isocreator::MEDIA_CD);

			ISOPlaylistLoader loader(isocreator); // make a callback object for playlist loading
			AGAVE_API_PLAYLISTMANAGER->Load(playlist->filename, &loader);

			isocreator->Write(destination, 0);
		}
		ReleaseISOCreator(isocreator);
	}
}

/** Load all playlists and write them to the ISO file
** this function is a bit complex, since we have to load the playlist
**	through api_playlistmanager.  This involves creating an playlist loader callback
**	(ifc_playlistloadercallback) which gets called for each playlist item
**/
void ConvertPlaylistsToISO(const mlPlaylist **playlists)
{
	obj_isocreator *isocreator = CreateISOCreator();
	if (isocreator)
	{
		wchar_t destination[MAX_PATH];

		if (PromptForFilename(destination, MAX_PATH))
		{
			// these values are hardcoded for this example.
			isocreator->Open(L"WinampISO", obj_isocreator::FORMAT_JOLIET, obj_isocreator::MEDIA_CD);
			while (*playlists)
			{
				const mlPlaylist *playlist = *playlists;
				ISOPlaylistLoader loader(isocreator); // make a callback object for playlist loading
				AGAVE_API_PLAYLISTMANAGER->Load(playlist->filename, &loader);
				playlists++;
			}
			isocreator->Write(destination, 0);
		}
		ReleaseISOCreator(isocreator);
	}
}

void ConvertUnicodeItemRecordListToISO(const itemRecordListW *list)
{
	obj_isocreator *isocreator = CreateISOCreator();
	if (isocreator)
	{
		wchar_t destination[MAX_PATH];

		if (PromptForFilename(destination, MAX_PATH))
		{
			// these values are hardcoded for this example.
			isocreator->Open(L"WinampISO", obj_isocreator::FORMAT_JOLIET, obj_isocreator::MEDIA_CD);

			wchar_t destinationPath[MAX_PATH];

			// loop through the files and add them
			for (int i=0;i<list->Size;i++)
			{
				itemRecordW &item = list->Items[i];

				// since we have metadata, we're going to auto-generate folders based on the album name
				const wchar_t *album = item.album;
				if (!album || !*album)
					album = L"Unknown Album";

				// isocreator requires a preceding backslash
				StringCbPrintfW(destinationPath, sizeof(destinationPath), L"\\%s\\%s", album, PathFindFileNameW(item.filename));

				isocreator->AddFile(item.filename, destinationPath);
			}
			isocreator->Write(destination, 0);
		}
		ReleaseISOCreator(isocreator);
	}
}

void ConvertUnicodeFilenamesToISO(const wchar_t *filenames)
{
	obj_isocreator *isocreator = CreateISOCreator();
	if (isocreator)
	{
		wchar_t destination[MAX_PATH];

		if (PromptForFilename(destination, MAX_PATH))
		{
			// these values are hardcoded for this example.
			isocreator->Open(L"WinampISO", obj_isocreator::FORMAT_JOLIET, obj_isocreator::MEDIA_CD);

			wchar_t destinationPath[MAX_PATH];

			// loop through the files and add them
			while (*filenames)
			{
				/**
				** see if this file is a playlist file
				** we'll do that by trying to load it
				** the Load() function will fail gracefully if it's not a playlist file
				** if it succeeds, it will call loader.OnFile() which adds it to the iso file
				**/
				ISOPlaylistLoader loader(isocreator); // make a callback object for playlist loading
				if (AGAVE_API_PLAYLISTMANAGER->Load(filenames, &loader) == PLAYLISTMANAGER_LOAD_NO_LOADER)
				{
					// not a playlist file, so load it normally

					// isocreator requires a preceding backslash
					StringCbPrintfW(destinationPath, sizeof(destinationPath), L"\\%s", PathFindFileNameW(filenames));

					isocreator->AddFile(filenames, destinationPath);
				}
				filenames+=wcslen(filenames)+1;
			}

			isocreator->Write(destination, 0);
		}
		ReleaseISOCreator(isocreator);
	}
}

/* --- Playlist Loader definition --- */
ISOPlaylistLoader::ISOPlaylistLoader(obj_isocreator *_creator)
{
	creator=_creator;
}

void ISOPlaylistLoader::OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
{
	// isocreator requires a preceding backslash
	wchar_t destinationPath[MAX_PATH];
	StringCbPrintfW(destinationPath, sizeof(destinationPath), L"\\%s", PathFindFileNameW(filename));
	// add file to .iso image
	creator->AddFile(filename, destinationPath);
}

#define CBCLASS ISOPlaylistLoader
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
//VCB(IFC_PLAYLISTLOADERCALLBACK_ONPLAYLISTINFO, OnPlaylistInfo)
END_DISPATCH;
