#if !defined(MP3HEADER_HPP)
#define MP3HEADER_HPP

#include <iosfwd>

class Mp3Header
{
public:
    Mp3Header(unsigned long);
    unsigned long  id;
    unsigned long  layer;
    unsigned long  protectionBit;
    unsigned long  bitRateIndex;
    unsigned long  samplingFrequency;
    unsigned long  paddingBit;
    unsigned long  privateBit;
    unsigned long  mode;
    unsigned long  modeExtension;
    unsigned long  copyright;
    unsigned long  originalOrCopy;
    unsigned long  emphasis;
    unsigned long  nch;
    unsigned long  sampleRate;
    unsigned long  bitRate;
    unsigned long  frameSize;
    unsigned short outFrameSize;

    enum { BITRATE_FREE = 0 };
    enum { MPEG_FORBIDDEN = -1};
    enum { SAMPLING_FREQUENCY_RESERVED = -1};

    enum IdTypes
    {
        MPEG1 = 1,
        MPEG2 = 2
    };

    enum AudioMode
    {
        STEREO_MODE = 0,
        JOINT_STEREO_MODE = 1,
        DUAL_CHANNEL_MODE = 2,
        SINGLE_CHANNEL_MODE = 3
    };

    /* layer code, very bad design */
    enum AudioLayer
    {
        AUDIO_LAYER_1 = 3,
        AUDIO_LAYER_2 = 2,
        AUDIO_LAYER_3 = 1,
        AUDIO_LAYER_RESERVED = 0
    };
    friend std::ostream& operator<<(std::ostream& os, const Mp3Header& mp3);

private:
    static const unsigned short samplingFrequencyTable[2][4];
    static const short m1BitRateTable[3][16];
    static const short m2BitRateTable[3][16];
    static const unsigned short outFrameSizes[2][4];

};
#endif