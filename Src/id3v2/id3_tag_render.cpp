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


#include <string.h>
#include <memory.h>
#include "id3_tag.h"
#include "id3_misc_support.h"
#include <windows.h>



luint ID3_Tag::Render( uchar *buffer )
{
	luint	bytesUsed	= 0;

	ID3_Elem		*cur	= frameList;
	ID3_TagHeader	header;

	if (version > ID3_TAGVERSION || (version == ID3_TAGVERSION && revision > ID3_TAGREVISION))
		SetVersion(version, revision);
	else
		SetVersion ( ID3_TAGVERSION, ID3_TAGREVISION );

	header.SetVersion ( version, revision );
	bytesUsed += header.Size();

	// set up the encryption and grouping IDs

	while	(cur)
	{
		if	(cur->frame)
		{
			cur->frame->compression = compression;
			cur->frame->SetVersion ( version, revision );
			bytesUsed += cur->frame->Render(&buffer[bytesUsed]);
		}

		cur = cur->next;
	}

	if (syncOn)
	{
		luint	newTagSize;

		newTagSize = GetUnSyncSize ( &buffer[ header.Size() ], bytesUsed - header.Size() );

		if	( newTagSize > 0 && ( newTagSize + header.Size() ) > bytesUsed )
		{
			uchar *tempz;
			if	( tempz = (uchar*)calloc(newTagSize, sizeof(uchar)) )
			{
				UnSync ( tempz, newTagSize, &buffer[ header.Size() ], bytesUsed - header.Size() );
				header.SetFlags ( ID3HF_UNSYNC );

				memcpy ( &buffer[ header.Size() ], tempz, newTagSize );
				bytesUsed = newTagSize + header.Size();
				free(tempz);
			}
			else
				ID3_THROW ( ID3E_NoMemory );
		}
	}

	// zero the remainder of the buffer so that our
	// padding bytes are zero
	luint paddingSize = PaddingSize(bytesUsed);
	for	( luint i = 0; i < paddingSize; i++ )
		buffer[ bytesUsed + i ] = 0;

	bytesUsed += paddingSize;

	header.SetDataSize ( bytesUsed - header.Size() );
	header.Render ( buffer );

	// set the flag which says that the tag hasn't changed
	hasChanged = false;

	return bytesUsed;
}



luint ID3_Tag::Size( void )
{
	luint			bytesUsed	= 0;
	ID3_Elem		*cur		= frameList;
	ID3_TagHeader	header;

	header.SetVersion(version, revision);
	bytesUsed += header.Size();

	while	(cur)
	{
		if	(cur->frame)
		{
			cur->frame->SetVersion ( version, revision );
			bytesUsed += cur->frame->Size();
		}

		cur = cur->next;
	}

	// add 30% for sync
	if	(syncOn)
		bytesUsed += bytesUsed / 3;

	bytesUsed += PaddingSize(bytesUsed);

	return bytesUsed;
}


void			ID3_Tag::RenderExtHeader		( uchar *buffer )
{
	if	( version == 3 && revision == 0 )
	{
	}

	return;
}

#define	ID3_PADMULTIPLE		( 2048 )
#define	ID3_PADMAX			( 4096 )

luint	ID3_Tag::PaddingSize(luint curSize)
{
	luint	newSize	= 0;

	// if padding is switched off
	if	(!padding)
		return 0;

	if (forcedPadding)
		return forcedPadding;
	else
		newSize = ( ( curSize / ID3_PADMULTIPLE ) + 1 ) * ID3_PADMULTIPLE;

	return newSize - curSize;
}


