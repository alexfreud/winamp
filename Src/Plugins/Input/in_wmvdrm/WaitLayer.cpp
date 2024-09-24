#include "main.h"
#include "WaitLayer.h"
#include "util.h"

WaitLayer::WaitLayer(IWMReader *_reader)
: reader(_reader), stopEvent(0)
{
  reader->AddRef();
  openEvent = CreateEvent(0, TRUE, FALSE, 0);
}

WaitLayer::~WaitLayer()
{
  reader->Release();
  reader=0;
}

void WaitLayer::Opened()
{
  SetEvent(openEvent);
  WMHandler::Opened();
}

bool WaitLayer::IsOpen()
{
  return WaitForSingleObject(openEvent, 0) == WAIT_OBJECT_0;
}

void WaitLayer::OpenCalled()
{ 
  SetEvent(openEvent);
  WMHandler::OpenCalled();
}

void WaitLayer::OpenFailed()
{ 
  SetEvent(openEvent);
  WMHandler::OpenFailed();
}

bool WaitLayer::WaitForOpen(int time_ms)
{
  return WaitForSingleObject(openEvent, time_ms) == WAIT_OBJECT_0;
}

void WaitLayer::ResetForOpen()
{
  ResetEvent(openEvent);
}