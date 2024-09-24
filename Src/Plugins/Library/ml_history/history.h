#pragma once
#ifndef NULLSOFT_ML_HISTORY_HISTORY_H
#define NULLSOFT_ML_HISTORY_HISTORY_H

#include <time.h>
typedef struct
{
  wchar_t *filename;
  wchar_t *title;
  wchar_t *ext;
  int length;
  unsigned int playcnt;
  __time64_t lastplayed;
  int offset;
} historyRecord;

typedef struct 
{
  historyRecord *Items;
  int Size;
  int Alloc;
} historyRecordList;

enum
{
	HISTORY_SORT_LASTPLAYED = 0,
	HISTORY_SORT_PLAYCOUNT = 1,
	HISTORY_SORT_TITLE = 2,
	HISTORY_SORT_LENGTH = 3,
	HISTORY_SORT_FILENAME = 4,
	HISTORY_SORT_OFFSET = 5,
};


#endif