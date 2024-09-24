#include "PlaybackBase2.h"

PlaybackImpl::PlaybackImpl()
{
	playback=0;
	playback_parameters=0;
}

PlaybackImpl::~PlaybackImpl()
{
	// TODO: decide if we need playback->Release() or not
	if (playback_parameters)
		playback_parameters->Release();
}

void PlaybackImpl::Connect(PlaybackBase2 *playback, ifc_playback_parameters *playback_parameters)
{
	this->playback = playback;
	// TODO: decide if we need playback->Retain() or not

	this->playback_parameters = playback_parameters;
	if (playback_parameters)
		playback_parameters->Retain();

}

/* ---------- */
PlaybackBase2::PlaybackBase2()
{
	out=0;
	implementation=0;
	filelocker=0;
	paused=false;
	last_position=0;
	output_pointers=0;
	exact_length=false;
	memset(&parameters, 0, sizeof(parameters));
}

PlaybackBase2::~PlaybackBase2()
{
	/* out should have hopefully been close already. just in case */
	if (out)
		out->Release();
	out=0;

	if (filelocker)
		filelocker->Release();
	delete implementation;
	free(output_pointers);

}

ns_error_t PlaybackBase2::Initialize(api_service *service_manager, PlaybackImpl *implementation, nx_uri_t filename, ifc_player *player)
{
	service_manager->GetService(&filelocker);

	this->implementation = implementation;
	ns_error_t ret = PlaybackBase::Initialize(filename, player);
	if (ret != NErr_Success)
		return ret;

	implementation->Connect(this, secondary_parameters);

	this->ifc_playback::Retain(); /* the thread needs to hold a reference to this object so that it doesn't disappear out from under us */
	NXThreadCreate(&playback_thread, PlayerThreadFunction, this);
	return NErr_Success;
}

nx_thread_return_t PlaybackBase2::PlayerThreadFunction(nx_thread_parameter_t param)
{
	PlaybackBase2 *playback = (PlaybackBase2 *)param;
	NXThreadCurrentSetPriority(NX_THREAD_PRIORITY_PLAYBACK);
	nx_thread_return_t ret = playback->DecodeLoop();
	playback->ifc_playback::Release(); /* give up the reference that was acquired before spawning the thread */
	return ret;
}

int PlaybackBase2::Init()
{
	if (filelocker)
		filelocker->WaitForReadInterruptable(filename, this);

	ns_error_t ret = implementation->Open(filename);
	if (ret != NErr_Success)
		return ret;

	ifc_metadata *metadata;
	if (implementation->GetMetadata(&metadata) == NErr_Success)
	{
		player->SetMetadata(metadata);
		metadata->Release();
	}
	else
		player->SetMetadata(0);

	player->SetSeekable(implementation->IsSeekable()?1:0);

	double length;
	ret = implementation->GetLength(&length, &exact_length);
	if (ret == NErr_Success)
		player->SetLength(length);

	return NErr_Success;
}

nx_thread_return_t PlaybackBase2::DecodeLoop()
{
	player->OnLoaded(filename);

	int ret = Init();
	if (ret != NErr_Success)
	{
		implementation->Close();
		if (filelocker)
			filelocker->UnlockFile(filename);
		player->OnError(ret);
		return 0;
	}

	player->OnReady();

	/* wait for Play (or Stop to abort) */
	for (;;)
	{
		ns_error_t ret = Wake(WAKE_PLAY|WAKE_STOP|WAKE_INTERRUPT);
		if (ret == WAKE_PLAY)
		{
			break;
		}
		else if (ret == WAKE_STOP)
		{
			player->OnStopped();
			goto cleanup;
		}
		else if (ret == WAKE_INTERRUPT)
		{
			ns_error_t ret = Internal_Interrupt();
			if (ret != NErr_Success)
			{
				implementation->Close();
				player->OnError(ret);
				goto cleanup;
			}			
		}
	}

	/* at this point, we know that PLAY is on */
	for (;;)
	{
		int ret = Check(WAKE_STOP|WAKE_PAUSE|WAKE_INTERRUPT);
		if (ret == WAKE_PAUSE)
		{
			if (out)
				out->Pause(1);
			paused=true;
			continue; /* continue in case there's another wake reason */
		}
		else if (ret== WAKE_UNPAUSE)
		{
			if (out)
				out->Pause(0);
			paused=false;
			continue; /* continue in case there's another wake reason */
		}
		else if (ret == WAKE_STOP)
		{
			if (out)
			{
				out->Stop();
				out->Release();
				out=0;
			}
			player->OnStopped();
			goto cleanup;
		}
		else if (ret == WAKE_INTERRUPT)
		{
			ns_error_t ret = Internal_Interrupt();
			if (ret != NErr_Success)
			{
				implementation->Close();
				player->OnError(ret);
				goto cleanup;
			}
			continue;
		}

		Agave_Seek *seek = PlaybackBase::GetSeek();
		if (seek)
		{
			ns_error_t seek_error;
			double new_position;
			ns_error_t ret = implementation->Seek(seek, &seek_error, &new_position);
			if (ret != NErr_Success)
			{
				player->OnError(ret);
				goto cleanup;
			}
			if (out)
			out->Flush(new_position);
			player->OnSeekComplete(seek_error, new_position);
			PlaybackBase::FreeSeek(seek);
		}

		ret = implementation->DecodeStep();
		if (ret == NErr_EndOfFile)
		{
			if (out)
				out->Done();

			PlaybackBase::OnStopPlaying();
			player->OnEndOfFile();

			ret = WaitForClose();
			if (out)
				out->Release();
			out=0;

			if (ret != NErr_True)
				goto cleanup;
		}
		else if (ret != NErr_Success)
		{
			if (out)
			{
				out->Done();
				out->Release();
				out=0;
			}
			if (ret != NErr_False)
				player->OnError(NErr_Error); // TODO: find better error code
			goto cleanup;
		}
		else
		{
			if (!exact_length)
			{
				double length;
				ret = implementation->GetLength(&length, &exact_length);
				if (ret == NErr_Success)
					player->SetLength(length);
			}
		}
	}

cleanup:
	implementation->Close();

	if (filelocker)
		filelocker->UnlockFile(filename);
	return 0;
}

ns_error_t PlaybackBase2::WaitForClose()
{
	if (!out)
	{
		player->OnClosed();
		return NErr_False;
	}
	else for (;;)
	{
		int ret = Wait(10, WAKE_PLAY|WAKE_KILL|WAKE_STOP);
		if (ret == WAKE_KILL)
		{
			player->OnClosed();
			return NErr_False;
		}
		else if (ret == WAKE_PLAY)
		{
			return NErr_True;
		}
		else if (ret == WAKE_STOP)
		{
			player->OnStopped();
			return NErr_False;
		}
		else
		{
			if (out->Playing() == NErr_True)
				player->SetPosition(last_position - out->Latency());
			else
			{
				player->SetPosition(last_position);
				player->OnClosed();
				return NErr_False;
			}			
		}
	}
}

ns_error_t PlaybackBase2::OpenOutput(const ifc_audioout::Parameters *_parameters)
{
	// if out is already set, it means that there was a change in parameters, so we'll start a new stream
	if (out)
	{
		// check to see that the parameters actually changed
		if (!memcmp(&parameters, _parameters, sizeof(parameters)))
			return NErr_Success;

		out->Done();
		out=0;
	}

	parameters = *_parameters;

	free(output_pointers);
	output_pointers = (const uint8_t **)malloc(parameters.audio.number_of_channels * sizeof(const uint8_t *));
	if (!output_pointers)
		return NErr_OutOfMemory;

	ns_error_t ret = output_service->AudioOpen(&parameters, player, secondary_parameters, &out);
	if (ret != NErr_Success)
	{
		player->OnError(ret);
		return ret;
	}

	if (paused)
		out->Pause(1);
	else
		out->Pause(0);


	return NErr_True;
}

int PlaybackBase2::OutputNonInterleaved(const void *decode_buffer, size_t decoded, double start_position)
{
	int ret;
	size_t frames_written=0;
	const uint8_t **buffer = (const uint8_t **)decode_buffer;

	for (size_t c=0;c<parameters.audio.number_of_channels;c++)
	{
		output_pointers[c] = buffer[c];
	}

	while (decoded)
	{
		size_t to_write = out->CanWrite();
		if (to_write)
		{
			if (decoded < to_write)
				to_write = decoded;

			ret = out->Output(output_pointers, to_write);
			if (ret != NErr_Success)
			{
				out->Release();
				out=0;
				return ret;
			}

			decoded -= to_write;
			for (size_t c=0;c<parameters.audio.number_of_channels;c++)
			{
				output_pointers[c] += to_write/parameters.audio.number_of_channels;
			}
			frames_written += to_write/parameters.audio.number_of_channels;
			player->SetPosition(start_position + (double)frames_written/parameters.audio.sample_rate - out->Latency());
		}
		else
		{
			ns_error_t ret = OutputWait();
			if (ret != NErr_Success)
				return ret;
		}
	}
	return NErr_True;
}

int PlaybackBase2::Output(const void *decode_buffer, size_t decoded, double start_position)
{
	int ret;
	size_t frames_written=0;
	const uint8_t *decode8 = (const uint8_t *)decode_buffer;
	size_t buffer_position=0;
	while (decoded)
	{
		size_t to_write = out->CanWrite();
		if (to_write)
		{
			if (decoded < to_write)
				to_write = decoded;

			ret = out->Output(&decode8[buffer_position], to_write);
			if (ret != NErr_Success)
			{
				out->Release();
				out=0;
				return ret;
			}

			decoded -= to_write;
			buffer_position += to_write;
			frames_written += to_write/parameters.audio.number_of_channels;
			player->SetPosition(start_position + (double)frames_written/parameters.audio.sample_rate - out->Latency());
		}
		else
		{
			ns_error_t ret = OutputWait();
			if (ret != NErr_Success)
				return ret;	
		}
	}
	return NErr_True;
}

ns_error_t PlaybackBase2::OutputWait()
{
	if (paused)
	{
		/* if we're paused, we need to sit and wait until we're eiter unpaused or stopped */
		for (;;)
		{
			int ret = Wake(WAKE_STOP|WAKE_PAUSE|WAKE_INTERRUPT);
			if (ret == WAKE_STOP)
			{
				out->Stop();
				out->Release();
				out=0;
				player->OnStopped();
				return NErr_False;
			}
			else if (ret == WAKE_UNPAUSE)
			{
				out->Pause(0);
				paused=false;
				break;
			}
			else if (ret == WAKE_PAUSE)
			{
				out->Pause(1);
				paused=true;
			}
			else if (PlaybackBase::PendingSeek())
			{
				return NErr_True;
			}
			else if (ret == WAKE_INTERRUPT)
			{
				ns_error_t ret = Internal_Interrupt();
				if (ret != NErr_Success)
					return ret;				
			}
		}
	}
	else
	{
		int ret = Wait(10, WAKE_STOP);
		if (ret == WAKE_STOP)
		{
			out->Stop();
			out->Release();
			out=0;
			player->OnStopped();
			return NErr_False;
		}
	}
	return NErr_Success;
}

ns_error_t PlaybackBase2::Internal_Interrupt()
{
	Agave_Seek resume_information;
	implementation->Interrupt(&resume_information);
	ns_error_t ret = filelocker->UnlockFile(filename);
	if (ret != NErr_Success)
	{
		implementation->Close();
		return ret;
	}
	PlaybackBase::OnInterrupted();
	if (filelocker)
		filelocker->WaitForReadInterruptable(filename, this);
	ret = implementation->Resume(&resume_information);


	if (ret != NErr_Success)
		return ret;
	ifc_metadata *metadata;
	if (implementation->GetMetadata(&metadata) == NErr_Success)
	{
		player->SetMetadata(metadata);
		metadata->Release();
	}
	return NErr_Success;
}