//PORTABLE
#ifndef _GURU_H
#define _GURU_H

#include <api/wnd/basewnd.h>

#ifdef WASABI_COMPILE_WND
#define GURU_PARENT BaseWnd
#else
class _Guru {};
#define GURU_PARENT _Guru
#endif

class SystemObject;

#define GURU_TIMERID 2482

class Guru : public GURU_PARENT {
public:
#ifdef WASABI_COMPILE_WND
	Guru();
	virtual ~Guru();
#endif

	static void spawn(SystemObject *_script, int code, const wchar_t *pub = NULL, int intinfo = 0);

#ifdef WASABI_COMPILE_WND
	virtual int onPaint(Canvas *canvas);
	virtual int onLeftButtonUp(int x, int y);
	virtual int onInit();
	void setCode(int c);
	void setPublicTxt(const wchar_t *t);
	void setIntInfo(int info);
#endif

#ifdef WASABI_COMPILE_WND
protected:
	virtual void timerCallback(int id);

private:
	int code;
	const wchar_t *txt;
	int fcount;
	int intinfo;
	static int mustquit;
	static int last_iterator;
	static SystemObject * script;
#endif
};

#endif
