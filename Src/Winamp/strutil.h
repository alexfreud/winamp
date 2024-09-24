/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/

#ifndef NULLSOFT_STRUTILH
#define NULLSOFT_STRUTILH

#ifndef FILENAME_SIZE
#define FILENAME_SIZE (MAX_PATH * 4)
#endif

#ifndef FILETITLE_SIZE
#define FILETITLE_SIZE 400
#endif

#ifdef __cplusplus
extern "C" {
#endif
char *SkipX(char *str, int count);
wchar_t *SkipXW(wchar_t *str, int count);
void CopyChar(char *dest, const char *src);
ptrdiff_t CopyCharW(wchar_t *dest, const wchar_t *src); // returns number of character copied
void MakeRelativePathName(const wchar_t *filename, wchar_t *outFile, const wchar_t *path);
int FileCompareLogicalN(const wchar_t *str1, ptrdiff_t str1size, const wchar_t *str2, ptrdiff_t str2size);
int FileCompareLogical(const wchar_t *str1, const wchar_t *str2);
int CompareStringLogical(const wchar_t * str1, const wchar_t * str2);
char *GetLastCharacter(char *string);
wchar_t *GetLastCharacterW(wchar_t *string);
const char *GetLastCharacterc(const char *string);
const wchar_t *GetLastCharactercW(const wchar_t *string);

wchar_t *scanstr_backW(wchar_t *str, wchar_t *toscan, wchar_t *defval);
const wchar_t *scanstr_backcW(const wchar_t *str, const wchar_t *toscan, const wchar_t *defval);
char *scanstr_back(char *str, char *toscan, char *defval);
char *scanstr_backc(const char *str, char *toscan, char *defval);

char *extension(const char *fn);
wchar_t *extensionW(const wchar_t *fn);
const char *extensionc(const char *fn);
void extension_ex(const char *fn, char *buf, int buflen);
void extension_exW(const wchar_t *fn, wchar_t *buf, int buflen);

#ifdef __cplusplus
}
#endif

#endif