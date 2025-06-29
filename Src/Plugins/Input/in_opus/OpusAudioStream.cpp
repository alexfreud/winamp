#include "OpusAudioStream.h"
#include <windows.h> // For MultiByteToWideChar if needed for errors, and basic types
#include <limits.h>  // For SHRT_MAX, SHRT_MIN

// Error codes for ifc_audiostream can be defined here or taken from a common header if available
// enum AudioStreamErrorCodes { ASE_SUCCESS = 0, ASE_ERROR_OPENING_FILE = -1, ASE_READ_ERROR = -2, ... };

OpusAudioStream::OpusAudioStream(const wchar_t *filepath, int *errorCode) :
    opusFile(NULL),
    opusHead(NULL),
    outputSampleRate(48000), // Opus default output, can be overridden by link
    numChannels(0),
    bitsPerSampleOut(16),
    totalPcmSamples(0),
    stream_error(0) // ASE_SUCCESS
{
    if (errorCode) *errorCode = 0; // Default to success

    char *filepath_char = AutoCharFn_downcast(filepath); // Convert wchar_t to char* for opusfile
    if (!filepath_char) {
        stream_error = OP_EFAULT; // Or a custom error code
        if (errorCode) *errorCode = -1; // Indicate error
        return;
    }

    int opus_err = 0;
    opusFile = op_open_file(filepath_char, &opus_err);

    if (!opusFile || opus_err != 0) {
        stream_error = opus_err ? opus_err : OP_EFAULT; // Store the error from opusfile
        if (opusFile) { // Should not happen if opus_err is set, but good practice
            op_free(opusFile);
            opusFile = NULL;
        }
        if (errorCode) *errorCode = -1; // Indicate generic error
        return;
    }

    opusHead = op_head(opusFile, 0); // Assuming the first logical stream
    if (!opusHead) {
        stream_error = OP_EBADHEADER; // Or some other error
        op_free(opusFile);
        opusFile = NULL;
        if (errorCode) *errorCode = -1;
        return;
    }

    numChannels = opusHead->channel_count;
    // libopusfile always decodes to 48kHz by default, but it's good to check the current link's rate
    const OpusFileCallbacks* callbacks = op_get_callbacks(opusFile); // Check if callbacks are set (usually NULL for op_open_file)
    int link_index = op_current_link(opusFile);
    if (link_index != -1) { // Should always be valid after successful open
         outputSampleRate = op_link_get_rate(opusFile,link_index); // This will be the actual output sample rate
    }


    totalPcmSamples = op_pcm_total(opusFile, -1); // -1 for current link
    if (totalPcmSamples == OP_EINVAL) { // Could happen if stream is not seekable / length unknown
        totalPcmSamples = 0; // Mark as unknown length rather than error for streaming
    }
}

OpusAudioStream::~OpusAudioStream()
{
    if (opusFile) {
        op_free(opusFile);
        opusFile = NULL;
    }
}

int OpusAudioStream::GetError() const
{
    return stream_error;
}

long OpusAudioStream::GetSampleRate() const
{
    return outputSampleRate;
}

int OpusAudioStream::GetChannels() const
{
    return numChannels;
}

int OpusAudioStream::GetBitsPerSample() const
{
    return bitsPerSampleOut;
}

ogg_int64_t OpusAudioStream::GetTotalPCMSamples() const
{
    return totalPcmSamples;
}


void OpusAudioStream::FloatToShort(float *in, short *out, size_t num_samples)
{
    for (size_t i = 0; i < num_samples; i++) {
        float sample = in[i];
        if (sample > 1.0f) sample = 1.0f;
        else if (sample < -1.0f) sample = -1.0f;
        out[i] = (short)(sample * SHRT_MAX);
    }
}

size_t OpusAudioStream::ReadAudio(void *buffer, size_t sizeBytes, int *killswitch, int *outErrorCode)
{
    if (outErrorCode) *outErrorCode = 0; // Default to success
    if (!opusFile || stream_error != 0) {
        if (outErrorCode) *outErrorCode = stream_error; // Or map to a generic error
        return 0; // Error or already failed state
    }

    if (killswitch && *killswitch) {
        return 0; // Playback aborted
    }

    // Calculate number of samples to read based on output format (16-bit PCM)
    size_t bytes_per_sample_out = (bitsPerSampleOut / 8) * numChannels;
    if (bytes_per_sample_out == 0) { // Should not happen
         if (outErrorCode) *outErrorCode = OP_EFAULT; // Or some internal error
        return 0;
    }
    size_t samples_to_read_total = sizeBytes / bytes_per_sample_out;

    if (samples_to_read_total == 0) { // Not enough space in buffer for even one frame
        return 0;
    }

    // libopusfile's op_read_float reads per channel, non-interleaved
    // We need a temporary buffer for float samples
    // Max samples per channel we might read:
    // opus_read_float can return up to 120ms of data at 48kHz. 0.120 * 48000 = 5760 samples.
    // Let's use a buffer that can hold this for stereo.
    // A more dynamic approach or smaller fixed buffer with looping might be better for memory.
    // For now, a reasonably sized static buffer or stack buffer if small enough.
    // Let's aim for the number of samples requested.

    float *float_pcm_buffer = new (std::nothrow) float[samples_to_read_total * numChannels];
    if (!float_pcm_buffer) {
        if (outErrorCode) *outErrorCode = OP_ALLOC_FAIL; // Or map to generic memory error
        return 0;
    }

    int samples_read_per_channel = op_read_float(opusFile, float_pcm_buffer, (int)samples_to_read_total, NULL);

    if (samples_read_per_channel < 0) { // Error
        stream_error = samples_read_per_channel; // Store opusfile error
        if (outErrorCode) *outErrorCode = stream_error;
        delete[] float_pcm_buffer;
        return 0;
    }

    if (samples_read_per_channel == 0) { // EOF
        if (outErrorCode) *outErrorCode = 0; // Not an error, just EOF
        delete[] float_pcm_buffer;
        return 0;
    }

    // op_read_float returns samples interleaved.
    // Convert the float PCM to 16-bit integer PCM directly into the output buffer.
    FloatToShort(float_pcm_buffer, (short *)buffer, (size_t)samples_read_per_channel * numChannels);

    delete[] float_pcm_buffer;

    return (size_t)samples_read_per_channel * bytes_per_sample_out;
}


int OpusAudioStream::SeekToTimeMs(int millisecs)
{
    if (!opusFile || stream_error != 0) return 0; // Error or not open

    // op_pcm_seek expects PCM offset
    // We need to convert milliseconds to PCM offset based on the *output* sample rate
    ogg_int64_t pcm_offset = (ogg_int64_t)millisecs * outputSampleRate / 1000;

    int result = op_pcm_seek(opusFile, pcm_offset);
    if (result == 0) {
        return 1; // Success
    } else {
        stream_error = result; // Store opusfile error
        return 0; // Failure
    }
}

int OpusAudioStream::CanSeek()
{
    if (!opusFile || stream_error != 0) return 0;
    return op_seekable(opusFile) ? 1 : 0;
}
