#include "OutPlugin.h"
#include "../Winamp/In2.h"

#include "WMDRMModule.h"
extern In_Module plugin;
OutPlugin pluginOut;

OutPlugin::OutPlugin()
{}

void OutPlugin::Init()
{
	plugin.outMod->Init();
}
void OutPlugin::Quit()
{
	plugin.outMod->Quit();
}
int OutPlugin::CanWrite()
{
	return plugin.outMod->CanWrite();
}
int OutPlugin::GetWrittenTime()
{
	return plugin.outMod->GetWrittenTime();
}
int OutPlugin::IsPlaying()
{
	return plugin.outMod->IsPlaying();
}
int OutPlugin::Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{
	return plugin.outMod->Open(samplerate, numchannels, bitspersamp, bufferlenms, prebufferms);
}
void OutPlugin::Close()
{
	plugin.outMod->Close();
}
int OutPlugin::Write(char *buf, int len)
{
	return plugin.outMod->Write(buf, len);
}
void OutPlugin::Flush(int t)
{
	plugin.outMod->Flush(t);
}
void OutPlugin::SetVolume(int _volume)
{
	plugin.outMod->SetVolume(_volume);
}
int OutPlugin::Pause(int new_state)
{
	return plugin.outMod->Pause(new_state);
}
int OutPlugin::GetOutputTime()
{
	return plugin.outMod->GetOutputTime();
}
void OutPlugin::SetPan(int _pan)
{
	plugin.outMod->SetPan(_pan);
}
void OutPlugin::About(HWND p)
{
	plugin.outMod->About(p);
}
void OutPlugin::Config(HWND w)
{
	plugin.outMod->Config(w);
}
