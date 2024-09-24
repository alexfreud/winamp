#ifndef NULLSOFT_VIDEOOUTPUTH
#define NULLSOFT_VIDEOOUTPUTH

extern "C" {
#include "main.h" // would rather not do this.
};
#include <ddraw.h>

#include "../nu/AutoLock.h"

#include "VideoOSD.h"
#include "VideoAspectAdjuster.h"
#include "vid_d3d.h"

class VideoRenderer;

extern IVideoOSD *posd;

#define VIDEO_OPEN_

#define SHOW_STREAM_TITLE_AT_TOP 1
void ResizeVideoWindowToCurrent();
void updateTrackSubmenu();
using namespace Nullsoft::Utility;
class VideoOutput : public IVideoOutput, public VideoAspectAdjuster
{
public:
	VideoOutput(HWND parent_hwnd = NULL, int initxpos = CW_USEDEFAULT, int initypos = CW_USEDEFAULT);
	~VideoOutput();
	int open(int w, int h, int vflip, double aspectratio, unsigned int fmt);
	void close();
	void draw(void *frame);
	void drawSubtitle(SubsItem *item);
	void showStatusMsg(const char *text);
	void notifyBufferState(int bufferstate); /* 0-255*/
	int get_latency();
	void setcallback(LRESULT (*msgcallback)(void *token, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam), void *token) { m_msgcallback_tok = token; m_msgcallback = msgcallback; }
	void fullscreen();
	void remove_fullscreen();
	int is_fullscreen();
	void adjustAspect(RECT &rd);
	INT_PTR extended(INT_PTR param1, INT_PTR param2, INT_PTR param3);
	int isVideoPlaying() { return m_opened; }
	DWORD GetWidthHeightDWORD() { return width | (height << 16); }
	ITrackSelector *getTrackSelector() { return m_tracksel; }
	void SetVideoPosition(int x, int y, int width, int height);
	HWND getHwnd() { return video_hwnd; }

	void mainthread_Create();

private:
	void UpdateText(const wchar_t *videoInfo);

	int openUser(int w, int h, int vflip, double aspectratio, unsigned int fmt);
	void LoadLogo();
	void OpenVideoSize(int newWidth, int newHeight, double newAspect);
	VideoRenderer *FindBestRenderer();
	void PaintLogo(int bufferState);
	void DrawLogo(HDC canvas, RECT *canvas_size);
	void resetSubtitle();
	void UpdateVideoSize(int newWidth, int newHeight, double aspect = 1.0, int zoom = ID_VIDEOWND_ZOOM100);
	bool is_fs, fs_has_resized;
	unsigned int last_fullscreen_exit_time;
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static int class_refcnt;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND video_hwnd;
	double aspect;
	int width, height;
	unsigned int type;


	RECT oldfsrect;  // the old window rect, BEFORE fullscreen mode was entered
	RECT lastfsrect; // the most recent bounding rect when in fullscreen mode
	int m_bufferstate;
	SubsItem *curSubtitle;
	LRESULT (*m_msgcallback)(void *token, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void *m_msgcallback_tok;
	Direct3DVideoOutput *m_video_output;
	char *m_statusmsg;
	bool m_need_change, m_ignore_change;
	bool m_opened;
	HBITMAP m_logo;
	int m_logo_w, m_logo_h;
	int m_lastbufinvalid;
	HWND fs_reparented;
	HRGN fs_reparented_rgn;
	ITrackSelector *m_tracksel;
	LockGuard guard, textGuard;
	int clickx, clicky;
	RGBQUAD *video_palette;
//	IVideoOSD osd;
	bool userVideo;
	bool video_created;
};

#endif
