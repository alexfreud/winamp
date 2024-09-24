#ifndef NSVPLAY_SUBTITLES_H
#define NSVPLAY_SUBTITLES_H

#include "wa_ipc.h"
//#include "main.h"
//#include "../nsvbs.h"

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

class SubsItem {
public:
  SubsItem(unsigned int ptimestart, unsigned int ptimeend, const char *ptext) :
      timestart(ptimestart) , timeend(ptimeend) { 
        text=_strdup(ptext);
        xPos=128;
        yPos=255;
        colorRed=colorGreen=colorBlue=0xff;
        extraDataSize=0;
        extraData=0;
        muxed_subtitle=0;
        fontSize=origFontSize=0;
      }
  ~SubsItem() { 
    free((void*)text); 
    if(extraDataSize) free((void *)extraData);
  }

  unsigned int timestart;
  unsigned int timeend; 
  const char *text;

  unsigned char xPos, yPos;
  unsigned char colorRed, colorGreen, colorBlue;
  int extraDataSize;
  const void *extraData;

  int muxed_subtitle; //so we free it when we seek/display

  int fontSize;

  int origFontSize;
};

class Subtitles {
public:
  Subtitles(const char *filename);

  SubsItem *getSubtitle(unsigned int time, unsigned int frame); // time in ms
  void addSubtitlePacket(SUBTITLE_INFO *sti);

  void setFontSizeModifier(int size) { m_font_size_mod=size; }
  
private:
  void decodeSrtFile(char *text);
  unsigned int getTimeFromSrtText(const char *text);

  void decodeSubFile(char *text);

  //ClassList<SubsItem> m_subs; //FUCKO
  int m_frame_based;
  int m_last_sub;
  int m_font_size_mod;
};

#endif