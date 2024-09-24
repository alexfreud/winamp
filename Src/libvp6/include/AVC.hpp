#ifndef AVC_HPP
#define AVC_HPP

namespace AVC
{
    struct Extra
    {
        unsigned __int16 samplesPerBlock;
        unsigned __int16 blocksPerChunk;
        unsigned __int32 version;
        unsigned __int32 datarate;
        unsigned __int32 flags;
    };

    enum 
    { 
        kFormatTag_SingleBlockPerChunk = 0x0500,
        kFormatTag_MultipleBlocksPerChunk = 0x0501
    };

    enum 
    { 
        kArchaicFlag = 1,
        kFancyFlag = 2,
        kLooseFlag = 4
    };
}

#endif
