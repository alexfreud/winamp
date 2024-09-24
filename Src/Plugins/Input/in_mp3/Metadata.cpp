#include "Metadata.h"
#include "main.h"
#include "api__in_mp3.h"
#include "LAMEInfo.h"
#include "AACFrame.h"
#include "config.h"
#include "LAMEInfo.h"
#include <shlwapi.h>
#include <assert.h>
#include <foundation/error.h>
#include <strsafe.h>

#define INFO_READ_SIZE 32768

Metadata::Metadata( CGioFile *_file, const wchar_t *_filename )
{
	if ( !PathIsURL( _filename ) )
		filename = _wcsdup( _filename );

	ReadTags( _file );
	if ( bitrate = _file->GetAvgVBRBitrate() * 1000 )
	{
		length_ms = _file->m_vbr_ms;
		vbr       = _file->m_vbr_flag || _file->m_vbr_hdr;
	}
}

void GetFileDescription(const wchar_t *file, CGioFile &_file, wchar_t *data, size_t datalen);
void GetAudioInfo(const wchar_t *filename, CGioFile *file, int *len, int *channels, int *bitrate, int *vbr, int *sr);

int Metadata::Open(const wchar_t *_filename)
{
	if ( filename && *filename )
		free( filename );

	filename = _wcsdup(_filename);
	if (file.Open(filename, INFO_READ_SIZE/1024) != NErr_Success)
		return 1;

	GetAudioInfo(filename, &file, &length_ms, &channels, &bitrate, &vbr, &sampleRate);
	ReadTags(&file);
	file.Close();
	return METADATA_SUCCESS;
}

Metadata::~Metadata()
{
	if (filename)
	{
		free(filename);
		filename=0;
	}
}

void Metadata::ReadTags(CGioFile *_file)
{
	// Process ID3v1
	if (config_parse_id3v1)
	{
		void *id3v1_data = _file->GetID3v1();
		if (id3v1_data)
			id3v1.Decode(id3v1_data);
	}

	if (config_parse_id3v2)
	{
		uint32_t len = 0;
		void *id3v2_data = _file->GetID3v2(&len);
		if (id3v2_data)
			id3v2.Decode(id3v2_data, len);
	}

	if (config_parse_lyrics3)
	{
		uint32_t len = 0;
		void *lyrics3_data = _file->GetLyrics3(&len);
		if (lyrics3_data)
			lyrics3.Decode(lyrics3_data, len);
	}

	if (config_parse_apev2)
	{
		uint32_t len = 0;
		void *apev2_data = _file->GetAPEv2(&len);
		if (apev2_data)
			apev2.Decode(apev2_data, len);
	}
}

static int ID3Write(const wchar_t *filename, HANDLE infile, DWORD offset, void *data, DWORD len)
{
	wchar_t tempFile[MAX_PATH] = {0};
	StringCchCopyW(tempFile, MAX_PATH, filename);
	PathRemoveExtension(tempFile);
	StringCchCatW(tempFile, MAX_PATH, L".tmp");

	// check to make sure the filename was actually different!
	// benski> TODO: we should just try to mangle the filename more rather than totally bail out
	if (!_wcsicmp(tempFile, filename))
		return SAVE_ERROR_CANT_OPEN_TEMPFILE;

	// TODO: overlapped I/O
	HANDLE outfile = CreateFile(tempFile, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if (outfile != INVALID_HANDLE_VALUE)
	{
		DWORD written=0;
		if (data && len)
			WriteFile(outfile, data, len, &written, NULL);
		SetFilePointer(infile, offset, 0, FILE_BEGIN);

		DWORD read=0;
		do
		{
			char data[4096] = {0};
			written = read = 0;
			ReadFile(infile, data, 4096, &read, NULL);
			if (read) WriteFile(outfile, data, read, &written, NULL);
		}
		while (read != 0);
		CloseHandle(outfile);
		CloseHandle(infile);
		if (!MoveFile(tempFile, filename))
		{
			if (!CopyFile(tempFile, filename, FALSE))
			{
				DeleteFile(tempFile);
				return SAVE_ERROR_ERROR_OVERWRITING;
			}
			DeleteFile(tempFile);
		}
		return SAVE_SUCCESS;
	}
	return SAVE_ERROR_CANT_OPEN_TEMPFILE;

}

bool Metadata::IsDirty()
{
	return id3v1.IsDirty() || id3v2.IsDirty() || lyrics3.IsDirty() || apev2.IsDirty();
}

int Metadata::Save()
{
	if (!IsDirty())
		return SAVE_SUCCESS;

	int err=SAVE_SUCCESS;
	if (GetFileAttributes(filename)&FILE_ATTRIBUTE_READONLY)
		return SAVE_ERROR_READONLY;

	HANDLE metadataFile = CreateFile(filename, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if (metadataFile == INVALID_HANDLE_VALUE)
		return SAVE_ERROR_OPENING_FILE;

	if (file.Open(filename, INFO_READ_SIZE/1024) != NErr_Success)
	{
		CloseHandle(metadataFile);
		return SAVE_ERROR_OPENING_FILE;
	}

	bool strippedID3v1=false; // this flag will get set to true when we remove ID3v1 as a side effect of removing APEv2 or Lyrics3 (or ID3v2.4 end-tag if/when we implement)
	bool strippedLyrics3=false;

	/* Strip APEv2 */
	if (config_parse_apev2 && config_write_apev2 && apev2.IsDirty())
	{
		uint32_t len = 0;
		void *apev2_data = file.GetAPEv2(&len);
		if (apev2_data)
		{
			uint32_t lyrics3_len = 0;
			void *lyrics3_data = file.GetLyrics3(&lyrics3_len);
			if (lyrics3_data)
				SetFilePointer(metadataFile, -(LONG)(len + 15 + lyrics3_len + (file.GetID3v1()?128:0)), NULL, FILE_END);
			else
				SetFilePointer(metadataFile, -(LONG)(len + (file.GetID3v1()?128:0)), NULL, FILE_END);
			SetEndOfFile(metadataFile);
			strippedLyrics3=true;
			strippedID3v1=true;
		}
	}

	/* Strip Lyrics3 tag */
	if (!strippedLyrics3 && config_parse_lyrics3 && lyrics3.IsDirty())
	{
		uint32_t len = 0;
		void *lyrics3_data = file.GetLyrics3(&len);
		if (lyrics3_data)
		{
			SetFilePointer(metadataFile, -(LONG)(len + 15 + (file.GetID3v1()?128:0)), NULL, FILE_END);
			SetEndOfFile(metadataFile);
			strippedID3v1=true;
		}
	}

	/* Strip ID3v1(.1) tag */
	if (!strippedID3v1  /* if we stripped lyrics3 tag, then we ended up stripping id3v1 also */
	    && config_parse_id3v1 && config_write_id3v1 && id3v1.IsDirty())
	{
		if (file.GetID3v1()) // see if we have ID3v1
		{
			SetFilePointer(metadataFile, -128, NULL, FILE_END);
			SetEndOfFile(metadataFile);
		}
	}

	/* Write APEv2 */
	if (config_parse_apev2 && config_write_apev2 && apev2.IsDirty() && apev2.HasData())
	{
		switch(config_apev2_header)
		{
			case ADD_HEADER:
				apev2.SetFlags(APEv2::FLAG_HEADER_HAS_HEADER, APEv2::FLAG_HEADER_HAS_HEADER);
				break;
			case REMOVE_HEADER:
				apev2.SetFlags(0, APEv2::FLAG_HEADER_HAS_HEADER);
				break;
		}

		size_t apev2_len = apev2.EncodeSize();
		void *apev2_data = malloc(apev2_len);
		if (apev2_data && apev2.Encode(apev2_data, apev2_len) == APEv2::APEV2_SUCCESS)
		{
			SetFilePointer(metadataFile, 0, NULL, FILE_END);
			DWORD bytesWritten=0;
			WriteFile(metadataFile, apev2_data, (DWORD)apev2_len, &bytesWritten, 0);
			free(apev2_data);
			apev2_data = 0;
			if (bytesWritten != apev2_len)
			{
				err=SAVE_APEV2_WRITE_ERROR;
				goto fail;
			}
		}
		else
		{
			free(apev2_data);
			apev2_data = 0;
			err=SAVE_APEV2_WRITE_ERROR;
			goto fail;
		}
	}

	/* Write Lyrics3 */
	if (strippedLyrics3) /* if we need to rewrite it because we stripped it (e.g. removing an APEv2 tag)*/
	{
		/* since we don't modify lyrics3 (yet) we'll just rewrite the original binary data */
		uint32_t len = 0;
		void *lyrics3_data = file.GetLyrics3(&len);
		if (lyrics3_data)
		{
			SetFilePointer(metadataFile, 0, NULL, FILE_END);
			DWORD bytesWritten=0;
			WriteFile(metadataFile, lyrics3_data, len, &bytesWritten, NULL);
			if (bytesWritten != len)
			{
				err=SAVE_LYRICS3_WRITE_ERROR;
				goto fail;
			}
			char temp[7] = {0};
			StringCchPrintfA(temp, 7, "%06u", len);
			bytesWritten = 0; 
			WriteFile(metadataFile, temp, 6, &bytesWritten, NULL);
			if (bytesWritten != 6)
			{
				err=SAVE_LYRICS3_WRITE_ERROR;
				goto fail;
			}
			bytesWritten = 0; 
			WriteFile(metadataFile, "LYRICS200", 9, &bytesWritten, NULL);
			if (bytesWritten != 9)
			{
				err=SAVE_LYRICS3_WRITE_ERROR;
				goto fail;
			}
		}
	}

	/* Write ID3v1 */
	if (config_parse_id3v1 && config_write_id3v1 && id3v1.IsDirty())
	{
		uint8_t id3v1_data[128] = {0};
		if (id3v1.Encode(id3v1_data) == METADATA_SUCCESS)
		{
			SetFilePointer(metadataFile, 0, NULL, FILE_END);
			DWORD bytesWritten=0;
			WriteFile(metadataFile, id3v1_data, 128, &bytesWritten, NULL);
			if (bytesWritten != 128)
			{
				err=SAVE_ID3V1_WRITE_ERROR;
				goto fail;
			}
		}
	}
	else if (strippedID3v1)
	{
		/** if we stripped lyrics3 or apev2 but didn't modify id3v1 (or are configured not to use it),
		 ** we need to rewrite it back to the original data
		 **/
		void *id3v1_data=file.GetID3v1();
		if (id3v1_data)
		{
			SetFilePointer(metadataFile, 0, NULL, FILE_END);
			DWORD bytesWritten=0;
			WriteFile(metadataFile, id3v1_data, 128, &bytesWritten, NULL);
			if (bytesWritten != 128)
			{
				err=SAVE_ID3V1_WRITE_ERROR;
				goto fail;
			}
		}
	}

	/* Write ID3v2 */
	if (config_parse_id3v2 && config_write_id3v2 && id3v2.IsDirty())
	{
		uint32_t oldlen=0;
		void *old_id3v2_data = file.GetID3v2(&oldlen);
		id3v2.id3v2.SetPadding(false); // turn off padding to see if we can get away with non re-writing the file
		uint32_t newlen = id3v2.EncodeSize();
		if (old_id3v2_data && !newlen) // there's an old tag, but no new tag
		{
			err = ID3Write(filename, metadataFile, oldlen, 0, 0);
			if (err == SAVE_SUCCESS)
				metadataFile = INVALID_HANDLE_VALUE; // ID3Write returns true if it closed the handle
			else
				goto fail;
		}
		else if (!old_id3v2_data && !newlen) // no old tag, no new tag.. easy :)
		{
		}
		else
		{
			id3v2.id3v2.SetPadding(true);
			if (newlen <= oldlen) // if we can fit in the old tag
			{
				if (oldlen != newlen)
					id3v2.id3v2.ForcePading(oldlen-newlen); // pad out the rest of the tag
				else
					id3v2.id3v2.SetPadding(false);
				assert(id3v2.EncodeSize() == oldlen);
				newlen = oldlen;
				uint8_t *new_id3v2_data = (uint8_t *)calloc(newlen, sizeof(uint8_t));
				if (new_id3v2_data && id3v2.Encode(new_id3v2_data, newlen) == METADATA_SUCCESS)
				{
					// TODO: deal with files with multiple starting id3v2 tags
					SetFilePointer(metadataFile, 0, NULL, FILE_BEGIN);
					DWORD bytesWritten=0;
					WriteFile(metadataFile, new_id3v2_data, newlen, &bytesWritten, NULL);
					free(new_id3v2_data);
					new_id3v2_data = 0;
					if (bytesWritten != newlen)
					{
						err = SAVE_ID3V2_WRITE_ERROR;
						goto fail;
					}
				}
				else
				{
					free(new_id3v2_data);
					new_id3v2_data = 0;
					err = SAVE_ID3V2_WRITE_ERROR;
					goto fail;
				}
			}
			else // otherwise we have to pad out the start
			{
				newlen = id3v2.EncodeSize();
				uint8_t *new_id3v2_data = (uint8_t *)calloc(newlen, sizeof(uint8_t));
				if (new_id3v2_data && id3v2.Encode(new_id3v2_data, newlen) == METADATA_SUCCESS)
				{
					// TODO: deal with files with multiple starting id3v2 tags
					SetFilePointer(metadataFile, 0, NULL, FILE_BEGIN);
					DWORD bytesWritten=0;
					err = ID3Write(filename, metadataFile, oldlen, new_id3v2_data, newlen);
					free(new_id3v2_data);
					new_id3v2_data = 0;
					if (err == SAVE_SUCCESS)
						metadataFile = INVALID_HANDLE_VALUE; // ID3Write returns true if it closed the handle
					else
						goto fail;
				}
				else
				{
					free(new_id3v2_data);
					new_id3v2_data = 0;
					err = SAVE_ID3V2_WRITE_ERROR;
					goto fail;
				}
			}
		}
	}

fail:
	file.Close();
	if (metadataFile != INVALID_HANDLE_VALUE)
		CloseHandle(metadataFile);
	return err;
}

int Metadata::GetExtendedData(const char *tag, wchar_t *data, int dataLen)
{
	int understood=0;
	switch (id3v2.GetString(tag, data, dataLen))
	{
		case -1:
			data[0]=0;
			understood=1;
			break;

		case 1:
			return 1;
	}

	switch (apev2.GetString(tag, data, dataLen))
	{
		case -1:
			data[0]=0;
			understood=1;
			break;

		case 1:
			return 1;
	}

	switch (lyrics3.GetString(tag, data, dataLen))
	{
		case -1:
			data[0]=0;
			understood=1;
			break;

		case 1:
			return 1;
	}

	switch (id3v1.GetString(tag, data, dataLen))
	{
		case -1:
			data[0]=0;
			understood=1;
			break;

		case 1:
			return 1;
	}

	switch (GetString(tag, data, dataLen))
	{
		case -1:
			data[0]=0;
			understood=1;
			break;

		case 1:
			return 1;
	}

	return understood;
}

int Metadata::SetExtendedData(const char *tag, const wchar_t *data)
{
	int understood=0;
	if (config_create_id3v2 || id3v2.HasData())
		understood |= id3v2.SetString(tag, data);
	if (config_create_apev2 || apev2.HasData()) 
		understood |= apev2.SetString(tag, data);
	if (config_create_id3v1 || id3v1.HasData()) 
		understood |= id3v1.SetString(tag, data);
	return understood;
}

int Metadata::GetString(const char *tag, wchar_t *data, int dataLen)
{
	if (!_stricmp(tag, "formatinformation"))
	{
		data[0]=0;
		if (filename)
		{
			if (file.Open(filename, INFO_READ_SIZE/1024) == NErr_Success)
				GetFileDescription(filename, file, data, dataLen);
			file.Close();
		}
	}
	else if (!_stricmp(tag, "length"))
	{
		StringCchPrintfW(data, dataLen, L"%d", length_ms);
	}
	else if (!_stricmp(tag, "stereo"))
	{
		StringCchPrintfW(data, dataLen, L"%d", channels==2);
	}
	else if (!_stricmp(tag, "vbr"))
	{
		StringCchPrintfW(data, dataLen, L"%d", vbr);
	}
	else if (!_stricmp(tag, "bitrate"))
	{
		StringCchPrintfW(data, dataLen, L"%d", bitrate/1000);
	}
	else if (!_stricmp(tag, "gain"))
	{
		StringCchPrintfW(data, dataLen, L"%-+.2f dB", file.GetGain());
	}
	else if (!_stricmp(tag, "pregap"))
	{
		if (file.prepad)
		{
			StringCchPrintfW(data, dataLen, L"%u", file.prepad);
			return 1;
		}
		return -1;
	}
	else if (!_stricmp(tag, "postgap"))
	{
		if (file.prepad) // yes, we check for this because postpad could legitimately be 0
		{
			StringCchPrintfW(data, dataLen, L"%u", file.postpad);
			return 1;
		}
		return -1;
	}
	else if (!_stricmp(tag, "numsamples"))
	{
		if (file.m_vbr_samples)
		{
			StringCchPrintfW(data, dataLen, L"%I64u", file.m_vbr_samples);
			return 1;
		}
		return -1;
	}
	else if (!_stricmp(tag, "endoffset"))
	{
		if (file.m_vbr_frames)
		{
			int totalFrames = file.m_vbr_frames;
			if (totalFrames > 8)
			{
				int seekPoint = 0;
				// we're using m_vbr_bytes here instead of file.ContentLength(), because we're already trusting the other LAME header info
				#define MAX_SIZE_8_FRAMES (1448 * 8) // mp3 frames won't be ever be any bigger than this (320kbps 32000Hz + padding)
				if (file.m_vbr_bytes > MAX_SIZE_8_FRAMES)
					seekPoint = (int)(file.m_vbr_bytes - MAX_SIZE_8_FRAMES);
				else
					seekPoint = 0;

				size_t offsets[8] = {0};
				size_t offsetsRead = 0;
				size_t offsetPosition = 0;

				unsigned char header[6] = {0};
				MPEGFrame frame;

				if (file.Open(filename, INFO_READ_SIZE/1024) != NErr_Success)
					return -1;

				// first we need to sync
				while (1)
				{
					file.SetCurrentPosition(seekPoint, CGioFile::GIO_FILE_BEGIN);
					int read = 0;
					file.Read(header, 6, &read);
					if (read != 6)
						break;
					frame.ReadBuffer(header);
					if (frame.IsSync() && frame.GetLayer() == 3)
					{
						// make sure this isn't false sync - see if we can get another sync...
						int nextPoint = seekPoint + frame.FrameSize();
						file.SetCurrentPosition(nextPoint, CGioFile::GIO_FILE_BEGIN);
						file.Read(header, 6, &read);
						if (read != 6) // must be EOF
							break;
						frame.ReadBuffer(header);
						if (frame.IsSync() && frame.GetLayer() == 3)
							break;
					}
					seekPoint++;
				}
				while (1)
				{
					file.SetCurrentPosition(seekPoint, CGioFile::GIO_FILE_BEGIN);
					int read = 0;
					file.Read(header, 6, &read);
					if (read != 6)
						break;
					frame.ReadBuffer(header);
					if (frame.IsSync() && frame.GetLayer() == 3)
					{
						offsets[offsetPosition] = seekPoint;
						offsetPosition = (offsetPosition + 1) % 8;
						offsetsRead++;
						seekPoint += frame.FrameSize();
					}
					else
						break;
				}
				if (offsetsRead >= 8)
				{
					StringCchPrintfW(data, dataLen, L"%I32d", offsets[offsetPosition] + file.m_vbr_frame_len);
					file.Close();
					return 1;
				}

				file.Close();
			}
		}
		return -1;
	}
	else
		return 0;
	return 1;
}

int fixAACCBRbitrate(int br);