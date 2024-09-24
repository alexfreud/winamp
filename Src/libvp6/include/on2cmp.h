#ifndef ON2CMP_H
#define ON2CMP_H
//*********************************************************************
// name : on2comp.h
// desc : Stand-alone command line compression application
//        that takes as input rvd files that are in planar yv12 format
//        and produces as output one ore more files that contain one or
//        more vp31 compressed video streams, as determined by command
//        line parameters.
// date : 04/21/00
// Who  : Jim Bankoski
// Mods :
// Sep 21 2000 Reworked entire structure to be cleaner and more modular
//*********************************************************************

#include <vector>

typedef int (*ProgressFunction)
(
    int Frame,
    double SecondsWritten,
    double PercentComplete,
    int BytesWritten,
    std::vector<int> vBytesWritten,
    std::vector<int> vMinBufferSize
);

class fileCompressor
{
public:
    enum fileCompressorType
    {
        VP31,
        VP40,
        VP50
    };

    virtual void compress (
        ProgressFunction pf,
        int updateEvery,
        int argc,
        char* argv[],
        int outFileType) = 0;

    virtual ~fileCompressor();
    static fileCompressor* Make(fileCompressorType);
};

#endif  //  ON2CMP_H
