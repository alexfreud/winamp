#ifndef NULLSOFT_LINEINH
#define NULLSOFT_LINEINH

class LineIn
{
public:
	LineIn() : posinms(0), paused(false)
	{}
	int Play();
	void Stop();
	void Pause();
	void Unpause();
	int GetLength();
	int GetOutputTime();
	bool IsPaused() { return paused; }
	void SetOutputTime(int time_in_ms)
{}
	void SetVolume(int a_v, int a_p)
	{}
private:
	int posinms;
	bool paused;
};

#endif