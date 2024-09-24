#pragma once
#include <bfc/platform/types.h>
#include "../Winamp/in2.h"
#include "../Winamp/out.h"
#include "SpillBuffer.h"
#include <assert.h>

/* A class to manage Winamp input plugin audio output
** It handles the following for you:
** * Ensuring that Vis data is sent in chunks of 576
** * Dealing with gapless audio 
** (you need to pass in the number of pre-delay and post-delay samples)
** * dealing with the DSP plugin
** * Waiting for CanWrite()
** * dealing with inter-timestamps 
** e.g. you pass it >576 samples and it can give you a timestamp based on the divided chunk position

to use, you need to derive from a class that declares
int WaitOrAbort(int time_in_ms);
return 0 on success, non-zero when you need to abort.  the return value is passed back through Write()
*/

namespace nu // namespace it since "AudioOutput" isn't a unique enough name
{
	template <class wait_t>
	class AudioOutput : public wait_t
	{
	public:
		AudioOutput( In_Module *plugin ) : plugin( plugin )
		{
			Init( nullptr );
		}

		~AudioOutput()
		{
			post_buffer.reset();
			buffer576.reset();
		}

		/* Initializes and sets the output plugin pointer
		** for most input plugins, the nu::AudioOutput object will be a global, 
		** so this will be necessary to call at the start of Play thread */
		void Init( Out_Module *_output )
		{
			output          = _output;
			audio_opened    = false;
			first_timestamp = 0;
			sample_size     = 0;
			output_latency  = 0;

			post_buffer.reset();
			buffer576.reset();

			cut_size        = 0;
			pre_cut_size    = 0;
			pre_cut         = 0;
			decoder_delay   = 0;
			channels        = 0;
			sample_rate     = 0;
			bps             = 0;
		}

		/* sets end-of-stream delay (in samples)
		** WITHOUT componesating for post-delay. 
		** some filetypes (e.g. iTunes MP4) store gapless info this way */
		void SetPostDelay(int postSize)
		{
			if (postSize < 0)
			{
				postSize = 0;
			}
			else if (postSize)
			{
				if (sample_size)
					post_buffer.reserve(postSize*sample_size);

				cut_size = postSize;
			}
		}

		/* set end-of-stream zero padding, in samples
		** compensates for decoder delay */
		void SetZeroPadding(int postSize)
		{
			postSize -= decoder_delay;
			if (postSize < 0)
			{
				postSize = 0;
			}
			SetPostDelay(postSize);
		}

		/* set decoder delay, initial zero samples and end-of-stream zero samples, all in one shot
		** adjusts zero samples for decoder delay. call SetDelays() if your zero samples are already compensated */
		void SetGapless(int decoderDelaySize, int preSize, int postSize)
		{
			decoder_delay = decoderDelaySize;
			SetZeroPadding(postSize);

			pre_cut_size = preSize;
			pre_cut = pre_cut_size + decoder_delay;
		}

		/* set decoder delay, initial delay and end-of-stream delay, all in one shot
		** WITHOUT componesating for post-delay. 
		** some filetypes (e.g. iTunes MP4) store gapless info this way */
		void SetDelays(int decoderDelaySize, int preSize, int postSize)
		{
			decoder_delay = decoderDelaySize;
			SetPostDelay(postSize);

			pre_cut_size = preSize;
			pre_cut = pre_cut_size;
		}

		/* Call on seek */
		void Flush(int time_in_ms)
		{
			if (audio_opened)
			{
				pre_cut = pre_cut_size;

				output->Flush(time_in_ms);
				first_timestamp = 0; // once we've flushed, we should be accurate so no need for this anymore
				buffer576.clear();
				post_buffer.clear();
			}
			else
				first_timestamp = time_in_ms;
		}

		bool Opened() const
		{
			return audio_opened;
		}

		int GetLatency() const
		{
			return output_latency;
		}

		int GetFirstTimestamp() const
		{
			return first_timestamp;
		}

		/* timestamp is meant to be the first timestamp according to the containing file format
		** e.g. many MP4 videos start on 12ms or something, for accurate a/v syncing */
		bool Open(int timestamp, int channels, int sample_rate, int bps, int buffer_len_ms=-1, int pre_buffer_ms=-1)
		{
			if (!audio_opened)
			{
				int latency = output->Open(sample_rate, channels, bps, buffer_len_ms, pre_buffer_ms);
				if (latency < 0)
					return false;
				plugin->SAVSAInit(latency, sample_rate);
				plugin->VSASetInfo(sample_rate, channels);
				output->SetVolume(-666);
				plugin->SetInfo(-1, sample_rate / 1000, channels, /* TODO? 0*/1);

				output_latency = latency;
				first_timestamp = timestamp;
				sample_size = channels*bps / 8;
				this->channels=channels;
				this->sample_rate=sample_rate;
				this->bps=bps;
				SetPostDelay((int)cut_size); // set this again now that we know sample_size, so buffers get allocated correctly
				buffer576.reserve(576*sample_size);
				audio_opened=true;
			}
			return audio_opened;
		}

		void Close()
		{
			if (audio_opened && output)
			{
				output->Close();
				plugin->SAVSADeInit();
			}
			output = 0;
			first_timestamp = 0;
		}

		/* outSize is in bytes
		** */
		int Write(char *out, size_t outSize)
		{
			if (!out && !outSize)
			{
				/* --- write contents of buffered audio (end-zero-padding buffer) */
				if (!post_buffer.empty())
				{
					void *buffer = 0;
					size_t len = 0;
					if (post_buffer.get(&buffer, &len))
					{
						int ret = Write576((char *)buffer, len);
						if (ret != 0)
							return ret;
					}
				}

				/* --- write any remaining data in 576 spill buffer (skip vis) */
				if (!buffer576.empty())
				{
					void *buffer = 0;
					size_t len = 0;
					if (buffer576.get(&buffer, &len))
					{
						int ret = WriteOutput((char *)buffer, len);
						if (ret != 0)
							return ret;
					}
				}

				output->Write(0, 0);
				return 0;
			}

			// this probably should not happen but have seen it in some crash reports
			if (!sample_size)
				return 0;

			assert((outSize % sample_size) == 0);
			size_t outSamples = outSize / sample_size;

			/* --- cut pre samples, if necessary --- */
			size_t pre  = min(pre_cut, outSamples);
			out        += pre * sample_size;
			outSize    -= pre  * sample_size;
			pre_cut    -= pre;
			//outSize = outSamples * sample_size;

			// do we will have samples to output after cutting pre-delay?
			if (!outSize)
				return 0;

			/* --- if we don't have enough to fully fill the end-zero-padding buffer, go ahead and fill --- */
			if (outSize < post_buffer.length())
			{
				size_t bytes_written = post_buffer.write(out, outSize);
				out+=bytes_written;
				outSize-=bytes_written;
			}

			// if we're out of samples, go ahead and bail
			if (!outSize)
				return 0;

			/* --- write contents of buffered audio (end-zero-padding buffer) */
			if (!post_buffer.empty())
			{
				void *buffer = 0;
				size_t len = 0;
				if (post_buffer.get(&buffer, &len))
				{
					int ret = Write576((char *)buffer, len);
					if (ret != 0)
						return ret;
				}
			}

			/* --- make sure we have enough samples left over to fill our post-zero-padding buffer --- */
			size_t remainingFill = /*cut_size - */post_buffer.remaining();
			int outWrite = max(0, (int)outSize - (int)remainingFill);

			/* --- write the output that doesn't end up in the post buffer */
			if (outWrite)
			{
				int ret = Write576(out, outWrite);
				if (ret != 0)
					return ret;
			}
			out += outWrite;
			outSize -= outWrite;

			/* --- write whatever is left over into the end-zero-padding buffer --- */
			if (outSize)
			{
				post_buffer.write(out, outSize);
			}
			return 0;
		}

		/* meant to be called after Write(0,0) */
		int WaitWhilePlaying()
		{
			while (output->IsPlaying())
			{
				int ret = WaitOrAbort(10);
				if (ret != 0)
					return ret;

				output->CanWrite();		// some output drivers need CanWrite
				// to be called on a regular basis.
			}
			return 0;
		}
	private:
		/* helper methods */
		int WaitForOutput(int write_size_bytes)
		{
			while (output->CanWrite() < write_size_bytes)
			{
				int ret = WaitOrAbort(55);
				if (ret != 0)
					return ret;
			}
			return 0;
		}


		/* writes one chunk (576 samples) to the output plugin, waiting as necessary */
		int WriteOutput(char *buffer, size_t len)
		{
			int ret = WaitForOutput((int)len);
			if (ret != 0)
				return ret;

			// write vis data before so we guarantee 576 samples
			if (len == 576*sample_size)
			{
				plugin->SAAddPCMData(buffer, channels, bps, output->GetWrittenTime() + first_timestamp);
				plugin->VSAAddPCMData(buffer, channels, bps, output->GetWrittenTime() + first_timestamp);
			}

			if (plugin->dsp_isactive())
				len = sample_size * plugin->dsp_dosamples((short *)buffer, (int)(len / sample_size), bps, channels, sample_rate);

			output->Write(buffer, (int)len);
			return 0;
		}

		/* given a large buffer, writes 576 sample chunks to the vis, dsp and output plugin */
		int Write576(char *buffer, size_t out_size)
		{
			/* if we have some stuff leftover in the 576 sample spill buffer, fill it up */
			if (!buffer576.empty())
			{
				size_t bytes_written = buffer576.write(buffer, out_size);
				out_size -= bytes_written;
				buffer += bytes_written;
			}

			if (buffer576.full())
			{
				void *buffer = 0;
				size_t len = 0;
				if (buffer576.get(&buffer, &len))
				{
					int ret = WriteOutput((char *)buffer, len);
					if (ret != 0)
						return ret;
				}
			}

			while (out_size >= 576*sample_size)
			{
				int ret = WriteOutput(buffer, 576*sample_size);
				if (ret != 0)
					return ret;

				out_size -= 576*sample_size;
				buffer+=576*sample_size;
			}

			if (out_size) 
			{
				assert(out_size < 576*sample_size);
				buffer576.write(buffer, out_size);
			}
			return 0;
		}

	private:
		Out_Module *output;
		In_Module *plugin;
		SpillBuffer post_buffer, buffer576;
		size_t cut_size;
		size_t pre_cut, pre_cut_size, decoder_delay;
		bool audio_opened;
		int first_timestamp; /* timestamp of the first decoded audio frame, necessary for accurate video syncing */
		size_t sample_size; /* size, in bytes, of one sample of audio (channels*bps/8) */
		int output_latency; /* as returned from Out_Module::Open() */
		int channels, sample_rate, bps;
	};
}
