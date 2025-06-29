#ifndef NULLSOFT_IN_OPUS_RAWMEDIAREADER_H
#define NULLSOFT_IN_OPUS_RAWMEDIAREADER_H

#include "../../../Agave/DecodeFile/svc_raw_media_reader.h"
#include "OpusAudioStream.h" // To create instances of OpusAudioStream
#include <api/service/svcs/svc_filetypeeditor.h> // For file type registration (optional but good)
#include "../../../nu/ns_wc.h" // for _wcsicmp
#include "main.h" // For InOpusLangGUID, plugin version constants

// {GUID} - Generate a new GUID for OpusRawMediaReader service factory
// Example: {C5D8F2B0-A2D8-4B0C-8F2D-7E8A1B3C5D6E}
static const GUID OpusRawMediaReaderGUID =
{ 0xc5d8f2b0, 0xa2d8, 0x4b0c, { 0x8f, 0x2d, 0x7e, 0x8a, 0x1b, 0x3c, 0x5d, 0x6e } };

class OpusRawMediaReader : public svc_raw_media_reader
{
public:
    static const char *getServiceName() { return "Opus Raw Media Reader"; }
    static GUID getServiceGuid() { return OpusRawMediaReaderGUID; }

    int CreateAudioStream(nx_uri_t filename, ifc_audiostream **stream, int flags);
    int GetInformation(nx_uri_t filename, ifc_metadata **metadata, int flags);
		int GetAlbumArt(nx_uri_t filename,artwork_type type, ifc_albumart **artwork, int flags);

protected:
    RECVS_DISPATCH;
};

// Optional: File Type Editor integration (for "Open File Type" in preferences)
// {GUID} - Generate a new GUID for OpusFileTypeEditor factory
// Example: {D8E1A3C5-F2B0-4D8A-9C1F-A7B3C5D6E8F0}
static const GUID OpusFileTypeEditorGUID =
{ 0xd8e1a3c5, 0xf2b0, 0x4d8a, { 0x9c, 0x1f, 0xa7, 0xb3, 0xc5, 0xd6, 0xe8, 0xf0 } };

class OpusFileTypeEditor : public svc_filetypeeditor
{
public:
	static const char *getServiceName() { return "Opus File Type Editor"; }
	static GUID getServiceGuid() { return OpusFileTypeEditorGUID; }
	const wchar_t *GetFileExtensions(size_t index);
	const wchar_t *GetFileType(size_t index);
	int CanRemove(size_t index);
	int CanAdd();
	int AddType(const wchar_t *filetype, const wchar_t *extensions);
	int RemoveType(size_t index);
	size_t GetNumFileTypes();

protected:
	RECVS_DISPATCH;
private:
	// Currently, we only support one fixed type
	static const wchar_t *opus_extensions;
	static const wchar_t *opus_file_type;
};


#endif //NULLSOFT_IN_OPUS_RAWMEDIAREADER_H
