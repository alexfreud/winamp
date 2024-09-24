#if !defined(VFWSETTING_HPP)
#define VFWSETTING_HPP
//______________________________________________________________________________
//
//  VFWSetting.hpp
//  

#include "FourCC.hpp"
#include <iosfwd>

namespace on2vp
{

    //--------------------------------------
    class VFWSetting
    {
        friend std::ostream& operator<<(std::ostream& os, const VFWSetting& vfws);

    public:

        enum Mode
        {
            M_Setting,
            M_Config
        };

        enum
        {
            HeaderSize = 8,
            Size = 16
        };

        VFWSetting(FourCC fcc);
        ~VFWSetting();

        FourCC fcc() const;
        Mode mode() const;

        int setting() const;
        int value() const;
        void settingValue(int iSetting, int iValue);  //  Sets mode to M_Setting

        long size() const;
        const void* data() const;
        int data(const void* pData, unsigned long ulSize);

    private:

        VFWSetting(const VFWSetting& vfws);  //  Not implemented
        VFWSetting& operator=(const VFWSetting& vfws);  //  Not implemented

        int extract_(const void* pData, unsigned long ulSize);
        void update_() const;

        FourCC m_fcc;
        Mode m_mode;
        int m_iSetting;
        int m_iValue;

        mutable unsigned char m_pData[Size];
    };

}  //  namespace on2vp

#endif  //  VFWSETTING_HPP
