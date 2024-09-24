#include "DAEPlay.h"
#include "api__in_cdda.h"
#include "../nu/AutoWide.h"
#include <strsafe.h>

int DAEPlay::getTrackInfo()
{
	CDROM_TOC tableOfContents = {0};
	DWORD tocSize = sizeof(tableOfContents);

	if (!DeviceIoControl(hDrive, IOCTL_CDROM_READ_TOC, NULL, 0, &tableOfContents, tocSize, &tocSize, 0))
    {
        return 1;
    }

    for (int i = tableOfContents.FirstTrack - 1 ; i < tableOfContents.LastTrack ; i += 1)
    {
		if (i == g_track)
		{
			track_length = MSFToBlocks(tableOfContents.TrackData[i+1].Address) - (start_address = MSFToBlocks(tableOfContents.TrackData[i].Address));
			return 0;
		}
    }

    return 1;
}

DAEPlay::CDTextArray* DAEPlay::getCDText()
{
	// if we've already cached it, return asap
	if (cd_text != 0)
	{
		return cd_text;
	}

	CDROM_TOC tableOfContents = {0};
	DWORD tocSize = sizeof(tableOfContents), returned = 0;

	if (!DeviceIoControl(hDrive, IOCTL_CDROM_READ_TOC, NULL, 0, &tableOfContents, tocSize, &tocSize, 0))
	{
		// issue accessing drive
		return (cd_text = (CDTextArray *)-1);
	}

	// MMC-3 Draft Revision 10g: Table 222 Q Sub-channel control field
	tableOfContents.TrackData[0].Control &= 5;
	if (!(tableOfContents.TrackData[0].Control == 0/* || tableOfContents.TrackData[0 - 1].Control == 1*/))
	{
		// invalid format
		return (cd_text = (CDTextArray *)-1);
	}

	CDROM_READ_TOC_EX tableOfContentsEx = {0};
	tableOfContentsEx.Format = CDROM_READ_TOC_EX_FORMAT_CDTEXT;
	WORD tocSizeEx = 0;
	// Read the contents to get the size of the actual data
	if (!DeviceIoControl(hDrive, IOCTL_CDROM_READ_TOC_EX, &tableOfContentsEx, sizeof(tableOfContentsEx), &tocSizeEx, sizeof(tocSizeEx), &returned, 0))
	{
		// issue accessing drive
		return (cd_text = (CDTextArray *)-1);
	}

	// The bytes are swapped so we need to switch them around
	tocSizeEx = ((tocSizeEx>>8) | (tocSizeEx<<8)) + sizeof(tocSizeEx);

	// Allocate a buffer for reading the actual CD Text data block
	char *pCDTextData = new char[tocSizeEx];
	if (!pCDTextData)
	{
		return (cd_text = (CDTextArray *)-1);
	}

	memset(pCDTextData, 0, tocSizeEx);

	// Now read the data
	if(!DeviceIoControl(hDrive, IOCTL_CDROM_READ_TOC_EX, &tableOfContentsEx, sizeof(tableOfContentsEx), pCDTextData, tocSizeEx, &returned, 0))
	{
		delete []pCDTextData;
		return (cd_text = (CDTextArray *)-1);
	}

	tocSizeEx = (WORD)(returned - sizeof(CDROM_TOC_CD_TEXT_DATA));

	if(!tocSizeEx)
	{
		delete []pCDTextData;
		return (cd_text = (CDTextArray *)-1);
	}

	// This is the stuff we really need.  It's an array of packs with the data (mostly text)
	CDROM_TOC_CD_TEXT_DATA_BLOCK* pCDTextBlock = ((CDROM_TOC_CD_TEXT_DATA*)(BYTE*)pCDTextData)->Descriptors;

	// As we go through the packets we'll store the strings in this array.  The strings are often in different packets and need to be concatenated together.
	cd_text = new CDTextArray[CD_TEXT_NUM_PACKS];
	UINT m_nGenreCode = -1;

	// Loop through all the packets extracting the data we need.  Each packet starts with a packet type, the track number, language code, 
	// character offset, and character size.  The packets end with a 2 byte CRC.  We don't need most of this stuff to display the info.  We can get
	// the packets for any track and they'll have all the data for each track (it's duplicated).
	for( ;tocSizeEx >= sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK); tocSizeEx -= sizeof(CDROM_TOC_CD_TEXT_DATA_BLOCK), pCDTextBlock++)
	{
		if (pCDTextBlock->TrackNumber > tableOfContents.LastTrack) // Track is beyond what is on the disc
			continue;
	
		// Only looking for CD Text item packets
		if ((pCDTextBlock->PackType -= 0x80) >= 0x10)
			continue;
		
		// Genre is encoded with the code and the supplemental text in the same packet
		if (m_nGenreCode == -1 && pCDTextBlock->PackType == CD_TEXT_GENRE)
		{
			// TODO
			m_nGenreCode = pCDTextBlock->Text[0]*16 + pCDTextBlock->Text[1];
			/*CString Text = !pCDTextBlock->Unicode 
				? CString(CStringA((CHAR*)pCDTextBlock->Text+2, CD_TEXT_FIELD_LENGTH-2))
				: CString(CStringW((WCHAR*)pCDTextBlock->WText+2, CD_TEXT_FIELD_LENGTH-2));
			cd_text[pCDTextBlock->PackType][0] = Text;*/
		}
		else
		{
			// Parse the text.  There could be more than one item in the text block separated by null bytes so we need to check the whole thing
			// The text is in a block of up to 12 characters.  We'll just keep adding to our buffers until we go through all of the packets.
			int nLengthRemaining = CD_TEXT_FIELD_LENGTH;
			UINT nTrack = pCDTextBlock->TrackNumber;
			UINT nOffset = 0;
			// We're at the end of text when:
			// Used up 12 chars
			// Got to a null
			// On the last track
			while (nTrack <= tableOfContents.LastTrack && nLengthRemaining > 0 && nOffset < CD_TEXT_FIELD_LENGTH)
			{
				wchar_t *text = (wchar_t*)calloc(nLengthRemaining + 1, sizeof(wchar_t));
				if (!text) continue;

				lstrcpyn(text, (pCDTextBlock->Unicode ? pCDTextBlock->WText + nOffset : AutoWide((char *)pCDTextBlock->Text + nOffset)), nLengthRemaining + 1);

				if (!cd_text[pCDTextBlock->PackType][nTrack])
					cd_text[pCDTextBlock->PackType][nTrack] = (wchar_t*)calloc(lstrlen(text) + 1, sizeof(wchar_t));
				else // TODO error handling
					cd_text[pCDTextBlock->PackType][nTrack] = (wchar_t*)realloc(cd_text[pCDTextBlock->PackType][nTrack], (lstrlen(cd_text[pCDTextBlock->PackType][nTrack]) + lstrlen(text) + 1) * sizeof(wchar_t));

				if (text[0])
					lstrcat(cd_text[pCDTextBlock->PackType][nTrack], text);

				nOffset += lstrlen(text) + 1;
				nLengthRemaining = nLengthRemaining - lstrlen(text) - 1;
				++nTrack;
				free(text);
			}
		}
	} while (0);

	delete []pCDTextData;
	return cd_text;
}

int DAEPlay::threadProc2()
{
	while (1)
	{
		if (need_seek != -1)
		{
			current_sector = ((need_seek * CD_BLOCKS_PER_SECOND) / 1000) / DEF_SECTORS_PER_READ;
			bytes_in_sbuf = 0;
			line.outMod->Flush(need_seek);
			need_seek = -1;
		}

		if (fillBuffer(killswitch))
		{
			PostMessage(line.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
			return 0;
		}

		if (!bytes_in_sbuf && !killswitch)
		{
			//wait for output to be finished
			line.outMod->Write(NULL, 0);
			while (!killswitch && line.outMod->IsPlaying()) Sleep(10);
			if (!killswitch)
			{
				CloseHandle(hDrive);
				hDrive = INVALID_HANDLE_VALUE;
				PostMessage(line.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
			}
			return 0;
		}

		if (killswitch)
		{
			CloseHandle(hDrive);
			hDrive = INVALID_HANDLE_VALUE;
			return 0;
		}

		char sample_buffer[576*4*2] = {0};
		int bytes = sizeof(sample_buffer) / 2; // enough room for dsp bullcrap
		bytes = min((int)bytes_in_sbuf, (int)bytes);
		memcpy(sample_buffer, sbuf, bytes);
		if (bytes_in_sbuf > bytes) memcpy(sbuf, sbuf + bytes, bytes_in_sbuf - bytes);
		bytes_in_sbuf -= bytes;

		line.VSAAddPCMData(sample_buffer, g_nch, 16, line.outMod->GetWrittenTime());
		line.SAAddPCMData(sample_buffer, g_nch, 16, line.outMod->GetWrittenTime());

		if (line.dsp_isactive())
			bytes = line.dsp_dosamples((short *)sample_buffer, bytes / g_nch / 2, 16, g_nch, 44100) * (g_nch * 2);

		while (line.outMod->CanWrite() < bytes && !killswitch) Sleep(66);
		if (killswitch)
		{
			CloseHandle(hDrive);
			hDrive = INVALID_HANDLE_VALUE;
			return 0;
		}

		line.outMod->Write(sample_buffer, bytes);
	}

	CloseHandle(hDrive);
	hDrive = INVALID_HANDLE_VALUE;
	return 0;
}

void DAEPlay::stop()
{
	if (hThread)
	{
		killswitch = 1;
		WaitForSingleObject(hThread, INFINITE);
	}

	line.outMod->Close();
}

int DAEPlay::open(wchar_t drive, int track) //called by winampGetExtendedRead
{
	g_track = track-1;
	if (g_track < 0)
		return 1;

	g_drive = drive;

	wchar_t CDDevice[8]=L"\\\\.\\x:";
	CDDevice[4] = drive;

	if (hDrive != INVALID_HANDLE_VALUE)
		CloseHandle(hDrive);

    hDrive = CreateFile(CDDevice, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDrive == INVALID_HANDLE_VALUE)
		return 1;

	// TODO only store the request track!
	if (getTrackInfo())
		return 1;

	g_playlength = (track_length / CD_BLOCKS_PER_SECOND) * 1000;

	bytes_in_sbuf = 0;

	if (!sbuf)
		sbuf = (unsigned char *)malloc(2352 * buf_size);
	if (!sbuf)
		return 1;

	data = new BYTE[DEF_SECTORS_PER_READ * CDROM_RAW_BYTES_PER_SECTOR];
	if (!data)
		return 1;

	return 0;
}

int DAEPlay::play(wchar_t drive, int track) //called by winamp2's normal(old) play() interface
{
	int old_drive = g_drive;

	if (open(drive, track)) return 1;

	// do this here as it helps to prevent an audio glitch on first playback and volume is set low
	setvolume(a_v, a_p);

	int maxlat = line.outMod->Open(44100, g_nch, 16, -1, -1);
	if (maxlat < 0)
	{
		g_drive = 0;
		return 1;
	}

	// to prevent re-spinning as we're not going to get cd-text later
	// if it's not able to be obtained when first opening the device.
	if (old_drive != drive)
	{
		if ((int)cd_text > 0)
			delete []cd_text;
		cd_text = NULL;
		getCDText();
	}

	line.SetInfo(1411, 44, g_nch, 1);
	line.SAVSAInit(maxlat, 44100);
	line.VSASetInfo(g_nch, 44100);
	line.is_seekable = 1;

	bytes_in_sbuf = 0;
	current_sector = 0;
	killswitch = 0;
	DWORD thread_id = 0;
	hThread = CreateThread(NULL, NULL, &threadProc, (LPVOID)this, NULL, &thread_id);
	SetThreadPriority(hThread, AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));

	//open the device thru MCI (for getfileinfo to work properly)
	g_playtrack = track;

	return 0;
}

int DAEPlay::fillBuffer(int kill)
{
	if (!kill && bytes_in_sbuf <= 0)
	{
		if (current_sector < (track_length / DEF_SECTORS_PER_READ))
		{
			// Contains an offset into the CD-ROM disc where data will be read. You can calculate this offset by multiplying the starting sector number for the request times 2048.
			RAW_READ_INFO _RawReadInfo = {0};
			_RawReadInfo.DiskOffset.QuadPart = ((current_sector * DEF_SECTORS_PER_READ) + start_address) * CDROM_COOKED_BYTES_PER_SECTOR;
			_RawReadInfo.TrackMode = CDDA;
			_RawReadInfo.SectorCount = DEF_SECTORS_PER_READ;

			DWORD data_length = DEF_SECTORS_PER_READ * CDROM_RAW_BYTES_PER_SECTOR;
			if (!DeviceIoControl(hDrive, IOCTL_CDROM_RAW_READ, &_RawReadInfo, sizeof(_RawReadInfo),
								 data, data_length, &data_length, 0))
			{
				CloseHandle(hDrive);
				hDrive = INVALID_HANDLE_VALUE;
				return 1;
			}

			// TODO make sure the buffer size is enough for our needs
			memcpy(sbuf, data, DEF_SECTORS_PER_READ * CDROM_RAW_BYTES_PER_SECTOR);
			bytes_in_sbuf += DEF_SECTORS_PER_READ * CDROM_RAW_BYTES_PER_SECTOR;
		}
		current_sector++;
	}
	return 0;
}

int DAEPlay::read(char *dest, int len, int *killswitch) //called by winampGetExtendedRead_getData
{
	int l = 0;

	// TODO make sure this will handle a CD ejection...
	while (l < len && !*killswitch)
	{
		if (fillBuffer(*killswitch))
		{
			return -1;
		}

		if (!bytes_in_sbuf) break;

		int bytes = min(bytes_in_sbuf, len - l);
		memcpy(dest + l, sbuf, bytes);

		if (bytes_in_sbuf > bytes) memcpy(sbuf, sbuf + bytes, bytes_in_sbuf - bytes);
		bytes_in_sbuf -= bytes;

		l += bytes;
	}

	return l;
}

DAEPlay::DAEPlay()
{
	sbuf = NULL;
	data = NULL;
	cd_text = NULL;
	bytes_in_sbuf = 0;
	buf_size = 64; //make it configitem
	g_track = -1;

	// fix these three as that's normal
	g_nch = 2;
	g_srate = 44100;
	g_bps = 16;

	killswitch = 0;
	hDrive = INVALID_HANDLE_VALUE;
	hThread = NULL;
	need_seek = -1;
	current_sector = 0;
	start_address = 0;
	track_length = 0;
}

DAEPlay::~DAEPlay()
{
	if (hDrive != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDrive);
		hDrive = INVALID_HANDLE_VALUE;
	}

	if (sbuf)
	{
		free(sbuf);
		sbuf = NULL;
	}

	if (data)
	{
		delete []data;
		data = NULL;
	}

	if ((int)cd_text > 0)
	{
		delete []cd_text;
	}
	cd_text = NULL;
}