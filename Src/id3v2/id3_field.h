//  The authors have released ID3Lib as Public Domain (PD) and claim no copyright,
//  patent or other intellectual property protection in this work.  This means that
//  it may be modified, redistributed and used in commercial and non-commercial
//  software and hardware without restrictions.  ID3Lib is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
//  
//  The ID3Lib authors encourage improvements and optimisations to be sent to the
//  ID3Lib coordinator, currently Dirk Mahoney (dirk@id3.org).  Approved
//  submissions may be altered, and will be included and released under these terms.
//  
//  Mon Nov 23 18:34:01 1998
// improved/optimized/whatEVER jan-08-2006 benski

#ifndef	ID3LIB_FIELD_H
#define	ID3LIB_FIELD_H


#include <wchar.h>
#include "id3_types.h"
#include "id3_error.h"


// field flags
#define	ID3FF_NULL			(1 << 0)
#define	ID3FF_NULLDIVIDE	(1 << 1)
#define	ID3FF_ADJUSTENC		(1 << 2)
#define	ID3FF_ADJUSTEDBY	(1 << 3)


enum ID3_TextEnc
{
  ID3TE_ASCII			= 0,
  ID3TE_UNICODE = 1, // UTF-16
  ID3TE_UTF16_BE = 2, // UTF-16 big endian, no BOM
  ID3TE_UTF8 = 3, // UTF-8
};

enum ID3_FieldType
{
  ID3FTY_INTEGER		= 0,
  ID3FTY_BITFIELD,
  ID3FTY_BINARY,
  ID3FTY_ASCIISTRING,
  ID3FTY_UNICODESTRING,
  ID3FTY_UTF8STRING,
};

enum ID3_FieldID
{
  ID3FN_NOFIELD			= 0,
  ID3FN_TEXTENC,
  ID3FN_TEXT,
  ID3FN_URL,
  ID3FN_DATA,
  ID3FN_DESCRIPTION,
  ID3FN_OWNER,
  ID3FN_EMAIL,
  ID3FN_RATING,
  ID3FN_FILENAME,
  ID3FN_LANGUAGE,
  ID3FN_PICTURETYPE,
  ID3FN_IMAGEFORMAT,
  ID3FN_MIMETYPE,
  ID3FN_TIMESTAMP,
  ID3FN_CONTENTTYPE,
  ID3FN_COUNTER,
  ID3FN_SYMBOL,
  ID3FN_VOLUMEADJ,
  ID3FN_NUMBITS,
  ID3FN_VOLCHGRIGHT,
  ID3FN_VOLCHGLEFT,
  ID3FN_PEAKVOLRIGHT,
  ID3FN_PEAKVOLLEFT,
  ID3FN_CD_TOC,

  ID3FN_LASTFIELDID
};


enum ID3_FrameID
{
  ID3FID_NOFRAME			= 0,
  ID3FID_ENCODEDBY,
  ID3FID_ORIGALBUM,
  ID3FID_PUBLISHER,
  ID3FID_ENCODERSETTINGS,
  ID3FID_ORIGFILENAME,
  ID3FID_LANGUAGE,
  ID3FID_PARTINSET,
  ID3FID_DATE,
  ID3FID_TIME,
  ID3FID_RECORDINGDATES,
  ID3FID_MEDIATYPE,
  ID3FID_FILETYPE,
  ID3FID_NETRADIOSTATION,
  ID3FID_NETRADIOOWNER,
  ID3FID_LYRICIST,
  ID3FID_ORIGARTIST,
  ID3FID_ORIGLYRICIST,
  ID3FID_CONTENTGROUP,
  ID3FID_TITLE,
  ID3FID_SUBTITLE,
  ID3FID_LEADARTIST,
  ID3FID_BAND,
  ID3FID_CONDUCTOR,
  ID3FID_MIXARTIST,
  ID3FID_ALBUM,
  ID3FID_YEAR,
  ID3FID_COMPOSER,
  ID3FID_COPYRIGHT,
  ID3FID_PRODUCEDNOTICE,
  ID3FID_CONTENTTYPE,
  ID3FID_TRACKNUM,
  ID3FID_USERTEXT,
  ID3FID_COMMENT,
  ID3FID_TERMSOFUSE,
  ID3FID_UNSYNCEDLYRICS,
  ID3FID_SYNCEDLYRICS,
  ID3FID_SYNCEDTEMPOCODE,
  ID3FID_WWWAUDIOFILE,
  ID3FID_WWWARTIST,
  ID3FID_WWWAUDIOSOURCE,
  ID3FID_WWWCOMMERCIALINFO,
  ID3FID_WWWCOPYRIGHT,
  ID3FID_WWWPUBLISHER,
  ID3FID_WWWPAYMENT,
  ID3FID_WWWRADIOPAGE,
  ID3FID_WWWUSER,
  ID3FID_INVOLVEDPEOPLE,
  ID3FID_PICTURE,
  ID3FID_GENERALOBJECT,
  ID3FID_UNIQUEFILEID,
  ID3FID_PRIVATE,
  ID3FID_PLAYCOUNTER,
  ID3FID_POPULARIMETER,
  ID3FID_CRYPTOREG,
  ID3FID_GROUPINGREG,
  ID3FID_SIGNATURE,
  ID3FID_MCDI,
  ID3FID_BPM,
  ID3FID_KEY,
  ID3FID_MOOD,
  ID3FID_ISRC,
  ID3FID_RECORDINGTIME,
  ID3FID_COMPILATION,
  ID3FID_ALBUMSORT,
  ID3FID_ALBUMARTISTSORT,
  ID3FID_PERFORMERSORT,
  ID3FID_COMPOSERSORT,
  ID3FID_TITLESORT,
  ID3FID_REPLAYGAIN,
  ID3FID_VOLUMEADJ,
  ID3FID_INVOLVEDPEOPLE2,
  ID3FID_CREDITS,
  ID3FID_ENCODINGTIME,
  ID3FID_FILEOWNER,
  ID3FID_LENGTH,
  ID3FID_ORIGYEAR,
  ID3FID_ORIGRELEASETIME,
  ID3FID_RELEASETIME,
  ID3FID_SETSUBTITLE,
  ID3FID_TAGGINGTIME,
  ID3FID_PLAYLISTDELAY,
  ID3FID_PODCAST,
  ID3FID_PODCASTCATEGORY,
  ID3FID_PODCASTDESC,
  ID3FID_PODCASTID,
  ID3FID_PODCASTURL,
};


enum ID3_VerCtl
{
  ID3VC_HIGHER	= 0,
  ID3VC_LOWER
};


struct ID3_FieldDef
{
  ID3_FieldID		id;
  ID3_FieldType	type;
  lsint			fixedLength;
  uchar			version;
  uchar			revision;
  ID3_VerCtl		control;
  luint			flags;
  ID3_FieldID		linkedField;
};


class ID3_Frame;

// TODO: add minimum/maximum version & revision
struct ID3_FrameDef
{
  ID3_FrameID	id;
  char *shortTextID;
  char *longTextID;
  bool tagDiscard;
  bool fileDiscard;
  bool (*parseHandler)(ID3_Frame *frame);
  ID3_FieldDef *fieldDefs;
};


class ID3_Field
{
public:
  ID3_Field(void);
  ~ID3_Field(void);

  void			Clear(void);
  luint			Size(void);
  luint			GetNumTextItems(void);
  // integer field functions
  //ID3_Field&		operator=						(luint newData);
  void			Set(luint newData);
  luint			Get(void);
  // Unicode string field functions
  //ID3_Field&		operator=						(wchar_t *string);
  void SetUnicode(const wchar_t *string);
  luint GetUnicode(wchar_t *buffer, luint maxChars, luint itemNum = 1);
  void AddUnicode(const wchar_t *string);
  // ASCII string field functions
  //ID3_Field&		operator=						(char *string);
  void SetLatin(const char *string);
  void SetLocal(const char *string);
  void SetUTF8(const char *string);

  luint GetLocal(char *buffer, luint maxChars, luint itemNum = 1);
  luint GetLatin(char *buffer, luint maxChars, luint itemNum = 1);

  void AddLocal(const char *string);
  void AddLatin(const char *string);

  // binary field functions
  void			Set(uchar *newData, luint newSize);
  void			Get(uchar *buffer, luint buffLength);

  // *** PRIVATE INTERNAL DATA - DO NOT USE *** PRIVATE INTERNAL DATA - DO NOT USE ***

  luint			BinSize(bool withExtras = true);
  bool			HasChanged(void);
  void			SetVersion(uchar ver, uchar rev);
  luint			Render(uchar *buffer);
  luint			Parse(uchar *buffer, luint posn, luint buffSize);
  ID3_FieldID		name;							// the ID of this field
  ID3_FieldType	type;							// what type is this field or should be
  lsint			fixedLength;					// if this is positive, the length of the field is fixed
  uchar			ioVersion;						// specific version
  uchar			ioRevision;						// specific revision
  ID3_VerCtl		control;						// render if ver/rev is higher, or lower than frame::version, frame::revision?
  luint			flags;							// special field flags
  uchar			version;						// the version being rendered/parsed
  uchar			revision;						// the revision being rendered/parsed
  bool			hasChanged;						// has the field changed since the last parse/render?
protected:
  luint			RenderInteger					(uchar *buffer);
  luint			RenderLatinString(uchar *buffer);
  luint			RenderUnicodeString				(uchar *buffer);
	luint			RenderUTF8String(uchar *buffer);
  luint			RenderBinary					(uchar *buffer);

  luint			ParseInteger					(uchar *buffer, luint posn, luint buffSize);
  luint			ParseASCIIString				(uchar *buffer, luint posn, luint buffSize);
  luint			ParseUnicodeString				(uchar *buffer, luint posn, luint buffSize);
  luint	ParseUTF8String(uchar *buffer, luint posn, luint buffSize);
  luint			ParseBinary						(uchar *buffer, luint posn, luint buffSize);

  uchar			*data;
  luint			size;
};


ID3_FrameDef	*ID3_FindFrameDef	(ID3_FrameID id);
ID3_FrameID		ID3_FindFrameID(const char *id);


#endif


