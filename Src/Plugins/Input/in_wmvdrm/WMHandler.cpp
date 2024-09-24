#include "main.h"
#include <assert.h>

void WMHandler::OpenFailed()
{
	if (next)
		next->OpenFailed();
}

void WMHandler::ReOpen()
{
	if (next)
		next->ReOpen();
}

void WMHandler::Started()
{
	if (next)
		next->Started();
}

void WMHandler::Stopped()
{
	if (next)
		next->Stopped();
}

void WMHandler::PreRollComplete()
{
	if (next)
		next->PreRollComplete();
}

void WMHandler::EndOfFile()
{
	if (next)
		next->EndOfFile();
}

void WMHandler::Closed()
{
	if (next)
		next->Closed();
}

void WMHandler::BufferingStarted()
{
	if (next)
		next->BufferingStarted();
}

void WMHandler::BufferingStopped()
{
	if (next)
		next->BufferingStopped();
}

void WMHandler::NewMetadata()
{
	if (next)
		next->NewMetadata();
}

void WMHandler::Individualize()
{
	if (next)
		next->Individualize();
}

void WMHandler::SignatureState(WMT_DRMLA_TRUST *&state)
{
	if (next)
		next->SignatureState(state);
}

void WMHandler::NoRightsEx(WM_GET_LICENSE_DATA *&licenseData)
{
	if (next)
		next->NoRightsEx(licenseData);
}

void WMHandler::AcquireLicense(WM_GET_LICENSE_DATA *&licenseData)
{
	if (next)
		next->AcquireLicense(licenseData);
}

void WMHandler::AllocateOutput(long outputNum, long bufferSize, INSSBuffer *&buffer)
{
	if (next)
		next->AllocateOutput(outputNum, bufferSize, buffer);
}

void WMHandler::VideoCatchup(QWORD time)
{
	if (next)
		next->VideoCatchup(time);
}

void WMHandler::TimeToSync(QWORD timeStamp,__int64 &diff)
{
	if (next)
		next->TimeToSync(timeStamp, diff);
}

void WMHandler::Error()
{
	if (next)
		next->Error();
  }

void WMHandler::LicenseRequired()
{
	if (next)
		next->LicenseRequired();
}

void WMHandler::NoRights(wchar_t *licenseData)
{
	if (next)
		next->NoRights(licenseData);
}


WMHandler::WMHandler() : next(0), prev(0)
	{}
		WMHandler::~WMHandler()
	{
		if (next)
			next->prev = prev;

		if (prev)
			prev->next = next;

	}
	
	WMHandler &WMHandler::operator << (WMHandler &chain)
	{
		assert(chain.next == 0);
		assert(prev == 0);

		prev = &chain;
		chain.next = this;

		return chain;
	}

	WMHandler &WMHandler::operator >> (WMHandler &chain)
	{
		if (chain.prev)
		{
			operator >>(chain.prev);
			return chain;
		}

		assert (next == 0);
		assert (chain.prev == 0);

		next = &chain;
		chain.prev = this;

		return chain;
	}
	WMHandler&WMHandler::operator << (WMHandler *chain)
	{
		return operator <<(*chain);
	}

	WMHandler &WMHandler::operator >> (WMHandler *chain)
	{
		return operator >>(*chain);
	}

	WMHandler &WMHandler::First()
	{
		if (prev)
			return prev->First();
		else
			return  *this;
	}

