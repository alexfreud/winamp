#ifndef MP3_HPP
#define MP3_HPP
//______________________________________________________________________________
//
//  MP3.hpp
//  

//______________________________________________________________________________
//

#pragma warning(disable:4786)

#include "mp3header.hpp"
#include <windows.h>
#include <string>
#include <exception>
#include <iosfwd>

namespace MP3
{

//______________________________________________________________________________
//

    typedef __int64 offset_t;

    //--------------------------------------
    class Exception : public std::exception
    {
        public:
            Exception(DWORD dwMessage);
            Exception(const char* szMessage);
            const char* what() const;
        private:
            std::string m_strMessage;
    };

    //--------------------------------------
    struct Header
    {
        unsigned long m_ulChannels;
        unsigned long m_ulSamplesPerSecond;
        unsigned long m_ulSamplesPerBlock;
        unsigned long m_ulBytesPerBlock;
        unsigned long m_ulBlocks;

        void clear();
    };

    std::ostream& operator<<(std::ostream& os, const Header& h);

    //--------------------------------------
    class File
    {
    public:

        enum mode_t {in, out, inout};

        File();
        File(const char* szName, mode_t mode);

        ~File();

        void open(const char* szName, mode_t mode, DWORD dwFlags = 0);
        void close();

        bool isOpen() const;
        bool eof() const;

        const char* name() const;
        mode_t mode() const;

        unsigned long channels() const;
        unsigned long samplesPerSecond() const;
        unsigned long samplesPerBlock() const;
        unsigned long bytesPerBlock() const;
        unsigned long blocks() const;

        const Header& header() const;

        void read(void* pBuffer, size_t size) const;
        void write(const void* pBuffer, size_t size);

        void seek(offset_t) const;

    private:

        File(const File& f);  //  Not implemented
        File& operator=(const File& f);  //  Not implemented

        int readHeader();

        offset_t size() const;
        offset_t tell() const;

        HANDLE m_handle;

        std::string m_strName;
        mode_t m_mode;

        Header m_header;

        offset_t m_fileSize;
        offset_t m_fileOffset;
    };

}  //  namespace MP3

#endif  //  MP3_HPP
