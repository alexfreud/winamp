#ifndef NULLSOFT_AUDIOLAYERH
#define NULLSOFT_AUDIOLAYERH

#include "WMHandler.h"
#include <mmreg.h>
#include "AudioThread.h"
#include "AudioFormat.h"

class AudioLayer : public WMHandler, public AudioFormat
{
public:
	AudioLayer(IWMReader *_reader);
	~AudioLayer();
bool IsOpen()
{
	return opened;
}
	void Kill();
	bool OpenAudio();

	void StartAudioThread();
private:
	// WMHandler events

	void Opened();
	void SampleReceived(QWORD &timeStamp, QWORD &duration, unsigned long &outputNum, unsigned long &flags, INSSBuffer *&sample);
	void VideoCatchup(QWORD time);
	void Closed();
	void EndOfFile();	
	void Started();
	void Stopped();
	
	// other people's data
	IWMReader *reader;
	
	// our data
	QWORD startPosition;
	
	int audioOutputNum;
	IWMReaderAdvanced2 *reader2;
	QWORD offset;
	DWORD new_offset;
	QWORD videoCatchup;
	bool opened;
	HANDLE killSwitch;
	int latency;

	AudioThread audioThread;
};

#endif
