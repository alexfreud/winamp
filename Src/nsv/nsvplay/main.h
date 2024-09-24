#ifndef NSVPLAY_MAIN_H
#define NSVPLAY_MAIN_H

#include <bfc/platform/types.h>
#include "../nsvlib.h"
#include "../dec_if.h"

#define SHOW_STREAM_TITLE_AT_TOP 1

class Subtitles;
class SubsItem;

#include "IDataReader.h"


typedef struct
{
  const char *language;
  const char *utf8_text;
  unsigned int start_frame, end_frame;
  unsigned char xPos, yPos;
  unsigned char colorRed, colorGreen, colorBlue;
  signed char fontSize;
  int extraDataSize;
  const void *extraData;
} SUBTITLE_INFO;

class IVideoOutput
{
  public:
    virtual ~IVideoOutput() { }
    virtual int open(int w, int h, int vflip, double aspectratio, unsigned int fmt)=0;
#ifdef _WIN32
    virtual void setcallback(LRESULT (*msgcallback)(void *token, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam), void *token) { }
#else
		virtual void setcallback(void *func, void *token) { } // currently unused, here to reserve the spot in the vtable
#endif
    virtual void close()=0;
    virtual void draw(void *frame)=0;
    virtual void drawSubtitle(SubsItem *item) { }
    virtual void showStatusMsg(const char *text) { }
    virtual int get_latency() { return 0; }
    virtual void notifyBufferState(int bufferstate) { } /* 0-255*/

    virtual intptr_t extended(intptr_t param1, intptr_t param2, intptr_t param3) { return 0; } // Dispatchable, eat this!
};

template<class T>
class ClassList {
public:
  ~ClassList() {
    int l=getlen()-1;
    for(int i=l;i>-1;i--) delete(get(i));
  }
  void put(T *item) {
    m_buf.add(&item,sizeof(T *));
  }
  T *get(int n) {
    if(n>=getlen()) return NULL;
    return ((T **)m_buf.get())[n];
  }
  int getlen() {
    return (int)(m_buf.getlen()/sizeof(SubsItem *));
  }
private:
   GrowBuf m_buf;
};

class SubtitlesItem {
public:
  SubtitlesItem(const char *language, Subtitles *subs) :
    m_subs(subs) { 
      m_language=_strdup(language);
      m_subs=subs;
    }
  ~SubtitlesItem() {
    free((void *)m_language);
  }
  const char *m_language;
  Subtitles *m_subs;
};

class NSVDecoder {

public:

  NSVDecoder(const char *url, IVideoOutput *output, char *subtitleurl=NULL);
  ~NSVDecoder();

  char *get_error();
  char *get_status();
  int run(int * volatile quit=NULL);

  void pause(int pause);  
  ULONGLONG getpos();
  unsigned int getpos_frames() { return framecnt; }
  unsigned int getlen(); // returns 0xFFFFFFFF on unknown
  
  void setvolume(int volume) { m_volume=volume; if (aud_output) aud_output->setvolume(volume); }
  void setpan(int pan) { m_pan=pan; if (aud_output) aud_output->setpan(pan); }

  int getvolume() { return m_volume; }
  int getpan() { return m_pan; }

  int canseek();
  void seek(unsigned int newpos);

  char *getFromMeta(char *name);
  char *getUrl() { return m_url; }
  const char *getServerHeader(char *name);

  char *getTitle();
  char *getStatus();

  void getAudioDesc(char *buf);
  void getVideoDesc(char *buf);

  char *getAudioType() { return m_audio_type; }
  char *getVideoType() { return m_video_type; }
  unsigned int getFileSize(); // 0xFFFFFFFF if unknown
  int   getBitrate();
  int   getAudioBitrate();
  int   getVideoBitrate();
  int   getWidth() { return unpacket.getWidth(); }
  int   getHeight() { return unpacket.getHeight(); }
  double getFrameRate() { return use_framerate_override?framerate_override:unpacket.getFrameRate(); }
  int   getBufferPos() { if (m_prebuffer) return m_bufstate; return 256; } // 0-256

  int subsEnabled() { return m_enable_subtitles; }
  void enableSubs(int e) { 
    m_enable_subtitles=e; 
    if(!e&&m_out) m_out->drawSubtitle(NULL); 
  }
  int getSubsFontSize() { return m_subs_fontsize; }
  void setSubsFontSize(int s) { 
    m_subs_fontsize=s; 
    if(m_out) m_out->drawSubtitle(NULL); //will redraw current subtitle with new size
  }

  void SetPreciseSeeking(int prec) { m_precise_seeking=prec; }
  void SetBuffering(int total_ms, int initial_ms, int after_underrun);
  void SetBufferMemoryLimit(int bytes) { m_buf_memlimit=bytes; }

  const char *getSubLanguage(int index);
  void setSubLanguage(int index) { m_cur_subtitle=index; }
  int getCurSubLanguage() { return m_cur_subtitle; }

	void CloseVideo()
	{
		if (m_out_opened)
			 m_out->close();
		m_out_opened=0;
	}

private:
  ULONGLONG m_avresync_time;
  int m_pb_init,m_pb_init_ur,m_buffer_total;
  int m_prebuffer;
  int m_bufstate;

  int	proTimerStart;
	int proTimerEnd;
	float profiletime;
	float prostart;
	float proend;
	float timeref; 

  int m_again;

  void ProcessSubtitleBlock(void *data, int len);

  int m_paused;

  ULONGLONG hack_l_curpos;
  ULONGLONG hack_l_curpos_ot;

  int m_buf_memlimit;
  IVideoOutput *m_out;
  int m_out_opened;

  nsv_InBS inbs;
  nsv_InBS audiobs,videobs;  
  nsv_InBS auxbs;
  int video_frames_avail,audio_frames_avail;
  nsv_Unpacketer unpacket;
  unsigned int framecnt;
  int hdrsearched;
  int nsvbitstream_search;
  int64_t m_audio_writepos;
  unsigned int m_need_seek;

  int seek_dumpframes, seek_dumpaudiosamples;
  int pcm_samplerate;

  int m_precise_seeking;

  char *m_err;
  char *m_url;
  char *m_title;

  int vid_decoder_isnotnull,aud_decoder_isnotnull;
  IVideoDecoder *vid_decoder;
  IAudioDecoder *aud_decoder;
  IAudioOutput *aud_output;
  IDataReader *file_reader;
  int needkf;
  double aspect;
  double framerate_override;
  int use_framerate_override;

  nsv_fileHeader fileheader;

  unsigned int avg_framesize_cnt,avg_framesize_tot;
  unsigned int avg_framesize_cnt_v,avg_framesize_tot_v;
  unsigned int avg_framesize_cnt_a,avg_framesize_tot_a;

  int vidout_ready;
  int vid_flip;
  void *vidout;
  unsigned int vidout_type;
  unsigned int vidout_time;
	unsigned int vidout_codec_width;
	unsigned int vidout_codec_height;

  char m_audio_type[5];
  char m_video_type[5];

  int m_volume, m_pan;

  int m_enable_subtitles;
  int m_subs_fontsize;
  ClassList<SubtitlesItem> m_subtitles;
  int m_cur_subtitle;

  Subtitles *insertSubtitlesItem(const char *language, const char *subfile);
  Subtitles *findSubtitles(const char *language);

};

void Decoders_Init(char *wapluginspath=NULL);
void Decoders_Quit();


IAudioDecoder *CreateAudioDecoder(unsigned int type, int *wasNotNull, IAudioOutput **output);
IVideoDecoder *CreateVideoDecoder(int w, int h, double framerate, unsigned int type, int *flip, int *wasNotNull=NULL);
IDataReader *CreateReader(const char *url);

#endif //NSVPLAY_MAIN_H