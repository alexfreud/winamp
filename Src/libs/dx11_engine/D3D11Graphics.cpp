#include "D3D11Graphics.h"
#include <comdef.h> // For _com_error

// Helper macro for error handling (simple version)
#define HR(x, msg) \
    do { \
        hr = (x); \
        if (FAILED(hr)) { \
            _com_error err(hr); \
            OutputDebugStringW(L msg L": "); \
            OutputDebugStringW(err.ErrorMessage()); \
            OutputDebugStringW(L"\n"); \
            return false; \
        } \
    } while (0)

D3D11Graphics::D3D11Graphics() :
    m_d3dDevice(nullptr),
    m_immediateContext(nullptr),
    m_swapChain(nullptr),
    m_renderTargetView(nullptr),
    m_featureLevel(D3D_FEATURE_LEVEL_11_0),
    m_hwnd(nullptr),
    m_width(0),
    m_height(0),
    m_fullscreen(false)
{
}

D3D11Graphics::~D3D11Graphics()
{
    Shutdown();
}

bool D3D11Graphics::InitializeDeviceAndSwapChain(HWND hwnd, int width, int height, bool fullscreen)
{
    m_hwnd = hwnd;
    m_width = width;
    m_height = height;
    m_fullscreen = fullscreen;
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1, // If targeting Win8+ and have appropriate SDK
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        hr = D3D11CreateDevice(
            nullptr,
            driverTypes[driverTypeIndex],
            nullptr,
            createDeviceFlags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &m_d3dDevice,
            &m_featureLevel,
            &m_immediateContext
        );

        if (SUCCEEDED(hr))
            break;
    }
    HR(hr, L"D3D11CreateDevice Failed");

    // Get DXGI Factory
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        HR(m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice)), L"QueryInterface IDXGIDevice Failed");

        IDXGIAdapter* adapter = nullptr;
        HR(dxgiDevice->GetAdapter(&adapter), L"GetAdapter Failed");

        HR(adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory)), L"GetParent IDXGIFactory1 Failed");

        adapter->Release();
        dxgiDevice->Release();
    }

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2; // Double buffered
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60; // Placeholder
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = !fullscreen;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // Or DXGI_SWAP_EFFECT_FLIP_DISCARD for more modern approach

    HR(dxgiFactory->CreateSwapChain(m_d3dDevice, &sd, &m_swapChain), L"CreateSwapChain Failed");
    dxgiFactory->Release();

    CreateRenderTargetView();

    // Set the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_immediateContext->RSSetViewports(1, &vp);

    return true;
}

void D3D11Graphics::CreateRenderTargetView()
{
    HRESULT hr;
    ID3D11Texture2D* pBackBuffer = nullptr;
    HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer)), L"GetBuffer Failed");

    HR(m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView), L"CreateRenderTargetView Failed");
    pBackBuffer->Release();

    // Typically you would also create a depth stencil view here if needed
    // m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
    m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
}

void D3D11Graphics::ReleaseSizeDependentResources()
{
    if (m_renderTargetView) m_renderTargetView->Release(); m_renderTargetView = nullptr;
    if (m_immediateContext) m_immediateContext->ClearState(); // Important before resizing swap chain
}


void D3D11Graphics::Shutdown()
{
    ReleaseSizeDependentResources();
    if (m_swapChain) m_swapChain->Release(); m_swapChain = nullptr;
    if (m_immediateContext) m_immediateContext->Release(); m_immediateContext = nullptr;
    if (m_d3dDevice) m_d3dDevice->Release(); m_d3dDevice = nullptr;
}

void D3D11Graphics::ClearRenderTarget(float r, float g, float b, float a)
{
    if (m_immediateContext && m_renderTargetView)
    {
        const float clearColor[4] = { r, g, b, a };
        m_immediateContext->ClearRenderTargetView(m_renderTargetView, clearColor);
    }
}

void D3D11Graphics::Present()
{
    if (m_swapChain)
    {
        // First parameter is VSync interval. 0 for no vsync, 1 for vsync.
        m_swapChain->Present(1, 0);
    }
}

void D3D11Graphics::OnResize(int width, int height)
{
    if (!m_swapChain || !m_d3dDevice || !m_immediateContext)
        return;

    m_width = width;
    m_height = height;

    ReleaseSizeDependentResources();

    HRESULT hr;
    HR(m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0), L"ResizeBuffers Failed");

    CreateRenderTargetView();

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_immediateContext->RSSetViewports(1, &vp);
}
