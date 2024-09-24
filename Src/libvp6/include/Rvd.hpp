#ifndef RVD_HPP
#define RVD_HPP

#pragma warning(disable:4786)

#include "FourCC.hpp"
//#include <io.h>
#include <windows.h>
#include <exception>
#include <cstdlib>
#include <cstddef>
#include <string>
#include <map>

namespace Rvd
{
    class FileError : public exception
    {
    public:
        explicit FileError(const char*);
        explicit FileError(DWORD);
        const char* what() const;
    private:
        std::string message;        
    };

    bool nameIsOK(const char* name);

    enum Format { rgb24, rgb32, yuv12 };
    const char* asStr(Format f);

    struct Header
    {
        FourCC code;
        __int32 width;
        __int32 height;
        __int32 blockSize;
        FourCC compression;
        __int32 unknown;  //frame count?
        __int32 rate;
        __int32 scale;
        __int8 pad[480];
    };

    typedef unsigned __int64 size_type;

    size_type estimatedFileSize(
        int width,
        int height,
        Format format,
        int frameCount);


    class File
    {
    public:

        enum mode_t {in, out, inout};

        File();
        File(const char* name, mode_t mode);

        ~File();

        void open(const char* name, mode_t mode, DWORD flags = 0);
        void close();
        
        void setFormat(Format);
        
        void setWidth(int w);
        void setHeight(int h);

        void setRate(int rate);
        void setScale(int scale);

        void reset(int frameNum) const;
        void advance(int count) const;

        void read(void* frame) const;
        void write(const void* frame);

        void writeHeader();

        bool isOpen() const;
        bool eof() const;

        mode_t mode() const;
        const char* name() const;

        int frameNum() const;

        int frameCount() const;
        bool frameCountIsOK() const;

        Format format() const;
        const FourCC compression() const;

        int width() const;
        int height() const;

        int rate() const;
        int scale() const;

        size_t frameSize() const;
        size_t paddedFrameSize() const;

    private:

        File& operator=(const File&);
        File(const File&);

        //int file;
        HANDLE m_handle;

        mutable int frameNum_;
        int frameCount_;
        bool frameCountIsOK_;
        char name_[_MAX_PATH];
        mode_t mode_;
 
        Header* const m_header;
        Format format_;
        size_t frameSize_;
        size_t paddedFrameSize_;
        size_t paddingSize_;

        bool unbuffered;
    };


    class MappedFile
    {
    public:

        MappedFile();
        MappedFile(const char* name);

        ~MappedFile();

        void open(const char* name);
        void close();

        bool isOpen() const;
        const char* name() const;

        int frameCount() const;

        Format format() const;

        int width() const;
        int height() const;

        int rate() const;
        int scale() const;

        size_t frameSize() const;

        void* map(int frameNum) const;

        void unmap(int frameNum) const;
        void unmap() const;

        bool isMapped(int frameNum) const;

    private:

        MappedFile(const MappedFile&);
        MappedFile& operator=(const MappedFile&);

        void init();

        Header header;
        Format m_format;
        size_t m_frameSize;
        size_t m_paddedFrameSize;

        std::string m_name;

        HANDLE file;
        HANDLE mapping;

        DWORD allocationGranularity;

        int m_frameCount;

        struct ViewInfo
        {
            void* view;
            void* frame;
        };

        typedef std::map<int, ViewInfo> Views;
        mutable Views views;

    };
}

#endif
