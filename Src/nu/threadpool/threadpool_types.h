#pragma once
#include <deque>
#include <windows.h>
namespace ThreadPoolTypes
{
	typedef std::deque<HANDLE> HandleList;
	const int MAX_SEMAPHORE_VALUE = 1024; //some arbitrarily high amount*
}