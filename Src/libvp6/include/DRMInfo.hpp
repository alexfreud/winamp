#if !defined(DRMINFO_HPP)
#define DRMINFO_HPP
//______________________________________________________________________________
//
//  DRMInfo.hpp
//  

//______________________________________________________________________________
//  Include Files and Forward Declarations

#include <string>
#include <exception>
#include <iosfwd>
#include "FourCC.hpp"

namespace on2vp
{

//______________________________________________________________________________
//  Macro, Enumeration, and Constant Definitions

//______________________________________________________________________________
//  Type, Struct, and Class Definitions

    //--------------------------------------
    class DRMInfo
    {
        friend std::ostream& operator<<(std::ostream& os, const DRMInfo& drmi);

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
        DRMInfo();
        DRMInfo(const DRMInfo& drmi);
        ~DRMInfo();

        DRMInfo& operator=(const DRMInfo& drmi);

        const FourCC scheme() const;
        long scope() const;
        long amount() const;
        const unsigned char* data() const;
        long dataSize() const;
        const unsigned char* drmx() const;
        long drmxSize() const;

        void scheme(FourCC fccScheme);
        void scope(long lScope);
        void amount(long lAmount);
        void data(const unsigned char* pData, long lDataSize);

        void init(FourCC fccScheme, long lScope, long lAmount, const unsigned char* pData, long lDataSize);
        void drmx(const unsigned char* pDRMX, long lDRMXSize);

    private:
        enum
        {
            DRMXHeaderSize = 16
        };

        FourCC m_fccScheme;
        long m_lScope;
        long m_lAmount;
        unsigned char* m_pData;
        long m_lDataSize;
        mutable unsigned char* m_pDRMX;
        long m_lDRMXSize;
    };

//______________________________________________________________________________
//  Object and Function Declarations

//______________________________________________________________________________
//  Object and Function Definitions

}  //  namespace on2vp

#endif  //  DRMINFO_HPP
