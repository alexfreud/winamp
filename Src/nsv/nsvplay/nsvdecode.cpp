#include <bfc/platform/platform.h>
#include <bfc/platform/strcmp.h>
#include <stdio.h>
#include <locale.h>
#include "audiostub.h"
#include "api.h"
#include "main.h"
#include "subtitles.h"
#include "../in_nsv/resource.h"

extern int config_subtitles;
#ifdef __APPLE__
#include <sys/time.h>
#include <unistd.h>
uint32_t GetTickCount()
{
  struct timeval newtime;
  
	gettimeofday(&newtime, 0);
  return newtime.tv_sec*1000 + newtime.tv_usec/1000;
}

int MulDiv(int a, int b, int d)
{
  return int ((int64_t)a * (int64_t)b) / (int64_t)d;
}
#endif
class NullAudioOutput : public IAudioOutput 
{
  public:
    NullAudioOutput() { m_pause=0; m_pause_t=0; m_start_t=GetTickCount64(); }
    ~NullAudioOutput(){}
    int canwrite() { return m_pause ? 0 : 65536; } // returns bytes writeable
    void write(void *buf, int len) { }
    ULONGLONG getpos() { return m_pause?m_pause_t:(GetTickCount64()-m_start_t); }
    int isplaying(void) { return -1; }
    ULONGLONG getwritepos() { return getpos(); }
    void flush(unsigned int newtime) 
    { 
      if (m_pause) m_pause_t=newtime;
      else m_start_t = GetTickCount64()-newtime; 
    }
    void pause(int pause)
    {
      if (pause && !m_pause)
      {
        m_pause_t=GetTickCount64()-m_start_t;
        m_pause=1;
      }
      else if (!pause && m_pause)
      {
        m_start_t = GetTickCount64()-m_pause_t;
        m_pause=0;
        m_pause_t=0;
      }
    }

  private:
    int m_pause;
    ULONGLONG m_pause_t;
    ULONGLONG m_start_t;
};

void NSVDecoder::pause(int pause)
{
  if (aud_output) aud_output->pause(pause);
  m_paused=pause;
}

ULONGLONG NSVDecoder::getpos()
{
  return aud_output ? aud_output->getpos() : 0;
}

unsigned int NSVDecoder::getlen()
{
  if (!fileheader.header_size || fileheader.file_lenms == 0xFFFFFFFF)
  {
    if (!file_reader || !avg_framesize_cnt || !avg_framesize_tot) return 0xFFFFFFFF;
    int s=(int)file_reader->getsize();
    if (s == 0xFFFFFFFF) return 0xFFFFFFFF;

    int bytespersec=(int)((double)avg_framesize_tot/(double)avg_framesize_cnt*getFrameRate());

    if (!bytespersec) return 0xFFFFFFFF;
    return MulDiv(s,1000,bytespersec);
  }
  return fileheader.file_lenms;
}

void NSVDecoder::seek(unsigned int newpos)
{
  m_need_seek=newpos;
  m_again=0;
}

int NSVDecoder::canseek()
{
  return file_reader ? file_reader->canseek() : 0;
}

char *NSVDecoder::getStatus()
{
  if (m_prebuffer) return "Buffering";
  return NULL;
}

void NSVDecoder::SetBuffering(int total_ms, int initial_ms, int after_underrun)
{
  if (m_prebuffer) m_prebuffer=initial_ms;
  m_pb_init=initial_ms;
  m_pb_init_ur=after_underrun;
  m_buffer_total=total_ms;
}


int64_t pdReadResolution(void)
{
  int64_t myfeq;
#ifdef _WIN32
  LARGE_INTEGER feq;

  QueryPerformanceFrequency( &feq);
  myfeq = feq.QuadPart;
#else
  myfeq = 1000000;
#endif

  return myfeq;
}

int64_t pdReadTimer(void)
{
  int64_t mynow;

#ifdef _WIN32
  LARGE_INTEGER now;
  
  QueryPerformanceCounter( &now );
  mynow = now.QuadPart;
#else
  struct timeval newtime;

	gettimeofday(&newtime,NULL);
	mynow  = (newtime.tv_sec * 1000000) + newtime.tv_usec ;
#endif

  return mynow;
}

NSVDecoder::NSVDecoder(const char *url, IVideoOutput *output, char *subtitleurl)
{
  m_precise_seeking=1;
  pcm_samplerate=0;
  seek_dumpframes=0;
  seek_dumpaudiosamples=0;
  m_avresync_time=0;
  nsvbitstream_search=0;
  m_paused=0;
  m_bufstate=0;
  hack_l_curpos=-1;
  hack_l_curpos_ot=0;
  vid_decoder_isnotnull=1;
  aud_decoder_isnotnull=1;
  video_frames_avail=0;
  audio_frames_avail=0;
  m_buffer_total=1000; // bufferahead
  m_pb_init=m_prebuffer=1500; // initial prebuffer
  m_pb_init_ur=1500; // after underrun
  m_buf_memlimit=32*1024*1024;
  m_pan=0;
  m_volume=192;
  m_title=0;
  m_audio_type[0]=m_video_type[0]=0;
  m_out_opened=0;
  m_out=output;
  vidout_ready=0;
  vidout=0;
  vidout_type=0;
  vidout_time=0;
	vidout_codec_width=0;
	vidout_codec_height=0;
  m_url=_strdup(url);
  m_need_seek=~0;
  needkf=1;
  aspect=1.0;
  framerate_override=0.0;
  use_framerate_override=0;
  vid_decoder=NULL;
  aud_decoder=NULL;
  aud_output=NULL;
  framecnt=0;
  hdrsearched=0;
  file_reader=CreateReader(url);
  if (!file_reader) m_err="Error opening input";
  else m_err=NULL;

  m_enable_subtitles=1;
  m_subs_fontsize=0;
  m_cur_subtitle=0;
  if (subtitleurl && *subtitleurl) insertSubtitlesItem("From subtitle file",subtitleurl);

  memset(&fileheader,0,sizeof(fileheader));

  avg_framesize_tot=0;
  avg_framesize_cnt=0;
  avg_framesize_tot_v=0;
  avg_framesize_cnt_v=0;
  avg_framesize_tot_a=0;
  avg_framesize_cnt_a=0;

  unpacket.setAudioOut(&audiobs);
  unpacket.setVideoOut(&videobs);
  unpacket.setAuxOut(&auxbs);

	proTimerStart = 1;
	proTimerEnd   = 0;
	profiletime = 0.00;
	prostart = 0.00;
	proend = 0.00;
	timeref = (float)pdReadResolution();


  m_again = 0;
}

NSVDecoder::~NSVDecoder()
{
  if (m_out_opened) m_out->close();
  if (WASABI_API_MEMMGR)
		WASABI_API_MEMMGR->Delete(vid_decoder);
	else
		delete vid_decoder;
	if (WASABI_API_MEMMGR)
		WASABI_API_MEMMGR->Delete(aud_decoder);
	else
		delete aud_decoder;
  delete aud_output;
  delete file_reader;
  free(fileheader.metadata);
  free(fileheader.toc);
  free(m_url);
  free(m_title);
}

char *NSVDecoder::get_error()
{
  return m_err;
}

char *NSVDecoder::get_status()
{
  return NULL;
}

char *NSVDecoder::getFromMeta(char *name)
{
  if (file_reader)
  {
    char *t=(char*)malloc(strlen(name)+8);
    if (t)
    {
      strcpy(t,"x-nsv-");
      strcat(t,name); 
      char *v=file_reader->getheader(t);
      free(t);
      if (v) return _strdup(v);
    }
  }
  return nsv_getmetadata(fileheader.metadata,name);
}


unsigned int NSVDecoder::getFileSize()
{
  if (file_reader) return (unsigned int)file_reader->getsize();
  return ~0;
}

int NSVDecoder::getBitrate()
{
  if (!fileheader.header_size || !fileheader.file_lenms || !fileheader.file_lenbytes ||
    fileheader.file_lenms == 0xFFFFFFFF || fileheader.file_lenbytes == 0xFFFFFFFF)
  {
    if (!avg_framesize_cnt) return 0;
    return (int) (8.0 * getFrameRate() * (double)avg_framesize_tot / (double)avg_framesize_cnt);
  }
  return MulDiv(fileheader.file_lenbytes,8000,fileheader.file_lenms);
}

const char *NSVDecoder::getServerHeader(char *name)
{
  return file_reader?file_reader->getheader(name):NULL;
}

void NSVDecoder::getAudioDesc(char *buf)
{
  char *t=getAudioType();
  if (t && *t) 
  {
    if (strcmp(t, "VLB")==0)
      sprintf(buf,"Dolby AAC ");   // special case; make sure the user never sees "VLB" name
    else if (strcmp(t, "AACP")==0)
			sprintf(buf,"HE-AAC ");
		else if (strcmp(t, "AAC")==0)
			sprintf(buf,"AAC LC ");
		else
      sprintf(buf,"%s ",t);

    char *p=buf+strlen(buf);
    if (aud_output) aud_output->getdescstr(p);
    if (!*p) *--p=0;

    int a=(getAudioBitrate()+500)/1000;
	if (a) sprintf(buf+strlen(buf)," ~%d%s",a,WASABI_API_LNGSTRING(IDS_KBPS));
  }
  else *buf=0;
}

void NSVDecoder::getVideoDesc(char *buf)
{
  char *t=getVideoType();
  if (t && *t) 
  {
    int fr=(int)(getFrameRate()*100.0);
    sprintf(buf,"%s %dx%d@%d.%02d%s",t,getWidth(),getHeight(),fr/100,fr%100,WASABI_API_LNGSTRING(IDS_FPS));

    int a=(getVideoBitrate()+500)/1000;
	if (a) sprintf(buf+strlen(buf)," ~%d%s",a,WASABI_API_LNGSTRING(IDS_KBPS));
  }
  else *buf=0;
}


int NSVDecoder::getAudioBitrate()
{
  if (!avg_framesize_cnt_a) return 0;
  return (int) (8.0 * getFrameRate() * (double)avg_framesize_tot_a / (double)avg_framesize_cnt_a);
}

int NSVDecoder::getVideoBitrate()
{
  if (!avg_framesize_cnt_v) return 0;
  return (int) (8.0 * getFrameRate() * (double)avg_framesize_tot_v / (double)avg_framesize_cnt_v);
}

char *NSVDecoder::getTitle()
{
  char *v=getFromMeta("TITLE");
  if (v) 
  {
    if (!m_title || strcmp(v,m_title)) 
    {
      free(m_title);
      m_title=v;
    }
    else free(v);
    return m_title;
  }
  if (file_reader)
  {
    v=file_reader->gettitle();
    if (v) return v;
  }

  if (!m_url) return "";

  v=m_url+strlen(m_url);
  while (v >= m_url && *v != '/' && *v != '\\' && *v != '=') v--;
  return ++v;
}

void NSVDecoder::ProcessSubtitleBlock(void *data, int len)
{
  unsigned char *dataptr=(unsigned char *)data;
  while (len > 2)
  {
    int len_block=(dataptr[0] | ((unsigned short)dataptr[1] << 8)); 
    dataptr += 2;
    len -= 2;
    if (len_block > len) break;
  
    SUBTITLE_INFO sti={0,};
    sti.language = (char *)dataptr;
    while (len_block > 0 && *dataptr)
    {
      dataptr++;
      len_block--;
      len--;
    }
    dataptr++;
    len_block--;
    len--;
    if (len_block < 1) break;
    sti.utf8_text = (char *)dataptr;
    while (len_block > 0 && *dataptr)
    {
      dataptr++;
      len_block--;
      len--;
    }
    dataptr++;
    len_block--;
    len--;
    if (len_block < 6) break;

    sti.start_frame = framecnt + video_frames_avail + (dataptr[0] | ((int)dataptr[1] << 8));
    dataptr+=2;
    len-=2;
    len_block-=2;

    sti.end_frame = sti.start_frame + (dataptr[0] | ((int)dataptr[1] << 8) | ((int)dataptr[2] << 16) | ((int)dataptr[3] << 24));
    dataptr+=4;
    len-=4;
    len_block-=4;

    // set defaults for color/position
    sti.xPos=128;
    sti.yPos=255;
    sti.colorRed=255;
    sti.colorGreen=255;
    sti.colorBlue=255;
    sti.fontSize=0;

    if (len_block >= 2)
    {
      sti.xPos = *dataptr++;
      sti.yPos = *dataptr++;
      len-=2;
      len_block-=2;
      if (len_block >= 3)
      {
        sti.colorRed=*dataptr++;
        sti.colorGreen=*dataptr++;
        sti.colorBlue=*dataptr++;
        len-=3;
        len_block-=3;
        if (len_block > 0)
        {
          sti.fontSize=(signed char) *dataptr++;
          len--;
          len_block--;
        }

        if (len_block > 0)
        {
          sti.extraData=dataptr;
          sti.extraDataSize=len_block;
        }

      }
    }

    Subtitles *sub=findSubtitles(sti.language);
    if(!sub) sub=insertSubtitlesItem(sti.language,NULL);
    sub->addSubtitlePacket(&sti);

    if (len_block > 0)
    {
      len-=len_block;
      dataptr+=len_block;
    }

  }
}

int NSVDecoder::run(int * volatile quit) // returns -1 on error, 0 if no frames processed, 1 if frames processed, 2 if underrun
{
  int retval=0;


  if (!file_reader) return -1;

  if (!aud_decoder_isnotnull && !vid_decoder_isnotnull && vid_decoder && aud_decoder)
  {
    m_err="Codec(s) not found";
    return -1;
  }

  if (aud_output && vidout_ready)
  {
      ULONGLONG curpos=aud_output->getpos();
    if (!m_paused)
    {
      if (!audio_frames_avail && curpos == hack_l_curpos && unpacket.getEof())
      {
        hack_l_curpos=curpos;
        curpos += GetTickCount64() - hack_l_curpos_ot;
      }
      else
      {
        hack_l_curpos_ot=GetTickCount64();
        hack_l_curpos=curpos;
      }
    }

    if (curpos >= vidout_time && !m_prebuffer)
    {
			if (vidout) 
			{
				// send this to get the flip state updated so when toggled on-the-fly it will work with nsv
				m_out->extended(0x1002/*VIDUSER_SET_VFLIP*/,vid_flip,0);
				m_out->draw(vidout);
			}

      if (m_enable_subtitles) 
      {
        SubtitlesItem *it=m_subtitles.get(m_cur_subtitle);
        if(it && config_subtitles) {
          it->m_subs->setFontSizeModifier(m_subs_fontsize);
          m_out->drawSubtitle(it->m_subs->getSubtitle((unsigned int)aud_output->getpos(),framecnt));
        }
		if ( it && !config_subtitles )
		{
			m_out->drawSubtitle(NULL);
		}
      }
      vidout=0;
      vidout_ready=0;
    }
  }

  if (hdrsearched && m_need_seek!=~0 && aud_output)
  {
    unsigned int newpos=m_need_seek;
    m_need_seek=~0;
    seek_dumpaudiosamples=0;
    seek_dumpframes=0;
    if (file_reader->canseek() && file_reader->getsize() != 0xFFFFFFFF)
    {
      int nbpos;

      if (!fileheader.toc_size || !fileheader.toc || !fileheader.file_lenms || 
          !fileheader.file_lenbytes ||
          fileheader.file_lenms == 0xFFFFFFFF || fileheader.file_lenbytes == 0xFFFFFFFF)
      {
        int avg_framesize=avg_framesize_tot/avg_framesize_cnt;
        int pos_frames=(int)(newpos*getFrameRate());
        nbpos=fileheader.header_size+MulDiv(pos_frames,avg_framesize,1000);
      }
      else // calculate offset using TOC
      {
        if (fileheader.toc_ex) // use extended toc, find next earliest time closest
        {
          int x;
          double scale;
          if (unpacket.isValid() && getFrameRate() > 0.0001)
            scale=1000.0 / getFrameRate();
          else scale = 1000.0 / 30.0;
          for (x = 0; x < (int)fileheader.toc_size; x ++)
          {
            if (newpos < (unsigned int) (fileheader.toc_ex[x]*scale)) break;
          }
          unsigned int hdr[2]={0,0};
          if (--x >= 0)
          {
            hdr[0]=fileheader.toc[x];
            hdr[1]=(unsigned int) (fileheader.toc_ex[x] * scale);
          }
          //hdr[1] is the time of that keyframe

          if (m_precise_seeking) // precise seek
          {
            int timediff = newpos - hdr[1];
            double fr;
            if (unpacket.isValid() && getFrameRate() >= 0.0001) fr = getFrameRate() / 1000.0;
            else fr = 30.0 / 1000.0;

            seek_dumpframes=(int) (timediff * fr);
            seek_dumpaudiosamples=MulDiv(timediff,pcm_samplerate,1000);
          }
          else
          {
            newpos=hdr[1];
          }

          nbpos = fileheader.header_size + hdr[0];
        }
        else
        {
          double tocpos=(newpos*(double)fileheader.toc_size)/(double)fileheader.file_lenms;
          unsigned int tocidx=(unsigned int)tocpos;
          if (tocidx > fileheader.toc_size)
          {
            nbpos=fileheader.header_size + fileheader.file_lenbytes;
          }
          else
          {
            unsigned int a,b;
            if (tocidx<0) tocidx=0;
            a = fileheader.toc[tocidx];
            if (tocidx < fileheader.toc_size-1)
              b = fileheader.toc[tocidx+1];
            else b=fileheader.file_lenbytes;
            double frac=tocpos-tocidx;
          
            nbpos = fileheader.header_size + (int) (a * (1.0 - frac) + b * frac);
          }
        }
      }
      if (!file_reader->seek(nbpos))
      {
        framecnt=(int)(newpos*getFrameRate()/1000.0);     
        unpacket.reset(0);
        unpacket.setEof(0);
        hdrsearched=1;
        needkf=1;
        m_avresync_time=0;
        vidout=0;
        vidout_ready=0;
        video_frames_avail=0;
        audio_frames_avail=0;
        m_prebuffer=m_pb_init;
        inbs.clear();
        audiobs.clear();
        videobs.clear();
        auxbs.clear();
        if (aud_output) aud_output->flush(newpos);
        if (aud_decoder) aud_decoder->flush();
        if (vid_decoder) vid_decoder->flush();
   	  }
    }
  } // end of seeking

  if (!hdrsearched) // search for header
  {
readagain_header:
    if (quit && *quit) return 0;
    int ret=nsv_readheader(inbs,&fileheader);
    if (ret <= 0) 
    {
      hdrsearched++;
      if (!ret)
      {
		_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();
        char *v=getFromMeta("ASPECT");
        if (v)
		{
		  aspect=_atof_l(v,C_locale);
		}
        free(v);
        v=getFromMeta("FRAMERATE");
        if (v)
        {
          framerate_override=_atof_l(v,C_locale);
          if (framerate_override >= 0.01) use_framerate_override=1;
          free(v);
        }
      }
    }
    else
    {
      char buf[8192] = {0};
      size_t ret2=file_reader->read(buf,sizeof(buf));
      
      if (file_reader->iseof()) unpacket.setEof(1);

      if (file_reader->geterror()) 
      {
        m_err=file_reader->geterror();
        return -1;
      }
      if (ret2>0) 
      {
        inbs.add(buf, (int)ret2);
        goto readagain_header;
      }
    }
  }
  if (!hdrsearched) return 0;
  // end of header search

  // read from source
  if (m_prebuffer || 
    (((!unpacket.isValid() || audio_frames_avail < (int)((getFrameRate() * m_buffer_total)/1000.0)) || !videobs.avail()) && 
        (!m_buf_memlimit || ((int)(audiobs.avail()+videobs.avail()) < m_buf_memlimit*8))))
  {
readagain_stream:
    if (quit && *quit) return 0;
    int lavail= (int)inbs.avail();

    int ret=unpacket.unpacket(inbs);
  
    inbs.compact();

    if (ret)
    {
      if (!unpacket.isValid() && nsvbitstream_search > 8*1024*1024) // only scan for 8mb max
      {
        m_err="Error synching to NSV stream";
        return -1;
      }
      if (unpacket.getEof())
      {
        if (!videobs.avail() && !video_frames_avail && (!aud_output || aud_output->isplaying() <= 0))  retval=-1;
      // do nothing
      }
      else
      {
        char buf[8192] = {0};

        size_t ret2=file_reader->read(buf,sizeof(buf));
      
        if (ret2 == 0 && file_reader->iseof()) 
        {
          m_prebuffer=0;
          unpacket.setEof(1);
        }

        if (file_reader->geterror()) 
        {
          m_err=file_reader->geterror();
          return -1;
        }

        if (ret2 > 0) 
        {
          nsvbitstream_search+= (int)ret2;
          inbs.add(buf, (int)ret2);
          goto readagain_stream;
        }
      }
    }
    else
    {
      nsvbitstream_search=0;
      video_frames_avail++;
      audio_frames_avail++;
      nsv_type_to_string(unpacket.getAudFmt(),m_audio_type);
      nsv_type_to_string(unpacket.getVidFmt(),m_video_type);

      avg_framesize_cnt++;
      avg_framesize_tot+=lavail- (int)((inbs.avail())/8);
      if (avg_framesize_tot > 0x80000000)
      {
        avg_framesize_tot/=2;
        avg_framesize_cnt/=2;
      }

      if (1)
      {
        //parse aux packets
        while (auxbs.avail() >= 8*8)
        {
          size_t thislen = auxbs.getbits(32);
          unsigned int thistype = auxbs.getbits(32);
          if (thislen > auxbs.avail()/8) break;

          if (thistype == NSV_MAKETYPE('S','U','B','T')  ) ProcessSubtitleBlock(auxbs.getcurbyteptr(), (int)thislen);
          else if (thistype == NSV_MAKETYPE('A','S','Y','N'))
          {
            //if (thislen == 0) // resyncpoint
            {
              audiobs.addint(-1);
              audiobs.addint(framecnt+video_frames_avail);
            }
            //else // handle other form [todo]
            //{
            //}
          }

          auxbs.seek(thislen*8);
        }
      }
      auxbs.clear();
    }
  }

  if (unpacket.isValid() && !m_prebuffer && !audiobs.avail() && aud_output)
  {
    if (aud_output->isplaying()) aud_output->write(0,0);
    else  if (!unpacket.getEof())
    {
      m_prebuffer=m_pb_init_ur+(int)((video_frames_avail/getFrameRate())*1000.0);
      if (m_prebuffer < m_pb_init_ur) m_prebuffer=m_pb_init_ur;
      if (m_prebuffer > m_pb_init_ur*3) m_prebuffer=m_pb_init_ur*3;
      retval=2;
    }
  }

  if (video_frames_avail > (int)((m_prebuffer*getFrameRate())/1000.0)) 
  {
    m_bufstate=256;
    m_prebuffer=0;
  }
  else
  {
    int a = (int) (video_frames_avail * 256.0 / ((m_prebuffer*getFrameRate())/1000.0));
    if (a > 255) a=255;
    if (a < 0) a=0;
    m_bufstate=a;
    if (m_out) m_out->notifyBufferState(a);
  }

  if (unpacket.isValid() && !vidout_ready && videobs.avail() && !m_prebuffer) // decode frame
  {
    if (!vid_decoder)
    {
      vid_flip=0;
      vid_decoder = CreateVideoDecoder(unpacket.getWidth(),unpacket.getHeight(),
        getFrameRate(),unpacket.getVidFmt(),&vid_flip,&vid_decoder_isnotnull);
    }

    int kf=0;
    int l=videobs.getbits(32);
		unsigned int local_vidout_type[3] = { 1, 0, 0 };
    int ret=vid_decoder->decode(needkf,videobs.getcurbyteptr(),l,&vidout,local_vidout_type,&kf);
		vidout_type = local_vidout_type[0];
		if (ret == 0)
		{
			
			vidout_codec_width = local_vidout_type[1];
			vidout_codec_height = local_vidout_type[2];
		}

    if (kf) needkf=0;
    video_frames_avail--;
    videobs.seek(l*8);
    videobs.compact();

    avg_framesize_cnt_v++;
    avg_framesize_tot_v+=l;

    if (needkf || seek_dumpframes > 0)
    {
      vidout=NULL;
      if (seek_dumpframes>0) seek_dumpframes--;
    }
    else
    {
      if (!m_out_opened && vid_decoder_isnotnull) 
      {
				m_out->extended(0x1008/*VIDUSER_SET_THREAD_SAFE*/, 1, 0);
				unsigned int vidwidth = vidout_codec_width, vidheight = vidout_codec_height;
				if (vidwidth && vidheight)
				{
					/* if the codec provided a width/height to use */
					double aspect_adjust = (double)vidwidth / (double)vidheight / ((double)unpacket.getWidth() / (double)unpacket.getHeight());
					if (m_out->open(vidwidth,vidheight,vid_flip,aspect * aspect_adjust,vidout_type))
					{
						m_err="Error opening video output";
	          return -1;
					}
				}
				else
				{
					if (m_out->open(unpacket.getWidth(),unpacket.getHeight(),vid_flip,aspect,vidout_type))
					{
						m_err="Error opening video output";
	          return -1;
					}
				}
        m_out_opened=1;
      }

      vidout_ready=1;
      vidout_time = (unsigned int)(framecnt++ / getFrameRate() * 1000.0) + (unsigned int)m_avresync_time;
      int offs=unpacket.getSyncOffset() + m_out->get_latency();

      if ((int)vidout_time <= offs) vidout_time=0;
      else vidout_time-=offs;

      //drop the image if we're late
      if(aud_output && vidout_time<aud_output->getpos()) {
        vidout=0;
        vidout_ready=0;
      }
    }

    retval=1;
  }

  if ( ( audiobs.avail() && !m_prebuffer ) || m_again )
  {
    if (needkf)
    {
      int l=audiobs.getbits(32);
      if (l == -1) audiobs.seek(32); // skip over 32 bits of our audiobs info
      else
      {
        audiobs.seek(l*8);
        audio_frames_avail--;
      }      

      audiobs.compact();
    }
    else // decode some audio
    {
      char pcmbuf[16384] = {0};
      int outbufl=512;
      if (!aud_decoder)
      {
        aud_decoder=CreateAudioDecoder(unpacket.getAudFmt(),&aud_decoder_isnotnull,&aud_output);
        if (aud_output && m_paused) aud_output->pause(1);
      }
      
      if (aud_output) outbufl=aud_output->canwrite();
      
      if (outbufl)
      {
        unsigned int outfmt[8] = {0};

        retval=1;
        if (outbufl > sizeof(pcmbuf)) outbufl=sizeof(pcmbuf);

        int inl=audiobs.getbits(32);

        if (inl == -1)
        {
          int vidframe=audiobs.getbits(32);
          if (aud_output)
          {
            ULONGLONG audpos = aud_output->getwritepos();
            int vidpos = (int)(vidframe / getFrameRate() * 1000.0);

            m_avresync_time = audpos-vidpos;
          }
        }
        else
        {
          int ret=aud_decoder->decode(audiobs.getcurbyteptr(), inl, pcmbuf, &outbufl, outfmt);

          if ( !outbufl ) m_again = 0;
          else m_again =1;

          if (ret < 1)
          {
            avg_framesize_cnt_a++;
            avg_framesize_tot_a+=inl;
            audiobs.seek(inl*8);
            audiobs.compact();
            audio_frames_avail--;
          }
          else audiobs.seek(-32);

          if (outbufl || outfmt[0])
          {
            if (!aud_output)
            {
              aud_output=PCMOUT_CREATE(outfmt);
              pcm_samplerate=outfmt[1];
              if (!aud_output) aud_output = new NullAudioOutput;
              aud_output->setvolume(m_volume);
              aud_output->setpan(m_pan);
              if (aud_output && m_paused) aud_output->pause(1);
            }
            if (outbufl) 
            {
              if (seek_dumpaudiosamples)
              {
                int nch=outfmt[2];
                int bps=outfmt[3];
                int dump=seek_dumpaudiosamples*nch*(bps/8);
                if (dump >= outbufl)
                {
                  seek_dumpaudiosamples -= outbufl/nch/(bps/8);
                }
                else // partial write
                {                  
                  aud_output->write(pcmbuf+dump,outbufl-dump);
                  seek_dumpaudiosamples=0;
                }
              }
              else aud_output->write(pcmbuf,outbufl);
            }
          }
        }
      } // if can write
    } //end of decode
  } // audiobs avail

  if (m_prebuffer) 
#ifdef _WIN32
		Sleep(10);
#else
		usleep(10000);
#endif
  
  return retval;
}

Subtitles *NSVDecoder::insertSubtitlesItem(const char *language, const char *subfile) {
  Subtitles *sub=new Subtitles(subfile);
  m_subtitles.put(new SubtitlesItem(language, sub));
  return sub;
}

Subtitles *NSVDecoder::findSubtitles(const char *language) {
  for(int i=0;i<m_subtitles.getlen();i++) {
    SubtitlesItem *s=m_subtitles.get(i);
    if(!_stricmp(language,s->m_language) ) return s->m_subs;
  }
  return NULL;
}

const char *NSVDecoder::getSubLanguage(int index) {
  if(index>=m_subtitles.getlen()) return NULL;
  return m_subtitles.get(index)->m_language;
}