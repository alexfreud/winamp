#ifndef __DDECOM_H
#define __DDECOM_H

class DdeCom {
public:
	static void sendCommand(wchar_t *application, wchar_t *command, DWORD minInterval);
};

#endif
