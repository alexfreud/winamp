#ifndef WAV_HPP
#define WAV_HPP

#include "FourCC.hpp"
#include <io.h>
#include <exception>
#include <string>
#include <iosfwd>
#include <vector>


namespace WAV
{
    class FileError : public exception
    {
    public:
        explicit FileError(const char* message);
        const char* what() const;
    private:
        const std::string message;
    };


    struct Format
    {
        unsigned short formatTag;
        unsigned short nChannels;
        unsigned long samplesPerSec;
        unsigned long avgBytesPerSec;
        unsigned short blockAlign;
        unsigned short bitsPerSample;

        typedef std::vector<unsigned char> ByteArray;
        ByteArray extra;
    };

    std::ostream& operator<<(std::ostream&, const Format&);
    
    typedef __int64 offset_t;


    class File
    {
    public:

        enum mode_t { in, out, inout };

        File();
        File(const char* name, mode_t mode);

        ~File();

        void open(const char* name, mode_t mode);
        void close();

        bool isOpen() const;
        bool eof() const;

        //size_t sampleNum() const;
        //size_t sampleCount() const;

        //void seekSample(size_t sampleNum) const;

        offset_t offset() const;
        void seekOffset(offset_t) const;

        size_t read(void* buffer, size_t size) const;
        void write(const void* buffer, size_t size);

        mode_t mode() const;
        const char* name() const;

        const Format& format() const;
        Format& format();

        void seekFormat() const;

        //void readFormat() const;
        void readFormatChunk() const;

        void readFactChunk() const;

        void setFactSize(size_t);
        size_t factSize() const;

        void seekFact() const;

        void writeFact(const void*, size_t);
        void readFact(void* buffer, size_t size) const;

        void writeFormat();
        void writeFormatChunk();

        size_t dataSize() const;

    private:

        File(const File&);
        File& operator=(const File&);

        void init();

        void seek(__int64, int) const;

        const FourCC queryId() const;
        const FourCC readId() const;

        void writeId(const char* id);

        void writeSize(size_t size);
        size_t readSize() const;

        int handle_;

        __int64 dataPosn;
        size_t m_dataSize;

        //size_t m_sampleCount;

        char name_[_MAX_PATH];
        mode_t mode_;

        mutable Format format_;

        mutable size_t m_factSize;
    };    

}

#endif
