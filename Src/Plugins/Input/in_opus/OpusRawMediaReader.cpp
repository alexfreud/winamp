#include "OpusRawMediaReader.h"
#include "OpusAudioStream.h"
#include "../../../nix/nxuri.h" // For NXURIGetPath
#include "../../../Agave/Metadata/api_metadata.h" // For ifc_metadata (though we might not fully implement it here)
#include <shlwapi.h> // For PathFindExtensionW

// --- OpusRawMediaReader Implementation ---

// Define the dispatch table
BEGIN_DISPATCH;
    CB(SVC_RAWREAD_CREATEAUDIOSTREAM, CreateAudioStream);
    CB(SVC_RAWREAD_GETINFORMATION, GetInformation);
		CB(SVC_RAWREAD_GETALBUMART, GetAlbumArt);
END_DISPATCH;

int OpusRawMediaReader::CreateAudioStream(nx_uri_t filename, ifc_audiostream **stream, int flags)
{
    if (!stream)
        return NErr_BadParameter;

    *stream = NULL;
    const wchar_t *filepath = NXURIGetPath(filename);
    if (!filepath)
        return NErr_Empty;

    // Check if it's an Opus file (basic check, could be more robust)
    const wchar_t *ext = PathFindExtensionW(filepath);
    if (!ext || _wcsicmp(ext, L".opus") != 0)
    {
        // Not our file type based on extension.
        // More advanced plugins might try to "sniff" the file content here.
        return NErr_False;
    }

    int errorCode = 0;
    OpusAudioStream *newStream = new (std::nothrow) OpusAudioStream(filepath, &errorCode);

    if (!newStream)
        return NErr_OutOfMemory;

    if (errorCode != 0 || newStream->GetError() != 0)
    {
        delete newStream;
        // TODO: Map newStream->GetError() (opus specific error) to an NErr_ value if possible
        return NErr_Error;
    }

    *stream = newStream;
    return NErr_Success;
}

int OpusRawMediaReader::GetInformation(nx_uri_t filename, ifc_metadata **metadata, int flags)
{
    // For now, we don't provide ifc_metadata directly through this service.
    // Metadata is primarily handled by winampGetExtendedFileInfoW and GetFileInfo in main.cpp.
    // A more advanced implementation could create an ifc_metadata object here.
    if (metadata)
        *metadata = NULL;
    return NErr_NotImplemented;
}

int OpusRawMediaReader::GetAlbumArt(nx_uri_t filename, artwork_type type, ifc_albumart **artwork, int flags)
{
	    // Album art is not handled by this basic implementation.
    // More advanced plugins might integrate with AlbumArt API here.
    if (artwork)
	    *artwork = NULL;
    return NErr_NotImplemented;
}


// --- OpusFileTypeEditor Implementation (Optional but good for UI integration) ---
const wchar_t *OpusFileTypeEditor::opus_extensions = L"opus";
const wchar_t *OpusFileTypeEditor::opus_file_type = L"Opus Audio";

BEGIN_DISPATCH;
    CB(SVC_FILETYPEEDITOR_GETEXTENSIONS, GetFileExtensions);
    CB(SVC_FILETYPEEDITOR_GETFILETYPE, GetFileType);
    CB(SVC_FILETYPEEDITOR_CANREMOVE, CanRemove);
    CB(SVC_FILETYPEEDITOR_CANADD, CanAdd);
    CB(SVC_FILETYPEEDITOR_ADDTYPE, AddType);
    CB(SVC_FILETYPEEDITOR_REMOVETYPE, RemoveType);
    CB(SVC_FILETYPEEDITOR_GETNUMFILETYPES, GetNumFileTypes);
END_DISPATCH;

const wchar_t *OpusFileTypeEditor::GetFileExtensions(size_t index)
{
	if (index == 0)
		return opus_extensions;
	return NULL;
}

const wchar_t *OpusFileTypeEditor::GetFileType(size_t index)
{
	if (index == 0)
		return opus_file_type;
	return NULL;
}

int OpusFileTypeEditor::CanRemove(size_t index)
{
	return 0; // Our built-in type cannot be removed by the user through this interface
}

int OpusFileTypeEditor::CanAdd()
{
	return 0; // We don't support adding new types via this editor for this plugin
}

int OpusFileTypeEditor::AddType(const wchar_t *filetype, const wchar_t *extensions)
{
	return NErr_NotImplemented; // Not supported
}

int OpusFileTypeEditor::RemoveType(size_t index)
{
	return NErr_NotImplemented; // Not supported
}

size_t OpusFileTypeEditor::GetNumFileTypes()
{
	return 1; // We manage one file type: Opus
}
