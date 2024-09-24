#include "WindacPlay.h"
#include "api__in_cdda.h"

int WindacPlay::threadProc2()
{
	while (1)
	{
		if (need_seek != -1)
		{
			current_sector = start_sector;
			current_sector += ((need_seek * 75) / 1000);
			bytes_in_sbuf = 0;
			line.outMod->Flush(need_seek);
			decode_pos_ms = need_seek;
			need_seek = -1;
		}
		if (!killswitch && bytes_in_sbuf <= 0 && current_sector.GetHSG() < end_sector.GetHSG())
		{
			if (!scsi->Get_DriveStatus().CDPresent)
			{
				//infos->error("No CD present!");
				PostMessage(line.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				return 0;
			}
			unsigned char *s = sbuf;
			while ((bytes_in_sbuf < buf_size*2352) && (current_sector.GetHSG() < end_sector.GetHSG()) && !killswitch)
			{
				int n = min((int)16, (int)(end_sector.GetHSG() - current_sector.GetHSG()));
				memset(s, 0, n*2352);
				scsi->ReadCDDA(current_sector, n, s);
				while (!scsi->WaitCDDA() && !killswitch) Sleep(66);
				bytes_in_sbuf += n * 2352;
				s += n * 2352;
				current_sector += n;
			}
		}

		if (!bytes_in_sbuf && !killswitch)
		{
			//wait for output to be finished
			line.outMod->Write(NULL, 0);
			while (!killswitch && line.outMod->IsPlaying()) Sleep(10);
			if (!killswitch)
				PostMessage(line.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
			return 0;
		}

		if (killswitch) return 0;

		char sample_buffer[576*4*2] = {0};
		int bytes = sizeof(sample_buffer) / 2; // enough room for dsp bullcrap
		bytes = min((int)bytes_in_sbuf, (int)bytes);
		memcpy(sample_buffer, sbuf, bytes);
		if (bytes_in_sbuf > bytes) memcpy(sbuf, sbuf + bytes, bytes_in_sbuf - bytes);
		bytes_in_sbuf -= bytes;

		int obytes = bytes;

		line.VSAAddPCMData(sample_buffer, g_nch, 16, line.outMod->GetWrittenTime() /*decode_pos_ms*/);
		line.SAAddPCMData(sample_buffer, g_nch, 16, line.outMod->GetWrittenTime() /*decode_pos_ms*/);

		if (line.dsp_isactive())
			bytes = line.dsp_dosamples((short *)sample_buffer, bytes / g_nch / 2, 16, g_nch, 44100) * (g_nch * 2);

		while (line.outMod->CanWrite() < bytes && !killswitch) Sleep(66);
		if (killswitch) return 0;

		line.outMod->Write(sample_buffer, bytes);

		decode_pos_ms += ((obytes / g_nch / 2) * 1000) / 44100;
	}
	return 0;
}

void WindacPlay::stop()
{
	if (hThread)
	{
		killswitch = 1;
		WaitForSingleObject(hThread, INFINITE);
	}
	if (needsToClose)
	{
		needsToClose = false;
	}
	line.outMod->Close();
}

int WindacPlay::open(wchar_t drive, int track) //called by winampGetExtendedRead
{
	g_drive = drive;
	if (!inited && !LoadASPI()) 
	{
		g_drive = 0;
		return 1;
	}

	inited = 1;

	int drivenum = 0;
	getTrackInfos(&drivenum, (char)drive);

	m_pMapDrive = new CMapDrive(TRUE);
	int nbdrives = m_pMapDrive->GetMaxDrives();
	if (!nbdrives) return 0;

	int host = -1, id = -1, lun = -1;
	if (getSCSIIDFromDrive((char)drive, &host, &id, &lun))
	{
		int found = 0;
		for (int i = 0;i < nbdrives;i++)
		{
			drive_info = m_pMapDrive->GetInfo(i);
			if (drive_info.HostAdapterNumber == host && drive_info.ID == id && drive_info.LUN == lun)
			{
				found = 1;
				break;
			}
		}
		if (!found)
		{
			s_last_error = "Drive not found";
			return 1;
		}
	}
	else
	{
		// can't figure out the SCSI ID, oh well, try the gay method
		TDriveInfo *tdi = &m_pMapDrive->GetInfo(drivenum);
		if (!tdi)
		{
			s_last_error = "Drive not found";
			return 1;
		}

		drive_info = *tdi;
	}

	scsi = new CSCSICD((char)drive, drive_info);

	TDriveStatus status = scsi->Get_DriveStatus();

	if (!status.CDPresent)
	{
		s_last_error = "CD not present";
		//infos->warning("No CD present!");
		g_drive=0;
		return 1;
	}

	TTrackList track_info = {0};
	track_info.TrackNummer = track;
	scsi->ReadTrackInfo(track_info);

	if (track_info.Flags.DataTrack)
	{
		s_last_error = "Cannot play track";
		//infos->warning("Can't play data tracks");
		g_drive=0;
		return 1;
	}

	start_sector = track_info.StartSektor;
	current_sector = start_sector;
	end_sector = start_sector;
	slength = track_info.Laenge;
	end_sector += slength;

	g_playlength = (slength / 75) * 1000;

	g_nch = track_info.Flags.AudioChannels;
	g_srate = 44100;
	g_bps = 16;

	scsi->PrepareCDDA();

	if (!sbuf)
		sbuf = (unsigned char *)malloc(2352 * buf_size);
	bytes_in_sbuf = 0;

	last_eject_scan = 0;

	return 0;
}

int WindacPlay::play(wchar_t drive, int track) //called by winamp2's normal(old) play() interface
{
	if (open(drive, track)) return 1;

	// do this here as it helps to prevent an audio glitch on first playback and volume is set low
	setvolume(a_v, a_p);

	int maxlat = line.outMod->Open(44100, g_nch, 16, -1, -1);
	if (maxlat < 0) 
	{
		g_drive=0;
		return 1;
	}

	line.SetInfo(1411, 44, g_nch, 1);
	line.SAVSAInit(maxlat, 44100);
	line.VSASetInfo(g_nch, 44100);
	line.is_seekable = 1;

	killswitch = 0;
	DWORD thread_id;
	hThread = CreateThread(NULL, NULL, &threadProc, (LPVOID)this, NULL, &thread_id);
	SetThreadPriority(hThread, AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));

	//open the device thru MCI (for getfileinfo to work properly)
	g_playtrack = track;
	needsToClose = true;

	return 0;
}

int WindacPlay::read(char *dest, int len, int *killswitch) //called by winampGetExtendedRead_getData
{
	int l = 0;

	while (l < len && !*killswitch)
	{
		if (!*killswitch && bytes_in_sbuf <= 0 && current_sector.GetHSG() < end_sector.GetHSG())
		{
			//scan for ejected CD only every 2 seconds
			if (last_eject_scan + 5000 < GetTickCount())
			{
				int cnt = 5;
				while (!scsi->Get_DriveStatus().CDPresent && cnt--)
				{
					Sleep(100);
				}
				if (cnt < 0 && !scsi->Get_DriveStatus().CDPresent)
				{
					//infos->error("No CD present!");
					return -1;
				}
				last_eject_scan = GetTickCount();
			}

			unsigned char *s = sbuf;
			while ((bytes_in_sbuf < buf_size*2352) && (current_sector.GetHSG() < end_sector.GetHSG()))
			{
				int n = min((int)16, (int)(end_sector.GetHSG() - current_sector.GetHSG()));
				memset(s, 0, n*2352);
				scsi->ReadCDDA(current_sector, n, s);
				while (!scsi->WaitCDDA() && !*killswitch) Sleep(66);
				if (*killswitch) break;
				bytes_in_sbuf += n * 2352;
				s += n * 2352;
				current_sector += n;
			}
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

void WindacPlay::getTrackInfos(int *drivenum, char driveletter)
{
	//finds first cdrom drive letter
	char firstcd = 'D';
	{
		DWORD drives = GetLogicalDrives();
		int nb = 0;
		for (int drivemask = 0; (drivemask < 32) && (nb < 4); drivemask++)
		{
			if (drives&(1 << drivemask))
			{
				wchar_t tmp[16] = {0};
				wsprintf(tmp, L"%c:\\", L'A' + drivemask);
				if (GetDriveType(tmp) == DRIVE_CDROM)
				{
					firstcd = 'A' + drivemask;
					break;
				}
			}
		}
	}

	*drivenum = driveletter - (unsigned char)firstcd;
}

WindacPlay::WindacPlay()
{
	scsi = NULL;
	sbuf = NULL;
	m_pMapDrive = NULL;
	buf_size = 64; //make it configitem
	hThread = NULL;
	decode_pos_ms = 0;
	inited = 0;
	need_seek = -1;
	needsToClose = false;
}

WindacPlay::~WindacPlay()
{
	if (scsi)
	{
		scsi->FinishCDDA();
		delete(scsi);
	}
	delete(m_pMapDrive);
	free(sbuf);
	if (inited) FreeASPI();

	if (needsToClose)
	{
		needsToClose = false;
	}
}