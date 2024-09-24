#include "PlayStatus.h"

//std::map<char, DriveStatus> playStatus;
PlayStatus playStatus;
Nullsoft::Utility::LockGuard *playStatusGuard = 0;

DriveStatus::DriveStatus() : ripping(false)
{}

void DriveStatus::RippingStarted()
{
	ripping = true;
}

void DriveStatus::RippingStopped()
{
	ripping = false;
}

bool DriveStatus::IsRipping() const
{
	return ripping;
}
