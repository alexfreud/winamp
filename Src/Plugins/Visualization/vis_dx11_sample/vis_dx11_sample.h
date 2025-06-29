#pragma once

#include "../../../Winamp/VIS.H" // Path to Winamp's vis.h
#include "../../../../libs/dx11_engine/D3D11Graphics.h" // Path to our DX11 engine

#include <windows.h>
#include <d3d11.h>

// Example structure for a simple bar in the spectrum
struct VertexBar
{
    float pos[3]; // x, y, z
    float color[4]; // r, g, b, a
};

class DX11VisPlugin
{
public:
    DX11VisPlugin(winampVisModule* module);
    ~DX11VisPlugin();

    int Init();
    int Render();
    void Quit();
    void Config(HWND hwndParent);

private:
    bool CreateResources();
    void ReleaseResources();
    void UpdateSpectrumBars();

    winampVisModule* m_module; // Pointer to the module structure provided by Winamp
    D3D11Graphics m_d3d11Graphics;

    // D3D11 resources for a simple spectrum visualizer
    ID3D11Buffer*       m_vertexBuffer;
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader*  m_pixelShader;
    ID3D11InputLayout*  m_inputLayout;

    int m_numBars; // Number of spectrum bars
    // Add other necessary member variables
};

// Helper function to be called from the C-style plugin interface
DX11VisPlugin* GetPluginInstance(winampVisModule* module);
void ReleasePluginInstance(winampVisModule* module);
