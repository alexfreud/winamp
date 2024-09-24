#ifndef _WA2PLEDIT_H
#define _WA2PLEDIT_H

#include <api/wnd/wndclass/listwnd.h>
#include "wa2playlist.h"

#define WA2PLAYLISTEDITOR_PARENT ListWnd

class Wa2PlaylistEditor;

class Wa2PlaylistEditor : public WA2PLAYLISTEDITOR_PARENT
{
public:
    static GUID getInterfaceGuid()
    {
        // {265947B2-3EDB-453e-B748-EC17890F4FE4}
        const GUID guid =
        { 0x265947b2, 0x3edb, 0x453e, { 0xb7, 0x48, 0xec, 0x17, 0x89, 0xf, 0x4f, 0xe4 } };
        return guid;
    }

    Wa2PlaylistEditor();
    virtual ~Wa2PlaylistEditor();

    virtual int onInit();
    virtual int onResize();
    virtual int wantHScroll()                                         { return 0; }
    virtual void onVScrollToggle( int set );
    virtual COLORREF getTextColor( LPARAM lParam );
    virtual COLORREF getBgColor( LPARAM lParam );
    virtual void *getInterface( GUID interface_guid );
    virtual void setPlaylist( Wa2Playlist *playlist ); // -1 for working playlist
    virtual int needFocusRect( LPARAM lParam );
    virtual COLORREF getFocusRectColor( LPARAM lParam );
    virtual void onSetVisible( int show );
    virtual int onDeferredCallback( intptr_t p1, intptr_t p2 );
    virtual void timerCallback( int id );
    virtual void onDoubleClick( int itemnum );

    // object
    virtual void onNewCurrentIndex( int idx );
    virtual void onPlaylistModified();

    // class
    static void _onNewCurrentIndex( int idx );
    static void _onPlaylistModified();
    virtual void loadList();

private:
    void _loadList();
    void resizeCols();
    int calcTrackNumWidth();

    Wa2Playlist *curplaylist;
    static PtrList<Wa2PlaylistEditor> editors;
    int cur_index;
};

// -----------------------------------------------------------------------
extern const wchar_t Wa2PleditXuiObjectStr[];
extern char Wa2PleditXuiSvcName[];

class Wa2PleditXuiSvc : public XuiObjectSvc<Wa2PlaylistEditor, Wa2PleditXuiObjectStr, Wa2PleditXuiSvcName> {};


#endif


