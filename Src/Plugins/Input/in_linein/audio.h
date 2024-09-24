#ifndef NULLSOFT_AUDIOH
#define NULLSOFT_AUDIOH

int audioInit(int);
int audioGetPos();
void audioSetPos(int ms);
void audioGetWaveform(unsigned short data[576*2]);
void audioQuit();
void audioPause(int s);

#endif