#include "nxlog.h"
#include <stdio.h>
#include <stdarg.h>
//#include <windows.h>

static char *nx_log_tag = "libreplicant";
#define MAX_FMT_SIZE 512

void NXLog(int priority, char *fmt, ...){

	char formatted_string[MAX_FMT_SIZE];

	 va_list argptr;
	 va_start(argptr,fmt);
	 vsnprintf(formatted_string, MAX_FMT_SIZE, fmt, argptr);
	 va_end(argptr);

//	OutputDebugString(formatted_string);
}
