#ifndef NULLSOFT_IN_MP3_IN_H
#define NULLSOFT_IN_MP3_IN_H

extern char *genres[];
extern size_t numberOfGenres;
enum
{
	ENCODING_AUTO=0,
	ENCODING_FORCE_ASCII = 1,
	ENCODING_FORCE_UNICODE = 2,
};

char *ID3_GetString(ID3_Frame *frame, ID3_FieldID fldName, size_t nIndex=1);
wchar_t *ID3_GetUnicodeString(ID3_Frame *frame, ID3_FieldID fldName, size_t nIndex=1);
wchar_t *ID3_FillUnicodeString(ID3_Frame *frame, ID3_FieldID fldName, wchar_t *dest, size_t destlen, size_t nIndex=1);

wchar_t *ID3_GetTitle(ID3_Tag *tag);
wchar_t *ID3_GetArtist(ID3_Tag *tag);
//char *ID3_GetAlbumLocal(ID3_Tag *tag);
wchar_t *ID3_GetAlbum(ID3_Tag *tag);
wchar_t *ID3_GetYear(ID3_Tag *tag);
wchar_t *ID3_GetComment(ID3_Tag *tag, wchar_t *dest, size_t destlen);
wchar_t *ID3_GetComment(ID3_Tag *tag, const wchar_t *desc, wchar_t *dest, size_t destlen);
char *ID3_GetTUID(ID3_Tag *tag);
char *ID3_GetGenre(ID3_Tag *tag);
wchar_t *ID3_GetTagText(ID3_Tag *tag, ID3_FrameID f);
wchar_t *ID3_GetTagText(ID3_Tag *tag, ID3_FrameID f, wchar_t *dest, size_t destlen);
wchar_t *ID3_GetTagUrl(ID3_Tag *tag, ID3_FrameID f, wchar_t *dest, size_t destlen);
char *ID3_GetGenreDisplayable(ID3_Tag *tag);
wchar_t *ID3_GetUserText(ID3_Tag *tag,  wchar_t *desc);
wchar_t *ID3_GetUserText(ID3_Tag *tag,  wchar_t *desc, wchar_t *dest, size_t destlen);
void ID3_AddUserText(ID3_Tag *tag, wchar_t *desc, const wchar_t *value, int encoding=ENCODING_AUTO);
void ID3_AddSetComment(ID3_Tag *tag, const wchar_t *comment);
void ID3_AddSetRating(ID3_Tag *tag, const wchar_t *rating);
wchar_t *ID3_GetRating(ID3_Tag *tag, wchar_t *dest, size_t destlen);
wchar_t *ID3_GetMusicbrainzRecordingID(ID3_Tag *tag, wchar_t *dest, size_t destlen);
wchar_t *ID3_GetGracenoteTagID(ID3_Tag *tag);
wchar_t *ID3_GetGracenoteTagID(ID3_Tag *tag, wchar_t *dest, size_t destlen);
void ID3_AddSetGracenoteTagID(ID3_Tag *tag, const wchar_t *tagID);

void SetFrameEncoding(ID3_Frame *frame, int encoding = ENCODING_AUTO);

#endif
