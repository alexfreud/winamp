#pragma once

#include <d3d11.h>
#include <dxgi1_2.h> // For DXGI 1.2 interfaces, if needed later

class D3D11Graphics
{
public:
    D3D11Graphics();
    ~D3D11Graphics();

    bool InitializeDeviceAndSwapChain(HWND hwnd, int width, int height, bool fullscreen);
    void Shutdown();

    ID3D11Device* GetDevice() const { return m_d3dDevice; }
    ID3D11DeviceContext* GetImmediateContext() const { return m_immediateContext; }
    IDXGISwapChain* GetSwapChain() const { return m_swapChain; }
    ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView; }

    void ClearRenderTarget(float r, float g, float b, float a);
    void Present();

    void OnResize(int width, int height);

private:
    void CreateRenderTargetView();
    void ReleaseSizeDependentResources();

    ID3D11Device*           m_d3dDevice;
    ID3D11DeviceContext*    m_immediateContext;
    IDXGISwapChain*         m_swapChain;
    ID3D11RenderTargetView* m_renderTargetView;
    D3D_FEATURE_LEVEL       m_featureLevel;

    // Store window properties
    HWND m_hwnd;
    int m_width;
    int m_height;
    bool m_fullscreen;
};
