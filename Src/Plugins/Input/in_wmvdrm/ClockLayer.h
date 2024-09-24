#ifndef NULLSOFT_CLOCKLAYERH
#define NULLSOFT_CLOCKLAYERH

#include "WMHandler.h"
class ClockLayer : public WMHandler
{
public:
	ClockLayer(IWMReader *reader);

	void SetStartTimeMilliseconds(long time);
	QWORD GetStartTime();

	void GoRealTime();
	int GetOutputTime();
	void SetLastOutputTime(int _outputTime)
	{
		lastOutputTime = _outputTime;
	}
	void Clock();
private:
	// WMHandler
	void Opened();
	void Started();
	void TimeReached(QWORD &timeReached);
	void TimeToSync(QWORD timeStamp, __int64 &diff);
	void SampleReceived(QWORD &timeStamp, QWORD &duration, unsigned long &outputNum, unsigned long &flags, INSSBuffer *&sample);

	IWMReaderAdvanced *clock;
	
	QWORD startTime, clockTick, curTime;
	DWORD startTimeMilliseconds;
	bool realTime;
	int lastOutputTime;
};

#endif
