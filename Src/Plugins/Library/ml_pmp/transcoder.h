#ifndef _TRANSCODER_H_
#define _TRANSCODER_H_

class Transcoder {
protected:
  Transcoder(){}
  virtual ~Transcoder(){}
public:  
  // done at init()
  virtual void LoadConfigProfile(wchar_t *profile)=0; // deprecated, do not call
  virtual void AddAcceptableFormat(wchar_t *format)=0; // eg, L"mp3" or L"wma"
  virtual void AddAcceptableFormat(unsigned int format)=0; // eg, mmioFOURCC('M','4','A',' ')

  // done when file is added to transfer queue
  // returns:
  //  -1 for can't transcode
  //  output file size estimate if can transcode
  // if ext is supplied, it should be a buffer with space for 5 characters, and will be filled with 
  //   the output file type file extention, eg, L".mp3"
  virtual int CanTranscode(wchar_t *file, wchar_t *ext = NULL, int length = -1)=0; 

  // false if no transcoding needed
  virtual bool ShouldTranscode(wchar_t *file)=0;

  // done just before transfer OR in background after file is added to queue
  // extention is added to outputFile, allow 5 extra chars
  // callback, callbackcontext and killswitch should be similar to those passed by ml_pmp
  // return 0 for success, -1 for failed or cancelled
  virtual int TranscodeFile(wchar_t *inputFile, wchar_t *outputFile, int *killswitch,  void (*callback)(void * callbackContext, wchar_t * status), void* callbackContext, wchar_t * caption=L"Transcoding %d%%")=0;


  // get a filename which can be used as a staging area.
  // ext should be for example L".mp3"
  // make sure filename is a buffer at least MAX_PATH characters long.
  virtual void GetTempFilePath(const wchar_t *ext, wchar_t *filename)=0; 
};

#endif //_TRANSCODER_H_