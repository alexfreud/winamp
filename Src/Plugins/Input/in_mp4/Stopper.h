#pragma once
class Stopper
{
public:
	Stopper();
	void ChangeTracking(bool);
	void Stop();
	void Play();
	int isplaying, timems;
};

