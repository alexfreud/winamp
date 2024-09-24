#ifndef __TRUETYPEFONTDEF_H
#define __TRUETYPEFONTDEF_H

// Macros for TrueType portability
#define FS_2BYTE(p)  ( ((unsigned short)((p)[0]) << 8) |  (p)[1])
#define FS_4BYTE(p)  ( FS_2BYTE((p)+2) | ( (FS_2BYTE(p)+0L) << 16) )
#define SWAPW(a)        ((short) FS_2BYTE( (unsigned char FAR*)(&a) ))
#define SWAPL(a)        ((long) FS_4BYTE( (unsigned char FAR*)(&a) ))

typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;
typedef long sfnt_TableTag;

typedef struct {
    uint16 platformID;
    uint16 specificID;
    uint16 languageID;
    uint16 nameID;
    uint16 length;
    uint16 offset;
} sfnt_NameRecord;

typedef struct {
    uint16 format;
    uint16 count;
    uint16 stringOffset;
} sfnt_NamingTable;

typedef struct {
    sfnt_TableTag   tag;
    uint32          checkSum;
    uint32          offset;
    uint32          length;
} sfnt_DirectoryEntry;

typedef struct {
    int32 version;                  
    uint16 numOffsets;              
    uint16 searchRange;             
    uint16 entrySelector;           
    uint16 rangeShift;              
    sfnt_DirectoryEntry table[1];   
} sfnt_OffsetTable;
#define OFFSETTABLESIZE     12  

typedef struct {
  int dcstate;
  HFONT font;
  HFONT prevfont;
} fontslot;

#define tag_NamingTable         0x656d616e        /* 'name' */

#endif