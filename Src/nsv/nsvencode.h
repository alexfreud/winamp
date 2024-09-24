#ifndef _NSVENCODE_H_
#define _NSVENCODE_H_

#include "../jnetlib/jnetlib.h"
#include "nsvlib.h"
#include "enc_if.h"
#include <stdio.h>

class nsv_Encoder
{
  public:
    nsv_Encoder();
    ~nsv_Encoder();


    static void freeCoders(); // call this before quitting if you used any nsvencoders
    static void loadCoders();
    static HWND configAudio(unsigned int audfmt, HWND hwndParent, char *configfile);
    static HWND configVideo(unsigned int vidfmt, HWND hwndParent, char *configfile);
    static int enumAudio(int *state, char *name, unsigned int *fmt);
    static int enumVideo(int *state, char *name, unsigned int *fmt);


    char *getError() { return m_err; } // NULL on no error

    // compressor configuration (do not call these after calling openOutput())
    void setVideo(unsigned int vfmt, unsigned int w, unsigned int h, double frt, 
                  unsigned int srcfmt=NSV_MAKETYPE('R','G','B','A'),
                   char *configfile=NULL);
    void setAudio(unsigned int afmt, int srate, int nch, int bps, 
                  unsigned int srcfmt=NSV_MAKETYPE('P','C','M',' '),
                   char *configfile=NULL);

    // stream configuration (do not call these after calling openOutput())
    void setMaxAudioSendAhead(int offs) { m_audio_ahead_of_video=offs; }
    void setSyncFrameInterval(int minimum=0, int maximum=120) { m_max_syncframe_dist=maximum; m_min_syncframe_dist=minimum; }

    // setting one of these will enable file header writing on files
    // do NOT call these after calling openOutput(),
    int addHdrMetaData(char *name, char *data, int data_len); // returns nonzero on error
    int setHdrMetaData(char *data, int data_len); // nonzero on error
    void setHdrTOCSize(int tocsize);
    void forceHdrWrite() { m_usefilehdr=1; } // use if you dont want meta or toc, but want a basic header

    // output (call only once, do not close and reopen (yet))
    void openOutput(char *url); // call getError to check for error
    void closeOutput();
    
    // encode (call only after openoutput())
    void addAudioData(void *data, int data_len);
    int needAudioData(); // returns 1 if more audio data would be nice for sync, 2 if there is no audio data in buffer

    // for audio only, use compressor type NONE and call this with NULL 
    void addVideoFrame(void *frame);
    // calling this will put the aux data in the same frame as the NEXT addVideoFrame()
    void addAuxData(unsigned int fmt, void *data, int data_len);
    
    // setting audio eof will let the audio compressor know that it is the end of stream
    // so that it can flush (via passing an empty frame to it)
    void set_audioEof(int eof) { m_audio_eof=!!eof; }
    int get_audioEof() { return m_audio_eof; }

    // stats
    unsigned int getCurrentBitrate()
    {
      if (m_video_position_frames)
        return (unsigned int) ((double) m_bitstreambytesout / ((double) m_video_position_frames / m_framerate) * 8.0);
      return 0;
    }


  private:

    char *m_err;

    int m_usefilehdr;
    unsigned int m_tocsize;
    int m_audio_channels;
    int m_audio_samplerate;
    int m_audio_bits;
    unsigned int m_audio_srcfmt;
    unsigned int m_audio_coder;
    char *m_audio_configfile;


    double m_framerate;
    unsigned int m_w, m_h;
    unsigned int m_video_coder;
    unsigned int m_video_srcfmt;
    char *m_video_configfile;

    int m_audio_ahead_of_video;

    int m_max_syncframe_dist,m_min_syncframe_dist;
    nsv_fileHeader m_filehdr;

    FILE *m_outfile;
    JNL_Connection *m_outcon;
    int m_is_uvox; // for m_outco
    char *m_outcon_host,*m_outcon_headers;
    int m_outcon_port;
    unsigned int m_outcon_last_connect_time;

    nsv_Packeter m_packeter;
    nsv_InBS m_outconbs;
    __int64 m_bitstreambytesout;

    AudioCoder *m_audcoder;
    VideoCoder *m_vidcoder;

    __int64 m_audio_position_bytes;
    __int64 m_video_position_frames;
    char m_vidobuf[NSV_MAX_VIDEO_LEN];
    char m_audobuf[NSV_MAX_AUDIO_LEN];
    nsv_InBS m_audibuf;
    unsigned int m_total_frames;
    nsv_GrowBuf m_tocEntries;
    int m_frames_since_last_sync_frame;
    int m_audio_eof;

    void send_uvoxmessage(int msgclass, int msgtype, void *msg, int msglen);


    static VideoCoder *init_video(int w, int h, double frt, unsigned int pixt, unsigned int *outt, char *configfile);
    static AudioCoder *init_audio(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile);
    static int Coders_loaded;
    static HINSTANCE g_Coders[256];
    static int g_numCoders;
    static char g_dll_dir[MAX_PATH];
    static BOOL CALLBACK PcmProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);



};



class nsv_EncoderConfigFile
{
  public:

    nsv_EncoderConfigFile(char *filename=NULL) 
    { 
      m_filename=filename?_strdup(filename):0; 
      m_audfmt=m_vidfmt=0;
      m_minsync=0;
      m_maxsync=120;
      m_audiosendahead=0;
      m_hdrwrite=0;
      m_hdrtoc=1024;
      m_metadata=0;
	  m_crop_l=0;	// ##
	  m_crop_r=0;	// ##
	  m_crop_t=0;	// ##
	  m_crop_b=0;	// ##

      ReadIn();
    }


    ~nsv_EncoderConfigFile(); //ak 7-23-03 moved so breakpoint can be set
    unsigned int getVidFmt() { return m_vidfmt; }
    unsigned int getAudFmt() { return m_audfmt; }
    void setVidFmt(unsigned int fmt) { m_vidfmt=fmt; }
    void setAudFmt(unsigned int fmt) { m_audfmt=fmt; }

    char *getFilename() { return m_filename; }
    void setFilename(char *filename) 
    { 
      FlushOut();
      free(m_filename); 
      if (filename) 
      {
        m_filename=_strdup(filename); 
        ReadIn();
      }
      else m_filename=0; 
    }

    int getMinSyncFrameInt() { return m_minsync; }
    void setMinSyncFrameInt(int minint) { m_minsync=minint; }
    int getMaxSyncFrameInt() { return m_maxsync; }
    void setMaxSyncFrameInt(int maxint) { m_maxsync=maxint; }
    int getAudioSendAhead() { return m_audiosendahead; }
    void setAudioSendAhead(int sa) { m_audiosendahead=sa; }

    char *hdrGetMetadata() { return m_metadata; }
    int hdrGetDoWrite() { return m_hdrwrite; }
    int hdrSetDoWrite(int dw) { m_hdrwrite=!!dw; }
    int hdrGetTOCSize() { return m_hdrtoc; }
    int hdrSetTOCSize(int ts) { m_hdrtoc=ts; }

    int ConfigUI(HINSTANCE hInstance, HWND hwndParent);

  private:

    void FlushOut();
    void ReadIn();

    static BOOL CALLBACK _dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
    BOOL dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
    HWND audio_hwnd, video_hwnd;
    void initConfigChildWindows(HWND hwndDlg, int flags);

    char *m_filename;

    unsigned int m_audfmt,m_vidfmt;
    int m_minsync, m_maxsync;
    int m_audiosendahead;
    int m_hdrwrite;
    int m_hdrtoc;
    char *m_metadata;

  public:
	int m_crop_l;	// ##
	int m_crop_r;	// ##
	int m_crop_t;	// ##
	int m_crop_b;	// ##
};

#endif//_NSVENCODE_H_