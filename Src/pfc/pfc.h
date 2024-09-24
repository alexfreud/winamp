#ifndef STRICT
#define STRICT
#endif
#include <windows.h>

#ifndef NOVTABLE
#define NOVTABLE _declspec(novtable)
#endif

#define tabsize(x) (sizeof(x)/sizeof(*x))

#include "string.h"
#ifdef PFC_UNICODE
#include "string_unicode.h"
#endif
#include "cfg_var.h"
#include "critsec.h"
#include "grow_buf.h"
#include "mem_block.h"
#include "ptr_list.h"