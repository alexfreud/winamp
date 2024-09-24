#ifndef AC3_HPP
#define AC3_HPP
//______________________________________________________________________________
//
//  AC3.hpp
//  

//______________________________________________________________________________
//

#pragma warning(disable:4786)

#include <windows.h>
#include <string>
#include <vector>
#include <exception>
#include <iosfwd>
#include <cstdio>
extern "C"
{
    #include "ac3.h"
    #include "bitstream.h"
}

namespace AC3
{

//______________________________________________________________________________
//

    typedef __int64 offset_t;

    //--------------------------------------
    enum
    {
        SamplesPerBlock = 256 * 6
    };

    //--------------------------------------
    class FileError : public std::exception
    {
        public:
            FileError(DWORD messageId);
            FileError(const char* message);
            const char* what() const;
        private:
            std::string m_strMessage;
    };

    //--------------------------------------
    struct Header
    {
        unsigned long m_nBlocks;
        unsigned long m_ulSamplesPerSecond;
        unsigned long m_ulSamplesPerBlock;
        unsigned long m_ulBytesPerBlock;  //  blockAlign
    };

    std::ostream& operator<<(std::ostream& os, const Header& h);

    //--------------------------------------
    struct SyncFrame
    {
        syncinfo_t m_si;
        bsi_t m_bsi;
    };

    std::ostream& operator<<(std::ostream& os, const SyncFrame& sf);

    //--------------------------------------
    class File
    {
    public:

        enum mode_t {in, out, inout};

        typedef std::vector<SyncFrame> VectorSyncFrame;

        File();
        File(const char* szName, mode_t mode);

        ~File();

        void open(const char* szName, mode_t mode);
        void close();

        bool isOpen() const;
        bool eof() const;

        const char* name() const;
        mode_t mode() const;

        unsigned long blockCount() const;
        unsigned long samplesPerSecond() const;
        unsigned long samplesPerBlock() const;

        int samplesPerSec() const;

        const Header& header() const;

//        const SyncFrame& syncFrame(int nSyncFrame) const;
//        SyncFrame& syncFrame(int nSyncFrame);

    private:

        File(const File& rhs);  //  Not implemented
        File& operator=(const File& rhs);  //  Not implemented

        int readHeader();

        void readSyncFrame(SyncFrame& sf);

        void writeSyncFrame(const SyncFrame& sf);

        void read(void* buffer, size_t size) const;
        void write(const void* data, size_t size);

        offset_t size() const;
        void seekCurrent(__int64) const;
        void seek(offset_t) const;
        offset_t tell() const;


        FILE* m_pFile;
        int m_hFile;
        bitstream_t* m_pbs;

        std::string m_strName;
        mode_t m_mode;

        Header m_header;
        VectorSyncFrame m_vSyncFrame;

        __int64 m_fileSize;
        __int64 m_fileOffset;
    };

}  //  namespace AC3

#endif  //  AC3_HPP
