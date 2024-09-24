//=====================================================================
//
// Copyright (c) 1999-2003  On2 Technologies Inc. All Rights Reserved. 
//
//---------------------------------------------------------------------
//
//  File:     $Workfile: AVI.hpp$
//
//  Date:     $Date: 2010/07/23 19:10:47 $
//
//  Revision: $Revision: 1.1 $
//
//---------------------------------------------------------------------

#ifndef AVI_HPP
#define AVI_HPP

#pragma warning(disable:4786)

#include "FourCC.hpp"
#include <exception>
#include <iosfwd>
#include <list>
#include <deque>
#include <string>
#include <vector>

#if defined WIN32
#include <windows.h>
#endif

namespace AVI
{
#if defined WIN32
    typedef unsigned __int64 size_type;
    typedef DWORD dword;
    typedef __int64 offset_t;
    typedef unsigned __int32 length_t;
#elif defined LINUX
    typedef unsigned long long size_type;
    typedef unsigned long dword;
    typedef long long offset_t;
    typedef unsigned int length_t;
#endif

    int asStreamId(const FourCC&);

    enum ChunkType
    {
        waveform,
        waveform_encrypted,
        DIB_compressed,
        DIB_uncompressed,
        DIB_encrypted,
        kChunkTypeUnknown
    };

    ChunkType asChunkType(const FourCC&);

    const FourCC asChunkId(int stream, ChunkType type);
    const FourCC asIndexChunkExId(int stream);

    size_type estimatedFileSize(
        int width,
        int height,
        int frameCount);

    const std::string offtoa(offset_t);

    class FileError : public std::exception
    {
    public:
        FileError(dword messageId);
        FileError(const char* message);
        ~FileError() throw();
        const char* what() const throw();
        dword id() const;
    private:
        std::string message;
        dword m_id;
    };



    struct MainHeader
    {
        enum Flag
        {
            hasIndex        = 0x00000010,
            mustUseIndex    = 0x00000020,
            isInterleaved   = 0x00000100,
            indexIsAbsolute = 0x00000800,  //? "trust cktype"
            wasCaptureFile  = 0x00010000,
            copyrighted     = 0x00020000
        };

        dword microSecPerFrame;
        dword maxBytesPerSec;
        dword paddingGranularity;
        dword flags;
        dword totalFrames;
        dword initialFrames;
        dword streams;
        dword suggestedBufferSize;
        dword width;
        dword height;
        dword reserved[4];

        const std::string flagsAsStr() const;
    };

    std::ostream& operator<<(std::ostream&, const MainHeader&);


    class Chunk
    {
    public:

        Chunk(const FourCC, length_t, const unsigned char* data = 0);

        const FourCC fcc() const;

        length_t length() const;

        const unsigned char* data() const;
        unsigned char* data();

        void data(const unsigned char* d);

    private:
        FourCC m_fcc;
//        length_t m_length;
//        unsigned char* m_data;
        
        typedef std::vector<unsigned char> data_t;
        data_t m_data;
    };

    std::ostream& operator<<(std::ostream& os, const Chunk&);

    typedef std::vector<Chunk> ExtraHeaderVector;



    struct Rectangle
    {
        typedef unsigned short T;

        T left;
        T top;
        T right;
        T bottom;

        Rectangle()
            : left(0), top(0), right(0), bottom(0)
        {
        }

        Rectangle(T l, T t, T r, T b)
            : left(l), top(t), right(r), bottom(b)
        {
        }
    };


    struct StreamHeader
    {
        enum Flag
        {
            disabled      = 0x00000001,
            formatChanges = 0x00010000
        };

        FourCC fccType;
        FourCC fccHandler;
        dword flags;
        unsigned short priority;
        unsigned short language;
        dword initialFrames;
        dword scale;
        dword rate;
        dword start;
        dword length;
        dword suggestedBufferSize;
        long quality;
        dword sampleSize;
        Rectangle frame;

        const std::string flagsAsStr() const;
    };

    std::ostream& operator<<(std::ostream&, const StreamHeader&);


    struct BitmapInfoHeader
    {
        dword size;
        long width;
        long height;
        unsigned short planes;
        unsigned short bitCount;
        FourCC compression;
        dword sizeImage;
        long xPelsPerMeter;
        long yPelsPerMeter;
        dword clrUsed;
        dword clrImportant;
    };

    std::ostream& operator<<(std::ostream&, const BitmapInfoHeader&);


//    namespace Compression
//    {
//        enum CompressionType
//        {
//            RGB,
//            RLE8,
//            RLE4,
//            bitfields,
//            unknown
//        };
//
//        bool operator==(CompressionType, const FourCC&);
//        bool operator==(const FourCC&, CompressionType);
//
//        CompressionType asCompression(const FourCC&);
//        const FourCC asFourCC(CompressionType);
//
//        std::ostream& operator<<(std::ostream&, CompressionType);
//    }


    struct PCMWaveFormat
    {
        unsigned short formatTag;
        unsigned short nChannels;
        dword samplesPerSec;
        dword avgBytesPerSec;
        unsigned short blockAlign;
        unsigned short bitsPerSample;
    };

    struct WaveFormatEx : public PCMWaveFormat
    {
        typedef std::vector<unsigned char> ByteArray;
        ByteArray extra;
    };

    std::ostream& operator<<(std::ostream&, const WaveFormatEx&);




    //not currently used; it's for palette changes,
    //which isn't implemented yet
    struct RGBQuad
    {
        unsigned char blue;
        unsigned char green;
        unsigned char red;
        unsigned char reserved;
    };


    struct IndexEntry
    {
        enum Flags
        {
            list     = 0x00000001,
            keyframe = 0x00000010,
            notime   = 0x00000100,
            compuse  = 0x0FFF0000
        };

        FourCC chunkId;
        dword flags;
        dword chunkOffset;
        dword chunkLength;

        const std::string flagsAsStr() const;
    };

    std::ostream& operator<<(std::ostream&, const IndexEntry&);

    typedef std::vector<IndexEntry> IEVector;


    struct FrameIndexEntry
    {
        union
        {
            offset_t offset;

            struct
            {
                unsigned long offset_low;
                unsigned long offset_high;
            };
        };

        size_t size;
        bool keyframe;
    };

    typedef std::vector<FrameIndexEntry> FrameIEVector;
    typedef std::list<FrameIndexEntry> FrameIEList;

    typedef std::deque<FrameIndexEntry> FrameIndex;


    struct IndexChunkEx
    {
        FourCC code;
        unsigned long length;
        unsigned short longsPerEntry;
        unsigned char subtype;
        unsigned char type;
        unsigned long entriesInUse;
        FourCC chunkId;
        unsigned long reserved[3];
    };

    std::ostream& operator<<(std::ostream&, const IndexChunkEx&);


    struct StandardIndexChunk
    {
        FourCC code;
        unsigned long length;
        unsigned short longsPerEntry;
        unsigned char subtype;
        unsigned char type;
        unsigned long entriesInUse;
        FourCC chunkId;
        unsigned long baseOffset_low;
        unsigned long baseOffset_high;
        unsigned long reserved;

        struct Entry
        {
            unsigned long offset;
            unsigned long size;
        } index[1];
    };

    std::ostream& operator<<(std::ostream&, const StandardIndexChunk&);
    std::ostream& operator<<(std::ostream&, const StandardIndexChunk::Entry&);


    struct SuperIndexChunk
    {
        FourCC code;
        unsigned long length;
        unsigned short longsPerEntry;
        unsigned char subtype;
        unsigned char type;
        unsigned long entriesInUse;
        FourCC chunkId;
        unsigned long reserved[3];

        struct Entry
        {
            offset_t offset;
            unsigned long size;
            unsigned long duration;
        } index[1];
    };

    std::ostream& operator<<(std::ostream&, const SuperIndexChunk&);
    std::ostream& operator<<(std::ostream&, const SuperIndexChunk::Entry&);


    class File
    {
    public:

        enum mode_t {in, out, inout};

        enum OutputType
        {
            OT_AVI,
            OT_On2
        };

        File();
        File(const char* name, mode_t mode);

        ~File();

        void open(const char* name, mode_t mode, dword flags = 0);
        void outputType(OutputType ot);

        void close();

        bool isOpen() const;
        mode_t mode() const;
        const char* name() const;

        void mapinit();
        void mapfinal();

        unsigned long map(
            offset_t offset,
            length_t size,
            unsigned char*& base,
            length_t& baseSize,
            length_t& offsetInView) const;

        void unmap(unsigned char* base, length_t size) const;

        const MainHeader& mainHeader() const;
        MainHeader& mainHeader();

        int extraHeaderCount() const;
        const Chunk& extraHeader(int nChunk) const;

        void extraHeader(FourCC fcc, length_t length, unsigned char* data);
        void extraHeader(int nStream, FourCC fcc, length_t length, unsigned char* data);

        dword totalFrames() const;
        dword& totalFrames();

        int streamCount() const;

        int makeStreamHeaderVideo(int superIndexEntryCount = 0);
        int makeStreamHeaderAudio(int superIndexEntryCount = 0);

        const StreamHeader& streamHeader(int stream) const;
        StreamHeader& streamHeader(int stream);

        const unsigned char* strf(int nStream) const;
        size_t strfSize(int nStream) const;

        const BitmapInfoHeader& videoFormat(int stream) const;
        BitmapInfoHeader& videoFormat(int stream);

        const WaveFormatEx& audioFormat(int stream) const;
        WaveFormatEx& audioFormat(int stream);

        dword streamDataSize(int stream) const;
        const unsigned char* streamData(int stream) const;
        void setStreamData(int nStream, dword sds, const unsigned char* psd);

        const char* streamName(int stream) const;
        void setStreamName(int nStream, const char* psn);

        SuperIndexChunk& superIndexChunk(int stream);
        const SuperIndexChunk& superIndexChunk(int stream) const;

        int superIndexEntryCount(int stream) const;

        offset_t tell() const;

        void seek(offset_t) const;

        void seekCurrent(offset_t) const;

        offset_t dataOffset() const;

        offset_t idx1Offset() const;
        length_t idx1Size() const;

        void rewriteHeaders();
        //For use by header updaters, throws if position of movi list
        //changes, positions at beginning of data.

        void seekMainHeader();
        void writeMainHeader();

        length_t seekStreamHeader(int stream);
        void writeStreamHeader(int stream);

        void seekVideoFormat(int stream);
        void writeVideoFormat(int stream);

        void seekAudioFormat(int stream);
        void writeAudioFormat(int stream);

        void seekStreamData(int stream);
        void writeStreamData(int stream);

        void seekSuperIndexChunk(int stream);
        void writeSuperIndexChunk(int stream);

        int indexCount() const;
        void seekIndex() const;
        void read(IndexEntry&) const;

        void load(int stream, IEVector& index) const;
        void load(int stream, FrameIndex& index) const;

        void loadIndex(int, FrameIndex&) const;
        void loadIndexEx(int, FrameIndex&) const;

        void writeIndexChunkHeader(int number_of_index_entries);
        
        void write(const IndexEntry&) const;

        void writeIndexChunk(const IEVector& index);

        void writeIndexChunk(
            const FourCC& chunkId,
            const FrameIndex& index);

        void writeStandardIndexChunk(
            int stream,
            const FrameIndex& index,
            offset_t baseOffset);

        void writeStandardIndexChunk(
            int stream,
            const FrameIndex& index);

        size_t makeSegment();
        int segmentCount() const;
        offset_t segmentOffset() const;

        const FourCC readFourCC() const;
        void writeFourCC(const FourCC&);

        const FourCC testFourCC() const;

        length_t readLength() const;
        void writeLength(length_t length);

        void read(void* buffer, size_t size) const;
        void write(const void* data, size_t size, bool adjust = true);

        void writeJunkChunk(length_t);

        int countIndexEntries(int stream) const;

        bool indexIsRelative() const;

        offset_t size() const;

    private:

        File(const File& rhs);
        File& operator=(const File& rhs);

        //void readUnknownChunk() const;
        void readJunkChunk() const;

        bool readInit();
        bool readHeaderList();
        void readMainHeader();
        void readExtraHeaders();
        void readStreamHeaderList();
        void readStreamHeader(StreamHeader& h);
        void readStreamVideoFormat(BitmapInfoHeader& f);
        struct StreamInfoVideo;
        void readStreamVideoFormat(StreamInfoVideo* psiv);
        void readStreamAudioFormat(WaveFormatEx& f);
        void readStreamName(std::string& name);

        void readExtendedAVIHeader();
        void writeExtendedAVIHeader();

        bool readDataList();
        void readDataRecChunk() const;
        void readDataChunk() const;

        void readIndexList();

        void writeInit();
        void writeFinal();

        void writeHeader();
        void writeStreamHeaderList(int stream);
        void writeStreamHeader(const StreamHeader& h);

        void writeStreamVideoFormatChunk(const BitmapInfoHeader& f);
        void writeStreamVideoFormat(const BitmapInfoHeader& f);

        void writeStreamVideoFormatChunk(const unsigned char* pData, size_t sizeData);
        void writeStreamVideoFormat(const unsigned char* pData, size_t sizeData);

        void writeStreamAudioFormatChunk(const WaveFormatEx&);
        void writeStreamAudioFormat(const WaveFormatEx&);

        int headerLength() const;
        int streamHeaderLength(int stream) const;

        void load(const SuperIndexChunk::Entry&, FrameIndex&) const;

        class handle_t
        {
        public:

            handle_t();

            void open(const char*, mode_t, dword) throw (FileError);
            void close();

            bool isOpen() const;
            offset_t size() const;

            void read(void*, size_t) const;
            void write(const void*, size_t) const;

            void truncate() const;

            void seekCurrent(offset_t) const;
            void seek(offset_t) const;

            offset_t tell() const;

            void mapinit();
            void mapfinal();

            unsigned long map(
                offset_t offset,
                length_t size,
                unsigned char*& view,
                length_t& viewSize,
                length_t& offsetWithinView) const;

            void unmap(unsigned char*, length_t) const;

        private:
#if defined WIN32
            HANDLE m_hFile;
            HANDLE m_hFileMappingObject;
#elif defined LINUX
            int m_fd;
#endif
        };

        handle_t m_handle;

        mode_t m_mode;

        std::string m_name;

        OutputType m_ot;

        MainHeader m_mainHeader;

        ExtraHeaderVector m_extraHeaderVector;

        class indx_t
        {
        public:

            indx_t();
            indx_t(int entryCount);

            ~indx_t();

            int entryCount() const;

            size_t size() const;

            operator SuperIndexChunk&() const;

            void read(File&);
            void write(File&) const;

        private:

            indx_t(const indx_t&);
            indx_t& operator=(const indx_t&);

            dword* m_rep;            
            
        };

        struct StreamInfo
        {
            virtual ~StreamInfo();

            void write(File&) const;

            int length() const;

            StreamHeader header;

            typedef std::vector<unsigned char> data_t;

            data_t m_data;

            std::string m_name;

            ExtraHeaderVector m_extraHeaderVector;

            indx_t m_indx;

        protected:
            
            StreamInfo(int);
            StreamInfo(const StreamHeader&);

            virtual int strf_length() const = 0;
            virtual void strf_write(File&) const = 0;

        private:

            StreamInfo(const StreamInfo&);
            StreamInfo& operator=(const StreamInfo&);

        };

        struct StreamInfoVideo : public StreamInfo
        {
            StreamInfoVideo(int entryCount);
            StreamInfoVideo(const StreamHeader&);

            ~StreamInfoVideo();

//            BitmapInfoHeader m_strf;
            unsigned char* m_strf;
            size_t m_strfSize;
        protected:
            int strf_length() const;
            void strf_write(File&) const;
        };

        struct StreamInfoAudio : public StreamInfo
        {
            StreamInfoAudio(int entryCount);
            StreamInfoAudio(const StreamHeader&);
            
            WaveFormatEx m_strf;
        protected:
            int strf_length() const;
            void strf_write(File&) const;
        };

        friend struct StreamInfo;
        friend struct StreamInfoVideo;
        friend struct StreamInfoAudio;

        void readStreamData(StreamInfo::data_t&);

        typedef std::vector<StreamInfo*> infoVector_t;
        infoVector_t infoVector;

        dword m_totalFrames;

        offset_t m_dataPosn;

        offset_t m_indexPosn;
        int m_indexCount;
        length_t m_idx1Size;

        struct SegmentInfo
        {
            offset_t offset;
            size_t size;

            SegmentInfo() {}
            SegmentInfo(offset_t offset_) : offset(offset_) {}
        };

        typedef std::vector<SegmentInfo> SegmentInfoVector;
        SegmentInfoVector m_segmentInfo;
    };

#if defined WIN32
    class MappedFile
    {
    public:

        enum { invalid_offset = -1 };

        MappedFile();
        MappedFile(const char* name);

        ~MappedFile();

        void open(const char* name);
        void close();

        bool isOpen() const;
        const char* name() const;

        const MainHeader& mainHeader() const;

        const StreamHeader& streamHeader(int stream) const;

        const BitmapInfoHeader& videoFormat(int stream) const;

        const WaveFormatEx& audioFormat(int stream) const;

        const char* streamName(int stream) const;

        dword totalFrames() const;

        offset_t dataOffset() const;

        offset_t indexOffset() const;
        size_t indexSize() const;

        offset_t indexChunkExOffset(int stream) const;
        size_t indexChunkExSize(int stream) const;

        const void* map(offset_t, size_t) const;
        void unmap(const void*) const;

        void load(int stream, FrameIEVector& index) const;

    private:

        MappedFile(const MappedFile&);
        MappedFile& operator=(const MappedFile&);

        void init();
        void unmapAllViews();

        offset_t offset() const;
        void setFilePointerCurrent(LONG) const;

        const FourCC readFourCC() const;
        const FourCC queryFourCC() const;

        dword readLength() const;

        void readHeaderList();
        void readDataList();
        void readIndexList();
        void readJunkChunk() const;
        void readUnknownChunk() const;
        void readMainHeaderChunk();
        void readStreamHeaderList(int stream);
        void readStreamHeaderChunk(StreamHeader&) const;
        void readStreamVideoFormatChunk(BitmapInfoHeader&) const;
        void readStreamAudioFormatChunk(WaveFormatEx&) const;
        void readStreamNameChunk(std::string&) const;
        void readIndexChunkEx(offset_t&, size_t&) const;
        void readExtendedAVIHeaderList() const;

        std::string m_name;

        HANDLE m_file;
        HANDLE m_fileMappingObject;

        DWORD m_allocationGranularity;

        MainHeader m_mainHeader;

        struct StreamInfo
        {
            StreamHeader streamHeader;

            union
            {
                BitmapInfoHeader* videoFormat;
                WaveFormatEx* audioFormat;
            };

            std::string name;

            offset_t indexChunkExOffset;
            size_t indexChunkExSize;
        };


        typedef std::vector<StreamInfo> StreamInfoVector;
        StreamInfoVector m_streamInfoVector;

        size_t readChunkSize(offset_t) const;
        int countEntries(int, const IndexEntry*, int) const;
        int countEntries(const SuperIndexChunk&) const;
        void loadIndex(int, FrameIEVector&) const;
        void loadIndexEx(int, FrameIEVector&) const;
        void load(const SuperIndexChunk::Entry&, FrameIEVector&) const;

        mutable dword m_totalFrames;

        offset_t m_dataOffset;

        offset_t m_indexOffset;
        size_t m_indexSize;

        struct ViewInfo
        {
            unsigned char* pView;
            unsigned char* pChunk;
        };

        typedef std::list<ViewInfo> Views;
        mutable Views m_views;

        Views::iterator findView(const void*) const;
    };
#endif

}

//inline HANDLE AVI::File::handle() const
//{
//    return m_handle;
//}

inline AVI::Chunk::Chunk(
    const FourCC fcc,
    length_t length,
    const unsigned char* d)
    : m_fcc(fcc)
{
//    if (m_length > 0)
//    {
//        m_data = new unsigned char[m_length];
//        if (m_data == 0)
//        {
//            throw FileError("Error allocating Chunk data.");
//        }
//        if (data != 0)
//        {
//            memcpy(m_data, data, m_length);
//        }
//    }

    if (length)
    {
        if (d)
        {
            //typedef data_t::const_iterator iter_t;

            //const iter_t first = iter_t(d);

            //const data_t::size_type n = length;

            //const iter_t last = first + n;

            m_data.assign(d, d + length);
        }
        else
        {
            const data_t::size_type n = length;

            m_data.assign(n, 0);
        }
    }
    else
    {
        m_data.assign(data_t::size_type(0), 0);
    }
}

inline const FourCC AVI::Chunk::fcc() const
{
    return m_fcc;
}

inline AVI::length_t AVI::Chunk::length() const
{
    const data_t::size_type n = m_data.size();

    return n;
}

inline const unsigned char* AVI::Chunk::data() const
{
    return &m_data[0];
}

inline unsigned char* AVI::Chunk::data()
{
    return &m_data[0];
}

inline void AVI::Chunk::data(const unsigned char* d)
{
    //typedef data_t::const_iterator iter_t;

    //const iter_t first = iter_t(d);

    //const data_t::size_type n = m_data.size();

    //const iter_t last = first + n;

    m_data.assign(d, d + m_data.size());
}

inline AVI::dword AVI::FileError::id() const
{
    return m_id;
}


#endif
