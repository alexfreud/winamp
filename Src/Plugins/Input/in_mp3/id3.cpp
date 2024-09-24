#include "../id3v2/id3_tag.h"
#include "id3.h"
#include "config.h"
#include "../nu/ns_wc.h"
#include <strsafe.h>
#define _isdigit(x) (( x ) >= '0' && ( x ) <= '9')

/* id3 helper functions */

void SetFrameEncoding(ID3_Frame *frame, int encoding)
{
	switch (encoding)
	{
		case ENCODING_AUTO:
			if (config_write_mode == WRITE_UTF16)
				frame->Field(ID3FN_TEXTENC).Set(ID3TE_UNICODE);
			else
				frame->Field(ID3FN_TEXTENC).Set(ID3TE_ASCII);
			break;
		case ENCODING_FORCE_ASCII:
			frame->Field(ID3FN_TEXTENC).Set(ID3TE_ASCII);
			break;
		case ENCODING_FORCE_UNICODE:
			frame->Field(ID3FN_TEXTENC).Set(ID3TE_UNICODE);
			break;
	}
}

char *ID3_GetString(ID3_Frame *frame, ID3_FieldID fldName, size_t nIndex)
{
	char *text = NULL;
	if (NULL != frame)
	{
		size_t nText = frame->Field(fldName).Size();
		text = (char *)calloc(nText + 1, sizeof(char));
		frame->Field(fldName).GetLocal(text, nText + 1, nIndex);
	}
	return text;
}

wchar_t *ID3_GetUnicodeString(ID3_Frame *frame, ID3_FieldID fldName, size_t nIndex)
{
	wchar_t *text = NULL;
	if (NULL != frame)
	{
		size_t nText = frame->Field(fldName).Size();
		text = (wchar_t *)calloc(sizeof(wchar_t) * (nText + 1), sizeof(wchar_t));
		frame->Field(fldName).GetUnicode(text, nText + 1, nIndex);
	}
	return text;
}

wchar_t *ID3_FillUnicodeString(ID3_Frame *frame, ID3_FieldID fldName, wchar_t *dest, size_t destlen, size_t nIndex)
{
	memset(dest, 0, destlen * sizeof(wchar_t));
	if (NULL != frame)
	{
		frame->Field(fldName).GetUnicode(dest, destlen, nIndex);
		return dest;
	}
	else
		return NULL;
}

wchar_t *ID3_GetTitle(ID3_Tag *tag)
{
	wchar_t*sTitle = NULL;
	if (NULL == tag)
	{
		return sTitle;
	}
	ID3_Frame *frame = tag->Find(ID3FID_TITLE);
	if (frame != NULL)
	{
		sTitle = ID3_GetUnicodeString(frame, ID3FN_TEXT);
	}
	return sTitle;
}

wchar_t *ID3_GetArtist(ID3_Tag *tag)
{
	if (!tag) return 0;
	wchar_t *sArtist = NULL;
	ID3_Frame *frame = NULL;
	if ((frame = tag->Find(ID3FID_LEADARTIST)) || (frame = tag->Find(ID3FID_BAND)))
	{
		sArtist = ID3_GetUnicodeString(frame, ID3FN_TEXT);
	}
	return sArtist;
}

wchar_t *ID3_GetAlbum(ID3_Tag *tag)
{
	wchar_t *sAlbum = NULL;
	if (NULL == tag)
	{
		return sAlbum;
	}
	ID3_Frame *frame = tag->Find(ID3FID_ALBUM);
	if (frame != NULL)
	{
		sAlbum = ID3_GetUnicodeString(frame, ID3FN_TEXT);
	}
	return sAlbum;
}

wchar_t *ID3_GetYear(ID3_Tag *tag)
{
	wchar_t *sYear = NULL;
	if (NULL == tag)
	{
		return sYear;
	}
	ID3_Frame *frame = tag->Find(ID3FID_RECORDINGTIME);
	if (frame != NULL)
		sYear = ID3_GetUnicodeString(frame, ID3FN_TEXT);

	if (!sYear || !*sYear)
	{
		frame = tag->Find(ID3FID_YEAR);
		if (frame != NULL)
			sYear = ID3_GetUnicodeString(frame, ID3FN_TEXT);
	}

	return sYear;
}

void ID3_AddSetComment(ID3_Tag *tag, const wchar_t *comment)
{
	ID3_Frame *frame = tag->Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, L"");
	if (frame)
	{
		if (!comment || !comment[0])
			tag->RemoveFrame(frame);
		else
		{
			SetFrameEncoding(frame);
			frame->Field(ID3FN_TEXT).SetUnicode(comment);
			unsigned char null3[3] = {0, 0, 0};
			frame->Field(ID3FN_LANGUAGE).Get(null3, 3);
			if (!null3[0]) frame->Field(ID3FN_LANGUAGE).SetLatin("eng");
		}
	}
	else if (comment && comment[0])
	{
		frame = new ID3_Frame(ID3FID_COMMENT);
		SetFrameEncoding(frame);
		frame->Field(ID3FN_LANGUAGE).SetLatin("eng");
		//frame->Field(ID3FN_LANGUAGE).Set(null3, 3);
		frame->Field(ID3FN_DESCRIPTION).SetUnicode(L"");
		frame->Field(ID3FN_TEXT).SetUnicode(comment);
		tag->AddFrame(frame, TRUE);
	}
}

void ID3_AddSetRating(ID3_Tag *tag, const wchar_t *rating)
{
	luint rating_integer = 0;
	if (rating)
		rating_integer = _wtoi(rating);

	bool custom_frame = false, own_frame = false;
	ID3_Frame* frame = NULL;
	if (config_rating_email[0])
	{
		frame = tag->Find(ID3FID_POPULARIMETER, ID3FN_EMAIL, config_rating_email);
		if (!frame) custom_frame = true;
	}
	if (!frame)
	{
		frame = tag->Find(ID3FID_POPULARIMETER, ID3FN_EMAIL, "rating@winamp.com\0");
		if (frame) own_frame = true;
	}
	if (!frame)
	{
		frame = tag->Find(ID3FID_POPULARIMETER);
		if (frame) own_frame = true;
	}
	// try to use a custom field if our own was present and the custom wasn't
	if (custom_frame && own_frame)
	{
		frame->Clear();
		frame = NULL;
	}
	if (!frame)
	{
		frame = new ID3_Frame(ID3FID_POPULARIMETER);
		if (!config_rating_email[0])
			frame->Field(ID3FN_EMAIL).Set((uchar *)"rating@winamp.com\0", 18);
		else
		{
			frame->Field(ID3FN_EMAIL).Set((uchar *)config_rating_email, strlen(config_rating_email)+1);
		}
		tag->AddFrame(frame, TRUE);
	}
	if (frame)
	{ 
		switch(rating_integer)
		{
			case 2:
				rating_integer=64;
				break;
			case 3:
				rating_integer=128;
				break;
			case 4:
				rating_integer=196;
				break;
			case 5:
				rating_integer = 255;
				break;
		}

		if (!rating_integer)
			tag->RemoveFrame(frame);
		else
			frame->Field(ID3FN_RATING).Set(rating_integer);
	}
}

wchar_t *ID3_GetComment(ID3_Tag *tag, wchar_t *dest, size_t destlen)
{
	wchar_t *comment = NULL;
	if (NULL == tag)
	{
		return comment;
	}
	ID3_Frame* frame = tag->Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, L"");
	if (frame)
	{
		comment = ID3_FillUnicodeString(frame, ID3FN_TEXT, dest, destlen);
	}
	return comment;
}

wchar_t *ID3_GetRating(ID3_Tag *tag, wchar_t *dest, size_t destlen)
{
	if (NULL == tag)
	{
		return NULL;
	}
	ID3_Frame* frame = NULL;
	if (config_rating_email[0])
		frame = tag->Find(ID3FID_POPULARIMETER, ID3FN_EMAIL, config_rating_email);
	if (!frame)
		frame = tag->Find(ID3FID_POPULARIMETER, ID3FN_EMAIL, "rating@winamp.com\0");
	if (!frame)
		frame = tag->Find(ID3FID_POPULARIMETER);
	if (frame)
	{ 
		int rating = (int)frame->Field(ID3FN_RATING).Get();

		if (rating >= 224 && rating <= 255)
			rating = 5;
		else if (rating >= 160 && rating <= 223)
			rating = 4;
		else if (rating >= 96 && rating <= 159)
			rating = 3;
		else if (rating >= 32 && rating <= 95)
			rating = 2;
		else if (rating >= 1 && rating <= 31)
			rating = 1;
		else
			rating = 0;

		StringCchPrintfW(dest, destlen, L"%u", rating);
		return dest;
	}
	return 0;
}

wchar_t *ID3_GetComment(ID3_Tag *tag, const wchar_t *desc, wchar_t *dest, size_t destlen)
{
	wchar_t *comment = NULL;
	if (NULL == tag)
	{
		return comment;
	}
	ID3_Frame* frame = tag->Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, desc);
	if (frame)
	{
		comment = ID3_FillUnicodeString(frame, ID3FN_TEXT, dest, destlen);
	}
	return comment;
}

wchar_t *ID3_GetMusicbrainzRecordingID(ID3_Tag *tag, wchar_t *dest, size_t destlen)
{
	if (NULL == tag)
	{
		return 0;
	}
	ID3_Frame* frame = tag->Find(ID3FID_UNIQUEFILEID, ID3FN_OWNER, L"http://musicbrainz.org");
	if (frame)
	{
		uchar data[64] = {0};
		luint dataSize = frame->Field(ID3FN_DATA).Size();
			frame->Field(ID3FN_DATA).Get(data, 64);
		int converted = MultiByteToWideCharSZ(CP_ACP, 0, (const char *)data, (int)dataSize, dest, (int)destlen);
		dest[converted]=0;
		return dest;
	}
	return 0;
}

wchar_t *ID3_GetGracenoteTagID(ID3_Tag *tag)
{
	if (NULL == tag)
	{
		return 0;
	}
	ID3_Frame* frame = tag->Find(ID3FID_UNIQUEFILEID, ID3FN_OWNER, L"http://www.cddb.com/id3/taginfo1.html");
	if (frame)
	{
		uchar data[64] = {0};
		luint dataSize = frame->Field(ID3FN_DATA).Size();
		frame->Field(ID3FN_DATA).Get(data, 64);
		int converted = MultiByteToWideChar(CP_ACP, 0, (const char *)data, (int)dataSize, 0, 0);
		wchar_t *dest = (wchar_t *)calloc((converted+1), sizeof(wchar_t));
		converted = MultiByteToWideChar(CP_ACP, 0, (const char *)data, (int)dataSize, dest, converted);
		dest[converted]=0;
		return dest;
	}
	return 0;
}

wchar_t *ID3_GetGracenoteTagID(ID3_Tag *tag, wchar_t *dest, size_t destlen)
{
	if (NULL == tag)
	{
		return 0;
	}
	ID3_Frame* frame = tag->Find(ID3FID_UNIQUEFILEID, ID3FN_OWNER, L"http://www.cddb.com/id3/taginfo1.html");
	if (frame)
	{
		uchar data[64] = {0};
		luint dataSize = frame->Field(ID3FN_DATA).Size();
			frame->Field(ID3FN_DATA).Get(data, 64);
		int converted = MultiByteToWideCharSZ(CP_ACP, 0, (const char *)data, (int)dataSize, dest, (int)destlen);
		dest[converted]=0;
		return dest;
	}
	return 0;
}

void ID3_AddSetGracenoteTagID(ID3_Tag *tag, const wchar_t *tagID)
{
	ID3_Frame *frame = tag->Find(ID3FID_UNIQUEFILEID, ID3FN_OWNER, L"http://www.cddb.com/id3/taginfo1.html");
	if (frame)
	{
		if (!tagID || !tagID[0])
			tag->RemoveFrame(frame);
		else
		{
			size_t origLen = wcslen(tagID); // so we can not write the null terminator
			uchar data[64] = {0};
			luint dataSize = WideCharToMultiByte(CP_ACP, 0, tagID, (int)origLen, (char *)data, 64, 0, 0);
			frame->Field(ID3FN_DATA).Set(data, dataSize);
		}
	}
	else if (tagID && tagID[0])
	{
		frame = new ID3_Frame(ID3FID_UNIQUEFILEID);
		SetFrameEncoding(frame, ENCODING_FORCE_ASCII);
		frame->Field(ID3FN_OWNER).SetLatin("http://www.cddb.com/id3/taginfo1.html");
		size_t origLen = wcslen(tagID); // so we can not write the null terminator
		uchar data[64] = {0};
		luint dataSize = WideCharToMultiByte(CP_ACP, 0, tagID, (int)origLen, (char *)data, 64, 0, 0);
		frame->Field(ID3FN_DATA).Set(data, dataSize);		
		tag->AddFrame(frame, TRUE);
	}
}

#if 0 // benski> CUT
char *ID3_GetTUID(ID3_Tag *tag)
{
	char *tuid = NULL;
	if (NULL == tag)
	{
		return tuid;
	}
	ID3_Frame* frame = NULL;
	frame = tag->Find(ID3FID_UNIQUEFILEID);
	if (frame)
	{
		char *tmp = ID3_GetString(frame, ID3FN_DATA);
		if (tmp)
		{
			// verify first four characters are '3CD3'
			if (!strncmp(tmp, "3CD3", 4))
			{
				char m, n;
				char *p = tmp + 4;
				n = *p++;
				m = 'P' - n;
				p += m;

				n = *p++;
				m = 'Z' - n;    // length of TUID;
				tuid = _strdup(p);
				tuid[m] = 0;   // null terminate
			}

			free(tmp);
		}
	}
	return tuid;
}
#endif

char *ID3_GetGenre(ID3_Tag *tag)
{
	char *sGenre = NULL;
	if (NULL == tag)
	{
		return sGenre;
	}
	ID3_Frame *frame = tag->Find(ID3FID_CONTENTTYPE);
	if (frame != NULL)
	{
		sGenre = ID3_GetString(frame, ID3FN_TEXT);
	}
	return sGenre;
}

void ID3_AddUserText(ID3_Tag *tag, wchar_t *desc, const wchar_t *value, int encoding)
{
	ID3_Frame *frame = tag->Find(ID3FID_USERTEXT, ID3FN_DESCRIPTION, desc);
	if (frame)
	{
		if (!value || !value[0])
			tag->RemoveFrame(frame);
		else
		{
			SetFrameEncoding(frame, encoding);
			frame->Field(ID3FN_TEXT).SetUnicode(value);
		}
	}
	else if (value && value[0])
	{
		frame = new ID3_Frame(ID3FID_USERTEXT);
		SetFrameEncoding(frame, encoding);
		frame->Field(ID3FN_DESCRIPTION).SetUnicode(desc);
		frame->Field(ID3FN_TEXT).SetUnicode(value);
		tag->AddFrame(frame, TRUE);
	}
}

wchar_t *ID3_GetUserText(ID3_Tag *tag, wchar_t *desc)
{
	if (tag == NULL)
		return NULL;

	ID3_Frame *frame = tag->Find(ID3FID_USERTEXT, ID3FN_DESCRIPTION, desc);
	if (frame)
		return ID3_GetUnicodeString(frame, ID3FN_TEXT);
	else
		return 0;
}

wchar_t *ID3_GetUserText(ID3_Tag *tag,  wchar_t *desc, wchar_t *dest, size_t destlen)
{
	if (tag == NULL)
		return NULL;

	ID3_Frame *frame = tag->Find(ID3FID_USERTEXT, ID3FN_DESCRIPTION, desc);
	if (frame)
		return ID3_FillUnicodeString(frame, ID3FN_TEXT, dest, destlen);
	else
		return 0;
}

wchar_t *ID3_GetTagText(ID3_Tag *tag, ID3_FrameID f)
{
	wchar_t *sComposer = NULL;
	if (NULL == tag)
	{
		return sComposer;
	}
	ID3_Frame *frame = tag->Find(f);
	if (frame != NULL)
	{
		sComposer = ID3_GetUnicodeString(frame, ID3FN_TEXT);
	}
	return sComposer;
}

wchar_t *ID3_GetTagText(ID3_Tag *tag, ID3_FrameID f, wchar_t *dest, size_t destlen)
{
	wchar_t *sComposer = NULL;
	if (NULL == tag)
	{
		return sComposer;
	}
	ID3_Frame *frame = tag->Find(f);
	if (frame != NULL)
	{
		sComposer = ID3_FillUnicodeString(frame, ID3FN_TEXT, dest, destlen);
	}
	return sComposer;
}

wchar_t *ID3_GetTagUrl(ID3_Tag *tag, ID3_FrameID f, wchar_t *dest, size_t destlen)
{
	wchar_t *sComposer = NULL;
	if (NULL == tag)
	{
		return sComposer;
	}
	ID3_Frame *frame = tag->Find(f);
	if (frame != NULL)
	{
		sComposer = ID3_FillUnicodeString(frame, ID3FN_URL, dest, destlen);
	}
	return sComposer;
}

#if 0
char *ID3_GetGenreDisplayable(ID3_Tag *tag)
{
	char *sGenre = ID3_GetGenre(tag);
	if (!sGenre) return NULL;

	while (sGenre && *sGenre == ' ') sGenre++;

	if (sGenre[0] == '(' || _isdigit(sGenre[0]))
	{
		int isparam = !_isdigit(sGenre[0]);
		char *pCur = &sGenre[isparam];
		int cnt = 0;
		while (_isdigit(*pCur))
		{
			cnt++;
			pCur++;
		}
		while (pCur && *pCur == ' ') pCur++;

		if (cnt > 0 && (isparam && *pCur == ')') || (!isparam && !*pCur))
		{
			// if the genre number is greater than 255, its invalid.
			size_t ulGenre = atoi(&sGenre[isparam]);
			if (ulGenre >= 0 && ulGenre < numberOfGenres)
			{
				char *tmp = (char*)malloc(strlen(genres[ulGenre]) + 1);
				if (tmp)
				{
					memcpy(tmp, genres[ulGenre], strlen(genres[ulGenre]) + 1);
					free(sGenre);
					sGenre = tmp;
				}
			}
		}
	}
	return sGenre;
}
#endif