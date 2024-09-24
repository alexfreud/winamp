#if !defined(NSV_HPP)
#define NSV_HPP
//______________________________________________________________________________
//
//  NSV.hpp
//  NSV File I/O Classes

#include <FourCC.hpp>

#include <exception>
#include <string>
#include <vector>
#include <fstream>
#include <iosfwd>

namespace NSV
{

#if (defined(__POWERPC__) || defined(__APPLE__))
    typedef long long INT64;
    typedef long long POS_TYPE;
    typedef std::ios IOS_BASE;
#elif (defined(__linux) || defined(linux))
    typedef long long  INT64;  //  Assumes WIN32
    typedef std::ios::pos_type POS_TYPE;
    typedef std::ios_base IOS_BASE;
#else
    typedef __int64 INT64;  //  Assumes WIN32
    typedef std::ios::pos_type POS_TYPE;
    typedef std::ios_base IOS_BASE;
#endif

    enum
    {
        FrameRates = 256
    };

    double frameRate(int nFrameRate);
    int frameRate(unsigned int uiRateNum, unsigned int uiRateDenom);
    bool video(FourCC fccNSV);
    bool audio(FourCC fccNSV);
    FourCC videoFormat(FourCC fccCompression);
    FourCC audioFormat(unsigned short wFormatTag);
    FourCC videoCompression(FourCC fccNSV);
    int videoBitCount(FourCC fccNSV);
    unsigned short audioFormatTag(FourCC fccNSV);

    //--------------------------------------
    class Exception : public std::exception
    {
    public:
        Exception();
        Exception(const std::string& strException);
        Exception(const Exception& e);
        ~Exception() throw();

        Exception& operator=(const Exception& e);

        const char* what() const throw();

    private:
        std::string m_strException;
    };

    //--------------------------------------
    class IndexEntry
    {
    public:
        IndexEntry(POS_TYPE posOffset = 0, size_t sizeData = -1, bool bKeyFrame = false) :
            m_posOffset(posOffset),
            m_sizeData(sizeData),
            m_bKeyFrame(bKeyFrame)
        {
        }

    //private:
        POS_TYPE m_posOffset;
        size_t m_sizeData;
        bool m_bKeyFrame;
    };

    typedef std::vector<IndexEntry> Index;

    //--------------------------------------
    class FileHeader
    {
    public:
        friend std::ostream& operator<<(std::ostream& os, const FileHeader& fh);

        FileHeader() :
            m_fccSignature(0UL),
            m_sizeHeader(0),
            m_sizeFile(0),
            m_iFileSize_ms(0),
            m_sizeMetaData(0),
            m_nTOCAlloc(0),
            m_nTOCSize(0),
            m_strMetaData(),
            m_index()
        {
        }

    //private:
        FourCC m_fccSignature;
        size_t m_sizeHeader;
        size_t m_sizeFile;
        int m_iFileSize_ms;
        size_t m_sizeMetaData;
        int m_nTOCAlloc;
        int m_nTOCSize;
        std::string m_strMetaData;
        std::vector<size_t> m_index;
    };

    //--------------------------------------
    class FrameHeader
    {
    public:
        friend std::ostream& operator<<(std::ostream& os, const FrameHeader& fh);

        FrameHeader() :
            m_fccSignature(0UL),
            m_fccVideo(0UL),
            m_fccAudio(0UL),
            m_iWidth(0),
            m_iHeight(0),
            m_iFrameRate(0),
            m_iSyncOffset_ms(0),
            m_bKeyFrame(false)
        {
        }

    //private:
        FourCC m_fccSignature;
        FourCC m_fccVideo;
        FourCC m_fccAudio;
        int m_iWidth;
        int m_iHeight;
        int m_iFrameRate;
        int m_iSyncOffset_ms;
        bool m_bKeyFrame;
    };

    class Stream;

    //--------------------------------------
    class File
    {
        friend std::ostream& operator<<(std::ostream& os, const File& f);

    public:
        File();
        ~File();

        //  File header
        void header(const std::string& strMetaData, int nIndexEntries);
        void metaData(const std::string& strMetaData);
        const std::string& metaData() const;

        void indexEntry(int nEntry, POS_TYPE posOffset, size_t sizeData = -1, bool bKeyFrame = false);
        void appendIndexEntry(POS_TYPE posOffset, size_t sizeData = -1, bool bKeyFrame = false);
        Index& index();  //  Relative index
        int indexEntriesUsed() const;

        //  Synchronization frame header
        FourCC videoFormat() const;
        FourCC audioFormat() const;

        void size(int iWidth, int iHeight);
        int width()const;
        int height() const;

        void frameRate(unsigned char ucFrameRate);
        unsigned char frameRate() const;

        void rate(unsigned int uiRateNum, unsigned int uiRateDenom);
        unsigned int rateNum() const;
        unsigned int rateDenom() const;

        void syncOffset(int iSyncOffset_ms);
        int syncOffset() const;
        void finalizeSyncOffset();

        void frames(INT64 nFrames);
        INT64 frames() const;

        Stream& newStream(FourCC fccDataType, unsigned int uiRateNum = 0, unsigned int uiRateDenom = 0, INT64 nSamples = -1);
        int streams() const;
        int streamsAux() const;
        Stream& stream(int nStream);
        const Stream& stream(int nStream) const;

        int streamVideo() const;
        int streamAudio() const;
        int streamAux(int nAux) const;

        void dataOffset(POS_TYPE posDataOffset);
        POS_TYPE dataOffset();

    private:
        File(const File& f);  //  Not implemented
        File& operator=(const File& f);  //  Not implemented

        std::string m_strMetaData;
        Index m_index;
        int m_nIndexEntriesUsed;

        int m_iWidth;
        int m_iHeight;

        unsigned int m_uiRateNum;
        unsigned int m_uiRateDenom;
        unsigned char m_ucFrameRate;

        int m_iSyncOffset_ms;

        std::vector<Stream> m_vStream;

        POS_TYPE m_posDataOffset;

        int m_nStreamVideo;
        int m_nStreamAudio;

        INT64 m_nFrames;
    };

    //--------------------------------------
    class Stream
    {
        friend std::ostream& operator<<(std::ostream& os, const Stream& s);

    public:
        Stream(File* pFile, int nStream, FourCC fccDataType, unsigned int uiRateNum, unsigned int uiRateDenom, INT64 nSamples);
        Stream(const Stream& s);
        ~Stream();

        Stream& operator=(const Stream& s);

        void open();
        void close();

        int stream() const;
        FourCC type() const;  //  Returns AVI types, vids, auds, ...
        FourCC dataType() const;

        void samples(INT64 nSamples);
        INT64 samples() const;

        void sample(INT64 nSample);
        INT64 sample() const;

        void rate(unsigned int uiRateNum, unsigned int uiRateDenom);
        unsigned int rateNum() const;
        unsigned int rateDenom() const;

        void dataSize(size_t sizeData);
        void data(const unsigned char* pData, size_t sizeData);
        unsigned char* data();
        size_t dataSize() const;

        void keyFrame(bool bKeyFrame);
        bool keyFrame() const;


    private:
        File* m_pFile;
        int m_nStream;
        FourCC m_fccDataType;
        INT64 m_nSamples;
        INT64 m_nSample;

        unsigned int m_uiRateNum;
        unsigned int m_uiRateDenom;

        unsigned char* m_pBuffer;
        size_t m_sizeBuffer;
        size_t m_sizeData;

        bool m_bKeyFrame;
    };

/*
    //--------------------------------------
    class Reader
    {
    public:
        Reader(File& f);
        Reader(File& f, const std::string& strFile);
        ~Reader();

        void open(const std::string& strFile);
        void open();
        void close();

        File& file();
        const std::string& fileName() const;

        void readFileHeader();
        void readFrame();
        void readFrameInfo();
        void readFrameHeader();
        void readPayload();
        void readPayloadInfo();
        void readPayloadHeader(int& nAux, int& iAuxPlusVideo, int& iAudio);

        INT64 frames() const;
        INT64 frame() const;
        void seek(INT64 nFrame);
        void readSample(int nStream, unsigned char* pData, size_t sizeDataMax, size_t& sizeData, bool& bKeyFrame);
        bool eof();

        const FileHeader& fileHeader() const;
        const FrameHeader& frameHeader() const;

    private:
        Reader(const Reader& r);  //  Not implemented
        Reader& operator=(const Reader& r);  //  Not implemented

        short read_i16();
        unsigned short read_ui16();
        int read_i32();
        unsigned int read_ui32();

        File& m_file;
        std::string m_strFile;
        std::ifstream m_ifs;

        FileHeader m_fileHeader;
        FrameHeader m_frameHeader;
        bool m_bFrameHeader;

        INT64 m_nFrame;
    };
*/

    //--------------------------------------
    class Writer
    {
    public:
        Writer(File& f);
        Writer(File& f, const std::string& strFile);
        ~Writer();

        void open(const std::string& strFile);
        void open();
        void close();

        File& file();
        const std::string& fileName() const;

        INT64 frame() const;
        void seek(POS_TYPE posOffset);

        void header(const std::string& strMetaData, int nIndexEntries);
        void sample(int nStream, const unsigned char* pData, size_t sizeData, bool bKeyframe);
        void writeFrame();

    private:
        Writer(const Writer& w);  //  Not implemented
        Writer& operator=(const Writer& w);  //  Not implemented

        void writeFileHeader();
        void writeFrameHeader(bool bKeyFrame);
        void writePayloadHeader();

        void write_i16(short i16);
        void write_ui16(unsigned short ui16);
        void write_i32(int i32);
        void write_ui32(unsigned int ui32);

        File& m_file;
        std::string m_strFile;
        std::ofstream m_ofs;

        INT64 m_nFrame;
        bool m_bKeyFrame;
    };

}  //  namespace NSV

#endif  //  NSV_HPP
