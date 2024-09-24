#ifndef NULLSOFT_MAINTHREADH
#define NULLSOFT_MAINTHREADH
#include <windows.h>
class Lambda
{
public:
	virtual void Run() = 0;
};

template <class func_t, class param_t>
class LambdaC : public Lambda
{
public:
	LambdaC(func_t _func, param_t _param)
			: func(_func), param(_param)
	{
		event = CreateEvent(0, FALSE, FALSE, 0);
	}
	~LambdaC()
	{
		CloseHandle(event);
	}
	void Run()
	{
		func(param);
		SetEvent(event);
	}
private:
	HANDLE event;
	func_t func;
	param_t param;
};

template <class object_t, class func_t, class param_t>
class LambdaCPP
{
public:
	LambdaCPP(object_t *_object, func_t *_func, param_t _param)
			: func(_func), param(_param)
	{
		event = CreateEvent(0, FALSE, FALSE, 0);
	}
	~LambdaCPP()
	{
		CloseHandle(event);
	}
	void Run()
	{
		object->*func(param);
		SetEvent(event);
	}
private:
	HANDLE event;
	object_t *object;
	func_t *func;
	param_t param;
};

class MainThread
{
public:
	MainThread();
	template <class func_t, class param_t>
	void Run(func_t *func, param_t param)
	{
		Lambda *lambda = new LambdaC(func, param);
		PostMessage(mainWindow, WM_USER, lambda, 0);
	}

	template <class object_t, class func_t, class param_t>
	void Run(object_t *object, func_t *func, param_t param)
	{
		Lambda *lambda = new LambdaCPP(object, func, param);
		PostMessage(mainWindow, WM_USER, lambda, 0);
	}
private:
	HWND mainWindow;
};

extern MainThread mainThread;

#endif