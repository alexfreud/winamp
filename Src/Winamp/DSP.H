#ifndef NULLSOFT_WINAMP_DSP_H
#define NULLSOFT_WINAMP_DSP_H
// DSP plugin interface

typedef struct winampDSPModule
{
	char *description;		// description
	HWND hwndParent;			// parent window (filled in by calling app)
	HINSTANCE hDllInstance;	// instance handle to this DLL (filled in by calling app)

	void( __cdecl *Config )( struct winampDSPModule *this_mod );  // configuration dialog (if needed)
	int( __cdecl *Init )( struct winampDSPModule *this_mod );     // 0 on success, creates window, etc (if needed)

	// modify waveform samples: returns number of samples to actually write
	// (typically numsamples, but no more than twice numsamples, and no less than half numsamples)
	// numsamples should always be at least 128. should, but I'm not sure
	int( __cdecl *ModifySamples )( struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate );

	void( __cdecl *Quit )( struct winampDSPModule *this_mod );    // called when unloading

	void *userData; // user data, optional
} winampDSPModule;

typedef struct
{
	int version;       // DSP_HDRVER
	char *description; // description of library
	winampDSPModule *( __cdecl *getModule )( int );	// module retrieval function
	int( __cdecl *sf )( int key ); // DSP_HDRVER == 0x21
} winampDSPHeader;

// exported symbols
#ifdef USE_DSP_HDR_HWND
typedef winampDSPHeader *( __cdecl *winampDSPGetHeaderType )( HWND );
#define DSP_HDRVER 0x22
#else
// Note: Unless using USE_DSP_HDR_HWND or a Winamp 5.5+ client then winampDSPGetHeaderType(..)
//       will not receive a HWND parameter & with be called as winampDSPGetHeaderType(void).
//       This is only defined with an HWND to allow for correct compiling of the client exe.
typedef winampDSPHeader *( __cdecl *winampDSPGetHeaderType )( HWND );
// header version: 0x20 == 0.20 == winamp 2.0
#define DSP_HDRVER 0x20
#endif

// These are the return values to be used with the uninstall plugin export function:
// __declspec(dllexport) int __cdecl winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param)
// which determines if Winamp can uninstall the plugin immediately or on winamp restart.
// If this is not exported then Winamp will assume an uninstall with reboot is the only way.
//
#define DSP_PLUGIN_UNINSTALL_NOW    0x0
#define DSP_PLUGIN_UNINSTALL_REBOOT 0x1
//
// Uninstall support was added from 5.0+ and uninstall now support from 5.5+ though note
// that it is down to you to ensure that if uninstall now is returned that it will not
// cause a crash i.e. don't use if you've been subclassing the main window.
//
// The HWND passed in the calling of winampUninstallPlugin(..) is the preference page HWND.
//
//
// Version note:
//
// Added passing of Winamp's main hwnd in the call to the exported winampDSPHeader()
// which allows for primarily the use of localisation features with the bundled plugins.
// If you want to use the new version then either you can edit you version of dsp.h or
// you can add USE_DSP_HDR_HWND to your project's defined list or before use of dsp.h
//

// For a DSP plugin to be correctly detected by Winamp you need to ensure that
// the exported winampDSPGetHeader2(..) is exported as an undecorated function
// e.g.
// #ifdef __cplusplus
//   extern "C" {
// #endif
// __declspec(dllexport) winampDSPGetHeaderType * __cdecl winampDSPGetHeader2(){ return &plugin; }
// #ifdef __cplusplus
//   }
// #endif
//

#endif