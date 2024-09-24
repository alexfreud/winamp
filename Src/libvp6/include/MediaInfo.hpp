#if !defined(MEDIAINFO_HPP)
#define MEDIAINFO_HPP
//______________________________________________________________________________
//
//  MediaInfo.hpp
//  

#include "FourCC.hpp"

#include <string>
#include <exception>
#include <iosfwd>

namespace on2vp
{

    //--------------------------------------
    class MediaInfo
    {
        friend std::ostream& operator<<(std::ostream& os, const MediaInfo& mi);

    public:
        class Exception : public std::exception
        {
        public:
            Exception(const std::string& strText);
            ~Exception() throw();
            const char* what() const throw();
        private:
            std::string m_strText;
        };

        MediaInfo();
        MediaInfo(const MediaInfo& mi);
        ~MediaInfo();

        MediaInfo& operator=(const MediaInfo& mi);

        void parse(const std::string& strMediaInfo);

        const unsigned char* data() const;
        unsigned long dataSize() const;

    private:

        enum
        {
            DataSizeMax = 16384
        };

        void init_();
        void copy_(const MediaInfo& mi);
        void extract_(const std::string& strMediaInfo);
        void update_();
        unsigned long append_(FourCC fcc, const std::string& strData, char*& pData);

        std::string m_strArchivalLocation;
        std::string m_strArtist;
        std::string m_strCommissioned;
        std::string m_strComments;
        std::string m_strCopyright;
        std::string m_strCreationDate;
        std::string m_strCropped;
        std::string m_strDimensions;
        std::string m_strDotsPerInch;
        std::string m_strEngineer;
        std::string m_strGenre;
        std::string m_strKeywords;
        std::string m_strLightness;
        std::string m_strMedium;
        std::string m_strName;
        std::string m_strPaletteSetting;
        std::string m_strProduct;
        std::string m_strSubject;
        std::string m_strSoftware;
        std::string m_strSharpness;
        std::string m_strSource;
        std::string m_strSourceForm;
        std::string m_strTechnician;

        unsigned char* m_pData;
        unsigned long m_ulDataSize;
    };

}  //  namespace on2vp

#endif  //  MEDIAINFO_HPP
