#pragma once
/*
Service for creating Audio Encoders which encode straight to a file.

This interface is not meant to do the encoding itself, it is a factory to create and destroy
encoder objects (ifc_audioFileEncoder)
*/
#include <bfc/dispatch.h>
#include "ifc_audioFileEncoder.h"
#include "../DecodeFile/api_decodefile.h" // for AudioParameters struct
#include <bfc/std_mkncc.h> // for MKnCC()
class svc_audioFileEncoder : public Dispatchable
{
protected:
	svc_audioFileEncoder() {}
	~svc_audioFileEncoder() {}
public:
	  static FOURCC getServiceType() { return svc_albumArtProvider::SERVICETYPE; }
	/*
	  General parameter notes (for all methods of this class):
		@param profile is a filename of an INI file where the encoder settings are stored
		@param parameters defines the input audio data.  For methods where this parameter
		is optional (default parameter value == 0), it is meant to be passed to optionally
		limit configuration choices.  For example, an AAC encoder could hide surround-sound
		encoding options when parameters->channels == 2
	*/

	// return a user-friendly name for the encoder,
	// e.g. "Nullsoft FLAC encoder v2.1"
	const wchar_t *GetName();

	// retrieve information about a particular profile
	// @param profile_item information to retrieve.  currently, there are only three defined strings
	// "bitrate" -- retrieve an estimate bitrate (in kbps) for the profile
	// "profile" -- retrieve a more specific name for a given profile (and optional input parameters)
  // e.g. an AAC encoder could return "HE-AAC v2, 16kbps, MP4 File Format" or "AAC LC, ADTS stream"
	// "settings" -- retrieve a string that codifies the settings in the profile
	// for most codecs, this should mirror what the commandline string would be for the corresponding
	// commandline codec (flac.exe, lame.exe, etc).
	// e.g. "-V 2" 
	int GetProfileInfo(const wchar_t *profile, const wchar_t *profile_item, wchar_t *value, size_t value_cch, const AudioParameters *parameters = 0);

	// not currently used for anything
	int SetProfileInfo(const wchar_t *profile, const wchar_t *profile_item, const wchar_t *value, AudioParameters *parameters = 0);

	// this function gets the party started.  call after you have opened your input file (thru
	// api_decodefile, for example) so you can pass in a valid parameters struct.
	// @param filename destination filename
	int Create(ifc_audioFileEncoder **encoder, const wchar_t *filename, const wchar_t *profile, const AudioParameters *parameters);

	// call this when you are done
	void Destroy(ifc_audioFileEncoder *encoder);

	// if parent is NULL, pop up a modal dialog/window
	// if parent is not NULL, return a modeless child dialog/window 
	HWND Configure(HWND parent, const wchar_t *profile, const AudioParameters *parameters = 0);


	enum
	{
		SERVICETYPE = MK4CC('a','f','e', 'n')
	};

};