#ifndef NULLSOFT_IN_MP3_CONFIG_H
#define NULLSOFT_IN_MP3_CONFIG_H

#include <bfc/platform/guid.h>
enum
{
  WRITE_UTF16 = 0,
  WRITE_LATIN = 1,
  WRITE_LOCAL = 2,
  WRITE_UTF8 = 3,
};
extern int config_write_mode;

enum
{
  READ_LATIN = 0,
  READ_LOCAL = 1,
};

extern int config_read_mode;

extern int config_parse_apev2;
extern int config_parse_lyrics3;
extern int config_parse_id3v1;
extern int config_parse_id3v2;

extern int config_write_apev2;
extern int config_write_id3v1;
extern int config_write_id3v2;

extern int config_create_id3v1;
extern int config_create_id3v2;
extern int config_create_apev2;

enum
{
	RETAIN_HEADER = 0,
	ADD_HEADER = 1,
	REMOVE_HEADER = 2,
};
extern int config_apev2_header;

extern int config_id3v2_version;
extern int config_lp;
extern char config_rating_email[255];

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID = 
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

#endif
