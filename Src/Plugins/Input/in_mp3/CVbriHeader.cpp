//---------------------------------------------------------------------------\ 
//
//               (C) copyright Fraunhofer - IIS (2000)
//                        All Rights Reserved
//
//   filename: CVbriHeader.cpp
//             MPEG Layer-3 Audio Decoder
//   author  : Martin Weishart martin.weishart@iis.fhg.de
//   date    : 2000-02-11
//   contents/description: provides functions to read a VBRI header
//                         of a MPEG Layer 3 bitstream encoded 
//                         with variable bitrate using Fraunhofer 
//                         variable bitrate format 
//
//--------------------------------------------------------------------------/

#include <windows.h>

#include "CVbriHeader.h"
#include "LAMEInfo.h"
#include <malloc.h>

//---------------------------------------------------------------------------\ 
//
//   Constructor: set position in buffer to parse and create a 
//                VbriHeaderTable
//
//---------------------------------------------------------------------------/

CVbriHeader::CVbriHeader(){
  position = 0;
  VbriTable=0;
  VbriStreamFrames=0;
  encoderDelay=0;
  h_id=0;
  SampleRate=0;
  VbriTableSize=0;
  VbriEntryFrames=0;
  VbriStreamBytes=0;
}



//---------------------------------------------------------------------------\ 
//
//   Destructor: delete a VbriHeaderTable and a VbriHeader
//
//---------------------------------------------------------------------------/

CVbriHeader::~CVbriHeader(){
  free(VbriTable);
}



//---------------------------------------------------------------------------\  
//
//   Method:   checkheader
//             Reads the header to a struct that has to be stored and is 
//             used in other functions to determine file offsets
//   Input:    buffer containing the first frame
//   Output:   fills struct VbriHeader
//   Return:   0 on success; 1 on error
//
//---------------------------------------------------------------------------/

int CVbriHeader::readVbriHeader(unsigned char *Hbuffer)
{
	position=0;
  // MPEG header 
	MPEGFrame frame;
	frame.ReadBuffer(Hbuffer);
	if (!frame.IsSync())
		return 0;

	SampleRate = frame.GetSampleRate();
  h_id = frame.mpegVersion & 1;

  position += DWORD ;

  // data indicating silence
  position += (8*DWORD) ;
  
  // if a VBRI Header exists read it

  if ( *(Hbuffer+position  ) == 'V' &&
       *(Hbuffer+position+1) == 'B' &&
       *(Hbuffer+position+2) == 'R' &&
       *(Hbuffer+position+3) == 'I'){
    
    position += DWORD;
		
		//position += WORD;
    /*unsigned int vbriVersion = */readFromBuffer(Hbuffer, WORD);    // version

    encoderDelay = readFromBuffer(Hbuffer, WORD);     // delay

		position += WORD;
    //readFromBuffer(Hbuffer, WORD);     // quality

    VbriStreamBytes  = readFromBuffer(Hbuffer, DWORD);    
    VbriStreamFrames = readFromBuffer(Hbuffer, DWORD);    
    VbriTableSize    = readFromBuffer(Hbuffer, WORD);    
    unsigned int VbriTableScale   = readFromBuffer(Hbuffer, WORD);    
    unsigned int VbriEntryBytes   = readFromBuffer(Hbuffer, WORD);    
    VbriEntryFrames  = readFromBuffer(Hbuffer, WORD);
    
    if (VbriTableSize > 32768) return 1;
    
    VbriTable = (int *)calloc((VbriTableSize + 1), sizeof(int));

    for (unsigned int i = 0 ; i <= VbriTableSize ; i++){
      VbriTable[i] = readFromBuffer(Hbuffer, VbriEntryBytes*BYTE) 
        * VbriTableScale ;
    }
  }
  else
  {
    return 0;
  }
  return frame.FrameSize();
}



//---------------------------------------------------------------------------\ 
//
//   Method:   seekPointByTime
//             Returns a point in the file to decode in bytes that is nearest 
//             to a given time in seconds
//   Input:    time in seconds
//   Output:   None
//   Returns:  point belonging to the given time value in bytes
//
//---------------------------------------------------------------------------/

int CVbriHeader::seekPointByTime(float EntryTimeInMilliSeconds){

  unsigned int SamplesPerFrame, i=0, SeekPoint = 0 , fraction = 0;

  float TotalDuration ;
  float DurationPerVbriFrames ;
  float AccumulatedTime = 0.0f ;
 
  (SampleRate >= 32000) ? (SamplesPerFrame = 1152) : (SamplesPerFrame = 576) ;

  TotalDuration		= ((float)VbriStreamFrames * (float)SamplesPerFrame) 
						  / (float)SampleRate * 1000.0f ;
  DurationPerVbriFrames = (float)TotalDuration / (float)(VbriTableSize+1) ;
 
  if ( EntryTimeInMilliSeconds > TotalDuration ) EntryTimeInMilliSeconds = TotalDuration; 
 
  while ( AccumulatedTime <= EntryTimeInMilliSeconds ){
    
    SeekPoint	      += VbriTable[i] ;
    AccumulatedTime += DurationPerVbriFrames;
    i++;
    
  }
  
  // Searched too far; correct result
  fraction = ( (int)(((( AccumulatedTime - EntryTimeInMilliSeconds ) / DurationPerVbriFrames ) 
			 + (1.0f/(2.0f*(float)VbriEntryFrames))) * (float)VbriEntryFrames));

  
  SeekPoint -= (int)((float)VbriTable[i-1] * (float)(fraction) 
				 / (float)VbriEntryFrames) ;

  return SeekPoint ;

}

int CVbriHeader::getNumMS()
  { 
    if (!VbriStreamFrames || !SampleRate) return 0;

    int nf=VbriStreamFrames;
    int sr=SampleRate;
    if (sr >= 32000) sr/=2;
    //576
    return MulDiv(nf,576*1000,sr);       
  }

#if 0
//---------------------------------------------------------------------------\ 
//
//   Method:   seekTimeByPoint
//             Returns a time in the file to decode in seconds that is 
//             nearest to a given point in bytes
//   Input:    time in seconds
//   Output:   None
//   Returns:  point belonging to the given time value in bytes
//
//---------------------------------------------------------------------------/

float CVbriHeader::seekTimeByPoint(unsigned int EntryPointInBytes){

  unsigned int SamplesPerFrame, i=0, AccumulatedBytes = 0, fraction = 0;

  float SeekTime = 0.0f;
  float TotalDuration ;
  float DurationPerVbriFrames ;

  (SampleRate >= 32000) ? (SamplesPerFrame = 1152) : (SamplesPerFrame = 576) ;

  TotalDuration		= ((float)VbriStreamFrames * (float)SamplesPerFrame) 
						  / (float)SampleRate;
  DurationPerVbriFrames = (float)TotalDuration / (float)(VbriTableSize+1) ;
 
  while (AccumulatedBytes <= EntryPointInBytes){
    
    AccumulatedBytes += VbriTable[i] ;
    SeekTime	       += DurationPerVbriFrames;
    i++;
    
  }
  
  // Searched too far; correct result
  fraction = (int)(((( AccumulatedBytes - EntryPointInBytes ) /  (float)VbriTable[i-1]) 
                    + (1/(2*(float)VbriEntryFrames))) * (float)VbriEntryFrames);
  
  SeekTime -= (DurationPerVbriFrames * (float) ((float)(fraction) / (float)VbriEntryFrames)) ;
 
  return SeekTime ;

}



//---------------------------------------------------------------------------\ 
//
//   Method:   seekPointByPercent
//             Returns a point in the file to decode in bytes that is 
//             nearest to a given percentage of the time of the stream
//   Input:    percent of time
//   Output:   None
//   Returns:  point belonging to the given time percentage value in bytes
//
//---------------------------------------------------------------------------/

int CVbriHeader::seekPointByPercent(float percent){

  int SamplesPerFrame;

  float TotalDuration ;
  
  if (percent >= 100.0f) percent = 100.0f;
  if (percent <= 0.0f)   percent = 0.0f;

  (SampleRate >= 32000) ? (SamplesPerFrame = 1152) : (SamplesPerFrame = 576) ;

  TotalDuration = ((float)VbriStreamFrames * (float)SamplesPerFrame) 
				  / (float)SampleRate;
  
  return seekPointByTime( (percent/100.0f) * TotalDuration * 1000.0f );
  
}

#endif


//---------------------------------------------------------------------------\ 
//
//   Method:   GetSampleRate
//             Returns the sampling rate of the file to decode
//   Input:    Buffer containing the part of the first frame after the
//             syncword
//   Output:   None
//   Return:   sampling rate of the file to decode
//
//---------------------------------------------------------------------------/

/*int CVbriHeader::getSampleRate(unsigned char * buffer){
  
  unsigned char id, idx, mpeg ;
  
  id  = (0xC0 & (buffer[1] << 3)) >> 4;
  idx = (0xC0 & (buffer[2] << 4)) >> 6;
 
  mpeg = id | idx;
  
  switch ((int)mpeg){
    
  case 0 : return 11025;
  case 1 : return 12000;
  case 2 : return 8000;
  case 8 : return 22050;
  case 9 : return 24000;
  case 10: return 16000;
  case 12: return 44100;
  case 13: return 48000;
  case 14: return 32000;
  default: return 0;

  }
}*/



//---------------------------------------------------------------------------\ 
//
//   Method:   readFromBuffer
//             reads from a buffer a segment to an int value
//   Input:    Buffer containig the first frame
//   Output:   none
//   Return:   number containing int value of buffer segmenet
//             length
//
//---------------------------------------------------------------------------/

int CVbriHeader::readFromBuffer ( unsigned char * HBuffer, int length ){

  if (HBuffer)
  {
    int number = 0;   
    for(int i = 0;  i < length ; i++ )
   { 
      int b = length-1-i  ;                                                            
      number = number | (unsigned int)( HBuffer[position+i] & 0xff ) << ( 8*b );
    }
    position += length ;
    return number;
  }
  else{
    return 0;
  }
}