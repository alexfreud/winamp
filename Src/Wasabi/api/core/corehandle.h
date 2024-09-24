#ifndef _COREHANDLE_H
#define _COREHANDLE_H

#include <api/syscb/callbacks/corecb.h>

// a helper class to access the playback cores within an object for you

typedef unsigned int CoreToken;
#define NO_CORE_TOKEN (CoreToken)(0xffffffff)

// Fwd References
class CfgItem;
class ItemSequencer;

/**
  Helper class to access the currently instantiated
  playback cores.
  
  To create a corehandle on the main playback core
  use the core token value from the enum ("maincore_token") or
  the core name "main".
  
  Here is an example:
    CoreHandle * ch = new CoreHandle("main");
  
  @short  Access playback cores.
  @author Nullsoft
  @ver 1.0
  @see Core
*/
class CoreHandle {
public:
  enum { maincore_token=0 };

  /**
    Create a new CoreHandle while optionally
    setting the core token to be used.
    
    Core tokens are handles to cores
    currently instantiated.
    
    @see CoreHandle(const char *name)
    @param token Core token of the core to attach to.
  */
  CoreHandle(CoreToken token=maincore_token);
  
  /**
    Create a new CoreHandle for a core
    using it's name.

    The main core name is "main".
    
    @see CoreHandle(CoreToken toke=maincore_token)
    @param name Name of the core to attach to.
  */
  CoreHandle(const wchar_t *name);
  
  /**
    Detaches the CoreHandle from the Core.
  */
  virtual ~CoreHandle();

  int isCoreLoaded();	// are we attached to a core?
  
  /**
    Get the list of supported extensions
    from the core. This is a zero delimited list 
    with double zero termination.
    
    @see getExtSupportedExtensions()
    @see getExtensionFamily()
    @ret List of supported extensions.
  */
  const char *getSupportedExtensions(); //just the *.mp3 or whatever
  
  /**
    Get the extended list of supported extensions
    from the core. This will include the proper 
    names of the extensions, for example: 
    "MP3 Files (*.mp3)".
    
    This is returned as a zero delimited list with double
    zero termination. 
    
    @see getSupportedExtensions()
    @see getExtensionFamily()
    @ret Extended list of supported extensions.
  */
  const char *getExtSupportedExtensions(); // including names
  
  /**
    Get the family name to which the extension
    is associated with (Families are "Audio", "Video", etc.)
    
    @see getExtSupportedExtensions()
    @see getSupportedExtensions()
    @ret Family name of the extension.
    @param extension Extension to get family name of.
  */
  const wchar_t *getExtensionFamily(const wchar_t *extension);

  /**
    Register an extension with the core.
    
    @see unregisterExtension()
    @see getExtSupportedExtensions()
    @see getSupportedExtensions()
    @see getExtensionFamily()
    @param extensions Extension to register.
    @param extension_name Name of the extension.
  */
  void registerExtension(const char *extensions, const char *extension_name);
  
  /**
    Unregister an extension with the core.
    
    @see registerExtension()
    @see getExtSupportedExtensions()
    @see getSupportedExtensions()
    @see getExtensionFamily()
    @param extensions Extension to unregister.
  */
  void unregisterExtension(const char *extensions);

  String getTitle();

  /**
    Set the next file to be played by the core.
    
    You can manually select the output of this
    file. Either "WAVEOUT" or "DIRECTSOUND". If
    you do not specify one, it will be automatically
    selected for you.
    
    @ret 1, success; 0, failure;
    @param playstring   Playstring of the next file to be played.
    @param destination  Output to be used for the next file.
  */
  int setNextFile(const char *playstring, const char *destination=NULL);

  /**
    Get the playback status of the core.
    
    @see pause()
    @see play()
    @see stop()
    @ret -1, Paused; 0, Stopped; 1, Playing;
  */
  int getStatus(); // returns -1 if paused, 0 if stopped and 1 if playing
  
  /**
    Get the playstring of the currently playing
    item.
    
    @ret Playstring of the currently playing item.
  */  
  const char *getCurrent();
  
  /**
    Get the number of items (tracks) present
    in the currently registered sequencer.
    
    @see getCurPlaybackNumber()
    @ret Number of items present in the sequencer.
  */
  int getNumTracks();
  
  /**
    Get the index number of the currently
    playing item of the currently registered
    sequencer.
    
    @see getNumTracks()
    @ret Index number (in the sequencer) of the item playing.
  */
  int getCurPlaybackNumber();
  
  /**
    Get the playback position of the currently
    playing file.
    
    @see getWritePosition()
    @see getLength()
    @see setPosition()
    @ret Position in the file (in milliseconds).
  */
  int getPosition();
  
  /**
    Help?
    
    @see getPosition()
    @see getLength()
    @see setPosition()
    @ret Current write position (in milliseconds).
  */
  int getWritePosition();
  
  /**
    Seek to a specific position in the
    currently playing item.
    
    @see getPosition()
    @see getLength()
    @see getWritePosition()
    @ret 1, Success; 0, Failure;
    @param ms Position in the file (in milliseconds, 0 being the beginning). 
  */
  int setPosition(int ms);
  
  /**
    Get the length of the currently 
    playing item.
    
    @see getPosition()
    @see setPosition()
    @see getWritePosition()
    @ret Length of the item (in milliseconds).
  */
  int getLength();

  // this method queries the core plugins directly, bypassing the db
  /**
  */
  int getPluginData(const char *playstring, const char *name,
    char *data, int data_len, int data_type=0); // returns size of data

  /**
    Get the volume of the core.
    
    @see setVolume()
    @see getMute()
    @see setMute()
    @ret Volume (0 to 255).
  */
  unsigned int getVolume(); // 0..255
  
  /**
    Set the volume of the core.
    
    @see getVolume()
    @see getMute()
    @see setMute()
    @param vol Volume (0 to 255).
  */
  void setVolume(unsigned int vol); // 0..255
  
  /**
    Get the panning value of the core.
    
    @see setPan()
    @ret Panning value (-127 [left] to 127 [right]).
  */
  int getPan(); // -127..127
  
  /**
    Set the panning value of the core.
    
    @see getPan()
    @param bal Panning value (-127 [left] to 127 [right])
  */
  void setPan(int bal); // -127..127

  /**
    Mute the output.
    
    @see getVolume()
    @see setVolume()
    @see getMute()
    @param mute 0, No muting; 1, Mute;
  */
  void setMute(int mute);
  
  /**
    Get the mute state of the output.
    @see getVolume()
    @see setVolume()
    @see setMute()
    @ret 0, Not muted; 1, Muted;
  */
  int getMute();

  // register here for general callbacks in core status.
  /**
    Register a callback with the core to 
    receive core status callbacks.
    
    @see delCallback()
    @param cb Core Callback to register.
  */  
  void addCallback(CoreCallback *cb);
  
  /**
    Unregister a callback with the core.
    
    @see addCallback()
    @param cb Core Callback to unregister.
  */
  void delCallback(CoreCallback *cb);

  // get visualization data, returns 0 if you should blank out
  /**
    Get visualization data for the currently
    playing item.
    
    We suggest using a struct like this to read the vis
    data:
    
    typedef struct {
      enum {
        LEFT = 0,
        RIGHT = 1
      };
    unsigned char spectrumData[2][576];
    char waveformData[2][576];
    } VisData;
    
    A call using this struct would like so:
    
      getVisData(&myVisData, sizeof(VisData));
    
    @see getLeftVuMeter()
    @see getRightVuMeter()
    @ret 0, If there is no VIS data; > 0, VIS data available;
    @param dataptr Buffer to receive VIS data.
    @param sizedataptr Size of the buffer.
  */
  int getVisData(void *dataptr, int sizedataptr);
  
  /**
    Get the value of the left VU meter.
    
    @see getVisData()
    @see getRightVuMeter()
    @ret Value of the left VU meter (0 to 255).
  */
  int getLeftVuMeter();
  
  /**
    Get the value of the left VU meter.
    
    @see getVisData()
    @see getLeftVuMeter()
    @ret Value of the right VU meter (0 to 255).
  */
  int getRightVuMeter();
  
  /**
    Register an item sequencer with the core.
    The item sequencer feeds the playstrings
    of the next item to be played to the core.
    
    @see deregisterSequencer()
    @ret 1, Success; 0, Failure;
    @param seq Sequencer to register.
  */
  int registerSequencer(ItemSequencer *seq);
  
  /**
    Unregister a sequencer with the core.
    
    @see registerSequencer()
    @ret 1, Success; 0, Failure;
    @param seq Sequencer to unregister.
  */
  int deregisterSequencer(ItemSequencer *seq);

  ItemSequencer *getSequencer();

  /**
    Get the EQ status.
    
    @see setEqStatus()
    @see getEqPreamp()
    @see setEqPreamp()
    @see getEqBand()
    @see setEqBand()
    @ret 1, On; 0, Off;
  */
  int getEqStatus(); // returns 1 if on, 0 if off
  
  /**
    Set the EQ state.
    
    @see getEqStatus()
    @see getEqPreamp()
    @see setEqPreamp()
    @see getEqBand()
    @see setEqBand()
    @param enable 1, On; 0, Off;
  */
  void setEqStatus(int enable);
  
  /**
    Get the pre-amp value of the EQ.
    
    @see setEqStatus()
    @see getEqStatus()
    @see setEqPreamp()
    @see getEqBand()
    @see setEqBand()
    @ret Pre-amp value (-127 [-20dB] to 127 [+20dB]).
  */
  int getEqPreamp(); // -127 to 127 (-20db to +20db)
  
  /**
    Set the pre-amp value of the EQ.

    @see setEqStatus()    
    @see getEqStatus()
    @see getEqPreamp()
    @see getEqBand()
    @see setEqBand()
    @param pre Pre-amp value (-127 [-20dB] to 127 [+20dB]).
  */
  void setEqPreamp(int pre);
  
  /**
    Get the value of an EQ band. There
    are 10 bands available.
    
    Here is the list:
    
    0 - 60 Hz    1 - 170 Hz
    2 - 310 Hz   3 - 600 Hz
    4 - 1 kHz    5 - 3 kHz
    6 - 6 kHz    7 - 12 kHz
    8 - 14 kHz   9 - 16 kHz

    @see setEqStatus()    
    @see getEqStatus()
    @see getEqPreamp()
    @see setEqBand()
    @ret EQ band value (-127 [-20dB] to 127 [+20dB]).
    @param band EQ band to read (0 to 9).
  */
  int getEqBand(int band); // band=0-9
  
  /**
    Set the value of an EQ band. There
    are 10 bands available.

    Here is the list:
    
    0 - 60 Hz    1 - 170 Hz
    2 - 310 Hz   3 - 600 Hz
    4 - 1 kHz    5 - 3 kHz
    6 - 6 kHz    7 - 12 kHz
    8 - 14 kHz   9 - 16 kHz

    @see setEqStatus()    
    @see getEqStatus()
    @see getEqPreamp()
    @see setEqBand()
    @param band EQ band to set (0 to 9)
    @param val  EQ band value (-127 [-20dB] to 127 [+20dB]).
  */
  void setEqBand(int band, int val);
  
  /**
    Get the automatic EQ preset loading state.
    
    @see setEqAuto()
    @ret 1, On; 0, Off;
  */
  int getEqAuto(); // returns 1 if on, 0 if off
  
  /**
    Set the automatic EQ preset loading.
    
    @see getEqAuto()
    @param enable 1, On; 0, Off;
  */
  void setEqAuto(int enable);

  /**
    Trigger the previous event.
    
    @see next()
    @see play()
    @see stop()
    @see pause()
  */
  void prev();
  
  /**
    Trigger the play event.

    @see prev()    
    @see next()
    @see stop()
    @see pause()
  */
  void play();
  
  /**
    Trigger the pause event.
    
    @see prev()    
    @see next()
    @see stop()
    @see play()
  */
  void pause();
  
  /**
    Trigger the stop event.
  */
  void stop();
  
  /**
    Trigger the next event.
    
    @see prev()    
    @see stop()
    @see play()
    @see pause()
  */
  void next();
  
  /**
    Set the thread priority of the core.
    
    @see getPriority()
    @param priority Thread priority.
  */
  void setPriority(int priority);
  
  /**
    Get the thread priority of the core.
    
    @see setPriority()
    @ret Thread priority level.
  */
  int getPriority();

  /**
    As the function name implies, rebuilds the converters chain (no shit?)
  */
  void rebuildConvertersChain();

  /**
    Send a message to all converters in the current
    playback chain of a core.

    It's possible to pass any pointer using this messanging
    system, as long as the pointer is valid across dll boundries.

    @param msg Message.
    @param value Message value.
  */
  int sendConvertersMsg(const char *msg, const char *value);

private:
  void userButton(int button);
  CoreToken token;
  int createdcore;
};

#endif
