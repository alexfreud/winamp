#ifndef _REVERSESYNC_H
#define _REVERSESYNC_H

WIN32_FIND_DATA *File_Exists(char *buf);
char *Skip_Root(char *path);
BOOL RecursiveCreateDirectory(char* buf1);
char *FixReplacementVars(char *str, int str_size, itemRecord * song);

#endif