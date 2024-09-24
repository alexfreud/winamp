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


#ifndef	ID3LIB_DLLHEADERS_H
#define	ID3LIB_DLLHEADERS_H


typedef	unsigned char		uchar;
typedef short signed int	ssint;
typedef short unsigned int	suint;
typedef long signed int		lsint;
typedef long unsigned int	luint;
typedef	long double			ldoub;
typedef long unsigned int *	bitset;


struct ID3_VerInfo
{
char	name		[ 30 ];
luint	version,
		revision;
};


enum ID3_TextEnc
{
	ID3TE_ASCII			= 0,
	ID3TE_UNICODE
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
	ID3FN_COUNTER,
	ID3FN_SYMBOL,
	ID3FN_VOLUMEADJ,
	ID3FN_NUMBITS,
	ID3FN_VOLCHGRIGHT,
	ID3FN_VOLCHGLEFT,
	ID3FN_PEAKVOLRIGHT,
	ID3FN_PEAKVOLLEFT,

	ID3FN_LASTFIELDID
};


enum ID3_FrameID
{
	ID3FID_NOFRAME			= 0,
	ID3FID_ORIGALBUM,
	ID3FID_PUBLISHER,
	ID3FID_ENCODEDBY,
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
	ID3FID_SUBTITLE,
	ID3FID_MIXARTIST,
	ID3FID_USERTEXT,
	ID3FID_CONTENTGROUP,
	ID3FID_TITLE,
	ID3FID_LEADARTIST,
	ID3FID_BAND,
	ID3FID_ALBUM,
	ID3FID_YEAR,
	ID3FID_CONDUCTOR,
	ID3FID_COMPOSER,
	ID3FID_COPYRIGHT,
	ID3FID_CONTENTTYPE,
	ID3FID_TRACKNUM,
	ID3FID_COMMENT,
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
	ID3FID_UNSYNCEDLYRICS,
	ID3FID_PICTURE,
	ID3FID_GENERALOBJECT,
	ID3FID_UNIQUEFILEID,
	ID3FID_PLAYCOUNTER,
	ID3FID_POPULARIMETER,
	ID3FID_GROUPINGREG,
	ID3FID_CRYPTOREG
};


class ID3_Field;
class ID3_Frame;
class ID3_Tag;


void			ID3_GetVersion					( ID3_VerInfo *info );
// tag wrappers
ID3_Tag			*ID3Tag_New						( void );
void			ID3Tag_Delete					( ID3_Tag *tag );
void			ID3Tag_Clear					( ID3_Tag *tag );
bool			ID3Tag_HasChanged				( ID3_Tag *tag );
void			ID3Tag_SetUnsync				( ID3_Tag *tag, bool unsync );
void			ID3Tag_SetExtendedHeader		( ID3_Tag *tag, bool ext );
void			ID3Tag_SetCompression			( ID3_Tag *tag, bool comp );
void			ID3Tag_SetPadding				( ID3_Tag *tag, bool pad );
void			ID3Tag_AddFrame					( ID3_Tag *tag, ID3_Frame *frame );
void			ID3Tag_AddFrames				( ID3_Tag *tag, ID3_Frame *frames, luint num );
void			ID3Tag_RemoveFrame				( ID3_Tag *tag, ID3_Frame *frame );
void			ID3Tag_Parse					( ID3_Tag *tag, uchar header[ ID3_TAGHEADERSIZE ], uchar *buffer );
luint			ID3Tag_Link						( ID3_Tag *tag, char *fileName );
void			ID3Tag_Update					( ID3_Tag *tag );
void			ID3Tag_Strip					( ID3_Tag *tag, bool v1Also );
ID3_Frame		*ID3Tag_FindFrameWithID			( ID3_Tag *tag, ID3_FrameID id );
ID3_Frame		*ID3Tag_FindFrameWithINT		( ID3_Tag *tag, ID3_FrameID id, ID3_FieldID fld, luint data );
ID3_Frame		*ID3Tag_FindFrameWithASCII		( ID3_Tag *tag, ID3_FrameID id, ID3_FieldID fld, char *data );
ID3_Frame		*ID3Tag_FindFrameWithUNICODE	( ID3_Tag *tag, ID3_FrameID id, ID3_FieldID fld, wchar_t *data );
ID3_Frame		*ID3Tag_GetFrameNum				( ID3_Tag *tag, luint num );
luint			ID3Tag_NumFrames				( ID3_Tag *tag );
// frame wrappers
void			ID3Frame_Clear					( ID3_Frame *frame );
void			ID3Frame_SetID					( ID3_Frame *frame, ID3_FrameID id );
ID3_FrameID		ID3Frame_GetID					( ID3_Frame *frame );
ID3_Field		*ID3Frame_GetField				( ID3_Frame *frame, ID3_FieldID name );
// field wrappers
void			ID3Field_Clear					( ID3_Field *field );
luint			ID3Field_Size					( ID3_Field *field );
luint			ID3Field_GetNumTextItems		( ID3_Field *field );
void			ID3Field_SetINT					( ID3_Field *field, luint data );
luint			ID3Field_GetINT					( ID3_Field *field );
void			ID3Field_SetUNICODE				( ID3_Field *field, wchar_t *string );
luint			ID3Field_GetUNICODE				( ID3_Field *field, wchar_t *buffer, luint maxChars, luint itemNum );
void			ID3Field_AddUNICODE				( ID3_Field *field, wchar_t *string );
void			ID3Field_SetASCII				( ID3_Field *field, char *string );
luint			ID3Field_GetASCII				( ID3_Field *field, char *buffer, luint maxChars, luint itemNum );
void			ID3Field_AddASCII				( ID3_Field *field, char *string );
void			ID3Field_SetBINARY				( ID3_Field *field, uchar *data, luint size );
void			ID3Field_GetBINARY				( ID3_Field *field, uchar *buffer, luint buffLength );
void			ID3Field_FromFile				( ID3_Field *field, char *fileName );
void			ID3Field_ToFile					( ID3_Field *field, char *fileName );


#endif


