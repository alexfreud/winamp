#ifndef NULLSOFT_IN_OPUS_OPUSAUDIOSTREAM_H
#define NULLSOFT_IN_OPUS_OPUSAUDIOSTREAM_H

#include "../../../Agave/DecodeFile/ifc_audiostream.h"
#include <opusfile.h> // For OggOpusFile and related types
#include "../../../nu/AutoCharFn.h" // For path string conversion

class OpusAudioStream : public ifc_audiostream
{
public:
    OpusAudioStream(const wchar_t *filepath, int *errorCode);
    ~OpusAudioStream();

    // ifc_audiostream interface
    size_t ReadAudio(void *buffer, size_t sizeBytes, int *killswitch, int *outErrorCode) override; // Using the version with killswitch and error code
    int SeekToTimeMs(int millisecs) override;
    int CanSeek() override;

    // Additional methods if needed by the plugin
    long GetSampleRate() const;
    int GetChannels() const;
    int GetBitsPerSample() const; // Output bits per sample (typically 16 for Winamp)
    ogg_int64_t GetTotalPCMSamples() const;
    int GetError() const; // To retrieve error status after construction or operations

protected:
    // Using the ReadAudio that takes error code and killswitch, so the simpler one can be omitted or call this one.
    // size_t ReadAudio(void *buffer, size_t sizeBytes) override { int kill=0, err=0; return ReadAudio(buffer, sizeBytes, &kill, &err); }


private:
    OggOpusFile *opusFile;
    const OpusHead *opusHead; // Information about the stream (channels, sample rate)
    long outputSampleRate;    // Sample rate libopusfile is decoding to (usually 48kHz)
    int numChannels;
    int bitsPerSampleOut;     // Outputting 16-bit PCM
    ogg_int64_t totalPcmSamples;
    int stream_error;         // Stores error codes from libopusfile or construction

    // Helper for converting float samples to 16-bit int
    void FloatToShort(float *in, short *out, size_t num_samples);

public:
	DISPATCH_CODES
	{
	    IFC_AUDIOSTREAM_READAUDIO = 10, // Kept for compatibility if something calls the old one
		IFC_AUDIOSTREAM_READAUDIO2 = 11, // This is the one we will primarily implement via override
		IFC_AUDIOSTREAM_SEEKTOTIMEMS = 20,
		IFC_AUDIOSTREAM_CANSEEK = 30,
	};
};

#endif //NULLSOFT_IN_OPUS_OPUSAUDIOSTREAM_H
