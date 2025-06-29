#include "vid_d3d11.h"
#include "main.h" // For HWND, config variables etc.
#include "api.h"  // For WASABI_API_MEMMGR etc.
#include <windowsx.h> // For GET_X_LPARAM, GET_Y_LPARAM if needed for OSD later

// For shader compilation (ensure D3DCompiler.h is available and linked)
#include <d3dcompiler.h>
#include <vector>

// A simple vertex structure for drawing a fullscreen quad
struct SimpleVertex
{
    float Pos[3];
    float Tex[2];
};

// Helper to compile shaders
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    dwShaderFlags |= D3DCOMPILE_DEBUG;
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile(szFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}


Direct3D11VideoOutput::Direct3D11VideoOutput(HWND parentHwnd, VideoAspectAdjuster* adjuster)
    : m_hwnd(parentHwnd),
      m_adjuster(adjuster),
      m_videoTexture(nullptr),
      m_videoTextureSRV(nullptr),
      m_vertexShader(nullptr),
      m_pixelShaderYUV(nullptr),
      m_inputLayout(nullptr),
      m_vertexBuffer(nullptr),
      m_samplerState(nullptr),
      m_videoWidth(0),
      m_videoHeight(0),
      m_videoInputFourCC(0),
      m_videoTextureFormat(DXGI_FORMAT_UNKNOWN),
      m_flipOutput(false),
      m_isOpen(false),
      m_surfacesReady(false)
{
    // Ensure D3D11Graphics is ready. This might be better in a separate Init method
    // called once, but for now, OpenVideo can trigger it.
    if (!m_d3d11Graphics.GetDevice() && m_hwnd) {
        RECT clientRect;
        GetClientRect(m_hwnd, &clientRect);
        if (!m_d3d11Graphics.InitializeDeviceAndSwapChain(m_hwnd,
                                                        clientRect.right - clientRect.left > 0 ? clientRect.right - clientRect.left : 320,
                                                        clientRect.bottom - clientRect.top > 0 ? clientRect.bottom - clientRect.top : 240,
                                                        false)) {
            OutputDebugStringW(L"[vid_d3d11] D3D11Graphics initialization failed in constructor safeguard.\n");
            // Handle error: perhaps throw or set an internal error state
        }
    }
}

Direct3D11VideoOutput::~Direct3D11VideoOutput()
{
    Cleanup();
}

void Direct3D11VideoOutput::Cleanup()
{
    ReleaseVideoSurfaces();
    if (m_vertexBuffer) { m_vertexBuffer->Release(); m_vertexBuffer = nullptr; }
    if (m_samplerState) { m_samplerState->Release(); m_samplerState = nullptr; }
    if (m_inputLayout) { m_inputLayout->Release(); m_inputLayout = nullptr; }
    if (m_pixelShaderYUV) { m_pixelShaderYUV->Release(); m_pixelShaderYUV = nullptr; }
    if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = nullptr; }

    m_d3d11Graphics.Shutdown();
    m_isOpen = false;
    m_surfacesReady = false;
}

void Direct3D11VideoOutput::ReleaseVideoSurfaces()
{
    if (m_videoTextureSRV) { m_videoTextureSRV->Release(); m_videoTextureSRV = nullptr; }
    if (m_videoTexture) { m_videoTexture->Release(); m_videoTexture = nullptr; }
    m_surfacesReady = false;
}

bool Direct3D11VideoOutput::CreateVideoSurfaces(int w, int h, unsigned int type)
{
    ReleaseVideoSurfaces();

    m_videoWidth = w;
    m_videoHeight = h;
    m_videoInputFourCC = type;

    // Determine texture format based on input FourCC
    switch (type) {
        case MAKEFOURCC('Y', 'V', '1', '2'):
            // For YV12, we might create three R8_UNORM textures (Y, U, V) or one NV12/R8G8_UNORM texture
            // and convert/combine in a shader or during CPU copy.
            // For simplicity in this step, let's assume we'll convert it to BGRA on CPU side
            // and use a BGRA texture, similar to how D3D9 path might handle some YUV.
            // Later, a dedicated NV12 texture + shader path would be optimal.
            m_videoTextureFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
            OutputDebugStringW(L"[vid_d3d11] Input YV12, will use BGRA8 texture (CPU conversion assumed for now).\n");
            break;
        case MAKEFOURCC('N', 'V', '1', '2'):
            m_videoTextureFormat = DXGI_FORMAT_NV12; // NV12 is directly supported by D3D11. Good candidate for direct mapping.
            OutputDebugStringW(L"[vid_d3d11] Input NV12, using NV12 texture.\n");
            break;
        case MAKEFOURCC('Y', 'U', 'Y', '2'):
            m_videoTextureFormat = DXGI_FORMAT_YUY2; // YUY2 is directly supported.
            OutputDebugStringW(L"[vid_d3d11] Input YUY2, using YUY2 texture.\n");
            break;
        case MAKEFOURCC('R', 'G', 'B', '8'): // Palettized - needs CPU conversion to RGB32
        case MAKEFOURCC('R', 'G', '2', '4'): // RGB24 - needs CPU conversion to RGB32 (or shader swizzle)
        case MAKEFOURCC('R', '5', '5', '5'): // RGB15 - needs CPU conversion
        case MAKEFOURCC('R', '5', '6', '5'): // RGB16 - needs CPU conversion
        case MAKEFOURCC('R', 'G', '3', '2'): // RGB32 (likely BGRA or ARGB)
        default: // Default to BGRA and assume CPU conversion for unsupported or other RGB formats
            m_videoTextureFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
            OutputDebugStringW(L"[vid_d3d11] Input RGB/Other, using BGRA8 texture.\n");
            break;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = m_videoWidth;
    desc.Height = m_videoHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = m_videoTextureFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DYNAMIC; // For CPU updates
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;

    ID3D11Device* device = m_d3d11Graphics.GetDevice();
    if (!device) return false;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &m_videoTexture);
    if (FAILED(hr)) {
        OutputDebugStringW(L"Failed to create video texture.\n");
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(m_videoTexture, &srvDesc, &m_videoTextureSRV);
    if (FAILED(hr)) {
        OutputDebugStringW(L"Failed to create video texture SRV.\n");
        m_videoTexture->Release();
        m_videoTexture = nullptr;
        return false;
    }

    m_surfacesReady = true;
    return true;
}


int Direct3D11VideoOutput::OpenVideo(int w, int h, unsigned int type, int flipit, double aspectratio)
{
    if (!m_hwnd) return 0; // Ensure we have a window

    // Ensure D3D11Graphics is initialized (primary initialization point)
    if (!m_d3d11Graphics.GetDevice()) {
        if (!m_hwnd) {
            OutputDebugStringW(L"[vid_d3d11] OpenVideo called with null m_hwnd.\n");
            return 0; // Cannot initialize without a window
        }
        RECT clientRect;
        GetClientRect(m_hwnd, &clientRect);
        int windowWidth = clientRect.right - clientRect.left;
        int windowHeight = clientRect.bottom - clientRect.top;
        if (windowWidth <= 0) windowWidth = 320; // Default fallback
        if (windowHeight <= 0) windowHeight = 240; // Default fallback

        if (!m_d3d11Graphics.InitializeDeviceAndSwapChain(m_hwnd, windowWidth, windowHeight, false /* TODO: handle fullscreen */)) {
            OutputDebugStringW(L"[vid_d3d11] D3D11Graphics initialization failed in OpenVideo.\n");
            return 0;
        }
    }

    if (!CreateVideoSurfaces(w, h, type)) {
        OutputDebugStringW(L"[vid_d3d11] CreateVideoSurfaces failed.\n");
        return 0;
    }

    m_flipOutput = (config_video_fliprgb ? !flipit : !!flipit);
    m_isOpen = true;

    // Create shaders and vertex buffer for drawing video (simplified)
    ID3D11Device* device = m_d3d11Graphics.GetDevice();
    if (!device) return 0;

    // Simple Vertex Shader (Pass-through)
    // TODO: Load from file or resource
    const char* vsCode =
        "struct VS_INPUT { float4 Pos : POSITION; float2 Tex : TEXCOORD0; };\n"
        "struct PS_INPUT { float4 Pos : SV_POSITION; float2 Tex : TEXCOORD0; };\n"
        "PS_INPUT VS( VS_INPUT input ) {\n"
        "    PS_INPUT output = (PS_INPUT)0;\n"
        "    output.Pos = input.Pos;\n"
        "    output.Tex = input.Tex;\n"
        "    return output;\n"
        "}";
    ID3DBlob* pVSBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;
    HRESULT hr = D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr, "VS", "vs_4_0", 0, 0, &pVSBlob, &pErrorBlob);
    if (FAILED(hr)) {
        if (pErrorBlob) OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        if (pErrorBlob) pErrorBlob->Release();
        return 0;
    }
    hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_vertexShader);
    if (FAILED(hr)) {
        pVSBlob->Release();
        return 0;
    }

    // Define input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_inputLayout);
    pVSBlob->Release();
    if (FAILED(hr)) return 0;

    // Simple Pixel Shader (Texture Sample)
    // TODO: Add YUV to RGB conversion shaders
    const char* psCode =
        "Texture2D txDiffuse : register(t0);\n"
        "SamplerState samLinear : register(s0);\n"
        "struct PS_INPUT { float4 Pos : SV_POSITION; float2 Tex : TEXCOORD0; };\n"
        "float4 PS( PS_INPUT input) : SV_Target {\n"
        "    return txDiffuse.Sample(samLinear, input.Tex);\n"
        "}";
    ID3DBlob* pPSBlob = nullptr;
    hr = D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr, "PS", "ps_4_0", 0, 0, &pPSBlob, &pErrorBlob);
    if (FAILED(hr)) {
        if (pErrorBlob) OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        if (pErrorBlob) pErrorBlob->Release();
        return 0;
    }
    hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pixelShaderYUV); // Using YUV name, but it's generic for now
    pPSBlob->Release();
    if (FAILED(hr)) return 0;


    // Create vertex buffer for a quad
    SimpleVertex vertices[] = {
        { { -1.0f,  1.0f, 0.5f }, { 0.0f, 0.0f } },
        { {  1.0f,  1.0f, 0.5f }, { 1.0f, 0.0f } },
        { { -1.0f, -1.0f, 0.5f }, { 0.0f, 1.0f } },
        { {  1.0f, -1.0f, 0.5f }, { 1.0f, 1.0f } },
    };
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices;
    hr = device->CreateBuffer(&bd, &InitData, &m_vertexBuffer);
    if (FAILED(hr)) return 0;

    // Create sampler state
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&sampDesc, &m_samplerState);
    if (FAILED(hr)) return 0;

    return 1; // Success
}

void Direct3D11VideoOutput::displayFrame(const char* buf, int size, int time)
{
    if (!m_isOpen || !m_surfacesReady || !buf) return;

    ID3D11DeviceContext* context = m_d3d11Graphics.GetImmediateContext();
    if (!context || !m_videoTexture) return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = context->Map(m_videoTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        BYTE* dest_data_start = static_cast<BYTE*>(mappedResource.pData);
        unsigned int dest_pitch = mappedResource.RowPitch;

        if (m_videoTextureFormat == DXGI_FORMAT_B8G8R8A8_UNORM)
        {
            unsigned int src_bytes_per_pixel = 4; // Default for direct BGRA copy
            bool needs_conversion = false;

            if (m_videoInputFourCC == MAKEFOURCC('R', 'G', '3', '2')) { // Assuming input is BGRA or ARGB
                src_bytes_per_pixel = 4;
            } else if (m_videoInputFourCC == MAKEFOURCC('R', 'G', '2', '4')) { // RGB24
                src_bytes_per_pixel = 3;
                needs_conversion = true;
            } else if (m_videoInputFourCC == MAKEFOURCC('Y', 'V', '1', '2') || m_videoInputFourCC == MAKEFOURCC('I', '4', '2', '0')) {
                needs_conversion = true; // Requires YUV to RGB conversion
                // nsutil_image_Convert_YUV420_RGB32 would be used here.
                // For now, fill with a debug color to indicate this path.
                for (int y = 0; y < m_videoHeight; ++y) {
                    DWORD* row = (DWORD*)(dest_data_start + y * dest_pitch);
                    for (int x = 0; x < m_videoWidth; ++x) {
                        row[x] = 0xFFFF00FF; // Magenta - indicates YV12 path hit, CPU conversion needed
                    }
                }
                OutputDebugStringW(L"[vid_d3d11] YV12 in displayFrame: CPU conversion to BGRA required.\n");
            }
            // Add more input format checks (YUY2 to BGRA, etc.) if m_videoTextureFormat is BGRA

            if (!needs_conversion) {
                unsigned int src_pitch = m_videoWidth * src_bytes_per_pixel;
                BYTE* current_dest_row = m_flipOutput ? (dest_data_start + dest_pitch * (m_videoHeight - 1)) : dest_data_start;
                ptrdiff_t dest_pitch_delta = m_flipOutput ? -(ptrdiff_t)dest_pitch : (ptrdiff_t)dest_pitch;

                for (int y = 0; y < m_videoHeight; ++y) {
                    memcpy(current_dest_row, buf + y * src_pitch, min(src_pitch, dest_pitch));
                    current_dest_row += dest_pitch_delta;
                }
            }
        }
        else if (m_videoTextureFormat == DXGI_FORMAT_NV12)
        {
            // Assuming buf contains Y plane followed by UV plane (UV interleaved)
            const BYTE* src_Y_plane = reinterpret_cast<const BYTE*>(buf);
            const BYTE* src_UV_plane = src_Y_plane + m_videoWidth * m_videoHeight; // UV plane starts after Y data

            BYTE* dest_Y_plane_start = dest_data_start;
            // For NV12, the DXGI surface might have a larger pitch than width for Y, and UV starts after Y plane taking into account this pitch.
            // Or it might be two distinct subresources if using CreateTexture2D with ArraySize=2 and specific views.
            // For a single DYNAMIC texture, it's typically one contiguous block.
            // Let's assume RowPitch is for the Y plane. UV plane starts at pData + RowPitch * height.
            BYTE* dest_UV_plane_start = dest_data_start + dest_pitch * m_videoHeight;


            // Copy Y plane
            unsigned int src_pitch_Y = m_videoWidth;
            BYTE* current_dest_Y_row = m_flipOutput ? (dest_Y_plane_start + dest_pitch * (m_videoHeight - 1)) : dest_Y_plane_start;
            ptrdiff_t dest_pitch_delta_Y = m_flipOutput ? -(ptrdiff_t)dest_pitch : (ptrdiff_t)dest_pitch;

            for (int y = 0; y < m_videoHeight; ++y) {
                memcpy(current_dest_Y_row, src_Y_plane + y * src_pitch_Y, m_videoWidth);
                current_dest_Y_row += dest_pitch_delta_Y;
            }

            // Copy UV plane (interleaved, half height)
            // For NV12, UV pitch is the same as Y pitch in terms of bytes for the combined UV row.
            // Each UV row corresponds to two Y rows.
            unsigned int src_pitch_UV = m_videoWidth; // UV row is width bytes (U/V pairs)
            BYTE* current_dest_UV_row = m_flipOutput ? (dest_UV_plane_start + dest_pitch * ((m_videoHeight / 2) - 1)) : dest_UV_plane_start;
            // dest_pitch for UV might need to be different if hardware expects specific alignment for UV plane within the same surface.
            // This is often dest_pitch (same as Y) if it's one surface.

            for (int y = 0; y < m_videoHeight / 2; ++y) {
                memcpy(current_dest_UV_row, src_UV_plane + y * src_pitch_UV, m_videoWidth);
                current_dest_UV_row += dest_pitch_delta_Y; // Use Y pitch delta here
            }
            OutputDebugStringW(L"[vid_d3d11] NV12 displayFrame: Copied Y and UV planes.\n");
        }
        else if (m_videoTextureFormat == DXGI_FORMAT_YUY2)
        {
            unsigned int src_pitch = m_videoWidth * 2; // YUY2 is 2 bytes per pixel
            BYTE* current_dest_row = m_flipOutput ? (dest_data_start + dest_pitch * (m_videoHeight - 1)) : dest_data_start;
            ptrdiff_t dest_pitch_delta = m_flipOutput ? -(ptrdiff_t)dest_pitch : (ptrdiff_t)dest_pitch;

            for (int y = 0; y < m_videoHeight; ++y) {
                memcpy(current_dest_row, buf + y * src_pitch, min(src_pitch, dest_pitch));
                current_dest_row += dest_pitch_delta;
            }
            OutputDebugStringW(L"[vid_d3d11] YUY2 displayFrame: Copied data.\n");
        }
        // Add other format handlers as necessary

        context->Unmap(m_videoTexture, 0);
    } else {
        OutputDebugStringW(L"[vid_d3d11] Failed to map video texture for displayFrame.\n");
        return;
    }

    // Trigger a render
    onPaint(m_hwnd);
}

void Direct3D11VideoOutput::RenderFrameToSwapChain(const char* buf)
{
    ID3D11DeviceContext* context = m_d3d11Graphics.GetImmediateContext();
    ID3D11RenderTargetView* rtv = m_d3d11Graphics.GetRenderTargetView();

    if (!context || !rtv || !m_vertexShader || !m_pixelShaderYUV || !m_vertexBuffer || !m_samplerState || !m_videoTextureSRV)
        return;

    // Clear the back buffer (optional, depending on whether video covers whole screen)
    // m_d3d11Graphics.ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);
    // VideoOutput::onPaint will handle clearing / letterboxing via StretchRect in D3D9 version,
    // here we'd directly render the quad.

    // Set up the graphics pipeline for drawing the video quad
    context->IASetInputLayout(m_inputLayout);
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    context->VSSetShader(m_vertexShader, nullptr, 0);
    context->PSSetShader(m_pixelShaderYUV, nullptr, 0); // Use the appropriate pixel shader for format conversion
    context->PSSetShaderResources(0, 1, &m_videoTextureSRV);
    context->PSSetSamplers(0, 1, &m_samplerState);

    // TODO: Adjust aspect ratio here by modifying vertex positions or using shader constants
    // For now, just draw a fullscreen quad. The m_adjuster would be used here.
    RECT targetRect;
    GetClientRect(m_hwnd, &targetRect); // Full window
    if (m_adjuster) m_adjuster->adjustAspect(targetRect);
    // This RECT needs to be translated into viewport settings or quad vertex modifications.
    // For simplicity, we'll ignore it for the direct quad render and assume it's handled by viewport in OnResize.

    context->Draw(4, 0);

    // OSD and Subtitles would be drawn here on top
    if (posd && ((IVideoD3DOSD *)posd)->Showing() /* && ((IVideoD3DOSD *)posd)->isOSDReadyToDraw_D3D11() - hypothetical method */)
    {
        // Placeholder: Need to adapt IVideoD3DOSD to work with D3D11 device/context
        // For example: ((IVideoD3DOSD *)posd)->DrawOSD_D3D11(m_d3d11Graphics.GetImmediateContext(), m_d3d11Graphics.GetRenderTargetView());
        OutputDebugStringW(L"[vid_d3d11] Placeholder for OSD Drawing.\n");
    }

    // drawSubtitle(current_subtitle_item_if_any); // Call the class's drawSubtitle method

    m_d3d11Graphics.Present();
}


void Direct3D11VideoOutput::close()
{
    m_isOpen = false;
    ReleaseVideoSurfaces();
    // Don't call m_d3d11Graphics.Shutdown() here, as the device might be used by other things
    // or might be intended to persist. Shutdown should be called from the destructor or a higher-level manager.
}

int Direct3D11VideoOutput::onPaint(HWND hwnd)
{
    if (!m_isOpen || !m_d3d11Graphics.GetDevice() || !m_surfacesReady)
    {
        // Draw logo or clear screen if video not open/ready
        m_d3d11Graphics.ClearRenderTarget(0.1f, 0.1f, 0.1f, 1.0f); // Dark grey
        // TODO: Draw logo using D3D11 if possible
        m_d3d11Graphics.Present();
        return 1;
    }

    RenderFrameToSwapChain(nullptr); // The frame data is already in m_videoTexture
    return 1;
}

void Direct3D11VideoOutput::Refresh()
{
    if (m_isOpen && m_d3d11Graphics.GetDevice()) {
        onPaint(m_hwnd);
    }
}

void Direct3D11VideoOutput::timerCallback()
{
    // Could be used for animations or checking device status if needed
}

void Direct3D11VideoOutput::drawSubtitle(SubsItem* item)
{
    // TODO: Implement D3D11 subtitle rendering
    // This would involve creating text textures (e.g., using DirectWrite and D2D, then sharing surface with D3D11, or using a D3D11 font library)
    // and rendering them as textured quads.
}

void Direct3D11VideoOutput::resetSubtitle()
{
    // TODO: Clear any subtitle related resources or flags
}

void Direct3D11VideoOutput::setPalette(RGBQUAD* pal)
{
    // D3D11 doesn't use palettes in the same way DDraw/GDI did.
    // If the input format is palettized, it should be converted to RGB before creating the D3D11 texture,
    // or a shader would handle the palette lookup.
}

void Direct3D11VideoOutput::setVFlip(int on)
{
    m_flipOutput = (config_video_fliprgb ? !on : !!on);
    // This flag would be used in displayFrame when copying data to the texture,
    // or potentially by adjusting texture coordinates in the vertex shader.
}

void Direct3D11VideoOutput::OnWindowSize()
{
    if (m_d3d11Graphics.GetDevice()) {
        RECT clientRect;
        GetClientRect(m_hwnd, &clientRect);
        m_d3d11Graphics.OnResize(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

        // OSD and other elements might need resizing too.
        // if (posd && ((IVideoD3DOSD *)posd)->isOSDInited()) {
        //    ((IVideoD3DOSD *)posd)->UpdateOSD(m_hwnd, m_adjuster); // Assuming adjuster is compatible
        // }
    }
}

// Placeholder for VideoOutputBase if its definition is not available
// This is a basic structure based on what Direct3DVideoOutput might need.
// The actual definition should come from "videooutput.h".
#ifndef VIDEOOUTPUTBASE_DEFINED_PLACEHOLDER
#define VIDEOOUTPUTBASE_DEFINED_PLACEHOLDER
class VideoOutputBase {
public:
    virtual ~VideoOutputBase() {}
    virtual int OpenVideo(int w, int h, unsigned int type, int flipit, double aspectratio) = 0;
    virtual void displayFrame(const char* buf, int size, int time) = 0;
    virtual void close() = 0;
    virtual int onPaint(HWND hwnd) = 0;
    virtual void Refresh() = 0;
    virtual void timerCallback() = 0;
    virtual void drawSubtitle(SubsItem* item) = 0;
    virtual void resetSubtitle() = 0;
    virtual void setPalette(RGBQUAD* pal) = 0;
    virtual void setVFlip(int on) = 0;
    virtual void OnWindowSize() = 0;
};
#endif // VIDEOOUTPUTBASE_DEFINED_PLACEHOLDER
