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


#include "id3_misc_support.h"


int ID3_UnicodeToLatin(char *latin, const wchar_t *unicode, luint len, int inLen)
{
	if	(unicode && latin && len)
		return WideCharToMultiByte(28591, 0, unicode, inLen/*-1*/, latin, len, NULL, NULL) - 1;
	else if (latin && len)
		latin[0]=0;
	return 0;
}

int ID3_UnicodeToLocal(char *local, const wchar_t *unicode, luint len, int inLen)
{
	if	(unicode && local && len)
		return WideCharToMultiByte(CP_ACP, 0, unicode, inLen/*-1*/, local, len, NULL, NULL)-1;
	else if (local && len)
		local[0]=0;
	return 0;
}

int ID3_UnicodeToUTF8(char *local, const wchar_t *unicode, luint len, int inLen)
{
	if	(unicode && local && len)
		return WideCharToMultiByte(CP_UTF8, 0, unicode, inLen/*-1*/, local, len, NULL, NULL)-1;
	else if (local && len)
		local[0]=0;
	return 0;
}

void ID3_AddArtist(ID3_Tag *tag, char *text)
{
	if	(tag->Find(ID3FID_LEADARTIST ) == NULL &&
		tag->Find (ID3FID_BAND ) == NULL &&
		tag->Find (ID3FID_CONDUCTOR ) == NULL &&
		tag->Find (ID3FID_COMPOSER ) == NULL &&
		text && text[0])
	{
		ID3_Frame	*artistFrame=new ID3_Frame;
		artistFrame->SetID(ID3FID_LEADARTIST);
		artistFrame->Field(ID3FN_TEXT).SetLocal(text);
		tag->AddFrame(artistFrame, true);
	}

	return;
}

void ID3_AddArtist_Latin(ID3_Tag *tag, char *text)
{
	if	(tag->Find (ID3FID_LEADARTIST ) == NULL &&
		tag->Find (ID3FID_BAND ) == NULL &&
		tag->Find (ID3FID_CONDUCTOR ) == NULL &&
		tag->Find (ID3FID_COMPOSER ) == NULL &&
		text && text[0])
	{
		ID3_Frame	*artistFrame= new ID3_Frame;

		artistFrame->SetID (ID3FID_LEADARTIST);
		artistFrame->Field(ID3FN_TEXT).SetLatin(text);
		tag->AddFrame (artistFrame, true );

	}

	return;
}



void ID3_AddAlbum(ID3_Tag *tag, char *text)
{
	if	(tag->Find (ID3FID_ALBUM ) == NULL && strlen (text ) > 0 )
	{
		ID3_Frame	*albumFrame;

		if	(albumFrame = new ID3_Frame )
		{
			albumFrame->SetID (ID3FID_ALBUM );

			albumFrame->Field(ID3FN_TEXT).SetLocal(text);
			tag->AddFrame (albumFrame, true );
		}
		else
			ID3_THROW (ID3E_NoMemory );
	}

	return;
}

void ID3_AddAlbum_Latin(ID3_Tag *tag, char *text)
{
	if	(tag->Find(ID3FID_ALBUM) == NULL 
		&& text && text[0])
	{
		ID3_Frame	*albumFrame = new ID3_Frame;
		
			albumFrame->SetID (ID3FID_ALBUM );

			albumFrame->Field(ID3FN_TEXT).SetLocal(text);
			tag->AddFrame (albumFrame, true );
		
	}

	return;
}

void ID3_AddTitle(ID3_Tag *tag, char *text)
{
	if (tag->Find(ID3FID_TITLE) == NULL 
		&& text && text[0])
	{
		ID3_Frame	*titleFrame;

		if	(titleFrame = new ID3_Frame)
		{
			titleFrame->SetID(ID3FID_TITLE);
			titleFrame->Field(ID3FN_TEXT).SetLocal(text);
			tag->AddFrame (titleFrame, true );
		}
		else
			ID3_THROW(ID3E_NoMemory);
	}

	return;
}

void ID3_AddTitle_Latin(ID3_Tag *tag, char *text)
{
	if (tag->Find(ID3FID_TITLE) == NULL 
		&& text && text[0])
	{
		ID3_Frame	*titleFrame= new ID3_Frame;

		titleFrame->SetID(ID3FID_TITLE);
		titleFrame->Field(ID3FN_TEXT).SetLatin(text);
		tag->AddFrame(titleFrame, true);
	}

	return;
}


void ID3_AddLyrics(ID3_Tag *tag, char *text)
{
	if (tag->Find (ID3FID_UNSYNCEDLYRICS ) == NULL && strlen (text ) > 0 )
	{
		ID3_Frame	*lyricsFrame;

		if	(lyricsFrame = new ID3_Frame )
		{
			lyricsFrame->SetID (ID3FID_UNSYNCEDLYRICS );
			lyricsFrame->Field(ID3FN_LANGUAGE).SetLatin("eng");
			lyricsFrame->Field(ID3FN_TEXT ).SetLocal(text);
			tag->AddFrame (lyricsFrame, true );
		}
		else
			ID3_THROW (ID3E_NoMemory );
	}

	return;
}


