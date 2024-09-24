#ifndef NULLSOFT_RFCDATEH
#define NULLSOFT_RFCDATEH

void MakeRFCDateString(__time64_t convertTime, wchar_t *data_str, size_t len);
__time64_t MakeRFCDate(const wchar_t *date);
void MakeDateString(__time64_t convertTime, wchar_t *date_str, size_t len);
#endif