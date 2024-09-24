#ifndef NULLSOFT_API_PLAYLIST_COLOURISER_H
#define NULLSOFT_API_PLAYLIST_COLOURISER_H

/*
** Wasabi Playlist Colouriser API Interface v1.0 
** Note: This requires JTFE v1.2 and higher to work
** (Released: 28/09/2010)
**
**
** This header file provides the interfaces implemented by the JTFE plugin for other plugins and services to
** be able to make use of it's playlist colouriser which allows for highlighting of playlist entries in a
** different style to that currently defined by the skin in use to make for example queued items more visible.
**
** The interface allows for controlling aspects of the text colour and the background of playlist item entries
** as long as they exist in the playlist editor or there is data to show e.g. you cannot change the text colour
** of the time entry if there is no time entry shown such as for missing files or streams of unknown length).
**
** When specifying a colour, if you want the default skin colours to be used then you need to set the colour to
** be -1
**
** To use this api assumes you know already how to make use of the wasabi service based system
** (see the more complete examples provided in the SDK).
**
**
** Example:
**
** The following psuedo code shows how to show a flashing inverted selection on the first item being queried in
** the current playlist editor's contents.
**
** // this will setup an inverted entry when the playlist is queried to be painted
** // you could setup a timer to cause a playlist painting event to make it flash
** // if you toggle the returned state on and off in the ExampleCheck(..) callback
** Colouriser ExampleColouriser = {COLOURISER_FULL_BKGND | COLOURISER_FULL_TEXT |
** 								   COLOURISER_INVERTED | COLOURISER_INVERTED_TEXT,
**								   0,-1,-1,-1,-1, ExampleCheck};
**
** int ExampleCheck(int idx, wchar_t* file){
**   // will only apply the colouring on the first playlist item
**   return (idx == 0);
** }
**
** // use this to get an instance of the service (returns null or 1 on error or not supported)
** if(!WASABI_API_COLOURISER) ServiceBuild(WASABI_API_COLOURISER,PlaylistColouriserApiGUID);
**
**
** // this can be used to add or update an existing colouriser instance
** // using COLOURISER_DISABLED in the flags to disable it if not needed
** if(!WASABI_API_COLOURISER->ColouriserExists(&ExampleColouriser)){
**   WASABI_API_COLOURISER->AddColouriser(&ExampleColouriser);
** }
** else{
**   WASABI_API_COLOURISER->UpdateColouriser(&ExampleColouriser);
** }
**
** // with the above, if you wanted to change the code to just change the text
** // colour of playlist item then you could use something like the following:
** ExampleColouriser.flags = COLOURISER_FULL_TEXT;
** ExampleColouriser.main_text = ExcludeColouriser.time_text = <specify_your_colour>;
*/

#if (_MSC_VER <= 1200)
typedef int intptr_t;
#endif

#ifdef __cplusplus

#include <bfc/dispatch.h>

typedef struct{
	#define COLOURISER_TIME_BKGND		0x01	// override the time column background colour - uses time_bkgnd
	#define COLOURISER_MAIN_BKGND		0x02	// override the main column background colour - uses main_bkgnd
	#define COLOURISER_MAIN_BKGND_ALT	0x04	// allows for a different colour for the main column background
	#define COLOURISER_FULL_BKGND		COLOURISER_MAIN_BKGND | COLOURISER_TIME_BKGND

	#define COLOURISER_TIME_TEXT		0x10	// override the time column text colour - uses time_text
	#define COLOURISER_MAIN_TEXT		0x20	// override the main column text colour - uses main_text
	#define COLOURISER_FULL_TEXT		COLOURISER_TIME_TEXT | COLOURISER_MAIN_TEXT

	#define COLOURISER_BLEND			0x40	// will attempt to blend the colour with the existing colours

	#define COLOURISER_DISABLED			0x1000	// set this when you require your colouriser to be ignored
	#define COLOURISER_INVERTED			0x2000	// if colours are specified as -1 then use the inverse of the current skin values
	#define COLOURISER_INVERTED_TEXT	0x4000	// if colours are specified as -1 then use the inverse of the current skin values
												// this will only be used if COLOURISER_INVERTED is already specified

	int flags;		// determine which colours are to be used / handled
	int _me;		// used to identify the colouriser - don't alter!!!
	
	// when using ColouriserColour(..) the value for query_colour is shown to the right
	COLORREF time_bkgnd;	// 0
	COLORREF main_bkgnd;	// 1
	COLORREF time_text;		// 2
	COLORREF main_text;		// 3

	// callback function to see if the colouriser's colours need to be used on the passed playlist item
	int (*check)(int entry_index, wchar_t* entry_filepath);
} Colouriser;

class api_playlist_colouriser : public Dispatchable
{
protected:
	api_playlist_colouriser() {}
	~api_playlist_colouriser() {}

public:
	BOOL AddColouriser(Colouriser* colouriser);
	Colouriser* UpdateColouriser(Colouriser* colouriser);
	BOOL ColourPicker(HWND parent_hwnd, UINT control_id, UINT options_id, Colouriser* colouriser, wchar_t* window_title, wchar_t* button_text);
	BOOL ColouriserExists(Colouriser* colouriser);
	COLORREF ColouriserColour(COLORREF current_colour, UINT query_colour);

public:
	DISPATCH_CODES
	{
		API_COLOURISER_ADD = 1,
		API_COLOURISER_UPDATE = 2,
		API_COLOURISER_COLOURPICKER = 3,
		API_COLOURISER_EXISTS = 4,
		API_COLOURISER_COLOUR = 5,
	};
};

inline BOOL api_playlist_colouriser::AddColouriser(Colouriser* colouriser)
{
	return _call(API_COLOURISER_ADD, (BOOL)0, colouriser);
}

inline Colouriser* api_playlist_colouriser::UpdateColouriser(Colouriser* colouriser)
{
	return _call(API_COLOURISER_UPDATE, (Colouriser*)0, colouriser);
}

inline BOOL api_playlist_colouriser::ColourPicker(HWND parent_hwnd, UINT control_id, UINT options_id, Colouriser* colouriser, wchar_t* window_title, wchar_t* button_text)
{
	return _call(API_COLOURISER_COLOURPICKER, (BOOL)0, parent_hwnd, control_id, options_id, colouriser, window_title, button_text);
}

inline BOOL api_playlist_colouriser::ColouriserExists(Colouriser* colouriser)
{
	return _call(API_COLOURISER_EXISTS, (BOOL)0, colouriser);
}

inline COLORREF api_playlist_colouriser::ColouriserColour(COLORREF current_colour, UINT query_colour)
{
	return _call(API_COLOURISER_COLOUR, (COLORREF)0, current_colour, query_colour);
}

#endif

// {B8B8DA7C-1F35-4a6d-95FA-C7E9651D5DC0}
static const GUID PlaylistColouriserApiGUID = 
{ 0xb8b8da7c, 0x1f35, 0x4a6d, { 0x95, 0xfa, 0xc7, 0xe9, 0x65, 0x1d, 0x5d, 0xc0 } };

#endif