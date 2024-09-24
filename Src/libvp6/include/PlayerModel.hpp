#if !defined(PLAYERMODEL_HPP)
#define PLAYERMODEL_HPP
//______________________________________________________________________________
//
//  PlayerModel.hpp
//  

#include <string>
#include <exception>
#include <iosfwd>

namespace on2vp
{

    //--------------------------------------
    class PlayerModel
    {
        friend std::ostream& operator<<(std::ostream& os, const PlayerModel& pm);

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

        //  Any changes made to AspectRatio need to be reflected in XSAspectRatio defined in On2XS.h and On2XS.bas
        enum AspectRatio
        {
            AR_Null,
            AR_PC,
            AR_NTSC,
            AR_PAL
        };

        enum
        {
            Auto = -2,
            Null = -1
        };

        PlayerModel(long lRateNum = 0, long lRateDenom = 0);
        PlayerModel(const PlayerModel& pm);
        ~PlayerModel();

        PlayerModel& operator=(const PlayerModel& pm);

        long bitrate() const;
        long bufferMax() const;
        long prebufferMax() const;
        bool bufferPad() const;
        long bufferRequired() const;
        long prebufferRequired() const;
        long rebufferRequired() const;
        AspectRatio sourceAspectRatio() const;
        int frameWidth() const;
        int frameHeight() const;

        void bitrate(long lBitrate);
        void bufferMax(long lBufferMax);
        void prebufferMax(long lPrebufferMax);
        void bufferPad(bool bBufferPad);
        void bufferRequired(long lBufferRequired);
        void prebufferRequired(long lPrebufferRequired);
        void rebufferRequired(long lRebufferRequired);
        void frameSize(AspectRatio arSource, int iFrameWidth, int iFrameHeight);

        void rate(long lRateNum, long lRateDenom);
        void initialize(int iWidthOrig, int iHeightOrig);
        void finalize();

        void updateBuffer(long lBytesOut, long nSamples, bool bKeyFrame);
        long bufferPadding(long lBytesOut, long nSamples);

    private:

        void initializeInternal();

        void updateBuffer0(long lBytesOut, long nSamples, bool bKeyFrame);
        void updateBuffer1(long lBytesOut, long nSamples, bool bKeyFrame);  //  Given bitrate, calculate buffer and prebuffer required
        void updateBuffer2(long lBytesOut, long nSamples, bool bKeyFrame);  //  Given buffer and prebuffer size, calculate bitrate

        long m_lBitrate;
        long m_lBufferMax;
        long m_lPrebufferMax;
        bool m_bBufferPad;
        long m_lBufferRequired;
        long m_lPrebufferRequired;
        long m_lRebufferRequired;
        AspectRatio m_arSource;
        int m_iFrameWidth;
        int m_iFrameHeight;

        long m_lRateNum;
        long m_lRateDenom;
        double m_dBytesIn;
        double m_dBufferFilled;  //  Used to calculate buffer required and prebuffer required
        double m_dBufferFilled2;  //  Used to calculate rebuffer required
        bool m_bInitialized;
        bool m_bInitializedInternal;
        bool m_bFinalized;

        void (PlayerModel::*m_updateBuffer)(long lBytesOut, long nSamples, bool bKeyFrame);
    };

}  //  namespace on2vp

#endif  //  PLAYERMODEL_HPP
