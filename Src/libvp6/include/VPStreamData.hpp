#if !defined(VPSTREAMDATA_HPP)
#define VPSTREAMDATA_HPP
//______________________________________________________________________________
//
//  VPStreamData.hpp
//  

//______________________________________________________________________________
//  Include Files and Forward Declarations

#include "PlayerModel.hpp"
#include <vector>
#include <iosfwd>

namespace on2vp
{

//______________________________________________________________________________
//  Macro, Enumeration, and Constant Definitions

//______________________________________________________________________________
//  Type, Struct, and Class Definitions

    //--------------------------------------
    class StreamData
    {
        friend std::ostream& operator<<(std::ostream& os, const StreamData& sd);

    public:

        StreamData();
        StreamData(const unsigned char* const pData, unsigned long ulSize);
        StreamData(const StreamData& sd);
        ~StreamData();

        StreamData& operator=(const StreamData& sd);

        unsigned long versionOrig() const;
        unsigned long sizeOrig() const;
        unsigned long version() const;
        unsigned long size() const;
        const unsigned char* data() const;

        int data(const unsigned char* pData, unsigned long ulSize);

        //  Interpreted data

        const PlayerModel& playerModel() const;
        PlayerModel& playerModel();

        bool encrypted() const;
        void encrypted(bool bEncrypted);

    private:
        class VersionInfo
        {
        public:
            VersionInfo(long lVersion, long lSize, bool bExtra) :
                m_lVersion(lVersion),
                m_lSize(lSize),
                m_bExtra(bExtra)
            {
            }

            long version() const
            {
                return m_lVersion;
            }

            long size() const
            {
                return m_lSize;
            }

            bool extra() const
            {
                return m_bExtra;
            }

        private:
            long m_lVersion;
            long m_lSize;
            bool m_bExtra;
        };

        enum
        {
            VersionMax = 6
        };

        void init_();
        int extract_(const unsigned char* pData, unsigned long ulSize);
        void update_();

        static std::vector<VersionInfo> m_vVersionInfo;

        unsigned long m_ulVersionOrig;
        unsigned long m_ulSizeOrig;

        unsigned char* m_pData;
        long m_lDataSize;

        //  Interpreted data

        PlayerModel m_pm;
        bool m_bEncrypted;
    };

//______________________________________________________________________________
//  Object and Function Declarations

//______________________________________________________________________________
//  Object and Function Definitions

}  //  namespace on2vp

#endif  //  VPSTREAMDATA_HPP
