#include "main.h"
#include "vid_subs.h"

Subtitles::Subtitles(const char *filename)
{
  m_frame_based=0;
  m_last_sub=-1;
  m_font_size_mod=0;

#ifdef SUBTITLES_READER
  if(!filename) return;

  IDataReader *file_reader=CreateReader((char *)filename);
  if(file_reader) {
    char *text=NULL;
    int textpos=0;
    int allocsize=0;
    char buf[1024] = {0};
    unsigned aborttime=GetTickCount()+20000;
    for (;;)
    {
      int l=file_reader->read(buf,1024);
      if (l <= 0) 
      {
        if (file_reader->iseof()) break;
        if (file_reader->geterror() || GetTickCount() > aborttime) 
        {
          free(text);
          return;
        }
        Sleep(100);
      }
      else
      {
        if (textpos+l+1 >= allocsize)
        {
          allocsize = textpos+l+1+8192;
          text=(char*)realloc(text,allocsize);
        }
        memcpy(text+textpos,buf,l);
        textpos+=l;
      }
    }
    if (text) {
      text[textpos]=0;
      //fucko: check for case
      if(strstr(filename,".srt")) decodeSrtFile(text);
      else if(strstr(filename,".sub")) decodeSubFile(text);
      free(text);
    }
  }
  delete(file_reader);
#endif
}

#ifdef SUBTITLES_READER
void Subtitles::decodeSrtFile(char *text) {
  // parse subtitle file (.srt format)
  char *p=text;

  //for(int i=0;;i++) {
  while(1) {
    unsigned int time_start,time_end;

    // parse title nb
    char *p2=p;
    while(p2 && *p2 && *p2!='\n') p2++;
    *p2++=0;
    //if(atoi(p)!=i+1) break;
    if(atoi(p)<=0) break;
      
    // parse start time
    p=p2;
    while(p2 && *p2 && *p2!=' ') p2++;
    *p2++=0;
    time_start=getTimeFromSrtText(p);

    // parse "-->"
    while(p2 && *p2 && *p2!=' ') p2++;
    p2++;
    
    // parse end time
    p=p2;
    while(p2 && *p2 && *p2!='\n') p2++;
    *p2++=0;
    time_end=getTimeFromSrtText(p);

    // parse text
    p=p2;
    while(p2 && *p2 && !(*p2=='\r' || *p=='\n')) {
      while(p2 && *p2 && *p2!='\n') p2++;
      p2++;
    }
    *p2++=0;

    //remove trailing CR
    {
      int l=lstrlen(p);
      if(l) {
        if(p[l-1]=='\r' || p[l-1]=='\n') p[l-1]=0;
      }
    }

    m_subs.put(new SubsItem(time_start,time_end,p));

    if(*p2=='\n') p2++;
    p=p2;
  }
  m_frame_based=0;
}

unsigned int Subtitles::getTimeFromSrtText(const char *text) {
  int hours,mins,secs,mills;
  const char *p=text;
  hours=atoi(p);
  while(p && *p && *p!=':') p++;
  if (p) p++;
  mins=atoi(p);
  while(p && *p && *p!=':') p++;
  if (p) p++;
  secs=atoi(p);
  while(p && *p && *p!=',') p++;
  if (p) p++;
  mills=atoi(p);
  return mills+(secs*1000)+(mins*60000)+(hours*60*60000);
}

void Subtitles::decodeSubFile(char *text)
{
  char *p=text;
  while(p && *p && *p=='{') {
    int framestart,frameend;
    p++;
    char *p2=p;
    while(p2 && *p2 && *p2!='}') p2++;
    if (p2) *p2++=0;
    framestart=atoi(p);

    p2+=1;
    p=p2;
    while(p2 && *p2 && *p2!='}') p2++;
    if (p2) *p2++=0;
    frameend=atoi(p);

    p=p2;
    while(p2 && *p2 && *p2!='\r' && *p2!='\n') {
      //replace pipes with CR
      if(*p2=='|') *p2='\n';
      p2++;
    }
    *p2++=0;
    
    m_subs.put(new SubsItem(framestart,frameend,p));

    if(*p2=='\n') p2++;
    p=p2;
  }
  m_frame_based=1;
}
#endif

SubsItem *Subtitles::getSubtitle(unsigned int time, unsigned int frame)
{
  //FUCKO
#if 0
  unsigned int ref=m_frame_based?frame:time;

  //check with lastsub
  if(m_last_sub!=-1) {
    SubsItem *item=m_subs.get(m_last_sub);
    if(ref>=item->timestart && ref<=item->timeend) 
    {
      item->fontSize=item->origFontSize+m_font_size_mod;
      return item;
    }
    SubsItem *item2=m_subs.get(m_last_sub+1);
    if(item2) {
      if(ref>=item->timeend && ref<=item2->timestart) return NULL;
      if(ref>=item2->timestart && ref<=item2->timeend) {
        m_last_sub++;
        item2->fontSize=item2->origFontSize+m_font_size_mod;
        return item2;
      }
    }
  } 

  int l=m_subs.getlen();
  for(int i=0;i<l;i++) {
    SubsItem *item=m_subs.get(i);
    if(ref<item->timestart) break;
    if(ref>=item->timestart && ref<=item->timeend) {
      m_last_sub=i;
      item->fontSize=item->origFontSize+m_font_size_mod;
      return item;
    }
  }
  m_last_sub=-1;
#endif
  return NULL;
}

void Subtitles::addSubtitlePacket(SUBTITLE_INFO *sti) 
{
  //FUCKO
#if 0
  m_frame_based=1; //FUCKO: put this in subsitem struct
  SubsItem *i=new SubsItem(sti->start_frame,sti->end_frame,sti->utf8_text);
  i->xPos=sti->xPos;
  i->yPos=sti->yPos;
  i->colorBlue=sti->colorBlue;
  i->colorGreen=sti->colorGreen;
  i->colorRed=sti->colorRed;
  i->extraDataSize=sti->extraDataSize;
  i->origFontSize=sti->fontSize;
  if(sti->extraDataSize) {
    i->extraData=malloc(sti->extraDataSize);
    memcpy((void *)i->extraData,sti->extraData,sti->extraDataSize);
  }
  i->muxed_subtitle=1;
  m_subs.put(i);
#endif
}