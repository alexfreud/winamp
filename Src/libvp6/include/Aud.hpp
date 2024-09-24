#ifndef AUD_HPP
#define AUD_HPP

#include <exception>
#include <string>
#include <io.h>

namespace Aud
{
    class FileError : public exception
    {
    public:
        explicit FileError(const char* message);
        const char* what() const;
    private:
        const std::string message;
    };


    enum { samplesPerSec = 48000 };

    enum { sampleSizeInBits = 16, sampleSizeInBytes = 2 };

    enum { numberOfChannels = 2 };

    enum { blockAlign = numberOfChannels * sampleSizeInBytes };


    class File
    {
    public:

        enum mode_t { in, out };

        File();
        File(const char* name, mode_t mode);

        ~File();

        void open(const char* name, mode_t mode);
        void close();

        bool isOpen() const;
        bool eof() const;

        mode_t mode() const;
        const char* name() const;

        int sampleCount() const;

        void seekSample(int sampleNum) const;

        size_t read(void* buffer, size_t size) const;

        void write(const void* buffer, size_t size);

        typedef __int64 offset_t;
        offset_t offset() const;

        void seekOffset(offset_t) const;

    private:

        File(const File&);
        File& operator=(const File&);

        void init();

        int handle_;

        char name_[_MAX_PATH];
        mode_t mode_;

        int m_sampleCount;
    };    

} //end namespace Aud

#endif
