/***********************************************\
??? duck_io.c
\***********************************************/

#include <stdio.h> 
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>

#include "duck_io.h"
#include "duck_hfb.h"

int duck_strcmp(const char *s1, const char *s2)
{
    return strcmp(s1, s2); 
}


