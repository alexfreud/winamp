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


#include "id3_tag.h"


#ifdef __DLL


#include <string.h>


struct ID3_VerInfo
{
char	name		[ 30 ];
luint	version,
		revision;
};



// misc wrappers

CDLLEXPORT
void			ID3_GetVersion					( ID3_VerInfo *info )
{
	info->version	= ID3LIB_VER;
	info->revision	= ID3LIB_REV;
	strcpy ( info->name, ID3LIB_NAME );

	return;
}


// tag wrappers

CDLLEXPORT
ID3_Tag			*ID3Tag_New						( void )
{
	return new ID3_Tag;
}


CDLLEXPORT
void			ID3Tag_Delete					( ID3_Tag *tag )
{
	if	( tag )
		delete tag;

	return;
}


CDLLEXPORT
void			ID3Tag_Clear					( ID3_Tag *tag )
{
	if	( tag )
		tag->Clear();

	return;
}


CDLLEXPORT
bool			ID3Tag_HasChanged				( ID3_Tag *tag )
{
	bool	changed	= false;

	if	( tag )
		changed = tag->HasChanged();

	return changed;
}


CDLLEXPORT
void			ID3Tag_SetUnsync				( ID3_Tag *tag, bool unsync )
{
	if	( tag )
		tag->SetUnsync ( unsync );

	return;
}


CDLLEXPORT
void			ID3Tag_SetExtendedHeader		( ID3_Tag *tag, bool ext )
{
	if	( tag )
		tag->SetExtendedHeader ( ext );

	return;
}


CDLLEXPORT
void			ID3Tag_SetCompression			( ID3_Tag *tag, bool comp )
{
	if	( tag )
		tag->SetCompression ( comp );

	return;
}


CDLLEXPORT
void			ID3Tag_SetPadding				( ID3_Tag *tag, bool pad )
{
	if	( tag )
		tag->SetPadding ( pad );

	return;
}


CDLLEXPORT
void			ID3Tag_AddFrame					( ID3_Tag *tag, ID3_Frame *frame )
{
	if	( tag )
		tag->AddFrame ( frame );

	return;
}


CDLLEXPORT
void			ID3Tag_AddFrames				( ID3_Tag *tag, ID3_Frame *frames, luint num )
{
	if	( tag )
		tag->AddFrames ( frames, num );

	return;
}


CDLLEXPORT
void			ID3Tag_RemoveFrame				( ID3_Tag *tag, ID3_Frame *frame )
{
	if	( tag )
		tag->RemoveFrame ( frame );

	return;
}


CDLLEXPORT
void			ID3Tag_Parse					( ID3_Tag *tag, uchar header[ ID3_TAGHEADERSIZE ], uchar *buffer )
{
	if	( tag )
		tag->Parse ( header, buffer );

	return;
}


CDLLEXPORT
luint			ID3Tag_Link						( ID3_Tag *tag, char *fileName )
{
	luint	offset	= 0;

	if	( tag )
		offset = tag->Link ( fileName );

	return offset;
}


CDLLEXPORT
void			ID3Tag_Update					( ID3_Tag *tag )
{
	if	( tag )
		tag->Update();

	return;
}


CDLLEXPORT
void			ID3Tag_Strip					( ID3_Tag *tag, bool v1Also )
{
	if	( tag )
		tag->Strip ( v1Also );

	return;
}


CDLLEXPORT
ID3_Frame		*ID3Tag_FindFrameWithID			( ID3_Tag *tag, ID3_FrameID id )
{
	ID3_Frame	*frame	= NULL;

	if	( tag )
		frame = tag->Find ( id );

	return frame;
}


CDLLEXPORT
ID3_Frame		*ID3Tag_FindFrameWithINT		( ID3_Tag *tag, ID3_FrameID id, ID3_FieldID fld, luint data )
{
	ID3_Frame	*frame	= NULL;

	if	( tag )
		frame = tag->Find ( id, fld, data );

	return frame;
}


CDLLEXPORT
ID3_Frame		*ID3Tag_FindFrameWithASCII		( ID3_Tag *tag, ID3_FrameID id, ID3_FieldID fld, char *data )
{
	ID3_Frame	*frame	= NULL;

	if	( tag )
		frame = tag->Find ( id, fld, data );

	return frame;
}


CDLLEXPORT
ID3_Frame		*ID3Tag_FindFrameWithUNICODE	( ID3_Tag *tag, ID3_FrameID id, ID3_FieldID fld, wchar_t *data )
{
	ID3_Frame	*frame	= NULL;

	if	( tag )
		frame = tag->Find ( id, fld, data );

	return frame;
}


CDLLEXPORT
luint			ID3Tag_NumFrames				( ID3_Tag *tag )
{
	luint	num	= 0;

	if	( tag )
		num = tag->NumFrames();

	return num;
}


CDLLEXPORT
ID3_Frame		*ID3Tag_GetFrameNum				( ID3_Tag *tag, luint num )
{
	ID3_Frame	*frame	= NULL;

	if	( tag )
		frame = tag->GetFrameNum ( num );

	return frame;
}


// frame wrappers

CDLLEXPORT
void			ID3Frame_Clear					( ID3_Frame *frame )
{
	if	( frame )
		frame->Clear();

	return;
}


CDLLEXPORT
void			ID3Frame_SetID					( ID3_Frame *frame, ID3_FrameID id )
{
	if	( frame )
		frame->SetID ( id );

	return;
}


CDLLEXPORT
ID3_FrameID		ID3Frame_GetID					( ID3_Frame *frame )
{
	ID3_FrameID	id	= ID3FID_NOFRAME;

	if	( frame )
		id = frame->GetID();

	return id;
}


CDLLEXPORT
ID3_Field		*ID3Frame_GetField				( ID3_Frame *frame, ID3_FieldID name )
{
	ID3_Field	*field	= NULL;

	if	( frame )
		field = &( frame->Field ( name ) );

	return field;
}


// field wrappers


CDLLEXPORT
void			ID3Field_Clear					( ID3_Field *field )
{
	if	( field )
		field->Clear();

	return;
}


CDLLEXPORT
luint			ID3Field_Size					( ID3_Field *field )
{
	luint	size	= 0;

	if	( field )
		size = field->Size();

	return size;
}


CDLLEXPORT
luint			ID3Field_GetNumTextItems		( ID3_Field *field )
{
	luint	items	= 0;

	if	( field )
		items = field->GetNumTextItems();

	return items;
}


CDLLEXPORT
void			ID3Field_SetINT					( ID3_Field *field, luint data )
{
	if	( field )
		field->Set ( data );

	return;
}


CDLLEXPORT
luint			ID3Field_GetINT					( ID3_Field *field )
{
	luint	value	= 0;

	if	( field )
		value = field->Get();

	return value;
}


CDLLEXPORT
void			ID3Field_SetUNICODE				( ID3_Field *field, wchar_t *string )
{
	if	( field )
		field->Set ( string );

	return;
}


CDLLEXPORT
luint			ID3Field_GetUNICODE				( ID3_Field *field, wchar_t *buffer, luint maxChars, luint itemNum )
{
	luint	numChars	= 0;

	if	( field )
		numChars = field->Get ( buffer, maxChars, itemNum );

	return numChars;
}


CDLLEXPORT
void			ID3Field_AddUNICODE				( ID3_Field *field, wchar_t *string )
{
	if	( field )
		field->Add ( string );

	return;
}


CDLLEXPORT
void			ID3Field_SetASCII				( ID3_Field *field, char *string )
{
	if	( field )
		field->Set ( string );

	return;
}


CDLLEXPORT
luint			ID3Field_GetASCII				( ID3_Field *field, char *buffer, luint maxChars, luint itemNum )
{
	luint	numChars	= 0;

	if	( field )
		numChars = field->Get ( buffer, maxChars, itemNum );

	return numChars;
}


CDLLEXPORT
void			ID3Field_AddASCII				( ID3_Field *field, char *string )
{
	if	( field )
		field->Add ( string );

	return;
}


CDLLEXPORT
void			ID3Field_SetBINARY				( ID3_Field *field, uchar *data, luint size )
{
	if	( field )
		field->Set ( data, size );

	return;
}


CDLLEXPORT
void			ID3Field_GetBINARY				( ID3_Field *field, uchar *buffer, luint buffLength )
{
	if	( field )
		field->Get ( buffer, buffLength );

	return;
}


CDLLEXPORT
void			ID3Field_FromFile				( ID3_Field *field, char *fileName )
{
	if	( field )
		field->FromFile ( fileName );

	return;
}


CDLLEXPORT
void			ID3Field_ToFile					( ID3_Field *field, char *fileName )
{
	if	( field )
		field->ToFile ( fileName );

	return;
}


#endif


