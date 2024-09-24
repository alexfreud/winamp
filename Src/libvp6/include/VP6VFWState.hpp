//------------------------------------------------------------------------------
//
//  Copyright (c) 1999-2003  On2 Technologies Inc.  All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  $Workfile: VP6VFWState.hpp$
//  $Date: 2010/07/23 19:10:48 $
//  $Revision: 1.1 $
//
//------------------------------------------------------------------------------

#if !defined(VP6VFWSTATE_HPP)
#define VP6VFWSTATE_HPP

#include "FourCC.hpp"
#include <iosfwd>

namespace on2vp
{

    //--------------------------------------
    class VP6VFWState
    {
        friend std::ostream& operator<<(std::ostream& os, const VP6VFWState& vfws);

    public:

        enum Mode
        {
            M_Setting,
            M_Config
        };

        enum
        {
            HeaderSize = 8
        };

        enum
        {
            ConfigUsed = 1724
        };

        struct VPConfig
        {
            unsigned int Used;
            int Width;
            int Height;
            int TargetBitRate;
            int Quality;
            int RateNum;
            int RateDenom;
            int KeyFrameFrequency;
            int KeyFrameDataTarget;
            int AutoKeyFrameEnabled;
            int AutoKeyFrameThreshold;
            int MinimumDistanceToKeyFrame;
            int ForceKeyFrameEvery;
            int NoiseSensitivity;
            int AllowDF;
            int AllowSpatialResampling;
            int HScale;
            int HRatio;
            int VScale;
            int VRatio;
            int ScalingMode;
            int QuickCompress;
            int Speed;
            int Interlaced;
            int FixedQ;
            int StartingBufferLevel;
            int OptimalBufferLevel;
            int DropFramesWaterMark;
            int ResampleDownWaterMark;
            int ResampleUpWaterMark;
            int OutputFrameRate;
            int ErrorResilientMode;
            int Profile;
            int DisableGolden;
            int VBMode;
            int BestAllowedQ;
            int UnderShootPct;
            int MaxAllowedDatarate;
            int MaximumBufferSize;
            int TwoPassVBREnabled;
            int TwoPassVBRBias;
            int TwoPassVBRMaxSection;
            int TwoPassVBRMinSection;
            int Pass;
            int Mode;
            int EndUsage;
            int Sharpness;
            char FirstPassFile[512];
            char SettingsFile[512];
            char RootDirectory[512];

            char Reserved[2048 - ConfigUsed];

            VPConfig() :
                Used(ConfigUsed)
            {
            }
        };

        VP6VFWState(FourCC fcc);
        ~VP6VFWState();

        FourCC fcc() const;
        Mode mode() const;

        static size_t nominalSize();

        VPConfig& vpConfig();

        size_t vpStateSize() const;
        const void* vpState() const;
        void vpState(const void* pVPState, size_t sizeVPState);

        size_t size() const;
        const void* data() const;
        int data(const void* pData, size_t sizeData);

    private:

        VP6VFWState(const VP6VFWState& vfws);  //  Not implemented
        VP6VFWState& operator=(const VP6VFWState& vfws);  //  Not implemented

        int extract_(const void* pData, size_t sizeData);
        void update_(const void* pVPState, size_t sizeVPState) const;

        FourCC m_fcc;
        Mode m_mode;

        VPConfig m_vpConfig;

        mutable void* m_pData;
        mutable size_t m_sizeData;
    };

}  //  namespace on2vp

#endif  //  VP6VFWSTATE_HPP
