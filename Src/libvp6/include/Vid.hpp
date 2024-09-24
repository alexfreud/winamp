#ifndef VID_HPP
#define VID_HPP

#pragma warning (disable:4786)

#include <windows.h>
#include <cstdlib>
#include <string>
#include <map>
#include <exception>

namespace Vid
{

    const enum Format { uyvy }; // format = uyvy;

    enum { rate = 2997, scale = 100 };

    class FileError : public exception
    {
    public:
        explicit FileError(const char* message);
        explicit FileError(DWORD id);
        const char* what() const;
    private:
        std::string message;
    };


    bool nameIsOK(const char* name);

    typedef __int64 offset_type;
    typedef unsigned __int64 size_type;

    class File
    {
    public:

        static const Format format;

        enum mode_t {in, out};

        File();
        File(const char* name, mode_t mode, int iWidth = 0, int iHeight = 0);
        ~File();

        void open(
            const char* name, 
            mode_t mode, 
            DWORD flags = FILE_ATTRIBUTE_NORMAL);

        void close();

        void reset(int frameNum) const;
        void advance(int count) const;

        void read(void* frame) const;
        void write(const void* frame);

        bool isOpen() const;

        bool eof() const;

        mode_t mode() const;

        const char* name() const;

        int frameNum() const;

        int frameCount() const;

        size_type size() const;

        void dimensions(int iWidth, int iHeight);
        int width() const;
        int height() const;
        int frameSize() const;

    private:

        File& operator=(const File&);
        File(const File&);

        HANDLE m_handle;

        //  When opening for write with FILE_FLAG_OVERLAPPED
        HANDLE m_hEvent;
        OVERLAPPED m_overlapped;

        mutable int frameNum_;

        int frameCount_;

        char name_[_MAX_PATH];

        mode_t mode_;

        int m_iWidth;
        int m_iHeight;
        int m_iFrameSize;
    };


    class MappedFile
    {
    public:

        //For now, just implement read-only.

        MappedFile();
        MappedFile(const char* name, int iWidth = 0, int iHeight = 0);

        ~MappedFile();

        void open(const char* name);
        void close();

        const char* name() const;

        int frameCount() const;

        void* map(int frameNum) const;

        void unmap(int frameNum) const;
        void unmap() const;

        bool isMapped(int frameNum) const;
        int mapCount() const;

        void dimensions(int iWidth, int iHeight);
        int width() const;
        int height() const;
        int frameSize() const;

    private:

        MappedFile(const MappedFile&);
        MappedFile& operator=(const MappedFile&);

        void init(int iWidth = 0, int iHeight = 0);

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

        std::string m_name;

        int m_iWidth;
        int m_iHeight;
        int m_iFrameSize;
    };

}

#endif
