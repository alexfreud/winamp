#ifndef NULLSOFT_NU_CSTRLIBH
#define NULLSOFT_NU_CSTRLIBH
extern "C" 
{
	char *scanstr_back(char *str, char *toscan, char *defval);
	char *extension(char *fn);
}
void CleanDirectory(char *str);
void FormatSizeStr64(char *out, __int64 size);
#endif