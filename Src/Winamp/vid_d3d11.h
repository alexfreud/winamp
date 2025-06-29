#pragma once

#include "videooutputbase.h" // Assuming a base class like this exists or will be created based on videooutput.h
#include "../libs/dx11_engine/D3D11Graphics.h" // Path to our new DX11 engine
#include <d3d11.h>

// Forward declarations if necessary
struct SubsItem;
typedef struct tagRGBQUAD RGBQUAD;

class Direct3D11VideoOutput : public VideoOutputBase // Or whatever the base class is
{
public:
    Direct3D11VideoOutput(HWND parentHwnd, VideoAspectAdjuster* adjuster);
    virtual ~Direct3D11VideoOutput();

    // --- Interface from base video output class (guessed based on Direct3DVideoOutput from vid_d3d.cpp) ---
    virtual int OpenVideo(int w, int h, unsigned int type, int flipit, double aspectratio) override;
    virtual void displayFrame(const char* buf, int size, int time) override;
    virtual void close() override;
    virtual int onPaint(HWND hwnd) override;
    virtual void Refresh() override;
    virtual void timerCallback() override;
    virtual void drawSubtitle(SubsItem* item) override;
    virtual void resetSubtitle() override;
    virtual void setPalette(RGBQUAD* pal) override;
    virtual void setVFlip(int on) override;
    virtual void OnWindowSize() override;
    // --- End of guessed interface ---

private:
    void Cleanup();
    bool CreateVideoSurfaces(int w, int h, unsigned int type);
    void ReleaseVideoSurfaces();
    void RenderFrameToSwapChain(const char* buf);

    HWND m_hwnd;
    VideoAspectAdjuster* m_adjuster;
    D3D11Graphics m_d3d11Graphics; // Our D3D11 helper

    ID3D11Texture2D* m_videoTexture;    // Texture to hold the current video frame
    ID3D11ShaderResourceView* m_videoTextureSRV; // SRV for the video texture

    // Shaders for YUV->RGB conversion if needed
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader*  m_pixelShaderYUV; // Example for one YUV format
    ID3D11InputLayout*  m_inputLayout;
    ID3D11Buffer*       m_vertexBuffer;
    ID3D11SamplerState* m_samplerState;

    // Video properties
    int m_videoWidth;
    int m_videoHeight;
    unsigned int m_videoInputFourCC; // e.g., MAKEFOURCC('Y','V','1','2')
    DXGI_FORMAT m_videoTextureFormat; // DXGI format of m_videoTexture
    bool m_flipOutput;

    bool m_isOpen;
    bool m_surfacesReady;

    // For OSD/Subtitles (placeholder - will need more complex integration)
    // For example, using FW1FontWrapper or similar for text.
    // Potentially reuse or adapt parts of IVideoD3DOSD interface.
};
