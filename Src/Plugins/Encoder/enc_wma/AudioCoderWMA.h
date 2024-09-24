#ifndef AUDIOCODERWMA_H
#define AUDIOCODERWMA_H

#include "../nsv/enc_if.h"
#include "main.h"

class CustomWMWriterSink;

class AudioCoderWMA : public AudioCoder
{
public:
  AudioCoderWMA(int nch, int srate, int bps, configtype *cfg, char *configfile);
  virtual int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail); //returns bytes in out
  virtual ~AudioCoderWMA();
  int GetLastError();
  void PrepareToFinish();
  void OnFinished(const wchar_t *filename);
  HRESULT SelectAndLoadResampler(int numchannels, int samplerate, int bitpersamp);

  HRESULT CreateAndConfigureWriter(WORD nch, WORD srate, WORD bps, char *configfile);
  HRESULT CreateAndConfigureProfile(WAVEFORMATEX* pWaveLimits, IWMProfile** ppProfile, char *configfile);
  
private:
  bool begin_writing;
  int error;
  IWMWriterFileSink *sink;
  IWMWriter *writer;
	IWMWriterAdvanced *writerAdvanced;
  double timeunits_per_byte; // "100 nanosecond units" -- ie: ( ( (10000000.0) / (double)samplerate ) / (double)numchannels ) / ( (double)bitspersamp/ 8.0 )
  int input_bytecount;
	QWORD lastByteCount;
	wchar_t tempFilename[MAX_PATH];
  
};

enum AudioCoderWMAErrors
{
  WMA_NO_ERROR = 0,
  WMA_CANT_FIND_WMSDK = -1,
  WMA_CANT_LOAD_CREATOR = -2,
  WMA_CANT_CREATE_WRITER = -3,
  WMA_CANT_SET_INPUT_FORMAT = -4,
  WMA_CANT_SET_OUTPUT_FORMAT = -5,
  WMA_CANT_MAKE_CUSTOM_SINK = -6,
  WMA_CANT_QUERY_WRITER_INTERFACE = -7,
  WMA_CANT_QUERY_SINK_INTERFACE = -8,
  WMA_CANT_ADD_SINK = -9,
};

#endif//AUDIOCODERWMA_H
