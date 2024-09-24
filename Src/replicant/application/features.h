#pragma once
#include "foundation/types.h"
#include "foundation/mkncc.h"
#ifdef __cplusplus
#include "syscb/ifc_syscallback.h"
#endif
/*
This is a listing of features that can be added to Application::SetPermission and api_application::SetFeature
or queried with api_application::GetPermission or api_application::GetFeature

if you are linking from a C file, prepend application_feature_ before each of these guids 
if you are linking from a C++ file, these are in the Features namespace 
*/
#ifdef __cplusplus
extern "C" {
#define FEATURE(x) x
#define PERMISSION(x) x
	namespace Features {
#else
#define FEATURE(x) application_feature_ ## x
#define PERMISSION(x) x application_feature_ ## x
#endif

/* these gets sent via api_sysb when permissions change */

// {33C86C8A-281E-4CA7-88B0-9AF9A06C486D}
static const GUID FEATURE(event_type) = 
{ 0x33c86c8a, 0x281e, 0x4ca7, { 0x88, 0xb0, 0x9a, 0xf9, 0xa0, 0x6c, 0x48, 0x6d } };

static const int FEATURE(permissions_changed) = 0;
static const int FEATURE(features_changed) = 1;

#ifdef __cplusplus
class SystemCallback : public ifc_sysCallback
{
protected:
	GUID WASABICALL SysCallback_GetEventType() { return event_type; }
	int WASABICALL SysCallback_Notify(int msg, intptr_t param1, intptr_t param2)
	{
		switch(msg)
		{
		case permissions_changed:
			return FeaturesSystemCallback_OnPermissionsChanged();
		case features_changed:
			return FeaturesSystemCallback_OnFeaturesChanged();
		default:
			return NErr_Success;
		}
	}
	virtual int WASABICALL FeaturesSystemCallback_OnPermissionsChanged() { return NErr_Success; }
	virtual int WASABICALL FeaturesSystemCallback_OnFeaturesChanged() { return NErr_Success; }
};
#endif

#if 0
// {2E9CE2F8-E26D-4629-A3FF-5DF619136B2C}
static const GUID PERMISSION(crossfade) = 
{ 0x2e9ce2f8, 0xe26d, 0x4629, { 0xa3, 0xff, 0x5d, 0xf6, 0x19, 0x13, 0x6b, 0x2c } };

// {F7CCB4E1-CD8B-4D0A-973A-0C1F15FDDAE2}
static const GUID PERMISSION(replaygain) = 
{ 0xf7ccb4e1, 0xcd8b, 0x4d0a, { 0x97, 0x3a, 0xc, 0x1f, 0x15, 0xfd, 0xda, 0xe2 } };

// {9A52404E-764D-416D-9EC9-33AA84FBA13F}
static const GUID PERMISSION(equalizer) = 
{ 0x9a52404e, 0x764d, 0x416d, { 0x9e, 0xc9, 0x33, 0xaa, 0x84, 0xfb, 0xa1, 0x3f } };
#endif
// {69663D62-5EC5-4B25-B91C-65E388C85E09}
static const GUID FEATURE(aac_playback) = 
{ 0x69663d62, 0x5ec5, 0x4b25, { 0xb9, 0x1c, 0x65, 0xe3, 0x88, 0xc8, 0x5e, 0x9 } };

// {281E7D5B-3A46-4F60-9026-D3FCBFCDD2BB}
static const GUID PERMISSION(gapless) = 
{ 0x281e7d5b, 0x3a46, 0x4f60, { 0x90, 0x26, 0xd3, 0xfc, 0xbf, 0xcd, 0xd2, 0xbb } };

// {4D6A0C67-D2FF-4F81-BA4F-369AF90BE680}
static const GUID PERMISSION(flac_playback) = 
{ 0x4d6a0c67, 0xd2ff, 0x4f81, { 0xba, 0x4f, 0x36, 0x9a, 0xf9, 0xb, 0xe6, 0x80 } };

// {5599E564-400B-40EC-8393-A58D4A6BEBD9}
static const GUID PERMISSION(gracenote_autotag) = 
{ 0x5599e564, 0x400b, 0x40ec, { 0x83, 0x93, 0xa5, 0x8d, 0x4a, 0x6b, 0xeb, 0xd9 } };

// {1A9106D7-C226-4B72-9A30-E4977A519234}
static const GUID PERMISSION(alac_playback) = 
{ 0x1a9106d7, 0xc226, 0x4b72, { 0x9a, 0x30, 0xe4, 0x97, 0x7a, 0x51, 0x92, 0x34 } };


#ifdef __cplusplus
}
}
#endif
