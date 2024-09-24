#ifndef __WINAMP2FRONTEND_H
#define __WINAMP2FRONTEND_H

#define WA2_GETINFO_SAMPLERATE  0
#define WA2_GETINFO_BITRATE     1
#define WA2_GETINFO_CHANNELS    2

#define WA2_NUMBANDS           10

#define WA2_EQDATA_FIRSTBAND    0
#define WA2_EQDATA_LASTBAND     9
#define WA2_EQDATA_PREAMP      10
#define WA2_EQDATA_ENABLED     11
#define WA2_EQDATA_AUTO        12

#define IPC_GETWND_EQ           0 
#define IPC_GETWND_PE           1
#define IPC_GETWND_MB           2
#define IPC_GETWND_VIDEO        3

#define WA2_USERBUTTON_PREV     0
#define WA2_USERBUTTON_PLAY     1
#define WA2_USERBUTTON_PAUSE    2
#define WA2_USERBUTTON_STOP     3
#define WA2_USERBUTTON_NEXT     4

#define WA2_USERBUTTONMOD_NONE  0
#define WA2_USERBUTTONMOD_SHIFT 1
#define WA2_USERBUTTONMOD_CTRL  2

#define WINAMP_MAIN_WINDOW         40258
#define WINAMP_OPTIONS_MINIBROWSER 40298
#define WINAMP_OPTIONS_VIDEO       40328
#define WINAMP_OPTIONS_PLEDIT      40040
#define WINAMP_OPTIONS_EQ          40036

#define WINAMP_FILE_LOC                 40185
#define WINAMP_FILE_PLAY                40029
#define WINAMP_FILE_DIR                 40187

//-----------------------------------------------------------------------------------------------

#include <windows.h>
#include "../gen_ml/ml.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"

//-----------------------------------------------------------------------------------------------

class Winamp2FrontEnd {
  public:
    friend BOOL CALLBACK findVisWndProc(HWND hwnd, LPARAM lParam);
    Winamp2FrontEnd();
    virtual ~Winamp2FrontEnd();
  
    void init(HWND hwndParent);

    const char *getVersion();              

    void enqueueFile(const wchar_t *file);    

    /*
    WA2_USERBUTTON_PLAY     
    WA2_USERBUTTON_PAUSE    
    WA2_USERBUTTON_STOP     
    WA2_USERBUTTON_NEXT     
    WA2_USERBUTTON_PREV     

    WA2_USERBUTTONMOD_SHIFT
    WA2_USERBUTTONMOD_CTRL  
    */
    void userButton(int button, int modifier);

    int isPlaying();                       // 0 (false) or 1 (true)
    int isPaused();                        // 0 (false) or 1 (true)
    int isStopped();                       // if !isPlaying() and !isStopped() and !isPaused(), we're between tracks

    int getPosition();                     // in ms
    int getLength();                       // in ms
    int seekTo(int ms);                    // in millisecond

    void setVolume(int v);                 // 0 (silence) to 255 (loud)
    int getVolume();                       // 0 (silence) to 255 (loud)

    void setPanning(int p);                // 0 (left) to 255 (right)
    int getPanning();                      // 0 (left) to 255 (right)

    void setShuffle(int shuffle);
    int getShuffle();

    void setRepeat(int repeat);
    int getRepeat();

    void setManualPlaylistAdvance(int manual);
    int getManualPlaylistAdvance();

    /*
    WA2_GETINFO_SAMPLERATE                 :  Samplerate (i.e. 44100)
    WA2_GETINFO_BITRATE                    :  Bitrate  (i.e. 128)
    WA2_GETINFO_CHANNELS                   :  Channels (i.e. 2)
    */
    int getInfo(int wa2_getinfo);

    /*
    WA2_EQDATA_FIRSTBAND/LASTBAND (0 to 9) :  The 10 bands of EQ data. 0-63 (+20db - -20db)
    WA2_EQDATA_PREAMP                      :  The preamp value. 0-63 (+20db - -20db)
    WA2_EQDATA_ENABLED                     :  Enabled. zero if disabled, nonzero if enabled.
    WA2_EQDATA_AUTO                        :  Autoload. zero if disabled, nonzero if enabled.
    */
    int getEqData(int wa2_eqdata);
    void setEqData(int wa2_eqdata, int val);

    void enableWindows(int enabled);
    int areWindowsEnabled();
    void setWindowsVisible(int visible);
    int areWindowsVisible();
    int isMainWindowVisible();

	void *CanPlay(const wchar_t *);
	bool IsPlaylist(const wchar_t *fn);
    int getCurPlaylistEntry();
    int getPlaylistLength();

	const wchar_t *GetCurrentTitle();
	const wchar_t *GetCurrentFile();
    const wchar_t *getTitle(int plentry);
    const char *getFile(int plentry);
	const wchar_t *getFileW(int plentry);

	void setCurTitle(const wchar_t* new_title);

    void playAudioCD(int cd);              // id of cd (0 to 4)

    void openFileDialog(HWND w);
    void openUrlDialog(HWND w);
		void openUrl(const wchar_t *url);
    void openDirectoryDialog(HWND w);
    void ejectPopupMenu();
    void previousPopupMenu();
    void nextPopupMenu();
    void playPopupMenu();
    void pausePopupMenu();
    void stopPopupMenu();
    void setDialogBoxParent(HWND w);
		void updateDialogBoxParent(HWND w);

    /*
    IPC_GETWND_EQ 
    IPC_GETWND_PE
    IPC_GETWND_MB
    IPC_GETWND_VIDEO
    */
    HWND getWnd(int wnd);
    int isWindowVisible(intptr_t which);
    void setWindowVisible(intptr_t which, int visible);
    HWND getMediaLibrary();
    void ensureMediaLibraryLoaded();

    int isOnTop();
    void setOnTop(int ontop);
    void toggleOnTop();

    // screen coordinates
    void triggerPopupMenu(int x, int y);
    void triggerEQPresetMenu(int x, int y);
    int triggerFileMenu(int x, int y, int width, int height);
    int triggerPlayMenu(int x, int y, int width, int height);
    int triggerOptionsMenu(int x, int y, int width, int height);
    int triggerWindowsMenu(int x, int y, int width, int height);
    int triggerHelpMenu(int x, int y, int width, int height); 
    int triggerPEFileMenu(int x, int y, int width, int height);
    int triggerPEPlaylistMenu(int x, int y, int width, int height);
    int triggerPESortMenu(int x, int y, int width, int height);
    int triggerPEHelpMenu(int x, int y, int width, int height);
    int triggerMLFileMenu(int x, int y, int width, int height);
    int triggerMLViewMenu(int x, int y, int width, int height);
    int triggerMLHelpMenu(int x, int y, int width, int height);
    int triggerPEListOfListsMenu(int x, int y);
    
    HMENU getTopMenu();
    HMENU getPopupMenu();
    int adjustOptionsPopupMenu(int direction);
    
    enum {
      WA2_MAINMENUBAR_FILE     = 0,
      WA2_MAINMENUBAR_PLAY     = 1,
      WA2_MAINMENUBAR_OPTIONS  = 2,
      WA2_MAINMENUBAR_WINDOWS  = 3,
      WA2_MAINUMENUBAR_HELP    = 4,
    };

    HMENU getMenuBarMenu(int which); 
    int adjustFFWindowsMenu(int direction);
    int adjustFFOptionsMenu(int direction);

    HWND getMainWindow();

    void quit();

    char * (*export_sa_get_deprecated)();
		char * (*export_sa_get)(char data[75*2+8]);
    void (*export_sa_setreq)(int);
		int (*export_vu_get)(int channel);

    enum {
      WA2_PLEDITPOPUP_ADD  = 0,
      WA2_PLEDITPOPUP_REM  = 1, 
      WA2_PLEDITPOPUP_SEL  = 2,
      WA2_PLEDITPOPUP_MISC = 3,
      WA2_PLEDITPOPUP_LIST = 4,
    };

    void sendPlCmd(int which, int x=0, int y=0, int menu_align_flag=0);

    enum {
      WA2_MBCMD_BACK    = 0,
      WA2_MBCMD_FORWARD = 1,
      WA2_MBCMD_STOP    = 2,
      WA2_MBCMD_RELOAD  = 3,
      WA2_MBPOPUP_MISC  = 4,
    };

    void registerGlobalHotkey(const char *name, int msg, int wparam, int lparam, int flags, const char *id);

#ifdef MINIBROWSER_SUPPORT
    void sendMbCmd(int which, int x=0, int y=0, int menu_align_flag=0);
#endif

    enum {
      WA2_VIDCMD_FULLSCREEN = 0,
      WA2_VIDCMD_1X         = 1,
      WA2_VIDCMD_2X         = 2,
      WA2_VIDCMD_LIB        = 3,
      WA2_VIDPOPUP_MISC     = 4,
      WA2_VIDCMD_EXIT_FS    = 5,
    };

    void sendVidCmd(int which, int x=0, int y=0, int menu_align_flag=0);
	int hasVideoSupport();
    int isPlayingVideo();
    int isPlayingVideoFullscreen();
    int isDoubleSize();
    int getTimeDisplayMode();

    void toggleVis();
    int isVisRunning();
    HWND getVisWnd();

    IDropTarget *getDropTarget();

    int getBitrate(); // in kbps
    int getSamplerate(); // in khz
    int getChannels(); // 1 mono, 2 stereo ...

    int isValidEmbedWndState(embedWindowState *ws);

    int PE_getNumItems();
    fileinfo2 *PE_getFileTitle(int index);
	fileinfo2W *PE_getFileTitleW(int index);
    int PE_getCurrentIndex();
    void PE_setCurrentIndex(int i);

    void switchSkin(const wchar_t *skinname);
    void visNext();
    void visPrev();
    void visRandom(int set);
    void pollVisRandom();
    void visFullscreen();
    void visConfig();
    void visMenu();

    void setIdealVideoSize(int w, int h) { video_ideal_width = w; video_ideal_height = h; }
    void getIdealVideoSize(int *w, int *h);

    int getStopOnVideoClose();
    void setStopOnVideoClose(int stop);

	int GetVideoResize();
    void SetVideoResize(int stop);

    virtual int isVis(HWND hwnd); // checks children too

    HWND getPreferencesWindow();
    void setPlEditWidthHeight(int width, int height);

    HINSTANCE getLanguagePackInstance();

    void openTrackInfo();
    const char *getOutputPlugin();

    void setDrawBorders(int d);
    void disableSkinnedCursors(int disable);

    int getMetaData(const wchar_t *filename, const wchar_t *name, wchar_t *data, int data_len);
	void GetFileInfo(const wchar_t *filename, wchar_t *title, int titleCch, int *length);

    void invalidateCache();

    const char *getVideoInfoString();

    void playFile(const wchar_t *file);
    void rewind5s();
    void forward5s();
    void endOfPlaylist();
    void startOfPlaylist();
    void stopWithFade();
    void stopAfterCurrent();

	void clearPlaylist();

    int isWindowShade(int wnd);

	int getCurTrackRating();
	void setCurTrackRating(int rating);

    int isExitEnabled();
    int pushExitDisabled();
    int popExitDisabled();


	int DownloadFile(const char *url, const wchar_t *destfilepath = L"", bool addToMl = true, bool notifyDownloadsList = true);
	void getDownloadPath(wchar_t path2store[MAX_PATH]);
	void setDownloadPath(const wchar_t * path2store);

	bool GetAlbumArt(const wchar_t *filename);
	bool IsWinampPro();

private:
    void setFoundVis() { foundvis = 1; }
    char *m_version;
    HWND hwnd_winamp;
    HWND hwnd_playlist;
    int foundvis;
    int enabled;
    int visible;
    int video_ideal_width;
    int video_ideal_height;
    DWORD cached_length_time;
    int got_length_cache;
    int cached_length;

    DWORD cached_pos_time;
    int got_pos_cache;
    int cached_pos;

    int saved_video,
#ifdef MINIBROWSER_SUPPORT
      saved_mb, 
#endif
      saved_pe, saved_eq, saved_main;
};

//-----------------------------------------------------------------------------------------------

extern Winamp2FrontEnd wa2;

BOOL DoTrackPopup(HMENU hMenu, UINT fuFlags, int x, int y, HWND hwnd);

#endif