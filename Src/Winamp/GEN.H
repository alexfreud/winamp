#ifndef NULLSOFT_WINAMP_GEN_H
#define NULLSOFT_WINAMP_GEN_H
// General Purpose plugin interface

#include <windows.h>

typedef struct {
	int version;
	char* description;
	int(__cdecl* init)();
	void(__cdecl* config)();
	void(__cdecl* quit)();
	HWND hwndParent;
	HINSTANCE hDllInstance;
} winampGeneralPurposePlugin;

// return values from the init(..) which determines if Winamp will continue loading
// and handling the plugin or if it will disregard the load attempt. If GEN_INIT_FAILURE
// is returned then the plugin will be listed as [NOT LOADED] on the plug-in prefs page.
#define GEN_INIT_SUCCESS 0
#define GEN_INIT_FAILURE 1

#define GPPHDR_VER 0x10

// added 5.64+
#define GPPHDR_VER_U 0x11
// specify GPPHDR_VER_U if you want to provide a unicode (wchar_t*) description and only work on 5.64+
// specify GPPHDR_VER to use the original (char*) description as before
// note: we are using the fact that sizeof(char*) == sizeof(wchar_t*) to be able to allow this
//       so when using GPPHDR_VER_U you will need to cast description to (wchar_t*) to set

#ifdef __cplusplus
extern "C" {
#endif
	typedef winampGeneralPurposePlugin* (__cdecl* winampGeneralPurposePluginGetter)();
#ifdef __cplusplus
}
#endif

// These are the return values to be used with the uninstall plugin export function:
// __declspec(dllexport) int __cdecl winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param)
// which determines if Winamp can uninstall the plugin immediately or on winamp restart.
// If this is not exported then Winamp will assume an uninstall with reboot is the only way.
//
#define GEN_PLUGIN_UNINSTALL_NOW    0x1
#define GEN_PLUGIN_UNINSTALL_REBOOT 0x0
//
// Uninstall support was added from 5.0+ and uninstall now support from 5.5+ though note
// that it is down to you to ensure that if uninstall now is returned that it will not
// cause a crash i.e. don't use if you've been subclassing the main window.
//
// The HWND passed in the calling of winampUninstallPlugin(..) is the preference page HWND.
//
// The following is a psuedo example of using winampUninstallPlugin(..) which shows its usage:
//
//
// // use this as a control on saving settings as quit(..) will be called afterwards
// int no_uninstall = 1;
// __declspec(dllexport) int __cdecl winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param){
//   // prompt to remove our settings with default as no (just incase)
//   if(MessageBox(hwndDlg,"Do you also want to remove the saved settings for this plugin?",
//                 plugin.description,MB_YESNO|MB_DEFBUTTON2) == IDYES)
//   {
//     // PLUGIN_NAME is the name of the section you save settings into
//     // and this call will make the OS remove the setion passed
//     WritePrivateProfileString(PLUGIN_NAME,0,0,ini_file);
//     no_uninstall = 0;
//   }
//   // as we're doing too much in subclasses, etc we cannot allow for on-the-fly removal so need to do a normal reboot
//   return GEN_PLUGIN_UNINSTALL_REBOOT;
// }
//

// For a general purpose plugin to be correctly detected by Winamp you need to ensure that
// the exported winampGetGeneralPurposePlugin(..) is exported as an undecorated function
// e.g.
// #ifdef __cplusplus
//   extern "C" {
// #endif
// __declspec(dllexport) winampGeneralPurposePlugin * __cdecl winampGetGeneralPurposePlugin(){ return &plugin; }
// #ifdef __cplusplus
//   }
// #endif
//

#endif