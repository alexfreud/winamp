#ifndef _VBRIHEADER_H_
#define _VBRIHEADER_H_

class CVbriHeader{

public: 
  
  CVbriHeader();
  ~CVbriHeader();
  
  int				readVbriHeader(unsigned char *Hbuffer);

  int				seekPointByTime(float EntryTimeInSeconds);
#if 0
  float		  seekTimeByPoint(unsigned int EntryPointInBytes);
  int				seekPointByPercent(float percent);
#endif

  int getNumFrames() { return VbriStreamFrames; }
  int getNumMS();
	int getEncoderDelay() { return encoderDelay; }
	int getBytes() { return VbriStreamBytes; }
int h_id;
private:

  int				getSampleRate(unsigned char * buffer);
  int				readFromBuffer ( unsigned char * HBuffer, int length );

    int				SampleRate;
    unsigned int	        VbriStreamBytes;
    unsigned int	        VbriStreamFrames;
    unsigned int	        VbriTableSize;
    unsigned int	        VbriEntryFrames;
    int		*		VbriTable;
		int encoderDelay;

  int				position ;
	
  enum offset{
    
    BYTE	=		1,
    WORD	=		2,
    DWORD	=		4
    
  };

};

#endif//_VBRIHEADER_H_