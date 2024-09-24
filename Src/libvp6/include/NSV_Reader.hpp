#if !defined(NSV_READER_HPP)
#define NSV_READER_HPP
//______________________________________________________________________________
//
//  NSV_Reader.hpp
//  NSV Reader Class

#include "NSV.hpp"
#include "endian.hpp"

#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <cassert>

namespace NSV
{

    //--------------------------------------
    //  Defines the interface for the basic_Reader template instantiations
    class Reader_base
    {
    public:
        virtual ~Reader_base()
        {
        }

        virtual void open(const std::string& strFile) = 0;
        virtual void close() = 0;

        virtual File& file() = 0;
        virtual const std::string& fileName() const = 0;

        virtual void readFileHeader() = 0;
        virtual void readFrame() = 0;
        virtual void readFrameInfo() = 0;
        virtual void readFrameHeader() = 0;
        virtual void readPayload() = 0;
        virtual void readPayloadInfo() = 0;
        virtual void readPayloadHeader(int& nAux, int& iAuxPlusVideo, int& iAudio) = 0;

        virtual void buildIndex(int nIndexEntries) = 0;

        virtual INT64 frames() const = 0;
        virtual INT64 frame() const = 0;
        virtual void seek(INT64 nFrame) = 0;
        virtual void readSample(int nStream, unsigned char* pData, size_t sizeDataMax, size_t& sizeData, bool& bKeyFrame) = 0;
        virtual bool eof() = 0;

        virtual const FileHeader& fileHeader() const = 0;
        virtual const FrameHeader& frameHeader() const = 0;

        virtual void* get_ifs() = 0;
    };

    //--------------------------------------
    template<typename T>
    class basic_Reader : public Reader_base
    {
    public:
        basic_Reader(File& f);
        ~basic_Reader();

        void open(const std::string& strFile);
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

        void buildIndex(int nIndexEntries = 0);  //  index all frames by default

        INT64 frames() const;
        INT64 frame() const;
        void seek(INT64 nFrame);
        void readSample(int nStream, unsigned char* pData, size_t sizeDataMax, size_t& sizeData, bool& bKeyFrame);
        bool eof();

        const FileHeader& fileHeader() const;
        const FrameHeader& frameHeader() const;

        void* get_ifs();

    private:
        basic_Reader(const basic_Reader& r);  //  Not implemented
        basic_Reader& operator=(const basic_Reader& r);  //  Not implemented

        short read_i16();
        unsigned short read_ui16();
        int read_i32();
        unsigned int read_ui32();

        File& m_file;
        std::string m_strFile;
        T m_ifs;

        FileHeader m_fileHeader;
        FrameHeader m_frameHeader;
        bool m_bFrameHeader;

        INT64 m_nFrame;
    };

    //--------------------------------------
    template<typename T>
    basic_Reader<T>::basic_Reader(File& f) :
        m_file(f),
        m_strFile(),
        m_fileHeader(),
        m_frameHeader(),
        m_bFrameHeader(false),
        m_nFrame(0)
    {
    }

    //--------------------------------------
    template<typename T>
    basic_Reader<T>::~basic_Reader()
    {
        close();
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::open(const std::string& strFile)
    {
        m_strFile = strFile;

        m_ifs.open(m_strFile.c_str(), IOS_BASE::binary);
        if (!m_ifs)
        {
            std::ostringstream ossError;
            ossError << "Error opening file " << m_strFile;
            throw Exception(ossError.str());
        }

        readFileHeader();

        return;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::close()
    {
        if (m_ifs)
        {
            m_ifs.close();
        }

        m_strFile.erase();

        return;
    }

    //--------------------------------------
    template<typename T>
    File& basic_Reader<T>::file()
    {
        return m_file;
    }

    //--------------------------------------
    template<typename T>
    const std::string& basic_Reader<T>::fileName() const
    {
        return m_strFile;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::readFileHeader()
    {
        assert(m_ifs);

        //  Read file header signature
        char cSignature[5];
        m_ifs.read(cSignature, 4);
        if (strncmp(cSignature, "NSVf", 4) == 0)
        {
            cSignature[4] = '\0';
            m_fileHeader.m_fccSignature = cSignature;
            m_fileHeader.m_sizeHeader = read_i32();
            m_fileHeader.m_sizeFile = read_i32();
            m_fileHeader.m_iFileSize_ms = read_i32();
            m_fileHeader.m_sizeMetaData = read_i32();
            m_fileHeader.m_nTOCAlloc = read_i32();
            m_fileHeader.m_nTOCSize = read_i32();

            if ((m_fileHeader.m_sizeFile > 0 && m_fileHeader.m_sizeFile < m_fileHeader.m_sizeHeader)
                || m_fileHeader.m_nTOCSize > m_fileHeader.m_nTOCAlloc)
            {
                throw Exception("Invalid NSV file header");
            }

            if (m_fileHeader.m_sizeMetaData > 0)
            {
                std::auto_ptr<char> apcMetaData(new char[m_fileHeader.m_sizeMetaData + 1]);
                char* pcMetaData = apcMetaData.get();
                if (pcMetaData == 0)
                {
                    throw Exception("Out of memory");
                }

                m_ifs.read(pcMetaData, m_fileHeader.m_sizeMetaData);
                pcMetaData[m_fileHeader.m_sizeMetaData] = '\0';

                m_file.header(pcMetaData, m_fileHeader.m_nTOCSize);
            }
            else
            {
                m_file.header("", m_fileHeader.m_nTOCSize);
            }

            for (int nEntry = 0; nEntry < m_fileHeader.m_nTOCSize; ++nEntry)
            {
                POS_TYPE posOffset;
                posOffset = read_ui32();
                m_file.indexEntry(nEntry, posOffset);
            }

            m_ifs.ignore((m_fileHeader.m_nTOCAlloc - m_fileHeader.m_nTOCSize) * 4);

            if (m_ifs.tellg() > static_cast<POS_TYPE>(m_fileHeader.m_sizeHeader))
            {
                throw Exception("Invalid NSV file header");
            }

            m_file.dataOffset(m_fileHeader.m_sizeHeader);
            m_ifs.seekg(m_file.dataOffset());
        }
        else  //  No file header present
        {
            m_fileHeader.m_sizeHeader = 0;
            m_ifs.seekg(0, IOS_BASE::end);
            m_fileHeader.m_sizeFile = m_ifs.tellg();

            m_fileHeader.m_iFileSize_ms = 0;
            m_fileHeader.m_nTOCAlloc = 0;
            m_file.header("", 0);

            m_file.dataOffset(0);
            m_ifs.seekg(m_file.dataOffset());
        }

        //  Read stream info from first frame header
        readFrameHeader();
        int nAux;
        int iAuxPlusVideo;
        int iAudio;
        readPayloadHeader(nAux, iAuxPlusVideo, iAudio);

        m_file.size(m_frameHeader.m_iWidth, m_frameHeader.m_iHeight);
        m_file.frameRate(m_frameHeader.m_iFrameRate);

        if (m_fileHeader.m_iFileSize_ms > 0)
        {
            INT64 nFramesDenom = static_cast<INT64>(m_file.rateDenom()) * 1000;
            INT64 nFrames = (static_cast<INT64>(m_fileHeader.m_iFileSize_ms) * static_cast<INT64>(m_file.rateNum()) + nFramesDenom / 2) / nFramesDenom;
            m_file.frames(nFrames);
        }

        //  Set up primary video and audio streams
        m_file.newStream(m_frameHeader.m_fccVideo);
        m_file.newStream(m_frameHeader.m_fccAudio);
        m_file.stream(0).rate(m_file.rateNum(), m_file.rateDenom());
        m_file.stream(0).samples(m_file.frames());

        //  Set up aux streams
        for (int n = 0; n < nAux; ++n)
        {
            unsigned short uh = read_ui16();
            unsigned long ul = read_ui32();
            m_ifs.ignore(uh);

            m_file.newStream(FourCC(ul));
            //  More info ...
        }

        m_ifs.seekg(m_file.dataOffset());

        return;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::readFrame()
    {
        readFrameHeader();
        readPayload();

        return;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::readFrameInfo()
    {
        readFrameHeader();
        readPayloadInfo();

        return;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::readFrameHeader()
    {
        assert(m_ifs);

        //  Read frame header signature
        char cSignature[5];
        m_ifs.read(cSignature, 2);
        if (strncmp(cSignature, "\xef\xbe", 2) == 0)
        {
            m_frameHeader.m_fccSignature = 0UL;
            m_frameHeader.m_bKeyFrame = false;
            m_file.syncOffset(0);
            return;
        }

        m_ifs.read(&cSignature[2], 2);
        if (strncmp(cSignature, "NSVs", 4) != 0)
        {
            throw Exception("Invalid NSV frame header");
        }

        cSignature[4] = '\0';
        m_frameHeader.m_fccSignature = cSignature;
        m_ifs.read(reinterpret_cast<char*>(&m_frameHeader.m_fccVideo), 4);
        m_ifs.read(reinterpret_cast<char*>(&m_frameHeader.m_fccAudio), 4);
        m_frameHeader.m_iWidth = read_i16();
        m_frameHeader.m_iHeight = read_i16();
        unsigned char uc;
        m_ifs.read(reinterpret_cast<char*>(&uc), 1);
        m_frameHeader.m_iFrameRate = uc;
        m_frameHeader.m_iSyncOffset_ms = read_i16();
        m_frameHeader.m_bKeyFrame = true;

        if (!m_bFrameHeader)
        {
//            m_file.newStream(m_frameHeader.m_fccVideo);
//            m_file.newStream(m_frameHeader.m_fccAudio);
//            m_file.size(m_frameHeader.m_iWidth, m_frameHeader.m_iHeight);
//            m_file.frameRate(m_frameHeader.m_iFrameRate);

            m_bFrameHeader = true;
        }
        else
        {
            if ((m_file.streamVideo() >= 0 && m_file.videoFormat() != m_frameHeader.m_fccVideo)
                || (m_file.streamAudio() >= 0 && m_file.audioFormat() != m_frameHeader.m_fccAudio)
                || m_file.width() != m_frameHeader.m_iWidth
                || m_file.height() != m_frameHeader.m_iHeight
                || m_file.frameRate() != m_frameHeader.m_iFrameRate)
            {
                throw Exception("Invalid NSV frame header");
            }
        }

        m_file.syncOffset(m_frameHeader.m_iSyncOffset_ms);

        return;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::readPayload()
    {
        assert(m_ifs);

        int nAux;
        int iAuxPlusVideo;
        int iAudio;
        readPayloadHeader(nAux, iAuxPlusVideo, iAudio);

        int iAux = 0;

        for (int n = 0; n < nAux; ++n)
        {
            unsigned short uh = read_ui16();
            unsigned int ui = read_ui32();

            Stream& s = m_file.stream(m_file.streamAux(n));
            s.dataSize(uh);
            m_ifs.read(reinterpret_cast<char*>(s.data()), uh);

            iAux += uh;
        }

        if (m_file.streamVideo() >= 0)
        {
            int iVideo = iAuxPlusVideo - iAux;
            Stream& sVideo = m_file.stream(m_file.streamVideo());
            sVideo.dataSize(iVideo);
            m_ifs.read(reinterpret_cast<char*>(sVideo.data()), iVideo);
            sVideo.keyFrame(m_frameHeader.m_bKeyFrame);
        }
        else
        {
            m_ifs.seekg(iAuxPlusVideo - iAux, IOS_BASE::cur);
        }

        if (m_file.streamAudio() >= 0)
        {
            Stream& sAudio = m_file.stream(m_file.streamAudio());
            sAudio.dataSize(iAudio);
            m_ifs.read(reinterpret_cast<char*>(sAudio.data()), iAudio);
            sAudio.keyFrame(true);
        }
        else
        {
            m_ifs.seekg(iAudio, IOS_BASE::cur);
        }

        return;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::readPayloadInfo()
    {
        assert(m_ifs);

        int nAux;
        int iAuxPlusVideo;
        int iAudio;
        readPayloadHeader(nAux, iAuxPlusVideo, iAudio);

        int iAux = 0;

        for (int n = 0; n < nAux; ++n)
        {
            unsigned short uh = read_ui16();
            unsigned int ui = read_ui32();
            m_ifs.ignore(uh);

            Stream& s = m_file.stream(m_file.streamAux(n));
            s.dataSize(uh);

            iAux += uh;
        }

        if (m_file.streamVideo() >= 0)
        {
            int iVideo = iAuxPlusVideo - iAux;
            Stream& sVideo = m_file.stream(m_file.streamVideo());
            sVideo.dataSize(iVideo);
            sVideo.keyFrame(m_frameHeader.m_bKeyFrame);
        }

        if (m_file.streamAudio() >= 0)
        {
            Stream& sAudio = m_file.stream(m_file.streamAudio());
            sAudio.dataSize(iAudio);
            sAudio.keyFrame(true);
        }

        return;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::readPayloadHeader(int& nAux, int& iAuxPlusVideo, int& iAudio)
    {
        assert(m_ifs);

        char c;
        unsigned short uh;
        unsigned short uhAudio;

        m_ifs.get(c);
        uh = read_ui16();
        uhAudio = read_ui16();

        nAux = c & 0xf;
        iAuxPlusVideo = (static_cast<int>(uh) << 4) | ((c >> 4) & 0xf);
        iAudio = uhAudio;

        return;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::buildIndex(int nIndexEntries)
    {
        assert(nIndexEntries == 0);  //  Only creates full index for now ...

        m_file.index().clear();
        m_file.frames(0);

        m_ifs.seekg(m_file.dataOffset());

        INT64 nFrames = 0;
        for (; !eof(); ++nFrames)
        {
            m_file.appendIndexEntry(static_cast<POS_TYPE>(m_ifs.tellg()) - m_file.dataOffset());

            readFrameHeader();
            int nAux;
            int iAuxPlusVideo;
            int iAudio;
            readPayloadHeader(nAux, iAuxPlusVideo, iAudio);
            m_ifs.seekg(iAuxPlusVideo + iAudio, IOS_BASE::cur);
        }

        m_file.frames(nFrames);

        m_ifs.seekg(m_file.dataOffset());

        return;
    }

    //--------------------------------------
    template<typename T>
    INT64 basic_Reader<T>::frames() const
    {
        return m_file.frames();
    }

    //--------------------------------------
    template<typename T>
    INT64 basic_Reader<T>::frame() const
    {
        return m_nFrame;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::seek(INT64 nFrame)
    {
        assert(m_ifs);
        INT64 nFrames = m_file.frames();
        assert(nFrame < nFrames || nFrames == -1);

        int nIndexEntries = m_file.index().size();

        if (nIndexEntries > 0)
        {
            int nIndexEntry = nIndexEntries * nFrame / nFrames;
            INT64 nFrameIndex = (nIndexEntry * nFrames + nIndexEntries / 2) / nIndexEntries;

            m_ifs.seekg(m_file.dataOffset() + m_file.index()[nIndexEntry].m_posOffset);

            for (; nFrameIndex < nFrame; ++nFrameIndex)
            {
                readFrameHeader();
                int nAux;
                int iAuxPlusVideo;
                int iAudio;
                readPayloadHeader(nAux, iAuxPlusVideo, iAudio);
                m_ifs.seekg(iAuxPlusVideo + iAudio, IOS_BASE::cur);
            }

            m_nFrame = nFrame;
        }
        else
        {
            m_ifs.seekg(m_file.dataOffset());

            for (m_nFrame = 0; m_nFrame < nFrame; ++m_nFrame)
            {
                readFrameHeader();
                int nAux;
                int iAuxPlusVideo;
                int iAudio;
                readPayloadHeader(nAux, iAuxPlusVideo, iAudio);
                m_ifs.seekg(iAuxPlusVideo + iAudio, IOS_BASE::cur);
            }

            assert(m_nFrame == nFrame);
        }

        return;
    }

    //--------------------------------------
    template<typename T>
    void basic_Reader<T>::readSample(int nStream, unsigned char* pData, size_t sizeDataMax, size_t& sizeData, bool& bKeyFrame)
    {
        assert(m_ifs);
        assert(pData != 0);

        readFrame();

        Stream& s = m_file.stream(nStream);

        size_t size = s.dataSize();
        if (sizeDataMax < s.dataSize())
        {
            size = sizeDataMax;
        }

        memcpy(pData, s.data(), size);
        sizeData = s.dataSize();

        bKeyFrame = s.keyFrame();

        return;
    }

    //--------------------------------------
    template<typename T>
    bool basic_Reader<T>::eof()
    {
        return m_ifs.tellg() >= m_fileHeader.m_sizeFile;
    }

    //--------------------------------------
    template<typename T>
    const FileHeader& basic_Reader<T>::fileHeader() const
    {
        return m_fileHeader;
    }

    //--------------------------------------
    template<typename T>
    const FrameHeader& basic_Reader<T>::frameHeader() const
    {
        return m_frameHeader;
    }

    //--------------------------------------
    template<typename T>
    void* basic_Reader<T>::get_ifs()
    {
        return &m_ifs;
    }

    //--------------------------------------
    template<typename T>
    short basic_Reader<T>::read_i16()
    {
        assert(m_ifs);

        short i16;
        m_ifs.read(reinterpret_cast<char*>(&i16), 2);

        return native_endian(i16, false);
    }

    //--------------------------------------
    template<typename T>
    unsigned short basic_Reader<T>::read_ui16()
    {
        assert(m_ifs);

        unsigned short ui16;
        m_ifs.read(reinterpret_cast<char*>(&ui16), 2);

        return native_endian(ui16, false);
    }

    //--------------------------------------
    template<typename T>
    int basic_Reader<T>::read_i32()
    {
        assert(m_ifs);

        int i32;
        m_ifs.read(reinterpret_cast<char*>(&i32), 4);

        return native_endian(i32, false);
    }

    //--------------------------------------
    template<typename T>
    unsigned int basic_Reader<T>::read_ui32()
    {
        assert(m_ifs);

        unsigned int ui32;
        m_ifs.read(reinterpret_cast<char*>(&ui32), 4);

        return native_endian(ui32, false);
    }

}  //  namespace NSV

#endif  //  NSV_READER_HPP
