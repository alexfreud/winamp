#include "Main.h"
#include "ClockLayer.h"
#include "config.h"

ClockLayer::ClockLayer(IWMReader *reader)
	: clock(0), startTime(0),
	  clockTick(2500000), curTime(0),
	  startTimeMilliseconds(0),
	  realTime(false), 
	  lastOutputTime(0)
{
  if (FAILED(reader->QueryInterface(&clock)))
    clock=0;
}

void ClockLayer::Opened()
{
  realTime=!config_clock;
  WMHandler::Opened();

  if (!realTime)
  {
    HRESULT hr = clock->SetUserProvidedClock(TRUE);

    if (FAILED(hr))
    {
      realTime=false;
    }
  }
  else
    clock->SetUserProvidedClock(FALSE);
  curTime = startTime;
}

void ClockLayer::Started()
{
  if (!realTime && config_clock)
    clock->DeliverTime((QWORD) - 1);

	if (startTimeMilliseconds != 0)
		out->Flush(startTimeMilliseconds);

  SetLastOutputTime(startTimeMilliseconds);
  WMHandler::Started();
}

void ClockLayer::Clock()
{
  if (!realTime && config_clock)
    clock->DeliverTime((QWORD) - 1);
}

void ClockLayer::SetStartTimeMilliseconds(long time)
{
  startTimeMilliseconds=time;
  startTime = time;
  startTime *= 10000;
}

void ClockLayer::TimeReached(QWORD &timeReached)
{
  curTime = timeReached;
}

QWORD ClockLayer::GetStartTime()
{
  return startTime;
}

void ClockLayer::GoRealTime()
{
  realTime = true;
}

int ClockLayer::GetOutputTime()
{
  if (realTime)
    return (int) (curTime / 10000LL);
  else
    return lastOutputTime + (out->GetOutputTime() - out->GetWrittenTime());
}

void ClockLayer::TimeToSync(QWORD timeStamp, __int64 &diff)
{
  if (realTime)
    diff = 0;
  else
  {
    QWORD outputTime = this->GetOutputTime();
    outputTime *= 10000LL;
    diff = timeStamp - outputTime;
  }
}

void ClockLayer::SampleReceived(QWORD &timeStamp, QWORD &duration, unsigned long &outputNum, unsigned long &flags, INSSBuffer *&sample)
{
  if (realTime)
    curTime = timeStamp;

  WMHandler::SampleReceived(timeStamp, duration, outputNum, flags, sample);
}
