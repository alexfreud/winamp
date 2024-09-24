#ifndef __METADATA_UTILS_H_
#define __METADATA_UTILS_H_

#include "pmp.h"

typedef struct {
  wchar_t * fn; 
  itemRecordW * ice;
} filenameMap;

typedef struct {
  itemRecordW * item;
  songid_t songid;
} PlaylistAddItem;

typedef union {
  struct {
    wchar_t * filename;
    itemRecordW * ice;
    songid_t song;
  };
  struct {
    filenameMap map;
    songid_t song;
  };
  struct {
    wchar_t * filename;
    PlaylistAddItem pladd;
  };
} songMapping;

void MapItemRecordsToSongs(Device * dev, PlaylistAddItem ** map, int len, C_ItemList * itemRecordsNotOnDevice=NULL);
void mapFilesToItemRecords(filenameMap ** map0, int len, HWND centerWindow);

void ProcessDatabaseDifferences(Device * dev, itemRecordListW * ml,C_ItemList * itemRecordsOnDevice, C_ItemList * itemRecordsNotOnDevice, C_ItemList * songsInML,  C_ItemList * songsNotInML);
void ProcessDatabaseDifferences(Device * dev, C_ItemList * ml,C_ItemList * itemRecordsOnDevice, C_ItemList * itemRecordsNotOnDevice, C_ItemList * songsInML,  C_ItemList * songsNotInML);
void getTitle(Device * dev, songid_t song, const wchar_t * filename,wchar_t * buf, int len);
C_ItemList * fileListToItemRecords(wchar_t** files,int l, HWND centerWindow);
C_ItemList * fileListToItemRecords(C_ItemList * fileList, HWND centerWindow);
void filenameToItemRecord(wchar_t * file, itemRecordW * ice);

int __fastcall compareSongs(const void *elem1, const void *elem2, const void *context);
int compareItemRecords(itemRecordW * a, itemRecordW * b);
int compareItemRecordAndSongId(itemRecordW * item, songid_t song, Device *dev);

void GetFileSizeAndTime(const wchar_t *filename, __int64 *file_size, time_t *file_time);

#endif // __METADATA_UTILS_H_