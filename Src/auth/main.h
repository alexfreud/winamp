#pragma once

#define OPENAUTH_DEVID "ao1Ve8gG_VGE9aNP"
//#define OPENAUTH_DEVID "co1-aGYgRvsTuOey"

void Log(const wchar_t *format, ...);
const wchar_t *MakeDateString(__time64_t convertTime);

/* decode.cpp */
void UrlDecode(char *str);