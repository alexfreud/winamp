#ifndef NULLSOFT_IN_OPUS_MAINH
#define NULLSOFT_IN_OPUS_MAINH

#define PLUGIN_VERSION_OPUS L"0.1" // Initial version

// We will use libopusfile, which requires ogg and opus headers
#include <opusfile.h> // This will be the main include for Opus decoding

#include "../../../Winamp/in2.h" // Standard Winamp input plugin header
extern In_Module plugin;

extern int pan, volume; // Standard Winamp variables
extern volatile int currentSongLengthMs; // Keep track of current song length

// Forward declarations for any specific Opus config/about dialogs if needed later
// BOOL CALLBACK OpusPreferencesDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// void OpusConfig(HWND hwndParent);
// void OpusAbout(HWND hwndParent);

// GUID for Opus plugin specific language strings
// {7A8C2C4B-1B9A-4A2E-9B1F-3F9A7C6D2E8F}
static const GUID InOpusLangGUID = { 0x7a8c2c4b, 0x1b9a, 0x4a2e, { 0x9b, 0x1f, 0x3f, 0x9a, 0x7c, 0x6d, 0x2e, 0x8f } };

// GUID for playback configuration group (e.g., for thread priority)
// Copied from in_flac/main.h - this is a standard Agave/Winamp GUID
// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };


#endif
