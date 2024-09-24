/*
Spectrum Analyzer
*/

#include "Main.h"
#include <math.h>
#include "../nu/threadname.h"

static int last_pos;
typedef struct sa_l
{
	int timestamp;
	unsigned char data[2*75];
	char which;
} sa_l;

static int sa_fps = 76;
static sa_l *sa_bufs;
static int sa_position;
static int sa_length, sa_size;
static CRITICAL_SECTION cs;

void sa_init(int numframes)
{
	EnterCriticalSection(&cs);
	sa_length=0;
	if (numframes < 1) numframes = 1;

	if (numframes > sa_size)
	{
		free(sa_bufs);
		sa_bufs = (sa_l *)calloc(numframes, sizeof(sa_l));
		sa_size=numframes;
	}
	sa_position = 0;
	sa_length = numframes;
	last_pos = 0;
	LeaveCriticalSection(&cs);
}

void sa_deinit(void)
{
	EnterCriticalSection(&cs);
	//if (sa_bufs)
//	{
//		free(sa_bufs);
//		sa_bufs = 0;
		sa_length = 0;
	//}
	LeaveCriticalSection(&cs);
}

int sa_add(char *values, int timestamp, int csa)
{
	EnterCriticalSection(&cs);
	if (!sa_bufs || sa_length == 0) 
	{
		LeaveCriticalSection(&cs);
		return 1;
	}

	if (sa_length == 1)
	{
		sa_position = 0;
	}
	if (csa == 3) csa = 1; // dont let it happen unless it has a high bit set
	csa &= 0x7fffffff;

	sa_bufs[sa_position].timestamp = timestamp;
	sa_bufs[sa_position].which = (char)csa;

	if (csa & 1)
	{
		memcpy(sa_bufs[sa_position].data, values, 75);
		values += 75;
	}
	else
		memset(sa_bufs[sa_position].data, 0, 75);

	if (csa & 2)
		memcpy(sa_bufs[sa_position].data + 75, values, 75);
	else
		memset(sa_bufs[sa_position].data + 75, 0, 75);

	sa_position++;
	if (sa_position >= sa_length) sa_position -= sa_length;
	LeaveCriticalSection(&cs);
	return 0;
}

char *sa_get(int timestamp, int csa, char data[75*2+8])
{
	static int sa_pos;
	int closest = 1000000, closest_v = -1;
	EnterCriticalSection(&cs);	

	if (!sa_bufs || sa_length==0)
	{
		LeaveCriticalSection(&cs);
		return 0;
	}

	if (sa_length == 1)
	{
		memcpy(data, sa_bufs[0].data, 75*2);
		LeaveCriticalSection(&cs);
		return (data);
	}

	int i = last_pos;
	for (int x = 0; x < sa_length; x ++)
	{
		if (i >= sa_length) i = 0;
		int d = timestamp - sa_bufs[i].timestamp;
		if (d < 0) d = -d;
		if (d < closest)
		{
			closest = d;
			closest_v = i;
		}
		else if (closest <= 6) break;
		i++;
	}

	if (closest < 400 && closest_v >= 0 && sa_bufs[closest_v].which & csa)
	{
		sa_pos = 0;
		last_pos = closest_v;
		memcpy(data, sa_bufs[closest_v].data, 75*2);
		LeaveCriticalSection(&cs);
		return data;
	}

	if (closest_v < 0 || !(sa_bufs[closest_v].which & csa) || closest > 400)
	{
		memset(data, 0, 75);
		data[(sa_pos % 150) >= 75 ? 149 - (sa_pos % 150) : (sa_pos % 150)] = 15;
		for (int x = 0; x < 75; x ++)
			data[x + 75] = (char) (int) (7.0 * sin((sa_pos + x) * 0.1));
		sa_pos++;
		LeaveCriticalSection(&cs);
		return data;
	}
	LeaveCriticalSection(&cs);
	return 0;
}

volatile int sa_override;
void export_sa_setreq(int want)
{
	EnterCriticalSection(&cs);
	sa_override = want;
	LeaveCriticalSection(&cs);
}

char *export_sa_get_deprecated()
{
	static char data[75*2 + 8];
	int now = in_getouttime();
	char *p = sa_get(now, 3, data);
	if (!p) memset(data, 0, 75*2);

	return data;
}

char *export_sa_get(char data[75*2 + 8])
{
	try
	{
		int now = in_getouttime();
		char *p = sa_get(now, 3, data);
		if (!p) memset(data, 0, 75*2);
	}
	catch(...) {}
	return data;
}

#pragma optimize("", off) // for some reason, optimizations are breaking the case statement in bivis_thread
#define KILL_EVENT 0 
#define	BLANK_EVENT 1
#define ON_EVENT 2
#define	NUM_EVENTS 3

#define saKillEvent saEvents[0]
#define saBlankEvent saEvents[1]
#define saOnEvent saEvents[2]

HANDLE saEvents[NUM_EVENTS] = {0};

static int SA_Wait()
{
	if (WaitForSingleObject(saKillEvent, 16) == WAIT_OBJECT_0)
		return KILL_EVENT;

	if (WaitForSingleObject(saBlankEvent, 0) == WAIT_OBJECT_0)
		return BLANK_EVENT;

	return WaitForMultipleObjects(NUM_EVENTS, saEvents, FALSE, INFINITE)-WAIT_OBJECT_0;
}

static DWORD WINAPI bivis_thread(void *none)
{
	int cycleCount=0;
	__int8 data[75*2 + 8] = {0};
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);		
	SetThreadName((DWORD)-1, "Classic Viz");
	while (1)
	{
		switch(SA_Wait())
		{
		case KILL_EVENT:
			return 0;

		case BLANK_EVENT:
			draw_sa(NULL, 1);
			break;

		case ON_EVENT:
			{
				int draw=0;

				if (++cycleCount < config_saref)
					draw=0;
				else
				{
					cycleCount=0;
					draw=1;
				}

				if (config_sa
					&& !paused 
					&& playing 
					&& !config_minimized
					&& (config_mw_open || (config_pe_open && config_pe_width >= 350 && config_pe_height != 14)) 
					&& (!config_disvis || !vis_running()))
				{
					int a = in_getouttime();
					int t = config_sa;
					//if ((config_windowshade&&config_mw_open) && t == 1) t=4;
					char *c = sa_get(a, t, data);

					if (c)
					{
						if (t == 2) c += 75;
						else memset(c + 75, 0, 4);
						draw_sa((unsigned char*)c, draw);
					}
				}
			}
			break;
		}
	}
	return 0;
}

#pragma optimize("", on)
HANDLE saThread=0;
void SpectralAnalyzer_Create()
{
	DWORD threadId = 0;
	InitializeCriticalSection(&cs);
	saKillEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	saBlankEvent= CreateEvent(NULL, FALSE, FALSE, NULL);
	saOnEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	saThread = (HANDLE)	CreateThread(NULL, 256*1024, (LPTHREAD_START_ROUTINE) bivis_thread, 0, 0, &threadId);
	//cut: done on thread - SetThreadPriority(saThread, THREAD_PRIORITY_HIGHEST);		

	sa_length=sa_size=0;

	VU_Create();
}

void SpectralAnalyzer_Destroy()
{
	VU_Destroy();

	SetEvent(saKillEvent);
	WaitForSingleObject(saThread, INFINITE);
	CloseHandle(saThread);
	saThread = 0;

	CloseHandle(saKillEvent);
	CloseHandle(saBlankEvent);
	CloseHandle(saOnEvent);
	DeleteCriticalSection(&cs);

	free(sa_bufs);
	sa_bufs=0;
	sa_size=0;
}

volatile int sa_curmode;

/*
@param mode -1==shutdown 0==none, 1==spectral analyzer, 2==oscilloscope
*/

void sa_setthread(int mode)
{
	if (mode == -1)
		mode=0;

	sa_curmode = mode;

	if (mode)
	{
		SetEvent(saOnEvent);
	}
	else
	{
		ResetEvent(saOnEvent);
		SetEvent(saBlankEvent);
	}
}