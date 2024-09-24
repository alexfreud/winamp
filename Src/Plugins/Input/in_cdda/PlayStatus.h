#ifndef NULLSOFT_PLAYSTATUSH
#define NULLSOFT_PLAYSTATUSH
#pragma warning(disable:4786)
//#include <map>
#include "../nu/AutoLock.h"

class DriveStatus
{
public:
	DriveStatus(); 

	void RippingStarted(), RippingStopped();

	bool IsRipping() const;
private:
	bool ripping;
};

class PlayStatus
{
public:
	DriveStatus &operator [](int index) { return driveStatus[index-'A']; }
	DriveStatus driveStatus[26];
};
extern PlayStatus playStatus;
//extern std::map<char, DriveStatus> playStatus;
extern Nullsoft::Utility::LockGuard *playStatusGuard;
#endif